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
//  Various transformations on the PMT.
//
//----------------------------------------------------------------------------

#include "tsAbstractTablePlugin.h"
#include "tsPluginRepository.h"
#include "tsServiceDiscovery.h"
#include "tsTables.h"
#include "tsAudioLanguageOptions.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// Plugin definition
//----------------------------------------------------------------------------

namespace ts {
    class PMTPlugin: public AbstractTablePlugin
    {
    public:
        // Implementation of plugin API
        PMTPlugin(TSP*);
        virtual bool start() override;
        virtual Status processPacket(TSPacket&, bool&, bool&) override;

    private:
        // Description of a new component to add
        struct NewPID {
            PID     pid;
            uint8_t type;

            // Constructor.
            NewPID(PID pid_ = PID_NULL, uint8_t stype_ = 0) :
                pid(pid_),
                type(stype_)
            {
            }
        };

        // Map of new descriptors to add per component.
        typedef std::map<PID, SafePtr<DescriptorList>> DescriptorListByPID;

        // PMTPlugin instance fields
        ServiceDiscovery     _service;             // Service of PMT to modify
        std::vector<PID>     _removed_pid;         // Set of PIDs to remove from PMT
        std::vector<DID>     _removed_desc;        // Set of descriptor tags to remove
        std::vector<uint8_t> _removed_stream;      // Set of stream types to remove
        std::list<NewPID>    _added_pid;           // List of PID to add
        std::map<PID,PID>    _moved_pid;           // List of renamed PID's in PMT (key=old, value=new)
        bool                 _set_servid;          // Set a new service id
        uint16_t             _new_servid;          // New service id
        bool                 _set_pcrpid;          // Set a new PCR PID
        PID                  _new_pcrpid;          // New PCR PID
        PDS                  _pds;                 // Private data specifier for removed descriptors
        bool                 _add_stream_id;       // Add stream_identifier_descriptor on all components
        bool                 _ac3_atsc2dvb;        // Modify AC-3 signaling from ATSC to DVB method
        bool                 _eac3_atsc2dvb;       // Modify Enhanced-AC-3 signaling from ATSC to DVB method
        bool                 _cleanup_priv_desc;   // Remove private desc without preceding PDS desc
        DescriptorList       _add_descs;           // List of descriptors to add at program level
        DescriptorListByPID  _add_pid_descs;       // Lists of descriptors to add by PID
        AudioLanguageOptionsVector _languages;     // Audio languages to set

        // Implementation of AbstractTablePlugin.
        virtual void createNewTable(BinaryTable& table) override;
        virtual void modifyTable(BinaryTable& table, bool& is_target, bool& reinsert) override;

        // Add a descriptor for a given PID in _add_pid_descs.
        void addComponentDescriptor(PID pid, const AbstractDescriptor& desc);

        // Decode an option "pid/value[/hexa]". Hexa is allowed only if hexa is non zero.
        template<typename INT>
        bool decodeOptionForPID(const UChar* parameter_name, size_t parameter_index, PID& pid, INT& value, ByteBlock* hexa = nullptr);

        // Decode options like --set-stream-identifier which add a simple descriptor in a component.
        template<typename DESCRIPTOR, typename INT>
        bool decodeComponentDescOption(const UChar* parameter_name);

        // Inaccessible operations
        PMTPlugin() = delete;
        PMTPlugin(const PMTPlugin&) = delete;
        PMTPlugin& operator=(const PMTPlugin&) = delete;
    };
}

TSPLUGIN_DECLARE_VERSION
TSPLUGIN_DECLARE_PROCESSOR(pmt, ts::PMTPlugin)


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

ts::PMTPlugin::PMTPlugin(TSP* tsp_) :
    AbstractTablePlugin(tsp_, u"Perform various transformations on the PMT", u"[options]", u"PMT"),
    _service(nullptr, *tsp_),
    _removed_pid(),
    _removed_desc(),
    _removed_stream(),
    _added_pid(),
    _moved_pid(),
    _set_servid(false),
    _new_servid(0),
    _set_pcrpid(false),
    _new_pcrpid(PID_NULL),
    _pds(0),
    _add_stream_id(false),
    _ac3_atsc2dvb(false),
    _eac3_atsc2dvb(false),
    _cleanup_priv_desc(false),
    _add_descs(nullptr),
    _add_pid_descs(),
    _languages()
{
    option(u"ac3-atsc2dvb");
    help(u"ac3-atsc2dvb",
         u"Change the description of AC-3 audio streams from ATSC to DVB method. "
         u"In details, this means that all components with stream_type 0x81 are "
         u"modified with stream_type 0x06 (PES private data) and an AC-3_descriptor "
         u"is added on this component (if none was already there).");

    option(u"add-ca-descriptor", 0, STRING, 0, UNLIMITED_COUNT);
    help(u"add-ca-descriptor", u"casid/pid[/private-data]",
         u"Add a CA_descriptor at program level in the PMT with the specified CA "
         u"System Id and ECM PID. The optional private data must be a suite of "
         u"hexadecimal digits. Several --add-ca-descriptor options may be specified "
         u"to add several descriptors.");

    option(u"add-pid", 'a', STRING, 0, UNLIMITED_COUNT);
    help(u"add-pid", u"pid/stream_type",
         u"Add the specified PID / stream-type component in the PMT. Several "
         u"--add-pid options may be specified to add several components.");

    option(u"add-programinfo-id", 0, UINT32);
    help(u"add-programinfo-id",
         u"Add a registration_descriptor in the program-level descriptor list in the "
         u"PMT. The value is the format_identifier in registration_descriptor, e.g. "
         u"0x43554549 for CUEI.");

    option(u"add-stream-identifier");
    help(u"add-stream-identifier",
         u"Add a stream_identifier_descriptor on all components. The component_tag "
         u"are uniquely allocated inside the service. Existing stream_identifier "
         u"descriptors are left unmodified.");

    option(u"audio-language", 0, STRING, 0, UNLIMITED_COUNT);
    help(u"audio-language", AudioLanguageOptions::GetSyntaxString(),
         u"Specifies the language for an audio stream in the PMT. Several options "
         u"can be specified to set the languages of several audio streams.\n\n" +
         AudioLanguageOptions::GetHelpString());

    option(u"cleanup-private-descriptors", 0);
    help(u"cleanup-private-descriptors",
         u"Remove all private descriptors without preceding private_data_specifier descriptor.");

    option(u"eac3-atsc2dvb");
    help(u"eac3-atsc2dvb",
         u"Change the description of Enhanced-AC-3 (aka AC-3+ or DD+) audio streams "
         u"from ATSC to DVB method. In details, this means that all components with "
         u"stream_type 0x87 are modified with stream_type 0x06 (PES private data) "
         u"and an enhanced_AC-3_descriptor is added on this component (if none was "
         u"already there).");

    option(u"new-service-id", 'i', UINT16);
    help(u"new-service-id",
         u"Change the service id in the PMT.");

    option(u"move-pid", 'm', STRING, 0, UNLIMITED_COUNT);
    help(u"move-pid", u"old-pid/new-pid",
         u"Change the PID value of a component in the PMT. Several --move-pid "
         u"options may be specified to move several components.");

    option(u"pds", 0, UINT32);
    help(u"pds",
         u"With option --remove-descriptor, specify the private data specifier "
         u"which applies to the descriptor tag values above 0x80.");

    option(u"pmt-pid", 'p', PIDVAL);
    help(u"pmt-pid",
         u"Specify the PID carrying the PMT to modify. All PMT's in this PID will be "
         u"modified. Options --pmt-pid and --service are mutually exclusive. If "
         u"neither are specified, the first service in the PAT is used.");

    option(u"pcr-pid", 0, PIDVAL);
    help(u"pcr-pid",
         u"Change the PCR PID value in the PMT.");

    option(u"remove-descriptor", 0, UINT8, 0, UNLIMITED_COUNT);
    help(u"remove-descriptor",
         u"Remove from the PMT all descriptors with the specified tag. Several "
         u"--remove-descriptor options may be specified to remove several types of "
         u"descriptors. See also option --pds.");

    option(u"remove-pid", 'r', PIDVAL, 0, UNLIMITED_COUNT);
    help(u"remove-pid",
         u"Remove the component with the specified PID from the PMT. Several "
         u"--remove-pid options may be specified to remove several components.");

    option(u"remove-stream-type", 0, STRING, 0, UNLIMITED_COUNT);
    help(u"remove-stream-type", u"value[-value]",
         u"Remove all components with a stream type matching the specified value (or in the specified range of values). "
         u"Several --remove-stream-type options may be specified.");

    option(u"service", 's', STRING);
    help(u"service", u"name-or-id",
         u"Specify the service the PMT of which must be modified. If the argument is "
         u"an integer value (either decimal or hexadecimal), it is interpreted as a "
         u"service id. Otherwise, it is interpreted as a service name, as specified "
         u"in the SDT. The name is not case sensitive and blanks are ignored. "
         u"Options --pmt-pid and --service are mutually exclusive. If neither are "
         u"specified, the first service in the PAT is used.");

    option(u"set-cue-type", 0, STRING, 0, UNLIMITED_COUNT);
    help(u"set-cue-type", u"pid/type",
         u"In the component with the specified PID, add an SCTE 35 cue_identifier "
         u"descriptor with the specified cue stream type. Several --set-cue-type "
         u"options may be specified.");

    option(u"set-data-broadcast-id", 0, STRING, 0, UNLIMITED_COUNT);
    help(u"set-data-broadcast-id", u"pid/id[/selector]",
         u"In the component with the specified PID, add a data_broadcast_id_descriptor "
         u"with the specified data_broadcast_id. The optional selector is a suite of "
         u"hexadecimal characters representing the content of the selector bytes. "
         u"Several --set-data-broadcast-id options may be specified.");

    option(u"set-stream-identifier", 0, STRING, 0, UNLIMITED_COUNT);
    help(u"set-stream-identifier", u"pid/id",
         u"In the component with the specified PID, add a stream_identifier_descriptor "
         u"with the specified id. Several --set-stream-identifier options may be "
         u"specified.");
}


//----------------------------------------------------------------------------
// Add a descriptor for a given PID in _add_pid_descs.
//----------------------------------------------------------------------------

void ts::PMTPlugin::addComponentDescriptor(PID pid, const AbstractDescriptor& desc)
{
    // Get or create descriptor list for the component.
    if (_add_pid_descs[pid].isNull()) {
        _add_pid_descs[pid] = new DescriptorList(nullptr);
    }

    // Add the new descriptor.
    _add_pid_descs[pid]->add(desc);
}


//----------------------------------------------------------------------------
// Decode an option "pid/param[/hexa]".
//----------------------------------------------------------------------------

template<typename INT>
bool ts::PMTPlugin::decodeOptionForPID(const UChar* parameter_name, size_t parameter_index, PID& pid, INT& param, ByteBlock* hexa)
{
    // Get the parameter string value.
    const UString str(value(parameter_name, u"", parameter_index));

    // Get slash-separated fields.
    UStringVector fields;
    str.split(fields, u'/');

    // Check number of fields.
    const size_t count = fields.size();
    bool ok = (hexa == nullptr && count == 2) || (hexa != nullptr && (count == 2 || count == 3));

    // Get first two parameters.
    if (ok) {
        uint64_t v1 = 0, v2 = 0;
        ok = fields[0].toInteger(v1, u",") &&
             fields[1].toInteger(v2, u",") &&
             v1 < uint64_t(PID_MAX) &&
             v2 <= uint64_t(std::numeric_limits<INT>::max());
        if (ok) {
            pid = PID(v1);
            param = INT(v2);
        }
    }

    // Get third parameter.
    if (ok && hexa != nullptr) {
        if (count < 3) {
            hexa->clear();
        }
        else {
            ok = fields[2].hexaDecode(*hexa);
        }
    }

    // Process errors.
    if (!ok) {
        error(u"invalid value \"%s\" for --%s", {str, parameter_name});
    }
    return ok;
}


//----------------------------------------------------------------------------
// Decode options like --set-stream-identifier which add a simple descriptor in a component.
//----------------------------------------------------------------------------

template<typename DESCRIPTOR, typename INT>
bool ts::PMTPlugin::decodeComponentDescOption(const UChar* parameter_name)
{
    // Loop on all option values.
    const size_t opt_count = count(parameter_name);
    for (size_t n = 0; n < opt_count; n++) {
        PID pid = PID_NULL;
        INT param = 0;
        if (decodeOptionForPID(parameter_name, n, pid, param)) {
            // Add a new descriptor of the requested type.
            addComponentDescriptor(pid, DESCRIPTOR(param));
        }
        else {
            return false;
        }
    }
    return true;
}


//----------------------------------------------------------------------------
// Start method
//----------------------------------------------------------------------------

bool ts::PMTPlugin::start()
{
    _service.clear();
    _added_pid.clear();
    _moved_pid.clear();
    _add_descs.clear();
    _add_pid_descs.clear();

    // Get option values
    _set_servid = present(u"new-service-id");
    _new_servid = intValue<uint16_t>(u"new-service-id");
    _set_pcrpid = present(u"pcr-pid");
    _new_pcrpid = intValue<PID>(u"pcr-pid");
    _pds = intValue<PDS>(u"pds");
    _ac3_atsc2dvb = present(u"ac3-atsc2dvb");
    _eac3_atsc2dvb = present(u"eac3-atsc2dvb");
    _add_stream_id = present(u"add-stream-identifier");
    _cleanup_priv_desc = present(u"cleanup-private-descriptors");
    getIntValues(_removed_pid, u"remove-pid");
    getIntValues(_removed_desc, u"remove-descriptor");

    // Get list of components to add
    size_t opt_count = count(u"add-pid");
    for (size_t n = 0; n < opt_count; n++) {
        PID pid = PID_NULL;
        uint8_t stype = 0;
        if (decodeOptionForPID(u"add-pid", n, pid, stype)) {
            _added_pid.push_back(NewPID(pid, stype));
        }
        else {
            return false;
        }
    }

    // Get list of stream types to remove.
    opt_count = count(u"remove-stream-type");
    for (size_t n = 0; n < opt_count; n++) {
        const UString opt(value(u"remove-stream-type", u"", n));
        uint8_t s1 = 0, s2 = 0;
        if (opt.scan(u"%d", {&s1})) {
            _removed_stream.push_back(s1);
        }
        else if (opt.scan(u"%d-%d", {&s1, &s2}) && s1 <= s2) {
            for (uint8_t s = s1; s <= s2; ++s) {
                _removed_stream.push_back(s);
                if (s == 0xFF) {
                    break; // avoid overflow and infinite loop when s2 == 0xFF
                }
            }
        }
        else {
            tsp->error(u"invalid integer or integer range \"%s\" for --remove-stream-type", {opt});
            return false;
        }
    }

    // Get suboptions for component to add, type of identifier and tag
    if (!decodeComponentDescOption<StreamIdentifierDescriptor, uint8_t>(u"set-stream-identifier") ||
        !decodeComponentDescOption<CueIdentifierDescriptor, uint8_t>(u"set-cue-type"))
    {
        return false;
    }

    // Get list of data_broadcast_id_descriptors to add
    opt_count = count(u"set-data-broadcast-id");
    for (size_t n = 0; n < opt_count; n++) {
        PID pid = PID_NULL;
        DataBroadcastIdDescriptor desc;
        if (decodeOptionForPID(u"set-data-broadcast-id", n, pid, desc.data_broadcast_id, &desc.private_data)) {
            addComponentDescriptor(pid, desc);
        }
        else {
            return false;
        }
    }

    // Get list of components to move
    opt_count = count(u"move-pid");
    for (size_t n = 0; n < opt_count; n++) {
        const UString s(value(u"move-pid", u"", n));
        int opid = 0, npid = 0;
        if (!s.scan(u"%i/%i", {&opid, &npid}) || opid < 0 || opid >= PID_MAX || npid < 0 || npid >= PID_MAX) {
            error(u"invalid \"old-PID/new-PID\" value \"%s\"", {s});
            return false;
        }
        _moved_pid[PID(opid)] = PID(npid);
    }

    // Get audio languages to set.
    if (!_languages.getFromArgs(*this, u"audio-language")) {
        return false;
    }

    // Get list of descriptors to add
    UStringVector cadescs;
    getValues(cadescs, u"add-ca-descriptor");
    if (!CADescriptor::AddFromCommandLine(_add_descs, cadescs, *tsp)) {
        return false;
    }
    if (present(u"add-programinfo-id")) {
        _add_descs.add(RegistrationDescriptor(intValue<uint32_t>(u"add-programinfo-id")));
    }

    // Get PMT PID or service description
    if (present(u"pmt-pid") && present(u"service")) {
        error(u"options --pmt-pid and --service are mutually exclusive");
        return false;
    }
    if (present(u"pmt-pid")) {
        // A PMT PID is specified, we are now ready to modify all PMT's in this PID
        _service.setPMTPID(intValue<PID>(u"pmt-pid"));
    }
    else {
        _service.set(value(u"service"));
    }

    // Start superclass.
    return AbstractTablePlugin::start();
}


//----------------------------------------------------------------------------
// Invoked by the superclass to create an empty table.
//----------------------------------------------------------------------------

void ts::PMTPlugin::createNewTable(BinaryTable& table)
{
    PMT pmt;

    // If we know the expected service id, this is the one we need to create.
    if (_service.hasId()) {
        pmt.service_id = _service.getId();
    }

    pmt.serialize(table);
}


//----------------------------------------------------------------------------
// Invoked by the superclass when a table is found in the target PID.
//----------------------------------------------------------------------------

void ts::PMTPlugin::modifyTable(BinaryTable& table, bool& is_target, bool& reinsert)
{
    // If not the PMT we are looking for, reinsert without modification.
    is_target = table.tableId() == TID_PMT && (!_service.hasId() || table.tableIdExtension() == _service.getId());
    if (!is_target) {
        return;
    }

    // Process the PMT.
    PMT pmt(table);
    if (!pmt.isValid()) {
        tsp->warning(u"found invalid PMT");
        reinsert = false;
        return;
    }

    // ---- Global non-descriptor data

    // Modify service id
    if (_set_servid) {
        pmt.service_id = _new_servid;
    }

    // Modify PCR PID
    if (_set_pcrpid) {
        pmt.pcr_pid = _new_pcrpid;
    }

    // ---- Do removal first (otherwise it could remove things we add...)

    // Remove components by PID.
    for (auto it = _removed_pid.begin(); it != _removed_pid.end(); ++it) {
        pmt.streams.erase(*it);
    }

    // Remove components by stream type.
    for (auto it = _removed_stream.begin(); it != _removed_stream.end(); ++it) {
        for (auto str = pmt.streams.begin(); str != pmt.streams.end(); ) {
            if (str->second.stream_type == *it) {
                str = pmt.streams.erase(str);
            }
            else {
                ++str;
            }
        }
    }

    // Remove descriptors
    for (auto it = _removed_desc.begin(); it != _removed_desc.end(); ++it) {
        pmt.descs.removeByTag(*it, _pds);
        for (PMT::StreamMap::iterator smi = pmt.streams.begin(); smi != pmt.streams.end(); ++smi) {
            smi->second.descs.removeByTag(*it, _pds);
        }
    }

    // Remove private descriptors without preceding PDS descriptor
    if (_cleanup_priv_desc) {
        pmt.descs.removeInvalidPrivateDescriptors();
        for (PMT::StreamMap::iterator smi = pmt.streams.begin(); smi != pmt.streams.end(); ++smi) {
            smi->second.descs.removeInvalidPrivateDescriptors();
        }
    }

    // ---- Add components and descriptors

    // Add new components
    for (std::list<NewPID>::const_iterator it = _added_pid.begin(); it != _added_pid.end(); ++it) {
        PMT::Stream& ps(pmt.streams[it->pid]);
        ps.stream_type = it->type;
    }

    // Add new descriptors at program level.
    pmt.descs.add(_add_descs);

    // Add descriptors on components.
    for (auto it = _add_pid_descs.begin(); it != _add_pid_descs.end(); ++it) {
        const PID pid = it->first;
        const DescriptorList& dlist(*it->second);

        auto comp_it = pmt.streams.find(pid);
        if (comp_it == pmt.streams.end()) {
            tsp->warning(u"PID 0x%X (%d) not found in PMT", {pid, pid});
        }
        else {
            comp_it->second.descs.add(dlist);
        }
    }

    // Modify audio languages
    _languages.apply(pmt, *tsp);

    // Modify AC-3 signaling from ATSC to DVB method
    if (_ac3_atsc2dvb) {
        for (PMT::StreamMap::iterator smi = pmt.streams.begin(); smi != pmt.streams.end(); ++smi) {
            if (smi->second.stream_type == ST_AC3_AUDIO) {
                smi->second.stream_type = ST_PES_PRIV;
                if (smi->second.descs.search(DID_AC3) == smi->second.descs.count()) {
                    // No AC-3_descriptor present in this component, add one.
                    smi->second.descs.add(AC3Descriptor());
                }
            }
        }
    }

    // Modify Enhanced-AC-3 signaling from ATSC to DVB method
    if (_eac3_atsc2dvb) {
        for (PMT::StreamMap::iterator smi = pmt.streams.begin(); smi != pmt.streams.end(); ++smi) {
            if (smi->second.stream_type == ST_EAC3_AUDIO) {
                smi->second.stream_type = ST_PES_PRIV;
                if (smi->second.descs.search (DID_ENHANCED_AC3) == smi->second.descs.count()) {
                    // No enhanced_AC-3_descriptor present in this component, add one.
                    smi->second.descs.add(EnhancedAC3Descriptor());
                }
            }
        }
    }

    // Add stream_identifier_descriptor on all components.
    // Do this late to avoid clashing with descriptors we added.
    if (_add_stream_id) {

        // First, look for existing descriptors, collect component tags.
        std::bitset<256> ctags;
        for (PMT::StreamMap::iterator smi = pmt.streams.begin(); smi != pmt.streams.end(); ++smi) {
            const DescriptorList& dlist(smi->second.descs);
            for (size_t i = dlist.search(DID_STREAM_ID); i < dlist.count(); i = dlist.search(DID_STREAM_ID, i + 1)) {
                const StreamIdentifierDescriptor sid(*dlist[i]);
                if (sid.isValid()) {
                    ctags.set(sid.component_tag);
                }
            }
        }

        // Then, add a stream_identifier_descriptor on all components which do not have one.
        for (PMT::StreamMap::iterator smi = pmt.streams.begin(); smi != pmt.streams.end(); ++smi) {
            DescriptorList& dlist(smi->second.descs);
            // Skip components already containing a stream_identifier_descriptor
            if (dlist.search(DID_STREAM_ID) < dlist.count()) {
                continue;
            }
            // Allocate a new component tag
            StreamIdentifierDescriptor sid;
            for (size_t i = 0; i < ctags.size(); i++) {
                if (!ctags.test(i)) {
                    sid.component_tag = uint8_t(i);
                    ctags.set(i);
                    break;
                }
            }
            // Add the stream_identifier_descriptor in the component
            dlist.add(sid);
        }
    }

    // ---- Finally, do PID remapping

    for (std::map<PID, PID>::const_iterator it = _moved_pid.begin(); it != _moved_pid.end(); ++it) {
        // Check if component exists
        if (it->first != it->second && pmt.streams.find(it->first) != pmt.streams.end()) {
            pmt.streams[it->second] = pmt.streams[it->first];
            pmt.streams.erase(it->first);
        }
    }

    // Reserialize modified PMT.
    pmt.serialize(table);
}


//----------------------------------------------------------------------------
// Packet processing method
//----------------------------------------------------------------------------

ts::ProcessorPlugin::Status ts::PMTPlugin::processPacket(TSPacket& pkt, bool& flush, bool& bitrate_changed)
{
    // As long as the PMT PID is unknown, pass packets to the service discovery.
    if (!_service.hasPMTPID()) {
        _service.feedPacket(pkt);
    }

    // Abort when a service was specified and we realize it does not exist.
    if (_service.nonExistentService()) {
        return TSP_END;
    }

    // While we don't know which PID to modify, drop all packets to avoid transmitting partial unmodified table.
    if (!_service.hasPMTPID()) {
        return TSP_DROP;
    }

    // The first time we get the PMT PID, set it in the superclass.
    // In fact, set it all the time but this won't do anything when the PID is already known.
    setPID(_service.getPMTPID());

    // Finally, let the superclass do the job.
    return AbstractTablePlugin::processPacket(pkt, flush, bitrate_changed);
}
