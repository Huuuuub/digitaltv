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
//!  This class maps PID's with CA system ids.
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsSectionDemux.h"
#include "tsCADescriptor.h"

namespace ts {
    //!
    //! This class maps PID's with CA system ids.
    //! @ingroup mpeg
    //!
    //! All packets are passed through this object. It tracks the location of all
    //! EMM and ECM PID's and records the corresponding CAS attributes.
    //!
    class TSDUCKDLL CASMapper : private TableHandlerInterface
    {
    public:
        //!
        //! Constructor.
        //! @param [in,out] report Where to log errors.
        //!
        CASMapper(Report& report);

        //!
        //! This method feeds the CAS mapper with a TS packet.
        //! @param [in] pkt A new transport stream packet.
        //!
        void feedPacket(const TSPacket& pkt)
        {
            _demux.feedPacket(pkt);
        }

        //!
        //! Filter PSI tables based on current/next indicator.
        //! @param [in] current Use "current" tables. This is true by default.
        //! @param [in] next Use "next" tables. This is false by default.
        //!
        void setCurrentNext(bool current, bool next)
        {
            _demux.setCurrentNext(current, next);
        }

        //!
        //! Check if a PID is a known CA PID.
        //! @param [in] pid A PID to check.
        //! @return True if @ pid is a known ECM or EMM PID.
        //!
        bool knownPID(PID pid) const
        {
            return _pids.find(pid) != _pids.end();
        }

        //!
        //! Get the CAS family of a CA PID (ECM or EMM).
        //! @param [in] pid A PID to check.
        //! @return The CAS family or CAS_OTHER if unknown.
        //!
        CASFamily casFamily(PID pid) const;

        //!
        //! Get the CAS id of a CA PID (ECM or EMM).
        //! @param [in] pid A PID to check.
        //! @return The CAS id or zero if the PID is not known.
        //!
        uint16_t casId(PID pid) const;

        //!
        //! Check if a PID carries ECM's.
        //! @param [in] pid A PID to check.
        //! @return True if the PID carries ECM's, false otherwise.
        //!
        bool isECM(PID pid) const;

        //!
        //! Check if a PID carries EMM's.
        //! @param [in] pid A PID to check.
        //! @return True if the PID carries EMM's, false otherwise.
        //!
        bool isEMM(PID pid) const;

        //!
        //! Get the CA_descriptor which describes a CA PID (ECM or EMM).
        //! @param [in] pid A PID to check.
        //! @param [out] desc A safe pointer to the associated CA_descriptor.
        //! @return True if the CA_descriptor was found, false otherwise.
        //!
        bool getCADescriptor(PID pid, CADescriptorPtr& desc) const;

    private:
        // Description of one CA PID.
        class PIDDescription
        {
        public:
            uint16_t        cas_id;  //!< CA system id.
            bool            is_ecm;  //!< True for ECM, false for EMM.
            CADescriptorPtr ca_desc; //!< Corresponding CA descriptor.

            PIDDescription(uint16_t cas_id_ = 0, bool is_ecm_ = false, const CADescriptorPtr& ca_desc_ = CADescriptorPtr()) :
                cas_id(cas_id_),
                is_ecm(is_ecm_),
                ca_desc(ca_desc_)
            {
            }
        };

        // Map of key=PID to value=PIDDescription.
        typedef std::map<PID,PIDDescription> PIDDescriptionMap;

        // Explore a descriptor list and record EMM and ECM PID's.
        void analyzeCADescriptors(const DescriptorList& descs, bool is_ecm);

        // CAMapper private fields.
        Report&           _report;
        SectionDemux      _demux;
        PIDDescriptionMap _pids;

        // Hooks
        virtual void handleTable(SectionDemux&, const BinaryTable&) override;

        // Inaccessible operations.
        CASMapper() = delete;
        CASMapper(const CASMapper&) = delete;
        CASMapper& operator=(const CASMapper&) = delete;
    };
}
