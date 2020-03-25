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
#include <string>
#include <vector>
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmElement.h"
#include "CAmControllerPlugin.h"
#include "CAmTimerEvent.h"
#include "CAmSocketHandler.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmTimerEventTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmTimerEventTest
 *@brief : This class is used to test the CAmTimerEvent class functionality.
 */
CAmTimerEventTest::CAmTimerEventTest()
    : mpMockControlReceiveInterface(NULL)
    , mpControlReceive(NULL)
    , mpTimerEvent(NULL)
    , mpPlugin(NULL)
    , mpSocketHandler(NULL)
    , mpCallbacktimer(NULL)
    , mpParam(NULL)
{
}

CAmTimerEventTest::~CAmTimerEventTest()
{
}

void CAmTimerEventTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpControlReceive              = mpMockControlReceiveInterface;

    // This will check whether controller is initialized or not
    ASSERT_THAT(mpControlReceive, NotNull()) << " Controller Not Initialized";

    TCLAP::CmdLine *cmd(
        CAmCommandLineSingleton::instanciateOnce("AudioManager!", ' ', "7.5", true));
    mpTimerEvent = CAmTimerEvent::getInstance();
    ASSERT_THAT(mpTimerEvent, NotNull()) << " Timer Event  Not Initialized";
    mpSocketHandler = new CAmSocketHandler();
    ASSERT_THAT(mpSocketHandler, NotNull()) << " Socket Handler  Not Initialized";
    mpPlugin = new CAmControllerPlugin();
    ASSERT_THAT(mpPlugin, NotNull()) << " Controller Plugin  Not Initialized";

}

void CAmTimerEventTest::TearDown()
{
    CAmCommandLineSingleton::deleteInstance();

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

/**
 * @brief : Test to verify the setSocketHandle function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive, CAmSocketHandler,CAmControllerPlugin
 *        CAmCommandLineSingleton::instanciateOnce , CAmTimerEvent classes,
 *
 * @test : verify the "setSocketHandle" function when  socket handler and CAmcontolSend are give as input parameters
 *         then will check whether setSocketHandle function will return any error or not without any Gmock error.
 *
 * @result : "Pass" when setSocketHandle function return "void" without any Gmock error message
 */
TEST_F(CAmTimerEventTest, setSocketHandle)
{
    mpTimerEvent->setSocketHandle(mpSocketHandler, mpPlugin);
}

/**
 * @brief : Test to verify the setTimer function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive, CAmSocketHandler,CAmControllerPlugin
 *        CAmCommandLineSingleton::instanciateOnce , CAmTimerEvent classes,
 *
 * @test : verify the "setTimer" function when IAmShTimerCallBack,sh_timerHandle_t and time in millisecond
 *         parameter are give as input parameters but socket handle details is not set, then will check
 *         whether setTimer function will set the timer details in gc_TimerClient_s structure and then
 *         finally add structure details in addTimer function using the CAmSocketHandler,
 *         insert the details about a handle in list or not,without any Gmock error.
 *
 * @result : "Pass" when setTimer function return "false" without any Gmock error message
 */
TEST_F(CAmTimerEventTest, setTimerNegative)
{
    mSec = 20000;
    EXPECT_FALSE(mpTimerEvent->setTimer(mpCallbacktimer, mpParam, mSec, mHandle));
}

/**
 * @brief : Test to verify the setTimer function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive, CAmSocketHandler,CAmControllerPlugin
 *        CAmCommandLineSingleton::instanciateOnce , CAmTimerEvent classes,
 *
 * @test : verify the "setTimer" function when IAmShTimerCallBack,sh_timerHandle_t and time in millisecond
 *         parameter are give as input parameters and also socket handle & CAmControllerPlugin details are set, then will check
 *         whether setTimer function will set the timer details in gc_TimerClient_s structure and then
 *         finally add structure details in addTimer function using the CAmSocketHandler,
 *         insert the details about a handle in list or not,without any Gmock error.
 *
 * @result : "Pass" when setTimer function return "true" without any Gmock error message
 */
TEST_F(CAmTimerEventTest, setTimerPositive)
{
    mpTimerEvent->setSocketHandle(mpSocketHandler, mpPlugin);
    mSec = 2000;
    EXPECT_TRUE(mpTimerEvent->setTimer(mpCallbacktimer, mpParam, mSec, mHandle));
}

/**
 * @brief : Test to verify the removeTimer function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive, CAmSocketHandler,CAmControllerPlugin
 *        CAmCommandLineSingleton::instanciateOnce , CAmTimerEvent classes,
 *
 * @test : verify the "removeTimer" function when sh_timerHandle_t handle variable is give as input parameters
 *         and also socket handle & CAmControllerPlugin details are set using the setSocketHandle function,
 *         then will check whether removeTimer function will find the details about handle from the list of
 *         gc_TimerClient_s* and then remove the handle from list by call the removeTimer function
 *         & then it  will delete timer object and remove from the list or not,without any Gmock error.
 *
 * @result : "Pass" when removeTimer function return "void", without any Gmock error message
 */
TEST_F(CAmTimerEventTest, removeTimerPositive)
{
    mpTimerEvent->setSocketHandle(mpSocketHandler, mpPlugin);
    mSec = 2000;
    EXPECT_TRUE(mpTimerEvent->setTimer(mpCallbacktimer, mpParam, mSec, mHandle));
    EXPECT_NO_THROW(mpTimerEvent->removeTimer(mHandle));
}

/**
 * @brief : Test to verify the removeTimer function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive, CAmSocketHandler,CAmControllerPlugin
 *        CAmCommandLineSingleton::instanciateOnce , CAmTimerEvent classes,
 *
 * @test : verify the "removeTimer" function when new different sh_timerHandle_t handle variable is give as input parameters
 *         and also socket handle & CAmControllerPlugin details are set using the setSocketHandle function,
 *         then will check whether removeTimer function will find the details about handle from the list of
 *         gc_TimerClient_s* and then remove the handle from list by call the removeTimer function
 *         & then it  will delete timer object and remove from the list or not,without any Gmock error.
 *
 * @result : "Pass" when removeTimer function return "void", without any Gmock error message
 */
TEST_F(CAmTimerEventTest, removeTimerNegative)
{
    mSec = 2000;
    EXPECT_TRUE(mpTimerEvent->setTimer(mpCallbacktimer, mpParam, mSec, mHandle));
    sh_timerHandle_t Newhandle;
    EXPECT_NO_THROW(mpTimerEvent->removeTimer(Newhandle));
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
