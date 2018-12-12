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
//!  Transport stream scrambling using multiple algorithms.
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsArgs.h"
#include "tsCerrReport.h"
#include "tsTSPacket.h"
#include "tsDVBCSA2.h"
#include "tsIDSA.h"
#include "tsMPEG.h"

namespace ts {
    //!
    //! Transport stream scrambling using multiple algorithms.
    //! @ingroup mpeg
    //!
    //! Include command line arguments processing.
    //!
    //! The scrambling type is indicated by a constant as present in a scrambling_descriptor.
    //! Currently, only SCRAMBLING_DVB_CSA2 and SCRAMBLING_ATIS_IIF_IDSA are supported.
    //!
    //! With fixed control words from the command line:
    //! - For encryption, the next key is used each time setEncryptParity() is called
    //!   with a new parity.
    //! - For decryption, the next key is used each time a new scrambling_control
    //!   value is found in a TS header.
    //!
    class TSDUCKDLL TSScrambling
    {
    public:
        //!
        //! Default constructor.
        //! @param [in,out] report Where to report error and information.
        //! @param [in] scrambling Scrambling type.
        //!
        TSScrambling(Report& report = CERR, uint8_t scrambling = SCRAMBLING_DVB_CSA2);

        //!
        //! Copy constructor.
        //! @param [in] other Other instance to copy. Only the configuration parameters, typically
        //! from the command line, are copied. The state of @a other is not copied.
        //!
        TSScrambling(const TSScrambling& other);

        //!
        //! Define command line options in an Args.
        //! @param [in,out] args Command line arguments to update.
        //!
        void defineOptions(Args& args) const;

        //!
        //! Load arguments from command line.
        //! @param [in,out] args Command line arguments.
        //! @return True on success, false on error.
        //! Args error indicator is also set in case of incorrect arguments.
        //!
        bool loadArgs(Args& args);

        //!
        //! Check if fixed control words were loaded from the command line.
        //! @return True if this object uses fixed control words.
        //!
        bool hasFixedCW() const { return !_cw_list.empty(); }

        //!
        //! Get the number of fixed control words from the command line.
        //! @return Number of fixed control words from the command line.
        //!
        size_t fixedCWCount() const { return _cw_list.size(); }

        //!
        //! Restart the list of fixed control words from the beginning.
        //! Ignored if no control words were loaded from the command line.
        //!
        void rewindFixedCW();

        //!
        //! Get the scrambling algorithm name.
        //! @return The scrambling algorithm name.
        //!
        UString algoName() const { return _scrambler[0]->name(); }

        //!
        //! Get the required control word size in bytes.
        //! @return The required control word size in bytes.
        //!
        size_t cwSize() const { return _scrambler[0]->minKeySize(); }

        //!
        //! Force the usage of a given algorithm.
        //! @param [in] scrambling Scrambling type.
        //! @param [in] overrideExplicit If true, always set the scrambling type.
        //! If false, ignore it if an explicit type was set on the command line.
        //! @return True on success, false on unsupported type.
        //!
        bool setScramblingType(uint8_t scrambling, bool overrideExplicit = true);

        //!
        //! Get the current scrambling algorithm.
        //! @return The scrambling type.
        //!
        uint8_t scramblingType() { return _scrambling_type; }

        //!
        //! Check if a scrambling algorithm was specified on the command line.
        //! @return Tue if a scrambling algorithm was specified on the command line.
        //!
        bool explicitScramblingType() const { return _explicit_type; }

        //!
        //! Force the entropy mode of DVB-CSA2.
        //! By default, use settings from the command line.
        //! @param [in] mode DVB-CSA2 entropy mode.
        //!
        void setEntropyMode(DVBCSA2::EntropyMode mode);

        //!
        //! Set the control word for encrypt and decrypt.
        //! @param [in] cw The control word to use.
        //! @param [in] parity Use the parity of this integer value (odd or even).
        //! @return True on success, false on error.
        //!
        bool setCW(const ByteBlock& cw, int parity);

        //!
        //! Set the parity of all subsequent encryptions.
        //! @param [in] parity Use the parity of this integer value (odd or even).
        //! @return True on success, false on error (error setting next fixed CW, if any).
        //!
        bool setEncryptParity(int parity);

        //!
        //! Encrypt a TS packet with the current parity and corresponding CW.
        //! @param [in,out] pkt The packet to encrypt.
        //! @return True on success, false on error. An already encrypted packet is an error.
        //!
        bool encrypt(TSPacket& pkt);

        //!
        //! Decrypt a TS packet with the CW corresponding to the parity in the packet.
        //! @param [in,out] pkt The packet to decrypt.
        //! @return True on success, false on error. A clear packet is not an error.
        //!
        bool decrypt(TSPacket& pkt);

    private:
        // List of control words
        typedef std::list<ByteBlock> CWList;

        Report&          _report;
        uint8_t          _scrambling_type;
        bool             _explicit_type;
        CWList           _cw_list;
        CWList::iterator _next_cw;
        uint8_t          _encrypt_scv;  // Encryption: key to use (SC_EVEN_KEY or SC_ODD_KEY).
        uint8_t          _decrypt_scv;  // Decryption: previous scrambling_control value.
        DVBCSA2          _dvbcsa[2];    // Index 0 = even key, 1 = odd key.
        IDSA             _idsa[2];
        CipherChaining*  _scrambler[2];

        // Set the next fixed control word as scrambling key.
        bool setNextFixedCW(int parity);

        // Inaccessible operations.
        TSScrambling& operator=(const TSScrambling&) = delete;
    };
}
