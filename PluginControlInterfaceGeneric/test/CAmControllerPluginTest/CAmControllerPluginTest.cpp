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
#include <set>
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"
#include "CAmElement.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmDomainElement.h"
#include "CAmClassElement.h"
#include "CAmControllerPlugin.h"
#include "CAmHandleStore.h"
#include "CAmGatewayElement.h"
#include "CAmRouteElement.h"
#include "CAmMainConnectionElement.h"
#include "CAmRouteActionConnect.h"
#include "CAmActionConnect.h"
#include "CAmPolicyReceive.h"
#include "CAmMainConnectionActionConnect.h"
#include "CAmSocketHandler.h"
#include "CAmXmlConfigParser.h"
#include "CAmConfigurationReader.h"
#include "IAmPolicySend.h"
#include "CAmSystemElement.h"
#include "CAmActionContainer.h"
#include "MockIAmControlReceive.h"
#include "MockIAmPolicySend.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "TestObserver.h"
#include "CAmControllerPluginTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmControllerPluginTest
 *@brief : This class is used to test the CAmControllerPlugin class functionality.
 */
CAmControllerPluginTest::CAmControllerPluginTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmSinkElement(nullptr)
    , mpCAmSourceElement(nullptr)
    , mpCAmRouteElement(NULL)
    , mpMockIAmPolicySend(NULL)
    , mpPlugin(NULL)
    , pSocketHandler(NULL)
    , pSystem(nullptr)
    , mpCAmDomainElement(nullptr)
    , testObserver(NULL)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
    , domainInfo("AnyDomain1")
{
#define GC_UNIT_TEST
}

CAmControllerPluginTest::~CAmControllerPluginTest()
{

}

void CAmControllerPluginTest::InitializeCommonStruct()
{
    sink.name                   = "AnySink1";
    source.name                 = "AnySource1";
    type                        = SYP_GLOBAL_LOG_THRESHOLD;
    value                       = 5;
    domain.name                 = "AnyDomain1";
    domain.domainID             = 4;
    state                       = DS_CONTROLLED;
    domainInfo.busname          = "AnyBus";
    domainInfo.nodename         = "AnyNode";
    domainInfo.early            = true;
    domainInfo.complete         = false;
    domainInfo.state            = DS_UNKNOWN;
    domainInfo.registrationType = REG_CONTROLLER;
    listDomains.push_back(domain);
}

void CAmControllerPluginTest::SetUp()
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
        DoAll(SetArgReferee < 1 > (23), Return(E_OK)));
    mpCAmSourceElement = CAmSourceFactory::createElement(sourceInfo,
            mpCAmControlReceive);
    ASSERT_THAT(mpCAmSourceElement, NotNull())
        << " Source Element Is not Created";
    sourceIDList.push_back(mpCAmSourceElement->getID());
    sourceID         = mpCAmSourceElement->getID();
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    listRoutes.push_back(gcRoute);

    mainConnectionID = 0;
    // mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID          = mpCAmSinkElement->getID();
    mainConnectionData.sourceID        = mpCAmSourceElement->getID();
    mainConnectionData.delay           = 0;
    mainConnectionData.connectionState = CS_CONNECTED;
    gcRoutingElement.name              = mpCAmSourceElement->getName() + mpCAmSinkElement->getName();
    gcRoutingElement.sourceID          = mpCAmSourceElement->getID();
    gcRoutingElement.sinkID            = mpCAmSinkElement->getID();
    gcRoutingElement.domainID          = 2;
    gcRoutingElement.connectionFormat  = CF_GENIVI_STEREO;
    mpCAmRouteElement                  = new CAmRouteElement(gcRoutingElement,
            mpCAmControlReceive);
    ASSERT_THAT(mpCAmRouteElement, NotNull()) << " Route Element Is not Created";
    TCLAP::CmdLine *cmd(CAmCommandLineSingleton::instanciateOnce("The team of the AudioManager1 wishes you a nice day!", ' ', "7.5", true));
    mpMockIAmPolicySend = new MockIAmPolicySend();
    ASSERT_THAT(mpMockIAmPolicySend, NotNull()) << "MockIAmPolicySend Is not Created";

    mpPlugin= new CAmControllerPlugin();
    pSocketHandler   = new CAmSocketHandler();
    EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(IsDomainNamePresent(domain), _)).WillOnce(DoAll(SetArgReferee<1>(4), Return(E_OK)));
    mpCAmDomainElement = CAmDomainFactory::createElement(domainInfo, mpCAmControlReceive);
    mpCAmDomainElement->setID(4);
    domainID     = mpCAmDomainElement->getID();
    testObserver = new TestObserver();

}

void CAmControllerPluginTest::TearDown()
{
    CAmSourceFactory::destroyElement();
    CAmSinkFactory::destroyElement();
    CAmSystemFactory::destroyElement();

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }

    CAmDomainFactory::destroyElement();

    if (mpCAmRouteElement != NULL)
    {
        delete (mpCAmRouteElement);
    }

    CAmCommandLineSingleton::deleteInstance();
#undef GC_UNIT_TEST
}

/**
 * @brief : Test to verify the startupController functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "startupController" function when sink,source,class elements are initialized with default configuration data,
 *         and set the behavior of enterSinkClassDB,enterSourceClassDB, enterSystemPropertiesListDB, getSocketHandler functions
 *         to return the E_OK and pass the mpMockControlReceiveInterface to startupController function
 *         then will check whether startupController function will Instantiates timer event,Control Receive, policy send,
 *         policy receive,class element and system element, CAmPersistenceWrapper or not . without any gmock error.
 *
 * @result : "Pass" when startupController function return "E_OK" without any Gmock error message
 */
TEST_F(CAmControllerPluginTest, startupControllerPositive)
{
    sink.name                       = "AnySink1";
    source.name                     = "AnySource1";
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _))
    .Times(3).WillOnce(DoAll(SetArgReferee<1>(72), Return(E_OK))).WillOnce(DoAll(SetArgReferee<1>(72), Return(E_OK))).WillOnce(DoAll(SetArgReferee<1>(72), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).Times(3)
    .WillOnce(DoAll(SetArgReferee<0>(73), Return(E_OK)))
    .WillOnce(DoAll(SetArgReferee<0>(73), Return(E_OK)))
    .WillOnce(DoAll(SetArgReferee<0>(73), Return(E_OK)));

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System1";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    pSystem = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

}

/**
 * @brief : Test to verify the startupController functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "startupController" function when sink,source,class elements are not initialized with default configuration data,
 *         and not set the behavior of enterSinkClassDB,enterSourceClassDB, enterSystemPropertiesListDB, getSocketHandler functions
 *         to return the E_OK and pass the mpMockControlReceiveInterface to startupController function
 *         then will check whether startupController function will Instantiates timer event,Control Receive, policy send,
 *         policy receive,class element and system element, CAmPersistenceWrapper or not . without any gmock error.
 *
 * @result : "Pass" when startupController function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmControllerPluginTest, startupControllerNegative)
{
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_NOT_POSSIBLE)));
    EXPECT_EQ(E_NOT_POSSIBLE, mpPlugin->startupController(mpMockControlReceiveInterface));

}

/**
 * @brief : Test to verify the hookUserConnectionRequest functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserConnectionRequest" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSinkClassDB,enterSourceClassDB,enterSystemPropertiesListDB,
 *         getSocketHandler,startupController,enterMainConnectionDB,getMainConnectionInfoDB functions to return the E_OK
 *         and also initialize the CAmSinkActionSetVolume class with default data.
 *         And pass the sinkid,sourceId,mainconnectionID to hookUserConnectionRequest function
 *         then will check whether hookUserConnectionRequest function will connect the source and sink, and return the main connection id
 *         or not,without any gmock error.
 *
 * @result : "Pass" when hookUserConnectionRequest function return "E_OK" without any Gmock error message
 */
TEST_F(CAmControllerPluginTest, hookUserConnectionRequestPositive)
{
    if (mpMockIAmPolicySend == NULL)
    {
        ASSERT_THAT(mpCAmRouteElement, IsNull()) << " Route Element Is not Created";
    }

    sink.name   = "AnySink1";
    source.name = "AnySource1";
    am_sinkID_t   sinkID   = 0;
    am_sourceID_t sourceID = 0;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).Times(1).WillOnce(DoAll(SetArgReferee<1>(72), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).Times(1).WillOnce(DoAll(SetArgReferee<0>(73), Return(E_OK)));

//// SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System2";

    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
//// main connection ids
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Am route Info
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);

    // GC route Info
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTING;

    mainConnectionData1.mainConnectionID = 0;
    mainConnectionData1.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData1.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData1.delay            = 0;
    mainConnectionData1.connectionState  = CS_CONNECTED;

#if 0
    ActionSetVolume = new CAmSinkActionSetVolume(mpCAmSinkElement);
    mVolumeParam.setParam(25);
    ActionSetVolume->setParam(ACTION_PARAM_VOLUME, &mVolumeParam);
    mLimitTypeParam.setParam(LT_ABSOLUTE);
    ActionSetVolume->setParam(ACTION_PARAM_LIMIT_TYPE, &mLimitTypeParam);
    mLimitVolumeParam.setParam(23);
    ActionSetVolume->setParam(ACTION_PARAM_LIMIT_VOLUME, &mLimitVolumeParam);
    mRampTypeParam.setParam(RAMP_GENIVI_DIRECT);
    ActionSetVolume->setParam(ACTION_PARAM_RAMP_TYPE, &mRampTypeParam);
    mRampTimeParam.setParam(200);
    ActionSetVolume->setParam(ACTION_PARAM_RAMP_TIME, &mRampTimeParam);
#endif
    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    EXPECT_CALL(*mpMockControlReceiveInterface, getRoute(_, mpCAmSourceElement->getID(), mpCAmSinkElement->getID(), _)).Times(2).WillOnce(DoAll(SetArgReferee<3>(listRoutes), Return(E_OK)))
    .WillOnce(DoAll(SetArgReferee<3>(listRoutes), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(45, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    EXPECT_EQ(E_NOT_POSSIBLE, mpPlugin->hookUserConnectionRequest(sourceID, sinkID, mainConnectionID));

}

/**
 * @brief : Test to verify the hookUserConnectionRequest functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserConnectionRequest" function when sink,source,class elements are initialized with
 *         default configuration data and not set the behavior of enterSinkClassDB,enterSourceClassDB,enterSystemPropertiesListDB,
 *         getSocketHandler,startupController,enterMainConnectionDB,getMainConnectionInfoDB functions to return the E_OK
 *         And pass the sinkid,sourceId,mainconnectionID to hookUserConnectionRequest function
 *         then will check whether hookUserConnectionRequest function will connect the source and sink, and return the main connection id
 *         or not,without any gmock error.
 *
 * @result : "Pass" when hookUserConnectionRequest function return "E_NON_EXISTENT" without any Gmock error message
 */
TEST_F(CAmControllerPluginTest, hookUserConnectionRequestNegative)
{
    am_sinkID_t           sinkID           = 100;
    am_sourceID_t         sourceID         = 101;
    am_mainConnectionID_t mainConnectionID = 0;
    EXPECT_EQ(E_NOT_POSSIBLE, mpPlugin->hookUserConnectionRequest(sourceID, sinkID, mainConnectionID));
}

/**
 * @brief : Test to verify the hookUserConnectionRequest functions with RouteListEmpty for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserConnectionRequest" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSinkClassDB,enterSourceClassDB,enterSystemPropertiesListDB,
 *         getSocketHandler,startupController,enterMainConnectionDB,getMainConnectionInfoDB functions to return the E_OK
 *         and also initialize the CAmSinkActionSetVolume class with default data, and initialize the route element with incorrect details
 *         And pass the sinkid,sourceId,mainconnectionID to hookUserConnectionRequest function
 *         then will check whether hookUserConnectionRequest function will connect the source and sink, and return the main connection id
 *         or not,without any gmock error.
 *
 * @result : "Pass" when hookUserConnectionRequest function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmControllerPluginTest, hookUserConnectionRequestNegativeRouteListEmpty)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System2";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // main connection ids
    am_mainConnectionID_t mainConnectionID = 0;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Am route Info
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    EXPECT_CALL(*mpMockControlReceiveInterface, getRoute(_, mpCAmSourceElement->getID(), mpCAmSinkElement->getID(), _)).Times(2).WillOnce(DoAll(SetArgReferee<3>(listRoutes), Return(E_OK)))
        .WillOnce(DoAll(SetArgReferee<3>(listRoutes), Return(E_OK)));
    EXPECT_EQ(E_NOT_POSSIBLE, mpPlugin->hookUserConnectionRequest(sourceID, sinkID, mainConnectionID));
}

/**
 * @brief : Test to verify the hookUserDisconnectionRequest functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserDisconnectionRequest" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController,enterMainConnectionDB,getMainConnectionInfoDB functions to return the E_OK
 *         and also initialize the CAmSinkActionSetVolume class with default data and make connectionState as CS_DISCONNECTING
 *         And pass the mainconnectionID to hookUserDisconnectionRequest function
 *         then will check whether hookUserDisconnectionRequest function will disconnect the source and sink
 *         or not,without any gmock error.
 *
 * @result : "Pass" when hookUserDisconnectionRequest function will not return any Gmock error message
 */
TEST_F(CAmControllerPluginTest, hookUserDisconnectionRequestPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    am_sinkID_t   sinkID   = 0;
    am_sourceID_t sourceID = 0;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System4";
    pSystem                  = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    // main connection ids
    am_mainConnectionID_t mainConnectionID = 0;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Am route Info
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);

    // GC route Info
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTING;

    mainConnectionData1.mainConnectionID = 0;
    mainConnectionData1.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData1.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData1.delay            = 0;
    mainConnectionData1.connectionState  = CS_CONNECTED;

    // Sink Volume Handle
    handle.handleType = H_SETSINKVOLUME;
    handle.handle     = 0;

    // Connect Handle
    handleConnect.handleType = H_CONNECT;
    handleConnect.handle     = 0;
    am_volume_t         LimitVolumeParam = 50;
    am_CustomRampType_t RampTypeParam    = RAMP_GENIVI_DIRECT;
    am_time_t           RampTimeParam    = 200;

#if 0
    ActionSetVolume = new CAmSinkActionSetVolume(mpCAmSinkElement);
    mVolumeParam.setParam(25);
    ActionSetVolume->setParam(ACTION_PARAM_VOLUME, &mVolumeParam);
    mLimitTypeParam.setParam(LT_ABSOLUTE);
    ActionSetVolume->setParam(ACTION_PARAM_LIMIT_TYPE, &mLimitTypeParam);
    mLimitVolumeParam.setParam(23);
    ActionSetVolume->setParam(ACTION_PARAM_LIMIT_VOLUME, &mLimitVolumeParam);
    mRampTypeParam.setParam(RAMP_GENIVI_DIRECT);
    ActionSetVolume->setParam(ACTION_PARAM_RAMP_TYPE, &mRampTypeParam);
    mRampTimeParam.setParam(200);
    ActionSetVolume->setParam(ACTION_PARAM_RAMP_TIME, &mRampTimeParam);
#endif
    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    mainConnectionDataDisconnect.mainConnectionID = mainConnectionID;
    mainConnectionDataDisconnect.sinkID           = mpCAmSinkElement->getID();
    mainConnectionDataDisconnect.sourceID         = mpCAmSourceElement->getID();
    mainConnectionDataDisconnect.delay            = 0;
    mainConnectionDataDisconnect.connectionState  = CS_DISCONNECTING;

    mainConnectionDataDisconnect1.mainConnectionID = mainConnectionID;
    mainConnectionDataDisconnect1.sinkID           = mpCAmSinkElement->getID();
    mainConnectionDataDisconnect1.sourceID         = mpCAmSourceElement->getID();
    mainConnectionDataDisconnect1.delay            = 0;
    mainConnectionDataDisconnect1.connectionState  = CS_DISCONNECTED;

    // source State
    am_SourceState_e sourceState;

    am_Source_s source_out1;
    source_out1.sourceState = SS_ON;

    am_Source_s source_out2;
    source_out2.sourceState = SS_OFF;

    // Sink Volume Handle
    am_Handle_s handleSinkVol;
    handleSinkVol.handleType = H_SETSINKVOLUME;
    handleSinkVol.handle     = 50;

    am_Handle_s handlestate;
    handlestate.handleType = H_SETSOURCESTATE;
    handlestate.handle     = 30;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_NO_THROW(mpPlugin->hookUserDisconnectionRequest(mainConnectionID));

}

/**
 * @brief : Test to verify the hookUserDisconnectionRequest functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserDisconnectionRequest" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController,enterMainConnectionDB,getMainConnectionInfoDB functions to return the E_OK
 *         and also initialize the CAmSinkActionSetVolume class with default data and make connectionState as CS_DISCONNECTING
 *         And pass the incorrect mainconnectionID to hookUserDisconnectionRequest function
 *         then will check whether hookUserDisconnectionRequest function will disconnect the source and sink
 *         or not,without any gmock error.
 *
 * @result : "Pass" when hookUserDisconnectionRequest function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserDisconnectionRequestNegativeMainconnectionId)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System5";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    // main connection ids
    am_mainConnectionID_t mainConnectionID = 0;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Am route Info
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);

    // GC route Info
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTING;

    mainConnectionData1.mainConnectionID = 0;
    mainConnectionData1.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData1.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData1.delay            = 0;
    mainConnectionData1.connectionState  = CS_CONNECTED;

    // Sink Volume Handle
    handle.handleType = H_SETSINKVOLUME;
    handle.handle     = 0;

    // Connect Handle
    handleConnect.handleType = H_CONNECT;
    handleConnect.handle     = 0;
    // volume information

    am_volume_t         LimitVolumeParam = 50;
    am_CustomRampType_t RampTypeParam    = RAMP_GENIVI_DIRECT;
    am_time_t           RampTimeParam    = 200;
#if 0
    ActionSetVolume = new CAmSinkActionSetVolume(mpCAmSinkElement);
    mVolumeParam.setParam(25);
    ActionSetVolume->setParam(ACTION_PARAM_VOLUME, &mVolumeParam);
    mLimitTypeParam.setParam(LT_ABSOLUTE);
    ActionSetVolume->setParam(ACTION_PARAM_LIMIT_TYPE, &mLimitTypeParam);
    mLimitVolumeParam.setParam(23);
    ActionSetVolume->setParam(ACTION_PARAM_LIMIT_VOLUME, &mLimitVolumeParam);
    mRampTypeParam.setParam(RAMP_GENIVI_DIRECT);
    ActionSetVolume->setParam(ACTION_PARAM_RAMP_TYPE, &mRampTypeParam);
    mRampTimeParam.setParam(200);
    ActionSetVolume->setParam(ACTION_PARAM_RAMP_TIME, &mRampTimeParam);
#endif
    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    EXPECT_NO_THROW(mpPlugin->hookUserDisconnectionRequest(mainConnectionID));

}

/**
 * @brief : Test to verify the hookUserDisconnectionRequest functions with MainconnectionlistEmpty for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserDisconnectionRequest" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController,enterMainConnectionDB,getMainConnectionInfoDB functions to return the E_OK
 *         and also initialize the CAmSinkActionSetVolume class with default data and make connectionState as CS_DISCONNECTING
 *         And pass the mainconnectionID to hookUserDisconnectionRequest function but main connection id list is empty
 *         then will check whether hookUserDisconnectionRequest function will disconnect the source and sink
 *         or not,without any gmock error.
 *
 * @result : "Pass" when hookUserDisconnectionRequest function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserDisconnectionRequestNegativeMainconnectionlistEmpty)
{
    // sink element
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    am_sinkID_t   sinkID   = 0;
    am_sourceID_t sourceID = 0;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System6";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // main connection ids
    am_mainConnectionID_t mainConnectionID = 0;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Am route Info
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);
    // GC route Info
    // Route inforrmation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTING;

    mainConnectionData1.mainConnectionID = 0;
    mainConnectionData1.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData1.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData1.delay            = 0;
    mainConnectionData1.connectionState  = CS_CONNECTED;

    // Sink Volume Handle
    handle.handleType = H_SETSINKVOLUME;
    handle.handle     = 0;

    // Connect Handle
    handleConnect.handleType = H_CONNECT;
    handleConnect.handle     = 0;
    // volume information
    CAmActionParam<am_volume_t >         mVolumeParam;
    CAmActionParam<gc_LimitType_e >      mLimitTypeParam;
    CAmActionParam<am_volume_t >         mLimitVolumeParam;
    CAmActionParam<am_time_t >           mRampTimeParam;
    CAmActionParam<am_CustomRampType_t > mRampTypeParam;
#if 0
    am_volume_t         LimitVolumeParam = 50;
    am_CustomRampType_t RampTypeParam    = RAMP_GENIVI_DIRECT;
    am_time_t           RampTimeParam    = 200;
    ActionSetVolume = new CAmSinkActionSetVolume(mpCAmSinkElement);
    mVolumeParam.setParam(25);
    ActionSetVolume->setParam(ACTION_PARAM_VOLUME, &mVolumeParam);

    mLimitTypeParam.setParam(LT_ABSOLUTE);
    ActionSetVolume->setParam(ACTION_PARAM_LIMIT_TYPE, &mLimitTypeParam);

    mLimitVolumeParam.setParam(23);
    ActionSetVolume->setParam(ACTION_PARAM_LIMIT_VOLUME, &mLimitVolumeParam);

    mRampTypeParam.setParam(RAMP_GENIVI_DIRECT);
    ActionSetVolume->setParam(ACTION_PARAM_RAMP_TYPE, &mRampTypeParam);

    mRampTimeParam.setParam(200);
    ActionSetVolume->setParam(ACTION_PARAM_RAMP_TIME, &mRampTimeParam);
#endif
    // creating the class connection element

    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    pMainConnection = nullptr;
    pMainConnection = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(46, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    EXPECT_EQ(E_NOT_POSSIBLE, mpPlugin->hookUserDisconnectionRequest(mainConnectionID));
}

/**
 * @brief : Test to verify the hookUserSetMainSinkSoundProperty functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetMainSinkSoundProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK
 *         And pass the sinkID,am_MainSoundProperty_s struct to hookUserSetMainSinkSoundProperty function but main connection id list is empty
 *         then will check whether hookUserSetMainSinkSoundProperty function will set sound properties, e.g. Equalizer Values
 *         or not, and also check the USER_SET_SINK_MAIN_SOUND_PROPERTY trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetMainSinkSoundProperty function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetMainSinkSoundPropertyPositive)
{
    ASSERT_THAT(mpPlugin, NotNull()) << " CAmControllerPlugin Is not Created";
    ASSERT_THAT(pSocketHandler, NotNull()) << "SocketHandler Is not Created";

    amMainsoundProperty.type  = MSP_GENIVI_TREBLE;
    amMainsoundProperty.value = 15;

    gcMainSoundProperty.type     = MSP_GENIVI_TREBLE;
    gcMainSoundProperty.minValue = 2;
    gcMainSoundProperty.maxValue = 10;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    gcMainSoundProperty.type     = MSP_GENIVI_MID;
    gcMainSoundProperty.minValue = 5;
    gcMainSoundProperty.maxValue = 20;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);

    // sink element
    sink.name                          = "AnySink1";
    sinkInfo.listGCMainSoundProperties = listGCMainSoundProperties;
    source.name                        = "AnySource1";
    am_sinkID_t   sinkID   = 0;
    am_sourceID_t sourceID = 0;

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System7";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    propertyType = MSP_GENIVI_TREBLE;
    EXPECT_NO_THROW(mpPlugin->hookUserSetMainSinkSoundProperty(sinkID, amMainsoundProperty));

}

/**
 * @brief : Test to verify the hookUserSetMainSinkSoundProperty functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetMainSinkSoundProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK
 *         And pass the sinkID,am_MainSoundProperty_s structure to hookUserSetMainSinkSoundProperty function but their is no
 *         saturate the main sound property value then will check whether hookUserSetMainSinkSoundProperty function will
 *         set sound properties, e.g. Equalizer Values or not,
 *         and also check the USER_SET_SINK_MAIN_SOUND_PROPERTY trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetMainSinkSoundProperty function will return "E_UNKNOWN", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetMainSinkSoundPropertyNegativenoSaturateProperty)
{
    amMainsoundProperty.type  = MSP_GENIVI_TREBLE;
    amMainsoundProperty.value = 15;

    gcMainSoundProperty.minValue = 2;
    gcMainSoundProperty.maxValue = 10;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    gcMainSoundProperty.type     = MSP_GENIVI_MID;
    gcMainSoundProperty.minValue = 5;
    gcMainSoundProperty.maxValue = 20;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);

    // sink element
    sink.name                          = "AnySink1";
    sinkInfo.listGCMainSoundProperties = listGCMainSoundProperties;
    source.name                        = "AnySource1";
    am_sinkID_t   sinkID   = 0;
    am_sourceID_t sourceID = 0;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System8";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    EXPECT_EQ(E_UNKNOWN, mpPlugin->hookUserSetMainSinkSoundProperty(sinkID, amMainsoundProperty));

}

/**
 * @brief : Test to verify the hookUserSetMainSinkSoundProperty functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetMainSinkSoundProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK
 *         And pass the sinkID,am_MainSoundProperty_s structure to hookUserSetMainSinkSoundProperty function but
 *         sink id is out of range then will check whether hookUserSetMainSinkSoundProperty function will
 *         set sound properties, e.g. Equalizer Values or not,
 *         and also check the USER_SET_SINK_MAIN_SOUND_PROPERTY trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetMainSinkSoundProperty function will return "E_OUT_OF_RANGE", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetMainSinkSoundPropertyNegativeSinkidOutofRange)
{
    amMainsoundProperty.type  = MSP_GENIVI_TREBLE;
    amMainsoundProperty.value = 15;
    // GC Main Sound property
    gcMainSoundProperty.minValue = 2;
    gcMainSoundProperty.maxValue = 10;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    gcMainSoundProperty.type     = MSP_GENIVI_MID;
    gcMainSoundProperty.minValue = 5;
    gcMainSoundProperty.maxValue = 20;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);

    // sink element
    sink.name                          = "AnySink1";
    sinkInfo.listGCMainSoundProperties = listGCMainSoundProperties;
    source.name                        = "AnySource1";

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System9";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    EXPECT_EQ(E_OUT_OF_RANGE, mpPlugin->hookUserSetMainSinkSoundProperty(125, amMainsoundProperty));

}

/**
 * @brief : Test to verify the hookUserSetMainSourceSoundProperty functions for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetMainSourceSoundProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK
 *         And pass the sourceID,am_MainSoundProperty_s structure to hookUserSetMainSourceSoundProperty function
 *         then will check whether hookUserSetMainSourceSoundProperty function will set sound properties
 *         i,e. sets a user MainSourceSoundProperty,and also check the USER_SET_SOURCE_MAIN_SOUND_PROPERTY trigger
 *         is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetMainSourceSoundProperty function will return "E_OK", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetMainSourceSoundPropertyPositive)
{
    amMainsoundProperty.type  = MSP_GENIVI_TREBLE;
    amMainsoundProperty.value = 15;

    gcMainSoundProperty.type     = MSP_GENIVI_TREBLE;
    gcMainSoundProperty.minValue = 2;
    gcMainSoundProperty.maxValue = 10;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    gcMainSoundProperty.type     = MSP_GENIVI_MID;
    gcMainSoundProperty.minValue = 5;
    gcMainSoundProperty.maxValue = 20;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    sink.name                            = "AnySink1";
    sinkInfo.listGCMainSoundProperties   = listGCMainSoundProperties;
    source.name                          = "AnySource1";
    sourceInfo.listGCMainSoundProperties = listGCMainSoundProperties;
    am_sinkID_t   sinkID   = 0;
    am_sourceID_t sourceID = 0;

// creating the sink elememt
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

// Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

// sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

// SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

// System element
    systemConfiguration.name = "System1";
    if (pSystem != NULL)
    {
        pSystem = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    propertyType = MSP_GENIVI_TREBLE;
    if (mpPlugin!= NULL)
    {
        EXPECT_NE(E_OK, mpPlugin->hookUserSetMainSourceSoundProperty(sourceID, amMainsoundProperty));
    }
}

/**
 * @brief : Test to verify the hookUserSetMainSourceSoundProperty functions with no saturated property for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetMainSourceSoundProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK
 *         And pass the sourceID,am_MainSoundProperty_s structure to hookUserSetMainSourceSoundProperty function
 *         but their is no saturate the main sound property value  then will check whether hookUserSetMainSourceSoundProperty
 *         function will set sound properties i,e. sets a user MainSourceSoundProperty,and also check the
 *         USER_SET_SOURCE_MAIN_SOUND_PROPERTY trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetMainSourceSoundProperty function will return "E_UNKNOWN", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetMainSourceSoundPropertyNegativenoSaturateproperty)
{

    // Am Main Sound property
    amMainsoundProperty.type  = MSP_GENIVI_TREBLE;
    amMainsoundProperty.value = 15;
    // GC Main Sound property
    gcMainSoundProperty.minValue = 2;
    gcMainSoundProperty.maxValue = 10;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    gcMainSoundProperty.type     = MSP_GENIVI_MID;
    gcMainSoundProperty.minValue = 5;
    gcMainSoundProperty.maxValue = 20;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    // sink element
    sink.name                            = "AnySink1";
    sinkInfo.listGCMainSoundProperties   = listGCMainSoundProperties;
    source.name                          = "AnySource1";
    sourceInfo.listGCMainSoundProperties = listGCMainSoundProperties;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System10";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    EXPECT_EQ(E_UNKNOWN, mpPlugin->hookUserSetMainSourceSoundProperty(sourceID, amMainsoundProperty));

}

/**
 * @brief : Test to verify the hookUserSetMainSourceSoundProperty functions with SorceIdOutofRange for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetMainSourceSoundProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK
 *         And pass the sourceID,am_MainSoundProperty_s structure to hookUserSetMainSourceSoundProperty function
 *         but SorceId is set out of range then will check whether hookUserSetMainSourceSoundProperty
 *         function will set sound properties i,e. sets a user MainSourceSoundProperty,and also check the
 *         USER_SET_SOURCE_MAIN_SOUND_PROPERTY trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetMainSourceSoundProperty function will return "E_UNKNOWN", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetMainSourceSoundPropertyNegativeSorceIdOutofRange)
{

    // Am Main Sound property
    amMainsoundProperty.type  = MSP_GENIVI_TREBLE;
    amMainsoundProperty.value = 15;

    // GC Main Sound property
    gcMainSoundProperty.minValue = 2;
    gcMainSoundProperty.maxValue = 10;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    gcMainSoundProperty.type     = MSP_GENIVI_MID;
    gcMainSoundProperty.minValue = 5;
    gcMainSoundProperty.maxValue = 20;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);

    // sink element
    sink.name                            = "AnySink1";
    sinkInfo.listGCMainSoundProperties   = listGCMainSoundProperties;
    source.name                          = "AnySource1";
    sourceInfo.listGCMainSoundProperties = listGCMainSoundProperties;
    // creating the sink elememt
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    // creating the source elememt
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System11";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    EXPECT_EQ(E_OUT_OF_RANGE, mpPlugin->hookUserSetMainSourceSoundProperty(125, amMainsoundProperty));

}

/**
 * @brief : Test to verify the hookUserSetSystemProperty functions for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetSystemProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source & sink element to class element
 *         set the system property type and value details And pass the systemproperty to hookUserSetSystemProperty function
 *         then will check whether hookUserSetSystemProperty function will set system properties
 *         or not, and also check the USER_SET_SYSTEM_PROPERTY trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetSystemProperty function will return "E_OK", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetSystemPropertyPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    EXPECT_NO_THROW(mpPlugin->hookUserSetSystemProperty(systemproperty));
}

/**
 * @brief : Test to verify the hookUserSetSystemProperty functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetSystemProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source & sink element to class element
 *         set the system property type and value details And pass the systemproperty to hookUserSetSystemProperty function
 *         but system name is change then will check whether hookUserSetSystemProperty function will set system properties
 *         or not, and also check the USER_SET_SYSTEM_PROPERTY trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetSystemProperty function will return "E_NOT_POSSIBLE", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetSystemPropertyNegative)
{

    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System14";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    EXPECT_EQ(E_NOT_POSSIBLE, mpPlugin->hookUserSetSystemProperty(systemproperty));

}

/**
 * @brief : Test to verify the hookUserVolumeChange functions for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserVolumeChange" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source & sink element to class element
 *         set the system property type and value details And pass the sink id, main volume to hookUserVolumeChange function
 *         then will check whether hookUserVolumeChange function will change the volume of sink element
 *         or not, and also check the USER_SET_VOLUME trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserVolumeChange function will return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserVolumeChangePositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System15";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // Main Volume
    mainVolume    = 10;
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    handleVlumesink.handleType = H_SETSINKVOLUME;
    handleVlumesink.handle     = 40;

    am_volume_t         LimitVolumeParam = 0;
    am_CustomRampType_t RampTypeParam    = RAMP_GENIVI_DIRECT;
    am_time_t           RampTimeParam    = 200;
    EXPECT_NO_THROW(mpPlugin->hookUserVolumeChange(24, mainVolume));

}

/**
 * @brief : Test to verify the hookUserVolumeChange functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserVolumeChange" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK,
 *         & pass the incorrect sink id, main volume to hookUserVolumeChange function
 *         then will check whether hookUserVolumeChange function will set the volume or not
 *         and also check the USER_SET_VOLUME trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserVolumeChange function will return "E_OUT_OF_RANGE", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserVolumeChangeNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System16";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // Main Volume
    am_mainVolume_t mainvolume = 10;
    EXPECT_EQ(E_OUT_OF_RANGE, mpPlugin->hookUserVolumeChange(125, mainvolume));

}

/**
 * @brief : Test to verify the hookUserVolumeStep functions for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserVolumeStep" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the sink id, increment to hookUserVolumeStep function
 *         then will check whether hookUserVolumeStep function will set the main volume of sink element
 *         or not base on the increment value, and also check the USER_SET_VOLUME trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserVolumeStep function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserVolumeStepPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System17";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    increment                  = 5;
    handleVlumesink.handleType = H_SETSINKVOLUME;
    handleVlumesink.handle     = 40;
    LimitVolumeParam           = -333;
    RampTypeParam              = RAMP_GENIVI_DIRECT;
    RampTimeParam              = 200;
    EXPECT_NO_THROW(mpPlugin->hookUserVolumeStep(23, increment));
}

/**
 * @brief : Test to verify the hookUserVolumeStep functions for negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserVolumeStep" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the incorrect sink id, increment to hookUserVolumeStep function
 *         then will check whether hookUserVolumeStep function will set the main volume of sink element
 *         or not base on the increment value, and also check the USER_SET_VOLUME trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserVolumeStep function will return "E_OUT_OF_RANGE", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserVolumeStepNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value  = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System18";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    increment = 5;
    EXPECT_EQ(E_OUT_OF_RANGE, mpPlugin->hookUserVolumeStep(125, increment));

}

/**
 * @brief : Test to verify the hookUserSetSinkMuteState functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetSinkMuteState" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the sink id, mute state to hookUserSetSinkMuteState function
 *         then will check whether hookUserSetSinkMuteState function will set the mute state of sink element
 *         or not, and also check the USER_SET_SINK_MUTE_STATE trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetSinkMuteState function will return "E_OK", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetSinkMuteStatePositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;

    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System19";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    // Mute state
    am_MuteState_e mutestteate = MS_MUTED;
    // creating the sink elememt
    EXPECT_EQ(E_OK, mpPlugin->hookUserSetSinkMuteState(sinkID, mutestteate));

}

/**
 * @brief : Test to verify the hookUserSetSinkMuteState functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetSinkMuteState" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass incorrect the sink id, mute state to hookUserSetSinkMuteState function
 *         then will check whether hookUserSetSinkMuteState function will set the mute state of sink element
 *         or not, and also check the USER_SET_SINK_MUTE_STATE trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetSinkMuteState function will return "E_OUT_OF_RANGE", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetSinkMuteStateNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System20";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));

    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    am_MuteState_e mutestteate = MS_MUTED;
    EXPECT_EQ(E_OUT_OF_RANGE, mpPlugin->hookUserSetSinkMuteState(125, mutestteate));
}

/**
 * @brief : Test to verify the hookUserSetSinkMuteState functions with MuteStateError for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetSinkMuteState" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the sink id, mute state as "MS_UNKNOWN" to hookUserSetSinkMuteState function
 *         then will check whether hookUserSetSinkMuteState function will set the mute state of sink element
 *         or not, and also check the USER_SET_SINK_MUTE_STATE trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetSinkMuteState function will return "E_OUT_OF_RANGE", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetSinkMuteStateNegativeMuteStateError)
{

    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System21";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    am_MuteState_e mutestteate = MS_UNKNOWN;
    EXPECT_EQ(E_OUT_OF_RANGE, mpPlugin->hookUserSetSinkMuteState(sinkID, mutestteate));

}

/**
 * @brief : Test to verify the hookSystemInterruptStateChange functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "v" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the sourceID, interrupt state to hookSystemInterruptStateChange function
 *         then will check whether hookSystemInterruptStateChange function will set the interrupt state of source element
 *         or not, and also check the SYSTEM_INTERRUPT_STATE_CHANGED trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemInterruptStateChange function will return "E_OK", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemInterruptStateChangePositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System22";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSourceElement);
    // interrupt state
    am_InterruptState_e interruptstate = IS_INTERRUPTED;
    mpPlugin->hookSystemInterruptStateChange(sourceID, interruptstate);

}

/**
 * @brief : Test to verify the hookSystemInterruptStateChange functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "v" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the incorrect sourceID, interrupt state to hookSystemInterruptStateChange
 *         function then will check whether hookSystemInterruptStateChange function will set the interrupt state of source element
 *         or not, and also check the SYSTEM_INTERRUPT_STATE_CHANGED trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemInterruptStateChange function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemInterruptStateChangeNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System23";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSourceElement);
    // interrupt state
    am_InterruptState_e interruptstate = IS_INTERRUPTED;
    mpPlugin->hookSystemInterruptStateChange(125, interruptstate);
}

/**
 * @brief : Test to verify the hookSystemSinkAvailablityStateChange functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemSinkAvailablityStateChange" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the sinkID, availability and availability reason
 *         to hookSystemSinkAvailablityStateChange function then will check whether hookSystemSinkAvailablityStateChange
 *         function will change the state of availability of sink element or not,
 *         and also check the SYSTEM_SINK_AVAILABILITY_CHANGED trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemSinkAvailablityStateChange function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemSinkAvailablityStateChangePositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

// SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System24";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    // sink Availability
    am_Availability_s availability;
    availability.availability       = A_AVAILABLE;
    availability.availabilityReason = AR_GENIVI_TEMPERATURE;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkAvailabilityDB(setAvailable(availability), sinkID)).WillOnce(Return(E_OK));
    mpPlugin->hookSystemSinkAvailablityStateChange(sinkID, availability);

}

/**
 * @brief : Test to verify the hookSystemSinkAvailablityStateChange functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemSinkAvailablityStateChange" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the incorrect sinkID, availability and availability reason
 *         to hookSystemSinkAvailablityStateChange function then will check whether hookSystemSinkAvailablityStateChange
 *         function will change the state of availability of sink element or not,
 *         and also check the SYSTEM_SINK_AVAILABILITY_CHANGED trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemSinkAvailablityStateChange function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemSinkAvailablityStateChangeNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System25";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    // sink Availability
    am_Availability_s availability;
    availability.availability       = A_AVAILABLE;
    availability.availabilityReason = AR_GENIVI_TEMPERATURE;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkAvailabilityDB(setAvailable(availability), 125)).WillOnce(Return(E_OK));
    mpPlugin->hookSystemSinkAvailablityStateChange(125, availability);

}

/**
 * @brief : Test to verify the hookSystemSourceAvailablityStateChange functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemSourceAvailablityStateChange" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the sourceID, availability and availability reason
 *         to hookSystemSinkAvailablityStateChange function then will check whether hookSystemSourceAvailablityStateChange
 *         function will change the state of availability of sink element or not,
 *         and also check the SYSTEM_SINK_AVAILABILITY_CHANGED trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemSourceAvailablityStateChange function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemSourceAvailablityStateChangePositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System26";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSourceElement);
    // sink Availability
    am_Availability_s availability;
    availability.availability       = A_AVAILABLE;
    availability.availabilityReason = AR_GENIVI_TEMPERATURE;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSourceAvailabilityDB(setAvailable(availability), sourceID)).WillOnce(Return(E_OK));
    mpPlugin->hookSystemSourceAvailablityStateChange(sourceID, availability);

}

/**
 * @brief : Test to verify the hookSystemSourceAvailablityStateChange functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemSourceAvailablityStateChange" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the incorrect sourceID, availability and availability reason
 *         to hookSystemSinkAvailablityStateChange function then will check whether hookSystemSourceAvailablityStateChange
 *         function will change the state of availability of source element or not,
 *         and also check the hookSystemSourceAvailablityStateChange trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemSourceAvailablityStateChange function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemSourceAvailablityStateChangeNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System27";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSourceElement);

    // sink Availability
    am_Availability_s availability;
    availability.availability       = A_AVAILABLE;
    availability.availabilityReason = AR_GENIVI_TEMPERATURE;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSourceAvailabilityDB(setAvailable(availability), 127)).WillOnce(Return(E_OK));
    mpPlugin->hookSystemSourceAvailablityStateChange(127, availability);

}

/**
 * @brief : Test to verify the hookSinkNotificationDataChanged functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSinkNotificationDataChanged" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the sinkId, notification payload
 *         to hookSinkNotificationDataChanged function then will check whether hookSinkNotificationDataChanged
 *         function will change the notification pay load for sink element or not,
 *         and also check the SYSTEM_SINK_NOTIFICATION_DATA_CHANGED trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSinkNotificationDataChanged function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSinkNotificationDataChangedPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;

    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System28";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    // Notification payload
    am_NotificationPayload_s payload;
    payload.type  = NT_OVER_TEMPERATURE;
    payload.value = 20;
    EXPECT_CALL(*mpMockControlReceiveInterface, sendMainSinkNotificationPayload(sinkID, setNotificationApyload(payload))).WillOnce(Return());
    mpPlugin->hookSinkNotificationDataChanged(sinkID, payload);
}

/**
 * @brief : Test to verify the hookSinkNotificationDataChanged functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSinkNotificationDataChanged" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the incorrect sinkId, notification pay load
 *         to hookSinkNotificationDataChanged function then will check whether hookSinkNotificationDataChanged
 *         function will change the notification pay load for sink element or not,
 *         and also check the SYSTEM_SINK_NOTIFICATION_DATA_CHANGED trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSinkNotificationDataChanged function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSinkNotificationDataChangedNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System29";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);

    // Notification payload
    am_NotificationPayload_s payload;
    payload.type  = NT_OVER_TEMPERATURE;
    payload.value = 20;
    mpPlugin->hookSinkNotificationDataChanged(128, payload);

}

/**
 * @brief : Test to verify the hookSinkNotificationDataChanged functions for source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSinkNotificationDataChanged" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the sourceId, notification pay load
 *         to hookSinkNotificationDataChanged function then will check whether hookSinkNotificationDataChanged
 *         function will change the notification pay load for source element or not,
 *         and also check the SYSTEM_SOURCE_NOTIFICATION_DATA_CHANGED trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSinkNotificationDataChanged function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSourceNotificationDataChangedPositive)
{

    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value  = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System30";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSourceElement);

    // Notification payload
    am_NotificationPayload_s payload;
    payload.type  = NT_OVER_TEMPERATURE;
    payload.value = 20;
    EXPECT_CALL(*mpMockControlReceiveInterface, sendMainSourceNotificationPayload(sourceID, setNotificationApyload(payload))).WillOnce(Return());
    mpPlugin->hookSourceNotificationDataChanged(sourceID, payload);
}

/**
 * @brief : Test to verify the hookSinkNotificationDataChanged functions for source for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSinkNotificationDataChanged" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the incorrect sourceId, notification pay load
 *         to hookSinkNotificationDataChanged function then will check whether hookSinkNotificationDataChanged
 *         function will change the notification pay load for source element or not,
 *         and also check the SYSTEM_SOURCE_NOTIFICATION_DATA_CHANGED trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSinkNotificationDataChanged function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSourceNotificationDataChangedNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System31";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSourceElement);
    // Notification payload
    am_NotificationPayload_s payload;
    payload.type  = NT_OVER_TEMPERATURE;
    payload.value = 20;
    EXPECT_NO_THROW(mpPlugin->hookSourceNotificationDataChanged(128, payload));

}

/**
 * @brief : Test to verify the hookUserSetMainSinkNotificationConfiguration functions for sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetMainSinkNotificationConfiguration" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the sinkId, am_NotificationConfiguration_s structure details
 *         like type,state,value to hookUserSetMainSinkNotificationConfiguration function then will check whether
 *          hookUserSetMainSinkNotificationConfiguration function will set the main sink notification configuration or not,
 *         and also check the USER_SET_SINK_MAIN_NOTIFICATION_CONFIGURATION trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetMainSinkNotificationConfiguration function will return "E_OK",without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetMainSinkNotificationConfigurationPositive)
{

    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System32";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    // Notification Configuration
    notificationConfiguration.type      = NT_OVER_TEMPERATURE;
    notificationConfiguration.status    = NS_PERIODIC;
    notificationConfiguration.parameter = 20;
    EXPECT_EQ(E_OK, mpPlugin->hookUserSetMainSinkNotificationConfiguration(sinkID, notificationConfiguration));

}

/**
 * @brief : Test to verify the hookUserSetMainSinkNotificationConfiguration functions for sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetMainSinkNotificationConfiguration" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the incorect sinkId, am_NotificationConfiguration_s structure details
 *         like type,state,value to hookUserSetMainSinkNotificationConfiguration function then will check whether
 *          hookUserSetMainSinkNotificationConfiguration function will set the main sink notification configuration or not,
 *         and also check the USER_SET_SINK_MAIN_NOTIFICATION_CONFIGURATION trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetMainSinkNotificationConfiguration function will return "E_OK",without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetMainSinkNotificationConfigurationNegative)
{

    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System33";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    // Notification Configuration
    notificationConfiguration.type      = NT_OVER_TEMPERATURE;
    notificationConfiguration.status    = NS_PERIODIC;
    notificationConfiguration.parameter = 20;
    EXPECT_EQ(E_OUT_OF_RANGE, mpPlugin->hookUserSetMainSinkNotificationConfiguration(128, notificationConfiguration));

}

/**
 * @brief : Test to verify the hookUserSetMainSourceNotificationConfiguration functions for source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetMainSourceNotificationConfiguration" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the sinkId, am_NotificationConfiguration_s structure details
 *         like type,state,value to hookUserSetMainSourceNotificationConfiguration function then will check whether
 *          hookUserSetMainSourceNotificationConfiguration function will set the main source notification configuration or not,
 *         and also check the USER_SET_SOURCE_MAIN_NOTIFICATION_CONFIGURATION trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetMainSourceNotificationConfiguration function will return "E_OK",without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetMainSourceNotificationConfigurationPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System34";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSourceElement);

    // Notification Configuration
    notificationConfiguration.type      = NT_OVER_TEMPERATURE;
    notificationConfiguration.status    = NS_PERIODIC;
    notificationConfiguration.parameter = 20;
    EXPECT_EQ(E_OK, mpPlugin->hookUserSetMainSourceNotificationConfiguration(sourceID, notificationConfiguration));

}

/**
 * @brief : Test to verify the hookUserSetMainSourceNotificationConfiguration functions for source for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookUserSetMainSourceNotificationConfiguration" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the incorrect sinkId, am_NotificationConfiguration_s structure details
 *         like type,state,parameter to hookUserSetMainSourceNotificationConfiguration function then will check whether
 *          hookUserSetMainSourceNotificationConfiguration function will set the main source notification configuration or not,
 *         and also check the USER_SET_SOURCE_MAIN_NOTIFICATION_CONFIGURATION trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookUserSetMainSourceNotificationConfiguration function will return "E_OUT_OF_RANGE",without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookUserSetMainSourceNotificationConfigurationNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System35";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSourceElement);

    // Notification Configuration
    notificationConfiguration.type      = NT_OVER_TEMPERATURE;
    notificationConfiguration.status    = NS_PERIODIC;
    notificationConfiguration.parameter = 20;
    EXPECT_EQ(E_OUT_OF_RANGE, mpPlugin->hookUserSetMainSourceNotificationConfiguration(128, notificationConfiguration));

}

/**
 * @brief : Test to verify the hookSystemRegisterSink functions for sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemRegisterSink" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the sinkId, am_Sink_s structure details
 *         to hookSystemRegisterSink function then will check whether hookSystemRegisterSink will register the system sink element
 *         or not, and also check the SYSTEM_REGISTER_SINK trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemRegisterSink function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemRegisterSinkPositive)
{

    source.name     = "AnySource1";
    sink.name       = "AnySink1";
    domain.name     = "VirtDSP";
    domain.domainID = 4;
    gc_Domain_s domainInfo;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System55";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    sinkID1 = sink.sinkID;
    if (mpPlugin!= NULL)
    {
        EXPECT_NO_THROW(mpPlugin->hookSystemRegisterSink(sink, sinkID));
    }
}

/**
 * @brief : Test to verify the hookSystemRegisterSink functions for sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemRegisterSink" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the incorrect sinkId, am_Sink_s structure details
 *         to hookSystemRegisterSink function then will check whether hookSystemRegisterSink will register the system sink element
 *         or not, and also check the SYSTEM_REGISTER_SINK trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemRegisterSink function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemRegisterSinkNegative)
{
    ////am_Domain_s domain;
    domain.name     = "AMP";
    domain.domainID = 4;
    sink.name       = "AnySink1";
    source.name     = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System37";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    if (mpPlugin!= NULL)
    {
        EXPECT_NO_THROW(mpPlugin->hookSystemRegisterSink(sink, sink.sinkID));
    }
}

/**
 * @brief : Test to verify the hookSystemDeregisterSink functions for sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemDeregisterSink" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the sinkId to hookSystemDeregisterSink function
 *         then will check whether hookSystemDeregisterSink will de-register the system sink element
 *         or not, and also check the SYSTEM_DEREGISTER_SINK trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemDeregisterSink function will not throw any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemDeregisterSinkPositive)
{
    source.name = "AnySource1";
    sink.name   = "AnySink1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;

    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "SystemProp";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    // pSystem = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
// EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
// EXPECT_EQ(E_OK, pSystem->setSystemProperty(type, value));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    sinkID1 = sink.sinkID;
    EXPECT_NO_THROW(mpPlugin->hookSystemDeregisterSink(sinkID1));

}

/**
 * @brief : Test to verify the hookSystemDeregisterSink functions for sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemDeregisterSink" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink element to class element
 *         set the system property type and value details And pass the different sinkId to hookSystemDeregisterSink function
 *         then will check whether hookSystemDeregisterSink will de-register the system sink element
 *         or not, and also check the SYSTEM_DEREGISTER_SINK trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemDeregisterSink function will not throw any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemDeregisterSinkNegative)
{

    source.name = "AnySource1";
    sink.name   = "AnySink1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System39";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    sinkID1 = sink.sinkID;
    EXPECT_NO_THROW(mpPlugin->hookSystemDeregisterSink(sinkID1));
}

/**
 * @brief : Test to verify the hookSystemRegisterSource functions for source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemRegisterSource" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the sourceId, am_Source_s structure details
 *         to hookSystemRegisterSource function then will check whether hookSystemRegisterSource will register the system source element
 *         or not, and also check the SYSTEM_REGISTER_SOURCE trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemRegisterSource function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemRegisterSourcePositive)
{
    domain.name     = "Applications";
    domain.domainID = 4;
    // gc_Domain_s domainInfo;
    sink.name       = "AnySink1";
    source.name     = "AnySource1";
    source.sourceID = 23;
    source.domainID = 4;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
// Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value  = 5;
    // System element
    systemConfiguration.name = "System84";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    if (sourceID != 0)
    {
        EXPECT_EQ(E_OK, mpPlugin->hookSystemRegisterSource(source, source.sourceID));
    }
}

/**
 * @brief : Test to verify the hookSystemRegisterSource functions for source for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemRegisterSource" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the incorrect sourceId, am_Source_s structure details
 *         to hookSystemRegisterSource function then will check whether hookSystemRegisterSource will register the system source element
 *         or not, and also check the SYSTEM_REGISTER_SOURCE trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemRegisterSource function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemRegisterSourceNegative)
{
    domain.name     = "Applications";
    domain.domainID = 4;

    sink.name   = "AnySink1";
    source.name = "AnySource1";

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    systemproperty.type   = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value  = 5;
    // System element
    systemConfiguration.name = "System84";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    EXPECT_NO_THROW(mpPlugin->hookSystemRegisterSource(source, source.sourceID));
}

/**
 * @brief : Test to verify the hookSystemDeregisterSource functions for source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemDeregisterSource" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the sourceId to hookSystemDeregisterSource function
 *         then will check whether hookSystemDeregisterSource will de-register the system source element
 *         or not, and also check the SYSTEM_DEREGISTER_SOURCE trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemDeregisterSource function will not throw any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemDeregisterSourcePositive)
{

    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System40";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSourceElement);
    sourceID1        = source.sourceID;
    source1.name     = "Media";
    source1.sourceID = 127;
    source1.domainID = 4;
    EXPECT_NO_THROW(mpPlugin->hookSystemDeregisterSource(source1.sourceID));
}

/**
 * @brief : Test to verify the hookSystemDeregisterSource functions for source for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemDeregisterSource" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the source element to class element
 *         set the system property type and value details And pass the incorrect sourceId to hookSystemDeregisterSource function
 *         then will check whether hookSystemDeregisterSource will de-register the system source element
 *         or not, and also check the SYSTEM_DEREGISTER_SOURCE trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemDeregisterSource function will not throw any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemDeregisterSourceNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    // Source sink and main connection ids
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System41";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSourceElement);
    sourceID1 = 25;
    EXPECT_EQ(E_NOT_POSSIBLE, mpPlugin->hookSystemDeregisterSource(sourceID1));

}

/**
 * @brief : Test to verify the hookSystemRegisterDomain functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemRegisterDomain" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getListDomains
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink,source element to class element
 *         set the system property type and value details And pass the domainId, am_Domain_s structure details
 *         to hookSystemRegisterSource function then will check whether hookSystemRegisterDomain will register the system domain element
 *         or not, and also check the SYSTEM_REGISTER_DOMAIN trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemRegisterDomain function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemRegisterDomainPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System42";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    domain.name     = "AMP";
    domain.domainID = 7;
    listDomains.push_back(domain);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    EXPECT_NO_THROW(mpPlugin->hookSystemRegisterDomain(domain, domain.domainID));

}

/**
 * @brief : Test to verify the hookSystemRegisterDomain functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemRegisterDomain" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getListDomains
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink,source element to class element
 *         set the system property type and value details And pass the incorrect domainId, am_Domain_s structure details
 *         to hookSystemRegisterSource function then will check whether hookSystemRegisterDomain will register the system domain element
 *         or not, and also check the SYSTEM_REGISTER_DOMAIN trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemRegisterDomain function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemRegisterDomainNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System43";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Domain element
    domain.name     = "PulseAudio1";
    domain.domainID = 2;
    domainID        = 125;
    EXPECT_EQ(E_DATABASE_ERROR, mpPlugin->hookSystemRegisterDomain(domain, domainID));
}

/**
 * @brief : Test to verify the hookSystemDomainRegistrationComplete functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemDomainRegistrationComplete" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getListDomains
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink,source element to class element
 *         set the system property type and value details And pass the domainId details to hookSystemDomainRegistrationComplete
 *         function then will check whether hookSystemDomainRegistrationComplete will register the system all domain element
 *         or not, and also check the SYSTEM_DOMAIN_REGISTRATION_COMPLETE trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemDomainRegistrationComplete function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemDomainRegistrationCompletePositiveAllDomain)
{

    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System44";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // mock domain data-base
    map<am_domainID_t, am_Domain_s> mockDomainDB;
    auto mockGetListDomains = [&](vector<am_Domain_s>& data)
    {
        for (auto &dom : mockDomainDB) { data.push_back(dom.second); }
        return E_OK;
    };
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_))
        .WillRepeatedly(Invoke(mockGetListDomains));
    auto mockEnterDomainDB = [&](const am_Domain_s& data, am_domainID_t &domainID)
    {
        mockDomainDB[data.domainID] = data; domainID = data.domainID;
        return E_OK;
    };
    EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(_, _))
        .WillRepeatedly(Invoke(mockEnterDomainDB));

    gc_Domain_s domain, domain1;
    domain.name     = "VirtDSP";
    domain.domainID = 3;
    domain1.name     = "Applications";
    domain1.domainID = 4;
    CAmDomainFactory::createElement(domain, mpCAmControlReceive);
    CAmDomainFactory::createElement(domain1, mpCAmControlReceive);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(IsDomainNamePresent(domain1), _))
        .WillOnce(Invoke(mockEnterDomainDB));
    mpPlugin->hookSystemDomainRegistrationComplete(domain.domainID);
    EXPECT_TRUE(mockDomainDB[3].complete);

    CAmDomainFactory::destroyElement();
}

/**
 * @brief : Test to verify the hookSystemDomainRegistrationComplete functions with not all domain for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemDomainRegistrationComplete" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getListDomains
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink,source element to class element
 *         set the system property type and value details And pass the correct domainId details to hookSystemDomainRegistrationComplete
 *         function but domain list is not complete then will check whether hookSystemDomainRegistrationComplete will
 *         register the system all domain element or not,
 *         and also check the SYSTEM_DOMAIN_REGISTRATION_COMPLETE trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemDomainRegistrationComplete function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemDomainRegistrationCompletePositiveNotAllDomain)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System45";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Domain element element
    gc_Domain_s domain;
    domain.name     = "VirtDSP";
    domain.domainID = 3;
    // List of Domains
    std::vector<am_Domain_s >listDomains;
    listDomains.push_back(domain);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));

    EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(IsDomainNamePresent(domain), _))
        .WillOnce(DoAll(SetArgReferee<1>(3), Return(E_OK)));
    CAmDomainFactory::createElement(domain, mpCAmControlReceive);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(IsDomainNamePresent(domain), _))
        .WillOnce(DoAll(SetArgReferee<1>(3), Return(E_OK)));
    mpPlugin->hookSystemDomainRegistrationComplete(domain.domainID);

    CAmDomainFactory::destroyElement();
}

/**
 * @brief : Test to verify the hookSystemDomainRegistrationComplete functions with incorrect domain id for negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemDomainRegistrationComplete" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getListDomains
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink,source element to class element
 *         set the system property type and value details And pass the incorrect domainId details to hookSystemDomainRegistrationComplete
 *         function but domain list is incorrect then will check whether hookSystemDomainRegistrationComplete will
 *         register the system all domain element or not,
 *         and also check the SYSTEM_DOMAIN_REGISTRATION_COMPLETE trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemDomainRegistrationComplete function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemDomainRegistrationCompleteNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System46";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Domain element element
    domain.name     = "VirtDSP";
    domain.domainID = 4;
    // List of Domains
    std::vector<am_Domain_s >listDomains;
    listDomains.push_back(domain);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    mpPlugin->hookSystemDomainRegistrationComplete(125);

}

/**
 * @brief : Test to verify the hookSystemDeregisterDomain functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemDeregisterDomain" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink,source element to class element
 *         set the system property type and value details And pass the domain id to hookSystemDeregisterDomain function
 *         then will check whether hookSystemDeregisterDomain will de-register the system domain element
 *         or not, and also check the SYSTEM_DEREGISTER_DOMAIN trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemDeregisterDomain function will not throw any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemDeregisterDomainPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System47";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
   // EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    // mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Domain element element
    domain.name     = "DSP";
    domain.domainID = 5;
    // List of Domains
    listDomains.push_back(domain);
    domainID = domain.domainID;
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    ASSERT_NO_THROW(mpPlugin->hookSystemRegisterDomain(domain, domain.domainID));

    gatewayID = 0;
    listGatewaysIDs.push_back(gatewayID);
    sinkID1 = sink1.sinkID;
    std::vector<am_sinkID_t > listSinkIDs;
    listSinkIDs.push_back(sinkID1);
    sourceID1 = source1.sourceID;
    std::vector<am_sourceID_t > listSourceIDs;
    listSourceIDs.push_back(sourceID1);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListGatewaysOfDomain(domainID, _)).WillRepeatedly(DoAll(SetArgReferee<1>(listGatewaysIDs), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSinksOfDomain(domainID, _)).WillRepeatedly(DoAll(SetArgReferee<1>(listSinkIDs), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSourcesOfDomain(domainID, _)).WillRepeatedly(DoAll(SetArgReferee<1>(listSourceIDs), Return(E_OK)));
    ASSERT_NO_THROW(mpPlugin->hookSystemDeregisterDomain(domain.domainID));

}

/**
 * @brief : Test to verify the hookSystemDeregisterDomain functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemDeregisterDomain" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController functions to return the E_OK, and attach the sink,source element to class element
 *         set the system property type and value details And pass the incorrect domain id to hookSystemDeregisterDomain function
 *         then will check whether hookSystemDeregisterDomain will de-register the system domain element
 *         or not, and also check the SYSTEM_DEREGISTER_DOMAIN trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemDeregisterDomain function will return "E_UNKNOWN", without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemDeregisterDomainNegative)
{
    source.name = "AnySource1";
    sink.name   = "AnySink1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System48";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // Domain element element
    domain.name     = "PulseAudio";
    domain.domainID = 8;
    // List of Domains
    listDomains.push_back(domain);
    domainID = domain.domainID;
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    ASSERT_NO_THROW(mpPlugin->hookSystemRegisterDomain(domain, domain.domainID));
    gatewayID = 0;
    listGatewaysIDs.push_back(gatewayID);
    am_sinkID_t               sinkID1 = 27;
    std::vector<am_sinkID_t > listSinkIDs;
    listSinkIDs.push_back(sinkID1);
    am_sourceID_t               sourceID1 = 27;
    std::vector<am_sourceID_t > listSourceIDs;
    listSourceIDs.push_back(sourceID1);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    EXPECT_EQ(E_NON_EXISTENT, mpPlugin->hookSystemDeregisterDomain(125));
    EXPECT_EQ(E_UNKNOWN, mpPlugin->hookSystemDeregisterDomain(125));

}

/**
 * @brief : Test to verify the hookSystemUpdateSink functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemUpdateSink" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         getSinkInfoDB,changeSinkDB functions to return the E_OK, and attach the sink,source element to class element
 *         set the system property type and value details And pass the sinkClassID, listSoundProperties, listConnectionFormats,
 *         listMainSoundProperties to hookSystemUpdateSink function
 *         then will check whether hookSystemUpdateSink will update the sink details along with sound property,main sound property in db
 *         or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemUpdateSink function will return "E_OK", without the any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemUpdateSinkPositive)
{
    // Sound properties
    am_SoundProperty_s soundProperties;
    soundProperties.type  = SP_GENIVI_TREBLE;
    soundProperties.value = 10;
    std::vector<am_SoundProperty_s >listSoundProperties;
    listSoundProperties.push_back(soundProperties);
    // connection Format
    am_CustomConnectionFormat_t               connectionformat = CF_GENIVI_STEREO;
    std::vector<am_CustomConnectionFormat_t > listConnectionFormats;
    listConnectionFormats.push_back(connectionformat);
    // Main sound Properties
    am_MainSoundProperty_s mainsoundProperties;
    mainsoundProperties.type  = MSP_GENIVI_MID;
    mainsoundProperties.value = 10;
    std::vector<am_MainSoundProperty_s > listMainSoundProperties;
    listMainSoundProperties.push_back(mainsoundProperties);

    // GC Main Sound property
    std::vector<gc_MainSoundProperty_s > listGCMainSoundProperties;
    gc_MainSoundProperty_s               gcMainSoundProperty;
    gcMainSoundProperty.type     = MSP_GENIVI_TREBLE;
    gcMainSoundProperty.minValue = 2;
    gcMainSoundProperty.maxValue = 10;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    gcMainSoundProperty.type     = MSP_GENIVI_MID;
    gcMainSoundProperty.minValue = 5;
    gcMainSoundProperty.maxValue = 20;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);

    // Main soundProperty type and value
    am_CustomMainSoundPropertyType_t mainpropertyType       = MSP_GENIVI_MID;
    int16_t                          mainsoundPropertyValue = 10;
    // GcSoundproperty
    std::vector<gc_SoundProperty_s > listGCSoundProperties;
    gc_SoundProperty_s               gcSoundproperty;
    gcSoundproperty.type     = MSP_GENIVI_TREBLE;
    gcSoundproperty.minValue = 2;
    gcSoundproperty.maxValue = 10;
    listGCSoundProperties.push_back(gcSoundproperty);
    gcSoundproperty.type     = MSP_GENIVI_MID;
    gcSoundproperty.minValue = 5;
    gcSoundproperty.maxValue = 20;
    listGCSoundProperties.push_back(gcSoundproperty);

    // Soundproperty type and value
    am_CustomSoundPropertyType_t propertyType       = SP_GENIVI_TREBLE;
    int16_t                      soundPropertyValue = 15;
    sink.name                          = "AnySink1";
    sink.sinkClassID                   = 77;
    sink.listSoundProperties           = listSoundProperties;
    sink.listConnectionFormats         = listConnectionFormats;
    sink.listMainSoundProperties       = listMainSoundProperties;
    sinkInfo.listGCMainSoundProperties = listGCMainSoundProperties;
    sinkInfo.listGCSoundProperties     = listGCSoundProperties;
    source.name                        = "AnySource1";
    // sink.name = "AnySink1";

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System49";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    mpCAmSinkElement->saturateSoundPropertyRange(propertyType, soundPropertyValue);
    mpCAmSinkElement->saturateMainSoundPropertyRange(mainpropertyType, mainsoundPropertyValue);
    EXPECT_EQ(E_OK, mpPlugin->hookSystemUpdateSink(sinkID, sink.sinkClassID, listSoundProperties, listConnectionFormats, listMainSoundProperties));

}

/**
 * @brief : Test to verify the hookSystemUpdateSource functions for source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemUpdateSource" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         getSinkInfoDB,changeSinkDB functions to return the E_OK, and attach the sink,source element to class element
 *         set the system property type and value details And pass the sourceID, sourceClassID, listSoundProperties,
 *          listConnectionFormats, listMainSoundProperties to hookSystemUpdateSource function
 *         then will check whether hookSystemUpdateSource will update the source details along with sound property,main sound property in db
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSystemUpdateSource function will return "E_OK", without the any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemUpdateSourcePositive)
{

    // Sound properties
    am_SoundProperty_s soundProperties;
    soundProperties.type  = SP_GENIVI_TREBLE;
    soundProperties.value = 10;
    std::vector<am_SoundProperty_s >listSoundProperties;
    listSoundProperties.push_back(soundProperties);
    // connection Format
    am_CustomConnectionFormat_t connectionformat = CF_GENIVI_STEREO;

    std::vector<am_CustomConnectionFormat_t > listConnectionFormats;
    listConnectionFormats.push_back(connectionformat);

    // Main sound Properties
    am_MainSoundProperty_s mainsoundProperties;
    mainsoundProperties.type  = MSP_GENIVI_MID;
    mainsoundProperties.value = 10;

    std::vector<am_MainSoundProperty_s > listMainSoundProperties;
    listMainSoundProperties.push_back(mainsoundProperties);

    // GC Main Sound property
    std::vector<gc_MainSoundProperty_s > listGCMainSoundProperties;
    gc_MainSoundProperty_s               gcMainSoundProperty;
    gcMainSoundProperty.type     = MSP_GENIVI_TREBLE;
    gcMainSoundProperty.minValue = 2;
    gcMainSoundProperty.maxValue = 10;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    gcMainSoundProperty.type     = MSP_GENIVI_MID;
    gcMainSoundProperty.minValue = 5;
    gcMainSoundProperty.maxValue = 20;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);

    // Main soundProperty type and value
    am_CustomMainSoundPropertyType_t mainpropertyType       = MSP_GENIVI_MID;
    int16_t                          mainsoundPropertyValue = 10;

    // GcSoundproperty
    std::vector<gc_SoundProperty_s > listGCSoundProperties;
    gc_SoundProperty_s               gcSoundproperty;
    gcSoundproperty.type     = MSP_GENIVI_TREBLE;
    gcSoundproperty.minValue = 2;
    gcSoundproperty.maxValue = 10;
    listGCSoundProperties.push_back(gcSoundproperty);
    gcSoundproperty.type     = MSP_GENIVI_MID;
    gcSoundproperty.minValue = 5;
    gcSoundproperty.maxValue = 20;
    listGCSoundProperties.push_back(gcSoundproperty);

    // Soundproperty type and value
    am_CustomSoundPropertyType_t propertyType       = SP_GENIVI_TREBLE;
    int16_t                      soundPropertyValue = 15;
    // sink element
    sink.name                      = "AnySink1";
    source.name                    = "AnySource1";
    source.sourceClassID           = 77;
    source.listSoundProperties     = listSoundProperties;
    source.listConnectionFormats   = listConnectionFormats;
    source.listMainSoundProperties = listMainSoundProperties;

    // gc_Source_s sourceInfo;
    sourceInfo.listGCMainSoundProperties = listGCMainSoundProperties;
    sourceInfo.listGCSoundProperties     = listGCSoundProperties;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

// SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System50";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    mpCAmSourceElement->saturateSoundPropertyRange(propertyType, soundPropertyValue);
    mpCAmSourceElement->saturateMainSoundPropertyRange(mainpropertyType, mainsoundPropertyValue);
    EXPECT_EQ(E_OK, mpPlugin->hookSystemUpdateSource(sourceID, source.sourceClassID, listSoundProperties, listConnectionFormats, listMainSoundProperties));
}

/**
 * @brief : Test to verify the hookSystemRegisterGateway functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemRegisterGateway" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         enterGatewayDB functions to return the E_OK, and attach the sink,source element to class element
 *         set the system property type and value details And pass the am_Gateway_s structure details,gatewayID to
 *         hookSystemRegisterGateway function then will check whether hookSystemRegisterGateway function will
 *         register the system  gateway element or not, and also check the SYSTEM_REGISTER_GATEWAY trigger is generated
 *         or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemRegisterGateway function will return "E_OK", without the any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemRegisterGatewayPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System51";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
   // mpCAmControlReceive->changeSystemPropertyDB(systemproperty);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // GatewayElement
    gateway.name                  = "Gateway0";
    gateway.gatewayID             = 1;
    gatewayInfo.sinkName          = "Gateway0";     // Gateway Sinkname
    gatewayInfo.sourceName        = "Gateway0";     // Gateway Sourcename
    gatewayInfo.controlDomainName = "Applications"; // Gateway controlling Domainname
    EXPECT_CALL(*mpMockControlReceiveInterface, enterGatewayDB(IsGatewayNamePresent(gateway), _)).WillOnce(DoAll(SetArgReferee<1>(1), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->hookSystemRegisterGateway(gateway, gateway.gatewayID));

}

/**
 * @brief : Test to verify the hookSystemRegisterGateway functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemRegisterGateway" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         enterGatewayDB functions to return the E_OK, and attach the sink,source element to class element
 *         set the system property type and value details And pass incorrect the am_Gateway_s structure details,gatewayID to
 *         hookSystemRegisterGateway function then will check whether hookSystemRegisterGateway function will
 *         register the system  gateway element or not, and also check the SYSTEM_REGISTER_GATEWAY trigger is generated
 *         or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemRegisterGateway function will return "E_NOT_POSSIBLE", without the any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemRegisterGatewayNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System52";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    gateway.name                  = "Gateway2";
    gateway.gatewayID             = 2;
    gatewayInfo.sinkName          = "Gateway0";     // Gateway Sinkname
    gatewayInfo.sourceName        = "Gateway0";     // Gateway Sourcename
    gatewayInfo.controlDomainName = "Applications"; // Gateway controlling Domainname
    EXPECT_EQ(E_NOT_POSSIBLE, mpPlugin->hookSystemRegisterGateway(gateway, gateway.gatewayID));

}

/**
 * @brief : Test to verify the hookSystemDeregisterGateway functions for Gateway for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemDeregisterGateway" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController,enterGatewayDB,removeGatewayDB functions to return the E_OK
 *         and attach the sink,source element to class element
 *         set the system property type and value details And pass the gateway Id to hookSystemDeregisterGateway function
 *         then will check whether hookSystemDeregisterGateway will de-register the system gateway element
 *         or not, and also check the SYSTEM_DEREGISTER_GATEWAY trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemDeregisterGateway function will return "E_OK" without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemDeregisterGatewayPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System53";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // GatewayElement
    gateway.name                  = "Gateway1";
    gateway.gatewayID             = 2;
    gatewayInfo.sinkName          = "Gateway1";     // Gateway Sinkname
    gatewayInfo.sourceName        = "Gateway1";     // Gateway Sourcename
    gatewayInfo.controlDomainName = "Applications"; // Gateway controlling Domainname
    EXPECT_CALL(*mpMockControlReceiveInterface, enterGatewayDB(IsGatewayNamePresent(gateway), _)).WillOnce(DoAll(SetArgReferee<1>(2), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->hookSystemRegisterGateway(gateway, gateway.gatewayID));
    //EXPECT_EQ(E_OK, mpPlugin->hookSystemDeregisterGateway(gateway.gatewayID));
}

/**
 * @brief : Test to verify the hookSystemDeregisterGateway functions for Gateway for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "hookSystemDeregisterGateway" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,
 *         getSocketHandler,startupController,enterGatewayDB,removeGatewayDB functions to return the E_OK
 *         and attach the sink,source element to class element
 *         set the system property type and value details And pass the incorrect gateway Id to hookSystemDeregisterGateway function
 *         then will check whether hookSystemDeregisterGateway will de-register the system gateway element
 *         or not, and also check the SYSTEM_DEREGISTER_GATEWAY trigger is generated or not, without any gmock error.
 *
 * @result : "Pass" when hookSystemDeregisterGateway function will return "E_NOT_POSSIBLE" without any Gmock error.
 */
TEST_F(CAmControllerPluginTest, hookSystemDeregisterGatewayNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element
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
    systemConfiguration.name = "System54";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
   // EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
  //  mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // GatewayElement
    gateway.name                  = "Gateway1";
    gateway.gatewayID             = 2;
    gatewayInfo.sinkName          = "Gateway1";     // Gateway Sinkname
    gatewayInfo.sourceName        = "Gateway1";     // Gateway Sourcename
    gatewayInfo.controlDomainName = "Applications"; // Gateway controlling Domainname
   // EXPECT_CALL(*mpMockControlReceiveInterface, enterGatewayDB(IsGatewayNamePresent(gateway), _)).WillOnce(DoAll(SetArgReferee<1>(2), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->hookSystemRegisterGateway(gateway, gateway.gatewayID));
    EXPECT_EQ(E_NOT_POSSIBLE, mpPlugin->hookSystemDeregisterGateway(125));

}

////CallBack Function Implementation

/**
 * @brief : Test to verify the cbAckConnect functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckConnect" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         connect functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make connect call and pass the handle,errorID to cbAckConnect function then will check
 *         whether cbAckConnect function will send acknowledgment for connect call or not, without any gmock error.
 *
 * @result : "Pass" when cbAckConnect function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckConnectPositive)
{

    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System57";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // Get the ControlRecive instace from control send
    am_connectionID_t           connectionID;
    am_CustomConnectionFormat_t format = CF_GENIVI_AUTO;

    am_Handle_s handle;
    handle.handleType = H_CONNECT;
    handle.handle     = 50;

    am_Error_e errorID = E_OK;
    // Registering Test Observer
    EXPECT_CALL(*mpMockControlReceiveInterface, connect(_, _, format, sourceID, sinkID))
        .WillOnce(DoAll(SetArgReferee<0>(handle), SetArgReferee<1>(44), Return(E_OK)));
    am_Handle_s assignedHandle;
    mpCAmControlReceive->connect(assignedHandle, connectionID, format, sourceID, sinkID);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);
    mpPlugin->cbAckConnect(assignedHandle, errorID);
}

/**
 * @brief : Test to verify the cbAckConnect functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckConnect" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         connect functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make connect call and pass the handle,incorrect errorID to cbAckConnect function then will check
 *         whether cbAckConnect function will send acknowledgment for connect call or not, without any gmock error.
 *
 * @result : "Pass" when cbAckConnect function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckConnectNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System58";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // Get the ControlRecive instace from control send
    am_connectionID_t           connectionID;
    am_CustomConnectionFormat_t format = CF_GENIVI_AUTO;

    am_Handle_s handle;
    handle.handleType = H_CONNECT;
    handle.handle     = 20;

    am_Handle_s handleerror;
    handleerror.handleType = H_CONNECT;
    handleerror.handle     = 20;
    am_Error_e errorID = E_NOT_POSSIBLE;
    // Registering Test Observer
    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, connect(_, _, format, sourceID, sinkID)).WillOnce(DoAll(SetArgReferee<0>(handle), SetArgReferee<1>(44), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmControlReceive->connect(assignedHandle, connectionID, format, sourceID, sinkID));
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckConnect(handleerror, errorID);

}

/**
 * @brief : Test to verify the cbAckDisconnect functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckDisconnect" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         connect,disconnect functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make connect,disconnect call and pass the handle,errorID to cbAckDisconnect function then will check
 *         whether cbAckDisconnect function will send acknowledgment for disconnect call or not, without any gmock error.
 *
 * @result : "Pass" when cbAckDisconnect function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckDisconnectPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System59";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Get the ControlRecive instace from control send
    am_connectionID_t           connectionID;
    am_CustomConnectionFormat_t format = CF_GENIVI_AUTO;

    // am_Handle_s handleConnect;
    handleConnect.handleType = H_CONNECT;
    handleConnect.handle     = 50;

    am_Handle_s handledisConnect;
    handledisConnect.handleType = H_DISCONNECT;
    handledisConnect.handle     = 51;
    am_Error_e errorID = E_OK;
    // Registering Test Observer
    am_Handle_s   assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, connect(_, _, format, sourceID, sinkID))
        .WillOnce(DoAll(SetArgReferee<0>(handleConnect), SetArgReferee<1>(44), Return(E_OK)));
    mpCAmControlReceive->connect(assignedHandle, connectionID, format, sourceID, sinkID);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);
    mpPlugin->cbAckConnect(assignedHandle, errorID);

    EXPECT_CALL(*mpMockControlReceiveInterface, disconnect(_, connectionID))
        .WillOnce(DoAll(SetArgReferee<0>(handledisConnect), Return(E_OK)));
    mpCAmControlReceive->disconnect(assignedHandle, connectionID);
    mpPlugin->cbAckDisconnect(handledisConnect, errorID);

}

/**
 * @brief : Test to verify the cbAckDisconnect functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckDisconnect" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         connect,disconnect functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make connect,disconnect call and pass the handle,incorrect errorID to cbAckDisconnect function then will check
 *         whether cbAckDisconnect function will send acknowledgment for disconnect call or not, without any gmock error.
 *
 * @result : "Pass" when cbAckDisconnect function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckDisconnectNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System60";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Get the ControlRecive instace from control send
    am_connectionID_t           connectionID;
    am_CustomConnectionFormat_t format = CF_GENIVI_AUTO;

    // am_Handle_s handleConnect;
    handleConnect.handleType = H_CONNECT;
    handleConnect.handle     = 50;

    am_Handle_s handledisConnect;
    handledisConnect.handleType = H_DISCONNECT;
    handledisConnect.handle     = 51;

    am_Handle_s handledisConnectError;
    handledisConnectError.handleType = H_DISCONNECT;
    handledisConnectError.handle     = 52;
    am_Error_e errorID = E_NOT_POSSIBLE;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, connect(_, _, format, sourceID, sinkID))
        .WillOnce(DoAll(SetArgReferee<0>(handleConnect), SetArgReferee<1>(44), Return(E_OK)));
    mpCAmControlReceive->connect(assignedHandle, connectionID, format, sourceID, sinkID);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);
    mpPlugin->cbAckConnect(handleConnect, errorID);

    EXPECT_CALL(*mpMockControlReceiveInterface, disconnect(_, connectionID))
        .WillOnce(DoAll(SetArgReferee<0>(handledisConnect), Return(E_OK)));
    mpCAmControlReceive->disconnect(assignedHandle, connectionID);
    ASSERT_NO_THROW(mpPlugin->cbAckDisconnect(handledisConnectError, errorID));

}

/**
 * @brief : Test to verify the cbAckSetSinkVolumeChange functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSinkVolumeChange" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         setSinkVolume functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make setSinkVolume call and pass the handle,volume,errorID to cbAckSetSinkVolumeChange function then will check
 *         whether cbAckSetSinkVolumeChange function will send acknowledgment for sink volume changes or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSinkVolumeChange function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSinkVolumeChangePositive)
{

    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System61";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
   // mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
   // EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Get the ControlRecive instace from control send
    am_Handle_s handleVlume;
    handleVlume.handleType = H_SETSINKVOLUME;
    handleVlume.handle     = 40;

    am_volume_t         LimitVolumeParam = 20;
    am_CustomRampType_t RampTypeParam    = RAMP_UNKNOWN;
    am_time_t           RampTimeParam    = 0;

    am_Error_e errorID = E_OK;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkVolume(_, sinkID, LimitVolumeParam, RampTypeParam, RampTimeParam))
        .WillOnce(DoAll(SetArgReferee<0>(handleVlume), Return(E_OK)));
    mpCAmControlReceive->setSinkVolume(assignedHandle, sinkID, LimitVolumeParam, RampTypeParam, RampTimeParam);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);
    mpPlugin->cbAckSetSinkVolumeChange(handleVlume, LimitVolumeParam, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSinkVolumeChange functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSinkVolumeChange" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         setSinkVolume functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make setSinkVolume call and pass the handle,volume,incorrect errorID to cbAckSetSinkVolumeChange function then will check
 *         whether cbAckSetSinkVolumeChange function will send acknowledgment for sink volume changes or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSinkVolumeChange function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSinkVolumeChangeNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System62";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
  //  EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
   // mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // Get the ControlRecive instace from control send

    am_Handle_s handleVlume;
    handleVlume.handleType = H_SETSINKVOLUME;
    handleVlume.handle     = 40;

    am_Handle_s handleVlumeError;
    handleVlumeError.handleType = H_SETSINKVOLUME;
    handleVlumeError.handle     = 42;

    am_volume_t         LimitVolumeParam = 20;
    am_CustomRampType_t RampTypeParam    = RAMP_UNKNOWN;
    am_time_t           RampTimeParam    = 0;
    am_Error_e          errorID          = E_NOT_POSSIBLE;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkVolume(_, sinkID, LimitVolumeParam, RampTypeParam, RampTimeParam))
        .WillOnce(DoAll(SetArgReferee<0>(handleVlume), Return(E_OK)));
    mpCAmControlReceive->setSinkVolume(assignedHandle, sinkID, LimitVolumeParam, RampTypeParam, RampTimeParam);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);
    mpPlugin->cbAckSetSinkVolumeChange(handleVlumeError, LimitVolumeParam, errorID);

}

/**
 * @brief : Test to verify the cbAckSetSourceVolumeChange functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSourceVolumeChange" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         setSourceVolume functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make setSourceVolume call and pass the handle,LimitVolumeParam,errorID to cbAckSetSourceVolumeChange function then will check
 *         whether cbAckSetSourceVolumeChange function will send acknowledgment for source volume changes or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSourceVolumeChange function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSourceVolumeChangePositive)
{

    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System63";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Get the ControlRecive instace from control send
    am_Handle_s handleVlume;
    handleVlume.handleType = H_SETSOURCEVOLUME;
    handleVlume.handle     = 30;

    am_volume_t         LimitVolumeParam = 25;
    am_CustomRampType_t RampTypeParam    = RAMP_UNKNOWN;
    am_time_t           RampTimeParam    = 0;

    am_Error_e errorID = E_OK;

    am_Handle_s  assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceVolume(_, sourceID, LimitVolumeParam, RampTypeParam, RampTimeParam))
        .WillOnce(DoAll(SetArgReferee<0>(handleVlume), Return(E_OK)));
    mpCAmControlReceive->setSourceVolume(assignedHandle, sourceID, LimitVolumeParam, RampTypeParam, RampTimeParam);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);
    mpPlugin->cbAckSetSourceVolumeChange(handleVlume, LimitVolumeParam, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSourceVolumeChange functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSourceVolumeChange" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         setSourceVolume functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make setSourceVolume call and pass the handle,LimitVolumeParam,incorrect errorID to cbAckSetSourceVolumeChange function
 *         then will check whether cbAckSetSourceVolumeChange function will send acknowledgment for source volume changes
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSourceVolumeChange function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSourceVolumeChangeNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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

    systemConfiguration.name = "System64";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    // mpCAmControlReceive->changeSystemPropertyDB(systemproperty);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Get the ControlRecive instace from control send
    am_Handle_s handleVlume;
    handleVlume.handleType = H_SETSOURCEVOLUME;
    handleVlume.handle     = 30;

    am_Handle_s handleVlumeError;
    handleVlumeError.handleType = H_SETSOURCEVOLUME;
    handleVlumeError.handle     = 32;

    am_volume_t         LimitVolumeParam = 25;
    am_CustomRampType_t RampTypeParam    = RAMP_UNKNOWN;
    am_time_t           RampTimeParam    = 0;

    am_Error_e errorID = E_NOT_POSSIBLE;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceVolume(_, sourceID, LimitVolumeParam, RampTypeParam, RampTimeParam))
        .WillOnce(DoAll(SetArgReferee<0>(handleVlume), Return(E_OK)));
    mpCAmControlReceive->setSourceVolume(assignedHandle, sourceID, LimitVolumeParam, RampTypeParam, RampTimeParam);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);
    mpPlugin->cbAckSetSourceVolumeChange(handleVlumeError, LimitVolumeParam, errorID);

}

/**
 * @brief : Test to verify the cbAckSetSourceState functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSourceState" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         SetSourceState functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSourceState call and pass the handle,LimitVolumeParam,errorID to cbAckSetSourceState function then will check
 *         whether cbAckSetSourceState function will send acknowledgment for setting of source states or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSourceState function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSourceStatePositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System65";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
//    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));

    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Get the ControlRecive instace from control send
    am_SourceState_e sourceState = SS_ON;
    am_Handle_s      handleSetSourceState;
    handleSetSourceState.handleType = H_SETSOURCESTATE;
    handleSetSourceState.handle     = 45;
    am_Error_e errorID = E_OK;

    am_Handle_s      assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceState(_, sourceID, sourceState))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSourceState), Return(E_OK)));
    mpCAmControlReceive->setSourceState(assignedHandle, sourceID, sourceState);

    mpPlugin->cbAckSetSourceState(handleSetSourceState, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSourceState functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSourceState" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         SetSourceState functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSourceState call and pass the handle,LimitVolumeParam,incorrect errorID to cbAckSetSourceState function then will check
 *         whether cbAckSetSourceState function will send acknowledgment for setting of source states or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSourceState function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSourceStateNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System66";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
//    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Get the ControlRecive instace from control send
    am_SourceState_e sourceState = SS_ON;

    am_Handle_s handleSetSourceState;
    handleSetSourceState.handleType = H_SETSOURCESTATE;
    handleSetSourceState.handle     = 45;

    am_Handle_s handleSetSourceStateError;
    handleSetSourceStateError.handleType = H_SETSOURCESTATE;
    handleSetSourceStateError.handle     = 43;
    am_Error_e errorID = E_NOT_POSSIBLE;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceState(_, sourceID, sourceState))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSourceState), Return(E_OK)));
    mpCAmControlReceive->setSourceState(assignedHandle, sourceID, sourceState);

    mpPlugin->cbAckSetSourceState(handleSetSourceStateError, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSourceSoundProperties functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSourceSoundProperties" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         setSourceSoundProperties functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSourceSoundProperties call and pass handle,errorID to cbAckSetSourceSoundProperties function then will check
 *         whether cbAckSetSourceSoundProperties function will send acknowledgment for setting of source sound properties
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSourceSoundProperties function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSourceSoundPropertiesPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element
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
    systemConfiguration.name = "System67";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
//    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Get the ControlRecive instace from control send
    // Sound properties
    std::vector<am_SoundProperty_s > listSoundProperties;
    am_SoundProperty_s               soundProperties;
    soundProperties.type  = SP_GENIVI_TREBLE;
    soundProperties.value = 10;
    listSoundProperties.push_back(soundProperties);
    soundProperties.type  = SP_GENIVI_BASS;
    soundProperties.value = 12;
    listSoundProperties.push_back(soundProperties);

    am_Handle_s handleSetSourcesound;
    handleSetSourcesound.handleType = H_SETSOURCESOUNDPROPERTIES;
    handleSetSourcesound.handle     = 48;
    am_Error_e errorID = E_OK;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceSoundProperties(_, sourceID, setMainSoundproperty(soundProperties)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSourcesound), Return(E_OK)));
    mpCAmControlReceive->setSourceSoundProperties(assignedHandle, sourceID, listSoundProperties);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSourceSoundProperties(handleSetSourcesound, errorID);

}

/**
 * @brief : Test to verify the cbAckSetSourceSoundProperties functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSourceSoundProperties" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         setSourceSoundProperties functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSourceState call and pass handle,incorrect errorID to cbAckSetSourceSoundProperties function then will check
 *         whether cbAckSetSourceSoundProperties function will send acknowledgment for setting of source sound properties
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSourceSoundProperties function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSourceSoundPropertiesNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System68";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
   // EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // Sound properties
    std::vector<am_SoundProperty_s > listSoundProperties;
    am_SoundProperty_s               soundProperties;
    soundProperties.type  = SP_GENIVI_TREBLE;
    soundProperties.value = 10;
    listSoundProperties.push_back(soundProperties);
    soundProperties.type  = SP_GENIVI_BASS;
    soundProperties.value = 12;
    listSoundProperties.push_back(soundProperties);

    am_Handle_s handleSetSourcesound;
    handleSetSourcesound.handleType = H_SETSOURCESOUNDPROPERTIES;
    handleSetSourcesound.handle     = 48;

    am_Handle_s handleSetSourcesoundError;
    handleSetSourcesoundError.handleType = H_SETSOURCESOUNDPROPERTIES;
    handleSetSourcesoundError.handle     = 49;

    am_Error_e errorID = E_NOT_POSSIBLE;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceSoundProperties(_, sourceID, setMainSoundproperty(soundProperties)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSourcesound), Return(E_OK)));
    mpCAmControlReceive->setSourceSoundProperties(assignedHandle, sourceID, listSoundProperties);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSourceSoundProperties(handleSetSourcesoundError, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSourceSoundProperty functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSourceSoundProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         setSourceSoundProperties functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSourceSoundProperty call and pass handle,errorID to cbAckSetSourceSoundProperty function then will check
 *         whether cbAckSetSourceSoundProperty function will send acknowledgment for setting of source sound properties
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSourceSoundProperty function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSourceSoundPropertyPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
   // EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    // System element
    systemConfiguration.name = "System69";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
   // EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
  // EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    pSystem = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Get the ControlRecive instace from control send

    // Sound properties
    am_SoundProperty_s soundProperty;
    soundProperty.type  = SP_GENIVI_TREBLE;
    soundProperty.value = 10;

    am_Handle_s handleSetSourcesound;
    handleSetSourcesound.handleType = H_SETSOURCESOUNDPROPERTY;
    handleSetSourcesound.handle     = 48;
    am_Error_e errorID = E_OK;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceSoundProperty(_, sourceID, setMainSoundproperty(soundProperty)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSourcesound), Return(E_OK)));
    mpCAmControlReceive->setSourceSoundProperty(assignedHandle, sourceID, soundProperty);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSourceSoundProperty(handleSetSourcesound, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSourceSoundProperty functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSourceSoundProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         SetSourceSoundProperty functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSourceSoundProperty call and pass handle,incorrect errorID to cbAckSetSourceSoundProperty function then will check
 *         whether cbAckSetSourceSoundProperty function will send acknowledgment for setting of source sound properties
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSourceSoundProperty function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSourceSoundPropertyNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System70";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
   // EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Get the ControlRecive instace from control send
    // Sound properties
    am_SoundProperty_s soundProperty;
    soundProperty.type  = SP_GENIVI_TREBLE;
    soundProperty.value = 10;

    am_Handle_s handleSetSourcesound;
    handleSetSourcesound.handleType = H_SETSOURCESOUNDPROPERTY;
    handleSetSourcesound.handle     = 48;

    am_Handle_s handleSetSourcesoundError;
    handleSetSourcesoundError.handleType = H_SETSOURCESOUNDPROPERTY;
    handleSetSourcesoundError.handle     = 49;
    am_Error_e errorID = E_OK;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceSoundProperty(_, sourceID, setMainSoundproperty(soundProperty)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSourcesound), Return(E_OK)));
    mpCAmControlReceive->setSourceSoundProperty(assignedHandle, sourceID, soundProperty);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSourceSoundProperty(handleSetSourcesoundError, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSinkSoundProperties functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSinkSoundProperties" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         SetSinkSoundProperties functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSinkSoundProperties call and pass handle,errorID to cbAckSetSinkSoundProperties function then will check
 *         whether cbAckSetSinkSoundProperties function will send acknowledgment for setting of sink sound properties
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSinkSoundProperties function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSinkSoundPropertiesPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System71";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
   //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
   // mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // Sound properties
    std::vector<am_SoundProperty_s > listSoundProperties;
    am_SoundProperty_s               soundProperties;
    soundProperties.type  = SP_GENIVI_TREBLE;
    soundProperties.value = 10;
    listSoundProperties.push_back(soundProperties);
    soundProperties.type  = SP_GENIVI_BASS;
    soundProperties.value = 12;
    listSoundProperties.push_back(soundProperties);

    am_Handle_s handleSetSinksound;
    handleSetSinksound.handleType = H_SETSINKSOUNDPROPERTIES;
    handleSetSinksound.handle     = 35;

    am_Error_e errorID = E_OK;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkSoundProperties(_, sinkID, setMainSoundproperty(soundProperties)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSinksound), Return(E_OK)));
    mpCAmControlReceive->setSinkSoundProperties(assignedHandle, sinkID, listSoundProperties);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSinkSoundProperties(handleSetSinksound, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSinkSoundProperties functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSinkSoundProperties" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         SetSinkSoundProperties functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSinkSoundProperties call and pass handle,incorrect errorID to cbAckSetSinkSoundProperties function then will check
 *         whether cbAckSetSinkSoundProperties function will send acknowledgment for setting of sink sound properties
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSinkSoundProperties function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSinkSoundPropertiesNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System72";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // Sound properties
    std::vector<am_SoundProperty_s > listSoundProperties;
    am_SoundProperty_s               soundProperties;
    soundProperties.type  = SP_GENIVI_TREBLE;
    soundProperties.value = 10;
    listSoundProperties.push_back(soundProperties);
    soundProperties.type  = SP_GENIVI_BASS;
    soundProperties.value = 12;
    listSoundProperties.push_back(soundProperties);

    am_Handle_s handleSetSinksound;
    handleSetSinksound.handleType = H_SETSINKSOUNDPROPERTIES;
    handleSetSinksound.handle     = 35;

    am_Handle_s handleSetSinksoundError;
    handleSetSinksoundError.handleType = H_SETSINKSOUNDPROPERTIES;
    handleSetSinksoundError.handle     = 36;
    am_Error_e errorID = E_OK;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkSoundProperties(_, sinkID, setMainSoundproperty(soundProperties)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSinksound), Return(E_OK)));
    mpCAmControlReceive->setSinkSoundProperties(assignedHandle, sinkID, listSoundProperties);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSinkSoundProperties(handleSetSinksoundError, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSinkSoundProperty functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSinkSoundProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         SetSinkSoundProperty functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSinkSoundProperty call and pass handle,errorID to cbAckSetSinkSoundProperty function then will check
 *         whether cbAckSetSinkSoundProperty function will send acknowledgment for setting of sink sound properties
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSinkSoundProperties function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSinkSoundPropertyPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System73";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // Sound properties
    am_SoundProperty_s soundProperty;
    soundProperty.type  = SP_GENIVI_TREBLE;
    soundProperty.value = 10;
    am_Handle_s handleSetSinksound;
    handleSetSinksound.handleType = H_SETSINKSOUNDPROPERTY;
    handleSetSinksound.handle     = 37;
    am_Error_e errorID = E_OK;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkSoundProperty(_, sinkID, setMainSoundproperty(soundProperty)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSinksound), Return(E_OK)));
    mpCAmControlReceive->setSinkSoundProperty(assignedHandle, sinkID, soundProperty);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSinkSoundProperty(handleSetSinksound, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSinkSoundProperty functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSinkSoundProperty" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         SetSinkSoundProperty functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSinkSoundProperty call and pass handle,incorrect errorID to cbAckSetSinkSoundProperty function then will check
 *         whether cbAckSetSinkSoundProperty function will send acknowledgment for setting of sink sound properties
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSinkSoundProperties function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSinkSoundPropertyNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System74";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // Sound properties
    am_SoundProperty_s soundProperty;
    soundProperty.type  = SP_GENIVI_TREBLE;
    soundProperty.value = 10;

    am_Handle_s handleSetSinksound;
    handleSetSinksound.handleType = H_SETSINKSOUNDPROPERTY;
    handleSetSinksound.handle     = 37;

    am_Handle_s handleSetSinksoundError;
    handleSetSinksoundError.handleType = H_SETSINKSOUNDPROPERTY;
    handleSetSinksoundError.handle     = 39;

    am_Error_e errorID = E_OK;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkSoundProperty(_, sinkID, setMainSoundproperty(soundProperty)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSinksound), Return(E_OK)));
    mpCAmControlReceive->setSinkSoundProperty(assignedHandle, sinkID, soundProperty);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSinkSoundProperty(handleSetSinksoundError, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSinkNotificationConfiguration functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSinkNotificationConfiguration" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         SetSinkNotificationConfiguration functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSinkNotificationConfiguration call and pass handle,errorID to cbAckSetSinkNotificationConfiguration function then will check
 *         whether cbAckSetSinkNotificationConfiguration function will send acknowledgment for setting of sink notification configuration
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSinkNotificationConfiguration function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSinkNotificationConfigurationPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System75";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Notification Configuration
    notificationConfiguration.type      = NT_OVER_TEMPERATURE;
    notificationConfiguration.status    = NS_PERIODIC;
    notificationConfiguration.parameter = 20;
    am_Handle_s handleSetSinkNotificationConfig;
    handleSetSinkNotificationConfig.handleType = H_SETSINKNOTIFICATION;
    handleSetSinkNotificationConfig.handle     = 37;
    am_Error_e errorID = E_OK;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkNotificationConfiguration(_, sinkID, setNotificationConfigurations(notificationConfiguration)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSinkNotificationConfig), Return(E_OK)));
    mpCAmControlReceive->setSinkNotificationConfiguration(assignedHandle, sinkID, notificationConfiguration);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSinkNotificationConfiguration(handleSetSinkNotificationConfig, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSinkNotificationConfiguration functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSinkNotificationConfiguration" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         SetSinkNotificationConfiguration functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSinkNotificationConfiguration call and pass handle,incorrect errorID to cbAckSetSinkNotificationConfiguration function then will check
 *         whether cbAckSetSinkNotificationConfiguration function will send acknowledgment for setting of sink notification configuration
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSinkNotificationConfiguration function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSinkNotificationConfigurationNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System76";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // Get the ControlRecive instace from control send
    // Notification Configuration
    notificationConfiguration.type      = NT_OVER_TEMPERATURE;
    notificationConfiguration.status    = NS_PERIODIC;
    notificationConfiguration.parameter = 20;

    am_Handle_s handleSetSinkNotificationConfig;
    handleSetSinkNotificationConfig.handleType = H_SETSINKNOTIFICATION;
    handleSetSinkNotificationConfig.handle     = 37;
    am_Handle_s handleSetSinkNotificationConfigError;
    handleSetSinkNotificationConfigError.handleType = H_SETSINKNOTIFICATION;
    handleSetSinkNotificationConfigError.handle     = 56;
    am_Error_e errorID = E_NOT_POSSIBLE;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkNotificationConfiguration(_, sinkID, setNotificationConfigurations(notificationConfiguration)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSinkNotificationConfig), Return(E_NOT_POSSIBLE)));
    mpCAmControlReceive->setSinkNotificationConfiguration(assignedHandle, sinkID, notificationConfiguration);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSinkNotificationConfiguration(handleSetSinkNotificationConfigError, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSourceNotificationConfiguration functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSourceNotificationConfiguration" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         SetSourceNotificationConfiguration functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSourceNotificationConfiguration call and pass handle,errorID to cbAckSetSourceNotificationConfiguration function then will check
 *         whether cbAckSetSourceNotificationConfiguration function will send acknowledgment for setting of source notification configuration
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSourceNotificationConfiguration function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSourceNotificationConfigurationPositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
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
    systemConfiguration.name = "System77";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);

    // Get the ControlRecive instace from control send
    // YS
    // Notification Configuration
    notificationConfiguration.type      = NT_OVER_TEMPERATURE;
    notificationConfiguration.status    = NS_PERIODIC;
    notificationConfiguration.parameter = 20;
    am_Handle_s handleSetSourceNotificationConfig;
    handleSetSourceNotificationConfig.handleType = H_SETSOURCENOTIFICATION;
    handleSetSourceNotificationConfig.handle     = 58;
    am_Error_e errorID = E_OK;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceNotificationConfiguration(_, sourceID, setNotificationConfigurations(notificationConfiguration)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSourceNotificationConfig), Return(E_OK)));
    mpCAmControlReceive->setSourceNotificationConfiguration(assignedHandle, sourceID, notificationConfiguration);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSourceNotificationConfiguration(handleSetSourceNotificationConfig, errorID);
}

/**
 * @brief : Test to verify the cbAckSetSourceNotificationConfiguration functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "cbAckSetSourceNotificationConfiguration" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         SetSourceNotificationConfiguration functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & make SetSourceNotificationConfiguration call and pass handle,incorrect errorID to cbAckSetSourceNotificationConfiguration function then will check
 *         whether cbAckSetSourceNotificationConfiguration function will send acknowledgment for setting of source notification configuration
 *         or not, without any gmock error.
 *
 * @result : "Pass" when cbAckSetSourceNotificationConfiguration function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, cbAckSetSourceNotificationConfigurationNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element
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
    systemConfiguration.name = "System78";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // Get the ControlRecive instace from control send
    // Notification Configuration
    notificationConfiguration.type      = NT_OVER_TEMPERATURE;
    notificationConfiguration.status    = NS_PERIODIC;
    notificationConfiguration.parameter = 20;

    am_Handle_s handleSetSourceNotificationConfig;
    handleSetSourceNotificationConfig.handleType = H_SETSOURCENOTIFICATION;
    handleSetSourceNotificationConfig.handle     = 58;

    am_Handle_s handleSetSourceNotificationConfigError;
    handleSetSourceNotificationConfigError.handleType = H_SETSOURCENOTIFICATION;
    handleSetSourceNotificationConfigError.handle     = 59;

    am_Error_e errorID = E_NOT_POSSIBLE;

    am_Handle_s assignedHandle;
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceNotificationConfiguration(_, sourceID, setNotificationConfigurations(notificationConfiguration)))
        .WillOnce(DoAll(SetArgReferee<0>(handleSetSourceNotificationConfig), Return(E_NOT_POSSIBLE)));
    mpCAmControlReceive->setSourceNotificationConfiguration(assignedHandle, sourceID, notificationConfiguration);
    CAmHandleStore::instance().saveHandle(assignedHandle, testObserver);

    mpPlugin->cbAckSetSourceNotificationConfiguration(handleSetSourceNotificationConfigError, errorID);
}

/**
 * @brief : Test to verify the confirmRoutingReady functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "confirmRoutingReady" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         getListSystemProperties functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & pass error to confirmRoutingReady function then will check
 *         whether confirmRoutingReady function will confirms the setRoutingReady call done
 *         or not, without any gmock error.
 *
 * @result : "Pass" when confirmRoutingReady function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, ConfirmRoutingReadyPositive)
{
    sink.name   = "AnySink79";
    source.name = "AnySource79";

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_REGISTRATION_DOMAIN_TIMEOUT;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System79";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    am_Error_e error = E_OK;
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    mpPlugin->confirmRoutingReady(error);

}

/**
 * @brief : Test to verify the confirmRoutingReady functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "confirmRoutingReady" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         getListSystemProperties functions to return the E_OK, and attach the sink,source element to class element
 *         set the details to am_Handle_s structure, register the test Observer using registerObserver call
 *         & pass incorrect error to confirmRoutingReady function then will check
 *         whether confirmRoutingReady function will confirms the setRoutingReady call done
 *         or not, without any gmock error.
 *
 * @result : "Pass" when confirmRoutingReady function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, ConfirmRoutingReadyNegative)
{
    sink.name   = "AnySink80";
    source.name = "AnySource80";

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;

    // SystemProperty creating
    systemproperty.type  = SYP_REGISTRATION_DOMAIN_TIMEOUT;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System80";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    am_Error_e error = E_ABORTED;
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_NOT_POSSIBLE)));
    mpPlugin->confirmRoutingReady(error);

}

/**
 * @brief : Test to verify the persistenceStore update for MainConnectionString functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "update" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         getMainConnectionInfoDB functions to return the E_OK, and attach the sink,source element to class element
 *         set the PersistencySupported  as enable for sink and source, create the main connection
 *         and push the main connection in list & pass pMainConnection and state to update function then will check
 *         whether update is done or not, without any gmock error.
 *
 * @result : "Pass" when update function will not return any Gmock error.
 */
TEST_F(CAmControllerPluginTest, persistenceStoreMainConnectionStringpositive)
{
    sink.name                               = "Gateway0";
    sinkInfo.isVolumeChangeSupported        = true;
    sinkInfo.isPersistencySupported         = true;
    sinkInfo.isVolumePersistencySupported   = true;
    source.name                             = "MediaPlayer";
    sourceInfo.isVolumeChangeSupported      = true;
    sourceInfo.isPersistencySupported       = true;
    sourceInfo.isVolumePersistencySupported = true;
    // Source sink and main connection ids
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();

    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "BASE";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "BASE";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);

    classElement.name                         = "BASE";
    classElement.type                         = C_PLAYBACK;
    classElement.priority                     = 1;
    classElement.isVolumePersistencySupported = true;
    classElement.defaultVolume                = 10;

    // sink Element belongs to class

    classElement.name                         = "BASE";
    classElement.type                         = C_CAPTURE;
    classElement.priority                     = 1;
    classElement.isVolumePersistencySupported = true;
    classElement.defaultVolume                = 10;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System81";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    //mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // creating the class connection element
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    // main connection ids
    am_mainConnectionID_t mainConnectionID = 0;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Am route Info
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    listRoutes.push_back(amRoute);

    // GC route Info
    // Route inforrmation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(50), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(50, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

// creating main connection Element
    pMainConnection = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    pClassElement->attach(pMainConnection);
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(50, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    am_ConnectionState_e state = CS_CONNECTED;
    pClassElement->update(pMainConnection, state);
}

/**
 * @brief : Test to verify the persistenceStore for Class level persistence sink volume functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "restoreVolume" function when sink,source,class elements are initialized with
 *         default configuration data and set the behavior of enterSystemPropertiesListDB,getSocketHandler,startupController
 *         getMainConnectionInfoDB functions to return the E_OK, and attach the sink,source element to class element
 *         set the PersistencySupported  as enable for sink and source, set the structure gc_SinkVolume_s,gc_LastClassVolume_s etc.
 *         and pass the gcLastClassVolume to restoreVolume function then will check whether restore Volume is done or
 *         not, without any gmock error.
 *
 * @result : "Pass" when restoreVolume function will not return any Gmock error.
 */

TEST_F(CAmControllerPluginTest, persistenceStoreClasslevelpersinkVolumepositive)
{
    sink.name                               = "Gateway0";
    sinkInfo.isVolumeChangeSupported        = true;
    sinkInfo.isPersistencySupported         = true;
    sinkInfo.isVolumePersistencySupported   = true;
    source.name                             = "MediaPlayer";
    sourceInfo.isVolumeChangeSupported      = true;
    sourceInfo.isPersistencySupported       = true;
    sourceInfo.isVolumePersistencySupported = true;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 72;
    sinkClass.name        = "BASE";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 73;
    sourceClass.name          = "BASE";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element

    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);

    classElement.name                         = "BASE";
    classElement.type                         = C_PLAYBACK;
    classElement.priority                     = 1;
    classElement.isVolumePersistencySupported = true;
    classElement.defaultVolume                = 10;

    // sink Element belongs to class

    classElement.name                         = "BASE";
    classElement.type                         = C_CAPTURE;
    classElement.priority                     = 1;
    classElement.isVolumePersistencySupported = true;
    classElement.defaultVolume                = 10;

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    // System element
    systemConfiguration.name = "System82";
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));
    pClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == pClassElement)
    {
        ASSERT_THAT(pClassElement, IsNull()) << " CAmControllerPluginTest: Not able to create classElement object ";
    }

    pClassElement->attach(mpCAmSinkElement);
    pClassElement->attach(mpCAmSourceElement);
    std::vector<gc_SinkVolume_s> listSinkVolume;
    gc_SinkVolume_s              gcSinkVolume;
    gcSinkVolume.sinkName   = sink.name;
    gcSinkVolume.mainVolume = 15;
    listSinkVolume.push_back(gcSinkVolume);
    gc_LastClassVolume_s gcLastClassVolume;
    gcLastClassVolume.className      = classElement.name;
    gcLastClassVolume.listSinkVolume = listSinkVolume;
    EXPECT_NO_THROW(pClassElement->restoreVolume(gcLastClassVolume));

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
