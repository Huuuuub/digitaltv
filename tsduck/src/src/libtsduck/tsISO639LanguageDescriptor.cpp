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
//  Representation of an ISO_639_language_descriptor
//
//----------------------------------------------------------------------------

#include "tsISO639LanguageDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"ISO_639_language_descriptor"
#define MY_DID ts::DID_LANGUAGE

TS_XML_DESCRIPTOR_FACTORY(ts::ISO639LanguageDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::ISO639LanguageDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::ISO639LanguageDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::ISO639LanguageDescriptor::Entry::Entry(const UChar* code, uint8_t type) :
    language_code(code),
    audio_type(type)
{
}

ts::ISO639LanguageDescriptor::Entry::Entry(const UString& code, uint8_t type) :
    language_code(code),
    audio_type(type)
{
}

ts::ISO639LanguageDescriptor::ISO639LanguageDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    entries()
{
    _is_valid = true;
}

ts::ISO639LanguageDescriptor::ISO639LanguageDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    entries()
{
    deserialize(desc, charset);
}

ts::ISO639LanguageDescriptor::ISO639LanguageDescriptor(const UString& code, uint8_t type) :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    entries()
{
    _is_valid = true;
    entries.push_back(Entry(code, type));
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::ISO639LanguageDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());

    for (EntryList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        if (!SerializeLanguageCode(*bbp, it->language_code, charset)) {
            desc.invalidate();
            return;
        }
        bbp->appendUInt8(it->audio_type);
    }

    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::ISO639LanguageDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    _is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() % 4 == 0;
    entries.clear();

    if (_is_valid) {
        const uint8_t* data = desc.payload();
        size_t size = desc.payloadSize();
        while (size >= 4) {
            entries.push_back(Entry(UString::FromDVB(data, 3, charset), data[3]));
            data += 4;
            size -= 4;
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::ISO639LanguageDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    while (size >= 4) {
        const uint8_t type = data[3];
        strm << margin << "Language: " << UString::FromDVB(data, 3, display.dvbCharset())
             << ", Type: " << names::AudioType(type, names::FIRST) << std::endl;
        data += 4; size -= 4;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::ISO639LanguageDescriptor::buildXML(xml::Element* root) const
{
    for (EntryList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        xml::Element* e = root->addElement(u"language");
        e->setAttribute(u"code", it->language_code);
        e->setIntAttribute(u"audio_type", it->audio_type, true);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::ISO639LanguageDescriptor::fromXML(const xml::Element* element)
{
    entries.clear();

    xml::ElementVector children;
    _is_valid =
        checkXMLName(element) &&
        element->getChildren(children, u"language", 0, MAX_ENTRIES);

    for (size_t i = 0; _is_valid && i < children.size(); ++i) {
        Entry entry;
        _is_valid =
            children[i]->getAttribute(entry.language_code, u"code", true, u"", 3, 3) &&
            children[i]->getIntAttribute<uint8_t>(entry.audio_type, u"audio_type", true, 0, 0x00, 0xFF);
        if (_is_valid) {
            entries.push_back(entry);
        }
    }
}
