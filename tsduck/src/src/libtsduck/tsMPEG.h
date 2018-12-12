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
//!  @ingroup mpeg
//!  Common definition for MPEG level.
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsPlatform.h"
#include "tsEnumeration.h"

namespace ts {

    // Base types

    typedef uint16_t PID;      //!< PID value.
    typedef uint8_t  TID;      //!< Table identifier.
    typedef uint8_t  DID;      //!< Descriptor identifier.
    typedef uint32_t PDS;      //!< Private data specifier.
    typedef uint32_t BitRate;  //!< Bitrate in bits/second.

    //!
    //! MPEG TS packet size in bytes.
    //!
    const size_t PKT_SIZE = 188;

    //!
    //! MPEG TS packet size in bits.
    //!
    const size_t PKT_SIZE_BITS = 8 * PKT_SIZE;

    //!
    //! Size in bytes of a Reed-Solomon outer FEC.
    //!
    const size_t RS_SIZE = 16;

    //!
    //! Size in bytes of a TS packet with trailing Reed-Solomon outer FEC.
    //!
    const size_t PKT_RS_SIZE = PKT_SIZE + RS_SIZE;

    //!
    //! Size in bytes of a timestamp preceeding a TS packet in M2TS files (Blu-ray disc).
    //!
    const size_t M2TS_HEADER_SIZE = 4;

    //!
    //! Size in bytes of an TS packet in M2TS files (Blu-ray disc).
    //! There is a leading 4-byte timestamp before the TS packet.
    //!
    const size_t PKT_M2TS_SIZE = M2TS_HEADER_SIZE + PKT_SIZE;

    //!
    //! Number of Transport Stream packets.
    //!
    //! TS packets are counted using 64-bit integers.
    //! Thus, PacketCounter will never overflow: at 100 Mb/s, 2^64 188-byte
    //! packets will take 8.7 million years to transmit. No process will
    //! ever run that long. On the contrary, using 32-bit integer would
    //! be insufficient: at 100 Mb/s, 2^32 188-byte packets will take
    //! only 17 hours to transmit.
    //!
    typedef uint64_t PacketCounter;

    //!
    //! A impossible value for PacketCounter, meaning "undefined".
    //!
    const PacketCounter INVALID_PACKET_COUNTER = std::numeric_limits<PacketCounter>::max();

    //!
    //! Number of sections.
    //!
    typedef uint64_t SectionCounter;

    //!
    //! Convert 188-byte packet bitrate into 204-byte packet bitrate.
    //! @param [in] bitrate188 Bitrate using 188-byte packet as reference.
    //! @return Corresponding bitrate using 204-byte packet as reference.
    //!
    TSDUCKDLL inline BitRate ToBitrate204(BitRate bitrate188)
    {
        return BitRate((uint64_t (bitrate188) * 204L) / 188L);
    }

    //!
    //! Convert 204-byte packet bitrate into 188-byte packet bitrate.
    //! @param [in] bitrate204 Bitrate using 204-byte packet as reference.
    //! @return Corresponding bitrate using 188-byte packet as reference.
    //!
    TSDUCKDLL inline BitRate ToBitrate188(BitRate bitrate204)
    {
        return BitRate((uint64_t (bitrate204) * 188L) / 204L);
    }

    //!
    //! Compute the interval, in milliseconds, between two packets.
    //! @param [in] bitrate TS bitrate in bits/second, based on 188-byte packets.
    //! @param [in] distance Distance between the two packets: 0 for the same
    //! packet, 1 for the next packet (the default), etc.
    //! @return Interval in milliseconds between the first byte of the first packet
    //! and the first byte of the second packet.
    //!
    TSDUCKDLL inline MilliSecond PacketInterval(BitRate bitrate, PacketCounter distance = 1)
    {
        return bitrate == 0 ? 0 : (distance * 8 * PKT_SIZE * MilliSecPerSec) / MilliSecond(bitrate);
    }

    //!
    //! Compute the number of packets transmitted during a given duration in milliseconds.
    //! @param [in] bitrate TS bitrate in bits/second, based on 188-byte packets.
    //! @param [in] duration Number of milliseconds.
    //! @return Number of packets during @a duration milliseconds.
    //!
    TSDUCKDLL inline PacketCounter PacketDistance(BitRate bitrate, MilliSecond duration)
    {
        return (PacketCounter (bitrate) * (duration >= 0 ? duration : -duration)) / (MilliSecPerSec * 8 * PKT_SIZE);
    }

    //!
    //! Compute the bitrate from a number of packets transmitted during a given duration in milliseconds.
    //! @param [in] packets Number of packets during @a duration milliseconds.
    //! @param [in] duration Number of milliseconds.
    //! @return TS bitrate in bits/second, based on 188-byte packets.
    //!
    TSDUCKDLL inline BitRate PacketBitRate(PacketCounter packets, MilliSecond duration)
    {
        return duration == 0 ? 0 : BitRate((packets * 8 * PKT_SIZE * MilliSecPerSec) / duration);
    }

    //!
    //! Compute the minimum number of TS packets required to transport a section.
    //! @param [in] section_size Total section size in bytes.
    //! @return Number of packets required for the section.
    //!
    TSDUCKDLL inline PacketCounter SectionPacketCount(size_t section_size)
    {
        // The required size for a section is section_size + 1 (1 for pointer_field
        // in first packet). In each packet, the useable size is 184 bytes.
        return PacketCounter((section_size + 184) / 184);
    }

    //!
    //! Value of a sync byte (first byte in a TS packet).
    //!
    const uint8_t SYNC_BYTE = 0x47;

    //!
    //! PES packet start code prefix (24 bits).
    //!
    const uint32_t PES_START = 0x000001;

    //!
    //! Size (in bits) of a PID field.
    //!
    const size_t PID_BITS = 13;

    //!
    //! Maximum number of PID's (8192).
    //!
    const PID PID_MAX = 1 << PID_BITS;

    //!
    //! A bit mask for PID values.
    //! Useful to implement PID filtering.
    //!
    typedef std::bitset <PID_MAX> PIDSet;

    //!
    //! PIDSet constant with no PID set.
    //!
    TSDUCKDLL extern const PIDSet NoPID;

    //!
    //! PIDSet constant with all PID's set.
    //!
    TSDUCKDLL extern const PIDSet AllPIDs;

    //!
    //! Size (in bits) of a Continuity Counter (CC) field.
    //!
    const size_t CC_BITS = 4;

    //!
    //! Mask to wrap a Continuity Counter (CC) value.
    //! CC values wrap at 16.
    //!
    const uint8_t CC_MASK = 0x0F;

    //!
    //! Maximum value of a Continuity Counter (CC).
    //!
    const uint8_t CC_MAX = 1 << CC_BITS;

    //!
    //! Size (in bits) of a section version field.
    //!
    const size_t SVERSION_BITS = 5;

    //!
    //! Mask to wrap a section version value.
    //! Section version values wrap at 32.
    //!
    const uint8_t SVERSION_MASK = 0x1F;

    //!
    //! Maximum value of a section version.
    //!
    const uint8_t SVERSION_MAX = 1 << SVERSION_BITS;

    //!
    //! Scrambling_control values (used in TS and PES packets headers)
    //!
    enum : uint8_t {
        SC_CLEAR        = 0,  //!< Not scrambled (MPEG-defined).
        SC_DVB_RESERVED = 1,  //!< Reserved for future use by DVB.
        SC_EVEN_KEY     = 2,  //!< Scrambled with even key (DVB-defined).
        SC_ODD_KEY      = 3   //!< Scrambled with odd key (DVB-defined).
    };

    //!
    //! Origin of Modified Julian Dates (MJD).
    //! The origin of MJD is 17 Nov 1858 00:00:00.
    //! The UNIX epoch (1 Jan 1970) is 40587 days from julian time origin.
    //!
    const uint32_t MJD_EPOCH = 40587;

    //!
    //! Video macroblock width in pixels.
    //! Valid for:
    //! - ISO 11172-2 (MPEG-1 video)
    //! - ISO 13818-2 (MPEG-2 video)
    //! - ISO 14496-10 (MPEG-4 Advanced Video Coding, AVC, ITU H.264)
    //!
    const size_t MACROBLOCK_WIDTH = 16;

    //!
    //! Video macroblock height in pixels.
    //! @see MACROBLOCK_WIDTH
    //!
    const size_t MACROBLOCK_HEIGHT = 16;

    //---------------------------------------------------------------------
    //! Predefined PID values
    //---------------------------------------------------------------------

    enum : PID {

        // Valid in all MPEG contexts:

        PID_PAT       = 0x0000, //!< PID for Program Association Table PAT
        PID_CAT       = 0x0001, //!< PID for Conditional Access Table
        PID_TSDT      = 0x0002, //!< PID for Transport Stream Description Table
        PID_MPEG_LAST = 0x000F, //!< Last reserved PID for MPEG.
        PID_NULL      = 0x1FFF, //!< PID for Null packets (stuffing)

        // Valid in DVB context:

        PID_NIT       = 0x0010, //!< PID for Network Information Table
        PID_SDT       = 0x0011, //!< PID for Service Description Table
        PID_BAT       = 0x0011, //!< PID for Bouquet Association Table
        PID_EIT       = 0x0012, //!< PID for Event Information Table
        PID_RST       = 0x0013, //!< PID for Running Status Table
        PID_TDT       = 0x0014, //!< PID for Time & Date Table
        PID_TOT       = 0x0014, //!< PID for Time Offset Table
        PID_NETSYNC   = 0x0015, //!< PID for Network synchronization
        PID_RNT       = 0x0016, //!< PID for TV-Anytime
        PID_INBSIGN   = 0x001C, //!< PID for Inband Signalling
        PID_MEASURE   = 0x001D, //!< PID for Measurement
        PID_DIT       = 0x001E, //!< PID for Discontinuity Information Table
        PID_SIT       = 0x001F, //!< PID for Selection Information Table
        PID_DVB_LAST  = 0x001F, //!< Last reserved PID for DVB.
    };

    //---------------------------------------------------------------------
    // MPEG clock representation:
    // - PCR (Program Clock Reference)
    // - PTS (Presentation Time Stamp)
    // - DTS (Decoding Time Stamp)
    //---------------------------------------------------------------------

    //!
    //! MPEG-2 System Clock frequency in Hz, used by PCR (27 Mb/s).
    //!
    const uint32_t SYSTEM_CLOCK_FREQ = 27000000;

    //!
    //! Subfactor of MPEG-2 System Clock subfrequency, used by PTS and DTS.
    //!
    const uint32_t SYSTEM_CLOCK_SUBFACTOR = 300;

    //!
    //! MPEG-2 System Clock subfrequency in Hz, used by PTS and DTS (90 Kb/s).
    //!
    const uint32_t SYSTEM_CLOCK_SUBFREQ = SYSTEM_CLOCK_FREQ / SYSTEM_CLOCK_SUBFACTOR;

    //!
    //! Size in bits of a PCR (Program Clock Reference).
    //!
    const size_t PCR_BIT_SIZE = 42;

    //!
    //! Size in bits of a PTS (Presentation Time Stamp) or DTS (Decoding Time Stamp).
    //!
    const size_t PTS_DTS_BIT_SIZE = 33;

    //!
    //! Mask for PCR values (wrap up at 2**42).
    //!
    const uint64_t PCR_MASK = TS_UCONST64(0x000003FFFFFFFFFF);

    //!
    //! Scale factor for PCR values (wrap up at 2**42).
    //!
    const uint64_t PCR_SCALE = TS_UCONST64(0x0000040000000000);

    //!
    //! Mask for PTS and DTS values (wrap up at 2**33).
    //!
    const uint64_t PTS_DTS_MASK = TS_UCONST64(0x00000001FFFFFFFF);

    //!
    //! Scale factor for PTS and DTS values (wrap up at 2**33).
    //!
    const uint64_t PTS_DTS_SCALE = TS_UCONST64(0x0000000200000000);

    //!
    //! An invalid PCR (Program Clock Reference) value, can be used as a marker.
    //!
    const uint64_t INVALID_PCR = TS_UCONST64(0xFFFFFFFFFFFFFFFF);

    //!
    //! An invalid PTS value, can be used as a marker.
    //!
    const uint64_t INVALID_PTS = TS_UCONST64(0xFFFFFFFFFFFFFFFF);

    //!
    //! An invalid DTS value, can be used as a marker.
    //!
    const uint64_t INVALID_DTS = TS_UCONST64(0xFFFFFFFFFFFFFFFF);

    //!
    //! Check if PCR2 follows PCR1 after wrap up.
    //! @param [in] pcr1 First PCR.
    //! @param [in] pcr2 Second PCR.
    //! @return True is @a pcr2 is probably following @a pcr1 after wrapping up at 2**42.
    //!
    TSDUCKDLL inline bool WrapUpPCR(uint64_t pcr1, uint64_t pcr2)
    {
        return pcr2 < pcr1 && (pcr1 - pcr2) > TS_UCONST64(0x000003F000000000);
    }

    //!
    //! Check if PTS2 follows PTS1 after wrap up.
    //! @param [in] pts1 First PTS.
    //! @param [in] pts2 Second PTS.
    //! @return True is @a pts2 is probably following @a pts1 after wrapping up at 2**33.
    //!
    TSDUCKDLL inline bool WrapUpPTS(uint64_t pts1, uint64_t pts2)
    {
        return pts2 < pts1 && (pts1 - pts2) > TS_UCONST64(0x00000001F0000000);
    }

    //!
    //! Check if two Presentation Time Stamp are in sequence.
    //!
    //! In MPEG video, B-frames are transported out-of-sequence.
    //! Their PTS is typically lower than the previous D-frame or I-frame
    //! in the transport. A "sequenced" PTS is one that is higher than
    //! the previous sequenced PTS (with possible wrap up).
    //! @param [in] pts1 First PTS.
    //! @param [in] pts2 Second PTS.
    //! @return True is @a pts2 is after @a pts1, possibly after wrapping up at 2**33.
    //!
    TSDUCKDLL inline bool SequencedPTS(uint64_t pts1, uint64_t pts2)
    {
        return pts1 <= pts2 || WrapUpPTS(pts1, pts2);
    }

    //---------------------------------------------------------------------
    //! Stream id values, as used in PES header.
    //---------------------------------------------------------------------

    enum : uint8_t {
        SID_PSMAP      = 0xBC, //!< Stream id for Program stream map
        SID_PRIV1      = 0xBD, //!< Stream id for Private stream 1
        SID_PAD        = 0xBE, //!< Stream id for Padding stream
        SID_PRIV2      = 0xBF, //!< Stream id for Private stream 2
        SID_AUDIO      = 0xC0, //!< Stream id for Audio stream, with number
        SID_AUDIO_MASK = 0x1F, //!< Stream id for Mask to get audio stream number
        SID_VIDEO      = 0xE0, //!< Stream id for Video stream, with number
        SID_VIDEO_MASK = 0x0F, //!< Stream id for Mask to get video stream number
        SID_ECM        = 0xF0, //!< Stream id for ECM stream
        SID_EMM        = 0xF1, //!< Stream id for EMM stream
        SID_DSMCC      = 0xF2, //!< Stream id for DSM-CC data
        SID_ISO13522   = 0xF3, //!< Stream id for ISO 13522 (hypermedia)
        SID_H222_1_A   = 0xF4, //!< Stream id for H.222.1 type A
        SID_H222_1_B   = 0xF5, //!< Stream id for H.222.1 type B
        SID_H222_1_C   = 0xF6, //!< Stream id for H.222.1 type C
        SID_H222_1_D   = 0xF7, //!< Stream id for H.222.1 type D
        SID_H222_1_E   = 0xF8, //!< Stream id for H.222.1 type E
        SID_ANCILLARY  = 0xF9, //!< Stream id for Ancillary stream
        SID_MP4_SLPACK = 0xFA, //!< Stream id for MPEG-4 SL-packetized stream
        SID_MP4_FLEXM  = 0xFB, //!< Stream id for MPEG-4 FlexMux stream
        SID_METADATA   = 0xFC, //!< Stream id for MPEG-7 metadata stream
        SID_EXTENDED   = 0xFD, //!< Stream id for Extended stream id
        SID_RESERVED   = 0xFE, //!< Stream id for Reserved value
        SID_PSDIR      = 0xFF, //!< Stream id for Program stream directory
    };

    //!
    //! Check if a stream id value indicates a video stream.
    //! @param [in] sid Stream id as found in a PES header.
    //! @return True if @a sid indicates a video stream.
    //!
    TSDUCKDLL inline bool IsVideoSID(uint8_t sid)
    {
        return (sid & ~SID_VIDEO_MASK) == SID_VIDEO;
    }

    //!
    //! Check if a stream id value indicates an audio stream.
    //! @param [in] sid Stream id as found in a PES header.
    //! @return True if @a sid indicates an audio stream.
    //!
    TSDUCKDLL inline bool IsAudioSID (uint8_t sid)
    {
        return (sid & ~SID_AUDIO_MASK) == SID_AUDIO;
    }

    //!
    //! Check if a stream id value indicates a PES packet with long header.
    //! @param [in] sid Stream id as found in a PES header.
    //! @return True if @a sid indicates a PES packet with long header.
    //!
    TSDUCKDLL bool IsLongHeaderSID(uint8_t sid);

    //---------------------------------------------------------------------
    //! PES start code values.
    //---------------------------------------------------------------------

    enum : uint8_t {
        PST_PICTURE         = 0x00,  //!< Picture PES start code.
        PST_SLICE_MIN       = 0x01,  //!< First slice PES start code.
        PST_SLICE_MAX       = 0xAF,  //!< Last slice PES start code.
        PST_RESERVED_B0     = 0xB0,  //!< Reserved PES start code.
        PST_RESERVED_B1     = 0xB1,  //!< Reserved PES start code.
        PST_USER_DATA       = 0xB2,  //!< User data PES start code.
        PST_SEQUENCE_HEADER = 0xB3,  //!< Sequence header PES start code.
        PST_SEQUENCE_ERROR  = 0xB4,  //!< Sequence error PES start code.
        PST_EXTENSION       = 0xB5,  //!< Extension PES start code.
        PST_RESERVED_B6     = 0xB6,  //!< Reserved PES start code.
        PST_SEQUENCE_END    = 0xB7,  //!< End of sequence PES start code.
        PST_GROUP           = 0xB8,  //!< Group PES start code.
        PST_SYSTEM_MIN      = 0xB9,  //!< First stream id value (SID_*).
        PST_SYSTEM_MAX      = 0xFF,  //!< Last stream id value (SID_*).
    };

    //---------------------------------------------------------------------
    //! Frame rate values (in MPEG-1/2 video sequence).
    //---------------------------------------------------------------------

    enum {
        FPS_23_976 = 0x01,  //!< 23.976 fps (24000/1001)
        FPS_24     = 0x02,  //!< 24 fps
        FPS_25     = 0x03,  //!< 25 fps
        FPS_29_97  = 0x04,  //!< 29.97 fps (30000/1001)
        FPS_30     = 0x05,  //!< 30 fps
        FPS_50     = 0x06,  //!< 50 fps
        FPS_59_94  = 0x07,  //!< 59.94 fps (60000/1001)
        FPS_60     = 0x08,  //!< 60 fps
    };

    //---------------------------------------------------------------------
    //! Aspect ratio values (in MPEG-1/2 video sequence header).
    //---------------------------------------------------------------------

    enum {
        AR_SQUARE = 1,  //!< 1/1 MPEG video aspect ratio.
        AR_4_3    = 2,  //!< 4/3 MPEG video aspect ratio.
        AR_16_9   = 3,  //!< 16/9 MPEG video aspect ratio.
        AR_221    = 4,  //!< 2.21/1 MPEG video aspect ratio.
    };

    //---------------------------------------------------------------------
    //! Chroma format values (in MPEG-1/2 video sequence header).
    //---------------------------------------------------------------------

    enum {
        CHROMA_MONO = 0,  //!< Monochrome MPEG video.
        CHROMA_420  = 1,  //!< Chroma 4:2:0 MPEG video.
        CHROMA_422  = 2,  //!< Chroma 4:2:2 MPEG video.
        CHROMA_444  = 3,  //!< Chroma 4:4:4 MPEG video.
    };

    //---------------------------------------------------------------------
    //! AVC access unit types
    //---------------------------------------------------------------------

    enum {
        AVC_AUT_NON_IDR      =  1, //!< Coded slice of a non-IDR picture (NALunit type).
        AVC_AUT_SLICE_A      =  2, //!< Coded slice data partition A (NALunit type).
        AVC_AUT_SLICE_B      =  3, //!< Coded slice data partition B (NALunit type).
        AVC_AUT_SLICE_C      =  4, //!< Coded slice data partition C (NALunit type).
        AVC_AUT_IDR          =  5, //!< Coded slice of an IDR picture (NALunit type).
        AVC_AUT_SEI          =  6, //!< Supplemental enhancement information (SEI) (NALunit type).
        AVC_AUT_SEQPARAMS    =  7, //!< Sequence parameter set (NALunit type).
        AVC_AUT_PICPARAMS    =  8, //!< Picture parameter set (NALunit type).
        AVC_AUT_DELIMITER    =  9, //!< Access unit delimiter (NALunit type).
        AVC_AUT_END_SEQUENCE = 10, //!< End of sequence (NALunit type).
        AVC_AUT_END_STREAM   = 11, //!< End of stream (NALunit type).
        AVC_AUT_FILLER       = 12, //!< Filler data (NALunit type).
        AVC_AUT_SEQPARAMSEXT = 13, //!< Sequence parameter set extension (NALunit type).
        AVC_AUT_PREFIX       = 14, //!< Prefix NAL unit in scalable extension (NALunit type).
        AVC_AUT_SUBSETPARAMS = 15, //!< Subset sequence parameter set (NALunit type).
        AVC_AUT_SLICE_NOPART = 19, //!< Coded slice without partitioning (NALunit type).
        AVC_AUT_SLICE_SCALE  = 20, //!< Coded slice in scalable extension (NALunit type).
    };

    //---------------------------------------------------------------------
    //! AVC SEI types
    //---------------------------------------------------------------------

    enum {
        AVC_SEI_BUF_PERIOD = 0,                 //!< SEI type: buffering_period
        AVC_SEI_PIC_TIMING = 1,                 //!< SEI type: pic_timing
        AVC_SEI_PAN_SCAN_RECT = 2,              //!< SEI type: pan_scan_rect
        AVC_SEI_FILLER_PAYLOAD = 3,             //!< SEI type: filler_payload
        AVC_SEI_USER_DATA_REG = 4,              //!< SEI type: user_data_registered_itu_t_t35
        AVC_SEI_USER_DATA_UNREG = 5,            //!< SEI type: user_data_unregistered
        AVC_SEI_RECOVERY_POINT = 6,             //!< SEI type: recovery_point
        AVC_SEI_DEC_REF_PIC_MAR_REP = 7,        //!< SEI type: dec_ref_pic_marking_repetition
        AVC_SEI_SPARE_PIC = 8,                  //!< SEI type: spare_pic
        AVC_SEI_SCENE_INFO = 9,                 //!< SEI type: scene_info
        AVC_SEI_SUB_SEQ_INFO = 10,              //!< SEI type: sub_seq_info
        AVC_SEI_SUB_SEQ_LAYER_CHARS = 11,       //!< SEI type: sub_seq_layer_characteristics
        AVC_SEI_SUB_SEQ_CHARS = 12,             //!< SEI type: sub_seq_characteristics
        AVC_SEI_FFRAME_FREEZE = 13,             //!< SEI type: full_frame_freeze
        AVC_SEI_FFRAME_FREEZE_RELEASE = 14,     //!< SEI type: full_frame_freeze_release
        AVC_SEI_FFRAME_SNAPSHOT = 15,           //!< SEI type: full_frame_snapshot
        AVC_SEI_PROG_REF_SEG_START = 16,        //!< SEI type: progressive_refinement_segment_start
        AVC_SEI_PROG_REF_SEG_END = 17,          //!< SEI type: progressive_refinement_segment_end
        AVC_SEI_MOTION_CSLICE_GROUP_SET = 18,   //!< SEI type: motion_constrained_slice_group_set
        AVC_SEI_FILM_GRAIN_CHARS = 19,          //!< SEI type: film_grain_characteristics
        AVC_SEI_DEBLOCK_FILTER_DISP_PREF = 20,  //!< SEI type: deblocking_filter_display_preference
        AVC_SEI_STEREO_VIDEO_INFO = 21,         //!< SEI type: stereo_video_info
        AVC_SEI_POST_FILTER_HINT = 22,          //!< SEI type: post_filter_hint
        AVC_SEI_TONE_MAPPING_INFO = 23,         //!< SEI type: tone_mapping_info
        AVC_SEI_SCALABILITY_INFO = 24,          //!< SEI type: scalability_info
        AVC_SEI_SUB_PIC_SCALABLE_LAYER = 25,    //!< SEI type: sub_pic_scalable_layer
        AVC_SEI_NON_REQUIRED_LAYER_REP = 26,    //!< SEI type: non_required_layer_rep
        AVC_SEI_PRIORITY_LAYER_INFO = 27,       //!< SEI type: priority_layer_info
        AVC_SEI_LAYERS_NOT_PRESENT = 28,        //!< SEI type: layers_not_present
        AVC_SEI_LAYER_DEP_CHANGE = 29,          //!< SEI type: layer_dependency_change
        AVC_SEI_SCALABLE_NESTING = 30,          //!< SEI type: scalable_nesting
        AVC_SEI_BASE_LAYER_TEMPORAL_HRD = 31,   //!< SEI type: base_layer_temporal_hrd
        AVC_SEI_QUALITY_LAYER_INTEG_CHECK = 32, //!< SEI type: quality_layer_integrity_check
        AVC_SEI_REDUNDANT_PIC_PROPERTY = 33,    //!< SEI type: redundant_pic_property
        AVC_SEI_TL0_PICTURE_INDEX = 34,         //!< SEI type: tl0_picture_index
        AVC_SEI_TL_SWITCHING_POINT = 35,        //!< SEI type: tl_switching_point
    };

    //! Size in bytes of a UUID in AVC SEI's.
    const size_t AVC_SEI_UUID_SIZE = 16;

    //---------------------------------------------------------------------
    //! Stream type values, as used in the PMT.
    //---------------------------------------------------------------------

    enum : uint8_t {
        ST_NULL          = 0x00, //!< Invalid stream type value, used to indicate an absence of value
        ST_MPEG1_VIDEO   = 0x01, //!< MPEG-1 Video
        ST_MPEG2_VIDEO   = 0x02, //!< MPEG-2 Video
        ST_MPEG1_AUDIO   = 0x03, //!< MPEG-1 Audio
        ST_MPEG2_AUDIO   = 0x04, //!< MPEG-2 Audio
        ST_PRIV_SECT     = 0x05, //!< MPEG-2 Private sections
        ST_PES_PRIV      = 0x06, //!< MPEG-2 PES private data
        ST_MHEG          = 0x07, //!< MHEG
        ST_DSMCC         = 0x08, //!< DSM-CC
        ST_MPEG2_ATM     = 0x09, //!< MPEG-2 over ATM
        ST_DSMCC_MPE     = 0x0A, //!< DSM-CC Multi-Protocol Encapsulation
        ST_DSMCC_UN      = 0x0B, //!< DSM-CC User-to-Network messages
        ST_DSMCC_SD      = 0x0C, //!< DSM-CC Stream Descriptors
        ST_DSMCC_SECT    = 0x0D, //!< DSM-CC Sections
        ST_MPEG2_AUX     = 0x0E, //!< MPEG-2 Auxiliary
        ST_AAC_AUDIO     = 0x0F, //!< Advanced Audio Coding (ISO 13818-7)
        ST_MPEG4_VIDEO   = 0x10, //!< MPEG-4 Video
        ST_MPEG4_AUDIO   = 0x11, //!< MPEG-4 Audio
        ST_MPEG4_PES     = 0x12, //!< MPEG-4 SL or FlexMux in PES packets
        ST_MPEG4_SECT    = 0x13, //!< MPEG-4 SL or FlexMux in sections
        ST_DSMCC_DLOAD   = 0x14, //!< DSM-CC Synchronized Download Protocol
        ST_MDATA_PES     = 0x15, //!< MPEG-7 MetaData in PES packets
        ST_MDATA_SECT    = 0x16, //!< MPEG-7 MetaData in sections
        ST_MDATA_DC      = 0x17, //!< MPEG-7 MetaData in DSM-CC Data Carousel
        ST_MDATA_OC      = 0x18, //!< MPEG-7 MetaData in DSM-CC Object Carousel
        ST_MDATA_DLOAD   = 0x19, //!< MPEG-7 MetaData in DSM-CC Sync Downl Proto
        ST_MPEG2_IPMP    = 0x1A, //!< MPEG-2 IPMP stream
        ST_AVC_VIDEO     = 0x1B, //!< AVC video
        ST_HEVC_VIDEO    = 0x24, //!< HEVC video
        ST_HEVC_SUBVIDEO = 0x25, //!< HEVC temporal video subset of an HEVC video stream
        ST_IPMP          = 0x7F, //!< IPMP stream
        ST_AC3_AUDIO     = 0x81, //!< AC-3 Audio (ATSC only)
        ST_SCTE35_SPLICE = 0x86, //!< SCTE 35 splice information tables
        ST_EAC3_AUDIO    = 0x87, //!< Enhanced-AC-3 Audio (ATSC only)
    };

    //!
    //! Check if an stream type value indicates a PES stream.
    //! @param [in] st Stream type as used in the PMT.
    //! @return True if @a st indicates a PES stream.
    //!
    TSDUCKDLL bool IsPES(uint8_t st);

    //!
    //! Check if an stream type value indicates a video stream.
    //! @param [in] st Stream type as used in the PMT.
    //! @return True if @a st indicates a video stream.
    //!
    TSDUCKDLL bool IsVideoST(uint8_t st);

    //!
    //! Check if an stream type value indicates an audio stream.
    //! @param [in] st Stream type as used in the PMT.
    //! @return True if @a st indicates an audio stream.
    //!
    TSDUCKDLL bool IsAudioST(uint8_t st);

    //!
    //! Check if an stream type value indicates a stream carrying sections.
    //! @param [in] st Stream type as used in the PMT.
    //! @return True if @a st indicates a stream carrying sections.
    //!
    TSDUCKDLL bool IsSectionST(uint8_t st);

    //---------------------------------------------------------------------
    // PSI, SI and data sections and tables
    //---------------------------------------------------------------------

    //! Maximum size of a descriptor (255 + 2-byte header).
    const size_t MAX_DESCRIPTOR_SIZE = 257;

    //! Header size of a short section.
    const size_t SHORT_SECTION_HEADER_SIZE = 3;

    //! Header size of a long section.
    const size_t LONG_SECTION_HEADER_SIZE = 8;

    //! Size of the CRC32 field in a long section.
    const size_t SECTION_CRC32_SIZE = 4;

    //! Maximum size of a PSI section (MPEG-defined).
    const size_t MAX_PSI_SECTION_SIZE = 1024;

    //! Maximum size of a private section (including DVB-defined sections).
    const size_t MAX_PRIVATE_SECTION_SIZE  = 4096;

    //! Minimum size of a short section.
    const size_t MIN_SHORT_SECTION_SIZE = SHORT_SECTION_HEADER_SIZE;

    //! Minimum size of a long section.
    const size_t MIN_LONG_SECTION_SIZE = LONG_SECTION_HEADER_SIZE + SECTION_CRC32_SIZE;

    //! Maximum size of the payload of a short section.
    const size_t MAX_PSI_SHORT_SECTION_PAYLOAD_SIZE = MAX_PSI_SECTION_SIZE - SHORT_SECTION_HEADER_SIZE;

    //! Maximum size of the payload of a PSI long section.
    const size_t MAX_PSI_LONG_SECTION_PAYLOAD_SIZE  = MAX_PSI_SECTION_SIZE - LONG_SECTION_HEADER_SIZE - SECTION_CRC32_SIZE;

    //! Maximum size of the payload of a private short section.
    const size_t MAX_PRIVATE_SHORT_SECTION_PAYLOAD_SIZE = MAX_PRIVATE_SECTION_SIZE - SHORT_SECTION_HEADER_SIZE;

    //! Maximum size of the payload of a private long section.
    const size_t MAX_PRIVATE_LONG_SECTION_PAYLOAD_SIZE  = MAX_PRIVATE_SECTION_SIZE - LONG_SECTION_HEADER_SIZE - SECTION_CRC32_SIZE;

    //---------------------------------------------------------------------
    //! Table identification (TID) values
    //---------------------------------------------------------------------

    enum : TID {

        // Valid in all MPEG contexts:

        TID_PAT           = 0x00, //!< Table id for Program Association Table PAT
        TID_CAT           = 0x01, //!< Table id for Conditional Access Table
        TID_PMT           = 0x02, //!< Table id for Program Map Table
        TID_TSDT          = 0x03, //!< Table id for Transport Stream Description Table
        TID_MP4SDT        = 0x04, //!< Table id for MPEG-4 Scene Description Table
        TID_MP4ODT        = 0x05, //!< Table id for MPEG-4 Object Descriptor Table
        TID_MDT           = 0x06, //!< Table id for MetaData Table
        TID_DSMCC_MPE     = 0x3A, //!< Table id for DSM-CC Multi-Protocol Encapsulated data
        TID_DSMCC_UNM     = 0x3B, //!< Table id for DSM-CC User-to-Network Messages
        TID_DSMCC_DDM     = 0x3C, //!< Table id for DSM-CC Download Data Messages
        TID_DSMCC_SD      = 0x3D, //!< Table id for DSM-CC Stream Descriptors
        TID_DSMCC_PD      = 0x3E, //!< Table id for DSM-CC Private Data
        TID_NULL          = 0xFF, //!< Reserved table id value, end of TS packet PSI payload

        // Valid in DVB context:

        TID_NIT_ACT       = 0x40, //!< Table id for Network Information Table - Actual network
        TID_NIT_OTH       = 0x41, //!< Table id for Network Information Table - Other network
        TID_SDT_ACT       = 0x42, //!< Table id for Service Description Table - Actual TS
        TID_SDT_OTH       = 0x46, //!< Table id for Service Description Table - Other TS
        TID_BAT           = 0x4A, //!< Table id for Bouquet Association Table
        TID_UNT           = 0x4B, //!< Table id for Update Notification Table (SSU, ETSI TS 102 006)
        TID_INT           = 0x4C, //!< Table id for IP/MAC Notification Table (MPE, ETSI EN 301 192)
        TID_EIT_PF_ACT    = 0x4E, //!< Table id for EIT present/following - Actual network
        TID_EIT_PF_OTH    = 0x4F, //!< Table id for EIT present/following - Other network
        TID_EIT_S_ACT_MIN = 0x50, //!< Table id for EIT schedule - Actual network
        TID_EIT_S_ACT_MAX = 0x5F, //!< Table id for EIT schedule - Actual network
        TID_EIT_S_OTH_MIN = 0x60, //!< Table id for EIT schedule - Other network
        TID_EIT_S_OTH_MAX = 0x6F, //!< Table id for EIT schedule - Other network
        TID_TDT           = 0x70, //!< Table id for Time & Date Table
        TID_RST           = 0x71, //!< Table id for Running Status Table
        TID_ST            = 0x72, //!< Table id for Stuffing Table
        TID_TOT           = 0x73, //!< Table id for Time Offset Table
        TID_AIT           = 0x74, //!< Table id for Application Information Table (HbbTV, ETSI TS 102 809)
        TID_CT            = 0x75, //!< Table id for Container Table (TV-Anytime)
        TID_RCT           = 0x76, //!< Table id for Related Content Table (TV-Anytime)
        TID_CIT           = 0x77, //!< Table id for Content Identifier Table (TV-Anytime)
        TID_MPE_FEC       = 0x78, //!< Table id for MPE-FEC Table (Data Broadcasting)
        TID_RNT           = 0x79, //!< Table id for Resolution Notification (TV-Anytime)
        TID_MPE_IFEC      = 0x7A, //!< Table id for MPE-IFEC Table
        TID_DIT           = 0x7E, //!< Table id for Discontinuity Information Table
        TID_SIT           = 0x7F, //!< Table id for Selection Information Table

        TID_ECM_80        = 0x80, //!< Table id for ECM 80
        TID_ECM_81        = 0x81, //!< Table id for ECM 81
        TID_EMM_FIRST     = 0x82, //!< Start of Table id range for EMM's
        TID_EMM_LAST      = 0x8F, //!< End of Table id range for EMM's

        // Ranges by type

        TID_EIT_MIN       = 0x4E, //!< Table id for EIT, first TID
        TID_EIT_MAX       = 0x6F, //!< Table id for EIT, last TID
        TID_CAS_FIRST     = 0x80, //!< Start of Table id range for CAS
        TID_CAS_LAST      = 0x8F, //!< End of Table id range for CAS

        // Valid in SafeAccess CAS context:

        TID_SA_CECM_82    = 0x82, //!< Table id for SafeAccess Complementary ECM
        TID_SA_CECM_83    = 0x83, //!< Table id for SafeAccess Complementary ECM
        TID_SA_EMM_STB_U  = 0x84, //!< Table id for SafeAccess STB or CI-CAM unique EMM
        TID_SA_EMM_STB_G  = 0x85, //!< Table id for SafeAccess STB global EMM
        TID_SA_EMM_A      = 0x86, //!< Table id for SafeAccess Global EMM ("all")
        TID_SA_EMM_U      = 0x87, //!< Table id for SafeAccess Unique EMM
        TID_SA_EMM_S      = 0x88, //!< Table id for SafeAccess Group EMM ("shared")
        TID_SA_EMM_CAM_G  = 0x89, //!< Table id for SafeAccess CI-CAM global EMM
        TID_SA_RECM_8A    = 0x8A, //!< Table id for SafeAccess Record ECM
        TID_SA_RECM_8B    = 0x8B, //!< Table id for SafeAccess Record ECM
        TID_SA_EMM_T      = 0x8F, //!< Table id for SafeAccess Technical EMM

        // Valid in Logiways context:

        TID_LW_DMT        = 0x90, //!< Table id for Logiways Download Marker Table
        TID_LW_BDT        = 0x91, //!< Table id for Logiways Binary Data Table
        TID_LW_VIT        = 0x92, //!< Table id for Logiways VoD Information Table
        TID_LW_VCT        = 0x93, //!< Table id for Logiways VoD Command Table

        // Valid in Viaccess CAS context:

        TID_VIA_EMM_FT_E  = 0x86, //!< Table id for Viaccess EMM-FT (even)
        TID_VIA_EMM_FT_O  = 0x87, //!< Table id for Viaccess EMM-FT (odd)
        TID_VIA_EMM_U     = 0x88, //!< Table id for Viaccess EMM-U and EMM-D-U
        TID_VIA_EMM_GA_E  = 0x8A, //!< Table id for Viaccess EMM-GA and EMM-D-GA (even)
        TID_VIA_EMM_GA_O  = 0x8B, //!< Table id for Viaccess EMM-GA and EMM-D-GA (odd)
        TID_VIA_EMM_GH_E  = 0x8C, //!< Table id for Viaccess EMM-GH (even)
        TID_VIA_EMM_GH_O  = 0x8D, //!< Table id for Viaccess EMM-GH (odd)
        TID_VIA_EMM_S     = 0x8E, //!< Table id for Viaccess EMM-S

        // Valid in MediaGuard CAS context:

        TID_MG_EMM_U      = 0x82, //!< Table id for MediaGuard EMM-U
        TID_MG_EMM_A      = 0x83, //!< Table id for MediaGuard EMM-A
        TID_MG_EMM_G      = 0x84, //!< Table id for MediaGuard EMM-G
        TID_MG_EMM_I      = 0x85, //!< Table id for MediaGuard EMM-I
        TID_MG_EMM_C      = 0x86, //!< Table id for MediaGuard EMM-C
        TID_MG_EMM_CG     = 0x89, //!< Table id for MediaGuard EMM-CG

        // Valid in ATSC / SCTE context:

        TID_SCTE35_SIT    = 0xFC, //!< Table id for SCTE 35 Splice Information Table
    };

    const size_t TID_MAX = 0x100; //!< Maximum number of TID values.

    //---------------------------------------------------------------------
    //! Private data specifier (PDS) values
    //---------------------------------------------------------------------

    enum {
        PDS_NAGRA     = 0x00000009, //!< Private data specifier for Nagra (1).
        PDS_NAGRA_2   = 0x0000000A, //!< Private data specifier for Nagra (2).
        PDS_NAGRA_3   = 0x0000000B, //!< Private data specifier for Nagra (3).
        PDS_NAGRA_4   = 0x0000000C, //!< Private data specifier for Nagra (4).
        PDS_NAGRA_5   = 0x0000000D, //!< Private data specifier for Nagra (5).
        PDS_TPS       = 0x00000010, //!< Private data specifier for TPS.
        PDS_EACEM     = 0x00000028, //!< Private data specifier for EACEM / EICTA.
        PDS_EICTA     = PDS_EACEM,  //!< Private data specifier for EACEM / EICTA.
        PDS_LOGIWAYS  = 0x000000A2, //!< Private data specifier for Logiways.
        PDS_CANALPLUS = 0x000000C0, //!< Private data specifier for Canal+.
        PDS_EUTELSAT  = 0x0000055F, //!< Private data specifier for EutelSat.
        PDS_NULL      = 0xFFFFFFFF, //!< An invalid private data specifier, can be used as placeholder.
    };

    //!
    //! Enumeration description of PDS values.
    //! Typically used to implement PDS-related command line options.
    //!
    TSDUCKDLL extern const Enumeration PrivateDataSpecifierEnum;

    //---------------------------------------------------------------------
    //! Descriptor tag values (descriptor identification, DID)
    //---------------------------------------------------------------------

    enum : DID {

        // Valid in all MPEG contexts:

        DID_VIDEO               = 0x02, //!< DID for video_stream_descriptor
        DID_AUDIO               = 0x03, //!< DID for audio_stream_descriptor
        DID_HIERARCHY           = 0x04, //!< DID for hierarchy_descriptor
        DID_REGISTRATION        = 0x05, //!< DID for registration_descriptor
        DID_DATA_ALIGN          = 0x06, //!< DID for data_stream_alignment_descriptor
        DID_TGT_BG_GRID         = 0x07, //!< DID for target_background_grid_descriptor
        DID_VIDEO_WIN           = 0x08, //!< DID for video_window_descriptor
        DID_CA                  = 0x09, //!< DID for CA_descriptor
        DID_LANGUAGE            = 0x0A, //!< DID for ISO_639_language_descriptor
        DID_SYS_CLOCK           = 0x0B, //!< DID for system_clock_descriptor
        DID_MUX_BUF_USE         = 0x0C, //!< DID for multiplex_buffer_utilization_desc
        DID_COPYRIGHT           = 0x0D, //!< DID for copyright_descriptor
        DID_MAX_BITRATE         = 0x0E, //!< DID for maximum bitrate descriptor
        DID_PRIV_DATA_IND       = 0x0F, //!< DID for private data indicator descriptor
        DID_SMOOTH_BUF          = 0x10, //!< DID for smoothing buffer descriptor
        DID_STD                 = 0x11, //!< DID for STD_descriptor
        DID_IBP                 = 0x12, //!< DID for IBP_descriptor
        DID_CAROUSEL_IDENTIFIER = 0x13, //!< DID for DSM-CC carousel identifier descriptor
        DID_ASSOCIATION_TAG     = 0x14, //!< DID for DSM-CC association tag descriptor
        DID_DEFERRED_ASSOC_TAGS = 0x15, //!< DID for DSM-CC deferred association tags descriptor
        DID_NPT_REFERENCE       = 0x17, //!< DID for DSM-CC NPT reference descriptor
        DID_NPT_ENDPOINT        = 0x18, //!< DID for DSM-CC NPT endpoint descriptor
        DID_STREAM_MODE         = 0x19, //!< DID for DSM-CC stream mode descriptor
        DID_STREAM_EVENT        = 0x1A, //!< DID for DSM-CC stream event descriptor
        DID_MPEG4_VIDEO         = 0x1B, //!< DID for MPEG-4_video_descriptor
        DID_MPEG4_AUDIO         = 0x1C, //!< DID for MPEG-4_audio_descriptor
        DID_IOD                 = 0x1D, //!< DID for IOD_descriptor
        DID_SL                  = 0x1E, //!< DID for SL_descriptor
        DID_FMC                 = 0x1F, //!< DID for FMC_descriptor
        DID_EXT_ES_ID           = 0x20, //!< DID for External_ES_id_descriptor
        DID_MUXCODE             = 0x21, //!< DID for MuxCode_descriptor
        DID_FMX_BUFFER_SIZE     = 0x22, //!< DID for FmxBufferSize_descriptor
        DID_MUX_BUFFER          = 0x23, //!< DID for MultiplexBuffer_descriptor
        DID_CONTENT_LABELING    = 0x24, //!< DID for Content_labeling_descriptor
        DID_METADATA_ASSOC      = 0x25, //!< DID for Metadata_association_descriptor
        DID_METADATA            = 0x26, //!< DID for Metadata_descriptor
        DID_METADATA_STD        = 0x27, //!< DID for Metadata_STD_descriptor
        DID_AVC_VIDEO           = 0x28, //!< DID for AVC_video_descriptor
        DID_MPEG2_IPMP          = 0x29, //!< DID for MPEG-2_IPMP_descriptor
        DID_AVC_TIMING_HRD      = 0x2A, //!< DID for AVC_timing_and_HRD_descriptor
        DID_MPEG2_AAC_AUDIO     = 0x2B, //!< DID for MPEG-2 AAC Audio descriptor
        DID_FLEX_MUX_TIMING     = 0x2C, //!< DID for FlexMuxTiming descriptor
        DID_MPEG4_TEXT          = 0x2D, //!< DID for MPEG-4 Text descriptor
        DID_MPEG4_AUDIO_EXT     = 0x2E, //!< DID for MPEG-4 Audio Extension descriptor
        DID_AUX_VIDEO           = 0x2F, //!< DID for Auxiliary Video Stream descriptor
        DID_SVC_EXT             = 0x30, //!< DID for SVC Extension descriptor
        DID_MVC_EXT             = 0x31, //!< DID for MVC Extension descriptor
        DID_J2K_VIDEO           = 0x32, //!< DID for J2K Video descriptor
        DID_MVC_OPER_POINT      = 0x33, //!< DID for MVC Operation Point descriptor
        DID_STEREO_VIDEO_FORMAT = 0x34, //!< DID for MPEG-2 Stereoscopic Video Format descriptor
        DID_STEREO_PROG_INFO    = 0x35, //!< DID for Stereoscopic Program Info descriptor
        DID_STEREO_VIDEO_INFO   = 0x36, //!< DID for Stereoscopic Video Info descriptor
        DID_TRANSPORT_PROFILE   = 0x37, //!< DID for Transport Profile descriptor
        DID_HEVC_VIDEO          = 0x38, //!< DID for HEVC Video descriptor
        DID_MPEG_EXTENSION      = 0x3F, //!< DID for MPEG-2 Extension descriptor

        // Valid in DVB context:

        DID_NETWORK_NAME        = 0x40, //!< DID for DVB network_name_descriptor
        DID_SERVICE_LIST        = 0x41, //!< DID for DVB service_list_descriptor
        DID_STUFFING            = 0x42, //!< DID for DVB stuffing_descriptor
        DID_SAT_DELIVERY        = 0x43, //!< DID for DVB satellite_delivery_system_desc
        DID_CABLE_DELIVERY      = 0x44, //!< DID for DVB cable_delivery_system_descriptor
        DID_VBI_DATA            = 0x45, //!< DID for DVB VBI_data_descriptor
        DID_VBI_TELETEXT        = 0x46, //!< DID for DVB VBI_teletext_descriptor
        DID_BOUQUET_NAME        = 0x47, //!< DID for DVB bouquet_name_descriptor
        DID_SERVICE             = 0x48, //!< DID for DVB service_descriptor
        DID_COUNTRY_AVAIL       = 0x49, //!< DID for DVB country_availability_descriptor
        DID_LINKAGE             = 0x4A, //!< DID for DVB linkage_descriptor
        DID_NVOD_REFERENCE      = 0x4B, //!< DID for DVB NVOD_reference_descriptor
        DID_TIME_SHIFT_SERVICE  = 0x4C, //!< DID for DVB time_shifted_service_descriptor
        DID_SHORT_EVENT         = 0x4D, //!< DID for DVB short_event_descriptor
        DID_EXTENDED_EVENT      = 0x4E, //!< DID for DVB extended_event_descriptor
        DID_TIME_SHIFT_EVENT    = 0x4F, //!< DID for DVB time_shifted_event_descriptor
        DID_COMPONENT           = 0x50, //!< DID for DVB component_descriptor
        DID_MOSAIC              = 0x51, //!< DID for DVB mosaic_descriptor
        DID_STREAM_ID           = 0x52, //!< DID for DVB stream_identifier_descriptor
        DID_CA_ID               = 0x53, //!< DID for DVB CA_identifier_descriptor
        DID_CONTENT             = 0x54, //!< DID for DVB content_descriptor
        DID_PARENTAL_RATING     = 0x55, //!< DID for DVB parental_rating_descriptor
        DID_TELETEXT            = 0x56, //!< DID for DVB teletext_descriptor
        DID_TELEPHONE           = 0x57, //!< DID for DVB telephone_descriptor
        DID_LOCAL_TIME_OFFSET   = 0x58, //!< DID for DVB local_time_offset_descriptor
        DID_SUBTITLING          = 0x59, //!< DID for DVB subtitling_descriptor
        DID_TERREST_DELIVERY    = 0x5A, //!< DID for DVB terrestrial_delivery_system_desc
        DID_MLINGUAL_NETWORK    = 0x5B, //!< DID for DVB multilingual_network_name_desc
        DID_MLINGUAL_BOUQUET    = 0x5C, //!< DID for DVB multilingual_bouquet_name_desc
        DID_MLINGUAL_SERVICE    = 0x5D, //!< DID for DVB multilingual_service_name_desc
        DID_MLINGUAL_COMPONENT  = 0x5E, //!< DID for DVB multilingual_component_descriptor
        DID_PRIV_DATA_SPECIF    = 0x5F, //!< DID for DVB private_data_specifier_descriptor
        DID_SERVICE_MOVE        = 0x60, //!< DID for DVB service_move_descriptor
        DID_SHORT_SMOOTH_BUF    = 0x61, //!< DID for DVB short_smoothing_buffer_descriptor
        DID_FREQUENCY_LIST      = 0x62, //!< DID for DVB frequency_list_descriptor
        DID_PARTIAL_TS          = 0x63, //!< DID for DVB partial_transport_stream_desc
        DID_DATA_BROADCAST      = 0x64, //!< DID for DVB data_broadcast_descriptor
        DID_SCRAMBLING          = 0x65, //!< DID for DVB scrambling_descriptor
        DID_DATA_BROADCAST_ID   = 0x66, //!< DID for DVB data_broadcast_id_descriptor
        DID_TRANSPORT_STREAM    = 0x67, //!< DID for DVB transport_stream_descriptor
        DID_DSNG                = 0x68, //!< DID for DVB DSNG_descriptor
        DID_PDC                 = 0x69, //!< DID for DVB PDC_descriptor
        DID_AC3                 = 0x6A, //!< DID for DVB AC-3_descriptor
        DID_ANCILLARY_DATA      = 0x6B, //!< DID for DVB ancillary_data_descriptor
        DID_CELL_LIST           = 0x6C, //!< DID for DVB cell_list_descriptor
        DID_CELL_FREQ_LINK      = 0x6D, //!< DID for DVB cell_frequency_link_descriptor
        DID_ANNOUNCE_SUPPORT    = 0x6E, //!< DID for DVB announcement_support_descriptor
        DID_APPLI_SIGNALLING    = 0x6F, //!< DID for DVB application_signalling_descriptor
        DID_ADAPTFIELD_DATA     = 0x70, //!< DID for DVB adaptation_field_data_descriptor
        DID_SERVICE_ID          = 0x71, //!< DID for DVB service_identifier_descriptor
        DID_SERVICE_AVAIL       = 0x72, //!< DID for DVB service_availability_descriptor
        DID_DEFAULT_AUTHORITY   = 0x73, //!< DID for DVB default_authority_descriptor
        DID_RELATED_CONTENT     = 0x74, //!< DID for DVB related_content_descriptor
        DID_TVA_ID              = 0x75, //!< DID for DVB TVA_id_descriptor
        DID_CONTENT_ID          = 0x76, //!< DID for DVB content_identifier_descriptor
        DID_TIME_SLICE_FEC_ID   = 0x77, //!< DID for DVB time_slice_fec_identifier_desc
        DID_ECM_REPETITION_RATE = 0x78, //!< DID for DVB ECM_repetition_rate_descriptor
        DID_S2_SAT_DELIVERY     = 0x79, //!< DID for DVB S2_satellite_delivery_system_descriptor
        DID_ENHANCED_AC3        = 0x7A, //!< DID for DVB enhanced_AC-3_descriptor
        DID_DTS                 = 0x7B, //!< DID for DVB DTS_descriptor
        DID_AAC                 = 0x7C, //!< DID for DVB AAC_descriptor
        DID_XAIT_LOCATION       = 0x7D, //!< DID for DVB XAIT_location_descriptor (DVB-MHP)
        DID_FTA_CONTENT_MGMT    = 0x7E, //!< DID for DVB FTA_content_management_descriptor
        DID_DVB_EXTENSION       = 0x7F, //!< DID for DVB extension_descriptor

        // Valid in an AIT (Application Information Table, ETSI TS 102 809):

        DID_AIT_APPLICATION     = 0x00, //!< DID for AIT application_descriptor.
        DID_AIT_APP_NAME        = 0x01, //!< DID for AIT application_name_descriptor.
        DID_AIT_TRANSPORT_PROTO = 0x02, //!< DID for AIT transport_protocol_descriptor.
        DID_AIT_DVBJ_APP        = 0x03, //!< DID for AIT dvb_j_application_descriptor.
        DID_AIT_DVBJ_APP_LOC    = 0x04, //!< DID for AIT dvb_j_application_location_descriptor.
        DID_AIT_EXT_APP_AUTH    = 0x05, //!< DID for AIT external_application_authorisation_descriptor.
        DID_AIT_APP_RECORDING   = 0x06, //!< DID for AIT application_recording_descriptor.
        DID_AIT_HTML_APP        = 0x08, //!< DID for AIT dvb_html_application_descriptor.
        DID_AIT_HTML_APP_LOC    = 0x09, //!< DID for AIT dvb_html_application_location_descriptor.
        DID_AIT_HTML_APP_BOUND  = 0x0A, //!< DID for AIT dvb_html_application_boundary_descriptor.
        DID_AIT_APP_ICONS       = 0x0B, //!< DID for AIT application_icons_descriptor.
        DID_AIT_PREFETCH        = 0x0C, //!< DID for AIT prefetch_descriptor.
        DID_AIT_DII_LOCATION    = 0x0D, //!< DID for AIT DII_location_descriptor.
        DID_AIT_APP_STORAGE     = 0x10, //!< DID for AIT application_storage_descriptor.
        DID_AIT_IP_SIGNALLING   = 0x11, //!< DID for AIT IP_signalling_descriptor.
        DID_AIT_GRAPHICS_CONST  = 0x14, //!< DID for AIT graphics_constraints_descriptor.
        DID_AIT_APP_LOCATION    = 0x15, //!< DID for AIT simple_application_location_descriptor.
        DID_AIT_APP_USAGE       = 0x16, //!< DID for AIT application_usage_descriptor.
        DID_AIT_APP_BOUNDARY    = 0x17, //!< DID for AIT simple_application_boundary_descriptor.

        // Valid in an INT (IP/MAC Notification Table, ETSI EN 301 192):

        DID_INT_SMARTCARD       = 0x06, //!< DID for INT target_smartcard_descriptor
        DID_INT_MAC_ADDR        = 0x07, //!< DID for INT target_MAC_address_descriptor
        DID_INT_SERIAL_NUM      = 0x08, //!< DID for INT target_serial_number_descriptor
        DID_INT_IP_ADDR         = 0x09, //!< DID for INT target_IP_address_descriptor
        DID_INT_IPV6_ADDR       = 0x0A, //!< DID for INT target_IPv6_address_descriptor
        DID_INT_PF_NAME         = 0x0C, //!< DID for INT IP/MAC_platform_name_descriptor
        DID_INT_PF_PROVIDER     = 0x0D, //!< DID for INT IP/MAC_platform_provider_name_descriptor
        DID_INT_MAC_ADDR_RANGE  = 0x0E, //!< DID for INT target_MAC_address_range_descriptor
        DID_INT_IP_SLASH        = 0x0F, //!< DID for INT target_IP_slash_descriptor
        DID_INT_IP_SRC_SLASH    = 0x10, //!< DID for INT target_IP_source_slash_descriptor
        DID_INT_IPV6_SLASH      = 0x11, //!< DID for INT target_IPv6_slash_descriptor
        DID_INT_IPV6_SRC_SLASH  = 0x12, //!< DID for INT target_IPv6_source_slash_descriptor
        DID_INT_STREAM_LOC      = 0x13, //!< DID for INT IP/MAC_stream_location_descriptor
        DID_INT_ISP_ACCESS      = 0x14, //!< DID for INT ISP_access_mode_descriptor
        DID_INT_GEN_STREAM_LOC  = 0x15, //!< DID for INT IP/MAC_generic_stream_location_descriptor

        // Valid in a UNT (Update Notification Table, ETSI TS 102 006):

        DID_UNT_SCHEDULING      = 0x01, //!< DID for UNT scheduling_descriptor
        DID_UNT_UPDATE          = 0x02, //!< DID for UNT update_descriptor
        DID_UNT_SSU_LOCATION    = 0x03, //!< DID for UNT ssu_location_descriptor
        DID_UNT_MESSAGE         = 0x04, //!< DID for UNT message_descriptor
        DID_UNT_SSU_EVENT_NAME  = 0x05, //!< DID for UNT ssu_event_name_descriptor
        DID_UNT_SMARTCARD       = 0x06, //!< DID for UNT target_smartcard_descriptor
        DID_UNT_MAC_ADDR        = 0x07, //!< DID for UNT target_MAC_address_descriptor
        DID_UNT_SERIAL_NUM      = 0x08, //!< DID for UNT target_serial_number_descriptor
        DID_UNT_IP_ADDR         = 0x09, //!< DID for UNT target_IP_address_descriptor
        DID_UNT_IPV6_ADDR       = 0x0A, //!< DID for UNT target_IPv6_address_descriptor
        DID_UNT_SUBGROUP_ASSOC  = 0x0B, //!< DID for UNT ssu_subgroup_association_descriptor
        DID_UNT_ENHANCED_MSG    = 0x0C, //!< DID for UNT enhanced_message_descriptor
        DID_UNT_SSU_URI         = 0x0D, //!< DID for UNT ssu_uri_descriptor

        // Valid in a SIT (Splice Information Table, SCTE 35).

        DID_SPLICE_AVAIL        = 0x00, //!< DID for SCTE 35 SIT avail_descriptor
        DID_SPLICE_DTMF         = 0x01, //!< DID for SCTE 35 SIT DTMF_descriptor
        DID_SPLICE_SEGMENT      = 0x02, //!< DID for SCTE 35 SIT segmentation_descriptor
        DID_SPLICE_TIME         = 0x03, //!< DID for SCTE 35 SIT time_descriptor

        // Valid in ATSC / SCTE context:

        DID_ATSC_STUFFING       = 0X80, //!< DID for ATSC stuffing_descriptor
        DID_AC3_AUDIO_STREAM    = 0x81, //!< DID for ATSC ac3_audio_stream_descriptor
        DID_ATSC_PID            = 0x85, //!< DID for ATSC program_identifier_descriptor
        DID_CAPTION             = 0x86, //!< DID for ATSC caption_service_descriptor
        DID_CONTENT_ADVIS       = 0x87, //!< DID for ATSC content_advisory_descriptor
        DID_CUE_IDENTIFIER      = 0x8A, //!< DID for SCTE 35 cue_identifier_descriptor
        DID_EXT_CHAN_NAME       = 0xA0, //!< DID for ATSC extended_channel_name_descriptor
        DID_SERV_LOCATION       = 0xA1, //!< DID for ATSC service_location_descriptor
        DID_ATSC_TIME_SHIFT     = 0xA2, //!< DID for ATSC time_shifted_event_descriptor
        DID_COMPONENT_NAME      = 0xA3, //!< DID for ATSC component_name_descriptor
        DID_ATSC_DATA_BRDCST    = 0xA4, //!< DID for ATSC data_broadcast_descriptor
        DID_PID_COUNT           = 0xA5, //!< DID for ATSC pid_count_descriptor
        DID_DOWNLOAD            = 0xA6, //!< DID for ATSC download_descriptor
        DID_MPROTO_ENCAPS       = 0xA7, //!< DID for ATSC multiprotocol_encapsulation_desc

        // Valid after PDS_LOGIWAYS private_data_specifier

        DID_LW_SUBSCRIPTION      = 0x81, //!< DID for Logiways subscription_descriptor
        DID_LW_SCHEDULE          = 0xB0, //!< DID for Logiways schedule_descriptor
        DID_LW_PRIV_COMPONENT    = 0xB1, //!< DID for Logiways private_component_descriptor
        DID_LW_PRIV_LINKAGE      = 0xB2, //!< DID for Logiways private_linkage_descriptor
        DID_LW_CHAPTER           = 0xB3, //!< DID for Logiways chapter_descriptor
        DID_LW_DRM               = 0xB4, //!< DID for Logiways DRM_descriptor
        DID_LW_VIDEO_SIZE        = 0xB5, //!< DID for Logiways video_size_descriptor
        DID_LW_EPISODE           = 0xB6, //!< DID for Logiways episode_descriptor
        DID_LW_PRICE             = 0xB7, //!< DID for Logiways price_descriptor
        DID_LW_ASSET_REFERENCE   = 0xB8, //!< DID for Logiways asset_reference_descriptor
        DID_LW_CONTENT_CODING    = 0xB9, //!< DID for Logiways content_coding_descriptor
        DID_LW_VOD_COMMAND       = 0xBA, //!< DID for Logiways vod_command_descriptor
        DID_LW_DELETION_DATE     = 0xBB, //!< DID for Logiways deletion_date_descriptor
        DID_LW_PLAY_LIST         = 0xBC, //!< DID for Logiways play_list_descriptor
        DID_LW_PLAY_LIST_ENTRY   = 0xBD, //!< DID for Logiways play_list_entry_descriptor
        DID_LW_ORDER_CODE        = 0xBE, //!< DID for Logiways order_code_descriptor
        DID_LW_BOUQUET_REFERENCE = 0xBF, //!< DID for Logiways bouquet_reference_descriptor

        // Valid after PDS_EUTELSAT private_data_specifier

        DID_EUTELSAT_CHAN_NUM   = 0x83, //!< DID for eutelsat_channel_number_descriptor

        // Valid after PDS_EACEM/EICTA private_data_specifier

        DID_LOGICAL_CHANNEL_NUM = 0x83, //!< DID for EACEM/EICTA logical_channel_number_descriptor
        DID_PREF_NAME_LIST      = 0x84, //!< DID for EACEM/EICTA preferred_name_list_descriptor
        DID_PREF_NAME_ID        = 0x85, //!< DID for EACEM/EICTA preferred_name_identifier_descriptor
        DID_EACEM_STREAM_ID     = 0x86, //!< DID for EACEM/EICTA eacem_stream_identifier_descriptor
        DID_HD_SIMULCAST_LCN    = 0x88, //!< DID for EACEM/EICTA HD_simulcast_logical_channel_number_descriptor

        // Valid after PDS_CANALPLUS private_data_specifier

        DID_DTG_STREAM_IND      = 0x80, //!< DID for Canal+ DTG_Stream_indicator_descriptor
        DID_PIO_OFFSET_TIME     = 0X80, //!< DID for Canal+ pio_offset_time_descriptor
        DID_LOGICAL_CHANNEL_81  = 0x81, //!< DID for Canal+ logical_channel_descriptor
        DID_PRIVATE2            = 0x82, //!< DID for Canal+ private_descriptor2
        DID_LOGICAL_CHANNEL     = 0x83, //!< DID for Canal+ logical_channel_descriptor
        DID_PIO_CONTENT         = 0x83, //!< DID for Canal+ pio_content_descriptor
        DID_PIO_LOGO            = 0x84, //!< DID for Canal+ pio_logo_descriptor
        DID_ADSL_DELIVERY       = 0x85, //!< DID for Canal+ adsl_delivery_system_descriptor
        DID_PIO_FEE             = 0x86, //!< DID for Canal+ pio_fee_descriptor
        DID_PIO_EVENT_RANGE     = 0x88, //!< DID for Canal+ pio_event_range_descriptor
        DID_PIO_COPY_MANAGEMENT = 0x8B, //!< DID for Canal+ pio_copy_management_descriptor
        DID_PIO_COPY_CONTROL    = 0x8C, //!< DID for Canal+ pio_copy_control_descriptor
        DID_PIO_PPV             = 0x8E, //!< DID for Canal+ pio_ppv_descriptor
        DID_PIO_STB_SERVICE_ID  = 0x90, //!< DID for Canal+ pio_stb_service_id_descriptor
        DID_PIO_MASKING_SERV_ID = 0x91, //!< DID for Canal+ pio_masking_service_id_descriptor
        DID_PIO_STB_SERVMAP_UPD = 0x92, //!< DID for Canal+ pio_stb_service_map_update_desc
        DID_NEW_SERVICE_LIST    = 0x93, //!< DID for Canal+ new_service_list_descriptor
        DID_MESSAGE_NAGRA       = 0x94, //!< DID for Canal+ message_descriptor_Nagra
        DID_ITEM_EVENT          = 0xA1, //!< DID for Canal+ item_event_descriptor
        DID_ITEM_ZAPPING        = 0xA2, //!< DID for Canal+ item_zapping_descriptor
        DID_APPLI_MESSAGE       = 0xA3, //!< DID for Canal+ appli_message_descriptor
        DID_LIST                = 0xA4, //!< DID for Canal+ list_descriptor
        DID_KEY_LIST            = 0xB0, //!< DID for Canal+ key_list_descriptor
        DID_PICTURE_SIGNALLING  = 0xB1, //!< DID for Canal+ picture_signalling_descriptor
        DID_COUNTER_BB          = 0xBB, //!< DID for Canal+ counter_descriptor
        DID_DATA_COMPONENT_BD   = 0xBD, //!< DID for Canal+ data_component_descriptor
        DID_SYSTEM_MGMT_BE      = 0xBE, //!< DID for Canal+ system_management_descriptor
        DID_VO_LANGUAGE         = 0xC0, //!< DID for Canal+ vo_language_descriptor
        DID_DATA_LIST           = 0xC1, //!< DID for Canal+ data_list_descriptor
        DID_APPLI_LIST          = 0xC2, //!< DID for Canal+ appli_list_descriptor
        DID_MESSAGE             = 0xC3, //!< DID for Canal+ message_descriptor
        DID_FILE                = 0xC4, //!< DID for Canal+ file_descriptor
        DID_RADIO_FORMAT        = 0xC5, //!< DID for Canal+ radio_format_descriptor
        DID_APPLI_STARTUP       = 0xC6, //!< DID for Canal+ appli_startup_descriptor
        DID_PATCH               = 0xC7, //!< DID for Canal+ patch_descriptor
        DID_LOADER              = 0xC8, //!< DID for Canal+ loader_descriptor
        DID_CHANNEL_MAP_UPDATE  = 0xC9, //!< DID for Canal+ channel_map_update_descriptor
        DID_PPV                 = 0xCA, //!< DID for Canal+ ppv_descriptor
        DID_COUNTER_CB          = 0xCB, //!< DID for Canal+ counter_descriptor
        DID_OPERATOR_INFO       = 0xCC, //!< DID for Canal+ operator_info_descriptor
        DID_SERVICE_DEF_PARAMS  = 0xCD, //!< DID for Canal+ service_default_parameters_desc
        DID_FINGER_PRINTING     = 0xCE, //!< DID for Canal+ finger_printing_descriptor
        DID_FINGER_PRINTING_V2  = 0xCF, //!< DID for Canal+ finger_printing_descriptor_v2
        DID_CONCEALED_GEO_ZONES = 0xD0, //!< DID for Canal+ concealed_geo_zones_descriptor
        DID_COPY_PROTECTION     = 0xD1, //!< DID for Canal+ copy_protection_descriptor
        DID_MG_SUBSCRIPTION     = 0xD3, //!< DID for Canal+ subscription_descriptor
        DID_CABLE_BACKCH_DELIV  = 0xD4, //!< DID for Canal+ cable_backchannel_delivery_system
        DID_INTERACT_SNAPSHOT   = 0xD5, //!< DID for Canal+ Interactivity_snapshot_descriptor
        DID_ICON_POSITION       = 0xDC, //!< DID for Canal+ icon_position_descriptor
        DID_ICON_PIXMAP         = 0xDD, //!< DID for Canal+ icon_pixmap_descriptor
        DID_ZONE_COORDINATE     = 0xDE, //!< DID for Canal+ Zone_coordinate_descriptor
        DID_HD_APP_CONTROL_CODE = 0xDF, //!< DID for Canal+ HD_application_control_code_desc
        DID_EVENT_REPEAT        = 0xE0, //!< DID for Canal+ Event_Repeat_descriptor
        DID_PPV_V2              = 0xE1, //!< DID for Canal+ PPV_V2_descriptor
        DID_HYPERLINK_REF       = 0xE2, //!< DID for Canal+ Hyperlink_ref_descriptor
        DID_SHORT_SERVICE       = 0xE4, //!< DID for Canal+ Short_service_descriptor
        DID_OPERATOR_TELEPHONE  = 0xE5, //!< DID for Canal+ Operator_telephone_descriptor
        DID_ITEM_REFERENCE      = 0xE6, //!< DID for Canal+ Item_reference_descriptor
        DID_MH_PARAMETERS       = 0xE9, //!< DID for Canal+ MH_Parameters_descriptor
        DID_LOGICAL_REFERENCE   = 0xED, //!< DID for Canal+ Logical_reference_descriptor
        DID_DATA_VERSION        = 0xEE, //!< DID for Canal+ Data_Version_descriptor
        DID_SERVICE_GROUP       = 0xEF, //!< DID for Canal+ Service_group_descriptor
        DID_STREAM_LOC_TRANSP   = 0xF0, //!< DID for Canal+ Stream_Locator_Transport_desc
        DID_DATA_LOCATOR        = 0xF1, //!< DID for Canal+ Data_Locator_descriptor
        DID_RESIDENT_APP        = 0xF2, //!< DID for Canal+ resident_application_descriptor
        DID_RESIDENT_APP_SIGNAL = 0xF3, //!< DID for Canal+ Resident_Application_Signalling
        DID_MH_LOGICAL_REF      = 0xF8, //!< DID for Canal+ MH_Logical_Reference_descriptor
        DID_RECORD_CONTROL      = 0xF9, //!< DID for Canal+ record_control_descriptor
        DID_CMPS_RECORD_CONTROL = 0xFA, //!< DID for Canal+ cmps_record_control_descriptor
        DID_EPISODE             = 0xFB, //!< DID for Canal+ episode_descriptor
        DID_CMP_SELECTION       = 0xFC, //!< DID for Canal+ CMP_Selection_descriptor
        DID_DATA_COMPONENT_FD   = 0xFD, //!< DID for Canal+ data_component_descriptor
        DID_SYSTEM_MGMT_FE      = 0xFE, //!< DID for Canal+ system_management_descriptor
    };

    //---------------------------------------------------------------------
    //! MPEG extended descriptor tag values (in MPEG extension_descriptor)
    //---------------------------------------------------------------------

    enum : DID {
        MPEG_EDID_OBJ_DESC_UPD  = 0x02, //!< Ext.DID for ObjectDescriptorUpdate.
        MPEG_EDID_HEVC_TIM_HRD  = 0x03, //!< Ext.DID for HEVC_timing_and_HRD_descriptor.
        MPEG_EDID_NULL          = 0xFF, //!< Invalid EDID value, can be used as placeholder.
    };

    //---------------------------------------------------------------------
    //! DVB extended descriptor tag values (in DVB extension_descriptor)
    //---------------------------------------------------------------------

    enum : DID {
        EDID_IMAGE_ICON         = 0x00, //!< Ext.DID for image_icon_descriptor
        EDID_CPCM_DELIVERY_SIG  = 0x01, //!< Ext.DID for cpcm_delivery_signalling_descriptor
        EDID_CP                 = 0x02, //!< Ext.DID for CP_descriptor
        EDID_CP_IDENTIFIER      = 0x03, //!< Ext.DID for CP_identifier_descriptor
        EDID_T2_DELIVERY        = 0x04, //!< Ext.DID for T2_delivery_system_descriptor
        EDID_SH_DELIVERY        = 0x05, //!< Ext.DID for SH_delivery_system_descriptor
        EDID_SUPPL_AUDIO        = 0x06, //!< Ext.DID for supplementary_audio_descriptor
        EDID_NETW_CHANGE_NOTIFY = 0x07, //!< Ext.DID for network_change_notify_descriptor
        EDID_MESSAGE            = 0x08, //!< Ext.DID for message_descriptor
        EDID_TARGET_REGION      = 0x09, //!< Ext.DID for target_region_descriptor
        EDID_TARGET_REGION_NAME = 0x0A, //!< Ext.DID for target_region_name_descriptor
        EDID_SERVICE_RELOCATED  = 0x0B, //!< Ext.DID for service_relocated_descriptor
        EDID_XAIT_PID           = 0x0C, //!< Ext.DID for XAIT_PID_descriptor
        EDID_C2_DELIVERY        = 0x0D, //!< Ext.DID for C2_delivery_system_descriptor
        EDID_DTS_HD_AUDIO       = 0x0E, //!< Ext.DID for DTS_HD_audio_stream_descriptor
        EDID_DTS_NEURAL         = 0x0F, //!< Ext.DID for DTS_Neural_descriptor
        EDID_VIDEO_DEPTH_RANGE  = 0x10, //!< Ext.DID for video_depth_range_descriptor
        EDID_T2MI               = 0x11, //!< Ext.DID for T2MI_descriptor
        EDID_URI_LINKAGE        = 0x13, //!< Ext.DID for URI_linkage_descriptor
        EDID_CI_ANCILLARY_DATA  = 0x14, //!< Ext.DID for CI_ancillary_data_descriptor
        EDID_AC4                = 0x15, //!< Ext.DID for AC4_descriptor
        EDID_C2_BUNDLE_DELIVERY = 0x16, //!< Ext.DID for C2_bundle_system_delivery_descriptor
        EDID_NULL               = 0xFF, //!< Invalid EDID value, can be used as placeholder.
    };

    //---------------------------------------------------------------------
    //! Linkage type values (in linkage_descriptor)
    //---------------------------------------------------------------------

    enum : uint8_t {
        LINKAGE_INFO            = 0x01, //!< Information service
        LINKAGE_EPG             = 0x02, //!< EPG service
        LINKAGE_CA_REPLACE      = 0x03, //!< CA replacement service
        LINKAGE_TS_NIT_BAT      = 0x04, //!< TS containing complet network/bouquet SI
        LINKAGE_SERVICE_REPLACE = 0x05, //!< Service replacement service
        LINKAGE_DATA_BROADCAST  = 0x06, //!< Data broadcast service
        LINKAGE_RCS_MAP         = 0x07, //!< RCS map
        LINKAGE_HAND_OVER       = 0x08, //!< Mobile hand-over
        LINKAGE_SSU             = 0x09, //!< System software update service
        LINKAGE_SSU_TABLE       = 0x0A, //!< TS containing SSU BAT or NIT
        LINKAGE_IP_NOTIFY       = 0x0B, //!< IP/MAC notification service
        LINKAGE_INT_BAT_NIT     = 0x0C, //!< TS containing INT BAT or NIT
        LINKAGE_EVENT           = 0x0D, //!< Event linkage
        LINKAGE_EXT_EVENT_MIN   = 0x0E, //!< Extented event linkage, first value
        LINKAGE_EXT_EVENT_MAX   = 0x1F, //!< Extented event linkage, last value
    };

    //---------------------------------------------------------------------
    //! Scrambling mode values (in scrambling_descriptor)
    //---------------------------------------------------------------------

    enum : uint8_t {
        SCRAMBLING_DVB_CSA1      = 0x01, //!< DVB-CSA1
        SCRAMBLING_DVB_CSA2      = 0x02, //!< DVB-CSA2
        SCRAMBLING_DVB_CSA3_STD  = 0x03, //!< DVB-CSA3, standard mode
        SCRAMBLING_DVB_CSA3_MIN  = 0x04, //!< DVB-CSA3, minimally enhanced mode
        SCRAMBLING_DVB_CSA3_FULL = 0x05, //!< DVB-CSA3, fully enhanced mode
        SCRAMBLING_DVB_CISSA1    = 0x10, //!< DVB-CISSA v1
        SCRAMBLING_ATIS_IIF_IDSA = 0x70, //!< ATIS IIF IDSA for MPEG-2 TS
    };

    //---------------------------------------------------------------------
    //! Data broadcast id values (in data_broadcast[_id]_descriptor)
    //---------------------------------------------------------------------

    enum : uint16_t {
        DBID_DATA_PIPE            = 0x0001, //!< Data pipe
        DBID_ASYNC_DATA_STREAM    = 0x0002, //!< Asynchronous data stream
        DBID_SYNC_DATA_STREAM     = 0x0003, //!< Synchronous data stream
        DBID_SYNCED_DATA_STREAM   = 0x0004, //!< Synchronised data stream
        DBID_MPE                  = 0x0005, //!< Multi protocol encapsulation
        DBID_DATA_CSL             = 0x0006, //!< Data Carousel
        DBID_OBJECT_CSL           = 0x0007, //!< Object Carousel
        DBID_ATM                  = 0x0008, //!< DVB ATM streams
        DBID_HP_ASYNC_DATA_STREAM = 0x0009, //!< Higher Protocols based on asynchronous data streams
        DBID_SSU                  = 0x000A, //!< System Software Update service [TS 102 006]
        DBID_IPMAC_NOTIFICATION   = 0x000B, //!< IP/MAC Notification service [EN 301 192]
        DBID_MHP_OBJECT_CSL       = 0x00F0, //!< MHP Object Carousel
        DBID_MHP_MPE              = 0x00F1, //!< Reserved for MHP Multi Protocol Encapsulation
        DBID_EUTELSAT_DATA_PIPE   = 0x0100, //!< Eutelsat Data Piping
        DBID_EUTELSAT_DATA_STREAM = 0x0101, //!< Eutelsat Data Streaming
        DBID_SAGEM_IP             = 0x0102, //!< SAGEM IP encapsulation in MPEG-2 PES packets
        DBID_BARCO_DATA_BRD       = 0x0103, //!< BARCO Data Broadcasting
        DBID_CIBERCITY_MPE        = 0x0104, //!< CyberCity Multiprotocol Encapsulation
        DBID_CYBERSAT_MPE         = 0x0105, //!< CyberSat Multiprotocol Encapsulation
        DBID_TDN                  = 0x0106, //!< The Digital Network
        DBID_OPENTV_DATA_CSL      = 0x0107, //!< OpenTV Data Carousel
        DBID_PANASONIC            = 0x0108, //!< Panasonic
        DBID_KABEL_DEUTSCHLAND    = 0x0109, //!< Kabel Deutschland
        DBID_TECHNOTREND          = 0x010A, //!< TechnoTrend Gorler GmbH
        DBID_MEDIAHIGHWAY_SSU     = 0x010B, //!< NDS France Technologies system software download
        DBID_GUIDE_PLUS           = 0x010C, //!< GUIDE Plus+ Rovi Corporation
        DBID_ACAP_OBJECT_CSL      = 0x010D, //!< ACAP Object Carousel
        DBID_MICRONAS             = 0x010E, //!< Micronas Download Stream
        DBID_POLSAT               = 0x0110, //!< Televizja Polsat
        DBID_DTG                  = 0x0111, //!< UK DTG
        DBID_SKYMEDIA             = 0x0112, //!< SkyMedia
        DBID_INTELLIBYTE          = 0x0113, //!< Intellibyte DataBroadcasting
        DBID_TELEWEB_DATA_CSL     = 0x0114, //!< TeleWeb Data Carousel
        DBID_TELEWEB_OBJECT_CSL   = 0x0115, //!< TeleWeb Object Carousel
        DBID_TELEWEB              = 0x0116, //!< TeleWeb
        DBID_BBC                  = 0x0117, //!< BBC
        DBID_ELECTRA              = 0x0118, //!< Electra Entertainment Ltd
        DBID_BBC_2_3              = 0x011A, //!< BBC 2 - 3
        DBID_TELETEXT             = 0x011B, //!< Teletext
        DBID_SKY_DOWNLOAD_1_5     = 0x0120, //!< Sky Download Streams 1-5
        DBID_ICO                  = 0x0121, //!< ICO mim
        DBID_CIPLUS_DATA_CSL      = 0x0122, //!< CI+ Data Carousel
        DBID_HBBTV                = 0x0123, //!< HBBTV Carousel
        DBID_ROVI_PREMIUM         = 0x0124, //!< Premium Content from Rovi Corporation
        DBID_MEDIA_GUIDE          = 0x0125, //!< Media Guide from Rovi Corporation
        DBID_INVIEW               = 0x0126, //!< InView Technology Ltd
        DBID_BOTECH               = 0x0130, //!< Botech Elektronik SAN. ve TIC. LTD.STI.
        DBID_SCILLA_PUSHVOD_CSL   = 0x0131, //!< Scilla Push-VOD Carousel
        DBID_CANAL_PLUS           = 0x0140, //!< Canal+
        DBID_OIPF_OBJECT_CSL      = 0x0150, //!< OIPF Object Carousel - Open IPTV Forum
        DBID_4TV                  = 0x4444, //!< 4TV Data Broadcast
        DBID_NOKIA_IP_SSU         = 0x4E4F, //!< Nokia IP based software delivery
        DBID_BBG_DATA_CSL         = 0xBBB1, //!< BBG Data Caroussel
        DBID_BBG_OBJECT_CSL       = 0xBBB2, //!< BBG Object Caroussel
        DBID_BBG                  = 0xBBBB, //!< Bertelsmann Broadband Group
   };

    //---------------------------------------------------------------------
    //! DVB-assigned Bouquet Identifier values
    //---------------------------------------------------------------------

    enum : uint16_t {
        BID_TVNUMERIC          = 0x0086,  //!< Bouquet id for TV Numeric on French TNT network
        BID_TVNUMERIC_EUTELSAT = 0xC030,  //!< Bouquet id for TV Numeric on Eutelsat network
        BID_TVNUMERIC_ASTRA    = 0xC031,  //!< Bouquet id for TV Numeric on Astra network
    };

    //---------------------------------------------------------------------
    //! DVB-assigned CA System Identifier values
    //---------------------------------------------------------------------

    enum : uint16_t {
        CASID_MEDIAGUARD_MIN  = 0x0100,  //!< Minimum CAS Id value for MediaGuard.
        CASID_MEDIAGUARD_MAX  = 0x01FF,  //!< Maximum CAS Id value for MediaGuard.
        CASID_VIACCESS_MIN    = 0x0500,  //!< Minimum CAS Id value for Viaccess.
        CASID_VIACCESS_MAX    = 0x05FF,  //!< Maximum CAS Id value for Viaccess.
        CASID_NAGRA_MIN       = 0x1800,  //!< Minimum CAS Id value for Nagravision.
        CASID_NAGRA_MAX       = 0x18FF,  //!< Maximum CAS Id value for Nagravision.
        CASID_THALESCRYPT_MIN = 0x4A80,  //!< Minimum CAS Id value for ThalesCrypt.
        CASID_THALESCRYPT_MAX = 0x4A8F,  //!< Maximum CAS Id value for ThalesCrypt.
        CASID_SAFEACCESS      = 0x4ADC,  //!< CAS Id value for SafeAccess.
    };

    //---------------------------------------------------------------------
    //! DVB-assigned Network Identifier values
    //---------------------------------------------------------------------

    enum : uint16_t {
        NID_TNT_FRANCE = 0x20FA,  //!< Network id for the French national terrestrial network.
    };

    //---------------------------------------------------------------------
    //! IEEE-assigned Organizationally Unique Identifier (OUI) values
    //---------------------------------------------------------------------

    enum {
        OUI_DVB      = 0x00015A,  //!< OUI for Digital Video Broadcasting
        OUI_SKARDIN  = 0x001222,  //!< OUI for Skardin (UK)
        OUI_LOGIWAYS = 0x002660,  //!< OUI for Logiways
    };

    //---------------------------------------------------------------------
    //! DVB-MHP transport protocol ids.
    //---------------------------------------------------------------------

    enum : uint16_t {
        MHP_PROTO_CAROUSEL = 0x0001,  //!< MHP Object Carousel
        MHP_PROTO_MPE      = 0x0002,  //!< IP via DVB-MPE
        MHP_PROTO_HTTP     = 0x0003,  //!< HTTP over interaction channel
    };

    //---------------------------------------------------------------------
    // T2-MI (DVB-T2 Modulator Interface)
    //---------------------------------------------------------------------

    //!
    //! Size in bytes of a T2-MI packet header.
    //!
    const size_t T2MI_HEADER_SIZE = 6;

    //!
    //! T2-MI packet types.
    //! @see ETSI EN 102 773, section 5.1.
    //!
    enum : uint8_t {
        T2MI_BASEBAND_FRAME        = 0x00, //!< Baseband Frame.
        T2MI_AUX_IQ_DATA           = 0x01, //!< Auxiliary stream I/Q data.
        T2MI_ARBITRARY_CELL        = 0x02, //!< Arbitrary cell insertion.
        T2MI_L1_CURRENT            = 0x10, //!< L1-current.
        T2MI_L1_FUTURE             = 0x11, //!< L1-future.
        T2MI_P2_BIAS_BALANCING     = 0x12, //!< P2 bias balancing cells.
        T2MI_DVBT2_TIMESTAMP       = 0x20, //!< DVB-T2 timestamp.
        T2MI_INDIVIDUAL_ADDRESSING = 0x21, //!< Individual addressing.
        T2MI_FEF_NULL              = 0x30, //!< FEF part: Null.
        T2MI_FEF_IQ_DATA           = 0x31, //!< FEF part: I/Q data.
        T2MI_FEF_COMPOSITE         = 0x32, //!< FEF part: composite.
        T2MI_FEF_SUBPART           = 0x33, //!< FEF sub-part.
        T2MI_INVALID_TYPE          = 0xFF  //!< Invalid T2MI packet (non standard value).
    };

    //!
    //! Size in bytes of a DVB-T2 Base Band Header.
    //! See ETSI EN 302 765, section 5.1.7.
    //!
    const size_t T2_BBHEADER_SIZE = 10;

    //---------------------------------------------------------------------
    // Teletext PES packets.
    // See ETSI EN 300 472 V1.3.1, "DVB; Specification for conveying ITU-R
    // System B Teletext in DVB bitstreams"
    //---------------------------------------------------------------------

    //!
    //! Size in bytes of a Teletext packet.
    //!
    const size_t TELETEXT_PACKET_SIZE = 44;

    const uint8_t TELETEXT_PES_FIRST_EBU_DATA_ID = 0x10;  //!< First EBU data_identifier value in PES packets conveying Teletext.
    const uint8_t TELETEXT_PES_LAST_EBU_DATA_ID  = 0x1F;  //!< Last EBU data_identifier value in PES packets conveying Teletext.

    //!
    //! Teletext data unit ids.
    //! @see ETSI EN 300 472
    //!
    enum : uint8_t {
        TELETEXT_DATA_UNIT_ID_NON_SUBTITLE    = 0x02,  //!< Data_unit_id for EBU Teletext non-subtitle data.
        TELETEXT_DATA_UNIT_ID_SUBTITLE        = 0x03,  //!< Data_unit_id for EBU Teletext subtitle data.
        TELETEXT_DATA_UNIT_ID_INVERTED        = 0x0C,  //!< Data_unit_id for EBU EBU Teletext Inverted (extension ?).
        TELETEXT_DATA_UNIT_ID_VPS             = 0xC3,  //!< Data_unit_id for VPS (extension ?).
        TELETEXT_DATA_UNIT_ID_CLOSED_CAPTIONS = 0xC5,  //!< Data_unit_id for Closed Caption (extension ?).
        TELETEXT_DATA_UNIT_ID_STUFFING        = 0xFF,  //!< Data_unit_id for stuffing data.
    };
}
