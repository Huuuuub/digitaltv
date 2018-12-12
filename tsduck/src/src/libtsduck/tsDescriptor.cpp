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
//  Representation of MPEG PSI/SI descriptors
//
//----------------------------------------------------------------------------

#include "tsDescriptor.h"
#include "tsMemoryUtils.h"
#include "tsAbstractDescriptor.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// Constructors for Descriptor
// Note that the max size of a descriptor is 257 bytes: 2 (header) + 255
//----------------------------------------------------------------------------

ts::Descriptor::Descriptor(const void* addr, size_t size) :
    _data(size >= 2 && size < 258 && (reinterpret_cast<const uint8_t*>(addr))[1] == size - 2 ? new ByteBlock(addr, size) : nullptr)
{
}

ts::Descriptor::Descriptor(const ByteBlock& bb) :
    _data(bb.size() >= 2 && bb.size() < 258 && bb[1] == bb.size() - 2 ? new ByteBlock(bb) : nullptr)
{
}

ts::Descriptor::Descriptor(DID tag, const void* data, size_t size) :
    _data(size < 256 ? new ByteBlock(size + 2) : nullptr)
{
    if (!_data.isNull()) {
        (*_data)[0] = tag;
        (*_data)[1] = uint8_t(size);
        ::memcpy(_data->data() + 2, data, size);
    }
}

ts::Descriptor::Descriptor(DID tag, const ByteBlock& data) :
    _data(data.size() < 256 ? new ByteBlock(2) : nullptr)
{
    if (!_data.isNull()) {
        (*_data)[0] = tag;
        (*_data)[1] = uint8_t(data.size());
        _data->append(data);
    }
}

ts::Descriptor::Descriptor(const ByteBlockPtr& bbp, CopyShare mode) :
    _data(nullptr)
{
    if (!bbp.isNull() && bbp->size() >= 2 && bbp->size() < 258 && (*bbp)[1] == bbp->size() - 2) {
        switch (mode) {
            case SHARE:
                _data = bbp;
                break;
            case COPY:
                _data = new ByteBlock(*bbp);
                break;
            default:
                // should not get there
                assert(false);
        }
    }
}

ts::Descriptor::Descriptor(const Descriptor& desc, CopyShare mode) :
    _data(nullptr)
{
    switch (mode) {
        case SHARE:
            _data = desc._data;
            break;
        case COPY:
            _data = new ByteBlock(*desc._data);
            break;
        default:
            // should not get there
            assert(false);
    }
}


//----------------------------------------------------------------------------
// Get the extended descriptor id.
//----------------------------------------------------------------------------

ts::EDID ts::Descriptor::edid(PDS pds) const
{
    if (!isValid()) {
        return EDID();  // invalid value.
    }
    const DID did = tag();
    if (did >= 0x80) {
        // Private descriptor.
        return EDID::Private(did, pds);
    }
    else if (did == DID_DVB_EXTENSION && payloadSize() > 0) {
        // DVB extension descriptor.
        return EDID::ExtensionDVB(payload()[0]);
    }
    else if (did == DID_MPEG_EXTENSION && payloadSize() > 0) {
        // MPEG extension descriptor.
        return EDID::ExtensionMPEG(payload()[0]);
    }
    else {
        // Standard descriptor.
        return EDID::Standard(did);
    }
}


//----------------------------------------------------------------------------
// Replace the payload of the descriptor. The tag is unchanged,
// the size is adjusted.
//----------------------------------------------------------------------------

void ts::Descriptor::replacePayload(const void* addr, size_t size)
{
    if (size > 255) {
        // Payload size too long, invalidate descriptor
        _data.clear();
    }
    else if (!_data.isNull()) {
        assert(_data->size() >= 2);
        // Erase previous payload
        _data->erase(2, _data->size() - 2);
        // Add new payload
        _data->append (addr, size);
        // Adjust descriptor size
        (*_data)[1] = uint8_t (_data->size() - 2);
    }
}


//----------------------------------------------------------------------------
// Resize (truncate or extend) the payload of the descriptor.
// The tag is unchanged, the size is adjusted.
// If the payload is extended, new bytes are zeroes.
//----------------------------------------------------------------------------

void ts::Descriptor::resizePayload(size_t new_size)
{
    if (new_size > 255) {
        // Payload size too long, invalidate descriptor
        _data.clear();
    }
    else if (!_data.isNull()) {
        assert(_data->size() >= 2);
        size_t old_size = _data->size() - 2;
        _data->resize (new_size + 2);
        // If payload extended, zero additional bytes
        if (new_size > old_size) {
            Zero(_data->data() + 2 + old_size, new_size - old_size);
        }
        // Adjust descriptor size
        (*_data)[1] = uint8_t (_data->size() - 2);
    }
}


//----------------------------------------------------------------------------
// Comparison
//----------------------------------------------------------------------------

bool ts::Descriptor::operator== (const Descriptor& desc) const
{
    return _data == desc._data ||
        (_data.isNull() && desc._data.isNull()) ||
        (!_data.isNull() && !desc._data.isNull() && *_data == *desc._data);
}


//----------------------------------------------------------------------------
// This method converts a descriptor to XML.
//----------------------------------------------------------------------------

ts::xml::Element* ts::Descriptor::toXML(xml::Element* parent, PDS pds, TID tid, bool forceGeneric, const DVBCharset* charset) const
{
    // Filter invalid descriptors.
    if (!isValid()) {
        return nullptr;
    }

    // The XML node we will generate.
    xml::Element* node = nullptr;

    // Try to generate a specialized XML structure.
    if (!forceGeneric) {
        // Do we know how to deserialize this descriptor?
        TablesFactory::DescriptorFactory fac = TablesFactory::Instance()->getDescriptorFactory(edid(pds), tid);
        if (fac != nullptr) {
            // We know how to deserialize it.
            AbstractDescriptorPtr dp = fac();
            if (!dp.isNull()) {
                // Deserialize from binary to object.
                dp->deserialize(*this, charset);
                if (dp->isValid()) {
                    // Serialize from object to XML.
                    node = dp->toXML(parent);
                }
            }
        }
    }

    // If we could not generate a typed node, generate a generic one.
    if (node == nullptr) {
        // Create the XML node.
        node = parent->addElement(TS_XML_GENERIC_DESCRIPTOR);
        node->setIntAttribute(u"tag", tag(), true);
        node->addHexaText(payload(), payloadSize());
    }

    return node;
}


//----------------------------------------------------------------------------
// This method converts an XML node as a binary descriptor.
//----------------------------------------------------------------------------

bool ts::Descriptor::fromXML(const xml::Element* node, TID tid, const DVBCharset* charset)
{
    // Filter invalid parameters.
    invalidate();
    if (node == nullptr) {
        // Not a valid XML name (not even an XML element).
        return false;
    }

    // If the table is specified and the XML descriptor is not allowed in this table, this is an error.
    if (!TablesFactory::Instance()->isDescriptorAllowed(node->name(), tid)) {
        node->report().error(u"<%s>, line %d, is not allowed here, must be in %s", {node->name(), node->lineNumber(), TablesFactory::Instance()->descriptorTables(node->name())});
        return false;
    }

    // Try to get the descriptor factory for that kind of XML tag.
    const TablesFactory::DescriptorFactory fac = TablesFactory::Instance()->getDescriptorFactory(node->name());
    if (fac != nullptr) {
        // Create a descriptor instance of the right type.
        AbstractDescriptorPtr desc = fac();
        if (!desc.isNull()) {
            desc->fromXML(node);
        }
        if (!desc.isNull() && desc->isValid()) {
            // Serialize the descriptor.
            desc->serialize(*this, charset);
        }
        // The XML element name was valid.
        return true;
    }

    // Try to decode a generic descriptor.
    if (node->name().similar(TS_XML_GENERIC_DESCRIPTOR)) {
        DID tag = 0xFF;
        ByteBlock payload;
        if (node->getIntAttribute<DID>(tag, u"tag", true, 0xFF, 0x00, 0xFF) && node->getHexaText(payload, 0, 255)) {
            // Build descriptor.
            _data = new ByteBlock(2);
            (*_data)[0] = tag;
            (*_data)[1] = uint8_t(payload.size());
            _data->append(payload);
            return true;
        }
        else {
            node->report().error(u"<%s>, line %d, is not a valid descriptor", {node->name(), node->lineNumber()});
        }
    }

    // The XML element name was not valid.
    return false;
}
