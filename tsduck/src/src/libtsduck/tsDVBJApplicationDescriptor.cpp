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

#include "tsDVBJApplicationDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"dvb_j_application_descriptor"
#define MY_DID ts::DID_AIT_DVBJ_APP
#define MY_TID ts::TID_AIT

TS_XML_TABSPEC_DESCRIPTOR_FACTORY(ts::DVBJApplicationDescriptor, MY_XML_NAME, MY_TID);
TS_ID_DESCRIPTOR_FACTORY(ts::DVBJApplicationDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));
TS_ID_DESCRIPTOR_DISPLAY(ts::DVBJApplicationDescriptor::DisplayDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::DVBJApplicationDescriptor::DVBJApplicationDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    parameters()
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::DVBJApplicationDescriptor::DVBJApplicationDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    DVBJApplicationDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::DVBJApplicationDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        bbp->append(it->toDVBWithByteLength(0, NPOS, charset));
    }
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::DVBJApplicationDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    parameters.clear();
    _is_valid = desc.isValid() && desc.tag() == _tag;

    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    while (_is_valid && size >= 1) {
        const size_t len = data[0];
        data += 1; size -= 1;
        _is_valid = len <= size;
        if (_is_valid) {
            parameters.push_back(UString::FromDVB(data, len, charset));
            data += len; size -= len;
        }
    }

    _is_valid = _is_valid && size == 0;
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::DVBJApplicationDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    while (size >= 1) {
        const size_t len = std::min<size_t>(data[0], size - 1);
        strm << margin << "Parameter: \"" << UString::FromDVB(data + 1, len, display.dvbCharset()) << "\"" << std::endl;
        data += 1 + len; size -= 1 + len;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::DVBJApplicationDescriptor::buildXML(xml::Element* root) const
{
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        root->addElement(u"parameter")->setAttribute(u"value", *it);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::DVBJApplicationDescriptor::fromXML(const xml::Element* element)
{
    parameters.clear();

    xml::ElementVector children;
    _is_valid =
        checkXMLName(element) &&
        element->getChildren(children, u"parameter");

    for (size_t i = 0; _is_valid && i < children.size(); ++i) {
        UString param;
        _is_valid = children[i]->getAttribute(param, u"value", true);
        if (_is_valid) {
            parameters.push_back(param);
        }
    }
}
