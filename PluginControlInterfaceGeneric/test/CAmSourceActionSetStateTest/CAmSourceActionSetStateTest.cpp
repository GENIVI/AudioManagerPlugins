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
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmSourceActionSetState.h"
#include "CAmElement.h"
#include "CAmControllerPlugin.h"
#include "CAmSourceElement.h"
#include "audiomanagertypes.h"
#include "CAmActionContainer.h"
#include "CAmSocketHandler.h"
#include "CAmTimerEvent.h"
#include "CAmHandleStore.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmSourceActionSetStateTest.h"
#include "CAmTestConfigurations.h"

using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmSourceActionSetStateTest
 *@brief : This class is used to test the CAmSourceActionSetState class functionality.
 */
CAmSourceActionSetStateTest::CAmSourceActionSetStateTest()
    : mpMockControlReceiveInterface(NULL)
    , mpControlReceive(NULL)
    , mpSourceElement(NULL)
    , mpNewSourceElement(NULL)
    , mState(SS_UNKNNOWN)
    , mpActionSetSourceState(NULL)
    , mpNewActionSetSourceState(NULL)
{
}

CAmSourceActionSetStateTest::~CAmSourceActionSetStateTest()
{
}

void CAmSourceActionSetStateTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpControlReceive              = mpMockControlReceiveInterface;

    // Initialize Source Element
    ut_gc_Source_s sourceInfo("AnySource1");

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSourceElement = CAmSourceFactory::createElement(sourceInfo, mpControlReceive);
    ASSERT_THAT(mpSourceElement, NotNull()) << " Source Element Is not Created";

    mpActionSetSourceState = new CAmSourceActionSetState(mpSourceElement);
    ASSERT_THAT(mpActionSetSourceState, NotNull()) << " Source Action SetState Is not Created";
}

void CAmSourceActionSetStateTest::TearDown()
{
    CAmSourceFactory::destroyElement();

    if (mpActionSetSourceState != NULL)
    {
        delete (mpActionSetSourceState);
    }

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

void CAmSourceActionSetStateTest::CreateNewSource()
{
    // Initialize Source Element
    ut_gc_Source_s sourceInfo("AnySource2");

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(23), Return(E_OK)));
    mpNewSourceElement = CAmSourceFactory::createElement(sourceInfo, mpControlReceive);

    ASSERT_THAT(mpNewSourceElement, NotNull()) << " Source Element Is not Created";

    mpNewActionSetSourceState = new CAmSourceActionSetState(mpNewSourceElement);
    ASSERT_THAT(mpNewActionSetSourceState, NotNull()) << " Source Action SetState Is not Created";
}

/**
 * @brief : Test to verify the _execute function with Parameter not set.
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *        CAmSourceActionSetState is initialized with source element.
 *
 * @test : verify the "_execute" function when no parameter is passed then will check whether
 *         _execute function will invokes the control receive class function to send request to AM and It
 *         register the observer to receive the acknowledgment of action completion or not,
 *         ,without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceActionSetStateTest, Execute_ParamNotSet)
{
    EXPECT_EQ(mpActionSetSourceState->execute(), E_OK);
}

/**
 * @brief : Test to verify the _execute function with Parameter set.
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *        CAmSourceActionSetState is initialized with source element.
 *
 * @test : verify the "_execute" function when source state action  parameter is set & set the behavior of
 *         setSourceState function to return the "E_OK" then will check whether
 *         _execute function will invokes the control receive class function to send request to AM &
 *          register the observer to receive the acknowledgment of action completion or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceActionSetStateTest, Execute_ParamSet)
{
    mState = SS_PAUSED;
    // Handle for setSourceState
    mHandle = { H_SETSOURCESTATE, 50 };

    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(mpSourceElement->getID(), _)).WillOnce(DoAll(SetArgReferee<1 >(ut_gc_Source_s(SS_ON)), Return(E_OK)));

    mSourceStateParam.setParam(SS_PAUSED);
    mpActionSetSourceState->setParam(ACTION_PARAM_SOURCE_STATE, &mSourceStateParam);

    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), mState)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), Return(E_OK)));
    EXPECT_EQ(mpActionSetSourceState->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
}

/**
 * @brief : Test to verify the _execute function with Parameter & state set.
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *        CAmSourceActionSetState is initialized with source element.
 *
 * @test : verify the "_execute" function when source state action parameter and state as "SS_ON" are set
 *         & set the behavior of getSourceInfoDB function to return the "E_OK" then will check whether
 *         _execute function will invokes the control receive class function to send request to AM &
 *          register the observer to receive the acknowledgment of action completion or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceActionSetStateTest, Execute_ParamSet_StateSame)
{
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(mpSourceElement->getID(), _)).WillOnce(DoAll(SetArgReferee<1 >(ut_gc_Source_s(SS_ON)), Return(E_OK)));

    mSourceStateParam.setParam(SS_ON);
    mpActionSetSourceState->setParam(ACTION_PARAM_SOURCE_STATE, &mSourceStateParam);
    EXPECT_EQ(mpActionSetSourceState->execute(), E_OK);
}

void *WorkerThread(void *data)
{
    CAmSocketHandler *pSocketHandler = (CAmSocketHandler *)data;
    pSocketHandler->start_listenting();
    return NULL;
}

/**
 * @brief : Test to verify the _timeOut function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *        CAmSourceActionSetState is initialized with source element.
 *
 * @test : verify the "_timeOut" function when CAmSocketHandler, CAmControllerPlugin, CAmTimerEvent CAmActionContainer classes
 *         are initialized &  source state action parameter and state as "SS_ON" are set, timeout value and
 *         undo flag is set using setUndoRequried function  & set the behavior of getSourceInfoDB,setSourceState,
 *         execute,notifyAsyncResult function to return the "E_OK" then will check whether
 *         _timeOut function will get the Control receive or not and then call the abortAction function to abort the
 *         action or not ,without any main loop exit error & Gmock error.
 *
 * @result : "Pass" when undo function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceActionSetStateTest, _timeOutTest)
{
    CreateNewSource();
    mState = SS_OFF;

    CAmSocketHandler     pSocketHandler;
    pthread_t            thread_id;
    CAmControllerPlugin *pPlugin;
    CAmTimerEvent       *mpTimerEvent = CAmTimerEvent::getInstance();
    mpTimerEvent->setSocketHandle(&pSocketHandler, pPlugin);

    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(mpSourceElement->getID(), _)).WillRepeatedly(
        DoAll(SetArgReferee<1 >(ut_gc_Source_s(SS_ON)), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(mpNewSourceElement->getID(), _)).WillRepeatedly(
        DoAll(SetArgReferee<1 >(ut_gc_Source_s(SS_ON)), Return(E_OK)));

    mSourceStateParam.setParam(SS_OFF);
    mpActionSetSourceState->setParam(ACTION_PARAM_SOURCE_STATE, &mSourceStateParam);

    mSourceStateParam.setParam(SS_OFF);
    mpNewActionSetSourceState->setParam(ACTION_PARAM_SOURCE_STATE, &mSourceStateParam);

    CAmActionContainer pCAmActionContainer;
    mpNewActionSetSourceState->setTimeout(3000);
    mpActionSetSourceState->setUndoRequried(true);
    mpNewActionSetSourceState->setUndoRequried(true);
    pCAmActionContainer.append(mpActionSetSourceState);
    pCAmActionContainer.append(mpNewActionSetSourceState);
    pCAmActionContainer.setUndoRequried(true);

    // Handle for setSourceState
    mHandle = { H_SETSOURCESTATE, 50 };

    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), mState)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), Return(E_OK)));
    EXPECT_EQ(mpActionSetSourceState->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);

    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceState(_, mpNewSourceElement->getID(), mState)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), Return(E_OK)));
    EXPECT_EQ(mpNewActionSetSourceState->execute(), E_OK);
    pthread_create(&thread_id, NULL, WorkerThread, &pSocketHandler);
    EXPECT_CALL(*mpMockControlReceiveInterface, abortAction(IsHandleOk(mHandle))).WillOnce(
        (Return(E_OK)));
    sleep(5);

    mState = SS_ON;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), mState)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), Return(E_OK)));
    EXPECT_EQ(mpActionSetSourceState->undo(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);

    pSocketHandler.stop_listening();
    pthread_join(thread_id, NULL);
}

/**
 * @brief : Test to verify the undo function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *        CAmSourceActionSetState is initialized with source element.
 *
 * @test : verify the "undo" function when CAmSocketHandler, CAmControlPlugin, CAmTimerEvent CAmActionContainer classes
 *         are initialized &  source state action parameter and state as "SS_ON" are set, timeout value and
 *         undo flag is set using setUndoRequried function  & set the behavior of getSourceInfoDB,setSourceState,
 *         execute,notifyAsyncResult function to return the "E_OK" then will check whether
 *         undo function will performs the undo operation if parent has requested for undo or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when undo function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceActionSetStateTest, _undoTest)
{
    CreateNewSource();
    mState = SS_OFF;

    TCLAP::CmdLine *cmd(CAmCommandLineSingleton::instanciateOnce("AudioManager!", ' ', "7.5", true));

    CAmControllerPlugin *pPlugin        = new CAmControllerPlugin();
    CAmSocketHandler    *pSocketHandler = new CAmSocketHandler();
    CAmTimerEvent       *mpTimerEvent   = CAmTimerEvent::getInstance();
    mpTimerEvent->setSocketHandle(pSocketHandler, pPlugin);

    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(mpSourceElement->getID(), _)).WillRepeatedly(DoAll(SetArgReferee<1>(ut_gc_Source_s(SS_ON)), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(mpNewSourceElement->getID(), _)).WillRepeatedly(DoAll(SetArgReferee<1>(ut_gc_Source_s(SS_ON)), Return(E_OK)));

    mSourceStateParam.setParam(SS_OFF);
    mpActionSetSourceState->setParam(ACTION_PARAM_SOURCE_STATE, &mSourceStateParam);

    mSourceStateParam.setParam(SS_OFF);
    mpNewActionSetSourceState->setParam(ACTION_PARAM_SOURCE_STATE, &mSourceStateParam);

    CAmActionContainer pCAmActionContainer;
    mpActionSetSourceState->setUndoRequried(true);
    mpNewActionSetSourceState->setUndoRequried(true);
    pCAmActionContainer.append(mpActionSetSourceState);
    pCAmActionContainer.append(mpNewActionSetSourceState);
    pCAmActionContainer.setUndoRequried(true);

    // Handle for setSourceState
    mHandle = { H_SETSOURCESTATE, 50 };

    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), mState)).WillOnce(DoAll(SetArgReferee<0>(mHandle), Return(E_OK)));

    EXPECT_EQ(mpActionSetSourceState->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);

    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceState(_, mpNewSourceElement->getID(), mState)).WillOnce(DoAll(SetArgReferee<0>(mHandle), Return(E_NOT_POSSIBLE)));
    EXPECT_EQ(mpNewActionSetSourceState->execute(), E_OK);

    mState = SS_ON;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), mState)).WillOnce(DoAll(SetArgReferee<0>(mHandle), Return(E_OK)));
    EXPECT_EQ(mpActionSetSourceState->undo(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
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
