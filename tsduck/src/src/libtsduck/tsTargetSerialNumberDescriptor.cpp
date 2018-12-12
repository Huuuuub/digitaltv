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

#include "tsTargetSerialNumberDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"target_serial_number_descriptor"
#define MY_DID ts::DID_INT_SERIAL_NUM

TS_XML_TABSPEC_DESCRIPTOR_FACTORY(ts::TargetSerialNumberDescriptor, MY_XML_NAME, ts::TID_INT, ts::TID_UNT);

TS_ID_DESCRIPTOR_FACTORY(ts::TargetSerialNumberDescriptor, ts::EDID::TableSpecific(MY_DID, ts::TID_INT));
TS_ID_DESCRIPTOR_FACTORY(ts::TargetSerialNumberDescriptor, ts::EDID::TableSpecific(MY_DID, ts::TID_UNT));

TS_ID_DESCRIPTOR_DISPLAY(ts::TargetSerialNumberDescriptor::DisplayDescriptor, ts::EDID::TableSpecific(MY_DID, ts::TID_INT));
TS_ID_DESCRIPTOR_DISPLAY(ts::TargetSerialNumberDescriptor::DisplayDescriptor, ts::EDID::TableSpecific(MY_DID, ts::TID_UNT));


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::TargetSerialNumberDescriptor::TargetSerialNumberDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    serial_data()
{
    _is_valid = true;
}

ts::TargetSerialNumberDescriptor::TargetSerialNumberDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    TargetSerialNumberDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::TargetSerialNumberDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->append(serial_data);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::TargetSerialNumberDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    _is_valid = desc.isValid() && desc.tag() == _tag;

    if (_is_valid) {
        serial_data.copy(desc.payload(), desc.payloadSize());
    }
    else {
        serial_data.clear();
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::TargetSerialNumberDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    display.out() << margin
                  << UString::Format(u"%*sSerial number (%d bytes): %s", {indent, u"", size, UString::Dump(data, size, UString::SINGLE_LINE)})
                  << std::endl;
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::TargetSerialNumberDescriptor::buildXML(xml::Element* root) const
{
    if (!serial_data.empty()) {
        root->addHexaText(serial_data);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::TargetSerialNumberDescriptor::fromXML(const xml::Element* element)
{
    serial_data.clear();

    _is_valid =
        checkXMLName(element) &&
        element->getHexaText(serial_data, 0, MAX_DESCRIPTOR_SIZE - 2);
}
