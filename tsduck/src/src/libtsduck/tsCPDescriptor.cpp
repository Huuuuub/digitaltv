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

#include "tsCPDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"CP_descriptor"
#define MY_DID ts::DID_DVB_EXTENSION
#define MY_EDID ts::EDID_CP

TS_XML_DESCRIPTOR_FACTORY(ts::CPDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::CPDescriptor, ts::EDID::ExtensionDVB(MY_EDID));
TS_ID_DESCRIPTOR_DISPLAY(ts::CPDescriptor::DisplayDescriptor, ts::EDID::ExtensionDVB(MY_EDID));


//----------------------------------------------------------------------------
// Constructors.
//----------------------------------------------------------------------------

ts::CPDescriptor::CPDescriptor(uint16_t cp_id_, PID cp_pid_) :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    cp_id(cp_id_),
    cp_pid(cp_pid_),
    private_data()
{
    _is_valid = true;
}

ts::CPDescriptor::CPDescriptor(const Descriptor& bin, const DVBCharset* charset) :
    CPDescriptor()
{
    deserialize(bin, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::CPDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt8(MY_EDID);
    bbp->appendUInt16(cp_id);
    bbp->appendUInt16(0xE000 | (cp_pid & 0x1FFF));
    bbp->append (private_data);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::CPDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();
    _is_valid = desc.isValid() && desc.tag() == _tag && size >= 5 && data[0] == MY_EDID;

    if (_is_valid) {
        cp_id = GetUInt16(data + 1);
        cp_pid = GetUInt16(data + 3) & 0x1FFF;
        private_data.copy(data + 5, size - 5);
    }
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::CPDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"CP_system_id", cp_id, true);
    root->setIntAttribute(u"CP_PID", cp_pid, true);
    if (!private_data.empty()) {
        root->addElement(u"private_data")->addHexaText(private_data);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::CPDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint16_t>(cp_id, u"CP_system_id", true, 0, 0x0000, 0xFFFF) &&
        element->getIntAttribute<PID>(cp_pid, u"CP_PID", true, 0, 0x0000, 0x1FFF) &&
        element->getHexaTextChild(private_data, u"private_data", false, 0, MAX_DESCRIPTOR_SIZE - 4);
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::CPDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    // Important: With extension descriptors, the DisplayDescriptor() function is called
    // with extension payload. Meaning that data points after descriptor_tag_extension.
    // See ts::TablesDisplay::displayDescriptorData()

    if (size >= 4) {
        std::ostream& strm(display.out());
        const std::string margin(indent, ' ');

        uint16_t id = GetUInt16(data);
        uint16_t pid = GetUInt16(data + 2) & 0x1FFF;
        strm << margin << UString::Format(u"CP System Id: %s, CP PID: %d (0x%X)", {DVBNameFromSection(u"CPSystemId", id, names::FIRST), pid, pid}) << std::endl;

        // CA private part.
        if (size > 4) {
            strm << margin << "Private CP data:" << std::endl
                 << UString::Dump(data + 4, size - 4, UString::HEXA | UString::ASCII | UString::OFFSET, indent);
        }
    }
    else {
        display.displayExtraData(data, size, indent);
    }
}
