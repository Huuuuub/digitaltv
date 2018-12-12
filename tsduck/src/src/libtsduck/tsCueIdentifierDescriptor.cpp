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

#include "tsCueIdentifierDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"cue_identifier_descriptor"
#define MY_DID ts::DID_CUE_IDENTIFIER

// The is a non-DVB descriptor with DID >= 0x80 => must set PDS to zero in EDID.
TS_XML_DESCRIPTOR_FACTORY(ts::CueIdentifierDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::CueIdentifierDescriptor, ts::EDID::Private(MY_DID, 0));
TS_ID_DESCRIPTOR_DISPLAY(ts::CueIdentifierDescriptor::DisplayDescriptor, ts::EDID::Private(MY_DID, 0));


//----------------------------------------------------------------------------
// Definition of names for cue stream types.
//----------------------------------------------------------------------------

const ts::Enumeration ts::CueIdentifierDescriptor::CueStreamTypeNames({
    {u"insert_null_schedule", 0x00},
    {u"all",                  0x01},
    {u"segmentation",         0x02},
    {u"tiered_splicing",      0x03},
    {u"tiered_segmentation",  0x04},
});


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::CueIdentifierDescriptor::CueIdentifierDescriptor(uint8_t type) :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    cue_stream_type(type)
{
    _is_valid = true;
}

ts::CueIdentifierDescriptor::CueIdentifierDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    cue_stream_type(CUE_ALL_COMMANDS)
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::CueIdentifierDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt8(cue_stream_type);
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::CueIdentifierDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    _is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() == 1;

    if (_is_valid) {
        cue_stream_type = *desc.payload();
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::CueIdentifierDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 1) {
        strm << margin << UString::Format(u"Cue stream type: 0x%X", {data[0]});
        switch (data[0]) {
            case 0x00: strm << " (splice_insert, splice_null, splice_schedule)"; break;
            case 0x01: strm << " (All commands)"; break;
            case 0x02: strm << " (Segmentation)"; break;
            case 0x03: strm << " (Tiered splicing)"; break;
            case 0x04: strm << " (Tiered segmentation)"; break;
            default: break;
        }
        strm << std::endl;
        data += 1; size -= 1;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::CueIdentifierDescriptor::buildXML(xml::Element* root) const
{
    root->setEnumAttribute(CueStreamTypeNames, u"cue_stream_type", cue_stream_type);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::CueIdentifierDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntEnumAttribute<uint8_t>(cue_stream_type, CueStreamTypeNames, u"cue_stream_type", true);
}
