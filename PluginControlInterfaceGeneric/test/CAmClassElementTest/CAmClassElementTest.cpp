/*******************************************************************************
 *  \copyright (c) 2018 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com> 2018
 *           Kapildev Patel, Yogesh Sharma <kpatel@jp.adit-jv.com> 2018
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
#include "CAmClassElement.h"
#include "CAmSystemElement.h"
#include "CAmMainConnectionElement.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmClassElementTest.h"
#include "CAmTestConfigurations.h"

#include "pthread.h"
#include <atomic>

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmClassElementTest
 *@brief : This class is used to test the CAmClassElement functionality.
 *
 */
CAmClassElementTest::CAmClassElementTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmSinkElement(nullptr)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
    , mpCAmSourceElement(nullptr)
    , mpCAmRouteElement(NULL)
    , mpCAmClassElement(nullptr)
{
}

CAmClassElementTest::~CAmClassElementTest()
{

}

void CAmClassElementTest::InitializeCommonStruct()
{
    sink.name                       = "AnySink1";
    source.name                     = "AnySource1";
    source.sourceClassID            = 81;
    sink.sinkClassID                = 83;
    mainConnectionID                = 0;
    classPropertyData.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classPropertyData.value         = 1;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(74), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(75), Return(E_OK)));

}

void CAmClassElementTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;
    ASSERT_THAT(mpCAmControlReceive, NotNull()) << " Controller Not Initialized";
    InitializeCommonStruct();
    EXPECT_CALL(*mpMockControlReceiveInterface,
        enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee < 1 > (22), Return(E_OK)));
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo,
            mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    source.name = "AnySource1";
    EXPECT_CALL(*mpMockControlReceiveInterface,
        enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee < 1 > (22), Return(E_OK)));
    mpCAmSourceElement = CAmSourceFactory::createElement(sourceInfo,
            mpCAmControlReceive);
    ASSERT_THAT(mpCAmSourceElement, NotNull())
        << " Source Element Is not Created";
    sourceIDList.push_back(mpCAmSourceElement->getID());
    sourceID = mpCAmSourceElement->getID();

    routingElement.connectionFormat = CF_GENIVI_STEREO;
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.sourceID         = mpCAmSourceElement->getID();

    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    listRoutes.push_back(gcRoute);

    mainConnectionID                    = 0;
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // create new class object for each class
    mpCAmClassElement                 = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    gcRoutingElement.name             = mpCAmSourceElement->getName() + mpCAmSinkElement->getName();
    gcRoutingElement.sourceID         = mpCAmSourceElement->getID();
    gcRoutingElement.sinkID           = mpCAmSinkElement->getID();
    gcRoutingElement.domainID         = 2;
    gcRoutingElement.connectionFormat = CF_GENIVI_STEREO;
    mpCAmRouteElement                 = new CAmRouteElement(gcRoutingElement,
            mpCAmControlReceive);
    ASSERT_THAT(mpCAmRouteElement, NotNull())
        << " Route Element Is not Created";
}

void CAmClassElementTest::TearDown()
{

    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();
    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }

    CAmClassFactory::destroyElement();

    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief  :  Test to verify the isSourceSinkPairBelongtoClass function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "isSourceSinkPairBelongtoClass" function when sink, source name is given as input parameters
 *               then it will check whether sink and source belong to that given class or not.
 *
 * @result : "Pass" when isSourceSinkPairBelongtoClass function return the expected output "True".
 */

TEST_F(CAmClassElementTest, isSourceSinkPairBelongtoClassPositive)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value  = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System1";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // Creating Class Element instancce
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    EXPECT_TRUE(mpCAmClassElement->isSourceSinkPairBelongtoClass(sink.name, source.name));
}

/**
 * @brief  :  Test to verify the isSourceSinkPairBelongtoClassNegative function Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "isSourceSinkPairBelongtoClass" function when different sink, source name is given as input parameters
 *               then it will check whether sink and source belong to that given class or not.
 *
 * @result : "Pass" when isSourceSinkPairBelongtoClass function return the expected output "False".
 */

TEST_F(CAmClassElementTest, isSourceSinkPairBelongtoClassNegative)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System2";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    EXPECT_FALSE(mpCAmClassElement->isSourceSinkPairBelongtoClass("AllSpeakers", "Navi"));
}

/**
 * @brief  :  Test to verify the createMainConnection function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "createMainConnection" function when sink, source name and mainConnectionID are given as input parameters
 *               then it will check whether createMainConnection function create the new main connection or not
 *
 * @result : "Pass" when createMainConnection function return the expected output "E_OK".
 */

TEST_F(CAmClassElementTest, createMainConnectionPositive)
{
    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value  = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System3";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    mainConnectionData.connectionState = CS_CONNECTED;
    // List of Route information
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Am route Info
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    if (mpCAmClassElement == NULL)
    {
        EXPECT_EQ(E_OK, mpCAmClassElement->createMainConnection(source.name, sink.name, mainConnectionID));
    }
}

/**
 * @brief  :  Test to verify the createMainConnection function Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "createMainConnection" function when different value for sink,source name are given as input parameters
 *               then it will check whether createMainConnection function create the new main connection or not
 *
 * @result : "Pass" when createMainConnection function return the expected output "E_NOT_POSSIBLE".
 */
TEST_F(CAmClassElementTest, createMainConnectionNegative)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value  = 5;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System4";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // Creating Class Element instancce
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmClassElement->createMainConnection("Navi", "AllSpeakers", mainConnectionID));
}

/**
 * @brief  :  Test to verify the disposeConnection function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with requirgetMainConnectioned input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "disposeConnection" function when CAmMainConnectionElement are given as input parameters
 *               then it will check whether CAmMainConnectionElement is destroy or not with out any error message
 *
 * @result : "Pass" when disposeConnection function return no Gmock assert error message
 */
TEST_F(CAmClassElementTest, disposeConnectionPositive)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value  = 5;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System5";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    routingElement.sourceID             = mpCAmSourceElement->getID();
    routingElement.sinkID               = mpCAmSinkElement->getID();
    routingElement.domainID             = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat     = CF_GENIVI_STEREO;
    amRoute.sourceID                    = mpCAmSourceElement->getID();
    amRoute.sinkID                      = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutinElement.name = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(46), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(46, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    std::vector<std::string> listMainConnections;
    listMainConnections.push_back(gcRoute.name);
    std::string mainconnectionName;
    mainconnectionName = mpCAmClassElement->getMainConnectionString();
    gc_LastMainConnections_s mLastMainConnections;
    mLastMainConnections.className = classElement.name;
    EXPECT_CALL(*mpMockControlReceiveInterface, removeMainConnectionDB(46)).WillOnce(Return(E_OK));
    mpCAmClassElement->disposeConnection(mpCAmMainConnectionElement);

}

/**
 * @brief  :  Test to verify the disposeConnection Negative Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "disposeConnection" function when CAmMainConnectionElement are given as input parameters with incorrect configuration
 *               then it will check whether CAmMainConnectionElement is destroy or not with out any error message
 *
 * @result : "Pass" when disposeConnection function return no Gmock assert error message
 */
TEST_F(CAmClassElementTest, disposeConnectionNegative)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System6";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // Creating Class Element instancce
    mainConnectionData.connectionState = CS_CONNECTED;
    routingElement.sourceID            = mpCAmSourceElement->getID();
    routingElement.sinkID              = mpCAmSinkElement->getID();
    routingElement.domainID            = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat    = CF_GENIVI_STEREO;
    amRoute.sourceID                   = mpCAmSourceElement->getID();
    amRoute.sinkID                     = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutinElement.name       = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mpCAmMainConnectionElement = nullptr;
    mpCAmClassElement->disposeConnection(mpCAmMainConnectionElement);

}

/**
 * @brief  :  Test to verify the getMainConnection with order high Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getMainConnection" function when sink,source name and listConnectionStates and high order are given as input parameters with correct configuration
 *                then it will check whether getMainConnection function will return the CAmMainConnectionelement or not
 *
 * @result : "Pass" when getMainConnection function return pointer of CAmMainConnectionelement without any error.
 */
TEST_F(CAmClassElementTest, getMainConnectionPositiveOrderHigh)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System7";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);

    // List of Route information
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Am route Info
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    // Gc Route Information
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    // Main Connection data information
    mainConnectionData.mainConnectionID = 1;
    mainConnectionData.connectionState  = CS_CONNECTED;

    am_ConnectionState_e connectionstate = CS_CONNECTED;
    listConnectionStates.push_back(connectionstate);
    gc_Order_e order = O_HIGH_PRIORITY;
    gcRoute.sinkID   = listRoutes.begin()->sinkID;
    gcRoute.sourceID = listRoutes.begin()->sourceID;
    gcRoute.route    = listRoutes.begin()->route;
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();

    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(50), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(50, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_EQ(mpCAmMainConnectionElement, mpCAmClassElement->getMainConnection(source.name, sink.name, listConnectionStates, order));

}

/**
 * @brief  :  Test to verify the getMainConnection with order Low Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getMainConnection" function when sink,source name and listConnectionStates and Low order are given as input parameters with correct configuration
 *                then it will check whether getMainConnection function will return the CAmMainConnectionelement or not
 *
 * @result : "Pass" when getMainConnection function return pointer of CAmMainConnectionelement without any error.
 */
TEST_F(CAmClassElementTest, getMainConnectionPositiveOrderLow)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value  = 5;

    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System8";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Am route Info
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    // Gc Route Information
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    // Main Connection data information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // Conneciton state
    am_ConnectionState_e connectionstate = CS_CONNECTED;
    listConnectionStates.push_back(connectionstate);

    // Gc Oreder
    gc_Order_e order = O_LOW_PRIORITY;

    gcRoute.sinkID   = listRoutes.begin()->sinkID;
    gcRoute.sourceID = listRoutes.begin()->sourceID;
    gcRoute.route    = listRoutes.begin()->route;
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(49), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(49, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_EQ(mpCAmMainConnectionElement, mpCAmClassElement->getMainConnection(source.name, sink.name, listConnectionStates, order));

}

/**
 * @brief  :  Test to verify the getMainConnection with order Oldest Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getMainConnection" function when sink,source name and listConnectionStates and Oldest order are given as input parameters with correct configuration
 *                then it will check whether getMainConnection function will return the CAmMainConnectionelement or not
 *
 * @result : "Pass" when getMainConnection function will return pointer of CAmMainConnectionelement without any error.
 */
TEST_F(CAmClassElementTest, getMainConnectionPositiveOrderOldest)
{
    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System9";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // Creating Class Element instancce
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Am route Info
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    // Gc Route Information
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    // Main Connection data information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // Conneciton state
    am_ConnectionState_e connectionstate = CS_CONNECTED;
    listConnectionStates.push_back(connectionstate);
    gc_Order_e order = O_OLDEST;

    gcRoute.sinkID   = listRoutes.begin()->sinkID;
    gcRoute.sourceID = listRoutes.begin()->sourceID;
    gcRoute.route    = listRoutes.begin()->route;
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();

    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(51), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(51, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_EQ(mpCAmMainConnectionElement, mpCAmClassElement->getMainConnection(source.name, sink.name, listConnectionStates, order));

}

/**
 * @brief  :  Test to verify the getMainConnection with order Newest Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getMainConnection" function when sink,source name and listConnectionStates and Newest order are given as input parameters with correct configuration
 *                then it will check whether getMainConnection function will return the CAmMainConnectionelement or not
 *
 * @result : "Pass" when getMainConnection function will return pointer of CAmMainConnectionelement without any error.
 */
TEST_F(CAmClassElementTest, getMainConnectionPositiveOrderNewest)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System10";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    source.name        = "AnySource1";
    am_sourceID_t sourceID = 0;
    sourceID  = mpCAmSourceElement->getID();
    sink.name = "AnySink1";
    am_sinkID_t sinkID = 0;
    sinkID = mpCAmSinkElement->getID();
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    amRoute.sourceID                = mpCAmSourceElement->getID();
    amRoute.sinkID                  = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    // Main Connection data information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // Conneciton state
    am_ConnectionState_e connectionstate = CS_CONNECTED;
    listConnectionStates.push_back(connectionstate);
    gc_Order_e order = O_NEWEST;

    gcRoute.sinkID   = listRoutes.begin()->sinkID;
    gcRoute.sourceID = listRoutes.begin()->sourceID;
    gcRoute.route    = listRoutes.begin()->route;
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();

    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(50), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(50, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_EQ(mpCAmMainConnectionElement, mpCAmClassElement->getMainConnection(source.name, sink.name, listConnectionStates, order));
}

/**
 * @brief  :  Test to verify the getMainConnection with Negative scenario.
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getMainConnection" function when sink,source name and listConnectionStates and Newest order are given as input parameters with correct configuration
 *                then it will check whether getMainConnection function will not return the E_NOT_POSSIBLE or not
 *
 * @result : "Pass" when getMainConnection function will return the "E_NOT_POSSIBLE".
 */
TEST_F(CAmClassElementTest, getMainConnectionNegative)
{
    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value  = 5;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System11";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    source.name        = "AnySource11";
    am_sourceID_t        sourceID        = 0;
    am_ConnectionState_e connectionstate = CS_CONNECTED;
    listConnectionStates.push_back(connectionstate);
    gc_Order_e order = O_NEWEST;
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmClassElement->createMainConnection("AUX", "BTSpeaker", mainConnectionID));
    EXPECT_EQ(mpCAmMainConnectionElement, mpCAmClassElement->getMainConnection(source.name, sink.name, listConnectionStates, order));
}

/**
 * @brief  :  Test to verify the getMainConnection with Positive scenario Only
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getMainConnection" function when list of main connection is given as input parameters with correct configuration
 *                then it will check whether getMainConnection function will return the any Gmock error message or not
 *
 * @result : "Pass" when getMainConnection function will not return any gmock error message.
 */

TEST_F(CAmClassElementTest, getListMainConnectionsPositive)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System12";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    amRoute.sourceID                = mpCAmSourceElement->getID();
    amRoute.sinkID                  = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(52), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(52, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);

}

/**
 * @brief  :  Test to verify the pushMainConnectionInQueue with Positive scenario Only
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "pushMainConnectionInQueue" function when main connection element is given as input parameters with correct configuration
 *                then it will check whether pushMainConnectionInQueue function will return the any Gmock error message or not
 *
 * @result : "Pass" when getMainConnection function will not return any gmock error message.
 */

TEST_F(CAmClassElementTest, pushMainConnectionInQueuePositive)
{

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System13";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    // Main Connection data information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // Routing element
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    gcRoute.sinkID                  = mpCAmSinkElement->getID();
    gcRoute.sourceID                = mpCAmSourceElement->getID();
    gcRoute.name                    = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(53), Return(E_OK)));
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(53, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
}

/**
 * @brief  :  Test to verify the pushMainConnectionInQueuePositivewithExistingconnection with Positive scenario Only
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "pushMainConnectionInQueue" function when existing main connection is given as input parameters with correct configuration
 *                then it will check whether pushMainConnectionInQueue function will return the any Gmock error message or not
 *
 * @result : "Pass" when getMainConnection function will not return any gmock error message.
 */

TEST_F(CAmClassElementTest, pushMainConnectionInQueuePositivewithExistingconnection)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value  = 5;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System14";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    routingElement.sourceID             = mpCAmSourceElement->getID();
    routingElement.sinkID               = mpCAmSinkElement->getID();
    routingElement.domainID             = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat     = CF_GENIVI_STEREO;
    amRoute.sourceID                    = mpCAmSourceElement->getID();
    amRoute.sinkID                      = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(54), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(54, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
}
#ifdef VOLUME_RELATED_TEST
/**
 * @brief  :  Test to verify the setLimitState function with Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "setLimitState" function when volume limit,state limit and pattern information is passed as input parameters with correct configuration
 *                then it will check whether setLimitState function will set the volume limit,state limit and it will return the E_OK, with any error message
 *
 * @result : "Pass" when setLimitState function will return the expected output "E_OK".
 */
TEST_F(CAmClassElementTest, setLimitStatePositiveLimited)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    systemConfiguration.name = "System15";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    gc_LimitState_e limitState = LS_LIMITED;
    limitVolume.limitType   = LT_RELATIVE;
    limitVolume.limitVolume = 10;
    uint32_t pattern = 15;
    EXPECT_EQ(E_OK, mpCAmClassElement->setLimitState(limitState, limitVolume, pattern));
}

/**
 * @brief  :  Test to verify the setLimitStatePositiveUnLimited function with Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "setLimitState" function when  unlimited volume , unlimited state and pattern information is passed as input parameters with correct configuration
 *                then it will check whether setLimitState function will set the unlimited volume and state, pattern information
 *                in the list maintained by the class or not.
 *
 * @result : "Pass" when setLimitState function will return the expected output "E_OK".
 */

TEST_F(CAmClassElementTest, setLimitStatePositiveUnLimited)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value  = 5;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System16";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    gc_LimitState_e limitStatelimited = LS_LIMITED;
    limitVolume.limitType   = LT_RELATIVE;
    limitVolume.limitVolume = 4;
    uint32_t pattern = 1;
    EXPECT_EQ(E_OK, mpCAmClassElement->setLimitState(limitStatelimited, limitVolume, pattern));
    gc_LimitState_e  limitStateUnlimited = LS_UNLIMITED;
    gc_LimitVolume_s limitVolumeunlmt;
    limitVolumeunlmt.limitType   = LT_ABSOLUTE;
    limitVolumeunlmt.limitVolume = 4;
    uint32_t patternunlmt = 1;
    EXPECT_EQ(E_OK, mpCAmClassElement->setLimitState(limitStateUnlimited, limitVolumeunlmt, patternunlmt));
}

/**
 * @brief  :  Test to verify the setLimitState function with UnLimited increase volume for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "setLimitState" function when  unlimited volume is increased, unlimited state and pattern information is passed as input parameters with correct configuration
 *                then it will check whether setLimitState function will set the increased volume and state, pattern information
 *                in the list maintained by the class or not.
 *
 * @result : "Pass" when setLimitState function will return the expected output "E_OK".
 */
TEST_F(CAmClassElementTest, setLimitStatePositiveUnLimitedincreasevol)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value  = 5;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System17";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    gc_LimitState_e limitStatelimited = LS_LIMITED;
    limitVolume.limitType   = LT_RELATIVE;
    limitVolume.limitVolume = 4;
    uint32_t pattern = 1;
    EXPECT_EQ(E_OK, mpCAmClassElement->setLimitState(limitStatelimited, limitVolume, pattern));
    gc_LimitState_e  limitStateUnlimited = LS_UNLIMITED;
    gc_LimitVolume_s limitVolumeunlmt;
    limitVolumeunlmt.limitType   = LT_ABSOLUTE;
    limitVolumeunlmt.limitVolume = 4;
    uint32_t patternunlmt = 12;
    EXPECT_EQ(E_OK, mpCAmClassElement->setLimitState(limitStateUnlimited, limitVolumeunlmt, patternunlmt));
}
#endif
/**
 * @brief  :  Test to verify the getListMainConnections function with Connection Filter for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getListMainConnections" function when  route information and main connection information along with CAmConnectionListFilter object
 *         are passed as input parameters with correct configuration then it will check whether getListMainConnections function
 *         will return main connection list or not.
 *
 * @result : "Pass" when getListMainConnections function will return the no gmock error message.
 */
TEST_F(CAmClassElementTest, getListMainConnectionsPositivewithConnectionfilterobject)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;

    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System18";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    amRoute.sourceID                = mpCAmSourceElement->getID();
    amRoute.sinkID                  = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    // Gc Route Information
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    // Main Connection data information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    CAmConnectionListFilter filterObject;
    // Setting source name using Connection filter object
    filterObject.setSourceName(source.name);
    // Setting sink name using Connection filter object
    filterObject.setSinkName(sink.name);
    // Conneciton state
    am_ConnectionState_e connectionstate = CS_CONNECTED;
    listConnectionStates.push_back(connectionstate);
    // Setting list of connection state using Connection filter object
    filterObject.setListConnectionStates(listConnectionStates);
}

/**
 * @brief  :  Test to verify the getListMainConnections function with Connection Filter and connection state list empyt for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getListMainConnections" function when  route information and main connection list empty along with CAmConnectionListFilter object
 *         are passed as input parameters with correct configuration then it will check whether getListMainConnections function
 *         will return main connection list or not.
 *
 * @result : "Pass" when getListMainConnections function will return the no gmock error message
 */
TEST_F(CAmClassElementTest, getListMainConnectionsPositivewithConnectionfilterobjectconnectionstatelistempty)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System19";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;

    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    // Gc Route Information

    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    // Main Connection data information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // Setting source name using Connection filter object
    filterObject.setSourceName(source.name);
    // Setting sink name using Connection filter object
    filterObject.setSinkName(sink.name);
    // Conneciton state
    am_ConnectionState_e connectionstate = CS_CONNECTED;
    // Setting list of connection state using Connection filter object
    filterObject.setListConnectionStates(listConnectionStates);
}
#ifdef VOLUME_RELATED_TEST
/**
 * @brief  :  Test to verify the getMuteState function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getMuteState" function when we set the limit state and required pattern using the setMuteState function with correct input
 *         parameters and with correct configuration then it will check whether getMuteState function
 *         will return mute state as accepted or not.
 *
 * @result : "Pass" when getMuteState function will return "MS_MUTED"
 */
TEST_F(CAmClassElementTest, getMuteStatePositive)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System20";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    gc_LimitState_e limitState = LS_LIMITED;
    limitVolume.limitType   = LT_ABSOLUTE;
    limitVolume.limitVolume = -3000;
    uint32_t pattern = 15;
    EXPECT_EQ(E_OK, mpCAmClassElement->setLimitState(limitState, limitVolume, pattern));
    am_MuteState_e muteState = MS_MUTED;
    EXPECT_EQ(muteState, mpCAmClassElement->getMuteState());
}

/**
 * @brief  :  Test to verify the getMuteState function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getMuteState" function when we set the limit state and required pattern using the setMuteState function with correct input
 *         parameters and with correct configuration then it will check whether getMuteState function
 *         will return mute state as accepted or not.
 *
 * @result : "Pass" when getMuteState function will return "MS_UNMUTED"
 */
TEST_F(CAmClassElementTest, getMuteStateNegative)
{
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System21";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // Creating Class Element instancce
    gc_LimitState_e limitState = LS_LIMITED;
    limitVolume.limitType   = LT_RELATIVE;
    limitVolume.limitVolume = -3000;
    uint32_t pattern = 15;
    EXPECT_EQ(E_OK, mpCAmClassElement->setLimitState(limitState, limitVolume, pattern));
    am_MuteState_e muteState = MS_UNMUTED;
    EXPECT_EQ(muteState, mpCAmClassElement->getMuteState());
}
#endif
/**
 * @brief  :  Test to verify the getElement function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElement" function when correct name of source and sink is given as input parameters along with correct other configuration
 *         parameters then check whether getElement function will class element or not
 *
 * @result : "Pass" when getElement function will not return any gmock error message
 */
TEST_F(CAmClassElementTest, getElementPositive)
{
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 73;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 74;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System22";
    EXPECT_EQ(mpCAmClassElement, mpCAmClassElement->getElement());
}

/**
 * @brief  :  Test to verify the getElement function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElement" function when incorrect name of source and sink is given as input parameters along with correct other configuration
 *         parameters then check whether getElement function will return NULL or not
 *
 * @result : "Pass" when getElement function will return "NULL"
 */
TEST_F(CAmClassElementTest, getElementNegative)
{
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 74;
    sinkClass.name                  = "AnySinkClass2";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 75;
    sourceClass.name                = "AnySourceClass2";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classElement.name     = "AnyClass2";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass2";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System23";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(74), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(75), Return(E_OK)));
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    ASSERT_NO_THROW(CAmClassFactory::getElement("Media", "CarSpeaker"));

}

/**
 * @brief  :  Test to verify the getElementsBySource function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElementsBySource" function when source name is given as input parameters along with correct other configuration
 *         parameters then check whether getElement function will return list of class element
 *
 * @result : "Pass" when getElementsBySource function will return no gmock error
 */

TEST_F(CAmClassElementTest, getElementsBySourcePositive)
{

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 74;
    sinkClass.name                  = "AnySinkClass3";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 75;
    sourceClass.name                = "AnySourceClass3";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classElement.name     = "AnyClass3";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass3";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value  = 5;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System24";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(74), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(75), Return(E_OK)));
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == mpCAmClassElement)
    {
        ASSERT_THAT(mpCAmClassElement, IsNull()) << " CAmClassElementTest: Not able to create classElement object ";
    }

    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
}

/**
 * @brief  :  Test to verify the getElementsBySource function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElementsBySource" function when incorrect source name is given as input parameters along with correct other configuration
 *         parameters then check whether getElement function will return list of class element
 *
 * @result : "Pass" when getElementsBySource function will return no gmock error
 */
TEST_F(CAmClassElementTest, getElementsBySourceNegative)
{
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 75;
    sinkClass.name                  = "AnySinkClass4";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 76;
    sourceClass.name                = "AnySourceClass4";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classElement.name     = "AnyClass4";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass4";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System25";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(75), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(76), Return(E_OK)));
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);

    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
}

/**
 * @brief  :  Test to verify the getElementsBySink function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElementsBySink" function when correct sink name is given as input parameters along with correct other configuration
 *         parameters then check whether getElement function will return list of class element
 *
 * @result : "Pass" when getElementsBySink function will return no gmock error
 */
TEST_F(CAmClassElementTest, getElementsBySinkPositive)
{
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 76;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    // am_SourceClass_s sourceClass;
    sourceClass.sourceClassID = 77;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System26";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // creating the class connection element
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);

}

/**
 * @brief  :  Test to verify the getElementsBySink function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElementsBySink" function when incorrect source name is given as input parameters along with correct other configuration
 *         parameters then check whether getElement function will return list of class element
 *
 * @result : "Pass" when getElementsBySink function will return no gmock error
 */
TEST_F(CAmClassElementTest, getElementsBySinkNegative)
{
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 77;
    sinkClass.name                  = "AnySinkClass6";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 78;
    sourceClass.name                = "AnySourceClass6";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classElement.name     = "AnyClass6";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass6";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System27";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(77), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(78), Return(E_OK)));

    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == mpCAmClassElement)
    {
        ASSERT_THAT(mpCAmClassElement, IsNull()) << " CAmClassElementTest: Not able to create classElement object ";
    }

    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
}

/**
 * @brief  :  Test to verify the getElementByConnection function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElementByConnection" function when route element is given as input parameters along with correct configuration
 *         parameters then check whether getElementByConnection function will return class element or not
 *
 * @result : "Pass" when getElementByConnection function will return Class element without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getElementByConnectionPositive)
{
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 78;
    sinkClass.name                  = "AnySinkClass7";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 79;
    sourceClass.name                = "AnySourceClass7";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classElement.name     = "AnyClass7";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass7";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    // System element

    systemConfiguration.name = "System28";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // creating the class connection element

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(78), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(79), Return(E_OK)));

    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == mpCAmClassElement)
    {
        ASSERT_THAT(mpCAmClassElement, IsNull()) << " CAmClassElementTest: Not able to create classElement object ";
    }

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;

    // Gc Route Information
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    // Main Connection data information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // Createing Class element instance
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(60), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(60, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));

    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_EQ(mpCAmClassElement, CAmClassFactory::getElementByConnection(gcRoute.name));
}

/**
 * @brief  :  Test to verify the getElementByConnection function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElementByConnection" function when route element is given as input parameters along with correct configuration
 *         parameters then check whether getElementByConnection function will return class element or not
 *
 * @result : "Pass" when getElementByConnection function will not return NULL without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getElementByConnectionNegative)
{

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 79;
    sinkClass.name        = "AnySinkClass8";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    // am_SourceClass_s sourceClass;
    sourceClass.sourceClassID = 80;
    sourceClass.name          = "AnySourceClass8";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // source Element belongs to class
    classElement.name     = "AnyClass8";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass8";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System29";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(79), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(80), Return(E_OK)));
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Gc Route Information
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName() + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    // Main Connection data information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);

    // Createing Class element instance
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(61), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(61, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
// EXPECT_EQ(NULL, CAmClassFactory::getElementByConnection(gcRoute.name));
}

/**
 * @brief  :  Test to verify the getElementBySourceClassID function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElementBySourceClassID" function when source class id is given as input parameters along with correct configuration
 *         parameters then check whether getElementBySourceClassID function will return class element or not
 *
 * @result : "Pass" when getElementBySourceClassID function will not return Class element without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getElementBySourceClassIDPositive)
{

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 80;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    // am_SourceClass_s sourceClass;
    sourceClass.sourceClassID = 81;
    sourceClass.name          = "AnySourceClass9";
    sourceClass.listClassProperties.push_back(classPropertyData);

    classElement.name     = "AnyClass9";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass9";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System30";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(80), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(81), Return(E_OK)));
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);

    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
    sourceClassID = mpCAmSourceElement->getClassID();
    EXPECT_EQ(mpCAmClassElement, CAmClassFactory::getElementBySourceClassID(sourceClassID));
}

/**
 * @brief  :  Test to verify the getElementBySourceClassID function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElementBySourceClassID" function when wrong source class id is given as input parameters along with correct configuration
 *         parameters then check whether getElementBySourceClassID function will return class element or Not
 *
 * @result : "Pass" when getElementBySourceClassID function will not return NULL without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getElementBySourceClassIDNegative)
{

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 81;
    sinkClass.name        = "AnySinkClass10";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    // am_SourceClass_s sourceClass;
    sourceClass.sourceClassID = 82;
    sourceClass.name          = "AnySourceClass10";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classElement.name     = "AnyClass10";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass10";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System31";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(81), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(82), Return(E_OK)));

    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == mpCAmClassElement)
    {
        ASSERT_THAT(mpCAmClassElement, IsNull()) << " CAmClassElementTest: Not able to create classElement object ";
    }

    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    am_sourceClass_t sourceClassID = 2;
}

/**
 * @brief  :  Test to verify the getElementBySinkClassID function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElementBySinkClassID" function when sink class id is given as input parameters along with correct configuration
 *         parameters then check whether getElementBySourceClassID function will return class element or not
 *
 * @result : "Pass" when getElementBySinkClassID function will return Class element without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getElementBySinkClassIDPositive)
{

    sink.sinkClassID                = 82;
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 82;
    sinkClass.name        = "AnySinkClass11";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    // am_SourceClass_s sourceClass;
    sourceClass.sourceClassID = 83;
    sourceClass.name          = "AnySourceClass11";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classElement.name     = "AnyClass11";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass11";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System32";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(82), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(83), Return(E_OK)));

    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == mpCAmClassElement)
    {
        ASSERT_THAT(mpCAmClassElement, IsNull()) << " CAmClassElementTest: Not able to create classElement object ";
    }

    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);

    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sink), Return(E_OK)));
    sinkClassID = mpCAmSinkElement->getClassID();
    EXPECT_EQ(mpCAmClassElement, CAmClassFactory::getElementBySinkClassID(sinkClassID));
}

/**
 * @brief  :  Test to verify the getElementBySinkClassID function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getElementBySourceClassID" function when wrong sink class id is given as input parameters along with correct configuration
 *         parameters then check whether getElementBySourceClassID function will return class element or Not
 *
 * @result : "Pass" when getElementBySourceClassID function will not return NULL without any Gmock error message.
 */

TEST_F(CAmClassElementTest, getElementBySinkClassIDNegative)
{

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 83;
    sinkClass.name        = "AnySinkClass12";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    // am_SourceClass_s sourceClass;
    sourceClass.sourceClassID = 84;
    sourceClass.name          = "AnySourceClass12";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classElement.name     = "AnyClass12";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass12";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System33";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(83), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(84), Return(E_OK)));

    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);

    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);

    am_sinkClass_t sinkClassID = 2;
// EXPECT_EQ(NULL, CAmClassFactory::getElementBySinkClassID(sinkClassID));
}
#ifdef VOLUME_RELATED_TEST
/**
 * @brief  :  Test to verify the getListClassLimit function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getListClassLimit" function when setLimitState function is called along with required input parameters like
 *          state limit with limited , volume limit and pattern information along with correct configuration parameters then
 *          check whether getListClassLimit function will return size of map list of volume or not
 *
 * @result : "Pass" when getListClassLimit function will return "E_OK" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getListClassLimitPositive)
{

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;

    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System34";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    gc_LimitState_e limitState = LS_LIMITED;
    // gc_LimitVolume_s limitVolume;
    limitVolume.limitType   = LT_RELATIVE;
    limitVolume.limitVolume = 10;
    uint32_t pattern = 15;
    mpCAmClassElement->setLimitState(limitState, limitVolume, pattern);
    // EXPECT_EQ(E_OK, mpCAmClassElement->getListClassLimits(mapLimits));
}

/**
 * @brief  :  Test to verify the getListClassLimit function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getListClassLimit" function when setLimitState function is called along with required input parameters like
 *          state limit with unlimited , volume limit and pattern information along with correct configuration parameters then
 *          check whether getListClassLimit function will return size of map list of volume or not
 *
 * @result : "Pass" when getListClassLimit function will return "E_OK" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getListClassLimitNegative)
{

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System35";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // Creating Class Element instancce
    // mpCAmClassElement = new CAmClassElement(classElement, mpCAmControlReceive);

    gc_LimitState_e limitState = LS_UNLIMITED;
    limitVolume.limitType   = LT_RELATIVE;
    limitVolume.limitVolume = 10;
    uint32_t pattern = 15;
    mpCAmClassElement->setLimitState(limitState, limitVolume, pattern);
    EXPECT_EQ(E_OK, mpCAmClassElement->getListClassLimits(mapLimits));
}
#endif
/**
 * @brief  :  Test to verify the isPerSinkClassVolumeEnabled function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "isPerSinkClassVolumeEnabled" function when class property properly set along with correct configuration parameters then
 *          check whether isPerSinkClassVolumeEnabled function will true when value of class property is not set to zero.
 *
 * @result : "Pass" when isPerSinkClassVolumeEnabled function will return "True" without any Gmock error message.
 */

// TEST_F(CAmClassElementTest, isPerSinkClassVolumeEnabledPositive)
// {
// classPropertyData.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
// classPropertyData.value         = 1;
// listClassProperties.push_back(classPropertyData);
// classElement.name                = "AnyClass1";
// classElement.type                = C_CAPTURE;
// classElement.priority            = 1;
// classElement.listClassProperties=listClassProperties;
//

// classElement.name     = "AnyClass1";
// classElement.type     = C_CAPTURE;
// classElement.priority = 1;

// classElement.listClassProperties = listClassProperties;
//// SystemProperty creating
// systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
// systemproperty.value = 5;
// listSystemProperties.push_back(systemproperty);
//// System element
////systemConfiguration.name                 = "System1";
////EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
////mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
//// Creating Class Element instancce
////mpCAmClassElement = new CAmClassElement(classElement, mpCAmControlReceive);
////mpCAmClassElement = CAmClassFactory::getElementByConnection()
// mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
// mpCAmClassElement->attach(mpCAmSourceElement);
// mpCAmClassElement->attach(mpCAmSinkElement);

// EXPECT_NO_THROW(mpCAmClassElement->isPerSinkClassVolumeEnabled());
// }

/**
 * @brief  :  Test to verify the isPerSinkClassVolumeEnabled function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "isPerSinkClassVolumeEnabled" function when class property is not set along with correct configuration parameters then
 *          check whether isPerSinkClassVolumeEnabled function will true when value of class property is not set to zero.
 *
 * @result : "Pass" when isPerSinkClassVolumeEnabled function will return "FALSE" without any Gmock error message.
 */

TEST_F(CAmClassElementTest, isPerSinkClassVolumeEnabledNegative)
{

    // topologyElementsource.name   = "AnySource37";

    ////am_ClassProperty_s classProperty;
    classProperty.value              = 10;
    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // topologyElementsink.name   = "AnySink37";

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System37";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    EXPECT_FALSE(mpCAmClassElement->isPerSinkClassVolumeEnabled());

}

/**
 * @brief  :  Test to verify the isElementBelongtoClass function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "isElementBelongtoClass" function when element name and element correct type i.e ET_SINK is set along with correct configuration parameters
 *         then check whether isElementBelongtoClass function will true or not.
 *
 * @result : "Pass" when isElementBelongtoClass function will return "True" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, isElementBelongtoClassPositive)
{
    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);

    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;

    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System38";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    elementType = ET_SINK;
    EXPECT_TRUE(mpCAmClassElement->isElementBelongtoClass(sink.name, elementType));

}

/**
 * @brief  :  Test to verify the isElementBelongtoClass function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "isElementBelongtoClass" function when element name and element incorrect type is set along with correct configuration parameters
 *         then check whether isElementBelongtoClass function will true or not.
 *
 * @result : "Pass" when isElementBelongtoClass function will return "True" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, isElementBelongtoClassNegative)
{

    // topologyElementsource.name   = "AnySource39";

    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);

    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;

    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System39";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    elementType        = ET_UNKNOWN;
    EXPECT_FALSE(mpCAmClassElement->isElementBelongtoClass(sink.name, elementType));

}

/**
 * @brief  :  Test to verify the getLastVolume with classlastMainVolume function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getLastVolume" function when sink name and  main volume is passed as input parameters along with correct configuration parameters
 *         then check whether getLastVolume function will return E_OK or not when  class volume is enabled for the given sink and main volume is set.
 *
 * @result : "Pass" when getLastVolume function will return "E_OK" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getLastVolumePositiveclasslastMainVolume)
{

    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);

    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;

    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System40";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    gc_SinkVolume_s gcSinkVolume;
    gcSinkVolume.sinkName   = sink.name;
    gcSinkVolume.mainVolume = 10;
    listSinkVolume.push_back(gcSinkVolume);
    gc_LastClassVolume_s gcLastClassVolume;
    gcLastClassVolume.className      = classElement.name;
    gcLastClassVolume.listSinkVolume = listSinkVolume;
    mpCAmClassElement->restoreVolume(gcLastClassVolume);
    am_mainVolume_t mainVolume;
    EXPECT_EQ(E_OK, mpCAmClassElement->getLastVolume(sink.name, mainVolume));

}

/**
 * @brief  :  Test to verify the getLastVolume with DefaultMainVolume function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getLastVolume" function when sink name and  main volume is passed as input parameters along with correct configuration parameters
 *         then check whether getLastVolume function will return E_OK or not, when class volume is not set to AM_MUTE i.e. -3000 default value.
 *
 * @result : "Pass" when getLastVolume function will return "E_OK" without any Gmock error message.
 */

TEST_F(CAmClassElementTest, getLastVolumePositiveDefaultMainVolume)
{

    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);
    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System41";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    gc_SinkVolume_s gcSinkVolume;
    gcSinkVolume.sinkName   = sink.name;
    gcSinkVolume.mainVolume = 10;
    gc_LastClassVolume_s gcLastClassVolume;
    gcLastClassVolume.className = classElement.name;
    mpCAmClassElement->restoreVolume(gcLastClassVolume);
    am_mainVolume_t mainVolume;
    EXPECT_EQ(E_OK, mpCAmClassElement->getLastVolume(sink.name, mainVolume));
}

/**
 * @brief  :  Test to verify the getLastVolume with classlastMainVolume function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getLastVolume" function when sink name and  main volume is passed as input parameters along with correct configuration parameters
 *         then check whether getLastVolume function will return E_OK or not when  class property is not set.
 *
 * @result : "Pass" when getLastVolume function will return "E_OK" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getLastVolumeNegativeLastClassMainVolume)
{
    classProperty.classProperty      = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value              = 10;
    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;

    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System42";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // Creating Class Element instancce
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    gc_SinkVolume_s gcSinkVolume;
    gcSinkVolume.sinkName   = sink.name;
    gcSinkVolume.mainVolume = 15;
    listSinkVolume.push_back(gcSinkVolume);
    gc_LastClassVolume_s gcLastClassVolume;
    gcLastClassVolume.className      = classElement.name;
    gcLastClassVolume.listSinkVolume = listSinkVolume;
    mpCAmClassElement->restoreVolume(gcLastClassVolume);
    am_mainVolume_t mainVolume;
    EXPECT_EQ(E_OK, mpCAmClassElement->getLastVolume(sink.name, mainVolume));
}

/**
 * @brief  :  Test to verify the setLastVolume function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "setLastVolume" function when sink name and  main volume is passed as input parameters along with correct configuration parameters
 *         then check whether setLastVolume function will return E_OK or not when sink name along with correct class property, with main volume is set.
 *
 * @result : "Pass" when setLastVolume function will return "E_OK" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, setLastVolumePositive)
{
    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);

    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System43";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    gc_SinkVolume_s gcSinkVolume;
    gcSinkVolume.sinkName   = sink.name;
    gcSinkVolume.mainVolume = 15;
    listSinkVolume.push_back(gcSinkVolume);
    gc_LastClassVolume_s gcLastClassVolume;
    gcLastClassVolume.className      = classElement.name;
    gcLastClassVolume.listSinkVolume = listSinkVolume;

    mpCAmClassElement->restoreVolume(gcLastClassVolume);
    am_mainVolume_t        mainVolume = 10;
    am_MainSoundProperty_s amMainsoundProperty;
    amMainsoundProperty.type  = MSP_SINK_PER_CLASS_VOLUME_TYPE(mpCAmClassElement->getID());
    amMainsoundProperty.value = 10;
    am_CustomMainSoundPropertyType_t type = MSP_SINK_PER_CLASS_VOLUME_TYPE(mpCAmClassElement->getID());
    int16_t                          value;
// EXPECT_EQ(E_OK, mpCAmClassElement->setLastVolume(sink.name, mainVolume));
}

/**
 * @brief  :  Test to verify the setLastVolume function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "setLastVolume" function when sink name and  main volume is passed as input parameters along with correct configuration parameters
 *         then check whether setLastVolume function will return E_OK or not when incorrect sink name which is not available in list of sink elememt along with correct class property
 *
 * @result : "Pass" when setLastVolume function will return "E_OK" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, setLastVolumeNegative)
{
    classProperty.classProperty      = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value              = 10;
    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;

    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System44";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // Creating Class Element instancce
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    gc_SinkVolume_s gcSinkVolume;
    gcSinkVolume.sinkName   = sink.name;
    gcSinkVolume.mainVolume = 15;
    listSinkVolume.push_back(gcSinkVolume);
    gc_LastClassVolume_s gcLastClassVolume;
    gcLastClassVolume.className      = classElement.name;
    gcLastClassVolume.listSinkVolume = listSinkVolume;
    mpCAmClassElement->restoreVolume(gcLastClassVolume);
    am_mainVolume_t mainVolume = 10;
// EXPECT_EQ(E_OK, mpCAmClassElement->setLastVolume("FrontSpeaker", mainVolume));
}

/**
 * @brief : Test to verify the getVolumeString function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getVolumeString" function when class name and sink name with main volume is set along with correct configuration parameters
 *         then check whether getVolumeString function will return excepted the Volume string or not.
 *
 * @result : "Pass" when getVolumeString function will return "E_OK" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getVolumeStringPositive)
{

    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);

    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System45";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    gc_SinkVolume_s gcSinkVolume;
    gcSinkVolume.sinkName   = sink.name;
    gcSinkVolume.mainVolume = 15;
    listSinkVolume.push_back(gcSinkVolume);

    gc_LastClassVolume_s gcLastClassVolume;
    gcLastClassVolume.className      = classElement.name;
    gcLastClassVolume.listSinkVolume = listSinkVolume;
    mpCAmClassElement->restoreVolume(gcLastClassVolume);
    std::string VolumeStr = "{AnyClass1,[AnySink1:15]}";
    EXPECT_EQ(VolumeStr, mpCAmClassElement->getVolumeString());
}

/**
 * @brief : Test to verify the getVolumeString function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getVolumeString" function when class name and sink name with main volume is set along with correct configuration parameters
 *         then check whether getVolumeString function will return excepted the Volume string or not, when compare with wrong set volume string.
 *
 * @result : "Pass" when getVolumeString function will return "E_OK" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getVolumeStringNegative)
{

    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);
    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;

    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System46";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    gc_SinkVolume_s gcSinkVolume;
    gcSinkVolume.sinkName   = sink.name;
    gcSinkVolume.mainVolume = 15;
    gc_LastClassVolume_s gcLastClassVolume;
    gcLastClassVolume.className = classElement.name;
    mpCAmClassElement->restoreVolume(gcLastClassVolume);
    std::string VolumeStr = "";
    EXPECT_EQ(VolumeStr, mpCAmClassElement->getVolumeString());

}

/**
 * @brief : Test to verify the getMainConnectionString function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getMainConnectionString" function when class name and sink name, source name, main connection element details is set
 *          along with correct configuration parameters then check whether getMainConnectionString function will return the main connection string or not.
 *
 * @result : "Pass" when getMainConnectionString function will return main connection string without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getMainConnectionStringPositive)
{
    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);
    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;

    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System47";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    source.name = "AnySource47";
    sink.name   = "AnySink47";
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(46), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(46, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));

    // Main Connection id
    mainConnectionData.mainConnectionID = 1;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // List of Route information
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;

    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutinElement.name       = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);

    state = CS_CONNECTED;
    mpCAmClassElement->update(mpCAmMainConnectionElement, state);
    std::string MainConnecitonStr = "";
    EXPECT_EQ(MainConnecitonStr, mpCAmClassElement->getMainConnectionString());
}

/**
 * @brief : Test to verify the getMainConnectionString function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "getMainConnectionString" function when class name and sink name, source name, main connection element details is set
 *          along with correct configuration parameters then check whether getMainConnectionString function will return the main connection string or not.
 *
 * @result : "Pass" when getMainConnectionString function will not return blank string without any Gmock error message.
 */
TEST_F(CAmClassElementTest, getMainConnectionStringNegative)
{

    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);

    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System48";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(46), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(46, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));

    // Main Connection id
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // List of Route information
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;

    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gc_RoutingElement_s gcRoutinElement;
    gcRoutinElement.name       = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    am_ConnectionState_e state = CS_CONNECTED;
    mpCAmClassElement->update(mpCAmMainConnectionElement, state);
    std::string MainConnecitonStr;
    EXPECT_EQ(MainConnecitonStr, mpCAmClassElement->getMainConnectionString());
}

/**
 * @brief : Test to verify the update function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "update" function when main connection element and connection state is set along with correct configuration parameters
 *         then check whether update function will update the last main connection in the local structure or not.
 *
 * @result : "Pass" when update function will return "E_OK" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, updatePositive)
{

    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);
    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System49";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(46), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(46, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    amRoute.sourceID                = mpCAmSourceElement->getID();
    amRoute.sinkID                  = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutinElement.name       = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    state                      = CS_CONNECTED;
    EXPECT_EQ(E_OK, mpCAmClassElement->update(mpCAmMainConnectionElement, state));
}

/**
 * @brief : Test to verify the update function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "update" function when main connection element and incorrect connection state is set along with correct configuration parameters
 *         then check whether update function will not update the last main connection in the local structure.
 *
 * @result : "Pass" when update function will return "E_NOT_POSSIBLE" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, updateNegative)
{
    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);
    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;

    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System50";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    mainConnectionData.connectionState = CS_CONNECTED;
    routingElement.sourceID            = mpCAmSourceElement->getID();
    routingElement.sinkID              = mpCAmSinkElement->getID();
    routingElement.domainID            = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat    = CF_GENIVI_STEREO;

    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutinElement.name = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
// EXPECT_EQ(E_NOT_POSSIBLE, mpCAmClassElement->update(mpCAmMainConnectionElement, state));
}

/**
 * @brief : Test to verify the update with NotifierElementNull function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source Elements are created with required input structure parameters like name,domainID,class name, registration type,volume etc
 *
 * @test : verify the "update" function when main connection element is null and connection state is set along with correct configuration parameters
 *         then check whether update function will not update the last main connection in the local structure.
 *
 * @result : "Pass" when update function will return "E_NOT_POSSIBLE" without any Gmock error message.
 */
TEST_F(CAmClassElementTest, updateNegativeNotifierElementNull)
{

    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);

    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System51";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    // Creating Class Element instancce
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    CAmMainConnectionElement *mpCAmMainConnectionElement = NULL;
    am_ConnectionState_e      state                      = CS_UNKNOWN;
    // This function is not available
    // EXPECT_EQ(E_NOT_POSSIBLE, mpCAmClassElement->update(mpCAmMainConnectionElement, state));

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
