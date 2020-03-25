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
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmMainConnectionActionConnect.h"
#include "CAmMainConnectionElement.h"
#include "CAmRouteElement.h"
#include "CAmTimerEvent.h"
#include "CAmHandleStore.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmMainConnectionActionConnectTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmMainConnectionActionConnectTest
 *@brief : This class is used to test the CAmMainConnectionActionConnect class functionality.
 */
CAmMainConnectionActionConnectTest::CAmMainConnectionActionConnectTest()
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

CAmMainConnectionActionConnectTest::~CAmMainConnectionActionConnectTest()
{
}

void CAmMainConnectionActionConnectTest::SetUp()
{
    // setup sink
    sinkInfo.available = {A_AVAILABLE, 0};
    EXPECT_CALL(mMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _))
        .WillOnce(DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSinkElement = CAmSinkFactory::createElement(sinkInfo, &mControlReceive);

    ASSERT_THAT(mpSinkElement, NotNull()) << " Sink Element Is not Created";

    sinkIDList.push_back(mpSinkElement->getID());
    auto mockSinkInfo = [&](const am_sinkID_t, am_Sink_s& data) { data = sinkInfo; return E_OK; };
    EXPECT_CALL(mMockControlReceiveInterface, getSinkInfoDB(mpSinkElement->getID(), _))
        .WillRepeatedly(Invoke(mockSinkInfo));

    // setup source
    sourceInfo.available = {A_AVAILABLE, 0};
    sourceInfo.sourceState = SS_OFF;
    EXPECT_CALL(mMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _))
        .WillOnce(DoAll(SetArgReferee<1 >(23), Return(E_OK)));
    mpSourceElement = CAmSourceFactory::createElement(sourceInfo, &mControlReceive);

    ASSERT_THAT(mpSourceElement, NotNull()) << " Source Element Is not Created";

    sourceIDList.push_back(mpSourceElement->getID());
    auto mockSourceInfo = [&](const am_sourceID_t, am_Source_s& data) { data = sourceInfo; return E_OK; };
    EXPECT_CALL(mMockControlReceiveInterface, getSourceInfoDB(mpSourceElement->getID(), _))
        .WillRepeatedly(Invoke(mockSourceInfo));

    routingElement.connectionFormat = CF_GENIVI_STEREO;
    routingElement.domainID         = mpSinkElement->getDomainID();
    routingElement.sinkID           = mpSinkElement->getID();
    routingElement.sourceID         = mpSourceElement->getID();

    gcRoute.sinkID   = mpSinkElement->getID();
    gcRoute.sourceID = mpSourceElement->getID();
    gcRoute.name     = mpSourceElement->getName() + ":" + mpSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    mMainConnectionData.mainConnectionID = 0;
    mMainConnectionData.sinkID           = mpSinkElement->getID();
    mMainConnectionData.sourceID         = mpSourceElement->getID();
    mMainConnectionData.delay            = 0;
    mMainConnectionData.connectionState  = CS_CONNECTING;

    LOG_FN_INFO(">>> ------------------------");
}

void CAmMainConnectionActionConnectTest::TearDown()
{
    LOG_FN_INFO("------------------------ <<<");

    CAmMainConnectionFactory::destroyElement();
    CAmRouteFactory::destroyElement();   // this workaround should not be necessary

    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sourceInfo.available = {A_UNKNOWN, 0};
    EXPECT_CALL(mMockControlReceiveInterface, getSourceInfoDB(_, _))
        .WillRepeatedly(Return(E_NON_EXISTENT));

    sinkIDList.clear();
    CAmSinkFactory::destroyElement();
    sinkInfo.available = {A_UNKNOWN, 0};
    EXPECT_CALL(mMockControlReceiveInterface, getSinkInfoDB(_, _))
        .WillRepeatedly(Return(E_NON_EXISTENT));
}

/**
 * @brief Test to verify the execute_update functions for Positive scenario
 *
 * @test Verify execution of CAmMainConnectionActionConnect with positive (success) acknowledgments
 *       to embedded child actions both CAmRouteActionDisconnect and CAmSourceActionSetState
 *
 *       Precondition:
 *        - source, sink and route elements in place
 *       Expected behavior:
 *        - invocation of IAmControlReceive::connect
 *        - invocation of IAmControlReceive::setSourceState(SS_ON)
 *       Expected final state:
 *        - route connected
 *        - source switched to SS_ON
 *        - action in error-free completed state
 */
TEST_F(CAmMainConnectionActionConnectTest, _execute_update_PositiveCase)
{
    // setup monitoring data for connection under test
    am_MainConnection_s state = gc_ut_MainConnection_s(CS_UNKNOWN);
    auto mockConnectionDB = [&](const am_mainConnectionID_t, am_MainConnection_s &data) {data = state; return E_OK; };
    EXPECT_CALL(mMockControlReceiveInterface, getMainConnectionInfoDB(44, _))
       .WillRepeatedly( Invoke(mockConnectionDB));

    EXPECT_CALL(mMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mMainConnectionData), _))
        .WillOnce(DoAll(SaveArg<0>(static_cast<am_MainConnection_s *>(&state)), Assign(&state.mainConnectionID, 44), SetArgReferee<1 >(44), Return(E_OK)));

    mpMainConnection = CAmMainConnectionFactory::createElement(gcRoute, &mControlReceive);
    ASSERT_THAT(mpMainConnection, NotNull()) << " MainConnection Element Is not Created";

    // setup the action
    EXPECT_CALL(mMockControlReceiveInterface, changeMainConnectionStateDB(44, CS_CONNECTING))
        .WillOnce(DoAll(Assign(&state.connectionState, CS_CONNECTING), Return(E_OK)));
    CAmMainConnectionActionConnect action(mpMainConnection);

    // let the execute() function invoke a connect() request to the routi  EXPECT_EQ(CS_DISCONNECTED, mpMainConnection->getState());ng side
    am_Handle_s handle_1 = { H_CONNECT, 50 };
    EXPECT_CALL(mMockControlReceiveInterface, connect(_, _, CF_GENIVI_STEREO, mpSourceElement->getID(), mpSinkElement->getID()))
        .WillOnce(DoAll(SetArgReferee<0 >(handle_1), SetArgReferee<1 >(44), Return(E_OK)));

    EXPECT_CALL(mMockControlReceiveInterface, changeMainConnectionStateDB(44, _))
        .WillRepeatedly(DoAll(SaveArg<1>(&state.connectionState), Return(E_OK)));
    EXPECT_EQ(action.execute(), E_OK);
    EXPECT_EQ(action.getStatus(), AS_EXECUTING);

    // positively acknowledge route segment connection
    CAmHandleStore::instance().notifyAsyncResult(handle_1, E_OK);

    // let the execute() function invoke a setSourceState() request to the routing side
    am_Handle_s handle_2 = { H_SETSOURCESTATE, 52 };
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_ON))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_ON), SetArgReferee<0 >(handle_2), Return(E_OK)));
    EXPECT_EQ(action.execute(), E_OK);

    // positively acknowledge source switching ON
    CAmHandleStore::instance().notifyAsyncResult(handle_2, E_OK);

    EXPECT_EQ(state.connectionState, CS_CONNECTED);

    mpMainConnection->removeRouteElements();
    CAmMainConnectionFactory::destroyElement();
    mpMainConnection = nullptr;
}

/**
 * @brief Test to verify the execute_update functions for Negative scenario
 *
 * @test Verify execution of CAmMainConnectionActionConnect with negative (failure) acknowledgments
 *       to embedded second child action CAmSourceActionSetState
 *
 *       Precondition:
 *        - source, sink and route elements in place
 *       Expected behavior:
 *        - invocation of IAmControlReceive::connect
 *        - invocation of IAmControlReceive::setSourceState(SS_ON)
 *       Expected final state:
 *        - main connection remains in state CS_CONNECTING
 *        - action state indicates error 
 */
TEST_F(CAmMainConnectionActionConnectTest, _execute_update_NegativeCase)
{
    // setup monitoring data for connection under test
    am_MainConnection_s state = gc_ut_MainConnection_s(CS_UNKNOWN);
    auto mockConnectionDB = [&](const am_mainConnectionID_t, am_MainConnection_s &data) {data = state; return E_OK; };
    EXPECT_CALL(mMockControlReceiveInterface, getMainConnectionInfoDB(44, _))
       .WillRepeatedly( Invoke(mockConnectionDB));

    EXPECT_CALL(mMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mMainConnectionData), _))
        .WillOnce(DoAll(SaveArg<0>(static_cast<am_MainConnection_s *>(&state)), Assign(&state.mainConnectionID, 44), SetArgReferee<1 >(44), Return(E_OK)));

    mpMainConnection = CAmMainConnectionFactory::createElement(gcRoute, &mControlReceive);
    ASSERT_THAT(mpMainConnection, NotNull()) << " MainConnection Element Is not Created";

    // setup the action
    EXPECT_CALL(mMockControlReceiveInterface, changeMainConnectionStateDB(44, CS_CONNECTING))
        .WillOnce(DoAll(Assign(&state.connectionState, CS_CONNECTING), Return(E_OK)));
    CAmMainConnectionActionConnect action(mpMainConnection);

    // let the execute() function invoke a connect() request to the routing side
    am_Handle_s handle_1 = { H_CONNECT, 50 };
    EXPECT_CALL(mMockControlReceiveInterface, connect(_, _, CF_GENIVI_STEREO, mpSourceElement->getID(), mpSinkElement->getID()))
        .WillOnce(DoAll(SetArgReferee<0 >(handle_1), SetArgReferee<1 >(44), Return(E_OK)));

    EXPECT_CALL(mMockControlReceiveInterface, changeMainConnectionStateDB(44, _))
        .WillRepeatedly(DoAll(SaveArg<1>(&state.connectionState), Return(E_OK)));
    EXPECT_EQ(action.execute(), E_OK);
    EXPECT_EQ(action.getStatus(), AS_EXECUTING);

    // positively acknowledge route segment connection
    CAmHandleStore::instance().notifyAsyncResult(handle_1, E_OK);

    // let the execute() function invoke a setSourceState() request to the routing side
    am_Handle_s handle_2 = { H_SETSOURCESTATE, 52 };
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_ON))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_ON), SetArgReferee<0 >(handle_2), Return(E_OK)));
    EXPECT_EQ(action.execute(), E_OK);

    // ------------------------------------------
    // negatively acknowledge source switching ON
    CAmHandleStore::instance().notifyAsyncResult(handle_2, E_COMMUNICATION);

    EXPECT_EQ(action.execute(), E_OK);

    EXPECT_EQ(AS_ERROR_STOPPED, action.getStatus());

    // connection never proceeds from transient state
    EXPECT_EQ(CS_CONNECTING, mpMainConnection->getState());

    mpMainConnection->removeRouteElements();
    CAmMainConnectionFactory::destroyElement();
    mpMainConnection = nullptr;
}

/**
 * @brief : Test to verify the _undo functions for Positive scenario
 *
 * @test Verify roll-back (undo) operation of CAmMainConnectionActionConnect after successful
 *       execution
 *
 *       Precondition:
 *        - source, sink and route elements in place
 *       Expected behavior:
 *        - invocation of IAmControlReceive::connect
 *        - invocation of IAmControlReceive::setSourceState(SS_ON)
 *        - invocation of IAmControlReceive::setSourceState(SS_OFF)
 *        - invocation of IAmControlReceive::disconnect
 *       Expected final state:
 *        - main connection remains in state CS_DISCONNECTED
 *        - action in state AS_UNDO_COMPLETE
 */
TEST_F(CAmMainConnectionActionConnectTest, _undoCase)
{
    // setup a connection in state CS_CONNECTED as in positive case

    // setup monitoring data for connection under test
    am_MainConnection_s state = gc_ut_MainConnection_s(CS_UNKNOWN);
    auto mockConnectionDB = [&](const am_mainConnectionID_t, am_MainConnection_s &data) {data = state; return E_OK; };
    EXPECT_CALL(mMockControlReceiveInterface, getMainConnectionInfoDB(44, _))
       .WillRepeatedly( Invoke(mockConnectionDB));

    EXPECT_CALL(mMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mMainConnectionData), _))
        .WillOnce(DoAll(SaveArg<0>(static_cast<am_MainConnection_s *>(&state)), Assign(&state.mainConnectionID, 44), SetArgReferee<1 >(44), Return(E_OK)));

    mpMainConnection = CAmMainConnectionFactory::createElement(gcRoute, &mControlReceive);
    ASSERT_THAT(mpMainConnection, NotNull()) << " MainConnection Element Is not Created";

    // setup the action and enable child rollback
    EXPECT_CALL(mMockControlReceiveInterface, changeMainConnectionStateDB(44, CS_CONNECTING))
        .WillOnce(DoAll(Assign(&state.connectionState, CS_CONNECTING), Return(E_OK)));
    CAmMainConnectionActionConnect action(mpMainConnection);
    action.setUndoRequried(true);

    // let the execute() function invoke a connect() request to the routing side
    am_Handle_s handle_1 = { H_CONNECT, 50 };
    EXPECT_CALL(mMockControlReceiveInterface, connect(_, _, CF_GENIVI_STEREO, mpSourceElement->getID(), mpSinkElement->getID()))
        .WillOnce(DoAll(SetArgReferee<0 >(handle_1), SetArgReferee<1 >(44), Return(E_OK)));

    EXPECT_CALL(mMockControlReceiveInterface, changeMainConnectionStateDB(44, _))
        .WillRepeatedly(DoAll(SaveArg<1>(&state.connectionState), Return(E_OK)));
    EXPECT_EQ(action.execute(), E_OK);
    EXPECT_EQ(action.getStatus(), AS_EXECUTING);

    // positively acknowledge route segment connection
    CAmHandleStore::instance().notifyAsyncResult(handle_1, E_OK);

    // let the execute() function invoke a setSourceState() request to the routing side
    am_Handle_s handle_2 = { H_SETSOURCESTATE, 52 };
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_ON))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_ON), SetArgReferee<0 >(handle_2), Return(E_OK)));
    EXPECT_EQ(action.execute(), E_OK);                    

    // positively acknowledge source switching ON
    CAmHandleStore::instance().notifyAsyncResult(handle_2, E_OK);

    EXPECT_EQ(state.connectionState, CS_CONNECTED);

    LOG_FN_INFO("-------- roll back ---------");

    // request acknowledge source switching SS_OFF
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_OFF))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_OFF), SetArgReferee<0 >(handle_2), Return(E_OK)));
    CAmHandleStore::instance().notifyAsyncResult(handle_2, E_OK);
    EXPECT_EQ(E_OK, action.undo());

    // positively acknowledge source switching OFF
    CAmHandleStore::instance().notifyAsyncResult(handle_2, E_OK);

    am_Handle_s handle_3 = { H_DISCONNECT, 53 };
    EXPECT_CALL(mMockControlReceiveInterface, disconnect(_, mpMainConnection->getID()))
        .WillOnce(DoAll(SetArgReferee<0 >(handle_3), Return(E_OK)));
    EXPECT_EQ(E_OK, action.undo());

    // positively acknowledge disconnection
    CAmHandleStore::instance().notifyAsyncResult(handle_3, E_OK);
    EXPECT_EQ(E_OK, action.undo());

    // check final status
    EXPECT_EQ(AS_UNDO_COMPLETE, action.getStatus());
    EXPECT_EQ(CS_DISCONNECTED, mpMainConnection->getState());

    CAmMainConnectionFactory::destroyElement();
    mpMainConnection = nullptr;
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
