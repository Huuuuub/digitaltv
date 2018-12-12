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
//!  Representation of a terrestrial_delivery_system_descriptor
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsAbstractDeliverySystemDescriptor.h"

namespace ts {
    //!
    //! Representation of a terrestrial_delivery_system_descriptor.
    //! @see ETSI 300 468, 6.2.13.4.
    //! @ingroup descriptor
    //!
    class TSDUCKDLL TerrestrialDeliverySystemDescriptor : public AbstractDeliverySystemDescriptor
    {
    public:
        // TerrestrialDeliverySystemDescriptor public members:
        uint32_t centre_frequency;   //!< Frequency, unit is 10 Hz.
        uint8_t  bandwidth;          //!< Bandwidth, 0..7 (3 bits).
        bool     high_priority;      //!< Must be true if hierarchy == 0.
        bool     no_time_slicing;    //!< No time slicing.
        bool     no_mpe_fec;         //!< NO MPE-FEC.
        uint8_t  constellation;      //!< Constellation, 0..3 (2 bits).
        uint8_t  hierarchy;          //!< Hierarchy, 0..7 (3 bits).
        uint8_t  code_rate_hp;       //!< Code Rate, high priority, 0..7 (3 bits).
        uint8_t  code_rate_lp;       //!< Code Rate, low priority, 0..7 (3 bits).
        uint8_t  guard_interval;     //!< Guard interval, 0..3 (2 bits).
        uint8_t  transmission_mode;  //!< Transmission mode, 0..3 (2 bits).
        bool     other_frequency;    //!< Other frequency.

        //!
        //! Default constructor.
        //!
        TerrestrialDeliverySystemDescriptor();

        //!
        //! Constructor from a binary descriptor.
        //! @param [in] bin A binary descriptor to deserialize.
        //! @param [in] charset If not zero, character set to use without explicit table code.
        //!
        TerrestrialDeliverySystemDescriptor(const Descriptor& bin, const DVBCharset* charset = nullptr);

        // Inherited methods
        virtual void serialize(Descriptor&, const DVBCharset* = nullptr) const override;
        virtual void deserialize(const Descriptor&, const DVBCharset* = nullptr) override;
        virtual void buildXML(xml::Element*) const override;
        virtual void fromXML(const xml::Element*) override;
        DeclareDisplayDescriptor();
    };
}
