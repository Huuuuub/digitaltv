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
//
//  CppUnit test suite for demux classes
//
//----------------------------------------------------------------------------

#include "tsSectionDemux.h"
#include "tsStandaloneTableDemux.h"
#include "tsOneShotPacketizer.h"
#include "tsPAT.h"
#include "tsCAT.h"
#include "tsPMT.h"
#include "tsSDT.h"
#include "tsNIT.h"
#include "tsBAT.h"
#include "tsTOT.h"
#include "tsTDT.h"
#include "tsNames.h"
#include "utestCppUnitTest.h"
TSDUCK_SOURCE;

#include "tables/psi_bat_cplus_packets.h"
#include "tables/psi_bat_cplus_sections.h"
#include "tables/psi_bat_tvnum_packets.h"
#include "tables/psi_bat_tvnum_sections.h"
#include "tables/psi_cat_r3_packets.h"
#include "tables/psi_cat_r3_sections.h"
#include "tables/psi_cat_r6_packets.h"
#include "tables/psi_cat_r6_sections.h"
#include "tables/psi_nit_tntv23_packets.h"
#include "tables/psi_nit_tntv23_sections.h"
#include "tables/psi_pat_r4_packets.h"
#include "tables/psi_pat_r4_sections.h"
#include "tables/psi_pmt_planete_packets.h"
#include "tables/psi_pmt_planete_sections.h"
#include "tables/psi_sdt_r3_packets.h"
#include "tables/psi_sdt_r3_sections.h"
#include "tables/psi_tdt_tnt_packets.h"
#include "tables/psi_tdt_tnt_sections.h"
#include "tables/psi_tot_tnt_packets.h"
#include "tables/psi_tot_tnt_sections.h"
#include "tables/psi_pmt_hevc_packets.h"
#include "tables/psi_pmt_hevc_sections.h"


//----------------------------------------------------------------------------
// The test fixture
//----------------------------------------------------------------------------

class DemuxTest: public CppUnit::TestFixture
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void testPAT();
    void testCATR3();
    void testCATR6();
    void testPMT();
    void testSDT();
    void testNIT();
    void testBATTvNumeric();
    void testBATCanalPlus();
    void testTDT();
    void testTOT();
    void testHEVC();

    CPPUNIT_TEST_SUITE(DemuxTest);
    CPPUNIT_TEST(testPAT);
    CPPUNIT_TEST(testCATR3);
    CPPUNIT_TEST(testCATR6);
    CPPUNIT_TEST(testPMT);
    CPPUNIT_TEST(testSDT);
    CPPUNIT_TEST(testNIT);
    CPPUNIT_TEST(testBATTvNumeric);
    CPPUNIT_TEST(testBATCanalPlus);
    CPPUNIT_TEST(testTDT);
    CPPUNIT_TEST(testTOT);
    CPPUNIT_TEST(testHEVC);
    CPPUNIT_TEST_SUITE_END();

private:
    // Compare a table with the list of reference sections
    bool checkSections(const char* test_name, const char* table_name, const ts::BinaryTable& table, const uint8_t* ref_sections, size_t ref_sections_size);

    // Compare a vector of packets with the list of reference packets
    bool checkPackets(const char* test_name, const char* table_name, const ts::TSPacketVector& packets, const uint8_t* ref_packets, size_t ref_packets_size);

    // Unitary test for one table.
    void testTable(const char* name, const uint8_t* ref_packets, size_t ref_packets_size, const uint8_t* ref_sections, size_t ref_sections_size);
};

CPPUNIT_TEST_SUITE_REGISTRATION(DemuxTest);


//----------------------------------------------------------------------------
// Initialization.
//----------------------------------------------------------------------------

// Test suite initialization method.
void DemuxTest::setUp()
{
}

// Test suite cleanup method.
void DemuxTest::tearDown()
{
}


//----------------------------------------------------------------------------
// Unitary tests.
//----------------------------------------------------------------------------

// Compare a table with the list of reference sections
bool DemuxTest::checkSections(const char* test_name, const char* table_name, const ts::BinaryTable& table, const uint8_t* ref_sections, size_t ref_sections_size)
{
    // First, compute and compare total size of the table
    size_t total_size(0);
    for (size_t si = 0; si < table.sectionCount(); ++si) {
        total_size += table.sectionAt(si)->size();
    }
    if (total_size != ref_sections_size) {
        utest::Out() << "DemuxTest: " << test_name << ", " << table_name
                     << ": total size of " << table.sectionCount() << " sections is "
                     << total_size << " bytes, expected " << ref_sections_size << " bytes"
                     << std::endl
                     << "DemuxTest: Reference sections:" << std::endl
                     << ts::UString::Dump(ref_sections, ref_sections_size, ts::UString::HEXA | ts::UString::OFFSET | ts::UString::ASCII, 2)
                     << "DemuxTest: " << table_name << std::endl;
        for (size_t si = 0; si < table.sectionCount(); ++si) {
            const ts::Section& sect(*table.sectionAt(si));
            utest::Out() << "DemuxTest: " << ts::UString::Dump(sect.content(), sect.size(), ts::UString::HEXA | ts::UString::OFFSET | ts::UString::ASCII, 2);
        }
        return false;
    }

    // Then compare contents of sections
    size_t sections_offset(0);
    for (size_t si = 0; si < table.sectionCount(); ++si) {
        const ts::Section& sect(*table.sectionAt(si));
        const uint8_t* ref = ref_sections + sections_offset;
        const uint8_t* sec = sect.content();
        size_t size = sect.size();
        sections_offset += size;
        for (size_t i = 0; i < size; ++i) {
            if (sec[i] != ref[i]) {
                utest::Out() << "DemuxTest: " << test_name << ", " << table_name
                             << ": difference at offset " << i << " in section " << si
                             << std::endl
                             << "DemuxTest: Reference section:" << std::endl
                             << ts::UString::Dump(ref, size, ts::UString::HEXA | ts::UString::OFFSET | ts::UString::ASCII, 2)
                             << "DemuxTest: " << table_name << std::endl
                             << ts::UString::Dump(sec, size, ts::UString::HEXA | ts::UString::OFFSET | ts::UString::ASCII, 2);
                return false;
            }
        }
    }
    return true;
}

// Compare a vector of packets with the list of reference packets
bool DemuxTest::checkPackets(const char* test_name, const char* table_name, const ts::TSPacketVector& packets, const uint8_t* ref_packets, size_t ref_packets_size)
{
    // First, compute and compare total size of the table
    if (packets.size() != ref_packets_size / ts::PKT_SIZE) {
        utest::Out() << "DemuxTest: " << test_name << ", " << table_name
                     << ": rebuilt " << packets.size() << " packets, expected " << (ref_packets_size / ts::PKT_SIZE)
                     << std::endl
                     << "DemuxTest: Reference packets:" << std::endl
                     << ts::UString::Dump(ref_packets, ref_packets_size, ts::UString::HEXA | ts::UString::OFFSET | ts::UString::ASCII, 2)
                     << "* " << table_name << ":" << std::endl
                     << ts::UString::Dump(packets[0].b, packets.size() * ts::PKT_SIZE, ts::UString::HEXA | ts::UString::OFFSET | ts::UString::ASCII, 2);
        return false;
    }

    // Then compare contents of packets
    for (size_t pi = 0; pi < packets.size(); ++pi) {
        const uint8_t* ref = ref_packets + pi * ts::PKT_SIZE;
        const uint8_t* pkt = packets[pi].b;
        for (size_t i = 0; i < ts::PKT_SIZE; ++i) {
            if (pkt[i] != ref[i]) {
                utest::Out() << "DemuxTest: " << test_name << ", " << table_name
                             << ": difference at offset " << i << " in packet " << pi
                             << std::endl
                             << "DemuxTest: Reference packet:" << std::endl
                             << ts::UString::Dump(ref, ts::PKT_SIZE, ts::UString::HEXA | ts::UString::OFFSET | ts::UString::ASCII, 2)
                             << "DemuxTest: " << table_name << ":" << std::endl
                             << ts::UString::Dump(pkt, ts::PKT_SIZE, ts::UString::HEXA | ts::UString::OFFSET | ts::UString::ASCII, 2);
                return false;
            }
        }
    }
    return true;
}

// Unitary test for one table.
void DemuxTest::testTable(const char* name, const uint8_t* ref_packets, size_t ref_packets_size, const uint8_t* ref_sections, size_t ref_sections_size)
{
    CPPUNIT_ASSERT(ref_packets_size % ts::PKT_SIZE == 0);
    utest::Out() << "DemuxTest: Testing " << name << std::endl;

    // Analyze TS packets. We expect only one table

    const ts::TSPacket* ref_pkt = reinterpret_cast<const ts::TSPacket*>(ref_packets);
    ts::StandaloneTableDemux demux(ts::AllPIDs);

    for (size_t pi = 0; pi < ref_packets_size / ts::PKT_SIZE; ++pi) {
        demux.feedPacket(ref_pkt[pi]);
    }
    CPPUNIT_ASSERT_EQUAL(size_t(1), demux.tableCount());

    // Compare contents of reference sections and demuxed sections.

    const ts::BinaryTable& table1(*demux.tableAt(0));
    utest::Out() << "DemuxTest: " << ts::UString::Format(u"  PID %d (0x%X)", {table1.sourcePID(), table1.sourcePID()}) << std::endl;
    CPPUNIT_ASSERT(checkSections(name, "demuxed table", table1, ref_sections, ref_sections_size));

    // Table-specific tests.
    // Check known values in the test tables.
    // Reserialize the table

    ts::BinaryTable table2;

    switch (table1.tableId()) {
        case ts::TID_PAT: { // TNT R4
            ts::PAT pat(table1);
            CPPUNIT_ASSERT(pat.ts_id == 0x0004);
            CPPUNIT_ASSERT(pat.nit_pid == 0x0010);
            CPPUNIT_ASSERT(pat.pmts.size() == 7);
            CPPUNIT_ASSERT(pat.pmts[0x0403] == 0x0136);
            pat.serialize(table2);
            break;
        }
        case ts::TID_CAT: { // TNT R3 or R6
            ts::CAT cat(table1);
            CPPUNIT_ASSERT(cat.descs.count() == 1 || cat.descs.count() == 2);
            cat.serialize(table2);
            break;
        }
        case ts::TID_PMT: { // Planete (TNT R3) or HEVC
            ts::PMT pmt(table1);
            switch (pmt.service_id) {
                case 0x0304: { // Planete
                    CPPUNIT_ASSERT(pmt.pcr_pid == 0x00A3);
                    CPPUNIT_ASSERT(pmt.descs.count() == 1);
                    CPPUNIT_ASSERT(pmt.descs[0]->tag() == ts::DID_CA);
                    CPPUNIT_ASSERT(pmt.streams.size() == 2);
                    CPPUNIT_ASSERT(pmt.streams[0x00A3].stream_type == 0x1B);
                    CPPUNIT_ASSERT(pmt.streams[0x00A3].descs.count() == 3);
                    CPPUNIT_ASSERT(pmt.streams[0x005C].stream_type == 0x04);
                    CPPUNIT_ASSERT(pmt.streams[0x005C].descs.count() == 3);
                    break;
                }
                case 0x11FB: { // HEVC
                    CPPUNIT_ASSERT(pmt.pcr_pid == 0x01C9);
                    CPPUNIT_ASSERT(pmt.descs.count() == 0);
                    CPPUNIT_ASSERT(pmt.streams.size() == 2);
                    CPPUNIT_ASSERT(pmt.streams[0x01C9].stream_type == 0x24);
                    CPPUNIT_ASSERT(pmt.streams[0x01C9].descs.count() == 1);
                    CPPUNIT_ASSERT(pmt.streams[0x01C9].descs[0]->tag() == ts::DID_HEVC_VIDEO);
                    CPPUNIT_ASSERT(pmt.streams[0x01CA].stream_type == 0x0F);
                    CPPUNIT_ASSERT(pmt.streams[0x01CA].descs.count() == 2);
                    break;
                }
                default: {
                    CPPUNIT_FAIL("unexpected service id");
                }
            }
            pmt.serialize(table2);
            break;
        }
        case ts::TID_SDT_ACT: { // TNT R3
            ts::SDT sdt(table1);
            CPPUNIT_ASSERT(sdt.ts_id == 0x0003);
            CPPUNIT_ASSERT(sdt.onetw_id == 0x20FA);
            CPPUNIT_ASSERT(sdt.services.size() == 8);
            CPPUNIT_ASSERT(sdt.services[0x0304].EITpf_present);
            CPPUNIT_ASSERT(!sdt.services[0x0304].EITs_present);
            CPPUNIT_ASSERT(sdt.services[0x0304].running_status == 4); // running
            CPPUNIT_ASSERT(sdt.services[0x0304].CA_controlled);
            CPPUNIT_ASSERT(sdt.services[0x0304].descs.count() == 1);
            CPPUNIT_ASSERT(sdt.services[0x0304].descs[0]->tag() == ts::DID_SERVICE);
            CPPUNIT_ASSERT(sdt.services[0x0304].serviceType() == 0x01);
            CPPUNIT_ASSERT(sdt.services[0x0304].serviceName() == u"PLANETE");
            CPPUNIT_ASSERT(sdt.services[0x0304].providerName() == u"CNH");
            sdt.serialize(table2);
            break;
        }
        case ts::TID_NIT_ACT: { // TNT v23
            ts::NIT nit(table1);
            CPPUNIT_ASSERT(nit.network_id == 0x20FA);
            CPPUNIT_ASSERT(nit.descs.count() == 8);
            CPPUNIT_ASSERT(nit.descs[0]->tag() == ts::DID_NETWORK_NAME);
            CPPUNIT_ASSERT(nit.descs[7]->tag() == ts::DID_LINKAGE);
            CPPUNIT_ASSERT(nit.transports.size() == 7);
            ts::TransportStreamId id(0x0004, 0x20FA); // TNT R4
            CPPUNIT_ASSERT(nit.transports[id].descs.count() == 4);
            CPPUNIT_ASSERT(nit.transports[id].descs[0]->tag() == ts::DID_PRIV_DATA_SPECIF);
            CPPUNIT_ASSERT(nit.transports[id].descs[3]->tag() == ts::DID_TERREST_DELIVERY);
            nit.serialize(table2);
            break;
        }
        case ts::TID_BAT: { // Tv Numeric or Canal+ TNT
            ts::BAT bat(table1);
            switch (bat.bouquet_id) {
                case 0x0086: { // Tv Numeric
                    CPPUNIT_ASSERT(bat.descs.count() == 5);
                    CPPUNIT_ASSERT(bat.descs[0]->tag() == ts::DID_BOUQUET_NAME);
                    CPPUNIT_ASSERT(bat.descs[4]->tag() == ts::DID_LW_SUBSCRIPTION);
                    CPPUNIT_ASSERT(bat.transports.size() == 3);
                    ts::TransportStreamId id(0x0006, 0x20FA); // TNT R6
                    CPPUNIT_ASSERT(bat.transports[id].descs.count() == 1);
                    CPPUNIT_ASSERT(bat.transports[id].descs[0]->tag() == ts::DID_SERVICE_LIST);
                    break;
                }
                case 0xC003: { // Canal+ TNT
                    CPPUNIT_ASSERT(bat.descs.count() == 4);
                    CPPUNIT_ASSERT(bat.descs[0]->tag() == ts::DID_BOUQUET_NAME);
                    CPPUNIT_ASSERT(bat.descs[1]->tag() == ts::DID_LINKAGE);
                    CPPUNIT_ASSERT(bat.transports.size() == 6);
                    ts::TransportStreamId id(0x0003, 0x20FA); // TNT R3
                    CPPUNIT_ASSERT(bat.transports[id].descs.count() == 5);
                    CPPUNIT_ASSERT(bat.transports[id].descs[0]->tag() == ts::DID_SERVICE_LIST);
                    break;
                }
                default: {
                    CPPUNIT_FAIL("unexpected bouquet id");
                }
            }
            bat.serialize(table2);
            break;
        }
        case ts::TID_TDT: { // TNT
            ts::TDT tdt(table1);
            CPPUNIT_ASSERT(tdt.utc_time == ts::Time(2007, 11, 23, 13, 25, 03));
            tdt.serialize(table2);
            break;
        }
        case ts::TID_TOT: { // TNT
            ts::TOT tot(table1);
            CPPUNIT_ASSERT(tot.utc_time == ts::Time(2007, 11, 23, 13, 25, 14));
            CPPUNIT_ASSERT(tot.regions.size() == 1);
            CPPUNIT_ASSERT(tot.descs.count() == 0);
            CPPUNIT_ASSERT(tot.regions[0].country == u"FRA");
            CPPUNIT_ASSERT(tot.regions[0].region_id == 0);
            CPPUNIT_ASSERT(tot.regions[0].time_offset == 60);
            CPPUNIT_ASSERT(tot.regions[0].next_change == ts::Time(2008, 3, 30, 1, 0, 0));
            CPPUNIT_ASSERT(tot.regions[0].next_time_offset == 120);
            tot.serialize(table2);
            break;
        }
        default: {
            CPPUNIT_FAIL("unexpected table id");
        }
    }

    // Now we have:
    //   BinaryTable table1  -> as demuxed from referenced packets
    //   BinaryTable table2  -> deserialized/check/serialized from table1
    //
    // It is not valid to compare the two binary tables. The
    // deserialization / serialization process may have changed the
    // order of some elements.

    // Repacketize table1 and check that the packets are identical to
    // the reference packets.

    ts::TSPacketVector packets;
    ts::OneShotPacketizer pzer(table1.sourcePID(), true);

    pzer.setNextContinuityCounter(ref_pkt[0].getCC());
    pzer.addTable(table1);
    pzer.getPackets(packets);

    CPPUNIT_ASSERT(checkPackets(name, "rebuilt packets", packets, ref_packets, ref_packets_size));

    // Packetize the serialized table

    pzer.reset();
    pzer.addTable(table2);
    pzer.getPackets(packets);

    // Reanalyze the packetized table and check it is identical to table2

    ts::StandaloneTableDemux demux2(ts::AllPIDs);

    for (ts::TSPacketVector::const_iterator it = packets.begin(); it != packets.end(); ++it) {
        demux2.feedPacket(*it);
    }
    CPPUNIT_ASSERT_EQUAL(size_t(1), demux2.tableCount());

    const ts::BinaryTable& table3(*demux2.tableAt(0));
    if (table2 != table3) {
        utest::Out() << "DemuxTest: " << name << ": rebuilt tables differ" << std::endl;
        utest::Out() << "DemuxTest:   Re-serialized table: " << ts::names::TID(table2.tableId())
            << ", " << table2.sectionCount() << " sections" << std::endl
            << "  Re-packetized table: " << ts::names::TID(table3.tableId())
            << ", " << table3.sectionCount() << " sections" << std::endl;
    }
    CPPUNIT_ASSERT(table2 == table3);
}

#define TEST_TABLE(title,name) testTable(title, \
         psi_##name##_packets, sizeof(psi_##name##_packets), \
         psi_##name##_sections, sizeof(psi_##name##_sections));

void DemuxTest::testPAT()
{
    TEST_TABLE("PAT: TNT R4", pat_r4);
}

void DemuxTest::testCATR3()
{
    TEST_TABLE("CAT: TNT R3", cat_r3);
}

void DemuxTest::testCATR6()
{
    TEST_TABLE("CAT: TNT R6", cat_r6);
}

void DemuxTest::testPMT()
{
    TEST_TABLE("PMT: Planete (TNT R3)", pmt_planete);
}

void DemuxTest::testSDT()
{
    TEST_TABLE("SDT: TNT R3", sdt_r3);
}

void DemuxTest::testNIT()
{
    TEST_TABLE("NIT: TNT v23", nit_tntv23);
}

void DemuxTest::testBATTvNumeric()
{
    TEST_TABLE("BAT: Tv Numeric", bat_tvnum);
}

void DemuxTest::testBATCanalPlus()
{
    TEST_TABLE("BAT: Canal+ TNT", bat_cplus);
}

void DemuxTest::testTDT()
{
    TEST_TABLE("TDT: TNT", tdt_tnt);
}

void DemuxTest::testTOT()
{
    TEST_TABLE("TOT: TNT", tot_tnt);
}

void DemuxTest::testHEVC()
{
    TEST_TABLE("PMT with HEVC descriptor", pmt_hevc);
}
