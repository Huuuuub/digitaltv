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
//!
//!  @file
//!  @ingroup cpp
//!  Some utilities on integers.
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsUString.h"

namespace ts {
    //!
    //! Perform a bounded addition without overflow.
    //! @tparam INT An integer type, any size, signed or unsigned.
    //! @param [in] a First integer.
    //! @param [in] b Second integer.
    //! @return The value @a a + @a b. The value is @e bounded, in
    //! case of underflow or overflow, the result is the min or max
    //! value of the type, respectively.
    //!
    template <typename INT, typename std::enable_if<std::is_integral<INT>::value>::type* = nullptr>
    INT BoundedAdd(INT a, INT b);

    //!
    //! Perform a bounded subtraction without overflow.
    //! @tparam INT An integer type, any size, signed or unsigned.
    //! @param [in] a First integer.
    //! @param [in] b Second integer.
    //! @return The value @a a - @a b. The value is @e bounded, in
    //! case of underflow or overflow, the result is the min or max
    //! value of the type, respectively.
    //!
    template <typename INT, typename std::enable_if<std::is_integral<INT>::value>::type* = nullptr>
    INT BoundedSub(INT a, INT b);

    //!
    //! Round @a x down to previous multiple of a factor @a f.
    //! @tparam INT An integer type.
    //! @param [in] x An integer value.
    //! @param [in] f A factor.
    //! @return The value @a x rounded down to previous multiple of @a f.
    //!
    template<typename INT, typename std::enable_if<std::is_integral<INT>::value>::type* = nullptr>
    inline INT RoundDown(INT x, INT f)
    {
        return x - x % f;
    }

    //!
    //! Round @a x up to next multiple of a factor @a f.
    //! @tparam INT An integer type.
    //! @param [in] x An integer value.
    //! @param [in] f A factor.
    //! @return The value @a x rounded up to next multiple of @a f.
    //!
    template<typename INT, typename std::enable_if<std::is_integral<INT>::value>::type* = nullptr>
    inline INT RoundUp(INT x, INT f)
    {
        return RoundDown(x + f - 1, f);
    }

    //!
    //! Compute the maximum width of the decimal representation of an integer type.
    //! @param [in] typeSize Size of the integer type in bytes (result of @c sizeof).
    //! @param [in] digitSeparatorSize Size in characters of the digit-grouping separator.
    //! @return The maximum width in characters.
    //!
    size_t MaxDecimalWidth(size_t typeSize, size_t digitSeparatorSize = 0);

    //!
    //! Compute the maximum width of the hexadecimal representation of an integer type.
    //! @param [in] typeSize Size of the integer type in bytes (result of @c sizeof).
    //! @param [in] digitSeparatorSize Size in characters of the digit-grouping separator.
    //! @return The maximum width in characters.
    //!
    size_t MaxHexaWidth(size_t typeSize, size_t digitSeparatorSize = 0);
}

#include "tsIntegerUtilsTemplate.h"
