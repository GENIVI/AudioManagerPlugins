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
#include "CAmElement.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmTimerEvent.h"
#include "CAmMainConnectionActionConnect.h"
#include "CAmMainConnectionActionSuspend.h"
#include "CAmMainConnectionElement.h"
#include "CAmRouteElement.h"
#include "CAmHandleStore.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmMainConnectionActionSuspendTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmMainConnectionActionSuspendTest
 *@brief : This class is used to test the CAmMainConnectionActionSuspend class functionality.
 */
CAmMainConnectionActionSuspendTest::CAmMainConnectionActionSuspendTest()
    : mSocketHandler()
    , mMockControlReceiveInterface()
    , mControlReceive(mMockControlReceiveInterface)
    , mpSinkElement(nullptr)
    , mpSourceElement(nullptr)
    , mpMainConnection(nullptr)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
{
    // required for undo
    CAmTimerEvent::getInstance()->setSocketHandle(&mSocketHandler, NULL);
}

CAmMainConnectionActionSuspendTest::~CAmMainConnectionActionSuspendTest()
{
}

void CAmMainConnectionActionSuspendTest::SetUp()
{
    // Initialize Sink Element for connected state
    sinkInfo.domainID = 7;
    sinkInfo.available = {A_AVAILABLE, AR_UNKNOWN};
    EXPECT_CALL(mMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _))
        .WillOnce(DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSinkElement = CAmSinkFactory::createElement(sinkInfo, &mControlReceive);
    ASSERT_THAT(mpSinkElement, NotNull()) << " Sink Element Is not Created";

    sinkIDList.push_back(mpSinkElement->getID());
    auto mockSinkInfo = [&](const am_sinkID_t, am_Sink_s& data) { data = sinkInfo; return E_OK; };
    EXPECT_CALL(mMockControlReceiveInterface, getSinkInfoDB(mpSinkElement->getID(), _))
        .WillRepeatedly(Invoke(mockSinkInfo));

    // Initialize Source Element for connected state
    sourceInfo.sourceState = SS_ON;
    sourceInfo.available = {A_AVAILABLE, AR_UNKNOWN};
    EXPECT_CALL(mMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _))
        .WillOnce(DoAll(SetArgReferee<1 >(23), Return(E_OK)));
    mpSourceElement = CAmSourceFactory::createElement(sourceInfo, &mControlReceive);
    ASSERT_THAT(mpSourceElement, NotNull()) << "    am_Handle_s handle_1 = { H_SETSOURCESTATE, 52 }; Source Element Is not Created";

    sourceIDList.push_back(mpSourceElement->getID());
    auto mockSourceInfo = [&](const am_sourceID_t, am_Source_s& data) { data = sourceInfo; return E_OK; };
    EXPECT_CALL(mMockControlReceiveInterface, getSourceInfoDB(mpSourceElement->getID(), _))
        .WillRepeatedly(Invoke(mockSourceInfo));

    // define route
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    routingElement.domainID         = mpSinkElement->getDomainID();
    routingElement.sinkID           = mpSinkElement->getID();
    routingElement.sourceID         = mpSourceElement->getID();
    routingElement.name             = "UnitTestDomain";

    gcRoute.sinkID   = mpSinkElement->getID();
    gcRoute.sourceID = mpSourceElement->getID();
    gcRoute.name     = mpSourceElement->getName() + ":" + mpSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    // prepare main connection in state CS_CONNECTED and its database mock
    mMainConnectionData.mainConnectionID = 0;
    mMainConnectionData.sinkID           = mpSinkElement->getID();
    mMainConnectionData.sourceID         = mpSourceElement->getID();
    mMainConnectionData.delay            = 0;
    mMainConnectionData.connectionState  = CS_UNKNOWN;
    EXPECT_CALL(mMockControlReceiveInterface, changeMainConnectionStateDB(44, _))
        .WillRepeatedly(DoAll(SaveArg<1>(&mMainConnectionData.connectionState), Return(E_OK)));
    auto mockConnectionDB = [&](const am_mainConnectionID_t, am_MainConnection_s &data)
            {data = mMainConnectionData; return E_OK; };
    EXPECT_CALL(mMockControlReceiveInterface, getMainConnectionInfoDB(44, _))
       .WillRepeatedly( Invoke(mockConnectionDB));

    EXPECT_CALL(mMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mMainConnectionData), _))
        .WillOnce(DoAll(SetArgReferee<1 >(44), Assign(&mMainConnectionData.mainConnectionID, 44), Return(E_OK)));
    mpMainConnection = CAmMainConnectionFactory::createElement(gcRoute, &mControlReceive);
    ASSERT_THAT(mpMainConnection, NotNull()) << " MainConnection Element Is not Created";

    // propagate main connection to valid CS_CONECTED state
    vector<shared_ptr<CAmRouteElement> > routesList;
    mpMainConnection->getListRouteElements(routesList);
    ASSERT_EQ(1, routesList.size());
    (*routesList.begin())->setID(mpMainConnection->getID());
    (*routesList.begin())->setState(CS_CONNECTED);
    mpMainConnection->updateState();
    ASSERT_EQ(CS_CONNECTED, mpMainConnection->getState());

    LOG_FN_INFO(">>> ------------------------");
}

void CAmMainConnectionActionSuspendTest::TearDown()
{
    LOG_FN_INFO("------------------------ <<<");

    mpMainConnection->removeRouteElements();
    CAmRouteFactory::destroyElement();
    mpMainConnection = nullptr;
    CAmMainConnectionFactory::destroyElement();

    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();
}

/**
 * @brief Test to verify the _execute() / _update() functions for Positive scenario
 *
 * @test Verify successful execution of CAmMainConnectionActionSuspend
 * 
 *       Precondition:
 *        - source, sink and route elements in place
 *        - established and activated main connection
 *
 *       Expected behavior:
 *        - invocation of IAmControlReceive::setSourceState(SS_PAUSED)
 *
 *       Expected final state:
 *        - source switched to SS_OFF
 *        - connection in CS_SUSPENDED state
 *        - action in error-free completed state
 */
TEST_F(CAmMainConnectionActionSuspendTest, _execute_update_PositiveCase)
{
    // setup test candidate
    CAmMainConnectionActionSuspend action(mpMainConnection);

    // execute action
    am_Handle_s handle_1 = { H_SETSOURCESTATE, 31 };
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_PAUSED))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_PAUSED), SetArgReferee<0 >(handle_1), Return(E_OK)));
    EXPECT_EQ(E_OK, action.execute());

    // positively acknowledge source switching PAUSED
    CAmHandleStore::instance().notifyAsyncResult(handle_1, E_OK);

    // finalize
    EXPECT_EQ(E_OK, action.execute());

    // validate final state
    EXPECT_EQ(AS_COMPLETED, action.getStatus());
    EXPECT_EQ(E_OK, action.getError());
    EXPECT_EQ(CS_SUSPENDED, mpMainConnection->getState());
    EXPECT_EQ(SS_PAUSED, mpSourceElement->getState());
}

/**
 * @brief  Test to verify the _execute() / _update() functions for negative scenario
 *
 * @test Verify failing execution of CAmMainConnectionActionSuspend
 *.
 *       Precondition:
 *        - source, sink and route elements in place
 *        - established and activated main connection
 *.
 *       Expected behavior:
 *        - invocation of IAmControlReceive::setSourceState(SS_PAUSED)
 *.
 *       Expected final state:
 *        - source remains in SS_PAUSED state
 *        - connection remains in CS_CONNECTED state
 *        - action state reflects error condition
 */
TEST_F(CAmMainConnectionActionSuspendTest, _execute_update_NegativeCase)
{
    // setup test candidate
    CAmMainConnectionActionSuspend action(mpMainConnection);

    // execute action
    am_Handle_s handle_1 = { H_SETSOURCESTATE, 31 };
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_PAUSED))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_PAUSED), SetArgReferee<0 >(handle_1), Return(E_OK)));
    EXPECT_EQ(E_OK, action.execute());

    // negatively acknowledge source switching PAUSED
    CAmHandleStore::instance().notifyAsyncResult(handle_1, E_ABORTED);

    // finalize
    EXPECT_EQ(E_OK, action.execute());

    // validate final state
    EXPECT_EQ(AS_ERROR_STOPPED, action.getStatus());
    EXPECT_EQ(E_ABORTED, action.getError());
    EXPECT_EQ(CS_CONNECTED, mpMainConnection->getState());
    EXPECT_EQ(SS_PAUSED, mpSourceElement->getState());
}

/**
 * @brief  Test to verify the _undo() function after positive (successful execution
 *
 * @test Verify roll-back of CAmMainConnectionActionSuspend
 *.
 *       Precondition:
 *        - source, sink and route elements in place
 *        - established and activated main connection
 *.
 *       Expected behavior:
 *        - invocation of IAmControlReceive::setSourceState(SS_PAUSED)
 *        - invocation of IAmControlReceive::setSourceState(SS_ON)
 *.
 *       Expected final state:
 *        - source switched back in SS_ON state
 *        - connection switched back to CS_CONNECTED state
 *        - action state reflects error-free undo condition
 */
TEST_F(CAmMainConnectionActionSuspendTest, _undoCase)
{
    // setup test candidate, configured for hierarchical (deep) undo operation
    CAmMainConnectionActionSuspend action(mpMainConnection);
    action.setUndoRequried(true);

    // execute action
    am_Handle_s handle_1 = { H_SETSOURCESTATE, 31 };
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_PAUSED))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_PAUSED), SetArgReferee<0 >(handle_1), Return(E_OK)));
    EXPECT_EQ(E_OK, action.execute());

    // positively acknowledge source switching PAUSED
    CAmHandleStore::instance().notifyAsyncResult(handle_1, E_OK);

    // finalize
    EXPECT_EQ(E_OK, action.execute());

    LOG_FN_INFO("-------- roll back ---------");

    // start roll-back
    am_Handle_s handle_2 = { H_SETSOURCESTATE, 32 };
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_ON))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_ON), SetArgReferee<0 >(handle_2), Return(E_OK)));
    EXPECT_EQ(E_OK, action.undo());

    // positively acknowledge source switching ON
    CAmHandleStore::instance().notifyAsyncResult(handle_2, E_OK);

    // finalize
    EXPECT_EQ(E_OK, action.undo());

    // validate final state
    EXPECT_EQ(AS_UNDO_COMPLETE, action.getStatus());
    EXPECT_EQ(E_OK, action.getError());
    EXPECT_EQ(CS_CONNECTED, mpMainConnection->getState());
    EXPECT_EQ(SS_ON, mpSourceElement->getState());
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
