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
//!  Representation of an IP/MAC Notification Table (INT).
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsAbstractLongTable.h"
#include "tsDescriptorList.h"

namespace ts {
    //!
    //! Representation of an IP/MAC Notification Table (INT).
    //! @see ETSI EN 301 192, section 8.4.3.
    //! @ingroup table
    //!
    class TSDUCKDLL INT : public AbstractLongTable
    {
    public:
        //!
        //! Description of a device.
        //!
        struct TSDUCKDLL Device : public EntryBase
        {
            DescriptorList target_descs;       //!< Target descriptor loop, describes the target device.
            DescriptorList operational_descs;  //!< Operational descriptor loop, describes the operations on the target device.

            //!
            //! Basic constructor.
            //! @param [in] table Parent table. A descriptor list is always attached to a table.
            //!
            explicit Device(const AbstractTable* table);

            //!
            //! Basic copy-like constructor.
            //! @param [in] table Parent table. A descriptor list is always attached to a table.
            //! @param [in] other Another instance to copy.
            //!
            Device(const AbstractTable* table, const Device& other);

            //!
            //! Assignment operator.
            //! The parent table remains unchanged.
            //! @param [in] other Another instance to copy.
            //! @return A reference to this object.
            //!
            Device& operator=(const Device& other);

        private:
            // Inaccessible operations.
            Device() = delete;
            Device(const Device&) = delete;
        };

        //!
        //! List of devices.
        //!
        typedef EntryWithDescriptorsList<Device> DeviceList;

        // INT public members:
        uint8_t        action_type;       //!< Action type.
        uint32_t       platform_id;       //!< Platform id, 24 bits.
        uint8_t        processing_order;  //!< Processing order code.
        DescriptorList platform_descs;    //!< Platforma descriptor loop.
        DeviceList     devices;           //!< List of device descriptions.

        //!
        //! Default constructor.
        //! @param [in] version Table version number.
        //! @param [in] is_current True if table is current, false if table is next.
        //!
        INT(uint8_t version = 0, bool is_current = true);

        //!
        //! Copy constructor.
        //! @param [in] other Other instance to copy.
        //!
        INT(const INT& other);

        //!
        //! Constructor from a binary table.
        //! @param [in] table Binary table to deserialize.
        //! @param [in] charset If not zero, character set to use without explicit table code.
        //!
        INT(const BinaryTable& table, const DVBCharset* charset = nullptr);

        // Inherited methods
        virtual void serialize(BinaryTable& table, const DVBCharset* = nullptr) const override;
        virtual void deserialize(const BinaryTable& table, const DVBCharset* = nullptr) override;
        virtual void buildXML(xml::Element*) const override;
        virtual void fromXML(const xml::Element*) override;
        DeclareDisplaySection();

    private:
        // Add a new section to a table being serialized
        // Section number is incremented. Data and remain are reinitialized.
        void addSection(BinaryTable& table, int& section_number, uint8_t* payload, uint8_t*& data, size_t& remain) const;

        // Serialize one device description. Update data and remain.
        // Return true if the service was completely serialized, false otherwise.
        bool serializeDevice(const Device& device, uint8_t*& data, size_t& remain) const;

        // Deserialize a descriptor list. Update data and remain. Return true on success.
        static bool GetDescriptorList(DescriptorList& dlist, const uint8_t*& data, size_t& remain);

        // Display a descriptor list. Update data and remain. Return true on success.
        static bool DisplayDescriptorList(TablesDisplay& display, TID tid, const uint8_t*& data, size_t& remain, int indent);
    };
}
