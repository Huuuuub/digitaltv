//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2018, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------
//
//  Transport stream processor shared library:
//  Regulate (slow down) the packet flow according to a bitrate.
//
//----------------------------------------------------------------------------

#include "tsPlugin.h"
#include "tsPluginRepository.h"
#include "tsBitRateRegulator.h"
#include "tsPCRRegulator.h"
TSDUCK_SOURCE;

#define DEF_PACKET_BURST 16


//----------------------------------------------------------------------------
// Plugin definition
//----------------------------------------------------------------------------

namespace ts {
    class RegulatePlugin: public ProcessorPlugin
    {
    public:
        // Implementation of plugin API
        RegulatePlugin(TSP*);
        virtual bool start() override;
        virtual bool isRealTime() override {return true;}
        virtual Status processPacket(TSPacket&, bool&, bool&) override;

    private:
        bool             _pcr_synchronous;
        BitRateRegulator _bitrate_regulator;
        PCRRegulator     _pcr_regulator;

        // Inaccessible operations
        RegulatePlugin() = delete;
        RegulatePlugin(const RegulatePlugin&) = delete;
        RegulatePlugin& operator=(const RegulatePlugin&) = delete;
    };
}

TSPLUGIN_DECLARE_VERSION
TSPLUGIN_DECLARE_PROCESSOR(regulate, ts::RegulatePlugin)


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

ts::RegulatePlugin::RegulatePlugin(TSP* tsp_) :
    ProcessorPlugin(tsp_, u"Regulate the TS packets flow based on PCR or bitrate", u"[options]"),
    _pcr_synchronous(false),
    _bitrate_regulator(tsp, Severity::Verbose),
    _pcr_regulator(tsp, Severity::Verbose)
{
    option(u"bitrate", 'b', POSITIVE);
    help(u"bitrate",
         u"Specify a bitrate in b/s and regulate (slow down only) the TS packets "
         u"flow according to this bitrate. By default, use the \"input\" bitrate, "
         u"typically resulting from the PCR analysis of the input file.");

    option(u"packet-burst", 'p', POSITIVE);
    help(u"packet-burst",
         u"Number of packets to burst at a time. Does not modify the average "
         u"output bitrate but influence smoothing and CPU load. The default "
         u"is " TS_STRINGIFY(DEF_PACKET_BURST) u" packets.");

    option(u"pcr-synchronous");
    help(u"pcr-synchronous",
         u"Regulate the flow based on the Program Clock Reference from the transport "
         u"stream. By default, use a bitrate, not PCR's.");

    option(u"pid-pcr", 0, PIDVAL);
    help(u"pid-pcr",
         u"With --pcr-synchronous, specify the reference PID for PCR's. By default, "
         u"use the first PID containing PCR's.");

    option(u"wait-min", 'w', POSITIVE);
    help(u"wait-min",
         u"With --pcr-synchronous, specify the minimum wait time in milli-seconds. "
         u"The default is " + UString::Decimal(PCRRegulator::DEFAULT_MIN_WAIT_NS / NanoSecPerMilliSec) + u" ms.");
}


//----------------------------------------------------------------------------
// Start method
//----------------------------------------------------------------------------

bool ts::RegulatePlugin::start()
{
    _pcr_synchronous = present(u"pcr-synchronous");
    const bool has_bitrate = present(u"bitrate");
    const BitRate bitrate = intValue<BitRate>(u"bitrate", 0);
    const bool has_pid = present(u"pid-pcr");
    const PID pid = intValue<PID>(u"pid-pcr", PID_NULL);
    const PacketCounter burst = intValue<PacketCounter>(u"packet-burst", DEF_PACKET_BURST);
    const MilliSecond wait_min = intValue<MilliSecond>(u"wait-min", PCRRegulator::DEFAULT_MIN_WAIT_NS / NanoSecPerMilliSec);

    if (has_bitrate && _pcr_synchronous) {
        tsp->error(u"--bitrate cannot be used with --pcr-synchronous");
        return false;
    }
    if (has_pid && !_pcr_synchronous) {
        tsp->error(u"--pid-pcr cannot be used without --pcr-synchronous");
        return false;
    }

    // Initialize the appropriate regulator.
    if (_pcr_synchronous) {
        _pcr_regulator.reset();
        _pcr_regulator.setBurstPacketCount(burst);
        _pcr_regulator.setReferencePID(pid);
        _pcr_regulator.setMinimimWait(wait_min * NanoSecPerMilliSec);
    }
    else {
        _bitrate_regulator.setBurstPacketCount(burst);
        _bitrate_regulator.setFixedBitRate(bitrate);
        _bitrate_regulator.start();
    }
    return true;
}


//----------------------------------------------------------------------------
// Packet processing method
//----------------------------------------------------------------------------

ts::ProcessorPlugin::Status ts::RegulatePlugin::processPacket(TSPacket& pkt, bool& flush, bool& bitrate_changed)
{
    if (_pcr_synchronous) {
        flush = _pcr_regulator.regulate(pkt);
    }
    else {
        _bitrate_regulator.regulate(tsp->bitrate(), flush, bitrate_changed);
    }
    return TSP_OK;
}
