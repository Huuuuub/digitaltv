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
//!  Representation of MPEG PSI/SI sections
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsPlatform.h"
#include "tsCerrReport.h"
#include "tsByteBlock.h"
#include "tsTablesPtr.h"
#include "tsCASFamily.h"
#include "tsCRC32.h"
#include "tsETID.h"
#include "tsTLVSyntax.h"

namespace ts {
    //!
    //! Representation of MPEG PSI/SI sections.
    //! @ingroup mpeg
    //!
    //! What to do with the CRC32 when building a section depends on the
    //! parameter named @a crc_op:
    //!
    //! - IGNORE:  Neither check nor compute.
    //! - CHECK:   Validate the CRC from the section data. Mark the section as invalid if CRC is incorrect.
    //! - COMPUTE: Compute the CRC and store it in the section.
    //!
    //! Typically, if the ByteBlock comes from the wire, use CHECK.
    //! If the ByteBlock is built by the application, use COMPUTE,
    //!
    class TSDUCKDLL Section
    {
    public:
        //!
        //! Default constructor.
        //! Section is initially marked invalid.
        //!
        Section();

        //!
        //! Copy constructor.
        //! @param [in] other Another instance to copy.
        //! @param [in] mode The section's data are either shared (ts::SHARE) between the
        //! two instances or duplicated (ts::COPY).
        //!
        Section(const Section& other, CopyShare mode);

        //!
        //! Constructor from full binary content.
        //! The content is copied into the section if valid.
        //! @param [in] content Address of the binary section data.
        //! @param [in] content_size Size in bytes of the section.
        //! @param [in] source_pid PID from which the section was read.
        //! @param [in] crc_op How to process the CRC32.
        //!
        Section(const void* content,
                size_t content_size,
                PID source_pid = PID_NULL,
                CRC32::Validation crc_op = CRC32::IGNORE);

        //!
        //! Constructor from full binary content.
        //! The content is copied into the section if valid.
        //! @param [in] content Binary section data.
        //! @param [in] source_pid PID from which the section was read.
        //! @param [in] crc_op How to process the CRC32.
        //!
        Section(const ByteBlock& content,
                PID source_pid = PID_NULL,
                CRC32::Validation crc_op = CRC32::IGNORE);

        //!
        //! Constructor from full binary content.
        //! The content is copied into the section if valid.
        //! @param [in] content_ptr Safe pointer to the binary section data.
        //! The content is referenced, and thus shared.
        //! Do not modify the referenced ByteBlock from outside the Section.
        //! @param [in] source_pid PID from which the section was read.
        //! @param [in] crc_op How to process the CRC32.
        //!
        Section(const ByteBlockPtr& content_ptr,
                PID source_pid = PID_NULL,
                CRC32::Validation crc_op = CRC32::IGNORE);

        //!
        //! Constructor from a short section payload.
        //! @param [in] tid Table id.
        //! @param [in] is_private_section If true, this is a private section (ie. not MPEG-defined).
        //! @param [in] payload Address of the payload data.
        //! @param [in] payload_size Size in bytes of the payload data.
        //! @param [in] source_pid PID from which the section was read.
        //!
        Section(TID tid,
                bool is_private_section,
                const void* payload,
                size_t payload_size,
                PID source_pid = PID_NULL);

        //!
        //! Constructor from a long section payload.
        //! The provided payload does not contain the CRC32.
        //! The CRC32 is automatically computed.
        //! @param [in] tid Table id.
        //! @param [in] is_private_section If true, this is a private section (ie. not MPEG-defined).
        //! @param [in] tid_ext Table id extension.
        //! @param [in] version Section version number.
        //! @param [in] is_current If true, this is a "current" section, not a "next" section.
        //! @param [in] section_number Section number.
        //! @param [in] last_section_number Number of last section in the table.
        //! @param [in] payload Address of the payload data.
        //! @param [in] payload_size Size in bytes of the payload data.
        //! @param [in] source_pid PID from which the section was read.
        //!
        Section(TID tid,
                bool is_private_section,
                uint16_t tid_ext,
                uint8_t version,
                bool is_current,
                uint8_t section_number,
                uint8_t last_section_number,
                const void* payload,
                size_t payload_size,
                PID source_pid = PID_NULL);

        //!
        //! Reload from full binary content.
        //! The content is copied into the section if valid.
        //! @param [in] content Address of the binary section data.
        //! @param [in] content_size Size in bytes of the section.
        //! @param [in] source_pid PID from which the section was read.
        //! @param [in] crc_op How to process the CRC32.
        //!
        void reload(const void* content,
                    size_t content_size,
                    PID source_pid = PID_NULL,
                    CRC32::Validation crc_op = CRC32::IGNORE)
        {
            initialize(new ByteBlock(content, content_size), source_pid, crc_op);
        }

        //!
        //! Reload from full binary content.
        //! @param [in] content Binary section data.
        //! @param [in] source_pid PID from which the section was read.
        //! @param [in] crc_op How to process the CRC32.
        //!
        void reload(const ByteBlock& content,
                     PID source_pid = PID_NULL,
                     CRC32::Validation crc_op = CRC32::IGNORE)
        {
            initialize(new ByteBlock(content), source_pid, crc_op);
        }

        //!
        //! Reload from full binary content.
        //! @param [in] content_ptr Safe pointer to the binary section data.
        //! The content is referenced, and thus shared.
        //! Do not modify the referenced ByteBlock from outside the Section.
        //! @param [in] source_pid PID from which the section was read.
        //! @param [in] crc_op How to process the CRC32.
        //!
        void reload(const ByteBlockPtr& content_ptr,
                    PID source_pid = PID_NULL,
                    CRC32::Validation crc_op = CRC32::IGNORE)
        {
            initialize(content_ptr, source_pid, crc_op);
        }

        //!
        //! Reload from a short section payload.
        //! @param [in] tid Table id.
        //! @param [in] is_private_section If true, this is a private section (ie. not MPEG-defined).
        //! @param [in] payload Address of the payload data.
        //! @param [in] payload_size Size in bytes of the payload data.
        //! @param [in] source_pid PID from which the section was read.
        //!
        void reload(TID tid,
                    bool is_private_section,
                    const void* payload,
                    size_t payload_size,
                    PID source_pid = PID_NULL);

        //!
        //! Reload from a long section payload.
        //! The provided payload does not contain the CRC32.
        //! The CRC32 is automatically computed.
        //! @param [in] tid Table id.
        //! @param [in] is_private_section If true, this is a private section (ie. not MPEG-defined).
        //! @param [in] tid_ext Table id extension.
        //! @param [in] version Section version number.
        //! @param [in] is_current If true, this is a "current" section, not a "next" section.
        //! @param [in] section_number Section number.
        //! @param [in] last_section_number Number of last section in the table.
        //! @param [in] payload Address of the payload data.
        //! @param [in] payload_size Size in bytes of the payload data.
        //! @param [in] source_pid PID from which the section was read.
        //!
        void reload(TID tid,
                    bool is_private_section,
                    uint16_t tid_ext,
                    uint8_t version,
                    bool is_current,
                    uint8_t section_number,
                    uint8_t last_section_number,
                    const void* payload,
                    size_t payload_size,
                    PID source_pid = PID_NULL);

        //!
        //! Clear section content.
        //! Becomes invalid section.
        //!
        void clear()
        {
            _is_valid = false;
            _source_pid = PID_NULL;
            _first_pkt = 0;
            _last_pkt = 0;
            _data.clear();
        }

        //!
        //! Assignment operator.
        //! The sections contents are referenced, and thus shared between the two section objects.
        //! @param [in] other Other section to assign to this object.
        //! @return A reference to this object.
        //!
        Section& operator=(const Section& other);

        //!
        //! Duplication.
        //! Similar to assignment but the sections are duplicated.
        //! @param [in] other Other section to duplicate into this object.
        //! @return A reference to this object.
        //!
        Section& copy(const Section& other);

        //!
        //! Check if the section has valid content.
        //! @return True if the section has valid content.
        //!
        bool isValid() const
        {
            return _is_valid;
        }

        //!
        //! Equality operator.
        //! The source PID's are ignored, only the section contents are compared.
        //! Invalid sections are never identical.
        //! @param [in] other Other section to compare.
        //! @return True if the two sections are identical. False otherwise.
        //!
        bool operator==(const Section& other) const;

        //!
        //! Unequality operator.
        //! The source PID's are ignored, only the section contents are compared.
        //! Invalid sections are never identical.
        //! @param [in] other Other section to compare.
        //! @return True if the two sections are different. False otherwise.
        //!
        bool operator!=(const Section& other) const
        {
            return !(*this == other);
        }

        //!
        //! Get the table id.
        //! @return The table id.
        //!
        TID tableId() const
        {
            return _is_valid ? (*_data)[0] : 0xFF;
        }

        //!
        //! Check if the section is a long one.
        //! @return True if the section is a long one.
        //!
        bool isLongSection() const
        {
            return _is_valid ? ((*_data)[1] & 0x80) != 0 : false;
        }

        //!
        //! Check if the section is a short one.
        //! @return True if the section is a short one.
        //!
        bool isShortSection() const {
            return _is_valid ? ((*_data)[1] & 0x80) == 0 : false;
        }

        //!
        //! Check if the section is a private one (ie. not MPEG-defined).
        //! @return True if the section is a private one (ie. not MPEG-defined).
        //!
        bool isPrivateSection() const
        {
            return _is_valid ? ((*_data)[1] & 0x40) != 0 : false;
        }

        //!
        //! Get the table id extension (long section only).
        //! @return The table id extension.
        //!
        uint16_t tableIdExtension() const
        {
            return isLongSection() ? GetUInt16(&(*_data)[3]) : 0;
        }

        //!
        //! Get the section version number (long section only).
        //! @return The section version number.
        //!
        uint8_t version() const
        {
            return isLongSection() ? (((*_data)[5] >> 1) & 0x1F) : 0;
        }

        //!
        //! Check if the section is "current", not "next" (long section only).
        //! @return True if the section is "current", false if it is "next".
        //!
        bool isCurrent() const
        {
            return isLongSection() ? ((*_data)[5] & 0x01) != 0 : false;
        }

        //!
        //! Check if the section is "next", not "current" (long section only).
        //! @return True if the section is "next", false if it is "current".
        //!
        bool isNext() const
        {
            return isLongSection() ? ((*_data)[5] & 0x01) == 0 : false;
        }

        //!
        //! Get the section number in the table (long section only).
        //! @return The section number in the table.
        //!
        uint8_t sectionNumber() const
        {
            return isLongSection() ? (*_data)[6] : 0;
        }

        //!
        //! Get the number of the last section in the table (long section only).
        //! @return The number of the last section in the table.
        //!
        uint8_t lastSectionNumber() const
        {
            return isLongSection() ? (*_data)[7] : 0;
        }

        //!
        //! Get the table id and id extension (long section only).
        //! @return The table id and id extension as an ETID.
        //!
        ETID etid() const
        {
            return isLongSection() ? ETID(tableId(), tableIdExtension()) : ETID(tableId());
        }

        //!
        //! Get the source PID.
        //! @return The source PID.
        //!
        PID sourcePID() const
        {
            return _source_pid;
        }

        //!
        //! Access to the full binary content of the section.
        //! Do not modify content.
        //! @return Address of the full binary content of the section.
        //! May be invalidated after modification in section.
        //!
        const uint8_t* content() const
        {
            return _data->data();
        }

        //!
        //! Size of the binary content of the section.
        //! @return Size of the binary content of the section.
        //!
        size_t size() const
        {
            return _data->size();
        }

        //!
        //! Size of the section header.
        //! @return Size of the section header.
        //!
        size_t headerSize() const
        {
            return _is_valid ? (isLongSection() ? LONG_SECTION_HEADER_SIZE : SHORT_SECTION_HEADER_SIZE) : 0;
        }

        //!
        //! Access to the payload of the section.
        //!
        //! For short sections, the payload starts after the
        //! private_section_length field. For long sections, the payload
        //! starts after the last_section_number field and ends before
        //! the CRC32 field. Do not modify payload content.
        //! May be invalidated after modification in section.
        //!
        //! @return Address of the payload of the section.
        //!
        const uint8_t* payload() const
        {
            return _is_valid ? _data->data() + (isLongSection() ? LONG_SECTION_HEADER_SIZE : SHORT_SECTION_HEADER_SIZE) : nullptr;
        }

        //!
        //! Get the size of the payload of the section.
        //! For long sections, the payload ends before the CRC32 field.
        //! @return Size in bytes of the payload of the section.
        //!
        size_t payloadSize() const
        {
            return _is_valid ? _data->size() - (isLongSection() ? LONG_SECTION_HEADER_SIZE + SECTION_CRC32_SIZE : SHORT_SECTION_HEADER_SIZE) : 0;
        }

        //!
        //! Minimum number of TS packets required to transport the section.
        //! @return The minimum number of TS packets required to transport the section.
        //!
        PacketCounter packetCount() const {return SectionPacketCount(size());}

        //!
        //! Set the table id extension (long section only).
        //! @param [in] tid_ext The table id extension.
        //! @param [in] recompute_crc If true, immediately recompute the CRC32 of the section.
        //!
        void setTableIdExtension(uint16_t tid_ext, bool recompute_crc = true);

        //!
        //! Set the section version number (long section only).
        //! @param [in] version The section version number.
        //! @param [in] recompute_crc If true, immediately recompute the CRC32 of the section.
        //!
        void setVersion(uint8_t version, bool recompute_crc = true);

        //!
        //! Set the section current/next flag (long section only).
        //! @param [in] is_current True if the table is "current", false if it is "next".
        //! @param [in] recompute_crc If true, immediately recompute the CRC32 of the section.
        //!
        void setIsCurrent(bool is_current, bool recompute_crc = true);

        //!
        //! Set the section number (long section only).
        //! @param [in] num The section number.
        //! @param [in] recompute_crc If true, immediately recompute the CRC32 of the section.
        //!
        void setSectionNumber(uint8_t num, bool recompute_crc = true);

        //!
        //! Set the number of the last section in the table (long section only).
        //! @param [in] num The number of the last section in the table.
        //! @param [in] recompute_crc If true, immediately recompute the CRC32 of the section.
        //!
        void setLastSectionNumber(uint8_t num, bool recompute_crc = true);

        //!
        //! Set one byte in the payload of the section.
        //! @param [in] offset Byte offset in the payload.
        //! @param [in] value The value to set in the payload.
        //! @param [in] recompute_crc If true, immediately recompute the CRC32 of the section.
        //!
        void setUInt8(size_t offset, uint8_t value, bool recompute_crc = true);

        //!
        //! Set a 16-bit integer in the payload of the section.
        //! @param [in] offset Byte offset in the payload.
        //! @param [in] value The value to set in the payload.
        //! @param [in] recompute_crc If true, immediately recompute the CRC32 of the section.
        //!
        void setUInt16(size_t offset, uint16_t value, bool recompute_crc = true);

        //!
        //! Set the source PID.
        //! @param [in] pid The source PID.
        //!
        void setSourcePID(PID pid)
        {
            _source_pid = pid;
        }

        //!
        //! Index of first TS packet of the section in the demultiplexed stream.
        //! Usually valid only if the section was extracted by a demux.
        //! @return The first TS packet of the section in the demultiplexed stream.
        //!
        PacketCounter getFirstTSPacketIndex() const
        {
            return _first_pkt;
        }

        //!
        //! Index of last TS packet of the section in the demultiplexed stream.
        //! Usually valid only if the section was extracted by a demux.
        //! @return The last TS packet of the section in the demultiplexed stream.
        //!
        PacketCounter getLastTSPacketIndex() const
        {
            return _last_pkt;
        }

        //!
        //! Set the first TS packet of the section in the demultiplexed stream.
        //! @param [in] i The first TS packet of the section in the demultiplexed stream.
        //!
        void setFirstTSPacketIndex(PacketCounter i)
        {
            _first_pkt = i;
        }

        //!
        //! Set the last TS packet of the section in the demultiplexed stream.
        //! @param [in] i The last TS packet of the section in the demultiplexed stream.
        //!
        void setLastTSPacketIndex(PacketCounter i)
        {
            _last_pkt = i;
        }

        //!
        //! This method recomputes and replaces the CRC32 of the section.
        //!
        void recomputeCRC();

        //!
        //! Check if the section has a "diversified" payload.
        //! A payload is "diversified" if its size is 2 bytes or more and if
        //! it contains at least 2 different byte values (not all 0x00 or not
        //! all 0xFF for instance).
        //! @return True if the payload is diversified.
        //!
        bool hasDiversifiedPayload() const;

        //!
        //! Read a section from standard streams (binary mode).
        //! @param [in,out] strm A standard stream in input mode.
        //! If a section is invalid (eof before end of section, wrong crc),
        //! the failbit of the stream is set.
        //! @param [in] crc_op How to process the CRC32 of the input packet.
        //! @param [in,out] report Where to report errors.
        //! @return A reference to the @a strm object.
        //!
        std::istream& read(std::istream& strm, CRC32::Validation crc_op = CRC32::IGNORE, Report& report = CERR);

        //!
        //! Write a section to standard streams (binary mode).
        //! @param [in,out] strm A standard stream in output mode.
        //! @param [in,out] report Where to report errors.
        //! @return A reference to the @a strm object.
        //!
        std::ostream& write(std::ostream& strm, Report& report = CERR) const;

        //!
        //! Hexa dump the section on an output stream without interpretation of the payload.
        //! @param [in,out] strm A standard stream in output mode (text mode).
        //! @param [in] indent Indicates the base indentation of lines.
        //! @param [in] cas CAS family, for CAS-specific information.
        //! @param [in] no_header If true, do not display the section header.
        //! @return A reference to the @a strm object.
        //!
        std::ostream& dump(std::ostream& strm, int indent = 0, CASFamily cas = CAS_OTHER, bool no_header = false) const;

        //!
        //! Static method to compute a section size.
        //! @param [in] content Address of the binary section data.
        //! @param [in] content_size Size in bytes of the buffer containing the section and possibly trailing additional data.
        //! @return The total size in bytes of the section starting at @a content or zero on error.
        //!
        static size_t SectionSize(const void* content, size_t content_size);

        //!
        //! Static method to compute a section size.
        //! @param [in] content Buffer containing the section and possibly trailing additional data.
        //! @return The total size in bytes of the section starting in @a content or zero on error.
        //!
        static size_t SectionSize(const ByteBlock& content) { return SectionSize(content.data(), content.size()); }

        //!
        //! Static method to compute the minimum number of TS packets required to transport a set of sections.
        //! @tparam CONTAINER A container class of SectionPtr as defined by the C++ Standard Template Library (STL).
        //! @param [in] container A container class of SectionPtr.
        //! @param [in] pack If true, assume that sections are packed in TS packets.
        //! When false, assume that each section starts at the beginning of a TS packet
        //! and stuffing in applied at the end of each section.
        //! @return The minimum number of TS packets required to transport the sections in @a container.
        //!
        template <class CONTAINER>
        static PacketCounter PacketCount(const CONTAINER& container, bool pack = true);

    private:
        // Private fields
        bool          _is_valid;    // Content of *_data is a valid section
        PID           _source_pid;  // Source PID (informational)
        PacketCounter _first_pkt;   // Index of first packet in stream (informational)
        PacketCounter _last_pkt;    // Index of last packet in stream (informational)
        ByteBlockPtr  _data;        // Full binary content of the section

        // Helpers for constructors
        void initialize(PID);
        void initialize(const ByteBlockPtr&, PID, CRC32::Validation);

        // Inaccessible operations
        Section(const Section&) = delete;
    };
}

#include "tsSectionTemplate.h"
