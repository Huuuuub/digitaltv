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

#include "tsSmoothingBufferDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"smoothing_buffer_descriptor"
#define MY_DID ts::DID_SMOOTH_BUF

TS_XML_DESCRIPTOR_FACTORY(ts::SmoothingBufferDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::SmoothingBufferDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::SmoothingBufferDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::SmoothingBufferDescriptor::SmoothingBufferDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    sb_leak_rate(0),
    sb_size(0)
{
    _is_valid = true;
}

ts::SmoothingBufferDescriptor::SmoothingBufferDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    SmoothingBufferDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::SmoothingBufferDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt24(0x00C00000 | sb_leak_rate);
    bbp->appendUInt24(0x00C00000 | sb_size);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::SmoothingBufferDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size == 6;

    if (_is_valid) {
        sb_leak_rate = GetUInt24(data) & 0x003FFFFF;
        sb_size = GetUInt24(data + 3) & 0x003FFFFF;
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::SmoothingBufferDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 6) {
        const uint32_t rate = GetUInt24(data) & 0x003FFFFF;
        const uint32_t sbsize = GetUInt24(data + 3) & 0x003FFFFF;
        strm << margin << UString::Format(u"Smoothing buffer leak rate: 0x%X (%d) x 400 b/s", {rate, rate}) << std::endl
             << margin << UString::Format(u"Smoothing buffer size: 0x%X (%d) bytes", {sbsize, sbsize}) << std::endl;
        data += 6; size -= 6;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::SmoothingBufferDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"sb_leak_rate", sb_leak_rate, true);
    root->setIntAttribute(u"sb_size", sb_size, true);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::SmoothingBufferDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint32_t>(sb_leak_rate, u"sb_leak_rate", true, 0, 0, 0x003FFFFF) &&
        element->getIntAttribute<uint32_t>(sb_size, u"sb_size", true, 0, 0, 0x003FFFFF);
}
