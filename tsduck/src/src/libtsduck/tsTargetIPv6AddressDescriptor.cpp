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

#include "tsTargetIPv6AddressDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"target_IPv6_address_descriptor"
#define MY_DID ts::DID_INT_IPV6_ADDR

TS_XML_TABSPEC_DESCRIPTOR_FACTORY(ts::TargetIPv6AddressDescriptor, MY_XML_NAME, ts::TID_INT, ts::TID_UNT);

TS_ID_DESCRIPTOR_FACTORY(ts::TargetIPv6AddressDescriptor, ts::EDID::TableSpecific(MY_DID, ts::TID_INT));
TS_ID_DESCRIPTOR_FACTORY(ts::TargetIPv6AddressDescriptor, ts::EDID::TableSpecific(MY_DID, ts::TID_UNT));

TS_ID_DESCRIPTOR_DISPLAY(ts::TargetIPv6AddressDescriptor::DisplayDescriptor, ts::EDID::TableSpecific(MY_DID, ts::TID_INT));
TS_ID_DESCRIPTOR_DISPLAY(ts::TargetIPv6AddressDescriptor::DisplayDescriptor, ts::EDID::TableSpecific(MY_DID, ts::TID_UNT));


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::TargetIPv6AddressDescriptor::TargetIPv6AddressDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    IPv6_addr_mask(),
    IPv6_addr()
{
    _is_valid = true;
}

ts::TargetIPv6AddressDescriptor::TargetIPv6AddressDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    TargetIPv6AddressDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::TargetIPv6AddressDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->append(IPv6_addr_mask.toBytes());
    for (auto it = IPv6_addr.begin(); it != IPv6_addr.end(); ++it) {
        bbp->append(it->toBytes());
    }
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::TargetIPv6AddressDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size >= 16 && size % 16 == 0;
    IPv6_addr.clear();

    if (_is_valid) {
        IPv6_addr_mask.setAddress(data, 16);
        data += 16; size -= 16;
        while (size >= 16) {
            IPv6_addr.push_back(IPv6Address(data, 16));
            data += 16; size -= 16;
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::TargetIPv6AddressDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    const char* header = "Address mask: ";
    while (size >= 16) {
        strm << margin << header << IPv6Address(data, 16) << std::endl;
        data += 16; size -= 16;
        header = "Address: ";
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::TargetIPv6AddressDescriptor::buildXML(xml::Element* root) const
{
    root->setIPv6Attribute(u"IPv6_addr_mask", IPv6_addr_mask);
    for (auto it = IPv6_addr.begin(); it != IPv6_addr.end(); ++it) {
        root->addElement(u"address")->setIPv6Attribute(u"IPv6_addr", *it);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::TargetIPv6AddressDescriptor::fromXML(const xml::Element* element)
{
    IPv6_addr.clear();

    xml::ElementVector children;
    _is_valid =
        checkXMLName(element) &&
        element->getIPv6Attribute(IPv6_addr_mask, u"IPv6_addr_mask", true) &&
        element->getChildren(children, u"address", 0, MAX_ENTRIES);

    for (size_t i = 0; _is_valid && i < children.size(); ++i) {
        IPv6Address addr;
        _is_valid = children[i]->getIPv6Attribute(addr, u"IPv6_addr", true);
        if (_is_valid) {
            IPv6_addr.push_back(addr);
        }
    }
}
