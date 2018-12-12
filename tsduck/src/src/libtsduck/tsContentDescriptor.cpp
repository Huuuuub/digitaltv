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
//
//  Representation of a content_descriptor
//
//----------------------------------------------------------------------------

#include "tsContentDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"content_descriptor"
#define MY_DID ts::DID_CONTENT

TS_XML_DESCRIPTOR_FACTORY(ts::ContentDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::ContentDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::ContentDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::ContentDescriptor::ContentDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    entries()
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::ContentDescriptor::ContentDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    entries()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::ContentDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp (new ByteBlock (2));
    CheckNonNull (bbp.pointer());

    for (EntryList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        bbp->appendUInt8 ((it->content_nibble_level_1 << 4) | (it->content_nibble_level_2 & 0x0F));
        bbp->appendUInt8 ((it->user_nibble_1 << 4) | (it->user_nibble_2 & 0x0F));
    }

    (*bbp)[0] = _tag;
    (*bbp)[1] = uint8_t(bbp->size() - 2);
    Descriptor d (bbp, SHARE);
    desc = d;
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::ContentDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    _is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() % 2 == 0;
    entries.clear();

    if (_is_valid) {
        const uint8_t* data = desc.payload();
        size_t size = desc.payloadSize();
        while (size >= 2) {
            entries.push_back (Entry (GetUInt16 (data)));
            data += 2; size -= 2;
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::ContentDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    while (size >= 2) {
        uint8_t content = data[0];
        uint8_t user = data[1];
        data += 2; size -= 2;
        strm << margin << UString::Format(u"Content: %s / User: 0x%X", {names::Content(content, names::FIRST), user}) << std::endl;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::ContentDescriptor::buildXML(xml::Element* root) const
{
    for (EntryList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        xml::Element* e = root->addElement(u"content");
        e->setIntAttribute(u"content_nibble_level_1", it->content_nibble_level_1);
        e->setIntAttribute(u"content_nibble_level_2", it->content_nibble_level_2);
        e->setIntAttribute(u"user_byte", uint8_t((it->user_nibble_1 << 4) | it->user_nibble_2), true);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::ContentDescriptor::fromXML(const xml::Element* element)
{
    entries.clear();

    xml::ElementVector children;
    _is_valid =
        checkXMLName(element) &&
        element->getChildren(children, u"content", 0, MAX_ENTRIES);

    for (size_t i = 0; _is_valid && i < children.size(); ++i) {
        Entry entry;
        uint8_t user = 0;
        _is_valid =
            children[i]->getIntAttribute<uint8_t>(entry.content_nibble_level_1, u"content_nibble_level_1", true, 0, 0x00, 0x0F) &&
            children[i]->getIntAttribute<uint8_t>(entry.content_nibble_level_2, u"content_nibble_level_2", true, 0, 0x00, 0x0F) &&
            children[i]->getIntAttribute<uint8_t>(user, u"user_byte", true, 0, 0x00, 0xFF);
        if (_is_valid) {
            entry.user_nibble_1 = (user >> 4) & 0x0F;
            entry.user_nibble_2 = user & 0x0F;
            entries.push_back(entry);
        }
    }
}
