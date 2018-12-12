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
//  Abstract base class for MPEG PSI/SI descriptors
//
//----------------------------------------------------------------------------

#include "tsAbstractDescriptor.h"
#include "tsDescriptorList.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// Protected constructor for subclasses.
//----------------------------------------------------------------------------

ts::AbstractDescriptor::AbstractDescriptor(DID tag, const UChar* xml_name, PDS pds) :
    AbstractSignalization(xml_name),
    _tag(tag),
    _required_pds(pds)
{
}


//----------------------------------------------------------------------------
// Deserialize from a descriptor list.
//----------------------------------------------------------------------------

void ts::AbstractDescriptor::deserialize(const DescriptorList& dlist, size_t index, const DVBCharset* charset)
{
    if (index > dlist.count()) {
        invalidate();
    }
    else {
        deserialize(*dlist[index], charset);
    }
}


//----------------------------------------------------------------------------
// Tools for serialization
//----------------------------------------------------------------------------

ts::ByteBlockPtr ts::AbstractDescriptor::serializeStart() const
{
    ByteBlockPtr bbp(new ByteBlock(2));
    CheckNonNull(bbp.pointer());
    (*bbp)[0] = _tag;
    (*bbp)[1] = 0;
    return bbp;
}

bool ts::AbstractDescriptor::serializeEnd(Descriptor& desc, const ByteBlockPtr& bbp) const
{
    if (bbp->size() > MAX_DESCRIPTOR_SIZE) {
        desc.invalidate();
        return false;
    }
    else {
        (*bbp)[0] = _tag;
        (*bbp)[1] = uint8_t(bbp->size() - 2);
        desc = Descriptor(bbp, SHARE);
        return true;
    }
}
