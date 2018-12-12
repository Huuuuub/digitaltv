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

#include "tsDeferredAssociationTagsDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"deferred_association_tags_descriptor"
#define MY_DID ts::DID_DEFERRED_ASSOC_TAGS

TS_XML_DESCRIPTOR_FACTORY(ts::DeferredAssociationTagsDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::DeferredAssociationTagsDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::DeferredAssociationTagsDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::DeferredAssociationTagsDescriptor::DeferredAssociationTagsDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    association_tags(),
    transport_stream_id(0),
    program_number(0),
    private_data()
{
    _is_valid = true;
}

ts::DeferredAssociationTagsDescriptor::DeferredAssociationTagsDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    DeferredAssociationTagsDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::DeferredAssociationTagsDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt8(uint8_t(association_tags.size() * sizeof(uint16_t)));
    for (auto it = association_tags.begin(); it != association_tags.end(); ++it) {
        bbp->appendUInt16(*it);
    }
    bbp->appendUInt16(transport_stream_id);
    bbp->appendUInt16(program_number);
    bbp->append(private_data);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::DeferredAssociationTagsDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    association_tags.clear();
    private_data.clear();

    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size >= 1;

    if (_is_valid) {
        size_t len = data[0];
        data++; size--;
        _is_valid = len % 2 == 0 && size >= len + 4;
        if (_is_valid) {
            while (len > 0) {
                association_tags.push_back(GetUInt16(data));
                data += 2; size -= 2; len -= 2;
            }
            transport_stream_id = GetUInt16(data);
            program_number = GetUInt16(data + 2);
            private_data.copy(data + 4, size - 4);
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::DeferredAssociationTagsDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 1) {
        size_t len = data[0];
        data++; size--;
        while (size >= 2 && len >= 2) {
            strm << margin << UString::Format(u"Association tag: 0x%X (%d)", {GetUInt16(data), GetUInt16(data)}) << std::endl;
            data += 2; size -= 2; len -= 2;
        }
        if (size >= 4 && len == 0) {
            strm << margin << UString::Format(u"Transport stream id: 0x%X (%d)", {GetUInt16(data), GetUInt16(data)}) << std::endl
                 << margin << UString::Format(u"Program number: 0x%X (%d)", {GetUInt16(data + 2), GetUInt16(data + 2)}) << std::endl;
            if (size > 4) {
                strm << margin << "Private data:" << std::endl
                     << UString::Dump(data + 4, size - 4, UString::HEXA | UString::ASCII | UString::OFFSET, indent);
            }
            size = 0;
        }
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::DeferredAssociationTagsDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"transport_stream_id", transport_stream_id, true);
    root->setIntAttribute(u"program_number", program_number, true);
    for (auto it = association_tags.begin(); it != association_tags.end(); ++it) {
        root->addElement(u"association")->setIntAttribute(u"tag", *it, true);
    }
    if (!private_data.empty()) {
        root->addElement(u"private_data")->addHexaText(private_data);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::DeferredAssociationTagsDescriptor::fromXML(const xml::Element* element)
{
    association_tags.clear();
    private_data.clear();
    xml::ElementVector children;

    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint16_t>(transport_stream_id, u"transport_stream_id", true) &&
        element->getIntAttribute<uint16_t>(program_number, u"program_number", true) &&
        element->getChildren(children, u"association") &&
        element->getHexaTextChild(private_data, u"private_data", false);

    for (size_t i = 0; _is_valid && i < children.size(); ++i) {
        uint16_t tag = 0;
        _is_valid = children[i]->getIntAttribute<uint16_t>(tag, u"tag", true);
        if (_is_valid) {
            association_tags.push_back(tag);
        }
    }
}
