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
//  Representation of a data_broadcast_descriptor
//
//----------------------------------------------------------------------------

#include "tsDataBroadcastDescriptor.h"
#include "tsDataBroadcastIdDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"data_broadcast_descriptor"
#define MY_DID ts::DID_DATA_BROADCAST

TS_XML_DESCRIPTOR_FACTORY(ts::DataBroadcastDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::DataBroadcastDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::DataBroadcastDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Constructors.
//----------------------------------------------------------------------------

ts::DataBroadcastDescriptor::DataBroadcastDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    data_broadcast_id(0),
    component_tag(0),
    selector_bytes(),
    language_code(),
    text()
{
    _is_valid = true;
}

ts::DataBroadcastDescriptor::DataBroadcastDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    data_broadcast_id(0),
    component_tag(0),
    selector_bytes(),
    language_code(),
    text()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::DataBroadcastDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 4) {
        const uint16_t dbid = GetUInt16(data);
        const uint8_t ctag = data[2];
        size_t slength = data[3];
        data += 4; size -= 4;
        if (slength > size) {
            slength = size;
        }
        strm << margin << "Data broadcast id: " << names::DataBroadcastId(dbid, names::BOTH_FIRST) << std::endl
             << margin << UString::Format(u"Component tag: %d (0x%X), ", {ctag, ctag})
             << std::endl;
        DataBroadcastIdDescriptor::DisplaySelectorBytes(display, data, slength, indent, dbid);
        data += slength; size -= slength;
        if (size >= 3) {
            strm << margin << "Language: " << UString::FromDVB(data, 3, display.dvbCharset()) << std::endl;
            data += 3; size -= 3;
            strm << margin << "Description: \"" << UString::FromDVBWithByteLength(data, size, display.dvbCharset()) << "\"" << std::endl;
        }
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::DataBroadcastDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());

    bbp->appendUInt16(data_broadcast_id);
    bbp->appendUInt8(component_tag);
    bbp->appendUInt8(int8_t(selector_bytes.size()));
    bbp->append(selector_bytes);
    if (!SerializeLanguageCode(*bbp, language_code, charset)) {
        desc.invalidate();
        return;
    }
    bbp->append(text.toDVBWithByteLength(0, NPOS, charset));

    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::DataBroadcastDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    selector_bytes.clear();
    language_code.clear();
    text.clear();

    if (!(_is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() >= 8)) {
        return;
    }

    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    data_broadcast_id = GetUInt16(data);
    component_tag = GetUInt8(data + 2);
    size_t length = GetUInt8(data + 3);
    data += 4; size -= 4;

    if (length + 4 > size) {
        _is_valid = false;
        return;
    }
    selector_bytes.copy(data, length);
    data += length; size -= length;

    language_code = UString::FromDVB(data, 3, charset);
    data += 3; size -= 3;
    text = UString::FromDVBWithByteLength(data, size, charset);
    _is_valid = size == 0;
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::DataBroadcastDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"data_broadcast_id", data_broadcast_id, true);
    root->setIntAttribute(u"component_tag", component_tag, true);
    root->setAttribute(u"language_code", language_code);
    if (!selector_bytes.empty()) {
        root->addElement(u"selector_bytes")->addHexaText(selector_bytes);
    }
    root->addElement(u"text")->addText(text);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::DataBroadcastDescriptor::fromXML(const xml::Element* element)
{
    selector_bytes.clear();
    language_code.clear();
    text.clear();

    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint16_t>(data_broadcast_id, u"data_broadcast_id", true) &&
        element->getIntAttribute<uint8_t>(component_tag, u"component_tag", true) &&
        element->getAttribute(language_code, u"language_code", true, u"", 3, 3) &&
        element->getHexaTextChild(selector_bytes, u"selector_bytes", true) &&
        element->getTextChild(text, u"text", true, false);
}
