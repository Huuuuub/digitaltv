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
//  Representation of a cable_delivery_system_descriptor
//
//----------------------------------------------------------------------------

#include "tsCableDeliverySystemDescriptor.h"
#include "tsBCD.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"cable_delivery_system_descriptor"
#define MY_DID ts::DID_CABLE_DELIVERY

TS_XML_DESCRIPTOR_FACTORY(ts::CableDeliverySystemDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::CableDeliverySystemDescriptor, ts::EDID::Standard(MY_DID));
TS_ID_DESCRIPTOR_DISPLAY(ts::CableDeliverySystemDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Default constructor:
//----------------------------------------------------------------------------

ts::CableDeliverySystemDescriptor::CableDeliverySystemDescriptor() :
    AbstractDeliverySystemDescriptor(MY_DID, DS_DVB_C, MY_XML_NAME),
    frequency(0),
    FEC_outer(0),
    modulation(0),
    symbol_rate(0),
    FEC_inner(0)
{
    _is_valid = true;
}


//----------------------------------------------------------------------------
// Constructor from a binary descriptor
//----------------------------------------------------------------------------

ts::CableDeliverySystemDescriptor::CableDeliverySystemDescriptor(const Descriptor& desc, const DVBCharset* charset) :
    CableDeliverySystemDescriptor()
{
    deserialize(desc, charset);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::CableDeliverySystemDescriptor::serialize(Descriptor& desc, const DVBCharset* charset) const
{
    ByteBlockPtr bbp(serializeStart());

    bbp->appendBCD(frequency, 8);
    bbp->appendUInt16(0xFFF0 | FEC_outer);
    bbp->appendUInt8(modulation);
    bbp->appendBCD(symbol_rate, 7);   // The last 4 bits are unused.
    const size_t last = bbp->size() - 1;
    (*bbp)[last] = ((*bbp)[last] & 0xF0) | (FEC_inner & 0x0F);

    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::CableDeliverySystemDescriptor::deserialize(const Descriptor& desc, const DVBCharset* charset)
{
    if (!(_is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() == 11)) {
        return;
    }

    const uint8_t* data = desc.payload();

    frequency = DecodeBCD(data, 8);
    FEC_outer = data[5] & 0x0F;
    modulation = data[6];
    symbol_rate = DecodeBCD(data + 7, 7);
    FEC_inner = data[10] & 0x0F;
}


//----------------------------------------------------------------------------
// Enumerations for XML.
//----------------------------------------------------------------------------

namespace {
    const ts::Enumeration ModulationNames({
        {u"16-QAM", 1},
        {u"32-QAM", 2},
        {u"64-QAM", 3},
        {u"128-QAM", 4},
        {u"256-QAM", 5},
    });

    const ts::Enumeration OuterFecNames({
        {u"undefined", 0},
        {u"none", 1},
        {u"RS", 2},
    });

    const ts::Enumeration InnerFecNames({
        {u"undefined", 0},
        {u"1/2", 1},
        {u"2/3", 2},
        {u"3/4", 3},
        {u"5/6", 4},
        {u"7/8", 5},
        {u"8/9", 6},
        {u"3/5", 7},
        {u"4/5", 8},
        {u"9/10", 9},
        {u"none", 15},
    });
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::CableDeliverySystemDescriptor::buildXML(xml::Element* root) const
{
    root->setIntAttribute(u"frequency", 100 * uint64_t(frequency), false);
    root->setIntEnumAttribute(OuterFecNames, u"FEC_outer", FEC_outer);
    root->setIntEnumAttribute(ModulationNames, u"modulation", modulation);
    root->setIntAttribute(u"symbol_rate", 100 * uint64_t(symbol_rate), false);
    root->setIntEnumAttribute(InnerFecNames, u"FEC_inner", FEC_inner);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::CableDeliverySystemDescriptor::fromXML(const xml::Element* element)
{
    uint64_t freq = 0;
    uint64_t symrate = 0;

    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint64_t>(freq, u"frequency", true) &&
        element->getIntEnumAttribute<uint8_t>(FEC_outer, OuterFecNames, u"FEC_outer", false, 2) &&
        element->getIntEnumAttribute<uint8_t>(modulation, ModulationNames, u"modulation", false, 1) &&
        element->getIntAttribute<uint64_t>(symrate, u"symbol_rate", true) &&
        element->getIntEnumAttribute(FEC_inner, InnerFecNames, u"FEC_inner", true);

    if (_is_valid) {
        frequency = uint32_t(freq / 100);
        symbol_rate = uint32_t(symrate / 100);
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::CableDeliverySystemDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    std::ostream& strm(display.out());
    const std::string margin(indent, ' ');

    if (size >= 11) {
        uint8_t fec_outer = data[5] & 0x0F;
        uint8_t modulation = data[6];
        uint8_t fec_inner = data[10] & 0x0F;
        std::string freq, srate;
        BCDToString(freq, data, 8, 4);
        BCDToString(srate, data + 7, 7, 3);
        data += 11; size -= 11;

        strm << margin << "Frequency: " << freq << " MHz" << std::endl
             << margin << "Symbol rate: " << srate << " Msymbol/s" << std::endl
             << margin << "Modulation: ";
        switch (modulation) {
            case 0:  strm << "not defined"; break;
            case 1:  strm << "16-QAM"; break;
            case 2:  strm << "32-QAM"; break;
            case 3:  strm << "64-QAM"; break;
            case 4:  strm << "128-QAM"; break;
            case 5:  strm << "256-QAM"; break;
            default: strm << "code " << int(modulation) << " (reserved)"; break;
        }
        strm << std::endl << margin << "Outer FEC: ";
        switch (fec_outer) {
            case 0:  strm << "not defined"; break;
            case 1:  strm << "none"; break;
            case 2:  strm << "RS(204/188)"; break;
            default: strm << "code " << int(fec_outer) << " (reserved)"; break;
        }
        strm << ", Inner FEC: ";
        switch (fec_inner) {
            case 0:  strm << "not defined"; break;
            case 1:  strm << "1/2 conv. code rate"; break;
            case 2:  strm << "2/3 conv. code rate"; break;
            case 3:  strm << "3/4 conv. code rate"; break;
            case 4:  strm << "5/6 conv. code rate"; break;
            case 5:  strm << "7/8 conv. code rate"; break;
            case 6:  strm << "8/9 conv. code rate"; break;
            case 7:  strm << "3/5 conv. code rate"; break;
            case 8:  strm << "4/5 conv. code rate"; break;
            case 9:  strm << "9/10 conv. code rate"; break;
            case 15: strm << "none"; break;
            default: strm << "code " << int(fec_inner) << " (reserved)"; break;
        }
        strm << std::endl;
    }

    display.displayExtraData(data, size, indent);
}
