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
//!  Representation of a Running Status Table (RST)
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsAbstractTable.h"
#include "tsEnumeration.h"

namespace ts {
    //!
    //! Representation of a Running Status Table (RST).
    //! @ingroup table
    //!
    class TSDUCKDLL RST : public AbstractTable
    {
    public:
        //!
        //! Description of an event.
        //!
        struct TSDUCKDLL Event
        {
            uint16_t transport_stream_id;  //!< Transport stream id.
            uint16_t original_network_id;  //!< Original network id.
            uint16_t service_id;           //!< Service id.
            uint16_t event_id;             //!< Event id.
            uint8_t  running_status;       //!< Running status of the event.

            //!
            //! Default constructor.
            //!
            Event() :
                transport_stream_id(0),
                original_network_id(0),
                service_id(0),
                event_id(0),
                running_status(0)
            {
            }
        };

        //!
        //! List of Events.
        //!
        typedef std::list<Event> EventList;

        // RST public members:
        EventList events;  //!< List of events with a running status.

        //!
        //! Definition of names for running status values.
        //!
        static const Enumeration RunningStatusNames;

        //!
        //! Default constructor.
        //!
        RST();

        //!
        //! Constructor from a binary table.
        //! @param [in] table Binary table to deserialize.
        //! @param [in] charset If not zero, character set to use without explicit table code.
        //!
        RST(const BinaryTable& table, const DVBCharset* charset);

        // Inherited methods
        virtual void serialize(BinaryTable& table, const DVBCharset* = nullptr) const override;
        virtual void deserialize(const BinaryTable& table, const DVBCharset* = nullptr) override;
        virtual void buildXML(xml::Element*) const override;
        virtual void fromXML(const xml::Element*) override;
        DeclareDisplaySection();
    };
}
