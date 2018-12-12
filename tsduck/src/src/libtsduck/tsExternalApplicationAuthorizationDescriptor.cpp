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

#include "tsExternalApplicationAuthorizationDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"external_application_authorization_descriptor"
#define MY_DID ts::DID_AIT_EXT_APP_AUTH
#define MY_TID ts::TID_AIT

TS_XML_TABSPEC_DESCRIPTOR_FACTORY(ts::ExternalApplicationAuthorizationDescriptor, MY_XML_NAME, MY_TID);
TS_ID_DESCRIPTOR_FACTORY(ts::ExternalApplicationAuthorizationDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));
TS_ID_DESCRIPTOR_DISPLAY(ts::ExternalApplicationAuthorizationDescriptor::DisplayDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::ExternalApplicationAuthorizationDescriptor::ExternalApplicationAuthorizationDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    entries()
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::ExternalApplicationAuthorizationDescriptor::ExternalApplicationAuthorizationDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    ExternalApplicationAuthorizationDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::ExternalApplicationAuthorizationDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    for (EntryList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        bbp->appendUInt32(it->application_identifier.organization_id);
        bbp->appendUInt16(it->application_identifier.application_id);
        bbp->appendUInt8(it->application_priority);
    }
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::ExternalApplicationAuthorizationDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    entries.clear();
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size % 7 == 0;

    if (_is_valid) {
        while (size >= 7) {
            entries.push_back(Entry(GetUInt32(data), GetUInt16(data + 4), GetUInt8(data + 6)));
            data += 7; size -= 7;
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::ExternalApplicationAuthorizationDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    while (size >= 7) {
        const uint32_t org = GetUInt32(data);
        const uint16_t app = GetUInt16(data + 4);
        const uint8_t prio = GetUInt8(data + 6);
        data += 7; size -= 7;
        strm << margin << UString::Format(u"- Organization id: 0x%X (%d)", {org, org}) << std::endl
             << margin << UString::Format(u"  Application id: 0x%X (%d)", {app, app}) << std::endl
             << margin << UString::Format(u"  Priority: 0x%X (%d)", {prio, prio}) << std::endl;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::ExternalApplicationAuthorizationDescriptor::buildXML(xml::Element* root) const
{
    for (EntryList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        xml::Element* e = root->addElement(u"application");
        e->setIntAttribute(u"organization_id", it->application_identifier.organization_id, true);
        e->setIntAttribute(u"application_id", it->application_identifier.application_id, true);
        e->setIntAttribute(u"application_priority", it->application_priority, false);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::ExternalApplicationAuthorizationDescriptor::fromXML(const xml::Element* element)
{
    entries.clear();

    xml::ElementVector children;
    _is_valid =
        checkXMLName(element) &&
        element->getChildren(children, u"application", 0, MAX_ENTRIES);

    for (size_t i = 0; _is_valid && i < children.size(); ++i) {
        Entry entry;
        _is_valid =
            children[i]->getIntAttribute<uint32_t>(entry.application_identifier.organization_id, u"organization_id", true) &&
            children[i]->getIntAttribute<uint16_t>(entry.application_identifier.application_id, u"application_id", true) &&
            children[i]->getIntAttribute<uint8_t>(entry.application_priority, u"application_priority", true);
        if (_is_valid) {
            entries.push_back(entry);
        }
    }
}
