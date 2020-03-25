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
#include "CAmMainConnectionElement.h"
#include "CAmRouteActionConnect.h"
#include "CAmRouteElement.h"
#include "CAmActionContainer.h"
#include "CAmTimerEvent.h"
#include "CAmHandleStore.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmRouteActionConnectTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmRouteActionConnectTest
 *@brief : This class is used to test the CAmRouteActionConnect class functionality.
 */
CAmRouteActionConnectTest::CAmRouteActionConnectTest()
    : mpMockControlReceiveInterface(NULL)
    , mpControlReceive(NULL)
    , mpSinkElement(nullptr)
    , mpNewSinkElement(nullptr)
    , mpSourceElement(nullptr)
    , mpNewSourceElement(nullptr)
    , mpRoutingElement(nullptr)
    , mpNewRoutingElement(nullptr)
    , mpActionRouteConnect(NULL)
    , mpNewActionRouteConnect(NULL)
{
}

CAmRouteActionConnectTest::~CAmRouteActionConnectTest()
{
}

void CAmRouteActionConnectTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpControlReceive              = mpMockControlReceiveInterface;

    // This will check whether controller is initialized or not
    ASSERT_THAT(mpControlReceive, NotNull()) << " Controller Not Initialized";

    // Initialize Source Element
    ut_gc_Source_s sourceInfo("AnySource1");
    // Initialize Sink Element
    ut_gc_Sink_s sinkInfo("AnySink1");

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _))
   .WillRepeatedly(DoAll(SetArgReferee<1 >(gc_ut_MainConnection_s(CS_CONNECTED)), Return(E_OK)));
   EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(23, _))
   .WillRepeatedly(DoAll(SetArgReferee<1>(sourceInfo), Return(E_OK)));
   EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(22, _))
   .WillRepeatedly(DoAll(SetArgReferee<1>(sinkInfo), Return(E_OK)));

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSinkElement = CAmSinkFactory::createElement(sinkInfo, mpControlReceive);

    ASSERT_THAT(mpSinkElement, NotNull()) << " Sink Element Is not Created";

    sinkIDList.push_back(mpSinkElement->getID());

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
    // mpActionRouteConnect = CAmRouteActionf  ::createElement(mpRoutingElement);
    ASSERT_THAT(mpActionRouteConnect, NotNull()) << " RouteAction Is not Created";
}

void CAmRouteActionConnectTest::TearDown()
{
    if (mpActionRouteConnect != NULL)
    {
        delete (mpActionRouteConnect);
        mpActionRouteConnect = NULL;
    }

    mpRoutingElement = nullptr;
    CAmRouteFactory::destroyElement();

    sourceIDList.clear();
    mpSourceElement = nullptr;
    CAmSourceFactory::destroyElement();

    sinkIDList.clear();
    mpSinkElement = nullptr;
    CAmSinkFactory::destroyElement();

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
        mpMockControlReceiveInterface = NULL;
    }
}

void CAmRouteActionConnectTest::CreateNewRoute()
{
    // Initialize Sink Element
    ut_gc_Sink_s sinkInfo("AnySink2");

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(23), Return(E_OK)));
    mpNewSinkElement = CAmSinkFactory::createElement(sinkInfo, mpControlReceive);

    ASSERT_THAT(mpNewSinkElement, NotNull()) << " Sink Element Is not Created";

    sinkIDList.push_back(mpSinkElement->getID());

    // Initialize Source Element
    ut_gc_Source_s sourceInfo("AnySource2");

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

}

/**
 * @brief : Test to verify the _execute_Update function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmRouteActionConnect classes,
 *        Sink, Source,routing Elements are created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *
 * @test : verify the "_execute_Update" function when set the behavior of connect function to return "E_OK" with input
 *         parameter as connection format,routing side source id,sink id provided  then will check whether
 *         _execute_Update function will invokes the control receive class function to send request to AM. and It
 *         register the observer to receive the acknowledgment of action completion or not,
 *         and then check updates is done or not for the connection state of router level connection
 *         using notifyAsyncResult function with state as E_OK or not ,without any Gmock error.
 *
 * @result : "Pass" when _execute_Update function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteActionConnectTest, _execute_Update_PositiveCase)
{
    // Handle for connect
    mHandle = { H_CONNECT, 50 };
    // calling the execute method
    EXPECT_CALL(*mpMockControlReceiveInterface, connect(_, _, mGcRoutingElement.connectionFormat, mGcRoutingElement.sourceID, mGcRoutingElement.sinkID)).WillOnce(
        DoAll(SetArgReferee<0>(mHandle), SetArgReferee<1>(44), Return(E_OK)));
    EXPECT_EQ(mpActionRouteConnect->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
}

/**
 * @brief : Test to verify the _execute_Update function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmRouteActionConnect classes,
 *        Sink, Source,routing Elements are created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *
 * @test : verify the "_execute_Update" function when set the behavior of connect function to return "E_NOT_POSSIBLE" with input
 *         parameter as connection format,routing side source id,sink id provided  then will check whether
 *         _execute_Update function will invokes the control receive class function to send request to AM. and It
 *         register the observer to receive the acknowledgment of action completion or not,
 *         without any Gmock error.
 *
 * @result : "Pass" when _execute_Update function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteActionConnectTest, _execute_NegativeCase)
{
    // Handle for connect
    mHandle = { H_CONNECT, 50 };
    // calling the execute method
    EXPECT_CALL(*mpMockControlReceiveInterface, connect(_, _, mGcRoutingElement.connectionFormat, mGcRoutingElement.sourceID, mGcRoutingElement.sinkID)).WillOnce(
        DoAll(SetArgReferee<0>(mHandle), SetArgReferee<1>(44), Return(E_OK)));
    EXPECT_EQ(mpActionRouteConnect->execute(), E_OK);
    EXPECT_EQ(mpActionRouteConnect->execute(), E_OK);
}

/**
 * @brief : Test to verify the _execute_Update function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmRouteActionConnect classes,
 *        Sink, Source,routing Elements are created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *
 * @test : verify the "_execute_Update" function when connection is set to  "CS_CONNECTED" then will check whether
 *         _execute_Update function will invokes the control receive class function to send request to AM. and It
 *         register the observer to receive the acknowledgment of action completion or not,
 *         and then check updates is done or not for the connection state of router level connection,
 *         without any Gmock error.
 *
 * @result : "Pass" when _execute_Update function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteActionConnectTest, _execute_NegativeCase_AlreadyConnected)
{
    mpRoutingElement->setState(CS_CONNECTED);
    // calling the execute method
    EXPECT_EQ(mpActionRouteConnect->execute(), E_OK);
}

/**
 * @brief : Test to verify the _execute_Update function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmRouteActionConnect classes,
 *        Sink, Source,routing Elements are created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *
 * @test : verify the "_execute_Update" function when set the behavior of connect function to return "E_OK" with input
 *         parameter as connection format,routing side source id,sink id provided  then will check whether
 *         _execute_Update function will invokes the control receive class function to send request to AM. and It
 *         register the observer to receive the acknowledgment of action completion or not,
 *         and then also check updates is done or not for the connection state of router level connection
 *         using notifyAsyncResult function with state as E_NOT_POSSIBLE ,without any Gmock error.
 *
 * @result : "Pass" when _execute_Update function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteActionConnectTest, _execute_updated_NegativeCase)
{
    // Handle for connect
    mHandle = { H_CONNECT, 50 };
    EXPECT_CALL(*mpMockControlReceiveInterface, connect(_, _, mGcRoutingElement.connectionFormat, mGcRoutingElement.sourceID, mGcRoutingElement.sinkID)).WillOnce(
        DoAll(SetArgReferee<0>(mHandle), SetArgReferee<1>(44), Return(E_OK)));
    EXPECT_EQ(mpActionRouteConnect->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_NOT_POSSIBLE);
}

/**
 * @brief : Test to verify the _undo function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmRouteActionConnect classes,
 *        Sink, Source,routing Elements are created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *
 * @test : verify the "_undo" function when set the Undo required flag to true ,append route connect action,
 *         and new route connect action in the action container, set the behavior of connect function for both route
 *         connections to return "E_OK" first time and then "E_NOT_POSSIBLE" for second then do the execute call
 *         then do the disconnect function call then will check whether _undo function will
 *         performs the undo operation or not if parent has requested for undo ,without any Gmock error.
 *
 * @result : "Pass" when _undo function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteActionConnectTest, _undo)
{
    EXPECT_EQ(mpActionRouteConnect->undo(), E_OK);
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
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmRouteActionConnect classes,
 *        Sink, Source,routing Elements are created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element is insert in the db.
 *
 * @test : verify the "_timeout" function when time out is set for he route action connect
 *         & undo function behavior is set to return "E_OK" then will check after  _timeout function
 *          stop the listening and exit the main loop or not, without any Gmock error.
 *
 * @result : "Pass" when _undo function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteActionConnectTest, _timeout)
{
    ASSERT_NO_THROW(mpActionRouteConnect->getTimeout());
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
