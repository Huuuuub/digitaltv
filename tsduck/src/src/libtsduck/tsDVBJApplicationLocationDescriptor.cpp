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

#include "tsDVBJApplicationLocationDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"dvb_j_application_location_descriptor"
#define MY_DID ts::DID_AIT_DVBJ_APP_LOC
#define MY_TID ts::TID_AIT

TS_XML_TABSPEC_DESCRIPTOR_FACTORY(ts::DVBJApplicationLocationDescriptor, MY_XML_NAME, MY_TID);
TS_ID_DESCRIPTOR_FACTORY(ts::DVBJApplicationLocationDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));
TS_ID_DESCRIPTOR_DISPLAY(ts::DVBJApplicationLocationDescriptor::DisplayDescriptor, ts::EDID::TableSpecific(MY_DID, MY_TID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::DVBJApplicationLocationDescriptor::DVBJApplicationLocationDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME),
    base_directory(),
    classpath_extension(),
    initial_class()
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::DVBJApplicationLocationDescriptor::DVBJApplicationLocationDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    DVBJApplicationLocationDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::DVBJApplicationLocationDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->append(base_directory.toDVBWithByteLength(0, NPOS, charset));
    bbp->append(classpath_extension.toDVBWithByteLength(0, NPOS, charset));
    bbp->append(initial_class.toDVB(0, NPOS, charset));
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::DVBJApplicationLocationDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    base_directory.clear();
    classpath_extension.clear();
    initial_class.clear();

    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    _is_valid = desc.isValid() && desc.tag() == _tag && size >= 1;

    if (_is_valid) {
        const size_t len1 = data[0];
        data += 1; size -= 1;
        _is_valid = len1 + 1 <= size;
        if (_is_valid) {
            base_directory = UString::FromDVB(data, len1, charset);
            const size_t len2 = data[len1];
            data += len1 + 1; size -= len1 + 1;
            _is_valid = len2 <= size;
            if (_is_valid) {
                classpath_extension = UString::FromDVB(data, len2, charset);
                initial_class = UString::FromDVB(data + len2, size - len2, charset);
            }
        }
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::DVBJApplicationLocationDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 1) {
        size_t len = std::min<size_t>(data[0], size - 1);
        strm << margin << "Base directory: \"" << UString::FromDVB(data + 1, len, display.dvbCharset()) << "\"" << std::endl;
        data += 1 + len; size -= 1 + len;
        if (size >= 1) {
            len = std::min<size_t>(data[0], size - 1);
            strm << margin << "Classpath ext: \"" << UString::FromDVB(data + 1, len, display.dvbCharset()) << "\"" << std::endl
                 << margin << "Initial class: \"" << UString::FromDVB(data + 1 + len, size - len - 1, display.dvbCharset()) << "\"" << std::endl;
            size = 0;
        }
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::DVBJApplicationLocationDescriptor::buildXML(xml::Element* root) const
{
    root->setAttribute(u"base_directory", base_directory);
    root->setAttribute(u"classpath_extension", classpath_extension);
    root->setAttribute(u"initial_class", initial_class);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::DVBJApplicationLocationDescriptor::fromXML(const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getAttribute(base_directory, u"base_directory", true) &&
        element->getAttribute(classpath_extension, u"classpath_extension", true) &&
        element->getAttribute(initial_class, u"initial_class", true);
}
