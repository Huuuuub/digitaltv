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
//  Representation of an AC-4_descriptor
//
//----------------------------------------------------------------------------

#include "tsAC4Descriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"AC4_descriptor"
#define MY_DID ts::DID_DVB_EXTENSION
#define MY_EDID ts::EDID_AC4

TS_XML_DESCRIPTOR_FACTORY(ts::AC4Descriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::AC4Descriptor, ts::EDID::ExtensionDVB(MY_EDID));
TS_ID_DESCRIPTOR_DISPLAY(ts::AC4Descriptor::DisplayDescriptor, ts::EDID::ExtensionDVB(MY_EDID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::AC4Descriptor::AC4Descriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    ac4_dialog_enhancement_enabled(),
    ac4_channel_mode(),
    ac4_dsi_toc(),
    additional_info()
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::AC4Descriptor::AC4Descriptor(const Descriptor& desc, const DVBCharset* charset) :
    AC4Descriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::AC4Descriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt8(MY_EDID);
    bbp->appendUInt8((ac4_dialog_enhancement_enabled.set() && ac4_channel_mode.set() ? 0x80 : 0x00) | (!ac4_dsi_toc.empty() ? 0x40 : 0x00));
    if (ac4_dialog_enhancement_enabled.set() && ac4_channel_mode.set()) {
        bbp->appendUInt8((ac4_dialog_enhancement_enabled.value() ? 0x80 : 0x00) | ((ac4_channel_mode.value() & 0x03) << 5));
    }
    if (!ac4_dsi_toc.empty()) {
        bbp->appendUInt8(uint8_t(ac4_dsi_toc.size()));
        bbp->append(ac4_dsi_toc);
    }
    bbp->append(additional_info);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::AC4Descriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() >= 2 && data[0] == MY_EDID;

    ac4_dialog_enhancement_enabled.reset();
    ac4_channel_mode.reset();
    ac4_dsi_toc.clear();
    additional_info.clear();

    uint8_t flags = 0;

    if (_is_valid) {
        flags = data[1];
        data += 2; size -= 2;
    }

    if (_is_valid && (flags & 0x80) != 0) {
        _is_valid = size >= 1;
        if (_is_valid) {
            ac4_dialog_enhancement_enabled = (data[0] & 0x80) != 0;
            ac4_channel_mode = (data[0] >> 5) & 0x03;
            data++; size--;
        }
    }

    if (_is_valid && (flags & 0x40) != 0) {
        _is_valid = size >= 1;
        if (_is_valid) {
            const size_t toc_size = data[0];
            _is_valid = size >= 1 + toc_size;
            if (_is_valid) {
                ac4_dsi_toc.copy(data + 1, data[0]);
                data += 1 + toc_size; size -= 1 + toc_size;
            }
        }
    }

    if (_is_valid) {
        additional_info.copy(data, size);
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::AC4Descriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    // Important: With extension descriptors, the DisplayDescriptor() function is called
    // with extension payload. Meaning that data points after descriptor_tag_extension.
    // See ts::TablesDisplay::displayDescriptorData()

    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 1) {
        const uint8_t flags = data[0];
        data++; size--;
        if ((flags & 0x80) != 0 && size >= 1) {
            uint8_t type = data[0];
            data++; size--;
            strm << margin
                 << UString::Format(u"Dialog enhancement enabled: %d, channel mode: %s",
                                    {(type >> 7) & 0x01, DVBNameFromSection(u"AC4ChannelMode", (type >> 5) & 0x03, names::FIRST)})
                 << std::endl;
        }
        if ((flags & 0x40) != 0 && size >= 1) {
            const size_t toc_size = std::min<size_t>(data[0], size - 1);
            if (toc_size > 0) {
                strm << margin << "AC-4 TOC (in DSI):" << std::endl
                     << UString::Dump(data + 1, toc_size, UString::HEXA | UString::ASCII | UString::OFFSET, indent + 2);

            }
            data += 1 + toc_size; size -= 1 + toc_size;
        }
        if (size > 0) {
            strm << margin << "Additional information:" << std::endl
                 << UString::Dump(data, size, UString::HEXA | UString::ASCII | UString::OFFSET, indent + 2);
            data += size; size = 0;
        }
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::AC4Descriptor::buildXML(xml::Element* root) const
{
    root->setOptionalBoolAttribute(u"ac4_dialog_enhancement_enabled", ac4_dialog_enhancement_enabled);
    root->setOptionalIntAttribute(u"ac4_channel_mode", ac4_channel_mode);
    if (!ac4_dsi_toc.empty()) {
        root->addElement(u"ac4_dsi_toc")->addHexaText(ac4_dsi_toc);
    }
    if (!additional_info.empty()) {
        root->addElement(u"additional_info")->addHexaText(additional_info);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::AC4Descriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getOptionalBoolAttribute(ac4_dialog_enhancement_enabled, u"ac4_dialog_enhancement_enabled") &&
        element->getOptionalIntAttribute<uint8_t>(ac4_channel_mode, u"ac4_channel_mode", 0, 3) &&
        element->getHexaTextChild(ac4_dsi_toc, u"ac4_dsi_toc", false, 0, MAX_DESCRIPTOR_SIZE - 6) &&
        element->getHexaTextChild(additional_info, u"additional_info", false, 0, MAX_DESCRIPTOR_SIZE - 6 - ac4_dsi_toc.size());
}
