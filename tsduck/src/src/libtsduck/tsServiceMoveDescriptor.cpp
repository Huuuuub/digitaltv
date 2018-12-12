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

#include "tsServiceMoveDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"service_move_descriptor"
#define MY_DID ts::DID_SERVICE_MOVE

TS_XML_DESCRIPTOR_FACTORY(ts::ServiceMoveDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::ServiceMoveDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::ServiceMoveDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::ServiceMoveDescriptor::ServiceMoveDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    new_original_network_id(0),
    new_transport_stream_id(0),
    new_service_id(0)
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::ServiceMoveDescriptor::ServiceMoveDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    ServiceMoveDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::ServiceMoveDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt16(new_original_network_id);
    bbp->appendUInt16(new_transport_stream_id);
    bbp->appendUInt16(new_service_id);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::ServiceMoveDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    _is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() == 6;

    if (_is_valid) {
        const uint8_t* data = desc.payload();
        new_original_network_id = GetUInt16(data);
        new_transport_stream_id = GetUInt16(data + 2);
        new_service_id = GetUInt16(data + 4);
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::ServiceMoveDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 6) {
        strm << margin << UString::Format(u"New original network id: 0x%X (%d)", {GetUInt16(data), GetUInt16(data)}) << std::endl
             << margin << UString::Format(u"New transport stream id: 0x%X (%d)", {GetUInt16(data + 2), GetUInt16(data + 2)}) << std::endl
             << margin << UString::Format(u"New service id: 0x%X (%d)", {GetUInt16(data + 4), GetUInt16(data + 4)}) << std::endl;
        data += 6; size -= 6;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::ServiceMoveDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"new_original_network_id", new_original_network_id, true);
    root->setIntAttribute(u"new_transport_stream_id", new_transport_stream_id, true);
    root->setIntAttribute(u"new_service_id", new_service_id, true);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::ServiceMoveDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint16_t>(new_original_network_id, u"new_original_network_id", true) &&
        element->getIntAttribute<uint16_t>(new_transport_stream_id, u"new_transport_stream_id", true) &&
        element->getIntAttribute<uint16_t>(new_service_id, u"new_service_id", true);
}
