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

#include "tsAbstractTablePlugin.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

ts::AbstractTablePlugin::AbstractTablePlugin(TSP* tsp_,
                                             const UString& description,
                                             const UString& syntax,
                                             const UString& table_name,
                                             PID pid,
                                             BitRate default_bitrate,
                                             const UString& new_table_help) :
    ProcessorPlugin(tsp_, description, syntax),
    _abort(false),
    _table_name(table_name),
    _default_bitrate(default_bitrate),
    _pid(pid),
    _found(false),
    _pkt_current(0),
    _pkt_create(0),
    _pkt_insert(0),
    _create_after_ms(0),
    _bitrate(0),
    _inter_pkt(0),
    _incr_version(false),
    _set_version(false),
    _new_version(0),
    _demux(this),
    _pzer(pid)
{
    option(u"bitrate", 'b', POSITIVE);
    help(u"bitrate",
         u"Specifies the bitrate in bits / second of the " + _table_name + " PID if a new one is "
         u"created. The default is " + UString::Decimal(_default_bitrate) + u" b/s.");

    option(u"create", 'c');
    help(u"create",
         u"Create a new empty " + _table_name + u" if none was received after one second. This is "
         u"equivalent to --create-after 1000.");

    option(u"create-after", 0, POSITIVE);
    help(u"create-after",
         u"Create a new empty " + _table_name + u" if none was received after the specified number "
         u"of milliseconds. If the actual " + _table_name + u" is received later, it will be used "
         u"as the base for transformations instead of the empty one." +
         UString(new_table_help.empty() ? u"" : u"\n\n") + new_table_help);

    option(u"increment-version", 'i');
    help(u"increment-version",
         u"Increment the version number of the " + _table_name + u".");

    option(u"inter-packet", 0, POSITIVE);
    help(u"inter-packet",
         u"When a new " + _table_name + u" is created and --bitrate is not present, this option "
         u"specifies the packet interval for the PID, that is to say the number of TS packets in "
         u"the transport between two packets of the PID. Use instead of --bitrate if the global "
         u"bitrate of the TS cannot be determined.");

    option(u"new-version", 'v', INTEGER, 0, 1, 0, 31);
    help(u"new-version",
         u"Specify a new value for the version of the " + _table_name + u".");
}


//----------------------------------------------------------------------------
// Set a new PID to process.
//----------------------------------------------------------------------------

void ts::AbstractTablePlugin::setPID(PID pid)
{
    // Reset demux and packetizer if we change PID.
    if (pid != _pid) {
        _pid = pid;
        _demux.reset();
        _demux.addPID(_pid);
        _pzer.reset();
        _pzer.setPID(_pid);
    }
}


//----------------------------------------------------------------------------
// Start method
//----------------------------------------------------------------------------

bool ts::AbstractTablePlugin::start()
{
    // Get option values
    _incr_version = present(u"increment-version");
    _create_after_ms = present(u"create") ? 1000 : intValue<MilliSecond>(u"create-after", 0);
    _bitrate = intValue<BitRate>(u"bitrate", _default_bitrate);
    _inter_pkt = intValue<PacketCounter>(u"inter-packet", 0);
    _set_version = present(u"new-version");
    _new_version = intValue<uint8_t>(u"new-version", 0);

    if (present(u"create") && present(u"create-after")) {
        tsp->error(u"options --create and --create-after are mutually exclusive");
        return false;
    }

    // Initialize the demux and packetizer
    _demux.reset();
    _demux.addPID(_pid);
    _pzer.reset();
    _pzer.setPID(_pid);

    // Reset other states
    _found = false;
    _pkt_current = 0;
    _pkt_create = 0;
    _pkt_insert = 0;

    return true;
}


//----------------------------------------------------------------------------
// Invoked by the demux when a table is found on the input PID.
//----------------------------------------------------------------------------

void ts::AbstractTablePlugin::handleTable(SectionDemux&, const BinaryTable& intable)
{
    // A modifiable version of the table.
    BinaryTable table(intable, SHARE);
    const int old_version = table.version();

    // Call subclass to process the table.
    bool is_target = true;
    bool reinsert = true;
    modifyTable(table, is_target, reinsert);

    // Case of the target table.
    if (is_target) {
        // If the target table is found, no longer need to create a new one.
        _found = true;
        _pkt_insert = 0;

        // Modify the table version.
        if (_incr_version) {
            table.setVersion((table.version() + 1) & 0x1F);
        }
        else if (_set_version) {
            table.setVersion(_new_version);
        }
    }

    // Place modified table in the packetizer.
    if (reinsert) {
        if (is_target) {
            tsp->verbose(u"%s version %d modified", {_table_name, old_version});
        }
        if (table.isShortSection()) {
            _pzer.removeSections(table.tableId());
        }
        else {
            _pzer.removeSections(table.tableId(), table.tableIdExtension());
        }
        _pzer.addTable(table);
    }
}


//----------------------------------------------------------------------------
// Packet processing method
//----------------------------------------------------------------------------

ts::ProcessorPlugin::Status ts::AbstractTablePlugin::processPacket(TSPacket& pkt, bool& flush, bool& bitrate_changed)
{
    const PID pid = pkt.getPID();

    // Count packets
    _pkt_current++;

    // Filter incoming sections
    _demux.feedPacket(pkt);

    // Determine when a new table shall be created. Executed only once, when the bitrate is known
    if (!_found && _create_after_ms > 0 && _pkt_create == 0) {
        const BitRate ts_bitrate = tsp->bitrate();
        _pkt_create = PacketDistance(ts_bitrate, _create_after_ms);
        tsp->debug(u"will create %s after %'d packets, %'d ms (bitrate: %'d b/s)", {_table_name, _pkt_create, _create_after_ms, ts_bitrate});
    }

    // Create a new table when necessary.
    if (!_found && _pkt_create > 0 && _pkt_current >= _pkt_create) {
        // Let the subclass create a new empty table.
        BinaryTable table;
        createNewTable(table);
        // Process it as if it comes from the TS.
        handleTable(_demux, table);
        // Insert first packet as soon as possible
        _pkt_insert = _pkt_current;
    }

    // Insertion of packets from the input PID.
    if (pid == PID_NULL && _pkt_insert > 0 && _pkt_current >= _pkt_insert) {
        // It is time to replace stuffing by a created table packet.
        _pzer.getNextPacket(pkt);
        // Next insertion point.
        if (_inter_pkt != 0) {
            // Packet interval was explicitly specified for the created PID.
            _pkt_insert += _inter_pkt;
        }
        else {
            // Compute packet interval from bitrates.
            const BitRate ts_bitrate = tsp->bitrate();
            if (ts_bitrate < _bitrate) {
                tsp->error(u"input bitrate unknown or too low, specify --inter-packet instead of --bitrate");
                return TSP_END;
            }
            _pkt_insert += ts_bitrate / _bitrate;
        }
    }
    else if (pid == _pid) {
        // Replace an existing input PID packet.
        _pzer.getNextPacket(pkt);
    }

    return _abort ? TSP_END : TSP_OK;
}
