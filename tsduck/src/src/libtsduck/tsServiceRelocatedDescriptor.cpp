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

#include "tsServiceRelocatedDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"service_relocated_descriptor"
#define MY_DID ts::DID_DVB_EXTENSION
#define MY_EDID ts::EDID_SERVICE_RELOCATED

TS_XML_DESCRIPTOR_FACTORY(ts::ServiceRelocatedDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::ServiceRelocatedDescriptor, ts::EDID::ExtensionDVB(MY_EDID));
TS_ID_DESCRIPTOR_DISPLAY(ts::ServiceRelocatedDescriptor::DisplayDescriptor, ts::EDID::ExtensionDVB(MY_EDID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::ServiceRelocatedDescriptor::ServiceRelocatedDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    old_original_network_id(0),
    old_transport_stream_id(0),
    old_service_id(0)
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::ServiceRelocatedDescriptor::ServiceRelocatedDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    ServiceRelocatedDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::ServiceRelocatedDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt8(MY_EDID);
    bbp->appendUInt16(old_original_network_id);
    bbp->appendUInt16(old_transport_stream_id);
    bbp->appendUInt16(old_service_id);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::ServiceRelocatedDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();

    _is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() == 7 && data[0] == MY_EDID;

    if (_is_valid) {
        old_original_network_id = GetUInt16(data + 1);
        old_transport_stream_id = GetUInt16(data + 3);
        old_service_id = GetUInt16(data + 5);
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::ServiceRelocatedDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    // Important: With extension descriptors, the DisplayDescriptor() function is called
    // with extension payload. Meaning that data points after descriptor_tag_extension.
    // See ts::TablesDisplay::displayDescriptorData()

    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 6) {
        strm << margin << UString::Format(u"Old original network id: 0x%X (%d)", {GetUInt16(data), GetUInt16(data)}) << std::endl
             << margin << UString::Format(u"Old transport stream id: 0x%X (%d)", {GetUInt16(data + 2), GetUInt16(data + 2)}) << std::endl
             << margin << UString::Format(u"Old service id: 0x%X (%d)", {GetUInt16(data + 4), GetUInt16(data + 4)}) << std::endl;
        data += 6; size -= 6;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::ServiceRelocatedDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"old_original_network_id", old_original_network_id, true);
    root->setIntAttribute(u"old_transport_stream_id", old_transport_stream_id, true);
    root->setIntAttribute(u"old_service_id", old_service_id, true);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::ServiceRelocatedDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint16_t>(old_original_network_id, u"old_original_network_id", true) &&
        element->getIntAttribute<uint16_t>(old_transport_stream_id, u"old_transport_stream_id", true) &&
        element->getIntAttribute<uint16_t>(old_service_id, u"old_service_id", true);
}
