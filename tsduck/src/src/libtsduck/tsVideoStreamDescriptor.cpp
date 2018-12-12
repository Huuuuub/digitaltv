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

#include "tsVideoStreamDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"video_stream_descriptor"
#define MY_DID ts::DID_VIDEO

TS_XML_DESCRIPTOR_FACTORY(ts::VideoStreamDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::VideoStreamDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::VideoStreamDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::VideoStreamDescriptor::VideoStreamDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    multiple_frame_rate(false),
    frame_rate_code(0),
    MPEG_1_only(false),
    constrained_parameter(false),
    still_picture(false),
    profile_and_level_indication(0),
    chroma_format(0),
    frame_rate_extension(false)

{
    _is_valid = true;
}

ts::VideoStreamDescriptor::VideoStreamDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    VideoStreamDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::VideoStreamDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt8((multiple_frame_rate ? 0x80 : 00) |
                     ((frame_rate_code & 0x0F) << 3) |
                     (MPEG_1_only ? 0x04 : 00) |
                     (constrained_parameter ? 0x02 : 00) |
                     (still_picture ? 0x01 : 00));
    if (!MPEG_1_only) {
        bbp->appendUInt8(profile_and_level_indication);
        bbp->appendUInt8(((chroma_format & 0x03) << 6) |
                         (frame_rate_extension ? 0x20 : 00) |
                         0x1F);
    }
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::VideoStreamDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size >= 1;

    if (_is_valid) {
        multiple_frame_rate = (data[0] & 0x80) != 0;
        frame_rate_code = (data[0] >> 3) & 0x0F;
        MPEG_1_only = (data[0] & 0x04) != 0;
        constrained_parameter = (data[0] & 0x02) != 0;
        still_picture = (data[0] & 0x01) != 0;
        _is_valid = (MPEG_1_only && size == 1) || (!MPEG_1_only && size == 3);
        if (_is_valid && !MPEG_1_only) {
            profile_and_level_indication = data[1];
            chroma_format = (data[2] >> 6) & 0x03;
            frame_rate_extension = (data[2] & 0x20) != 0;
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::VideoStreamDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 1) {
        const bool mp1only = (data[0] & 0x04) != 0;
        strm << margin
             << UString::Format(u"Multiple frame rate: %s, frame rate: %s",
                                {UString::TrueFalse((data[0] & 0x80) != 0),
                                 DVBNameFromSection(u"FrameRate", (data[0] >> 3) & 0x0F, names::FIRST)})
             << std::endl
             << margin
             << UString::Format(u"MPEG-1 only: %s, constained parameter: %s, still picture: %s",
                                {UString::TrueFalse(mp1only),
                                 UString::TrueFalse((data[0] & 0x02) != 0),
                                 UString::TrueFalse((data[0] & 0x01) != 0)})
             << std::endl;
        data++; size--;
        if (!mp1only && size >= 2) {
            strm << margin << UString::Format(u"Profile and level: 0x%X (%d)", {data[0], data[0]}) << std::endl
                 << margin << "Chroma format: " << DVBNameFromSection(u"ChromaFormat", (data[1] >> 6) & 0x03, names::FIRST) << std::endl
                 << margin << "Frame rate extension: " << UString::TrueFalse((data[1] & 0x20) != 0) << std::endl;
            data += 2; size -= 2;
        }
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::VideoStreamDescriptor::buildXML(xml::Element* root) const
{
    root->setBoolAttribute(u"multiple_frame_rate", multiple_frame_rate);
    root->setIntAttribute(u"frame_rate_code", frame_rate_code);
    root->setBoolAttribute(u"MPEG_1_only", MPEG_1_only);
    root->setBoolAttribute(u"constrained_parameter", constrained_parameter);
    root->setBoolAttribute(u"still_picture", still_picture);
    if (!MPEG_1_only) {
        root->setIntAttribute(u"profile_and_level_indication", profile_and_level_indication, true);
        root->setIntAttribute(u"chroma_format", chroma_format);
        root->setBoolAttribute(u"frame_rate_extension", frame_rate_extension);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::VideoStreamDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getBoolAttribute(multiple_frame_rate, u"multiple_frame_rate", true) &&
        element->getIntAttribute<uint8_t>(frame_rate_code, u"frame_rate_code", true, 0, 0x00, 0x0F) &&
        element->getBoolAttribute(MPEG_1_only, u"MPEG_1_only", true) &&
        element->getBoolAttribute(constrained_parameter, u"constrained_parameter", true) &&
        element->getBoolAttribute(still_picture, u"still_picture", true) &&
        element->getIntAttribute<uint8_t>(profile_and_level_indication, u"profile_and_level_indication", !MPEG_1_only) &&
        element->getIntAttribute<uint8_t>(chroma_format, u"chroma_format", !MPEG_1_only, 0, 0x00, 0x03) &&
        element->getBoolAttribute(frame_rate_extension, u"frame_rate_extension", !MPEG_1_only);
}
