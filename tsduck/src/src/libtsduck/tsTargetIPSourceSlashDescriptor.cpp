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

#include "tsTargetIPSourceSlashDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"target_IP_source_slash_descriptor"
#define MY_DID ts::DID_INT_IP_SRC_SLASH
#define MY_TID ts::TID_INT

TS_XML_TABSPEC_DESCRIPTOR_FACTORY(ts::TargetIPSourceSlashDescriptor, MY_XML_NAME, MY_TID);
TS_ID_DESCRIPTOR_FACTORY(ts::TargetIPSourceSlashDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));
TS_ID_DESCRIPTOR_DISPLAY(ts::TargetIPSourceSlashDescriptor::DisplayDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::TargetIPSourceSlashDescriptor::TargetIPSourceSlashDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    addresses()
{
    _is_valid = true;
}

ts::TargetIPSourceSlashDescriptor::TargetIPSourceSlashDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    TargetIPSourceSlashDescriptor()
{
    deserialize(desc, charset);
}

ts::TargetIPSourceSlashDescriptor::Address::Address(const IPAddress& addr1, uint8_t mask1, const IPAddress& addr2, uint8_t mask2) :
    IPv4_source_addr(addr1),
    IPv4_source_slash_mask(mask1),
    IPv4_dest_addr(addr2),
    IPv4_dest_slash_mask(mask2)
{
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::TargetIPSourceSlashDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    for (auto it = addresses.begin(); it != addresses.end(); ++it) {
        bbp->appendUInt32(it->IPv4_source_addr.address());
        bbp->appendUInt8(it->IPv4_source_slash_mask);
        bbp->appendUInt32(it->IPv4_dest_addr.address());
        bbp->appendUInt8(it->IPv4_dest_slash_mask);
    }
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::TargetIPSourceSlashDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size % 10 == 0;
    addresses.clear();

    if (_is_valid) {
        while (size >= 10) {
            addresses.push_back(Address(IPAddress(GetUInt32(data)), data[4], IPAddress(GetUInt32(data + 5)), data[9]));
            data += 10; size -= 10;
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::TargetIPSourceSlashDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    while (size >= 10) {
        strm << margin << "- Source:      " << IPAddress(GetUInt32(data)) << "/" << int(data[4]) << std::endl
             << margin << "  Destination: " << IPAddress(GetUInt32(data + 5)) << "/" << int(data[9]) << std::endl;
        data += 10; size -= 10;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::TargetIPSourceSlashDescriptor::buildXML(xml::Element* root) const
{
    for (auto it = addresses.begin(); it != addresses.end(); ++it) {
        xml::Element* e = root->addElement(u"address");
        e->setIPAttribute(u"IPv4_source_addr", it->IPv4_source_addr);
        e->setIntAttribute(u"IPv4_source_slash_mask", it->IPv4_source_slash_mask);
        e->setIPAttribute(u"IPv4_dest_addr", it->IPv4_dest_addr);
        e->setIntAttribute(u"IPv4_dest_slash_mask", it->IPv4_dest_slash_mask);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::TargetIPSourceSlashDescriptor::fromXML(const xml::Element* element)
{
    addresses.clear();

    xml::ElementVector children;
    _is_valid =
        checkXMLName(element) &&
        element->getChildren(children, u"address", 0, MAX_ENTRIES);

    for (size_t i = 0; _is_valid && i < children.size(); ++i) {
        Address addr;
        _is_valid =
                children[i]->getIPAttribute(addr.IPv4_source_addr, u"IPv4_source_addr", true) &&
                children[i]->getIntAttribute(addr.IPv4_source_slash_mask, u"IPv4_source_slash_mask", true) &&
                children[i]->getIPAttribute(addr.IPv4_dest_addr, u"IPv4_dest_addr", true) &&
                children[i]->getIntAttribute(addr.IPv4_dest_slash_mask, u"IPv4_dest_slash_mask", true);
        if (_is_valid) {
            addresses.push_back(addr);
        }
    }
}
