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
//!  Display PSI/SI tables.
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsTablesDisplayArgs.h"
#include "tsBinaryTable.h"
#include "tsSection.h"
#include "tsDescriptor.h"
#include "tsDescriptorList.h"

namespace ts {
    //!
    //! A class to display PSI/SI tables.
    //! @ingroup mpeg
    //!
    class TSDUCKDLL TablesDisplay
    {
    public:
        //!
        //! Constructor.
        //! By default, all displays are done on @c std::cout.
        //! Use redirect() to redirect the output to a file.
        //! @param [in] options Table logging options.
        //! @param [in,out] report Where to log errors.
        //!
        TablesDisplay(const TablesDisplayArgs& options, Report& report);

        //!
        //! Virtual destructor.
        //!
        virtual ~TablesDisplay() {}

        //!
        //! Display a table on the output stream.
        //! The content of the table is interpreted according to the table id.
        //! @param [in] table The table to display.
        //! @param [in] indent Indentation width.
        //! @param [in] cas CAS family of the table. If different from CAS_OTHER, override the CAS family in TablesDisplayArgs.
        //! @return A reference to the output stream.
        //!
        virtual std::ostream& displayTable(const BinaryTable& table, int indent = 0, CASFamily cas = CAS_OTHER);

        //!
        //! Display a section on the output stream.
        //! The content of the table is interpreted according to the table id.
        //! @param [in] section The section to display.
        //! @param [in] indent Indentation width.
        //! @param [in] cas CAS family of the table. If different from CAS_OTHER, override the CAS family in TablesDisplayArgs.
        //! @param [in] no_header If true, do not display the section header.
        //! @return A reference to the output stream.
        //!
        virtual std::ostream& displaySection(const Section& section, int indent = 0, CASFamily cas = CAS_OTHER, bool no_header = false);

        //!
        //! Display the payload of a section on the output stream.
        //! The content of the table is interpreted according to the table id.
        //! @param [in] section The section to display.
        //! @param [in] indent Indentation width.
        //! @param [in] cas CAS family of the table. If different from CAS_OTHER, override the CAS family in TablesDisplayArgs.
        //! @return A reference to the output stream.
        //!
        virtual std::ostream& displaySectionData(const Section& section, int indent = 0, CASFamily cas = CAS_OTHER);

        //!
        //! Display the payload of a section on the output stream as a one-line "log" message.
        //! @param [in] section The section to display.
        //! @param [in] header Header string to display as prefix on the line.
        //! @param [in] max_bytes Maximum number of bytes to log from the section. 0 means unlimited.
        //! @param [in] cas CAS family of the table. If different from CAS_OTHER, override the CAS family in TablesDisplayArgs.
        //! @return A reference to the output stream.
        //!
        virtual std::ostream& logSectionData(const Section& section,
                                             const UString& header = UString(),
                                             size_t max_bytes = 0,
                                             CASFamily cas = CAS_OTHER);

        //!
        //! Display a descriptor on the output stream.
        //! @param [in] desc The descriptor to display.
        //! @param [in] indent Indentation width.
        //! @param [in] tid Table id of table containing the descriptors.
        //! This is optional. Used by some descriptors the interpretation of which may
        //! vary depending on the table that they are in.
        //! @param [in] pds Private Data Specifier. Used to interpret private descriptors.
        //! @param [in] cas CAS family of the table. If different from CAS_OTHER, override the CAS family in TablesDisplayArgs.
        //! @return A reference to the output stream.
        //!
        virtual std::ostream& displayDescriptor(const Descriptor& desc,
                                               int indent = 0,
                                               TID tid = TID_NULL,
                                               PDS pds = 0,
                                               CASFamily cas = CAS_OTHER);

        //!
        //! Display the payload of a descriptor on the output stream.
        //! @param [in] did Descriptor id.
        //! @param [in] payload Address of the descriptor payload.
        //! @param [in] size Size in bytes of the descriptor payload.
        //! @param [in] indent Indentation width.
        //! @param [in] tid Table id of table containing the descriptors.
        //! This is optional. Used by some descriptors the interpretation of which may
        //! vary depending on the table that they are in.
        //! @param [in] pds Private Data Specifier. Used to interpret private descriptors.
        //! @param [in] cas CAS family of the table. If different from CAS_OTHER, override the CAS family in TablesDisplayArgs.
        //! @return A reference to the output stream.
        //!
        virtual std::ostream& displayDescriptorData(DID did,
                                                    const uint8_t* payload,
                                                    size_t size,
                                                    int indent = 0,
                                                    TID tid = TID_NULL,
                                                    PDS pds = 0,
                                                    CASFamily cas = CAS_OTHER);

        //!
        //! Display a list of descriptors from a memory area
        //! @param [in] data Address of the descriptor list.
        //! @param [in] size Size in bytes of the descriptor list.
        //! @param [in] indent Indentation width.
        //! @param [in] tid Table id of table containing the descriptors.
        //! This is optional. Used by some descriptors the interpretation of which may
        //! vary depending on the table that they are in.
        //! @param [in] pds Private Data Specifier. Used to interpret private descriptors.
        //! @param [in] cas CAS family of the table. If different from CAS_OTHER, override the CAS family in TablesDisplayArgs.
        //! @return A reference to the output stream.
        //!
        virtual std::ostream& displayDescriptorList(const void* data,
                                                    size_t size,
                                                    int indent = 0,
                                                    TID tid = TID_NULL,
                                                    PDS pds = 0,
                                                    CASFamily cas = CAS_OTHER);

        //!
        //! Display a list of descriptors.
        //! @param [in] list Descriptor list.
        //! @param [in] indent Indentation width.
        //! @param [in] tid Table id of table containing the descriptors.
        //! This is optional. Used by some descriptors the interpretation of which may
        //! vary depending on the table that they are in.
        //! @param [in] pds Private Data Specifier. Used to interpret private descriptors.
        //! @param [in] cas CAS family of the table. If different from CAS_OTHER, override the CAS family in TablesDisplayArgs.
        //! @return A reference to the output stream.
        //!
        virtual std::ostream& displayDescriptorList(const DescriptorList& list,
                                                    int indent = 0,
                                                    TID tid = TID_NULL,
                                                    PDS pds = 0,
                                                    CASFamily cas = CAS_OTHER);

        //!
        //! A utility method to dump extraneous bytes after expected data.
        //! @param [in] data Address of extra data to dump.
        //! @param [in] size Size of extra data to dump.
        //! @param [in] indent Indentation width.
        //! @return A reference to the output stream.
        //!
        virtual std::ostream& displayExtraData(const void *data, size_t size, int indent = 0);

        //!
        //! A utility method to interpret data as an ASCII string.
        //! @param [in] data Address of data.
        //! @param [in] size Size of data.
        //! @return If all bytes in data are ASCII (optioanlly padded with zeroes), return the
        //! equivalent ASCII string. Otherwise, return an empty string.
        //!
        static std::string ToASCII(const void *data, size_t size);

        //!
        //! A utility method to display data if it can be interpreted as an ASCII string.
        //! @param [in] data Address of data.
        //! @param [in] size Size of data.
        //! @param [in] prefix To print before the ASCII data.
        //! @param [in] suffix To print after the ASCII data.
        //! @return A reference to the output stream.
        //!
        virtual std::ostream& displayIfASCII(const void *data, size_t size, const UString& prefix = UString(), const UString& suffix = UString());

        //!
        //! Redirect the output stream to a file.
        //! The previous file is closed.
        //! @param [in] file_name The file name to create. If empty, reset to @c std::cout.
        //! @return True on success, false on error.
        //!
        virtual bool redirect(const UString& file_name = UString());

        //!
        //! Get the current output stream.
        //! @return A reference to the output stream.
        //!
        std::ostream& out();

        //!
        //! Get the default DVB character set for DVB strings without table code.
        //! @return The default DVB character set.
        //!
        const DVBCharset* dvbCharset() const
        {
            return _opt.default_charset;
        }

        //!
        //! Get the current output report.
        //! @return A reference to the current output report.
        //!
        Report& report()
        {
            return _report;
        }

        //!
        //! Flush the text output.
        //!
        virtual void flush();

    protected:
        //!
        //! Display the content of an unknown section.
        //! The command-line formatting options are used to analyze the content.
        //! @param [in] section The section to display.
        //! @param [in] indent Indentation width.
        //!
        void displayUnkownSectionData(const ts::Section& section, int indent);

        //!
        //! Display the content of an unknown descriptor.
        //! @param [in] did Descriptor id.
        //! @param [in] payload Address of the descriptor payload.
        //! @param [in] size Size in bytes of the descriptor payload.
        //! @param [in] indent Indentation width.
        //! @param [in] tid Table id of table containing the descriptors.
        //! @param [in] pds Private Data Specifier. Used to interpret private descriptors.
        //!
        void displayUnkownDescriptor(DID did, const uint8_t* payload, size_t size, int indent, TID tid, PDS pds);

        //!
        //! The actual CAS family to use.
        //! @param [in] cas CAS family of the table. If different from CAS_OTHER, can be overridden by subclass.
        //! @return The actual CAS family to use.
        //!
        virtual CASFamily casFamily(CASFamily cas) const;

        //!
        //! The actual private data specifier to use.
        //! @param [in] pds Current PDS, typically from a private_data_specifier_descriptor.
        //! @return The actual PDS to use.
        //!
        virtual PDS actualPDS(PDS pds) const;

        //!
        //! Display a memory area containing a list of TLV records.
        //!
        //! The displayed area extends from @a data to @a data + @a tlvStart + @a tlvSize.
        //! - From @a data to @a data + @a tlvStart : Raw data.
        //! - From @a data + @a tlvStart to @a data + @a tlvStart + @a tlvSize : TLV records.
        //!
        //! @param [in] data Starting address of memory area.
        //! @param [in] tlvStart Starting index of TLV records after @a data.
        //! @param [in] tlvSize Size in bytes of the TLV area.
        //! @param [in] dataOffset Display offset of @a data.
        //! @param [in] indent Left margin size.
        //! @param [in] innerIndent Inner margin size.
        //! @param [in] tlv TLV syntax.
        //!
        void displayTLV(const uint8_t* data,
                        size_t tlvStart,
                        size_t tlvSize,
                        size_t dataOffset,
                        int indent,
                        int innerIndent,
                        const TLVSyntax& tlv);

    private:
        const TablesDisplayArgs& _opt;
        Report&                  _report;
        std::ofstream            _outfile;
        bool                     _use_outfile;

        // Inaccessible operations.
        TablesDisplay() = delete;
        TablesDisplay(const TablesDisplay&) = delete;
        TablesDisplay& operator=(const TablesDisplay&) = delete;
    };
}
