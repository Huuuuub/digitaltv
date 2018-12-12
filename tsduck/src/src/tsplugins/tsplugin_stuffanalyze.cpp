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
//  Analyze the level of stuffing in tables.
//
//----------------------------------------------------------------------------

#include "tsPlugin.h"
#include "tsPluginRepository.h"
#include "tsCASSelectionArgs.h"
#include "tsSectionDemux.h"
#include "tsTables.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// Plugin definition
//----------------------------------------------------------------------------

namespace ts {
    class StuffAnalyzePlugin:
        public ProcessorPlugin,
        private TableHandlerInterface,
        private SectionHandlerInterface
    {
    public:
        // Implementation of plugin API
        StuffAnalyzePlugin(TSP*);
        virtual bool start() override;
        virtual bool stop() override;
        virtual Status processPacket(TSPacket&, bool&, bool&) override;

    private:
        // Analysis context for a PID.
        class PIDContext
        {
        public:
            uint64_t total_sections;     // Total number of sections.
            uint64_t stuffing_sections;  // Number of stuffing sections.
            uint64_t total_bytes;        // Total number of bytes in sections.
            uint64_t stuffing_bytes;     // Total number of bytes in stuffing sections.

            // Constructor.
            PIDContext();

            // Format as a string.
            UString toString() const;
        };

        typedef SafePtr<PIDContext, NullMutex> PIDContextPtr;
        typedef std::map<PID, PIDContextPtr> PIDContextMap;

        // Plugin private fields.
        UString          _output_name;    // Output file name
        std::ofstream    _output_stream;  // Output file stream
        std::ostream*    _output;         // Actual output stream
        CASSelectionArgs _cas_args;       // CAS selection
        PIDSet           _analyze_pids;   // List of PIDs to pass
        SectionDemux     _analyze_demux;  // Demux for sections to analyze for stuffing
        SectionDemux     _psi_demux;      // Demux for PSI parsing
        PIDContext       _total;          // Global context.
        PIDContextMap    _pid_contexts;   // Contexts of analyzed PID's.

        // Invoked by the demux when a complete table is available.
        virtual void handleTable(SectionDemux&, const BinaryTable&) override;
        virtual void handleSection(SectionDemux&, const Section&) override;

        // Inaccessible operations
        StuffAnalyzePlugin() = delete;
        StuffAnalyzePlugin(const StuffAnalyzePlugin&) = delete;
        StuffAnalyzePlugin& operator=(const StuffAnalyzePlugin&) = delete;
    };
}

TSPLUGIN_DECLARE_VERSION
TSPLUGIN_DECLARE_PROCESSOR(stuffanalyze, ts::StuffAnalyzePlugin)


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

ts::StuffAnalyzePlugin::StuffAnalyzePlugin(TSP* tsp_) :
    ProcessorPlugin(tsp_, u"Analyze the level of stuffing in tables", u"[options]"),
    TableHandlerInterface(),
    SectionHandlerInterface(),
    _output_name(),
    _output_stream(),
    _output(nullptr),
    _cas_args(),
    _analyze_pids(),
    _analyze_demux(nullptr, this),  // this one intercepts all sections for stuffing analysis
    _psi_demux(this, nullptr),      // this one is used for PSI parsing
    _total(),
    _pid_contexts()
{
    option(u"output-file", 'o', STRING);
    help(u"output-file",
         u"Specify the output text file for the analysis result. "
         u"By default, use the standard output.\n\n"
         u"Analyze the level of \"stuffing\" in sections in a list of selected PID's. "
         u"The PID's to analyze can be selected manually or using CAS criteria. "
         u"A section is considered as \"stuffing\" when its payload is filled with "
         u"the same byte value (all 0x00 or all 0xFF for instance).");

    option(u"pid", 'p', PIDVAL, 0, UNLIMITED_COUNT);
    help(u"pid", u"Analyze all tables from this PID. Several -p or --pid options may be specified.");

    // CAS filtering options.
    _cas_args.defineOptions(*this);
}


//----------------------------------------------------------------------------
// Start method
//----------------------------------------------------------------------------

bool ts::StuffAnalyzePlugin::start()
{
    // Get command line arguments
    _cas_args.load(*this);
    _output_name = value(u"output-file");
    getIntValues(_analyze_pids, u"pid");

    // Initialize the PSI demux.
    _psi_demux.reset();
    if (_cas_args.pass_emm) {
        // To get the EMM PID's we need to analyze the CAT.
        _psi_demux.addPID(PID_CAT);
    }
    if (_cas_args.pass_ecm) {
        // To get the ECM PID's we need to analyze the PMT's.
        // To get the PMT PID's, we need to analyze the PAT.
        _psi_demux.addPID(PID_PAT);
    }

    // Initialize the demux which analyzes sections.
    _analyze_demux.setPIDFilter(_analyze_pids);

    // Create the output file.
    if (_output_name.empty()) {
        _output = &std::cout;
    }
    else {
        _output = &_output_stream;
        _output_stream.open(_output_name.toUTF8().c_str());
        if (!_output_stream) {
            tsp->error(u"cannot create file %s", {_output_name});
            return false;
        }
    }

    return true;
}


//----------------------------------------------------------------------------
// PID context constructor.
//----------------------------------------------------------------------------

ts::StuffAnalyzePlugin::PIDContext::PIDContext() :
    total_sections(0),
    stuffing_sections(0),
    total_bytes(0),
    stuffing_bytes(0)
{
}


//----------------------------------------------------------------------------
// Format a PID context as a string.
//----------------------------------------------------------------------------

ts::UString ts::StuffAnalyzePlugin::PIDContext::toString() const
{
    return UString::Format(u"%10d %10d %10d %10d %9s",
                           {total_sections, stuffing_sections, total_bytes, stuffing_bytes,
                            UString::Percentage(stuffing_bytes, total_bytes)});
}


//----------------------------------------------------------------------------
// Stop method
//----------------------------------------------------------------------------

bool ts::StuffAnalyzePlugin::stop()
{
    // Now it is time to produce the report.
    (*_output) << "Number of analyzed PID's: " << _analyze_pids.count() << std::endl
               << "PID's with sections:      " << _pid_contexts.size() << std::endl
               << std::endl
               << "PID             Sections (stuffing)      Bytes (stuffing) (percent)" << std::endl
               << "------------- ---------- ---------- ---------- ---------- ---------" << std::endl;

    for (PIDContextMap::const_iterator it = _pid_contexts.begin(); it != _pid_contexts.end(); ++it) {
        const PID pid = it->first;
        const PIDContextPtr& ctx(it->second);
        if (!ctx.isNull()) {
            (*_output) << UString::Format(u"%4d (0x%04X) ", {pid, pid}) << ctx->toString() << std::endl;
        }
    }
    (*_output) << "Total         " << _total.toString() << std::endl;

    // Close output file
    if (!_output_name.empty()) {
        _output_stream.close();
    }

    return true;
}


//----------------------------------------------------------------------------
// Invoked by the demux when a complete PSI table is available.
//----------------------------------------------------------------------------

void ts::StuffAnalyzePlugin::handleTable(SectionDemux& demux, const BinaryTable& table)
{
    switch (table.tableId()) {

        case TID_PAT: {
            // Add all PMT PID's to PSI demux.
            PAT pat(table);
            if (pat.isValid() && table.sourcePID() == PID_PAT) {
                for (PAT::ServiceMap::const_iterator it = pat.pmts.begin(); it != pat.pmts.end(); ++it) {
                    _psi_demux.addPID(it->second);
                }
            }
            break;
        }

        case TID_CAT: {
            // Analyze stuffing on all required EMM PID's.
            CAT cat(table);
            if (cat.isValid() && table.sourcePID() == PID_CAT) {
                PIDSet pids;
                _cas_args.addMatchingPIDs(pids, cat, *tsp);
                _analyze_demux.addPIDs(pids);
                _analyze_pids |= pids;
            }
            break;
        }

        case TID_PMT: {
            // Analyze stuffing on all required EMM PID's.
            PMT pmt(table);
            if (pmt.isValid()) {
                PIDSet pids;
                _cas_args.addMatchingPIDs(pids, pmt, *tsp);
                _analyze_demux.addPIDs(pids);
                _analyze_pids |= pids;
            }
            break;
        }

        default: {
            break;
        }
    }
}


//----------------------------------------------------------------------------
// Invoked by the demux for each section to analyze.
//----------------------------------------------------------------------------

void ts::StuffAnalyzePlugin::handleSection(SectionDemux& demux, const Section& section)
{
    // Locate the PID context.
    const PID pid = section.sourcePID();
    PIDContextPtr ctx(_pid_contexts[pid]);
    if (ctx.isNull()) {
        // First section on this PID, allocate a context.
        // Note that the new context becomes managed by the safe pointer (assignment magic).
        ctx = new PIDContext;
        _pid_contexts[pid] = ctx;
    }

    // Count sizes.
    ctx->total_sections += 1;
    ctx->total_bytes += section.size();
    _total.total_sections += 1;
    _total.total_bytes += section.size();

    if (!section.hasDiversifiedPayload()) {
        // The section payload is full of identical values, all 00, all FF, whatever.
        // We consider this as a stuffing section.
        ctx->stuffing_sections += 1;
        ctx->stuffing_bytes += section.size();
        _total.stuffing_sections += 1;
        _total.stuffing_bytes += section.size();
    }
}


//----------------------------------------------------------------------------
// Packet processing method
//----------------------------------------------------------------------------

ts::ProcessorPlugin::Status ts::StuffAnalyzePlugin::processPacket(TSPacket& pkt, bool& flush, bool& bitrate_changed)
{
    _psi_demux.feedPacket(pkt);
    _analyze_demux.feedPacket(pkt);
    return TSP_OK;
}
