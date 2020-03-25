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
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"
#include "CAmElement.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmTriggerQueue.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmTriggerQueueTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmTriggerQueueTest
 *@brief : This class is used to test the CAmTriggerQueue class functionality.
 */
CAmTriggerQueueTest::CAmTriggerQueueTest()
    : mpMockControlReceiveInterface(NULL)
    , mpControlReceive(NULL)
    , mpTriggerQueue(NULL)
    , mpSinkElement(NULL)
    , mMuteState(MS_UNKNOWN)
    , mTriggerType(TRIGGER_UNKNOWN)
    , mpTriggerData(NULL)
{
}

CAmTriggerQueueTest::~CAmTriggerQueueTest()
{
}

void CAmTriggerQueueTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpControlReceive              = mpMockControlReceiveInterface;

    // This will check whether controller is initialized or not
    ASSERT_THAT(mpControlReceive, NotNull()) << " Controller Not Initialized";

    mpTriggerQueue = CAmTriggerQueue::getInstance();
    ASSERT_THAT(mpTriggerQueue, NotNull()) << " TriggerQueue Not Initialized";

    // Initialize Sink Element
    ut_gc_Sink_s sinkInfo("AnySink1");

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSinkElement = CAmSinkFactory::createElement(sinkInfo, mpControlReceive);

    ASSERT_THAT(mpSinkElement, NotNull()) << " Sink Element Is not Created";

}

void CAmTriggerQueueTest::TearDown()
{
    CAmSinkFactory::destroyElement();

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

/**
 * @brief : Test to verify the dequeue function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive, CAmTriggerQueue classes,
 *        sink element is created with required input structure parameters like name,domainID,bus name,state,
 *        node name,registration type etc. sink element details is enter in db for behavior set.
 *
 * @test : verify the "dequeue" function when trigger Type is give as input parameter which set to as USER_SET_SINK_MUTE_STATE
 *         but trigger detaila is not insert into  a queue then will check whether dequeue function will not return the struct gc_TriggerElement_s
 *         and it just return NULL,without any gmock error.
 *
 * @result : "Pass" when execute function return "NULL" without any Gmock error message
 */
TEST_F(CAmTriggerQueueTest, dequeueNegative)
{
    mMuteState    = MS_MUTED;
    mTriggerType  = USER_SET_SINK_MUTE_STATE;
    mpTriggerData = new gc_SinkMuteTrigger_s;
    ASSERT_THAT(mpTriggerData, NotNull()) << " TriggerData Is not Created";

    mpTriggerData->sinkName  = mpSinkElement->getName();
    mpTriggerData->muteState = mMuteState;

    EXPECT_EQ(NULL, mpTriggerQueue->dequeue(mTriggerType));
}

/**
 * @brief : Test to verify the dequeue function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive, CAmTriggerQueue classes,
 *        sink element is created with required input structure parameters like name,domainID,bus name,state,
 *        node name,registration type etc. sink element details is enter in db for behavior set.
 *
 * @test : verify the "dequeue" function when trigger Type is give as input parameter which set to as USER_SET_SINK_MUTE_STATE
 *         and put the trigger details like sink name , mute state etc in the trigger queue then will check whether dequeue function
 *         will return the gc_TriggerElement_s structure details or not, without any gmock error.
 *
 * @result : "Pass" when execute function return "mpTriggerData" without any Gmock error message
 */
TEST_F(CAmTriggerQueueTest, dequeuePositive)
{
    mMuteState    = MS_MUTED;
    mTriggerType  = USER_SET_SINK_MUTE_STATE;
    mpTriggerData = new gc_SinkMuteTrigger_s;
    ASSERT_THAT(mpTriggerData, NotNull()) << " TriggerData Is not Created";

    mpTriggerData->sinkName  = mpSinkElement->getName();
    mpTriggerData->muteState = mMuteState;

    EXPECT_EQ(E_OK, mpTriggerQueue->queue(mTriggerType, mpTriggerData));
    EXPECT_EQ(mpTriggerData, mpTriggerQueue->dequeue(mTriggerType));
}

/**
 * @brief : Test to verify the queue function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive, CAmTriggerQueue classes,
 *        sink element is created with required input structure parameters like name,domainID,bus name,state,
 *        node name,registration type etc. sink element details is enter in db for behavior set.
 *
 * @test : verify the "queue" function when trigger Type is give as input parameter which set to as USER_SET_VOLUME
 *         and put the trigger details like sink name , volume etc in the trigger queue then will check whether queue function
 *         will return the E_OK or not, without any gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmTriggerQueueTest, queuePositive)
{
    am_mainVolume_t mainVolume = 10;

    mTriggerType = USER_SET_VOLUME;
    gc_SinkVolumeChangeTrigger_s *triggerData = new gc_SinkVolumeChangeTrigger_s;
    ASSERT_THAT(triggerData, NotNull()) << " TriggerData Is not Created";

    triggerData->sinkName = mpSinkElement->getName();
    triggerData->volume   = mainVolume;

    EXPECT_EQ(E_OK, mpTriggerQueue->queue(mTriggerType, triggerData));
}

/**
 * @brief : Test to verify the queue function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive, CAmTriggerQueue classes,
 *        sink element is created with required input structure parameters like name,domainID,bus name,state,
 *        node name,registration type etc. sink element details is enter in db for behavior set.
 *
 * @test : verify the "pushTop" function when trigger Type is give as input parameter which set to as SYSTEM_INTERRUPT_STATE_CHANGED
 *         and put the trigger details like source name , interrupt state etc in the pushTop function then will check whether pushTop function
 *         will insert the data the list at top place and return "E_OK" or not, without any gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmTriggerQueueTest, pushTopPositive)
{
    am_InterruptState_e interrptstate = IS_INTERRUPTED;

    mTriggerType = SYSTEM_INTERRUPT_STATE_CHANGED;
    gc_SourceInterruptChangeTrigger_s *triggerData = new gc_SourceInterruptChangeTrigger_s;
    ASSERT_THAT(triggerData, NotNull()) << " TriggerData Is not Created";

    triggerData->sourceName    = mpSinkElement->getName();
    triggerData->interrptstate = interrptstate;

    EXPECT_EQ(E_OK, mpTriggerQueue->queueWithPriority(mTriggerType, triggerData));
}

int main(int argc, char * *argv)
{
    // initialize logging environment
    am::CAmLogWrapper::instantiateOnce("UTST", "Unit test for generic controller"
            , LS_ON, LOG_SERVICE_STDOUT);
    LOG_FN_CHANGE_LEVEL(LL_WARN);

    // redirect configuration path
    gc_utest::ConfigDocument config(gc_utest::ConfigDocument::Default);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
