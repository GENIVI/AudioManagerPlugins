/*******************************************************************************
 *  \copyright (c) 2018 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com> 2018
 *           Kapildev Patel, Satish Dwivedi <kpatel@jp.adit-jv.com> 2018
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
#include <vector>
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmSystemElement.h"
#include "CAmSystemActionSetProperty.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmSystemActionSetPropertyTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmSystemActionSetProperty
 *@brief : This class is used to test the CAmSystemActionSetProperty functionality.
 */
CAmSystemActionSetPropertyTest::CAmSystemActionSetPropertyTest()
    : mpMockControlReceiveInterface(NULL)
    , mpControlReceive(NULL)
    , mpActionSetSystemPropertyTest(NULL)
{
}

CAmSystemActionSetPropertyTest::~CAmSystemActionSetPropertyTest()
{
}

void CAmSystemActionSetPropertyTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpControlReceive              = mpMockControlReceiveInterface;

    // This will check whether controller is initialized or not
    ASSERT_THAT(mpControlReceive, NotNull()) << " Controller Not Initialized";

    // SystemProperty creating
    mSystemproperty.type  = SYP_DEBUG;
    mSystemproperty.value = 5;
    std::vector<gc_SystemProperty_s > listSystemProperties;
    listSystemProperties.push_back(mSystemproperty);

    // Initialize System Element
    gc_ut_System_s                    systemConfiguration("SystemProp", listSystemProperties);
    std::shared_ptr<CAmSystemElement> pSystem;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(mSystemproperty))).WillOnce(Return(E_OK));
    pSystem = CAmSystemFactory::createElement(systemConfiguration, mpControlReceive);
    ASSERT_THAT(pSystem, NotNull()) << " Sytem element Not Initialized";

    // mpActionSetSystemPropertyTest = new CAmSystemActionSetProperty(pSystem, mpControlReceive);
    mpActionSetSystemPropertyTest = new CAmSystemActionSetProperty(pSystem);
    ASSERT_THAT(mpActionSetSystemPropertyTest, NotNull()) << " SystemProperty Action Not Initialized";

}

void CAmSystemActionSetPropertyTest::TearDown()
{
    CAmSystemFactory::destroyElement();

    if (mpActionSetSystemPropertyTest != NULL)
    {
        delete (mpActionSetSystemPropertyTest);
    }

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

/**
 * @brief  :  Test to verify the Execute function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        System element is created and system property is push into list of system property.
 *        CAmSystemActionSetProperty is initialized.
 *
 * @test : verify the "Execute" function when system property type and value are set and we mock the changeSystemPropertyDB
 *         for given system property then check whether Execute function will set the system property or not using the setSystemProperty api call
 *         correctly or not without Gmock error.
 *
 * @result : "Pass" when setSystemProperty function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSystemActionSetPropertyTest, _ExecutePositive)
{
    mTypeParam.setParam(SYP_DEBUG);
    mValueParam.setParam(10);

    // setting paramter
    mpActionSetSystemPropertyTest->setParam(ACTION_PARAM_DEBUG_TYPE, &mTypeParam);
    mpActionSetSystemPropertyTest->setParam(ACTION_PARAM_DEBUG_VALUE, &mValueParam);

    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(mSystemproperty))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpActionSetSystemPropertyTest->execute());
    EXPECT_EQ(E_OK, mpActionSetSystemPropertyTest->getError());
}

/**
 * @brief  :  Test to verify the Execute function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        System element is created and system property is push into list of system property.
 *        CAmSystemActionSetProperty is initialized.
 *
 * @test : verify the "Execute" function when system property type and value are set and mocking for changeSystemPropertyDB is not done
 *         then check whether Execute function will set the system property or not using the setSystemProperty api call
 *         correctly or not without Gmock error.
 *
 * @result : "Pass" when Execute function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmSystemActionSetPropertyTest, _ExecuteNegative)
{
    mTypeParam.setParam(SYP_DEBUG);
    // setting paramter
    mpActionSetSystemPropertyTest->setParam(ACTION_PARAM_DEBUG_TYPE, &mTypeParam);

    EXPECT_EQ(E_OK, mpActionSetSystemPropertyTest->execute());
    EXPECT_EQ(E_NOT_POSSIBLE, mpActionSetSystemPropertyTest->getError());
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
