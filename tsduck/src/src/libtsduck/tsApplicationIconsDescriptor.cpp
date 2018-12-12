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

#include "tsApplicationIconsDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
#include "tsNames.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"application_icons_descriptor"
#define MY_DID ts::DID_AIT_APP_ICONS
#define MY_TID ts::TID_AIT

TS_XML_TABSPEC_DESCRIPTOR_FACTORY(ts::ApplicationIconsDescriptor, MY_XML_NAME, MY_TID);
TS_ID_DESCRIPTOR_FACTORY(ts::ApplicationIconsDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));
TS_ID_DESCRIPTOR_DISPLAY(ts::ApplicationIconsDescriptor::DisplayDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::ApplicationIconsDescriptor::ApplicationIconsDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    icon_locator(),
    icon_flags(0),
    reserved_future_use()
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::ApplicationIconsDescriptor::ApplicationIconsDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    ApplicationIconsDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::ApplicationIconsDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->append(icon_locator.toDVBWithByteLength(0, NPOS, charset));
    bbp->appendUInt16(icon_flags);
    bbp->append(reserved_future_use);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::ApplicationIconsDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    icon_locator.clear();
    reserved_future_use.clear();

    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size >= 1 && size >= size_t(data[0]) + 3;

    if (_is_valid) {
        icon_locator = UString::FromDVBWithByteLength(data, size, charset);
        assert(size >= 2);
        icon_flags = GetUInt16(data);
        reserved_future_use.copy(data + 2, size - 2);
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::ApplicationIconsDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size > 0) {
        strm << margin << "Icon locator: \"" << UString::FromDVBWithByteLength(data, size, display.dvbCharset()) << "\"" << std::endl;
        if (size >= 2) {
            const uint16_t flags = GetUInt16(data);
            strm << margin << UString::Format(u"Icon flags: 0x%X", {flags}) << std::endl;
            for (uint16_t mask = 0x0001; mask != 0; mask <<= 1) {
                if ((flags & mask) != 0) {
                    strm << margin << "  - " << DVBNameFromSection(u"ApplicationIconFlags", mask) << std::endl;
                }
            }
            if (size > 2) {
                strm << margin << "Reserved bytes:" << std::endl
                     << UString::Dump(data + 2, size - 2, UString::HEXA | UString::ASCII | UString::OFFSET, indent + 2);
            }
        }
    }
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::ApplicationIconsDescriptor::buildXML(xml::Element* root) const
{
    root->setAttribute(u"icon_locator", icon_locator);
    root->setIntAttribute(u"icon_flags", icon_flags, true);
    if (!reserved_future_use.empty()) {
        root->addElement(u"reserved_future_use")->addHexaText(reserved_future_use);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::ApplicationIconsDescriptor::fromXML(const xml::Element* element)
{
    icon_locator.clear();
    reserved_future_use.clear();

    _is_valid =
        checkXMLName(element) &&
        element->getAttribute(icon_locator, u"icon_locator", true) &&
        element->getIntAttribute<uint16_t>(icon_flags, u"icon_flags", true) &&
        element->getHexaTextChild(reserved_future_use, u"reserved_future_use");
}
