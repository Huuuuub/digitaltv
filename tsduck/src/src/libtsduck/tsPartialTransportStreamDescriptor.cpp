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

#include "tsPartialTransportStreamDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"partial_transport_stream_descriptor"
#define MY_DID ts::DID_PARTIAL_TS

TS_XML_DESCRIPTOR_FACTORY(ts::PartialTransportStreamDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::PartialTransportStreamDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::PartialTransportStreamDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::PartialTransportStreamDescriptor::PartialTransportStreamDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    peak_rate(0),
    minimum_overall_smoothing_rate(UNDEFINED_SMOOTHING_RATE),
    maximum_overall_smoothing_buffer(UNDEFINED_SMOOTHING_BUFFER)
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::PartialTransportStreamDescriptor::PartialTransportStreamDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    PartialTransportStreamDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::PartialTransportStreamDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt24(0x00C00000 | peak_rate);
    bbp->appendUInt24(0x00C00000 | minimum_overall_smoothing_rate);
    bbp->appendUInt16(0xC000 | maximum_overall_smoothing_buffer);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::PartialTransportStreamDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size == 8;

    if (_is_valid) {
        peak_rate = GetUInt24(data) & 0x003FFFFF;
        minimum_overall_smoothing_rate = GetUInt24(data + 3) & 0x003FFFFF;
        maximum_overall_smoothing_buffer = GetUInt16(data + 6) & 0x3FFF;
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::PartialTransportStreamDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 8) {
        const uint32_t peak = GetUInt24(data) & 0x003FFFFF;
        const uint32_t min_rate = GetUInt24(data + 3) & 0x003FFFFF;
        const uint16_t max_buffer = GetUInt16(data + 6) & 0x3FFF;
        strm << margin << UString::Format(u"Peak rate: 0x%X (%d) x 400 b/s", {peak, peak}) << std::endl
             << margin << "Min smoothing rate: ";
        if (min_rate == UNDEFINED_SMOOTHING_RATE) {
            strm << "undefined";
        }
        else {
            strm << UString::Format(u"0x%X (%d) x 400 b/s", {min_rate, min_rate});
        }
        strm << std::endl << margin << "Max smoothing buffer: ";
        if (max_buffer == UNDEFINED_SMOOTHING_BUFFER) {
            strm << "undefined";
        }
        else {
            strm << UString::Format(u"0x%X (%d) bytes", {max_buffer, max_buffer});
        }
        strm << std::endl;
        data += 8; size -= 8;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::PartialTransportStreamDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"peak_rate", peak_rate, true);
    if (minimum_overall_smoothing_rate != UNDEFINED_SMOOTHING_RATE) {
        root->setIntAttribute(u"minimum_overall_smoothing_rate", minimum_overall_smoothing_rate, true);
    }
    if (maximum_overall_smoothing_buffer != UNDEFINED_SMOOTHING_BUFFER) {
        root->setIntAttribute(u"maximum_overall_smoothing_buffer", maximum_overall_smoothing_buffer, true);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::PartialTransportStreamDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint32_t>(peak_rate, u"peak_rate", true, 0, 0, 0x003FFFFF) &&
        element->getIntAttribute<uint32_t>(minimum_overall_smoothing_rate, u"minimum_overall_smoothing_rate", false, 0x003FFFFF, 0, 0x003FFFFF) &&
        element->getIntAttribute<uint16_t>(maximum_overall_smoothing_buffer, u"maximum_overall_smoothing_buffer", false, 0x3FFF, 0, 0x3FFF);
}
