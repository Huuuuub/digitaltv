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

#include "tsMaximumBitrateDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"maximum_bitrate_descriptor"
#define MY_DID ts::DID_MAX_BITRATE

TS_XML_DESCRIPTOR_FACTORY(ts::MaximumBitrateDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::MaximumBitrateDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::MaximumBitrateDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::MaximumBitrateDescriptor::MaximumBitrateDescriptor(uint32_t mbr) :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    maximum_bitrate(mbr)
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::MaximumBitrateDescriptor::MaximumBitrateDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    MaximumBitrateDescriptor(0)
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::MaximumBitrateDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt24(0x00C00000 | (maximum_bitrate & 0x003FFFFF));
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::MaximumBitrateDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    _is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() == 3;

    if (_is_valid) {
        maximum_bitrate = GetUInt24(desc.payload()) & 0x003FFFFF;
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::MaximumBitrateDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 3) {
        const uint32_t mbr = GetUInt24(data) & 0x003FFFFF;
        data += 3; size -= 3;
        strm << margin << UString::Format(u"Maximum bitrate: 0x%X (%d), %'d bits/second", {mbr, mbr, mbr * BITRATE_UNIT}) << std::endl;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::MaximumBitrateDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"maximum_bitrate", maximum_bitrate * BITRATE_UNIT, false);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::MaximumBitrateDescriptor::fromXML(const xml::Element* element)
{
    uint32_t mbr = 0;

    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint32_t>(mbr, u"maximum_bitrate", true, 0, 0, 0x003FFFFF * BITRATE_UNIT);

    if (_is_valid) {
        maximum_bitrate = mbr / BITRATE_UNIT;
    }
}
