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
#include "CAmClassElement.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmActionDisconnect.h"
#include "CAmMainConnectionElement.h"
#include "CAmRouteElement.h"
#include "CAmSystemElement.h"
#include "CAmHandleStore.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmClassActionDisconnectTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmClassActionDisconnectTest
 *@brief : This class is used to test the CAmClassActionDisconnect functionality.
 */
CAmClassActionDisconnectTest::CAmClassActionDisconnectTest()
    : mpMockControlReceiveInterface(NULL)
    , mpControlReceive(NULL)
    , mpSinkElement(nullptr)
    , mpSourceElement(nullptr)
    , mpClassElement(nullptr)
    , mpMainConnection(nullptr)
    , mpRouteElement(nullptr)
    , mpClassActionDisConnect(NULL)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
{
}

CAmClassActionDisconnectTest::~CAmClassActionDisconnectTest()
{
}

void CAmClassActionDisconnectTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpControlReceive              = mpMockControlReceiveInterface;

    // This will check whether controller is initialized or not
    ASSERT_THAT(mpControlReceive, NotNull()) << " Controller Not Initialized";

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _))
    .WillRepeatedly(DoAll(SetArgReferee<1 >(gc_ut_MainConnection_s(CS_CONNECTED)), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(23, _))
    .WillRepeatedly(DoAll(SetArgReferee<1>(sourceInfo), Return(E_OK)));

    // Initialize Sink Element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSinkElement = CAmSinkFactory::createElement(sinkInfo, mpControlReceive);

    ASSERT_THAT(mpSinkElement, NotNull()) << " Sink Element Is not Created";

    sinkIDList.push_back(mpSinkElement->getID());

    // Initialize Source Element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(23), Return(E_OK)));
    mpSourceElement = CAmSourceFactory::createElement(sourceInfo, mpControlReceive);

    ASSERT_THAT(mpSourceElement, NotNull()) << " Source Element Is not Created";

    sourceIDList.push_back(mpSourceElement->getID());

    RoutingElement.connectionFormat = CF_GENIVI_STEREO;
    RoutingElement.domainID         = mpSinkElement->getDomainID();
    RoutingElement.sinkID           = mpSinkElement->getID();
    RoutingElement.sourceID         = mpSourceElement->getID();

    GCRoute.sinkID   = mpSinkElement->getID();
    GCRoute.sourceID = mpSourceElement->getID();
    GCRoute.name     = mpSourceElement->getName() + ":" + mpSinkElement->getName();
    GCRoute.route.push_back(RoutingElement);
    mRouteList.push_back(GCRoute);

    mMainConnectionData.mainConnectionID = 0;
    mMainConnectionData.sinkID           = mpSinkElement->getID();
    mMainConnectionData.sourceID         = mpSourceElement->getID();
    mMainConnectionData.delay            = 0;
    mMainConnectionData.connectionState  = CS_CONNECTED;

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 73;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 74;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classConnection.name     = "AnyClass1";
    classConnection.type     = C_CAPTURE;
    classConnection.priority = 1;
    systemproperty.type      = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value     = 5;

    std::vector<am_SystemProperty_s > listSystemProperties;
    listSystemProperties.push_back(systemproperty);
    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(
        DoAll(SetArgReferee<1 >(73), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(
        DoAll(SetArgReferee<0 >(74), Return(E_OK)));
    mpClassElement = CAmClassFactory::createElement(classConnection, mpControlReceive);

    ASSERT_THAT(mpClassElement, NotNull()) << " Class Element Is not Created";

    // creating the routing element
    gc_RoutingElement_s gcRoutingElement;
    gcRoutingElement.name             = mpSourceElement->getName() + ":" + mpSinkElement->getName();
    gcRoutingElement.sourceID         = mpSourceElement->getID();
    gcRoutingElement.sinkID           = mpSinkElement->getID();
    gcRoutingElement.domainID         = 2;
    gcRoutingElement.connectionFormat = CF_GENIVI_STEREO;

    mpRouteElement = CAmRouteFactory::createElement(gcRoutingElement, mpControlReceive);
    ASSERT_THAT(mpRouteElement, NotNull()) << " Route Element Is not Created";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(_, _))
    .WillOnce(DoAll(SetArgReferee<1>((am_mainConnectionID_t)44), Return(E_OK)));
    mpMainConnection = CAmMainConnectionFactory::createElement(GCRoute, mpControlReceive);
    ASSERT_THAT(mpMainConnection, NotNull()) << " MainConnection Element Is not Created";

    mpMainConnection->attach(mpSourceElement);
    mpMainConnection->attach(mpSinkElement);
    mpClassElement->attach(mpMainConnection);

    mpRouteElement->setID(44);
    mpRouteElement->setState(CS_CONNECTED);
    mpClassActionDisConnect = new CAmActionDisconnect();
    ASSERT_THAT(mpClassActionDisConnect, NotNull()) << " Class Action DisConnect Is not Created";
    CAmActionParam<std::string > classNameParam(sourceClass.name);
    mpClassActionDisConnect->setParam(ACTION_PARAM_CLASS_NAME, &classNameParam);
}

void CAmClassActionDisconnectTest::TearDown()
{
    CAmMainConnectionFactory::destroyElement();
    CAmRouteFactory::destroyElement();
    // sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    // sinkIDList.clear();
    CAmSinkFactory::destroyElement();
    CAmClassFactory::destroyElement();
    // CAmSystemFactory::destroyElement();

    if (mpClassActionDisConnect != NULL)
    {
        delete (mpClassActionDisConnect);
    }

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

/**
 * @brief  :  Test to verify the execute_update function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source, routing Elements are created with required input structure parameters like name,domainID,class name,
 *        registration type,volume etc. Also system and class element is created with required input configurations and source,sink elements
 *        are attached.CAmClassActionDisconnect class is initialized.
 *
 * @test : verify the "execute" function when for CAmClassActionDisconnect class, set the sink and source parameter and call getMainConnectionInfoDB function and
 *          chnage the MainConnectionStateDB  from CS_CONNECTED to CS_DISCONNECTING. and set the SourceState, also create the handle for disconnect
 *          then check whether execute function will creates the child action object if class is found whose connection needs to be
 *          disconnected. and It appends the child to base class for execution or not without any gmock error.
 *
 * @result : "Pass" when execute function return the expected output "E_OK".
 */
TEST_F(CAmClassActionDisconnectTest, execute_update_PositiveCase)
{
    mSourceNameParam.setParam(mpSourceElement->getName());
    mSinkNameParam.setParam(mpSinkElement->getName());

    mpClassActionDisConnect->setParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    mpClassActionDisConnect->setParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    // Handle for sourceState
    mHandle = { H_SETSOURCESTATE, 50 };
    EXPECT_EQ(mpClassActionDisConnect->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
    // Handle for Disconnect
    mHandle = { H_DISCONNECT, 50 };
    EXPECT_EQ(mpClassActionDisConnect->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
    EXPECT_EQ(mpClassActionDisConnect->update(CS_DISCONNECTED), E_OK);
}

/**
 * @brief  :  Test to verify the execute_update function Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source, routing Elements are created with required input structure parameters like name,domainID,class name,
 *        registration type,volume etc. Also system and class element is created with required input configurations and source,sink elements
 *        are attached.CAmClassActionDisconnect class is initialized.
 *
 * @test : verify the "execute" function when for CAmClassActionDisconnect class we set the different sink and source parameter and call getMainConnectionInfoDB function
 *          with argument as CS_CONNECTED then check whether execute function will creates the child action object if class is found whose
 *          connection needs to be disconnected. and It appends the child to base class for execution or not without any gmock error.
 *
 * @result : "Pass" when execute function return the expected output "E_UNKNOWN".
 */
TEST_F(CAmClassActionDisconnectTest, execute_NegativeCase)
{
    mSourceNameParam.setParam("Media");
    mSinkNameParam.setParam("Speaker");

    mpClassActionDisConnect->setParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    mpClassActionDisConnect->setParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _)).WillRepeatedly(
        DoAll(SetArgReferee<1 >(gc_ut_MainConnection_s(CS_CONNECTED)), Return(E_OK)));
    EXPECT_EQ(mpClassActionDisConnect->execute(), E_OK);
}

/**
 * @brief  :  Test to verify the Updated function Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source, routing Elements are created with required input structure parameters like name,domainID,class name,
 *        registration type,volume etc. Also system and class element is created with required input configurations and source,sink elements
 *        are attached.CAmClassActionDisconnect class is initialized.
 *
 * @test : verify the "Updated" function when for CAmClassActionDisconnect class we set the sink parameter and
 *          call getMainConnectionInfoDB function with argument as CS_CONNECTED then check whether Updated function
 *          API dispose the connection if its child action completes successfully or not without any gmock error.
 *
 * @result : "Pass" when execute function return the expected output "E_OK".
 */
TEST_F(CAmClassActionDisconnectTest, Updated_Negative_Case)
{
    mSinkNameParam.setParam(mpSinkElement->getName());
    mpClassActionDisConnect->setParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _)).WillRepeatedly(
        DoAll(SetArgReferee<1 >(gc_ut_MainConnection_s(CS_CONNECTED)), Return(E_OK)));
    mHandle = { H_SETSOURCESTATE, 50 };
    EXPECT_EQ(mpClassActionDisConnect->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
    // Handle for Disconnect
    mHandle = { H_DISCONNECT, 50 };
    EXPECT_EQ(mpClassActionDisConnect->execute(), E_OK);

    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
    EXPECT_EQ(mpClassActionDisConnect->update(CS_DISCONNECTED), E_OK);
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
