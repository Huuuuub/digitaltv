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
//  Representation of an STD_descriptor
//
//----------------------------------------------------------------------------

#include "tsSTDDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"STD_descriptor"
#define MY_DID ts::DID_STD

TS_XML_DESCRIPTOR_FACTORY(ts::STDDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::STDDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::STDDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::STDDescriptor::STDDescriptor(bool leak_valid_) :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    leak_valid(leak_valid_)
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::STDDescriptor::STDDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    leak_valid(false)
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::STDDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    uint8_t data[3];
    data[0] = _tag;
    data[1] = 1;
    data[2] = leak_valid ? 0xFF : 0xFE;

    Descriptor d(data, sizeof(data));
    desc = d;
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::STDDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    _is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() == 1;

    if (_is_valid) {
        leak_valid = (*desc.payload() & 0x01) != 0;
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::STDDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 1) {
        uint8_t leak = data[0] & 0x01;
        data += 1; size -= 1;
        strm << margin << "Link valid flag: " << int(leak)
             << (leak != 0 ? " (leak)" : " (vbv_delay)") << std::endl;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::STDDescriptor::buildXML(xml::Element* root) const
{
    root->setBoolAttribute(u"leak_valid", leak_valid);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::STDDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getBoolAttribute(leak_valid, u"leak_valid", true);
}
