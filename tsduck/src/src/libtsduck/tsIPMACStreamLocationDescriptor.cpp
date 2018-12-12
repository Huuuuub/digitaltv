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

#include "tsIPMACStreamLocationDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"IPMAC_stream_location_descriptor"
#define MY_DID ts::DID_INT_STREAM_LOC
#define MY_TID ts::TID_INT

TS_XML_TABSPEC_DESCRIPTOR_FACTORY(ts::IPMACStreamLocationDescriptor, MY_XML_NAME, MY_TID);
TS_ID_DESCRIPTOR_FACTORY(ts::IPMACStreamLocationDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));
TS_ID_DESCRIPTOR_DISPLAY(ts::IPMACStreamLocationDescriptor::DisplayDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::IPMACStreamLocationDescriptor::IPMACStreamLocationDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    network_id(0),
    original_network_id(0),
    transport_stream_id(0),
    service_id(0),
    component_tag(0)
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::IPMACStreamLocationDescriptor::IPMACStreamLocationDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    IPMACStreamLocationDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::IPMACStreamLocationDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt16(network_id);
    bbp->appendUInt16(original_network_id);
    bbp->appendUInt16(transport_stream_id);
    bbp->appendUInt16(service_id);
    bbp->appendUInt8(component_tag);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::IPMACStreamLocationDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size == 9;

    if (_is_valid) {
        network_id = GetUInt16(data);
        original_network_id = GetUInt16(data + 2);
        transport_stream_id = GetUInt16(data + 4);
        service_id = GetUInt16(data + 6);
        component_tag = data[8];
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::IPMACStreamLocationDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 9) {
        const uint16_t net = GetUInt16(data);
        const uint16_t onet = GetUInt16(data + 2);
        const uint16_t ts = GetUInt16(data + 4);
        const uint16_t srv = GetUInt16(data + 6);
        const uint8_t comp = data[8];
        strm << margin << UString::Format(u"Network id: 0x%X (%d)", {net, net}) << std::endl
             << margin << UString::Format(u"Original network id: 0x%X (%d)", {onet, onet}) << std::endl
             << margin << UString::Format(u"Transport stream id: 0x%X (%d)", {ts, ts}) << std::endl
             << margin << UString::Format(u"Service id: 0x%X (%d)", {srv, srv}) << std::endl
             << margin << UString::Format(u"Component tag: 0x%X (%d)", {comp, comp}) << std::endl;
        data += 9; size -= 9;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::IPMACStreamLocationDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"network_id", network_id, true);
    root->setIntAttribute(u"original_network_id", original_network_id, true);
    root->setIntAttribute(u"transport_stream_id", transport_stream_id, true);
    root->setIntAttribute(u"service_id", service_id, true);
    root->setIntAttribute(u"component_tag", component_tag, true);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::IPMACStreamLocationDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute(network_id, u"network_id", true) &&
        element->getIntAttribute(original_network_id, u"original_network_id", true) &&
        element->getIntAttribute(transport_stream_id, u"transport_stream_id", true) &&
        element->getIntAttribute(service_id, u"service_id", true) &&
        element->getIntAttribute(component_tag, u"component_tag", true);
}
