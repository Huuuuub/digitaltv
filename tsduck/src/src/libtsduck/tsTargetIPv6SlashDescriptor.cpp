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

#include "tsTargetIPv6SlashDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"target_IPv6_slash_descriptor"
#define MY_DID ts::DID_INT_IPV6_SLASH
#define MY_TID ts::TID_INT

TS_XML_TABSPEC_DESCRIPTOR_FACTORY(ts::TargetIPv6SlashDescriptor, MY_XML_NAME, MY_TID);
TS_ID_DESCRIPTOR_FACTORY(ts::TargetIPv6SlashDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));
TS_ID_DESCRIPTOR_DISPLAY(ts::TargetIPv6SlashDescriptor::DisplayDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::TargetIPv6SlashDescriptor::TargetIPv6SlashDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    addresses()
{
    _is_valid = true;
}

ts::TargetIPv6SlashDescriptor::TargetIPv6SlashDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    TargetIPv6SlashDescriptor()
{
    deserialize(desc, charset);
}

ts::TargetIPv6SlashDescriptor::Address::Address(const IPv6Address& addr, uint8_t mask) :
    IPv6_addr(addr),
    IPv6_slash_mask(mask)
{
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::TargetIPv6SlashDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    for (auto it = addresses.begin(); it != addresses.end(); ++it) {
        bbp->append(it->IPv6_addr.toBytes());
        bbp->appendUInt8(it->IPv6_slash_mask);
    }
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::TargetIPv6SlashDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size % 17 == 0;
    addresses.clear();

    if (_is_valid) {
        while (size >= 17) {
            addresses.push_back(Address(IPv6Address(data, 16), data[16]));
            data += 17; size -= 17;
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::TargetIPv6SlashDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    while (size >= 17) {
        strm << margin << "Address/mask: " << IPv6Address(data, 16) << "/" << int(data[16]) << std::endl;
        data += 17; size -= 17;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::TargetIPv6SlashDescriptor::buildXML(xml::Element* root) const
{
    for (auto it = addresses.begin(); it != addresses.end(); ++it) {
        xml::Element* e = root->addElement(u"address");
        e->setIPv6Attribute(u"IPv6_addr", it->IPv6_addr);
        e->setIntAttribute(u"IPv6_slash_mask", it->IPv6_slash_mask);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::TargetIPv6SlashDescriptor::fromXML(const xml::Element* element)
{
    addresses.clear();

    xml::ElementVector children;
    _is_valid =
        checkXMLName(element) &&
        element->getChildren(children, u"address", 0, MAX_ENTRIES);

    for (size_t i = 0; _is_valid && i < children.size(); ++i) {
        Address addr;
        _is_valid =
                children[i]->getIPv6Attribute(addr.IPv6_addr, u"IPv6_addr", true) &&
                children[i]->getIntAttribute(addr.IPv6_slash_mask, u"IPv6_slash_mask", true);
        if (_is_valid) {
            addresses.push_back(addr);
        }
    }
}
