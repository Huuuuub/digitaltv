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

#include "tsT2MIDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"T2MI_descriptor"
#define MY_DID ts::DID_DVB_EXTENSION
#define MY_EDID ts::EDID_T2MI

TS_XML_DESCRIPTOR_FACTORY(ts::T2MIDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::T2MIDescriptor, ts::EDID::ExtensionDVB(MY_EDID));
TS_ID_DESCRIPTOR_DISPLAY(ts::T2MIDescriptor::DisplayDescriptor, ts::EDID::ExtensionDVB(MY_EDID));


//----------------------------------------------------------------------------
// Constructor.
//----------------------------------------------------------------------------

ts::T2MIDescriptor::T2MIDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    t2mi_stream_id(0),
    num_t2mi_streams_minus_one(0),
    pcr_iscr_common_clock_flag(false),
    reserved()
{
    _is_valid = true;
}

ts::T2MIDescriptor::T2MIDescriptor(const Descriptor& bin, const DVBCharset* charset) :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    t2mi_stream_id(0),
    num_t2mi_streams_minus_one(0),
    pcr_iscr_common_clock_flag(false),
    reserved()
{
    deserialize(bin, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::T2MIDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt8(MY_EDID);
    bbp->appendUInt8(t2mi_stream_id & 0x07);
    bbp->appendUInt8(num_t2mi_streams_minus_one & 0x07);
    bbp->appendUInt8(pcr_iscr_common_clock_flag ? 0x01 : 0x00);
    bbp->append(reserved);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::T2MIDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    if (!(_is_valid = desc.isValid() && desc.tag() == _tag && size >= 4 && data[0] == MY_EDID)) {
        return;
    }

    t2mi_stream_id = data[1] & 0x07;
    num_t2mi_streams_minus_one = data[2] & 0x07;
    pcr_iscr_common_clock_flag = (data[3] & 0x01) != 0;
    reserved.copy(data + 4, size - 4);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::T2MIDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"t2mi_stream_id", t2mi_stream_id, true);
    root->setIntAttribute(u"num_t2mi_streams_minus_one", num_t2mi_streams_minus_one);
    root->setBoolAttribute(u"pcr_iscr_common_clock_flag", pcr_iscr_common_clock_flag);
    if (!reserved.empty()) {
        root->addElement(u"reserved")->addHexaText(reserved);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::T2MIDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint8_t>(t2mi_stream_id, u"t2mi_stream_id", true, 0, 0, 7) &&
        element->getIntAttribute<uint8_t>(num_t2mi_streams_minus_one, u"num_t2mi_streams_minus_one", false, 0, 0, 7) &&
        element->getBoolAttribute(pcr_iscr_common_clock_flag, u"pcr_iscr_common_clock_flag", false, false) &&
        element->getHexaTextChild(reserved, u"reserved", false, 0, MAX_DESCRIPTOR_SIZE - 6);
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::T2MIDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    // Important: With extension descriptors, the DisplayDescriptor() function is called
    // with extension payload. Meaning that data points after descriptor_tag_extension.
    // See ts::TablesDisplay::displayDescriptorData()

    if (size >= 3) {
        std::ostream& strm(display.out());
        const std::string margin(indent, ' ');
        strm << margin << "T2-MI stream id: " << int(data[0] & 0x07)
             << ", T2-MI stream count: " << int((data[1] & 0x07) + 1)
             << ", PCR/ISCR common clock: " << UString::YesNo(data[2] & 0x01)
             << std::endl;
        data += 3; size -= 3;
    }
    display.displayExtraData(data, size, indent);
}
