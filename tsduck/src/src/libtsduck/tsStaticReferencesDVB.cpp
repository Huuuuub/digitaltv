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

#include "tsStaticReferencesDVB.h"
#include "tsTables.h"
#include "tsDVBCharsetSingleByte.h"
#include "tsDVBCharsetUTF16.h"
#include "tsDVBCharsetUTF8.h"
TSDUCK_SOURCE;

// Macros to generate a unique symbol name.
#define REF_NAME1(a,b) a##b
#define REF_NAME2(a,b) REF_NAME1(a,b)
#define REF_NAME       REF_NAME2(REF,__LINE__)

// Macros to generate a dummy reference to a type using a local unused instance.
#define REF_TYPE(type) static const ts::type REF_NAME; _refs.push_back(&REF_NAME)

// Macros to generate a dummy reference to a type using an existing instance.
#define REF_OBJECT(obj) _refs.push_back(&ts::obj)

//
// Constructor of the dummy reference-maker.
//
ts::StaticReferencesDVB::StaticReferencesDVB() :
    _refs()
{
    _refs.reserve(100);

    // References to all object files containing DVB character sets.

    REF_OBJECT(DVBCharsetSingleByte::ISO_6937);
    REF_OBJECT(DVBCharsetUTF16::UNICODE);
    REF_OBJECT(DVBCharsetUTF8::UTF_8);

    // References to all DVB tables and descriptors.
    // The file tsRefType.h is automatically generated.

    #include "private/tsRefType.h"
}
