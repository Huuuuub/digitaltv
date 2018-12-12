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
//  Control HiDes modulator devices.
//
//----------------------------------------------------------------------------

#include "tsMain.h"
#include "tsCOM.h"
#include "tsHiDesDevice.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
//  Command line options
//----------------------------------------------------------------------------

class HiDesOptions: public ts::Args
{
public:
    bool          count;       // Only display device count.
    bool          gain_range;  // Only display output gain range.
    int           dev_number;  // Device adapter number.
    ts::UString   dev_name;    // Device name.
    uint64_t      frequency;   // Carrier frequency, in Hz.
    ts::BandWidth bandwidth;   // Bandwidth.

    // Constructor:
    HiDesOptions(int argc, char *argv[]);
};

// Constructor.
HiDesOptions::HiDesOptions(int argc, char *argv[]) :
    ts::Args(u"List HiDes modulator devices", u"[options]"),
    count(false),
    gain_range(false),
    dev_number(-1),
    dev_name(),
    frequency(0),
    bandwidth(ts::BW_8_MHZ)
{
    option(u"adapter", 'a', UNSIGNED);
    help(u"adapter", u"Specify the HiDes adapter number to list. By default, list all HiDes devices.");

    option(u"bandwidth", 'b', ts::Enumeration({
        {u"5", ts::BW_5_MHZ},
        {u"6", ts::BW_5_MHZ},
        {u"7", ts::BW_7_MHZ},
        {u"8", ts::BW_8_MHZ},
    }));
    help(u"bandwidth", u"Bandwidth in MHz with --gain-range. The default is 8 MHz.");

    option(u"count", 'c');
    help(u"count", u"Only display the number of devices.");

    option(u"device", 'd', STRING);
    help(u"device", u"name",
         u"Specify the HiDes device name to list. "
         u"By default, list all HiDes devices.");

    option(u"frequency", 'f', POSITIVE);
    help(u"frequency",
         u"Frequency, in Hz, of the output carrier with --gain-range. "
         u"The default is the first UHF channel.");

    option(u"gain-range", 'g');
    help(u"gain-range",
         u"Display the allowed range of output gain for the specified device, "
         u"using the specified frequency and bandwidth.");

    analyze(argc, argv);

    count = present(u"count");
    gain_range = present(u"gain-range");
    dev_number = intValue<int>(u"adapter", -1);
    dev_name = value(u"device");
    bandwidth = enumValue<ts::BandWidth>(u"bandwidth", ts::BW_8_MHZ);
    frequency = intValue<uint64_t>(u"frequency", ts::UHF::Frequency(ts::UHF::FIRST_CHANNEL));

    if (count && gain_range) {
        error(u"--count and --gain-range are mutually exclusive");
    }

    exitOnError();
}


//----------------------------------------------------------------------------
//  Main code. Isolated from main() to ensure that destructors are invoked
//  before COM uninitialize.
//----------------------------------------------------------------------------

namespace {
    void MainCode(HiDesOptions& opt)
    {
        ts::HiDesDevice dev;
        ts::HiDesDeviceInfo info;
        ts::HiDesDeviceInfoList devices;
        const bool one_device = opt.dev_number >= 0 || !opt.dev_name.empty();
        bool ok = false;

        // Open one device or get all devices.
        if (!opt.gain_range && !one_device) {
            // Get all HiDes devices.
            ok = ts::HiDesDevice::GetAllDevices(devices, opt);
        }
        else if (!opt.dev_name.empty()) {
            // Open one device by name.
            ok = dev.open(opt.dev_name, opt);
        }
        else {
            // One one device by number (default: first device).
            ok = dev.open(std::max<int>(0, opt.dev_number), opt);
        }

        if (!ok) {
            return;
        }
        else if (opt.count) {
            // Display device count.
            std::cout << devices.size() << std::endl;
        }
        else if (opt.gain_range) {
            // Display gain range.
            int min, max;
            if (dev.getInfo(info, opt) && dev.getGainRange(min, max, opt.frequency, opt.bandwidth, opt)) {
                std::cout << ts::UString::Format(u"Device: %s", {info.toString()}) << std::endl
                          << ts::UString::Format(u"Frequency: %'d Hz", {opt.frequency}) << std::endl
                          << ts::UString::Format(u"Bandwidth: %s", {ts::BandWidthEnum.name(opt.bandwidth)}) << std::endl
                          << ts::UString::Format(u"Min. gain: %d dB", {min}) << std::endl
                          << ts::UString::Format(u"Max. gain: %d dB", {max}) << std::endl;
            }
        }
        else if (one_device) {
            // Display one device.
            if (dev.getInfo(info, opt)) {
                std::cout << info.toString(opt.verbose()) << std::endl;
            }
        }
        else if (devices.empty()) {
            std::cout << "No HiDes device found" << std::endl;
        }
        else {
            // Display all devices.
            if (opt.verbose()) {
                std::cout << "Found " << devices.size() << " HiDes device" << (devices.size() > 1 ? "s" : "") << std::endl << std::endl;
            }
            for (auto it = devices.begin(); it != devices.end(); ++it) {
                std::cout << it->toString(opt.verbose()) << std::endl;
            }
        }
    }
}


//----------------------------------------------------------------------------
//  Program entry point
//----------------------------------------------------------------------------

int MainCode(int argc, char *argv[])
{
    HiDesOptions opt(argc, argv);
    ts::COM com(opt);

    if (com.isInitialized()) {
        MainCode(opt);
    }

    opt.exitOnError();
    return EXIT_SUCCESS;
}

TS_MAIN(MainCode)
