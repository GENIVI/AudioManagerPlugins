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
#include "CAmControllerPlugin.h"
#include "CAmRouteActionConnect.h"
#include "CAmRouteActionDisconnect.h"
#include "CAmRouteElement.h"
#include "CAmTimerEvent.h"
#include "CAmActionContainer.h"
#include "CAmHandleStore.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmRouteActionDisconnectTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmRouteActionDisconnectTest
 *@brief : This class is used to test the CAmRouteActionDisconnect class functionality.
 */
CAmRouteActionDisconnectTest::CAmRouteActionDisconnectTest()
    : mpMockControlReceiveInterface(NULL)
    , mpControlReceive(NULL)
    , mpSinkElement(NULL)
    , mpNewSinkElement(NULL)
    , mpSourceElement(NULL)
    , mpNewSourceElement(NULL)
    , mpRoutingElement(NULL)
    , mpNewRoutingElement(NULL)
    , mpActionRouteConnect(NULL)
    , mpNewActionRouteConnect(NULL)
    , mpActionRouteDisConnect(NULL)
    , mpNewActionRouteDisConnect(NULL)
{
}

CAmRouteActionDisconnectTest::~CAmRouteActionDisconnectTest()
{
}

void CAmRouteActionDisconnectTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpControlReceive              = mpMockControlReceiveInterface;

    // This will check whether controller is initialized or not
    ASSERT_THAT(mpControlReceive, NotNull()) << " Controller Not Initialized";

    // Initialize Sink Element
    ut_gc_Sink_s sinkInfo("AnySink1");
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(22, _))
    .WillRepeatedly(DoAll(SetArgReferee<1>(sinkInfo), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSinkElement = CAmSinkFactory::createElement(sinkInfo, mpControlReceive);

    ASSERT_THAT(mpSinkElement, NotNull()) << " Sink Element Is not Created";

    sinkIDList.push_back(mpSinkElement->getID());

    // Initialize Source Element
    ut_gc_Source_s sourceInfo("AnySource1");
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(23, _))
    .WillRepeatedly(DoAll(SetArgReferee<1>(sourceInfo), Return(E_OK)));

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(23), Return(E_OK)));
    mpSourceElement = CAmSourceFactory::createElement(sourceInfo, mpControlReceive);

    ASSERT_THAT(mpSourceElement, NotNull()) << " Source Element Is not Created";

    sourceIDList.push_back(mpSourceElement->getID());

    // creating the routing element
    mGcRoutingElement.name             = mpSourceElement->getName() + ":" + mpSinkElement->getName();
    mGcRoutingElement.sourceID         = mpSourceElement->getID();
    mGcRoutingElement.sinkID           = mpSinkElement->getID();
    mGcRoutingElement.domainID         = 2;
    mGcRoutingElement.connectionFormat = CF_GENIVI_STEREO;

    mpRoutingElement = CAmRouteFactory::createElement(mGcRoutingElement, mpControlReceive);
    ASSERT_THAT(mpRoutingElement, NotNull()) << " Route Element Is not Created";

    mpActionRouteConnect = new CAmRouteActionConnect(mpRoutingElement);
    ASSERT_THAT(mpActionRouteConnect, NotNull()) << " RouteAction Is not Created";

    // Handle for connect
    mHandle = { H_CONNECT, 50 };
    // calling the execute method
    EXPECT_CALL(*mpMockControlReceiveInterface, connect(_, _, mGcRoutingElement.connectionFormat, mGcRoutingElement.sourceID, mGcRoutingElement.sinkID)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), SetArgReferee<1 >(44), Return(E_OK)));
    EXPECT_EQ(mpActionRouteConnect->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);

    mpActionRouteDisConnect = new CAmRouteActionDisconnect(mpRoutingElement);
    ASSERT_THAT(mpActionRouteDisConnect, NotNull()) << " RouteAction Is not Created";
}

void CAmRouteActionDisconnectTest::TearDown()
{
    CAmRouteFactory::destroyElement();
    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();

    if (mpActionRouteDisConnect != NULL)
    {
        delete (mpActionRouteDisConnect);
    }

    if (mpActionRouteConnect != NULL)
    {
        delete (mpActionRouteConnect);
    }

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

void CAmRouteActionDisconnectTest::CreateNewRouteConnectDisconnect()
{
    // Initialize Sink Element
    ut_gc_Sink_s sinkInfo("AnySink2");
    ut_gc_Source_s sourceInfo("AnySource2");
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(23, _))
    .WillRepeatedly(DoAll(SetArgReferee<1>(sourceInfo), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(22, _))
    .WillRepeatedly(DoAll(SetArgReferee<1>(sinkInfo), Return(E_OK)));
/*    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(23), Return(E_OK)));*/
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpNewSinkElement = CAmSinkFactory::createElement(sinkInfo, mpControlReceive);

    ASSERT_THAT(mpNewSinkElement, NotNull()) << " Sink Element Is not Created";

    sinkIDList.push_back(mpSinkElement->getID());

    // Initialize Source Element


    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(23), Return(E_OK)));
    mpNewSourceElement = CAmSourceFactory::createElement(sourceInfo, mpControlReceive);

    ASSERT_THAT(mpNewSourceElement, NotNull()) << " Source Element Is not Created";

    sourceIDList.push_back(mpSourceElement->getID());

    // creating the routing element
    mNewGcRoutingElement.name             = mpNewSourceElement->getName() + ":" + mpNewSinkElement->getName();
    mNewGcRoutingElement.sourceID         = mpNewSourceElement->getID();
    mNewGcRoutingElement.sinkID           = mpNewSinkElement->getID();
    mNewGcRoutingElement.domainID         = 3;
    mNewGcRoutingElement.connectionFormat = CF_GENIVI_ANALOG;

    mpNewRoutingElement = CAmRouteFactory::createElement(mNewGcRoutingElement, mpControlReceive);
    ASSERT_THAT(mpNewRoutingElement, NotNull()) << " Route Element Is not Created";

    mpNewActionRouteConnect = new CAmRouteActionConnect(mpNewRoutingElement);
    ASSERT_THAT(mpNewActionRouteConnect, NotNull()) << " RouteAction Is not Created";

    // Handle for connect
    mHandle = { H_CONNECT, 50 };
    // calling the execute method
   EXPECT_CALL(*mpMockControlReceiveInterface, connect(_, _, mNewGcRoutingElement.connectionFormat, mNewGcRoutingElement.sourceID, mNewGcRoutingElement.sinkID)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), SetArgReferee<1 >(45), Return(E_OK)));

    EXPECT_EQ(mpNewActionRouteConnect->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);

    mpNewActionRouteDisConnect = new CAmRouteActionDisconnect(mpNewRoutingElement);
    ASSERT_THAT(mpNewActionRouteDisConnect, NotNull()) << " RouteAction Is not Created";

}

/**
 * @brief : Test to verify the _execute_Update function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmRouteActionConnect CAmRouteActionDisconnect classes,
 *        Sink, Source,routing Elements are created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *        route element is used to initialize the CAmRouteActionConnect,CAmRouteActionDisconnect classes
 *
 * @test : verify the "_execute_Update" function when set the behavior of disconnect function to return "E_OK" with input
 *         parameter as mHandle of Disconnect, then will check whether _execute_Update function will invokes the control
 *         receive class function to send request to AM. and It register the observer to receive the acknowledgment of
 *         action completion or not and then check updates is done or not for the connection state of router level connection
 *         using notifyAsyncResult function with state as E_OK or not ,without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteActionDisconnectTest, _execute_update_PositiveCase)
{
    // Handle for Disconnect
    mHandle = { H_DISCONNECT, 50 };

    EXPECT_CALL(*mpMockControlReceiveInterface, disconnect(_, 44)).WillOnce(DoAll(SetArgReferee<0>(mHandle), Return(E_OK)));
    EXPECT_EQ(mpActionRouteDisConnect->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
}

/**
 * @brief : Test to verify the _execute_Update function for Positive scenario with connection unknown already set.
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmRouteActionConnect CAmRouteActionDisconnect classes,
 *        Sink, Source,routing Elements are created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *        route element is used to initialize the CAmRouteActionConnect,CAmRouteActionDisconnect classes
 *
 * @test : verify the "_execute_Update" function when set route element state as CS_UNKNOWN , then will check whether _execute_Update
 *         function will invokes the control receive class function to send request to AM. and It register the observer
 *         to receive the acknowledgment of action completion or not and then check updates is done or not for the
 *         connection state of router level connection using notifyAsyncResult function with state as E_OK or not,
 *         without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteActionDisconnectTest, _execute_PositiveCase_ConnetionUnknownAlready)
{
    mpRoutingElement->setState(CS_UNKNOWN);
    EXPECT_EQ(mpActionRouteDisConnect->execute(), E_OK);
}

/**
 * @brief : Test to verify the _execute_Update function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmRouteActionConnect CAmRouteActionDisconnect classes,
 *        Sink, Source,routing Elements are created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *        route element is used to initialize the CAmRouteActionConnect,CAmRouteActionDisconnect classes
 *
 * @test : verify the "_execute_Update" function when set the behavior of disconnect function to return "E_OK" with input
 *         parameter as mHandle of Disconnect, then will check whether _execute_Update function will invokes the control
 *         receive class function to send request to AM. and It register the observer to receive the acknowledgment of
 *         action completion or not and then check updates is done or not for the connection state of router level connection
 *         using notifyAsyncResult function with state as E_NOT_POSSIBLE or not ,without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteActionDisconnectTest, update_NegativeCase_)
{
    // Handle for Disconnect
    mHandle = { H_DISCONNECT, 50 };

    EXPECT_CALL(*mpMockControlReceiveInterface, disconnect(_, 44)).WillOnce(DoAll(SetArgReferee<0>(mHandle), Return(E_OK)));
    EXPECT_EQ(mpActionRouteDisConnect->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_NOT_POSSIBLE);
}

/**
 * @brief : Test to verify the _undo function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmRouteActionConnect CAmRouteActionDisconnect classes,
 *        Sink, Source,routing Elements are created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *        route element is used to initialize the CAmRouteActionConnect,CAmRouteActionDisconnect classes
 *
 * @test : verify the "_undo" function when new route action connect, disconnect is created, CAmSocketHandler,
 *         CAmTimerEvent, CAmActionContainer classes are initialized, set undo required flag o true, In Action container
 *         append the action route connect and disconnect. set the behavior of disconnect function to return "E_OK" &  "E_NOT_POSSIBLE" with input
 *         parameter as mHandle of Disconnect, then call the execute method after each disconnect call.
 *         finally set the connect function behavior to return "E_OK"
 *         then will check whether _undo function will performs the undo operation or not
 *         if parent has requested for undo ,without any Gmock error.
 *
 * @result : "Pass" when _undo function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteActionDisconnectTest, _undo)
{
    CreateNewRouteConnectDisconnect();

    TCLAP::CmdLine *cmd(
        CAmCommandLineSingleton::instanciateOnce("AudioManager!", ' ', "7.5", true));

    CAmControllerPlugin *pPlugin     = new CAmControllerPlugin();
    CAmSocketHandler *pSocketHandler = new CAmSocketHandler();

    CAmTimerEvent *mpTimerEvent = CAmTimerEvent::getInstance();
    mpTimerEvent->setSocketHandle(pSocketHandler, pPlugin);

    CAmActionContainer pCAmActionContainer;
    mpActionRouteDisConnect->setUndoRequried(true);
    mpNewActionRouteDisConnect->setUndoRequried(true);
    pCAmActionContainer.append(mpActionRouteDisConnect);
    pCAmActionContainer.append(mpNewActionRouteDisConnect);
    pCAmActionContainer.setUndoRequried(true);

    // Handle for Disconnect
    mHandle = { H_DISCONNECT, 50 };
    EXPECT_CALL(*mpMockControlReceiveInterface, disconnect(_, 44)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), Return(E_OK)));
    EXPECT_EQ(mpActionRouteDisConnect->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);

    EXPECT_CALL(*mpMockControlReceiveInterface, disconnect(_, 45)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), Return(E_NOT_POSSIBLE)));
    EXPECT_EQ(mpNewActionRouteDisConnect->execute(), E_OK);

    // Handle for connect
    mHandle = { H_CONNECT, 50 };
    EXPECT_CALL(*mpMockControlReceiveInterface, connect(_, _, mGcRoutingElement.connectionFormat, mGcRoutingElement.sourceID, mGcRoutingElement.sinkID)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), SetArgReferee<1 >(44), Return(E_OK)));
    EXPECT_EQ(mpActionRouteDisConnect->undo(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
}

void *WorkerThread(void *data)
{
    CAmSocketHandler *pSocketHandler = (CAmSocketHandler *)data;
    pSocketHandler->start_listenting();
    return NULL;
}

/**
 * @brief : Test to verify the _timeout function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmRouteActionConnect CAmRouteActionDisconnect classes,
 *        Sink, Source,routing Elements are created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *        route element is used to initialize the CAmRouteActionConnect,CAmRouteActionDisconnect classes
 *
 * @test : verify the "_timeout" function when new route action connect, disconnect is created,  CAmSocketHandler,
 *         CAmTimerEvent, CAmActionContainer classes are initialized, set undo required flag to true, set the time out
 *         of the timer for route action disconnect. In Action container append the action route connect and disconnect.
 *         set the behavior of disconnect function to return "E_OK" with input
 *         parameter as mHandle of Disconnect, then call the execute method after disconnect call.
 *         Now set behavior of abortAction method to return E_OK  and sleep for 5 ms.
 *         finally set the connect function behavior to return "E_OK" then call the undo method
 *         then will check whether _timeout function will call the abortAction method of the CAmControlReceive or not,
 *         to abort the action, without any Gmock error.
 *
 * @result : "Pass" when _timeout function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteActionDisconnectTest, _timeout)
{
    CreateNewRouteConnectDisconnect();

    CAmSocketHandler pSocketHandler;
    pthread_t        thread_id;
    CAmControllerPlugin *pPlugin;
    CAmTimerEvent   *mpTimerEvent = CAmTimerEvent::getInstance();
    mpTimerEvent->setSocketHandle(&pSocketHandler, pPlugin);

    CAmActionContainer pCAmActionContainer;
    mpNewActionRouteDisConnect->setTimeout(3000);
    mpActionRouteDisConnect->setUndoRequried(true);
    mpNewActionRouteDisConnect->setUndoRequried(true);
    pCAmActionContainer.append(mpActionRouteDisConnect);
    pCAmActionContainer.append(mpNewActionRouteDisConnect);
    pCAmActionContainer.setUndoRequried(true);

    // Handle for Disconnect
    mHandle = { H_DISCONNECT, 50 };

    EXPECT_CALL(*mpMockControlReceiveInterface, disconnect(_, 44)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), Return(E_OK)));
    EXPECT_EQ(mpActionRouteDisConnect->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);

    EXPECT_CALL(*mpMockControlReceiveInterface, disconnect(_, 45)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), Return(E_OK)));
    EXPECT_EQ(mpNewActionRouteDisConnect->execute(), E_OK);

    pthread_create(&thread_id, NULL, WorkerThread, &pSocketHandler);
    EXPECT_CALL(*mpMockControlReceiveInterface, abortAction(IsHandleOk(mHandle))).WillOnce(
        (Return(E_OK)));
    sleep(5);

    // Handle for connect
    mHandle = { H_CONNECT, 50 };
    EXPECT_CALL(*mpMockControlReceiveInterface, connect(_, _, mGcRoutingElement.connectionFormat, mGcRoutingElement.sourceID, mGcRoutingElement.sinkID)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), SetArgReferee<1 >(44), Return(E_OK)));
    EXPECT_EQ(mpActionRouteDisConnect->undo(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
    pSocketHandler.exit_mainloop();
    pthread_join(thread_id, NULL);
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
