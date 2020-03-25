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

#include <map>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"
#include "CAmSystemElement.h"
#include "IAmPolicySend.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmSystemElementTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmSystemElementTest
 *@brief : This class is used to test the CAmSystemElement functionality.
 */
CAmSystemElementTest::CAmSystemElementTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , value(0)
{
}

CAmSystemElementTest::~CAmSystemElementTest()
{
}

void CAmSystemElementTest::InitializeCommonStruct()
{
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    type  = SYP_GLOBAL_LOG_THRESHOLD;
    value = 5;
}

void CAmSystemElementTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;

    // InitializeCommonStrutureOnly
    InitializeCommonStruct();
}

void CAmSystemElementTest::TearDown()
{
    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }

// if (mpCAmSystemElement != NULL)
// {
// delete (mpCAmSystemElement);
// mpCAmSystemElement = NULL;
// }
}

/**
 * @brief  :  Test to verify the setSystemProperty function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        system property is initialized and push into the list of the system property and custom system property type is initialized
 *
 * @test : verify the "setSystemProperty" function when system property type and value are given as input parameters
 *               then it will check whether setSystemProperty function will set the systemProperty or not.
 *
 * @result : "Pass" when setSystemProperty function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSystemElementTest, setSystemPropertyPositive)
{
    systemConfiguration.name = "SystemProp";
    // systemConfiguration.listSystemProperties = listSystemProperties;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSystemElement->setSystemProperty(type, value));
}

/**
 * @brief  :  Test to verify the setSystemProperty function with different Log Level Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        system property is initialized and push into the list of the system property and custom system property type is initialized
 *
 * @test : verify the "setSystemProperty" function with different log level when system property type set to
 *         SYP_REGISTRATION_ALLOW_UNKNOWN_ELEMENT and value are given as input parameters then it will check
 *         whether setSystemProperty function will set the systemProperty or not.
 *
 * @result : "Pass" when setSystemProperty function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSystemElementTest, setSystemPropertyPositivedifferentLogLevel)
{
    systemConfiguration.name = "SystemProp_1";
    // systemConfiguration.listSystemProperties = listSystemProperties;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    type               = SYP_REGISTRATION_ALLOW_UNKNOWN_ELEMENT;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSystemElement->setSystemProperty(type, value));

}

/**
 * @brief  :  Test to verify the setSystemProperty function Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        system property is initialized and push into the list of the system property and custom system property type is initialized
 *
 * @test : verify the "setSystemProperty" function when system property type and value are given as input parameters
 *         and also we mock the system property db to return incorrect data
 *         then will check whether setSystemProperty function will set the systemProperty or not.
 *
 * @result : "Pass" when setSystemProperty function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmSystemElementTest, setSystemPropertyNegative)
{
    systemConfiguration.name = "SystemProp_2";
    // systemConfiguration.listSystemProperties = listSystemProperties;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmSystemElement->setSystemProperty(type, value));
}

/**
 * @brief  :  Test to verify the getSystemProperty function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        system property is initialized and push into the list of the system property and custom system property type is initialized
 *
 * @test : verify the "getSystemProperty" function when system property type and value are given as input parameters
 *         then it will check whether getSystemProperty function will return E_OK
 *         when it find the system property type from the list of system property detail match's then it assign the value from system
 *         property list details to value variable.
 *
 * @result : "Pass" when getSystemProperty function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSystemElementTest, getSystemPropertyPositive)
{
    systemConfiguration.name = "SystemProp_3";
    // systemConfiguration.listSystemProperties = listSystemProperties;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSystemElement->setSystemProperty(type, value));
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSystemElement->getSystemProperty(type, value));
}

/**
 * @brief  :  Test to verify the getSystemProperty function Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        system property is initialized and push into the list of the system property and custom system property type is initialized
 *
 * @test : verify the "getSystemProperty" function when system property type and value are given as input parameters
 *         and also we have set the system property details using the setSystemProperty function correctly but
 *         we mock the getListSystemProperties to return error then
 *         then will check whether getSystemProperty function will get the systemProperty or not.
 *
 * @result : "Pass" when getSystemProperty function return "E_DATABASE_ERROR" without any Gmock error message
 */
TEST_F(CAmSystemElementTest, getSystemPropertyNegative)
{
    systemConfiguration.name = "SystemProp_4";
    // systemConfiguration.listSystemProperties = listSystemProperties;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSystemElement->setSystemProperty(type, value));
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_DATABASE_ERROR)));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSystemElement->getSystemProperty(type, value));

}

/**
 * @brief : Test to verify the getSystemProperty function with not found Debug level for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        system property is initialized and push into the list of the system property and custom system property type is initialized
 *
 * @test : verify the "getSystemProperty" function when system property type and value are given as input parameters
 *         and also we have set the system property details using the setSystemProperty function correctly but with different type details
 *         then will check whether getSystemProperty function will set the systemProperty or not.
 *
 * @result : "Pass" when getSystemProperty function return "E_DATABASE_ERROR" without any Gmock error message
 */
TEST_F(CAmSystemElementTest, getSystemPropertyNegativenotfindDebuglevel)
{
    systemConfiguration.name = "SystemProp_5";
    // systemConfiguration.listSystemProperties = listSystemProperties;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    type               = SYP_DEBUG;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSystemElement->setSystemProperty(type, value));
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmSystemElement->getSystemProperty(type, value));
}

int main(int argc, char * *argv)
{
    // initialize logging environment
    am::CAmLogWrapper::instantiateOnce("UTST", "Unit test for generic controller"
            , LS_ON, LOG_SERVICE_STDOUT);
    LOG_FN_CHANGE_LEVEL(LL_WARN);

    // redirect configuration path
    gc_utest::ConfigDocument config(gc_utest::ConfigDocument::Default);

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
