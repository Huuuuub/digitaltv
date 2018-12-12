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
//!  Representation of a Program Map Table (PMT)
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsAbstractLongTable.h"
#include "tsDescriptorList.h"

namespace ts {
    //!
    //! Representation of a Program Map Table (PMT).
    //! @ingroup table
    //!
    class TSDUCKDLL PMT : public AbstractLongTable
    {
    public:
        //!
        //! Description of an elementary stream.
        //!
        //! Note: by inheriting from EntryWithDescriptors, there is a
        //! public field "DescriptorList descs".
        //!
        struct TSDUCKDLL Stream : public EntryWithDescriptors
        {
            uint8_t stream_type;  //!< Stream type, one of ST_* (eg ts::ST_MPEG2_VIDEO).

            //!
            //! Constructor.
            //! @param [in] table Parent PMT.
            //! @param [in] type Stream type.
            //!
            explicit Stream(const AbstractTable* table, uint8_t type = 0) :
                EntryWithDescriptors(table),
                stream_type(type)
            {
            }

            //!
            //! Check if an elementary stream carries audio.
            //! Does not just look at the stream type.
            //! Also analyzes the descriptor list for additional information.
            //! @return True if the elementary stream carries audio.
            //!
            bool isAudio() const;

            //!
            //! Check if an elementary stream carries video.
            //! Does not just look at the stream type.
            //! Also analyzes the descriptor list for additional information.
            //! @return True if the elementary stream carries video.
            //!
            bool isVideo() const;

            //!
            //! Check if an elementary stream carries subtitles.
            //! Does not just look at the stream type.
            //! Also analyzes the descriptor list for additional information.
            //! @return True if the elementary stream carries subtitles.
            //!
            bool isSubtitles() const;

            //!
            //! Look for a component tag in a stream_identifier_descriptor.
            //! @param [out] tag First component tag found, unmodified if none found.
            //! @return True if a component tag was found.
            //!
            bool getComponentTag(uint8_t& tag) const;

        private:
            // Inaccessible operations.
            Stream() = delete;
            Stream(const Stream&) = delete;
        };

        //!
        //! List of elementary streams, indexed by PID.
        //!
        typedef EntryWithDescriptorsMap<PID, Stream> StreamMap;

        // PMT public members:
        uint16_t       service_id;  //!< Service id aka "program_number".
        PID            pcr_pid;     //!< PID for PCR data.
        DescriptorList descs;       //!< Program-level descriptor list.
        StreamMap      streams;     //!< Map of stream descriptions: key=PID, value=stream_description.

        //!
        //! Default constructor.
        //! @param [in] version Table version number.
        //! @param [in] is_current True if table is current, false if table is next.
        //! @param [in] service_id Service identifier.
        //! @param [in] pcr_pid PID of the PCR. Default: none.
        //!
        PMT(uint8_t  version = 0,
            bool     is_current = true,
            uint16_t service_id = 0,
            PID      pcr_pid = PID_NULL);

        //!
        //! Copy constructor.
        //! @param [in] other Other instance to copy.
        //!
        PMT(const PMT& other);

        //!
        //! Constructor from a binary table.
        //! @param [in] table Binary table to deserialize.
        //! @param [in] charset If not zero, character set to use without explicit table code.
        //!
        PMT(const BinaryTable& table, const DVBCharset* charset = nullptr);

        //!
        //! Search the component PID for a given component tag.
        //! @param [in] tag Component tag to search.
        //! @return The PID of the corresponding component of PID_NULL if not found.
        //!
        PID componentTagToPID(uint8_t tag) const;

        // Inherited methods
        virtual void serialize(BinaryTable& table, const DVBCharset* = nullptr) const override;
        virtual void deserialize(const BinaryTable& table, const DVBCharset* = nullptr) override;
        virtual void buildXML(xml::Element*) const override;
        virtual void fromXML(const xml::Element*) override;
        DeclareDisplaySection();
    };
}
