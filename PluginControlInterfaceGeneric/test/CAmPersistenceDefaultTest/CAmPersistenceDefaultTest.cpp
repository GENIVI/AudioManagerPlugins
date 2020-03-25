/*******************************************************************************
 *  \copyright (c) 2018 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com> 2018
 *           Kapildev Patel, Yogesh Sharma <kpatel@jp.adit-jv.com> 2018
 *
 *  \copyright The MIT License (MIT)
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 ******************************************************************************/

#include "CAmPersistenceDefaultTest.h"

#include "CAmLogger.h"
#include "CAmPersistenceDefault.h"
#include "CAmTestConfigurations.h"

#define LIBXML_C14N_ENABLED
#define LIBXML_OUTPUT_ENABLED
#include <libxml/c14n.h>

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

#define APPLICATION_NAME                  "genericController"

namespace gc_utest
{

CAmPersistenceDefaultTest::OuterPersistence::OuterPersistence(void)
    : _innerPath(GENERIC_CONTROLLER_PERSISTENCE_FILE)
{
    // memorize current configuration settings
    const char* path = getenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME);
    if (path)
    {
        _outerPath = path;
        _innerPath = path;
    }

    // memorize previous persistence data
    _outerXml = static_cast<std::ostringstream&>
        (std::ostringstream{} << std::ifstream{_innerPath}.rdbuf()).str();
}

CAmPersistenceDefaultTest::OuterPersistence::~OuterPersistence()
{
    // undo temporary configuration redirection
    if (_outerPath.empty())
    {
        unsetenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME);
    }
    else
    {
        setenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME, _outerPath.c_str(), true);
    }

    if (!_outerXml.empty())  // nested replacement
    {
        // restore previous persistence data
        ofstream(_innerPath) << _outerXml;
    }
}

/**
 *  Reference persistence data file for later comparison, given in canonical form (C14N)
 */
#define XML_HEADER "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
const string refXml
    = XML_HEADER
    "<persistence>\n"
    "  <systemProperty type=\"61440\" value=\"4\"></systemProperty>\n"
    "  <systemProperty type=\"61569\" value=\"100\"></systemProperty>\n"
    "  <class name=\"test-class\" volume=\"-32768\">\n"
    "    <stack sink=\"Amplifier\" volume=\"-32768\">\n"
    "      <connection lastconvalid=\"1\" order=\"-1\" source=\"MediaPlayer\" volume=\"3\"></connection>\n"
    "    </stack>\n"
    "    <mainSoundProperties sink=\"\" source=\"MediaPlayer\">\n"
    "      <mainSoundProperty type=\"1\" value=\"3\"></mainSoundProperty>\n"
    "      <mainSoundProperty type=\"2\" value=\"4\"></mainSoundProperty>\n"
    "    </mainSoundProperties>\n"
    "    <mainSoundProperties sink=\"Amplifier\" source=\"\">\n"
    "      <mainSoundProperty type=\"3\" value=\"5\"></mainSoundProperty>\n"
    "    </mainSoundProperties>\n"
    "  </class>\n"
    "</persistence>";

const string dummyXml
  = XML_HEADER
    "<persistence>\n"
    "  <systemProperty type=\"61440\" value=\"0\"/>\n"
    "</persistence>\n";

/**
 *@Class : CAmPersistenceDefaultTest
 *@brief : This class is used to test the CAmPersistenceDefault class functionality.
 */
CAmPersistenceDefaultTest::CAmPersistenceDefaultTest()
    : mDataFile(GENERIC_CONTROLLER_PERSISTENCE_FILE)
    , mpOuterData(NULL)
{
    const char* pPath = getenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME);
    if (pPath)
    {
        mDataFile = pPath;
    }
}

CAmPersistenceDefaultTest::~CAmPersistenceDefaultTest()
{
}

void CAmPersistenceDefaultTest::SetUp()
{
    // memorize current persistence data
    mpOuterData = new OuterPersistence();
    ASSERT_NE(nullptr, mpOuterData);

    // fake some dummy data
    ofstream(mDataFile) << dummyXml;
}

void CAmPersistenceDefaultTest::TearDown()
{
    // restore previous persistent data
    delete mpOuterData;
    mpOuterData = NULL;
}

/**
 * @brief : Test to verify the Open function for Positive scenario
 *
 * @test : verify the "Open" function when appName is given as input parameter, and set enviroment variable then will check whether Open function
 *         will open the file to copy the data into the map or not.
 *
 * @result : "Pass" when Open function return "E_OK" without any Gmock error message
 */
TEST_F(CAmPersistenceDefaultTest, OpenPositive)
{
    CAmPersistenceDefault persistenceDefault;

    setenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME, mDataFile.c_str(), true);
    EXPECT_EQ(E_OK, persistenceDefault.open(APPLICATION_NAME));
}

/**
 * @brief : Test to verify the Open function with incorrect directory name for Negative scenario
 *
 * @test : verify the "Open" function when appName is given as input parameter, and set environment variable with incorrect directory name
 *         then will check whether Open function will open the file to copy the data into the map or not.
 *
 * @result : "Pass" when Open function return "E_DATABASE_ERROR" without any Gmock error message
 */
TEST_F(CAmPersistenceDefaultTest, OpenNegativeWrongDirName)
{
    CAmPersistenceDefault persistenceDefault;

    setenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME, "/home", true);
    EXPECT_EQ(E_NON_EXISTENT, persistenceDefault.open(APPLICATION_NAME));
}

/**
 * @brief Test to verify the Open function with blank directory name for Negative scenario
 *
 * @test  verify the "Open" function and set environment variable with blank directory name
 *        then will check return value indicating the failure.
 */

TEST_F(CAmPersistenceDefaultTest, OpenNegativeVarDirNot)
{
    setenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME, " ", true);

    CAmPersistenceDefault persistenceDefault;
    EXPECT_EQ(E_NON_EXISTENT, persistenceDefault.open(APPLICATION_NAME));
}

/**
 * @brief  Test to verify the store capability for Positive scenario
 *
 * @test  verify the "_writeTOXML()" function which is invoked from destructor.
 *        Test passes if produced XML matches the reference XML
 */
TEST_F(CAmPersistenceDefaultTest, storePositive)
{
    setenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME, mDataFile.c_str(), true);
    {
        CAmPersistenceDefault persistenceDefault;
        EXPECT_EQ(E_OK, persistenceDefault.open(APPLICATION_NAME));

        // populate the document with valid data to compose the refXml
        const std::string writeData = "{test-class,MediaPlayer:Amplifier;}";
        EXPECT_EQ(E_OK, persistenceDefault.write("lastMainConnection", writeData, writeData.size()));

        string volumeString = "{test-class,[MediaPlayer:Amplifier=3]}";
        EXPECT_EQ(E_OK, persistenceDefault.write("lastMainConnectionVolume", volumeString, volumeString.size()));

        string systemProperties = "{(61440:4)(61569:100)}";
        persistenceDefault.write("lastSystemProperty", systemProperties, systemProperties.size());

        string soundProperties = "{test-class,[ET_SOURCE_MediaPlayer=(1:3)(2:4)][ET_SINK_Amplifier=(3:5)]}";
        persistenceDefault.write("lastMainSoundProperty", soundProperties, soundProperties.size());

    }  // persistenceDefault goes out of scope here and writes its data from destructor

    // now check the outcome at the correct path - convert generated XML to canonical form
    xmlDocPtr pDocument = xmlParseFile(mDataFile.c_str());
    ASSERT_NE(nullptr, pDocument);
    xmlChar  *pCanonical = new xmlChar[2048];
    EXPECT_LT(0, xmlC14NDocDumpMemory(pDocument, 0, XML_C14N_1_0, NULL, false, &pCanonical));

    string body((string((char *)pCanonical)));
    EXPECT_EQ(refXml.substr(strlen(XML_HEADER)), body);

    delete[] pCanonical;
}

/**
 * @brief  Test to verify the store capability for Negative scenario
 *
 * @test   Verify the "_writeTOXML()" function which is invoked from destructor.
 *         Storage location is given as invalid filename, so test passes if writing fails
 */
TEST_F(CAmPersistenceDefaultTest, storeNegative)
{
    setenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME, "~/var/", true);
    {
        CAmPersistenceDefault persistenceDefault;
        EXPECT_EQ(E_NON_EXISTENT, persistenceDefault.open(APPLICATION_NAME));

        // populate a new document with valid data
        const std::string writeData = "{test-class,MediaPlayer:Amplifier;}";
        EXPECT_EQ(E_OK, persistenceDefault.write("lastMainConnection", writeData, writeData.size()));

        string volumeString = "{test-class,[MediaPlayer:Amplifier=3]}";
        EXPECT_EQ(E_OK, persistenceDefault.write("lastMainConnectionVolume", volumeString, volumeString.size()));

        string systemProperties = "{(61440:4)(61569:100)}";
        persistenceDefault.write("lastSystemProperty", systemProperties, systemProperties.size());

        string soundProperties = "{test-class,[ET_SOURCE_MediaPlayer=(1:3)(2:4)][ET_SINK_Amplifier=(3:5)]}";
        persistenceDefault.write("lastMainSoundProperty", soundProperties, soundProperties.size());

    } // persistenceDefault goes out of scope and will try to write its data

    // check the outcome at the correct path, which should still be the dummyXml
    ifstream fs(mDataFile);
    string xml = static_cast<std::ostringstream&>(std::ostringstream{} << fs.rdbuf()).str();
    EXPECT_EQ(dummyXml, xml);
}

/**
 * @brief : Test to verify the read function for Positive scenario
 *
 * @test : verify the "read" function when read data and keyName is given as input parameters along with data size and other
 *         configuration details is initialized and set environment variable and initially we have write data into map of CAmPersistenceDefault class
 *         then will check whether read function will get the read data using the key Name details from map or not
 *
 * @result : "Pass" when read function return "E_OK" without any Gmock error message
 */
TEST_F(CAmPersistenceDefaultTest, readPositive)
{
    CAmPersistenceDefault persistenceDefault;

    setenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME, mDataFile.c_str(), true);
    EXPECT_EQ(E_OK, persistenceDefault.open(APPLICATION_NAME));
    const std::string keyName   = "lastMainConnection";
    const std::string writeData = "MainConnectionname";
    EXPECT_EQ(E_OK, persistenceDefault.write(keyName, writeData, writeData.size()));
    std::string readData;
    EXPECT_EQ(E_OK, persistenceDefault.read(keyName, readData, writeData.size()));
}

/**
 * @brief : Test to verify the read function for Negative scenario
 *
 * @test : verify the "read" function when read data and incorrect keyName is given as input parameters along with data size to read function and other
 *         configuration details is initialized and set environment variable and initially we have write data into map of CAmPersistenceDefault class
 *         then will check whether read function will get the read data using the key Name details from map or not
 *
 * @result : "Pass" when read function return "E_DATABASE_ERROR" without any Gmock error message
 */
TEST_F(CAmPersistenceDefaultTest, readNegative)
{
    CAmPersistenceDefault persistenceDefault;

    setenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME, mDataFile.c_str(), true);
    EXPECT_EQ(E_OK, persistenceDefault.open(APPLICATION_NAME));

    const std::string writeData = "MainConnectionname";
    EXPECT_EQ(E_OK, persistenceDefault.write("lastMainConnection", writeData,  writeData.size()));

    std::string       readData;
    EXPECT_EQ(E_DATABASE_ERROR, persistenceDefault.read("lastConnection", readData, writeData.size()));
}

}  // namespace gc_utest


int main(int argc, char * *argv)
{
    // initialize logging environment
    am::CAmLogWrapper::instantiateOnce("UTST", "Unit test for generic controller"
            , LS_ON, LOG_SERVICE_STDOUT);
    LOG_FN_CHANGE_LEVEL(LL_WARN);

    // redirect configuration path
    gc_utest::ConfigDocument config(gc_utest::ConfigDocument::Default);

    gc_utest::CAmPersistenceDefaultTest::OuterPersistence       outer;

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
