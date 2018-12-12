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
//  Representation of an AAC_descriptor
//
//----------------------------------------------------------------------------

#include "tsAACDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
#include "tsNames.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"AAC_descriptor"
#define MY_DID ts::DID_AAC

TS_XML_DESCRIPTOR_FACTORY(ts::AACDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::AACDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::AACDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::AACDescriptor::AACDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    profile_and_level(0),
    SAOC_DE(false),
    AAC_type(),
    additional_info()
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::AACDescriptor::AACDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    AACDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::AACDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());

    bbp->appendUInt8(profile_and_level);
    if (SAOC_DE || AAC_type.set() || !additional_info.empty()) {
        bbp->appendUInt8((AAC_type.set() ? 0x80 : 0x00) | (SAOC_DE ? 0x40 : 0x00));
        if (AAC_type.set()) {
            bbp->appendUInt8(AAC_type.value());
        }
        bbp->append(additional_info);
    }

    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::AACDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size >= 1;

    if (_is_valid) {
        profile_and_level = data[0];
        SAOC_DE = false;
        AAC_type.reset();
        additional_info.clear();
        data++; size--;

        if (size > 0) {
            SAOC_DE = (data[0] & 0x40) != 0;
            if ((data[0] & 0x40) != 0) {
                _is_valid = size > 1;
                if (_is_valid) {
                    AAC_type = data[1];
                    data++; size--;
                }
            }
            data++; size--;
            additional_info.copy(data, size);
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::AACDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 1) {
        uint8_t prof_lev = data[0];
        data++; size--;
        strm << margin << UString::Format(u"Profile and level: 0x%X", {prof_lev}) << std::endl;
        if (size >= 1) {
            uint8_t flags = data[0];
            data++; size--;
            if ((flags & 0x80) && size >= 1) { // AAC_type
                uint8_t type = data[0];
                data++; size--;
                strm << margin << "AAC type: " << DVBNameFromSection(u"ComponentType", 0x6F00 | type, names::HEXA_FIRST, 8) << std::endl;
            }
            if (size > 0) {
                strm << margin << "Additional information:" << std::endl
                     << UString::Dump(data, size, UString::HEXA | UString::ASCII | UString::OFFSET, indent);
                data += size; size = 0;
            }
        }
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::AACDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"profile_and_level", profile_and_level, true);
    root->setBoolAttribute(u"SAOC_DE", SAOC_DE);
    root->setOptionalIntAttribute(u"AAC_type", AAC_type, true);
    if (!additional_info.empty()) {
        root->addElement(u"additional_info")->addHexaText(additional_info);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::AACDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint8_t>(profile_and_level, u"profile_and_level", true) &&
        element->getBoolAttribute(SAOC_DE, u"SAOC_DE", false) &&
        element->getOptionalIntAttribute(AAC_type, u"AAC_type") &&
        element->getHexaTextChild(additional_info, u"additional_info", false, 0, MAX_DESCRIPTOR_SIZE - 5);
}
