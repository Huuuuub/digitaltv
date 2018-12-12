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
//  Cyclic packetization of MPEG sections into Transport Stream packets.
//
//----------------------------------------------------------------------------

#include "tsCyclingPacketizer.h"
#include "tsNames.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

ts::CyclingPacketizer::CyclingPacketizer(PID pid, StuffingPolicy stuffing, BitRate bitrate) :
    Packetizer(pid, this),
    _stuffing(stuffing),
    _bitrate(bitrate),
    _section_count(0),
    _sched_sections(),
    _other_sections(),
    _sched_packets(0),
    _current_cycle(1),
    _remain_in_cycle(0),
    _cycle_end(UNDEFINED)
{
}


//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------

ts::CyclingPacketizer::~CyclingPacketizer()
{
}


//----------------------------------------------------------------------------
// Add sections into the packetizer.
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::addSections(const SectionPtrVector& sects, MilliSecond rep_rate)
{
    for (SectionPtrVector::const_iterator it = sects.begin(); it != sects.end(); ++it) {
        addSection(*it, rep_rate);
    }
}


//----------------------------------------------------------------------------
// Add all sections of a table into the packetizer.
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::addTable(const BinaryTable& table, MilliSecond rep_rate)
{
    for (size_t i = 0; i < table.sectionCount(); ++i) {
        addSection(table.sectionAt(i), rep_rate);
    }
}


//----------------------------------------------------------------------------
// Add all sections of a table into the packetizer.
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::addTable(const AbstractTable& table, MilliSecond rep_rate)
{
    BinaryTable bin;
    table.serialize(bin);
    addTable(bin, rep_rate);
}


//----------------------------------------------------------------------------
// Insert a scheduled section in the list, sorted by due_packet, after other
// sections with the same due_packet.
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::addScheduledSection(const SectionDescPtr& sect)
{
    const PacketCounter due_packet = sect->due_packet;
    SectionDescList::iterator it;
    for (it = _sched_sections.begin(); it != _sched_sections.end() && (*it)->due_packet < due_packet; ++it) {}
    _sched_sections.insert(it, sect);
}


//----------------------------------------------------------------------------
// Add a section into the packetizer.
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::addSection(const SectionPtr& sect, MilliSecond rep_rate)
{
    SectionDescPtr desc(new SectionDesc(sect, rep_rate));

    if (rep_rate == 0 || _bitrate == 0) {
        // Unschedule section, simply add it at end of queue
        _other_sections.push_back(desc);
    }
    else {
        // Scheduled section, its due time is "now"
        desc->due_packet = packetCount();
        addScheduledSection(desc);
        _sched_packets += sect->packetCount();
    }

    _section_count++;
    _remain_in_cycle++;
}


//----------------------------------------------------------------------------
// Remove all sections with the specified table id.
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::removeSections(TID tid)
{
    removeSections(_sched_sections, tid, 0, false, true);
    removeSections(_other_sections, tid, 0, false, false);
}


//----------------------------------------------------------------------------
// Remove all sections with the specified table id and table id extension.
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::removeSections(TID tid, uint16_t tid_ext)
{
    removeSections(_sched_sections, tid, tid_ext, true, true);
    removeSections(_other_sections, tid, tid_ext, true, false);
}


//----------------------------------------------------------------------------
// Remove all sections with the specified tid/tid_ext in the specified list.
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::removeSections(SectionDescList& list, TID tid, uint16_t tid_ext, bool use_tid_ext, bool scheduled)
{
    SectionDescList::iterator it(list.begin());
    while (it != list.end()) {
        const SectionDescPtr& sp(*it);
        const Section& sect(*sp->section);
        if (sect.tableId() == tid && (!use_tid_ext || sect.tableIdExtension() == tid_ext)) {
            // Section match, remove it
            assert(_section_count > 0);
            _section_count--;
            if (sp->last_cycle != _current_cycle) {
                assert(_remain_in_cycle > 0);
                _remain_in_cycle--;
            }
            if (scheduled) {
                assert(_sched_packets >= sect.packetCount());
                _sched_packets -= sect.packetCount();
            }
            it = list.erase(it);
        }
        else {
            ++it;
        }
    }
}


//----------------------------------------------------------------------------
// Remove all sections in the packetized.
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::removeAll()
{
    _section_count = 0;
    _remain_in_cycle = 0;
    _sched_packets = 0;
    _sched_sections.clear();
    _other_sections.clear();
}


//----------------------------------------------------------------------------
// Reset the content of a packetizer. Becomes empty.
// If the last returned packet contained an unfinished section,
// this section will be lost. Inherited from Packetizer.
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::reset()
{
    removeAll();
    Packetizer::reset();
}


//----------------------------------------------------------------------------
// Set the bitrate of the generated PID.
// Useful only when using specific repetition rates for sections
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::setBitRate(BitRate new_bitrate)
{
    if (_bitrate == new_bitrate) {
        // Do not do anything if bitrate unchanged.
        return;
    }
    else if (new_bitrate == 0) {
        // Bitrate now unknown, unable to schedule sections, move them all
        // into the list of unscheduled sections.
        while (!_sched_sections.empty()) {
            _other_sections.push_back(_sched_sections.front());
            _sched_sections.pop_front();
        }
        _sched_packets = 0;
    }
    else if (_bitrate == 0) {
        // Bitrate was null but is not now. Move all scheduled sections
        // out of list of unscheduled sections.
        const PacketCounter current_packet(packetCount());
        SectionDescList::iterator it(_other_sections.begin());
        while (it != _other_sections.end()) {
            if ((*it)->repetition == 0) {
                // Not a scheduled section
                ++it;
            }
            else {
                // Scheduled section
                const SectionDescPtr sp(*it);
                it = _other_sections.erase(it);
                if (sp->due_packet < current_packet) {
                    sp->due_packet = current_packet;
                }
                addScheduledSection(sp);
                _sched_packets += sp->section->packetCount();
            }
        }
    }
    else {
        // Old and new bitrate not null. Compute new due packet for all
        // scheduled sections and re-sort list according to new due packet.
        SectionDescList tmp_list;
        tmp_list.swap(_sched_sections);
        while (!tmp_list.empty()) {
            SectionDesc* sp(tmp_list.back().pointer());
            sp->due_packet = sp->last_packet + PacketDistance(new_bitrate, sp->repetition);
            addScheduledSection(tmp_list.back());
            tmp_list.pop_back();
        }
    }

    // Remember new bitrate
    _bitrate = new_bitrate;
}


//----------------------------------------------------------------------------
// This hook is invoked when a new section is required.
// If a null pointer is provided, no section is available.
//----------------------------------------------------------------------------

void ts::CyclingPacketizer::provideSection(SectionCounter counter, SectionPtr& sect)
{
    const PacketCounter current_packet(packetCount());
    SectionDescPtr sp(nullptr);

    // Cycle end is initially undefined.
    // Will be defined only if end of cycle encountered.

    _cycle_end = UNDEFINED;

    // Address the "bitrate overflow" problem: When the minimum bitrate which
    // is required by all scheduled sections is higher than the bitrate of the
    // PID, the unscheduled sections will never pass. To address this, we
    // enforce that unscheduled section are passed from time to time.

    SectionDesc* spp = nullptr;
    bool force_unscheduled =
        // if there are sections in both lists...
        !_other_sections.empty() && !_sched_sections.empty() &&
        // .. and either previous unscheduled sections not passed in current cycle ...
        ((spp = _other_sections.back().pointer())->last_cycle != _current_cycle ||
         // .. or previous unscheduled section passed in this cycle a long time ago
         spp->last_packet + spp->section->packetCount() + _sched_packets < current_packet);

    if (!force_unscheduled && !_sched_sections.empty() && _sched_sections.front()->due_packet <= current_packet) {
        // One scheduled section is ready
        sp = _sched_sections.front();
        _sched_sections.pop_front();
        // Reschedule the section. Make sure we add at least one packet to
        // ensure that all scheduled sections may pass.
        sp->due_packet = current_packet + std::max(PacketCounter(1), PacketDistance(_bitrate, sp->repetition));
        addScheduledSection(sp);
    }
    else if (!_other_sections.empty()) {
        // An unscheduled section is ready
        sp = _other_sections.front();
        _other_sections.pop_front();
        // Move section back at end of queue
        _other_sections.push_back(sp);
    }

    if (sp.isNull()) {
        // No section to provide
        sect.clear();
    }
    else {
        // Provide this section
        sect = sp->section;
        // Remember packet index for this section
        sp->last_packet = current_packet;
        // Remember cycle index for this section
        if (sp->last_cycle != _current_cycle) {
            // First time this section is sent in this cycle
            sp->last_cycle = _current_cycle;
            assert(_remain_in_cycle > 0);
            if (--_remain_in_cycle == 0) {
                // No more section in this cycle, this section is the last one in the cycle
                _cycle_end = counter;
                _current_cycle++;
                _remain_in_cycle = _section_count;
            }
        }
    }
}


//----------------------------------------------------------------------------
// This hook returns true if stuffing to the next transport
// packet boundary shall be performed before the next section.
//----------------------------------------------------------------------------

bool ts::CyclingPacketizer::doStuffing()
{
    return _section_count == 0 ||   // no section => do stuffing
        _stuffing == ALWAYS ||      // always do stuffing
        (_stuffing == AT_END && _remain_in_cycle == _section_count); // At end of cycle
}


//----------------------------------------------------------------------------
// Return true when the last generated packet was the last packet in the cycle.
//----------------------------------------------------------------------------

bool ts::CyclingPacketizer::atCycleBoundary() const
{
    // Coverity false positive:  _cycle_end + 1 overflows only if _cycle_end == UNDEFINED, which is excluded just before.
    // coverity[INTEGER_OVERFLOW]
    return atSectionBoundary() && _cycle_end != UNDEFINED && _cycle_end + 1 == sectionCount();
}


//----------------------------------------------------------------------------
// Display the internal state of the packetizer SectionDesc, mainly for debug
//----------------------------------------------------------------------------

std::ostream& ts::CyclingPacketizer::SectionDesc::display(std::ostream& strm) const
{
    return strm
        << "    - " << names::TID(section->tableId()) << std::endl
        << "      Repetition rate: " << repetition << " ms" << std::endl
        << "      Last provided at cycle: " << last_cycle << std::endl
        << "      Last provided at packet: " << last_packet << std::endl
        << "      Due packet: " << due_packet << std::endl;
}


//----------------------------------------------------------------------------
// Display the internal state of the packetizer, mainly for debug
//----------------------------------------------------------------------------

std::ostream& ts::CyclingPacketizer::display(std::ostream& strm) const
{
    Packetizer::display(strm)
        << "  Stuffing policy: " << int(_stuffing) << std::endl
        << "  Bitrate: " << UString::Decimal(_bitrate) << " b/s" << std::endl
        << "  Current cycle: " << _current_cycle << std::endl
        << "  Remaining sections in cycle: " << _remain_in_cycle << std::endl
        << "  Section cycle end: " << (_cycle_end == UNDEFINED ? u"undefined" : UString::Decimal(_cycle_end)) << std::endl
        << "  Stored sections: " << _section_count << std::endl
        << "  Scheduled sections: " << _sched_sections.size() << std::endl
        << "  Scheduled packets max: " << _sched_packets << std::endl;
    for (SectionDescList::const_iterator it = _sched_sections.begin(); it != _sched_sections.end(); ++it) {
        (*it)->display(strm);
    }
    strm << "  Unscheduled sections: " << _other_sections.size() << std::endl;
    for (SectionDescList::const_iterator it = _other_sections.begin(); it != _other_sections.end(); ++it) {
        (*it)->display(strm);
    }
    return strm;
}
