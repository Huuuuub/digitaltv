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

#include "tsCPIdentifierDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"CP_identifier_descriptor"
#define MY_DID ts::DID_DVB_EXTENSION
#define MY_EDID ts::EDID_CP_IDENTIFIER

TS_XML_DESCRIPTOR_FACTORY(ts::CPIdentifierDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::CPIdentifierDescriptor, ts::EDID::ExtensionDVB(MY_EDID));
TS_ID_DESCRIPTOR_DISPLAY(ts::CPIdentifierDescriptor::DisplayDescriptor, ts::EDID::ExtensionDVB(MY_EDID));


//----------------------------------------------------------------------------
// Constructors.
//----------------------------------------------------------------------------

ts::CPIdentifierDescriptor::CPIdentifierDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    cpids()
{
    _is_valid = true;
}

ts::CPIdentifierDescriptor::CPIdentifierDescriptor(const Descriptor& bin, const DVBCharset* charset) :
    CPIdentifierDescriptor()
{
    deserialize(bin, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::CPIdentifierDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt8(MY_EDID);
    for (size_t n = 0; n < cpids.size(); ++n) {
        bbp->appendUInt16(cpids[n]);
    }
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::CPIdentifierDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();
    _is_valid = desc.isValid() && desc.tag() == _tag && size >= 1 && size % 2 == 1 && data[0] == MY_EDID;

    cpids.clear();
    if (_is_valid) {
        while (size >= 3) {
            cpids.push_back(GetUInt16(data + 1));
            data += 2;
            size -= 2;
        }
    }
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::CPIdentifierDescriptor::buildXML(xml::Element* root) const
{
    for (size_t i = 0; i < cpids.size(); ++i) {
        root->addElement(u"CP_system_id")->setIntAttribute(u"value", cpids[i], true);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::CPIdentifierDescriptor::fromXML(const xml::Element* element)
{
    cpids.clear();
    xml::ElementVector children;
    _is_valid =
        checkXMLName(element) &&
        element->getChildren(children, u"CP_system_id", 0, (MAX_DESCRIPTOR_SIZE - 3) / 2);
    for (size_t i = 0; _is_valid && i < children.size(); ++i) {
        uint16_t id = 0;
        _is_valid = children[i]->getIntAttribute<uint16_t>(id, u"value", true);
        if (_is_valid) {
            cpids.push_back(id);
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::CPIdentifierDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    // Important: With extension descriptors, the DisplayDescriptor() function is called
    // with extension payload. Meaning that data points after descriptor_tag_extension.
    // See ts::TablesDisplay::displayDescriptorData()

    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    while (size >= 2) {
        const uint16_t id = GetUInt16(data);
        data += 2; size -= 2;
        strm << margin << "CP System Id: " << DVBNameFromSection(u"CPSystemId", id, names::FIRST) << std::endl;
    }

    display.displayExtraData(data, size, indent);
}
