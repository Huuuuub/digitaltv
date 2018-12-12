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

#include "tsAssociationTagDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"association_tag_descriptor"
#define MY_DID ts::DID_ASSOCIATION_TAG

TS_XML_DESCRIPTOR_FACTORY(ts::AssociationTagDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::AssociationTagDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::AssociationTagDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::AssociationTagDescriptor::AssociationTagDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    association_tag(0),
    use(0),
    selector_bytes(),
    private_data()
{
    _is_valid = true;
}

ts::AssociationTagDescriptor::AssociationTagDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    AssociationTagDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::AssociationTagDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt16(association_tag);
    bbp->appendUInt16(use);
    bbp->appendUInt8(uint8_t(selector_bytes.size()));
    bbp->append(selector_bytes);
    bbp->append(private_data);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::AssociationTagDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    selector_bytes.clear();
    private_data.clear();

    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size >= 5;

    if (_is_valid) {
        association_tag = GetUInt16(data);
        use = GetUInt16(data + 2);
        const size_t len = GetUInt8(data + 4);
        data += 5; size -= 5;
        if (len > size) {
            _is_valid = false;
        }
        else {
            selector_bytes.copy(data, len);
            private_data.copy(data + len, size - len);
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::AssociationTagDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 5) {
        const uint16_t tag = GetUInt16(data);
        const uint16_t use = GetUInt16(data + 2);
        const size_t len = std::min<size_t>(size - 5, GetUInt8(data + 4));
        data += 5; size -= 5;

        strm << margin << UString::Format(u"Association tag: 0x%X (%d), use: 0x%X (%d)", {tag, tag, use, use}) << std::endl;
        if (len > 0) {
            strm << margin << "Selector bytes:" << std::endl
                 << UString::Dump(data, len, UString::HEXA | UString::ASCII | UString::OFFSET, indent);
        }
        if (size > len) {
            strm << margin << "Private data:" << std::endl
                 << UString::Dump(data + len, size - len, UString::HEXA | UString::ASCII | UString::OFFSET, indent);
        }
        size = 0;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::AssociationTagDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"association_tag", association_tag, true);
    root->setIntAttribute(u"use", use, true);
    if (!selector_bytes.empty()) {
        root->addElement(u"selector_bytes")->addHexaText(selector_bytes);
    }
    if (!private_data.empty()) {
        root->addElement(u"private_data")->addHexaText(private_data);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::AssociationTagDescriptor::fromXML(const xml::Element* element)
{
    selector_bytes.clear();
    private_data.clear();

    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint16_t>(association_tag, u"association_tag", true) &&
        element->getIntAttribute<uint16_t>(use, u"use", true) &&
        element->getHexaTextChild(selector_bytes, u"selector_bytes", false) &&
        element->getHexaTextChild(private_data, u"private_data", false);
}
