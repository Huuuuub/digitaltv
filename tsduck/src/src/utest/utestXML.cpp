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
//  CppUnit test suite for XML classes.
//
//----------------------------------------------------------------------------

#include "tsxmlDocument.h"
#include "tsxmlElement.h"
#include "tsTextFormatter.h"
#include "tsCerrReport.h"
#include "tsReportBuffer.h"
#include "tsSysUtils.h"
#include "utestCppUnitTest.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// The test fixture
//----------------------------------------------------------------------------

class XMLTest: public CppUnit::TestFixture
{
public:
    XMLTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testDocument();
    void testInvalid();
    void testFileBOM();
    void testValidation();
    void testCreation();
    void testKeepOpen();
    void testEscape();
    void testTweaks();

    CPPUNIT_TEST_SUITE(XMLTest);
    CPPUNIT_TEST(testDocument);
    CPPUNIT_TEST(testInvalid);
    CPPUNIT_TEST(testFileBOM);
    CPPUNIT_TEST(testValidation);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testKeepOpen);
    CPPUNIT_TEST(testEscape);
    CPPUNIT_TEST(testTweaks);
    CPPUNIT_TEST_SUITE_END();

private:
    ts::UString _tempFileName;
    ts::Report& report();
};

CPPUNIT_TEST_SUITE_REGISTRATION(XMLTest);


//----------------------------------------------------------------------------
// Initialization.
//----------------------------------------------------------------------------

// Constructor.
XMLTest::XMLTest() :
    _tempFileName(ts::TempFile(u".tmp.xml"))
{
}

// Test suite initialization method.
void XMLTest::setUp()
{
    ts::DeleteFile(_tempFileName);
}

// Test suite cleanup method.
void XMLTest::tearDown()
{
    ts::DeleteFile(_tempFileName);
}

ts::Report& XMLTest::report()
{
    if (utest::DebugMode()) {
        return CERR;
    }
    else {
        return NULLREP;
    }
}


//----------------------------------------------------------------------------
// Unitary tests.
//----------------------------------------------------------------------------

void XMLTest::testDocument()
{
    static const ts::UChar* const document =
        u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        u"<root attr1=\"val1\">\n"
        u"  <node1 a1=\"v1\" a2=\"v2\">Text in node1</node1>\n"
        u"  <node2 b1=\"x1\">Text in node2</node2>\n"
        u"  <node3 foo=\"bar\"/>\n"
        u"  <node4/>\n"
        u"</root>\n";

    ts::xml::Document doc(report());
    CPPUNIT_ASSERT(doc.parse(document));
    CPPUNIT_ASSERT(doc.hasChildren());
    CPPUNIT_ASSERT_EQUAL(size_t(2), doc.childrenCount());

    ts::xml::Element* root = doc.rootElement();
    CPPUNIT_ASSERT(root != nullptr);
    CPPUNIT_ASSERT(root->hasChildren());
    CPPUNIT_ASSERT_EQUAL(size_t(4), root->childrenCount());
    CPPUNIT_ASSERT(root->hasAttribute(u"attr1"));
    CPPUNIT_ASSERT(root->hasAttribute(u"AttR1"));
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"root", root->name());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"val1", root->attribute(u"attr1").value());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"val1", root->attribute(u"AtTr1").value());
    CPPUNIT_ASSERT(!root->hasAttribute(u"nonexistent"));
    CPPUNIT_ASSERT(!root->attribute(u"nonexistent", true).isValid());
    CPPUNIT_ASSERT(root->attribute(u"nonexistent", true).value().empty());
    CPPUNIT_ASSERT(root->attribute(u"nonexistent", true).name().empty());

    ts::xml::Element* elem = root->firstChildElement();
    CPPUNIT_ASSERT(elem != nullptr);
    CPPUNIT_ASSERT(elem->hasChildren());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"node1", elem->name());
    CPPUNIT_ASSERT(elem->hasAttribute(u"a1"));
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"v1", elem->attribute(u"a1").value());
    CPPUNIT_ASSERT(elem->hasAttribute(u"a2"));
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"v2", elem->attribute(u"a2").value());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"Text in node1", elem->text());

    elem = elem->nextSiblingElement();
    CPPUNIT_ASSERT(elem != nullptr);
    CPPUNIT_ASSERT(elem->hasChildren());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"node2", elem->name());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"x1", elem->attribute(u"b1").value());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"Text in node2", elem->text());

    elem = elem->nextSiblingElement();
    CPPUNIT_ASSERT(elem != nullptr);
    CPPUNIT_ASSERT(!elem->hasChildren());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"node3", elem->name());
    CPPUNIT_ASSERT(elem->hasAttribute(u"foo"));
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"bar", elem->attribute(u"foo").value());
    CPPUNIT_ASSERT(elem->text().empty());

    elem = elem->nextSiblingElement();
    CPPUNIT_ASSERT(elem != nullptr);
    CPPUNIT_ASSERT(!elem->hasChildren());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"node4", elem->name());
    CPPUNIT_ASSERT(!elem->hasAttribute(u"foo"));
    CPPUNIT_ASSERT(elem->text().empty());

    elem = elem->nextSiblingElement();
    CPPUNIT_ASSERT(elem == nullptr);
}

void XMLTest::testInvalid()
{
    // Incorrect XML document
    static const ts::UChar* xmlContent =
        u"<?xml version='1.0' encoding='UTF-8'?>\n"
        u"<foo>\n"
        u"</bar>";

    ts::ReportBuffer<> rep;
    ts::xml::Document doc(rep);
    CPPUNIT_ASSERT(!doc.parse(xmlContent));
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"Error: line 3: parsing error, expected </foo> to match <foo> at line 2", rep.getMessages());
}

void XMLTest::testFileBOM()
{
    // Binary content of XML file with BOM, accented characters and HTML entities.
    const ts::ByteBlock fileData({
        0xEF, 0xBB, 0xBF, 0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E,
        0x3D, 0x27, 0x31, 0x2E, 0x30, 0x27, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x69, 0x6E, 0x67, 0x3D,
        0x27, 0x55, 0x54, 0x46, 0x2D, 0x38, 0x27, 0x3F, 0x3E, 0x0A, 0x3C, 0x66, 0x6F, 0x6F, 0x3E, 0x0A,
        0x20, 0x20, 0x3C, 0x62, 0xC3, 0xA0, 0x41, 0xC3, 0xA7, 0x20, 0x66, 0xC3, 0xB9, 0x3D, 0x22, 0x63,
        0xC3, 0xA9, 0x22, 0x3E, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x66, 0x26, 0x6C, 0x74, 0x3B, 0x26, 0x67,
        0x74, 0x3B, 0x0A, 0x20, 0x20, 0x3C, 0x2F, 0x42, 0xC3, 0x80, 0x41, 0xC3, 0x87, 0x3E, 0x0A, 0x3C,
        0x2F, 0x66, 0x6F, 0x6F, 0x3E, 0x0A,
    });

    const ts::UString rootName(u"foo");
    const ts::UString childName({u'b', ts::LATIN_SMALL_LETTER_A_WITH_GRAVE, u'A', ts::LATIN_SMALL_LETTER_C_WITH_CEDILLA});
    const ts::UString childAttrName({u'f', ts::LATIN_SMALL_LETTER_U_WITH_GRAVE});
    const ts::UString childAttrValue({u'c', ts::LATIN_SMALL_LETTER_E_WITH_ACUTE});
    const ts::UString childText1(u"\n    f<>\n  ");
    const ts::UString childText2(u"f<>");

    CPPUNIT_ASSERT(fileData.saveToFile(_tempFileName, &report()));

    ts::xml::Document doc(report());
    CPPUNIT_ASSERT(doc.load(_tempFileName));

    ts::xml::Element* root = doc.rootElement();
    CPPUNIT_ASSERT_EQUAL(size_t(2), doc.childrenCount());
    CPPUNIT_ASSERT(root != nullptr);
    CPPUNIT_ASSERT_EQUAL(size_t(1), root->childrenCount());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(rootName, root->name());

    ts::xml::Element* elem = root->firstChildElement();
    CPPUNIT_ASSERT(elem != nullptr);
    CPPUNIT_ASSERT_USTRINGS_EQUAL(childName, elem->name());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(childAttrName, elem->attribute(childAttrName).name());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(childAttrValue, elem->attribute(childAttrName).value());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(childText1, elem->text(false));
    CPPUNIT_ASSERT_USTRINGS_EQUAL(childText2, elem->text(true));

    CPPUNIT_ASSERT(ts::DeleteFile(_tempFileName) == ts::SYS_SUCCESS);
}

void XMLTest::testValidation()
{
    ts::xml::Document model(report());
    CPPUNIT_ASSERT(model.load(u"tsduck.xml"));

    const ts::UString xmlContent(
        u"<?xml version='1.0' encoding='UTF-8'?>\n"
        u"<tsduck>\n"
        u"  <PAT version='2' transport_stream_id='27'>\n"
        u"    <service service_id='1' program_map_PID='1000'/>\n"
        u"    <service service_id='2' program_map_PID='2000'/>\n"
        u"    <service service_id='3' program_map_PID='3000'/>\n"
        u"  </PAT>\n"
        u"  <PMT version='3' service_id='789' PCR_PID='3004'>\n"
        u"    <CA_descriptor CA_system_id='500' CA_PID='3005'>\n"
        u"      <private_data>00 01 02 03 04</private_data>\n"
        u"    </CA_descriptor>\n"
        u"    <component stream_type='0x04' elementary_PID='3006'>\n"
        u"      <ca_descriptor ca_system_id='500' ca_PID='3007'>\n"
        u"        <private_data>10 11 12 13 14 15</private_data>\n"
        u"      </ca_descriptor>\n"
        u"    </component>\n"
        u"  </PMT>\n"
        u"</tsduck>");

    ts::xml::Document doc(report());
    CPPUNIT_ASSERT(doc.parse(xmlContent));
    CPPUNIT_ASSERT(doc.validate(model));
}

void XMLTest::testCreation()
{
    ts::xml::Document doc(report());
    ts::xml::Element* child1 = nullptr;
    ts::xml::Element* child2 = nullptr;
    ts::xml::Element* subchild2 = nullptr;

    ts::xml::Element* root = doc.initialize(u"theRoot");
    CPPUNIT_ASSERT(root != nullptr);
    CPPUNIT_ASSERT_EQUAL(size_t(0), doc.depth());
    CPPUNIT_ASSERT_EQUAL(size_t(1), root->depth());

    CPPUNIT_ASSERT((child1 = root->addElement(u"child1")) != nullptr);
    CPPUNIT_ASSERT_EQUAL(size_t(2), child1->depth());
    child1->setAttribute(u"str", u"a string");
    child1->setIntAttribute(u"int", -47);
    CPPUNIT_ASSERT(child1->addElement(u"subChild1") != nullptr);
    CPPUNIT_ASSERT((subchild2 = child1->addElement(u"subChild2")) != nullptr);
    subchild2->setIntAttribute(u"int64", TS_CONST64(0x7FFFFFFFFFFFFFFF));

    CPPUNIT_ASSERT((child2 = root->addElement(u"child2")) != nullptr);
    CPPUNIT_ASSERT(child2->addElement(u"fooBar") != nullptr);

    ts::UString str;
    CPPUNIT_ASSERT(child1->getAttribute(str, u"str", true));
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"a string", str);

    int i;
    CPPUNIT_ASSERT(child1->getIntAttribute(i, u"int", true));
    CPPUNIT_ASSERT_EQUAL(-47, i);

    int64_t i64;
    CPPUNIT_ASSERT(subchild2->getIntAttribute(i64, u"int64", true));
    CPPUNIT_ASSERT_EQUAL(TS_CONST64(0x7FFFFFFFFFFFFFFF), i64);

    CPPUNIT_ASSERT_USTRINGS_EQUAL(
        u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        u"<theRoot>\n"
        u"  <child1 str=\"a string\" int=\"-47\">\n"
        u"    <subChild1/>\n"
        u"    <subChild2 int64=\"9,223,372,036,854,775,807\"/>\n"
        u"  </child1>\n"
        u"  <child2>\n"
        u"    <fooBar/>\n"
        u"  </child2>\n"
        u"</theRoot>\n",
        doc.toString());
}

void XMLTest::testKeepOpen()
{
    static const ts::UChar* const document =
        u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        u"<root attr1=\"val1\">\n"
        u"  <node1>  Text in node1  </node1>\n"
        u"  <node2>\n"
        u"    <node21>\n"
        u"      <node211/>\n"
        u"    </node21>\n"
        u"    <node22/>\n"
        u"  </node2>\n"
        u"  <node3 foo=\"bar\"/>\n"
        u"  <node4/>\n"
        u"</root>\n";

    ts::xml::Document doc(report());
    CPPUNIT_ASSERT(doc.parse(document));

    ts::xml::Element* root = doc.rootElement();
    CPPUNIT_ASSERT(root != nullptr);

    ts::xml::Element* node2 = root->findFirstChild(u"NODE2");
    CPPUNIT_ASSERT(node2 != nullptr);
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"node2", node2->name());

    ts::TextFormatter out(report());
    node2->print(out.setString());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(
        u"<node2>\n"
        u"  <node21>\n"
        u"    <node211/>\n"
        u"  </node21>\n"
        u"  <node22/>\n"
        u"</node2>",
        out.toString());

    node2->print(out.setString(), true);
    CPPUNIT_ASSERT_USTRINGS_EQUAL(
        u"<node2>\n"
        u"  <node21>\n"
        u"    <node211/>\n"
        u"  </node21>\n"
        u"  <node22/>\n",
        out.toString());

    node2->printClose(out, 1);
    CPPUNIT_ASSERT_USTRINGS_EQUAL(
        u"<node2>\n"
        u"  <node21>\n"
        u"    <node211/>\n"
        u"  </node21>\n"
        u"  <node22/>\n"
        u"</node2>\n",
        out.toString());
}

void XMLTest::testEscape()
{
    ts::xml::Document doc(report());
    ts::xml::Element* child1 = nullptr;
    ts::xml::Element* child2 = nullptr;

    ts::xml::Element* root = doc.initialize(u"theRoot");
    CPPUNIT_ASSERT(root != nullptr);
    CPPUNIT_ASSERT_EQUAL(size_t(0), doc.depth());
    CPPUNIT_ASSERT_EQUAL(size_t(1), root->depth());

    CPPUNIT_ASSERT((child1 = root->addElement(u"child1")) != nullptr);
    CPPUNIT_ASSERT_EQUAL(size_t(2), child1->depth());
    child1->setAttribute(u"str", u"ab&<>'\"cd");

    CPPUNIT_ASSERT((child2 = root->addElement(u"child2")) != nullptr);
    CPPUNIT_ASSERT(child2->addText(u"text<&'\">text") != nullptr);

    const ts::UString text(doc.toString());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(
        u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        u"<theRoot>\n"
        u"  <child1 str=\"ab&amp;&lt;&gt;&apos;&quot;cd\"/>\n"
        u"  <child2>text&lt;&amp;'\"&gt;text</child2>\n"
        u"</theRoot>\n",
        text);

    ts::xml::Document doc2(report());
    CPPUNIT_ASSERT(doc2.parse(text));
    CPPUNIT_ASSERT(doc2.hasChildren());
    CPPUNIT_ASSERT_EQUAL(size_t(2), doc2.childrenCount());

    ts::xml::Element* root2 = doc2.rootElement();
    CPPUNIT_ASSERT(root2 != nullptr);
    CPPUNIT_ASSERT(root2->hasChildren());
    CPPUNIT_ASSERT_EQUAL(size_t(2), root2->childrenCount());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"theRoot", root2->name());

    ts::xml::Element* elem = root2->firstChildElement();
    CPPUNIT_ASSERT(elem != nullptr);
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"child1", elem->name());
    CPPUNIT_ASSERT(elem->hasAttribute(u"str"));
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"ab&<>'\"cd", elem->attribute(u"str").value());

    elem = elem->nextSiblingElement();
    CPPUNIT_ASSERT(elem != nullptr);
    CPPUNIT_ASSERT(elem->hasChildren());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"child2", elem->name());
    CPPUNIT_ASSERT_USTRINGS_EQUAL(u"text<&'\">text", elem->text());
}

void XMLTest::testTweaks()
{
    ts::xml::Document doc(report());
    ts::xml::Element* root = doc.initialize(u"root");
    CPPUNIT_ASSERT(root != nullptr);
    root->setAttribute(u"a1", u"foo");
    root->setAttribute(u"a2", u"ab&<>'\"cd");
    root->setAttribute(u"a3", u"ef\"gh");
    root->setAttribute(u"a4", u"ij'kl");
    CPPUNIT_ASSERT(root->addText(u"text<&'\">text") != nullptr);

    ts::xml::Tweaks tweaks; // default values
    doc.setTweaks(tweaks);

    CPPUNIT_ASSERT_USTRINGS_EQUAL(
        u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        u"<root a1=\"foo\" a2=\"ab&amp;&lt;&gt;&apos;&quot;cd\" a3=\"ef&quot;gh\" a4=\"ij&apos;kl\">text&lt;&amp;'\"&gt;text</root>\n",
        doc.toString());

    tweaks.strictAttributeFormatting = true;
    tweaks.strictTextNodeFormatting = true;
    doc.setTweaks(tweaks);

    CPPUNIT_ASSERT_USTRINGS_EQUAL(
        u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        u"<root a1=\"foo\" a2=\"ab&amp;&lt;&gt;&apos;&quot;cd\" a3=\"ef&quot;gh\" a4=\"ij&apos;kl\">text&lt;&amp;&apos;&quot;&gt;text</root>\n",
        doc.toString());

    tweaks.strictAttributeFormatting = false;
    tweaks.strictTextNodeFormatting = true;
    doc.setTweaks(tweaks);

    CPPUNIT_ASSERT_USTRINGS_EQUAL(
        u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        u"<root a1=\"foo\" a2=\"ab&amp;<>'&quot;cd\" a3='ef\"gh' a4=\"ij'kl\">text&lt;&amp;&apos;&quot;&gt;text</root>\n",
        doc.toString());

    tweaks.strictAttributeFormatting = false;
    tweaks.strictTextNodeFormatting = false;
    doc.setTweaks(tweaks);

    CPPUNIT_ASSERT_USTRINGS_EQUAL(
        u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        u"<root a1=\"foo\" a2=\"ab&amp;<>'&quot;cd\" a3='ef\"gh' a4=\"ij'kl\">text&lt;&amp;'\"&gt;text</root>\n",
        doc.toString());
}
