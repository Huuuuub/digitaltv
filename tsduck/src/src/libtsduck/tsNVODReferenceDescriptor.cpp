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

#include "tsNVODReferenceDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"NVOD_reference_descriptor"
#define MY_DID ts::DID_NVOD_REFERENCE

TS_XML_DESCRIPTOR_FACTORY(ts::NVODReferenceDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::NVODReferenceDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::NVODReferenceDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::NVODReferenceDescriptor::Entry::Entry(uint16_t ts, uint16_t net, uint16_t srv) :
    transport_stream_id(ts),
    original_network_id(net),
    service_id(srv)
{
}

ts::NVODReferenceDescriptor::NVODReferenceDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    entries()
{
    _is_valid = true;
}

ts::NVODReferenceDescriptor::NVODReferenceDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    NVODReferenceDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::NVODReferenceDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    for (EntryList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        bbp->appendUInt16(it->transport_stream_id);
        bbp->appendUInt16(it->original_network_id);
        bbp->appendUInt16(it->service_id);
    }
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::NVODReferenceDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    _is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() % 6 == 0;
    entries.clear();

    if (_is_valid) {
        const uint8_t* data = desc.payload();
        size_t size = desc.payloadSize();
        while (size >= 6) {
            entries.push_back(Entry(GetUInt16(data), GetUInt16(data + 2), GetUInt16(data + 4)));
            data += 6; size -= 6;
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::NVODReferenceDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    while (size >= 6) {
        const uint16_t ts = GetUInt16(data);
        const uint16_t net = GetUInt16(data + 2);
        const uint16_t srv = GetUInt16(data + 4);
        data += 6; size -= 6;
        strm << margin << UString::Format(u"- Transport stream id: 0x%X (%d)", {ts, ts}) << std::endl
             << margin << UString::Format(u"  Original network id: 0x%X (%d)", {net, net}) << std::endl
             << margin << UString::Format(u"  Service id: 0x%X (%d)", {srv, srv}) << std::endl;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::NVODReferenceDescriptor::buildXML(xml::Element* root) const
{
    for (EntryList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        xml::Element* e = root->addElement(u"service");
        e->setIntAttribute(u"transport_stream_id", it->transport_stream_id, true);
        e->setIntAttribute(u"original_network_id", it->original_network_id, true);
        e->setIntAttribute(u"service_id", it->service_id, true);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::NVODReferenceDescriptor::fromXML(const xml::Element* element)
{
    entries.clear();

    xml::ElementVector children;
    _is_valid =
        checkXMLName(element) &&
        element->getChildren(children, u"service", 0, MAX_ENTRIES);

    for (size_t i = 0; _is_valid && i < children.size(); ++i) {
        Entry entry;
        _is_valid =
            children[i]->getIntAttribute<uint16_t>(entry.transport_stream_id, u"transport_stream_id", true) &&
            children[i]->getIntAttribute<uint16_t>(entry.original_network_id, u"original_network_id", true) &&
            children[i]->getIntAttribute<uint16_t>(entry.service_id, u"service_id", true);
        if (_is_valid) {
            entries.push_back(entry);
        }
    }
}
