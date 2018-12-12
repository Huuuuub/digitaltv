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
//!  Representation of an extended_event_descriptor
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsAbstractDescriptor.h"
#include "tsDescriptorList.h"

namespace ts {
    //!
    //! Representation of a extended_event_descriptor.
    //! @see ETSI 300 468, 6.2.15.
    //! @ingroup descriptor
    //!
    class TSDUCKDLL ExtendedEventDescriptor : public AbstractDescriptor
    {
    public:
        // Item entry
        struct Entry;

        //!
        //! A list of item entries.
        //!
        typedef std::list<Entry> EntryList;

        // Public members
        uint8_t   descriptor_number;      //!< See ETSI 300 468, 6.2.15.
        uint8_t   last_descriptor_number; //!< See ETSI 300 468, 6.2.15.
        UString   language_code;          //!< ISO-639 language code, 3 characters.
        EntryList entries;                //!< The list of item entries.
        UString   text;                   //!< See ETSI 300 468, 6.2.15.

        //!
        //! Default constructor.
        //!
        ExtendedEventDescriptor();

        //!
        //! Constructor from a binary descriptor
        //! @param [in] bin A binary descriptor to deserialize.
        //! @param [in] charset If not zero, character set to use without explicit table code.
        //!
        ExtendedEventDescriptor(const Descriptor& bin, const DVBCharset* charset = nullptr);

        //!
        //! Split into several descriptors if neccesary and add them in a descriptor list.
        //!
        //! Split the content into several ExtendedEventDescriptor if the content
        //! is too long and add them in a descriptor list.
        //! @param [in,out] dlist List of descriptors.
        //! @param [in] charset If not zero, default character set to use.
        //!
        void splitAndAdd(DescriptorList& dlist, const DVBCharset* charset = nullptr) const;

        //!
        //! Normalize all ExtendedEventDescriptor in a descriptor list.
        //! Update all descriptor_number and last_descriptor_number per language.
        //! @param [in,out] desc_list_addr Address of a serialized descriptor list.
        //! @param [in] desc_list_size Descriptor list size in bytes.
        //! @param [in] charset If not zero, character set to use without explicit table code.
        //!
        static void NormalizeNumbering(uint8_t* desc_list_addr, size_t desc_list_size, const DVBCharset* charset = nullptr);

        // Inherited methods
        virtual void serialize(Descriptor&, const DVBCharset* = nullptr) const override;
        virtual void deserialize(const Descriptor&, const DVBCharset* = nullptr) override;
        virtual void buildXML(xml::Element*) const override;
        virtual void fromXML(const xml::Element*) override;
        DeclareDisplayDescriptor();

        //!
        //! An item entry.
        //!
        struct TSDUCKDLL Entry
        {
            // Public members
            UString item_description;  //!< Item description or name.
            UString item;              //!< Item text content.

            //!
            //! Constructor.
            //! @param [in] desc_ Item description or name.
            //! @param [in] item_ Item text content.
            //!
            Entry(const UString& desc_ = UString(), const UString& item_ = UString()) : item_description(desc_), item(item_) {}
        };
    };
}
