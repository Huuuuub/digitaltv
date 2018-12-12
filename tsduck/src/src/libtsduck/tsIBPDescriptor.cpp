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

#include "tsIBPDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"IBP_descriptor"
#define MY_DID ts::DID_IBP

TS_XML_DESCRIPTOR_FACTORY(ts::IBPDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::IBPDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::IBPDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::IBPDescriptor::IBPDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    closed_gop(false),
    identical_gop(false),
    max_gop_length(0)
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::IBPDescriptor::IBPDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    IBPDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::IBPDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt16((closed_gop ? 0x8000 : 0x0000) |
                      (identical_gop ? 0x4000 : 0x0000) |
                      (max_gop_length & 0x3FFF));
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::IBPDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size == 2;

    if (_is_valid) {
        closed_gop = (data[0] & 0x80) != 0;
        identical_gop = (data[0] & 0x40) != 0;
        max_gop_length = GetUInt16(data) & 0x3FFF;
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::IBPDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 2) {
        const uint16_t n = GetUInt16(data);
        data += 2; size -= 2;
        strm << margin
             << UString::Format(u"Closed GOP: %s, identical GOP: %s, max GOP length: 0x%X (%'d)",
                                {UString::YesNo((n & 0x8000) != 0),
                                 UString::YesNo((n & 0x4000) != 0),
                                 n & 0x3FFF, n & 0x3FFF})
             << std::endl;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::IBPDescriptor::buildXML(xml::Element* root) const
{
    root->setBoolAttribute(u"closed_gop", closed_gop);
    root->setBoolAttribute(u"identical_gop", identical_gop);
    root->setIntAttribute(u"max_gop_length", max_gop_length, false);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::IBPDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getBoolAttribute(closed_gop, u"closed_gop", true) &&
        element->getBoolAttribute(identical_gop, u"identical_gop", true) &&
        element->getIntAttribute<uint16_t>(max_gop_length, u"max_gop_length", true, 0, 0x0001, 0x3FFF);
}
