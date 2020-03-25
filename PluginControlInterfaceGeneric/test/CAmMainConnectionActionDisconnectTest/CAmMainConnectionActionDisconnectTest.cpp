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
#include "CAmMainConnectionActionDisconnect.h"
#include "CAmMainConnectionElement.h"
#include "CAmRouteElement.h"
#include "CAmHandleStore.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmMainConnectionActionDisconnectTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmMainConnectionActionDisconnectTest
 *@brief : This class is used to test the CAmMainConnectionActionDisconnect class functionality.
 */
CAmMainConnectionActionDisconnectTest::CAmMainConnectionActionDisconnectTest()
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

CAmMainConnectionActionDisconnectTest::~CAmMainConnectionActionDisconnectTest()
{
}

void CAmMainConnectionActionDisconnectTest::SetUp()
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

void CAmMainConnectionActionDisconnectTest::TearDown()
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
 * @brief Validate for Positive scenario
 *
 * @test Verify execution of CAmMainConnectionActionDisconnect with positive (success) acknowledgments
 *       to embedded child actions both CAmSourceActionSetState and CAmRouteActionDisconnect
 *
 *       Precondition:
 *        - source, sink and route elements in place
 *        - established and activated main connection
 *       Expected behavior:
 *        - invocation of IAmControlReceive::setSourceState(SS_OFF)
 *        - invocation of IAMCOntrolSend::disconnect
 *       Expected final state:
 *        - source switched to SS_OFF
 *        - route disconnected
 *        - action in error-free completed state
 */
TEST_F(CAmMainConnectionActionDisconnectTest, _execute_update_PositiveCase)
{
    CAmMainConnectionActionDisconnect action(mpMainConnection);

    am_Handle_s handle_1 = { H_SETSOURCESTATE, 52 };
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_OFF))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_OFF), SetArgReferee<0 >(handle_1), Return(E_OK)));
    EXPECT_EQ(E_OK, action.execute());
    EXPECT_EQ(CS_DISCONNECTING, mpMainConnection->getState());

    // positively acknowledge source switching OFF and expect consecutive disconnect() call
    am_Handle_s handle_2 = { H_DISCONNECT, 52 };
    EXPECT_CALL(mMockControlReceiveInterface, disconnect(_, mpMainConnection->getID()))
        .WillOnce(DoAll(SetArgReferee<0 >(handle_2), Return(E_OK)));
    CAmHandleStore::instance().notifyAsyncResult(handle_1, E_OK);

    EXPECT_EQ(E_OK, action.execute());

    // positively acknowledge disconnect() call
    CAmHandleStore::instance().notifyAsyncResult(handle_2, E_OK);

    // check final state
    EXPECT_EQ(E_OK, action.getError());
    EXPECT_EQ(AS_COMPLETED, action.getStatus());
    EXPECT_EQ(SS_OFF, sourceInfo.sourceState);
    EXPECT_EQ(CS_DISCONNECTED, mpMainConnection->getState());
}

/*
 *  validate stable handling if created with invalid main connection element pointer
 */
TEST_F(CAmMainConnectionActionDisconnectTest, _nullPtr)
{
    CAmMainConnectionActionDisconnect action(nullptr);
    action.setUndoRequried(true);

    EXPECT_EQ(action.execute(), E_OK);
    EXPECT_EQ(AS_ERROR_STOPPED, action.getStatus());

    // EXPECT_EQ(action.undo(), E_OK);     // not properly handled - will cause segmentation fault
}

/**
 * @brief Test to verify the execute_update functions for negative scenario
 *
 * @test Verify execution of CAmMainConnectionActionDisconnect with negative (error) acknowledgments
 *       to embedded second child action CAmRouteActionDisconnect
 *
 *       Precondition:
 *        - source, sink and route elements in place
 *        - established and activated main connection
 *       Expected behavior:
 *        - invocation of IAmControlReceive::setSourceState(SS_OFF)
 *        - invocation of IAMCOntrolSend::disconnect
 *       Expected final state:
 *        - main connection remains in transient state CS_DISCONNECTING
 *        - action state indicating error condition
 */
TEST_F(CAmMainConnectionActionDisconnectTest, _execute_update_NegativeCase)
{
    CAmMainConnectionActionDisconnect action(mpMainConnection);
    action.setUndoRequried(true);

    am_Handle_s handle_1 = { H_SETSOURCESTATE, 52 };
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_OFF))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_OFF), SetArgReferee<0 >(handle_1), Return(E_OK)));
    EXPECT_EQ(action.execute(), E_OK);
    EXPECT_EQ(CS_DISCONNECTING, mpMainConnection->getState());

    // positively acknowledge source switching OFF and expect consecutive disconnect() call
    am_Handle_s handle_2 = { H_DISCONNECT, 52 };
    EXPECT_CALL(mMockControlReceiveInterface, disconnect(_, mpMainConnection->getID()))
        .WillOnce(DoAll(SetArgReferee<0 >(handle_2), Return(E_OK)));
    CAmHandleStore::instance().notifyAsyncResult(handle_1, E_OK);

    EXPECT_EQ(E_OK, action.execute());

    // negatively acknowledge disconnect() call
    CAmHandleStore::instance().notifyAsyncResult(handle_2, E_NOT_POSSIBLE);

    // check final state
    EXPECT_EQ(AS_ERROR_STOPPED, action.getStatus());
    EXPECT_EQ(CS_DISCONNECTING, mpMainConnection->getState());
}

/**
 * @brief Test to verify the _undo functions for Positive scenario
 *
 * @test Verify rollback (undo) of CAmMainConnectionActionDisconnect after complete and successful
 *       execution (positive test case)
 *
 *       Precondition:
 *        - source, sink and route elements in place
 *        - established and activated main connection
 *       Expected behavior:
 *        - invocation of IAmControlReceive::setSourceState(SS_OFF)
 *        - invocation of IAMCOntrolSend::disconnect
 *       Expected final state:
 *        - source switched to SS_ON again
 *        - route connected again
 *        - action in undo-completion state
 */
TEST_F(CAmMainConnectionActionDisconnectTest, _undoCase)
{
    CAmMainConnectionActionDisconnect action(mpMainConnection);
    action.setUndoRequried(true);

    // prepare and execute (1)
    am_Handle_s handle_1 = { H_SETSOURCESTATE, 51 };
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_OFF))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_OFF), SetArgReferee<0 >(handle_1), Return(E_OK)));
    EXPECT_EQ(action.execute(), E_OK);
    EXPECT_EQ(CS_DISCONNECTING, mpMainConnection->getState());

    // positively acknowledge source switching OFF and and execute (2)
    am_Handle_s handle_2 = { H_DISCONNECT, 52 };
    EXPECT_CALL(mMockControlReceiveInterface, disconnect(_, mpMainConnection->getID()))
        .WillOnce(DoAll(SetArgReferee<0 >(handle_2), Return(E_OK)));
    CAmHandleStore::instance().notifyAsyncResult(handle_1, E_OK);

    EXPECT_EQ(E_OK, action.execute());

    // positively acknowledge disconnect() call and execute (3)
    CAmHandleStore::instance().notifyAsyncResult(handle_2, E_OK);
    EXPECT_EQ(E_OK, action.execute());

    LOG_FN_INFO("-------- roll back ---------");

    // start roll back
    am_Handle_s handle_3 = { H_CONNECT, 53 };
    EXPECT_CALL(mMockControlReceiveInterface, connect(_, _, CF_GENIVI_STEREO, mpSourceElement->getID(), mpSinkElement->getID()))
        .WillOnce(DoAll(SetArgReferee<0 >(handle_3), SetArgReferee<1 >(44), Return(E_OK)));
    action.undo();

    // positively acknowledge re-connect() call and expect consecutive source switch
    EXPECT_CALL(mMockControlReceiveInterface, setSourceState(_, mpSourceElement->getID(), SS_ON))
        .WillOnce(DoAll(Assign(&sourceInfo.sourceState, SS_ON), SetArgReferee<0 >(handle_1), Return(E_OK)));
    CAmHandleStore::instance().notifyAsyncResult(handle_3, E_OK);
    action.undo();

    // positively acknowledge source switching ON
    CAmHandleStore::instance().notifyAsyncResult(handle_1, E_OK);
    action.undo();

    // check final state
    EXPECT_EQ(AS_UNDO_COMPLETE, action.getStatus());
    EXPECT_EQ(CS_CONNECTED, mpMainConnection->getState());
    EXPECT_EQ(SS_ON, sourceInfo.sourceState);
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
