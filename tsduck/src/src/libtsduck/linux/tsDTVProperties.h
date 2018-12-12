//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
//!
//!  @file
//!  Encapsulation of Linux DVB property lists.
//!
//-----------------------------------------------------------------------------

#pragma once
#include "tsReport.h"

namespace ts {
    //!
    //! Encapsulation of Linux DVB property lists.
    //! @ingroup unix
    //!
    class TSDUCKDLL DTVProperties
    {
    public:
        //!
        //! Default constructor.
        //!
        DTVProperties();

        //!
        //! Destructor.
        //!
        virtual ~DTVProperties();

        //!
        //! Get the number of properties in the buffer.
        //! @return The number of properties in the buffer.
        //!
        size_t count() const
        {
            return size_t(_prop_head.num);
        }

        //!
        //! Add a new property.
        //! @param [in] cmd Command code.
        //! @param [in] data Optional command data.
        //! @return The index in property buffer.
        //!
        size_t add(uint32_t cmd, uint32_t data = -1);

        //!
        //! Search a property in the buffer.
        //! @param [in] cmd Command code.
        //! @return The index of @a cmd in buffer or count() if not found.
        //!
        size_t search(uint32_t cmd) const;

        //!
        //! Get the value of a property in the buffer.
        //! @param [in] cmd Command code.
        //! @return The data value of @a cmd in buffer or @link UNKNOWN @endlink if not found.
        //!
        uint32_t getByCommand(uint32_t cmd) const;

        //!
        //! Get the value of the property at a specified index.
        //! @param [in] index Index in buffer.
        //! @return The data value at @a index or @link UNKNOWN @endlink if out of range.
        //!
        uint32_t getByIndex(size_t index) const
        {
            return index >= size_t(_prop_head.num) ? UNKNOWN : _prop_buffer[index].u.data;
        }

        //!
        //! Get the address of the @c dtv_properties structure for @c ioctl() call.
        //! @return The address of the @c dtv_properties structure.
        //!
        const ::dtv_properties* getIoctlParam() const
        {
            return &_prop_head;
        }

        //!
        //! Get the address of the @c dtv_properties structure for @c ioctl() call.
        //! @return The address of the @c dtv_properties structure.
        //!
        ::dtv_properties* getIoctlParam()
        {
            return &_prop_head;
        }

        //!
        //! Returned value for unknown data.
        //!
        static const uint32_t UNKNOWN = ~0;

        //!
        //! Display the content of the object (for debug purpose).
        //! @param [in,out] report Where to display the content.
        //! @param [in] severity Severity level of messages (typically a debug level).
        //!
        void report(Report& report, int severity) const;

        //!
        //! Return the name of a command.
        //! @param [in] cmd Command code.
        //! @return A name for @a cmd or zero if unknown.
        //!
        static const char* CommandName(uint32_t cmd);

    private:
        // Private members:
        ::dtv_property   _prop_buffer[DTV_IOCTL_MAX_MSGS];
        ::dtv_properties _prop_head;
    };
}
