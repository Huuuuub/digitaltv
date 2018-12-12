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
//  CppUnit test suite for class ts::DoubleCheckLock
//
//----------------------------------------------------------------------------

#include "tsDoubleCheckLock.h"
#include "utestCppUnitTest.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// The test fixture
//----------------------------------------------------------------------------

class DoubleCheckLockTest: public CppUnit::TestFixture
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void testDoubleCheckLock();

    CPPUNIT_TEST_SUITE (DoubleCheckLockTest);
    CPPUNIT_TEST (testDoubleCheckLock);
    CPPUNIT_TEST_SUITE_END ();
};

CPPUNIT_TEST_SUITE_REGISTRATION (DoubleCheckLockTest);


//----------------------------------------------------------------------------
// Initialization.
//----------------------------------------------------------------------------

// Test suite initialization method.
void DoubleCheckLockTest::setUp()
{
}

// Test suite cleanup method.
void DoubleCheckLockTest::tearDown()
{
}


//----------------------------------------------------------------------------
// Unitary tests.
//----------------------------------------------------------------------------

void DoubleCheckLockTest::testDoubleCheckLock()
{
    TS_UNUSED int data = 0;
    ts::DoubleCheckLock lock;

    CPPUNIT_ASSERT(!lock.changed());

    // Writer
    {
        ts::DoubleCheckLock::Writer guard(lock);
        data = 1;
    }
    CPPUNIT_ASSERT(lock.changed());

    // Reader
    bool reader = false;
    if (lock.changed()) {
        ts::DoubleCheckLock::Reader guard(lock);
        reader = true;
    }
    CPPUNIT_ASSERT(reader);
    CPPUNIT_ASSERT(!lock.changed());

    // Reader
    if (lock.changed()) {
        ts::DoubleCheckLock::Reader guard(lock);
        CPPUNIT_FAIL("should not get there, data not updated");
    }
    CPPUNIT_ASSERT(!lock.changed());
}
