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

#include "tsDTSNeuralDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"DTS_neural_descriptor"
#define MY_DID ts::DID_DVB_EXTENSION
#define MY_EDID ts::EDID_DTS_NEURAL

TS_XML_DESCRIPTOR_FACTORY(ts::DTSNeuralDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::DTSNeuralDescriptor, ts::EDID::ExtensionDVB(MY_EDID));
TS_ID_DESCRIPTOR_DISPLAY(ts::DTSNeuralDescriptor::DisplayDescriptor, ts::EDID::ExtensionDVB(MY_EDID));


//----------------------------------------------------------------------------
// Constructors.
//----------------------------------------------------------------------------

ts::DTSNeuralDescriptor::DTSNeuralDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    config_id(0),
    additional_info()
{
    _is_valid = true;
}

ts::DTSNeuralDescriptor::DTSNeuralDescriptor(const Descriptor& bin, const DVBCharset* charset) :
    DTSNeuralDescriptor()
{
    deserialize(bin, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::DTSNeuralDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt8(MY_EDID);
    bbp->appendUInt8(config_id);
    bbp->append (additional_info);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::DTSNeuralDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();
    _is_valid = desc.isValid() && desc.tag() == _tag && size >= 2 && data[0] == MY_EDID;

    if (_is_valid) {
        config_id = data[1];
        additional_info.copy(data + 2, size - 2);
    }
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::DTSNeuralDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"config_id", config_id, true);
    if (!additional_info.empty()) {
        root->addElement(u"additional_info")->addHexaText(additional_info);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::DTSNeuralDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint8_t>(config_id, u"config_id", true) &&
        element->getHexaTextChild(additional_info, u"additional_info", false, 0, MAX_DESCRIPTOR_SIZE - 4);
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::DTSNeuralDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    // Important: With extension descriptors, the DisplayDescriptor() function is called
    // with extension payload. Meaning that data points after descriptor_tag_extension.
    // See ts::TablesDisplay::displayDescriptorData()

    if (size > 0) {
        std::ostream& strm(display.out());
        const std::string margin(indent, ' ');
        strm << margin << UString::Format(u"Config Id: 0x%X (%d))", {data[0], data[0]}) << std::endl;
        if (size > 1) {
            strm << margin << "Additional info:" << std::endl
                 << UString::Dump(data + 1, size - 1, UString::HEXA | UString::ASCII | UString::OFFSET, indent);
        }
    }
}
