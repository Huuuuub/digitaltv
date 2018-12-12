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
//  HTTP stream input
//
//----------------------------------------------------------------------------

#include "tsPushInputPlugin.h"
#include "tsPluginRepository.h"
#include "tsWebRequest.h"
#include "tsSysUtils.h"
TSDUCK_SOURCE;

#define DEFAULT_MAX_QUEUED_PACKETS  1000    // Default size in packet of the inter-thread queue.


//----------------------------------------------------------------------------
// Plugin definition
//----------------------------------------------------------------------------

namespace ts {
    class HttpInput: public PushInputPlugin, private WebRequestHandlerInterface
    {
    public:
        // Implementation of plugin API
        HttpInput(TSP*);
        virtual bool getOptions() override;
        virtual bool start() override;
        virtual void processInput() override;

        // Implementation of WebRequestHandlerInterface
        virtual bool handleWebStart(const WebRequest& request, size_t size) override;
        virtual bool handleWebData(const WebRequest& request, const void* data, size_t size) override;

    private:
        size_t      _repeat_count;
        bool        _ignore_errors;
        MilliSecond _reconnect_delay;
        WebRequest  _request;
        TSPacket    _partial;       // Buffer for incomplete packets.
        size_t      _partial_size;  // Number of bytes in partial.

        // Inaccessible operations
        HttpInput() = delete;
        HttpInput(const HttpInput&) = delete;
        HttpInput& operator=(const HttpInput&) = delete;
    };
}

TSPLUGIN_DECLARE_VERSION
TSPLUGIN_DECLARE_INPUT(http, ts::HttpInput)


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

ts::HttpInput::HttpInput(TSP* tsp_) :
    PushInputPlugin(tsp_, u"Read a transport stream from an HTTP server", u"[options] url"),
    _repeat_count(0),
    _ignore_errors(false),
    _reconnect_delay(0),
    _request(*tsp),
    _partial(),
    _partial_size(0)
{
    option(u"", 0, STRING, 1, 1);
    help(u"",
         u"Specify the URL from which to read the transport stream.");

    option(u"connection-timeout", 0, POSITIVE);
    help(u"connection-timeout",
         u"Specify the connection timeout in milliseconds. By default, let the "
         u"operating system decide.");

    option(u"ignore-errors");
    help(u"ignore-errors",
         u"With --repeat or --infinite, repeat also in case of error. By default, "
         u"repetition stops on error.");

    option(u"infinite", 'i');
    help(u"infinite",
         u"Repeat the playout of the content infinitely (default: only once). "
         u"The URL is re-opened each time and the content may be different.");

    option(u"max-queue", 0, POSITIVE);
    help(u"max-queue",
         u"Specify the maximum number of queued TS packets before their "
         u"insertion into the stream. The default is " +
         UString::Decimal(DEFAULT_MAX_QUEUED_PACKETS) + u".");

    option(u"proxy-host", 0, STRING);
    help(u"proxy-host", u"name",
         u"Optional proxy host name for Internet access.");

    option(u"proxy-password", 0, STRING);
    help(u"proxy-password", u"string",
         u"Optional proxy password for Internet access (for use with --proxy-user).");

    option(u"proxy-port", 0, UINT16);
    help(u"proxy-port",
         u"Optional proxy port for Internet access (for use with --proxy-host).");

    option(u"proxy-user", 0, STRING);
    help(u"proxy-user", u"name",
         u"Optional proxy user name for Internet access.");

    option(u"receive-timeout", 0, POSITIVE);
    help(u"receive-timeout",
         u"Specify the data reception timeout in milliseconds. This timeout applies "
         u"to each receive operation, individually. By default, let the operating "
         u"system decide.");

    option(u"reconnect-delay", 0, UNSIGNED);
    help(u"reconnect-delay",
         u"With --repeat or --infinite, wait the specified number of milliseconds "
         u"before reconnecting. By default, repeat immediately.");

    option(u"repeat", 'r', POSITIVE);
    help(u"repeat", u"count",
         u"Repeat the playout of the content the specified number of times "
         u"(default: only once). The URL is re-opened each time and the content "
         u"may be different.");
}


//----------------------------------------------------------------------------
// Command line options method
//----------------------------------------------------------------------------

bool ts::HttpInput::getOptions()
{
    // Decode options.
    _repeat_count = intValue<size_t>(u"repeat", present(u"infinite") ? std::numeric_limits<size_t>::max() : 1);
    _reconnect_delay = intValue<MilliSecond>(u"reconnect-delay", 0);
    _ignore_errors = present(u"ignore-errors");

    // Resize the inter-thread packet queue.
    setQueueSize(intValue<size_t>(u"max-queue", DEFAULT_MAX_QUEUED_PACKETS));

    // Prepare web request.
    _request.setURL(value(u""));
    _request.setAutoRedirect(true);
    _request.setProxyHost(value(u"proxy-host"), intValue<uint16_t>(u"proxy-port"));
    _request.setProxyUser(value(u"proxy-user"), value(u"proxy-password"));
    if (present(u"connection-timeout")) {
        _request.setConnectionTimeout(intValue<MilliSecond>(u"connection-timeout"));
    }

    return true;
}


//----------------------------------------------------------------------------
// Start method
//----------------------------------------------------------------------------

bool ts::HttpInput::start()
{
    _partial_size = 0;

    // Invoke superclass.
    return PushInputPlugin::start();
}


//----------------------------------------------------------------------------
// Input method. Executed in a separate thread.
//----------------------------------------------------------------------------

void ts::HttpInput::processInput()
{
    bool ok = true;

    // Loop on request count.
    for (size_t count = 0; count < _repeat_count && (ok || _ignore_errors); count++) {
        // Wait between reconnections.
        if (count > 0 && _reconnect_delay > 0) {
            SleepThread(_reconnect_delay);
        }
        // Perform one download.
        ok = _request.downloadToApplication(this);
    }
}


//----------------------------------------------------------------------------
// This hook is invoked at the beginning of the transfer.
//----------------------------------------------------------------------------

bool ts::HttpInput::handleWebStart(const WebRequest& request, size_t size)
{
    // Get complete MIME type.
    const UString mime(request.reponseHeader(u"Content-Type"));

    // Get initial type, before ';'.
    UStringVector types;
    mime.split(types, u';');
    types.resize(1);

    // Print a message.
    tsp->verbose(u"downloading from %s", {request.finalURL()});
    tsp->verbose(u"MIME type: %s, expected size: %s", {mime.empty() ? u"unknown" : mime, size == 0 ? u"unknown" : UString::Format(u"%d bytes", {size})});
    if (!types[0].empty() && !types[0].similar(u"video/mp2t")) {
        tsp->warning(u"MIME type is %d, maybe not a valid transport stream", {types[0]});
    }

    // Reinitialize partial packet if some bytes were left from a previous iteration.
    _partial_size = 0;
    return true;
}


//----------------------------------------------------------------------------
// This hook is invoked when a data chunk is available.
//----------------------------------------------------------------------------

bool ts::HttpInput::handleWebData(const WebRequest& request, const void* addr, size_t size)
{
    const uint8_t* data = reinterpret_cast<const uint8_t*>(addr);

    // If a partial packet is present, try to fill it.
    if (_partial_size > 0) {
        // Copy more data into partial packet.
        assert(_partial_size <= PKT_SIZE);
        const size_t more = std::min(size, PKT_SIZE - _partial_size);
        ::memcpy(_partial.b + _partial_size, data, more);

        data += more;
        size -= more;
        _partial_size += more;

        // If the partial packet is full, push it.
        if (_partial_size == PKT_SIZE) {
            if (!pushPackets(&_partial, 1)) {
                tsp->debug(u"error pushing packets");
                return false;
            }
            _partial_size = 0;
        }
    }

    // Compute number of complete packets to push.
    const size_t residue = size % PKT_SIZE;
    const size_t count = (size - residue) / PKT_SIZE;

    // Push complete packets.
    if (count > 0 && !pushPackets(reinterpret_cast<const TSPacket*>(data), count)) {
        tsp->debug(u"error pushing packets");
        return false;
    }

    // Save residue in partial packet.
    if (residue > 0) {
        ::memcpy(_partial.b, data + size - residue, residue);
        _partial_size = residue;
    }

    return true;
}
