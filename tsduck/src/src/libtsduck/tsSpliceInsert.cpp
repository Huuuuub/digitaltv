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

#include "tsSpliceInsert.h"
#include "tsTablesDisplay.h"
#include "tsNames.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"splice_insert"


//----------------------------------------------------------------------------
// Default constructor.
//----------------------------------------------------------------------------

ts::SpliceInsert::SpliceInsert() :
    AbstractSignalization(MY_XML_NAME),
    event_id(0),
    canceled(true),
    splice_out(false),
    immediate(false),
    program_splice(false),
    use_duration(false),
    program_pts(),
    components_pts(),
    duration_pts(INVALID_PTS),
    auto_return(false),
    program_id(0),
    avail_num(0),
    avails_expected(0)
{
}


//----------------------------------------------------------------------------
// Reset all fields to default initial values.
//----------------------------------------------------------------------------

void ts::SpliceInsert::clear()
{
    event_id = 0;
    canceled = true;
    splice_out = false;
    immediate = false;
    program_splice = false;
    use_duration = false;
    program_pts = INVALID_PTS;
    components_pts.clear();
    duration_pts = INVALID_PTS;
    auto_return = false;
    program_id = 0;
    avail_num = 0;
    avails_expected = 0;
}


//----------------------------------------------------------------------------
// Adjust PTS time values using the "PTS adjustment" field from a splice
// information section.
//----------------------------------------------------------------------------

void ts::SpliceInsert::adjustPTS(uint64_t adjustment)
{
    // Ignore null or invalid adjustment. And cancelation or immediate commands have no time.
    if (adjustment == 0 || adjustment > PTS_DTS_MASK || canceled || immediate) {
        return;
    }

    // Adjust program splice time.
    if (program_splice && program_pts.set() && program_pts.value() <= PTS_DTS_MASK) {
        program_pts = (program_pts.value() + adjustment) & PTS_DTS_MASK;
    }

    // Adjust components splice times.
    if (!program_splice) {
        for (SpliceByComponent::iterator it = components_pts.begin(); it != components_pts.end(); ++it) {
            if (it->second.set() && it->second.value() <= PTS_DTS_MASK) {
                it->second = (it->second.value() + adjustment) & PTS_DTS_MASK;
            }
        }
    }
}


//----------------------------------------------------------------------------
// Get the highest or lowest PTS value in the command.
//----------------------------------------------------------------------------

uint64_t ts::SpliceInsert::highestPTS() const
{
    uint64_t result = INVALID_PTS;
    if (!canceled && !immediate) {
        // Check program splice time.
        if (program_splice && program_pts.set() && program_pts.value() <= PTS_DTS_MASK) {
            result = program_pts.value();
        }
        // Check components splice times.
        if (!program_splice) {
            for (auto it = components_pts.begin(); it != components_pts.end(); ++it) {
                if (it->second.set() && it->second.value() <= PTS_DTS_MASK && (result == INVALID_PTS || it->second.value() > result)) {
                    result = it->second.value();
                }
            }
        }
    }
    return result;
}

uint64_t ts::SpliceInsert::lowestPTS() const
{
    uint64_t result = INVALID_PTS;
    if (!canceled && !immediate) {
        // Check program splice time.
        if (program_splice && program_pts.set() && program_pts.value() <= PTS_DTS_MASK) {
            result = program_pts.value();
        }
        // Check components splice times.
        if (!program_splice) {
            for (auto it = components_pts.begin(); it != components_pts.end(); ++it) {
                if (it->second.set() && it->second.value() <= PTS_DTS_MASK && (result == INVALID_PTS || it->second.value() < result)) {
                    result = it->second.value();
                }
            }
        }
    }
    return result;
}


//----------------------------------------------------------------------------
// Display a SpliceInsert command.
//----------------------------------------------------------------------------

void ts::SpliceInsert::display(TablesDisplay& display, int indent) const
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    strm << margin << UString::Format(u"Splice event id: 0x%X, cancel: %d", {event_id, canceled}) << std::endl;

    if (!canceled) {
        strm << margin
             << "Out of network: " << UString::YesNo(splice_out)
             << ", program splice: " << UString::YesNo(program_splice)
             << ", duration set: " << UString::YesNo(use_duration)
             << ", immediate: " << UString::YesNo(immediate)
             << std::endl;

        if (program_splice && !immediate) {
            // The complete program switches at a given time.
            strm << margin << "Time PTS: " << program_pts.toString() << std::endl;
        }
        if (!program_splice) {
            // Program components switch individually.
            strm << margin << "Number of components: " << components_pts.size() << std::endl;
            for (SpliceByComponent::const_iterator it = components_pts.begin(); it != components_pts.end(); ++it) {
                strm << margin << UString::Format(u"  Component tag: 0x%X (%d)", {it->first, it->first});
                if (!immediate) {
                    strm << ", time PTS: " << it->second.toString();
                }
                strm << std::endl;
            }
        }
        if (use_duration) {
            strm << margin << UString::Format(u"Duration PTS: 0x%09X (%d), auto return: %s", {duration_pts, duration_pts, UString::YesNo(auto_return)}) << std::endl;
        }
        strm << margin << UString::Format(u"Unique program id: 0x%X (%d), avail: 0x%X (%d), avails expected: %d", {program_id, program_id, avail_num, avail_num, avails_expected}) << std::endl;
    }
}


//----------------------------------------------------------------------------
// Deserialize a SpliceInsert command from binary data.
//----------------------------------------------------------------------------

int ts::SpliceInsert::deserialize(const uint8_t* data, size_t size)
{
    const uint8_t* const start = data;
    clear();
    if (size < 5) {
        return -1; // too short
    }

    event_id = GetUInt32(data);
    canceled = (data[4] & 0x80) != 0;
    data += 5; size -= 5;

    if (canceled) {
        _is_valid = true;
        return int(data - start);  // end of command
    }
    if (size < 1) {
        return -1; // too short
    }

    splice_out = (data[0] & 0x80) != 0;
    program_splice = (data[0] & 0x40) != 0;
    use_duration = (data[0] & 0x20) != 0;
    immediate = (data[0] & 0x10) != 0;
    data++; size--;

    if (program_splice && !immediate) {
        // The complete program switches at a given time.
        const int s = program_pts.deserialize(data, size);
        if (s < 0) {
            return -1; // invalid
        }
        data += s; size -= s;
    }
    if (!program_splice) {
        // Program components switch individually.
        if (size < 1) {
            return -1; // too short
        }
        size_t count = data[0];
        data++; size--;
        while (count-- > 0) {
            if (size < 1) {
                return -1; // too short
            }
            const uint8_t ctag = data[0];
            data++; size--;
            SpliceTime pts;
            if (!immediate) {
                const int s = pts.deserialize(data, size);
                if (s < 0) {
                    return -1; // invalid
                }
                data += s; size -= s;
            }
            components_pts.insert(std::make_pair(ctag, pts));
        }
    }
    if (use_duration) {
        if (size < 5) {
            return -1; // too short
        }
        auto_return = (data[0] & 0x80) != 0;
        duration_pts = (uint64_t(data[0] & 0x01) << 32) | uint64_t(GetUInt32(data + 1));
        data += 5; size -= 5;
    }
    if (size < 4) {
        return -1; // too short
    }
    program_id = GetUInt16(data);
    avail_num = data[2];
    avails_expected = data[3];
    data += 4; size -= 4;

    _is_valid = true;
    return int(data - start);
}


//----------------------------------------------------------------------------
// Serialize the SpliceInsert command.
//----------------------------------------------------------------------------

void ts::SpliceInsert::serialize(ByteBlock& data) const
{
    data.appendUInt32(event_id);
    data.appendUInt8(canceled ? 0xFF : 0x7F);

    if (!canceled) {
        data.appendUInt8((splice_out ? 0x80 : 0x00) |
                         (program_splice ? 0x40 : 0x00) |
                         (use_duration ? 0x20 : 0x00) |
                         (immediate ? 0x10 : 0x00) |
                         0x0F);
        if (program_splice && !immediate) {
            program_pts.serialize(data);
        }
        if (!program_splice) {
            data.appendUInt8(uint8_t(components_pts.size()));
            for (SpliceByComponent::const_iterator it = components_pts.begin(); it != components_pts.end(); ++it) {
                data.appendUInt8(it->first);
                if (!immediate) {
                    it->second.serialize(data);
                }
            }
        }
        if (use_duration) {
            data.appendUInt8((auto_return ? 0xFE : 0x7E) | uint8_t(duration_pts >> 32));
            data.appendUInt32(uint32_t(duration_pts));
        }
        data.appendUInt16(program_id);
        data.appendUInt8(avail_num);
        data.appendUInt8(avails_expected);
    }
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::SpliceInsert::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"splice_event_id", event_id, true);
    root->setBoolAttribute(u"splice_event_cancel", canceled);
    if (!canceled) {
        root->setBoolAttribute(u"out_of_network", splice_out);
        root->setBoolAttribute(u"splice_immediate", immediate);
        root->setIntAttribute(u"unique_program_id", program_id, true);
        root->setIntAttribute(u"avail_num", avail_num);
        root->setIntAttribute(u"avails_expected", avails_expected);
        if (program_splice && !immediate && program_pts.set()) {
            root->setIntAttribute(u"pts_time", program_pts.value());
        }
        if (use_duration) {
            xml::Element* e = root->addElement(u"break_duration");
            e->setBoolAttribute(u"auto_return", auto_return);
            e->setIntAttribute(u"duration", duration_pts);
        }
        if (!program_splice) {
            for (auto it = components_pts.begin(); it != components_pts.end(); ++it) {
                xml::Element* e = root->addElement(u"component");
                e->setIntAttribute(u"component_tag", it->first);
                if (!immediate && it->second.set()) {
                    e->setIntAttribute(u"pts_time", it->second.value());
                }
            }
        }
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::SpliceInsert::fromXML(const xml::Element* element)
{
    clear();

    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint32_t>(event_id, u"splice_event_id", true) &&
        element->getBoolAttribute(canceled, u"splice_event_cancel", false, false);

    if (_is_valid && !canceled) {
        xml::ElementVector breakDuration;
        xml::ElementVector components;
        _is_valid =
            element->getBoolAttribute(splice_out, u"out_of_network", true) &&
            element->getBoolAttribute(immediate, u"splice_immediate", false, false) &&
            element->getIntAttribute<uint16_t>(program_id, u"unique_program_id", true) &&
            element->getIntAttribute<uint8_t>(avail_num, u"avail_num", false, 0) &&
            element->getIntAttribute<uint8_t>(avails_expected, u"avails_expected", false, 0) &&
            element->getChildren(breakDuration, u"break_duration", 0, 1) &&
            element->getChildren(components, u"component", 0, 255);
        use_duration = !breakDuration.empty();
        program_splice = element->hasAttribute(u"pts_time") || (immediate && components.empty());
        if (_is_valid && use_duration) {
            assert(breakDuration.size() == 1);
            _is_valid =
                breakDuration[0]->getBoolAttribute(auto_return, u"auto_return", true) &&
                breakDuration[0]->getIntAttribute<uint64_t>(duration_pts, u"duration", true);
        }
        if (_is_valid && program_splice && !immediate) {
            _is_valid = element->getOptionalIntAttribute<uint64_t>(program_pts, u"pts_time", 0, PTS_DTS_MASK);
        }
        if (_is_valid && !program_splice) {
            for (size_t i = 0; _is_valid && i < components.size(); ++i) {
                uint8_t tag = 0;
                SpliceTime pts;
                _is_valid =
                    components[i]->getIntAttribute<uint8_t>(tag, u"component_tag", true) &&
                    components[i]->getOptionalIntAttribute<uint64_t>(pts, u"pts_time", 0, PTS_DTS_MASK);
                if (_is_valid) {
                    components_pts[tag] = pts;
                }
            }
        }
    }
}
