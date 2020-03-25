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
#include "CAmGatewayElement.h"
#include "CAmClassElement.h"
#include "CAmRouteElement.h"
#include "CAmSystemActionRegister.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmSystemActionRegisterTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;
#define DOMAINID 2

/**
 *@Class : CAmSystemActionRegisterTest
 *@brief : This class is used to test the CAmSystemActionRegister functionality.
 */
CAmSystemActionRegisterTest::CAmSystemActionRegisterTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmSinkElement(nullptr)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
    , mpCAmSourceElement(nullptr)
    , pSystem(nullptr)
    , mpCAmSystemActionRegister(NULL)
    , mpCAmRouteElement(NULL)
{
}

CAmSystemActionRegisterTest::~CAmSystemActionRegisterTest()
{
}

void CAmSystemActionRegisterTest::InitializeCommonStruct()
{
    sink.name                      = "AnySink1";
    sink.sinkClassID               = 78;
    source.name                    = "AnySource1";
    sink.domainID                  = 25;
    sinkInfo.domainName            = "AnyDomain5";
    sinkInfo.className             = "AnyClass5";
    sinkInfo.listConnectionFormats = listConnectionFormats;
    listSinks.push_back(sinkInfo);
    mListSinks.setParam(listSinks);
    source.domainID                  = 26;
    sourceInfo.domainName            = "AnyDomain5";
    sourceInfo.className             = "AnyClass5";
    sourceInfo.listConnectionFormats = listConnectionFormats;
    listSources.push_back(sourceInfo);
    mListSources.setParam(listSources);
    domain.name                 = "AnyDomain1";
    domain.domainID             = 25;
    state                       = DS_CONTROLLED;
    domainInfo.busname          = "AnyBus";
    domainInfo.nodename         = "AnyNode";
    domainInfo.early            = true;
    domainInfo.complete         = false;
    domainInfo.state            = DS_UNKNOWN;
    domainInfo.registrationType = REG_CONTROLLER;
    listDomains.push_back(domain);
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);

    type  = SYP_GLOBAL_LOG_THRESHOLD;
    value = 5;

}

void CAmSystemActionRegisterTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;
    // This will check whether controller is initialized or not
    ASSERT_THAT(mpCAmControlReceive, NotNull()) << " Controller Not Initialized";
    // Initializing Common Struture
    InitializeCommonStruct();
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    source.name = "AnySource1";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpCAmSourceElement = CAmSourceFactory::createElement(sourceInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSourceElement, NotNull()) << " Source Element Is not Created";
    sourceIDList.push_back(mpCAmSourceElement->getID());
    sourceID = mpCAmSourceElement->getID();

    mpCAmDomainElement = new CAmDomainElement(domainInfo, mpCAmControlReceive);
    mpCAmDomainElement->setID(25);
    domainID = mpCAmDomainElement->getID();

    // creating the routing element
    // gc_RoutingElement_s gcRoutingElement;
    gcRoutingElement.name             = mpCAmSourceElement->getName() + mpCAmSinkElement->getName();
    gcRoutingElement.sourceID         = mpCAmSourceElement->getID();
    gcRoutingElement.sinkID           = mpCAmSinkElement->getID();
    gcRoutingElement.domainID         = DOMAINID;
    gcRoutingElement.connectionFormat = CF_GENIVI_STEREO;
    mpCAmRouteElement                 = new CAmRouteElement(gcRoutingElement, mpCAmControlReceive);
    ASSERT_THAT(mpCAmRouteElement, NotNull()) << " Route Element Is not Created";

}

void CAmSystemActionRegisterTest::TearDown()
{
    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();
    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }

    if (mpCAmSystemActionRegister != NULL)
    {
        delete (mpCAmSystemActionRegister);
    }
}

/**
 * @brief : Test to verify the _execute functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive,CAmDomainElement,CAmRouteElement classes,
 *        Source, sink, domain, route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc.
 *        source,sink element details is enter in db for behavior set.
 *        and list of domain structure is created and details also push in that list about domain details.
 *
 * @test : verify the "_execute" function when domain element created & is enter in the db,class element is configured with all
 *         details like belonging source,sink name,topology,class property,registration type etc.
 *         & gateway element is created & configured and enter in db, CAmSystemActionRegister is initialized.
 *         & set the enterGatewayDB function to return the "E_OK" & sink,source, gateway action parameter is set for
 *         class CAmSystemActionRegister then will check whether _execute function based on parameter set invokes the
 *         routing manager API to register the element or not, without any gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSystemActionRegisterTest, executePositiveCase1)
{
    domain.name = "AnyDomain1";
    listDomains.push_back(domain);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(IsDomainNamePresent(domain), _)).WillOnce(DoAll(SetArgReferee<1>(25), Return(E_OK)));
    pDomainElement = CAmDomainFactory::createElement(domainInfo, mpCAmControlReceive);
    // Class Element
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 75;
    sinkClass.name                  = "AnySinkClass5";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 76;
    sourceClass.name                = "AnySourceClass5";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);
    // source Element belongs to class
    classElement.name                = "AnyClass1";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;
    classElement.name                = "AnyClass1";
    classElement.type                = C_CAPTURE;
    classElement.priority            = 1;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(75), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(76), Return(E_OK)));
    pClassElement    = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    connectionformat = CF_GENIVI_STEREO;
    listConnectionFormats.push_back(connectionformat);
    // Gateway Element
    gateway.name                  = "Gateway1";
    gateway.gatewayID             = 25;
    gatewayInfo.sinkName          = "AnySink1";   // Gateway Sinkname
    gatewayInfo.sourceName        = "AnySource1"; // Gateway Sourcename
    gatewayInfo.controlDomainName = "AnyDomain1"; // Gateway controlling Domainname
    listGateways.push_back(gatewayInfo);
    mListGateways.setParam(listGateways);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterGatewayDB(IsGatewayNamePresent(gateway), _)).WillOnce(DoAll(SetArgReferee<1>(25), Return(E_OK)));
    pGatewayElement          = CAmGatewayFactory::createElement(gatewayInfo, mpCAmControlReceive);
    systemConfiguration.name = "SystemProp";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    pSystem                   = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    mpCAmSystemActionRegister = new CAmSystemActionRegister(pSystem);

    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SOURCE_INFO, &mListSources);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SINK_INFO, &mListSinks);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_GATEWAY_INFO, &mListGateways);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    EXPECT_EQ(mpCAmSystemActionRegister->execute(), E_OK);

}

/**
 * @brief : Test to verify the _execute functions for with gateway details incorrect
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive,CAmDomainElement,CAmRouteElement classes,
 *        Source, sink, domain, route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc.
 *        source,sink element details is enter in db for behavior set.
 *        and list of domain structure is created and details also push in that list about domain details.
 *
 * @test : verify the "_execute" function when gateway details are incorrect & domain element created & is enter in the db,class element is configured with all
 *         details like belonging source,sink name,topology,class property,registration type etc.
 *         & gateway element is created & configured and enter in db, CAmSystemActionRegister is initialized.
 *         & set the enterGatewayDB function to return the "E_OK" & sink,source, gateway action parameter is set for
 *         class CAmSystemActionRegister then will check whether _execute function based on parameter set invokes the
 *         routing manager API to register the element or not, without any gmock error.
 *
 * @result : "Pass" when _execute function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmSystemActionRegisterTest, executeNegativeGateway)
{
    // Class Element
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 76;
    sinkClass.name                  = "AnySinkClass6";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 77;
    sourceClass.name                = "AnySourceClass6";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);
    classElement.name                = "AnyClass6";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;
    // sink Element belongs to class
    classElement.name     = "AnyClass6";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    connectionformat      = CF_GENIVI_STEREO;
    listConnectionFormats.push_back(connectionformat);
    sink.domainID   = 27;
    source.domainID = 28;
    // Gateway Element
    gateway.name      = "AnyGateway2";
    gateway.gatewayID = 28;
    // gc_Gateway_s gatewayInfo;
    gatewayInfo.sinkName          = "AnySink6";   // Gateway Sinkname
    gatewayInfo.sourceName        = "AnySource6"; // Gateway Sourcename
    gatewayInfo.controlDomainName = "AnyDomain7"; // Gateway controlling Domainname
    listGateways.push_back(gatewayInfo);
    mListGateways.setParam(listGateways);

    pGatewayElement          = CAmGatewayFactory::createElement(gatewayInfo, mpCAmControlReceive);
    systemConfiguration.name = "SystemProp1";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    pSystem                   = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    mpCAmSystemActionRegister = new CAmSystemActionRegister(pSystem);

    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SOURCE_INFO, &mListSources);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SINK_INFO, &mListSinks);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_GATEWAY_INFO, &mListGateways);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    EXPECT_EQ(mpCAmSystemActionRegister->execute(), E_OK);
}

/**
 * @brief : Test to verify the _execute functions for with source details incorrect
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive,CAmDomainElement,CAmRouteElement classes,
 *        Source, sink, domain, route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc.
 *        source,sink element details is enter in db for behavior set.
 *        and list of domain structure is created and details also push in that list about domain details.
 *
 * @test : verify the "_execute" function when source details are incorrect & domain element created & is enter in the db,class element is configured with all
 *         details like belonging source,sink name,topology,class property,registration type etc.
 *         & gateway element is created & configured and enter in db, CAmSystemActionRegister is initialized.
 *         & set the enterGatewayDB function to return the "E_OK" & sink,source, gateway action parameter is set for
 *         class CAmSystemActionRegister then will check whether _execute function based on parameter set invokes the
 *         routing manager API to register the element or not, without any gmock error.
 *
 * @result : "Pass" when _execute function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmSystemActionRegisterTest, executeNegativeSource)
{
    // Class Element
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 77;
    sinkClass.name                  = "AnySinkClass7";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 78;
    sourceClass.name                = "AnySourceClass7";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);
    // source Element belongs to class
    classElement.name                = "AnyClass7";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;
    // sink Element belongs to class
    classElement.name     = "AnyClass7";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(77), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(78), Return(E_OK)));
    pClassElement    = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    connectionformat = CF_GENIVI_STEREO;
    listConnectionFormats.push_back(connectionformat);
    // Sink element
    gateway.name      = "AnyGateway3";
    gateway.gatewayID = 29;

    systemConfiguration.name = "SystemProp2";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    pSystem                   = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    mpCAmSystemActionRegister = new CAmSystemActionRegister(pSystem);

    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SOURCE_INFO, &mListSources);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SINK_INFO, &mListSinks);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_GATEWAY_INFO, &mListGateways);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    EXPECT_EQ(mpCAmSystemActionRegister->execute(), E_OK);
}

/**
 * @brief : Test to verify the _execute functions
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive,CAmDomainElement,CAmRouteElement classes,
 *        Source, sink, domain, route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc.
 *        source,sink element details is enter in db for behavior set.
 *        and list of domain structure is created and details also push in that list about domain details.
 *
 * @test : verify the "_execute" function when domain element created & is enter in the db,class element is configured with all
 *         details like belonging source,sink name,topology,class property,registration type etc.
 *         & gateway element is created & configured and enter in db, CAmSystemActionRegister is initialized.
 *         & set the enterGatewayDB function to return the "E_OK" & sink,source, gateway action parameter is set for
 *         class CAmSystemActionRegister then will check whether _execute function based on parameter set invokes the
 *         routing manager API to register the element or not, without any gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSystemActionRegisterTest, executePositive2)
{
    ////Class Element
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 78;
    sinkClass.name                  = "AnySinkClass8";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 79;
    sourceClass.name                = "AnySourceClass8";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);
    // source Element belongs to class
    classElement.name                = "AnyClass8";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;
    // sink Element belongs to class
    classElement.name     = "AnyClass8";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(78), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(79), Return(E_OK)));
    pClassElement    = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    connectionformat = CF_GENIVI_STEREO;
    listConnectionFormats.push_back(connectionformat);
    gateway.name                  = "AnyGateway4";
    gateway.gatewayID             = 30;
    gatewayInfo.sinkName          = "AnySink8";    // Gateway Sinkname
    gatewayInfo.sourceName        = "AnySource8";  // Gateway Sourcename
    gatewayInfo.controlDomainName = "AnyDomain11"; // Gateway controlling Domainname
    listGateways.push_back(gatewayInfo);
    mListGateways.setParam(listGateways);
    pGatewayElement = CAmGatewayFactory::createElement(gatewayInfo, mpCAmControlReceive);

    systemConfiguration.name = "SystemProp3";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    pSystem                   = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    mpCAmSystemActionRegister = new CAmSystemActionRegister(pSystem);

    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SOURCE_INFO, &mListSources);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SINK_INFO, &mListSinks);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_GATEWAY_INFO, &mListGateways);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    EXPECT_EQ(mpCAmSystemActionRegister->execute(), E_OK);
}

/**
 * @brief : Test to verify the _execute functions with incorrect class details
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive,CAmDomainElement,CAmRouteElement classes,
 *        Source, sink, domain, route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc.
 *        source,sink element details is enter in db for behavior set.
 *        and list of domain structure is created and details also push in that list about domain details.
 *
 * @test : verify the "_execute" function when class details are incorrect & domain element created & is enter in the db,class element is configured with all
 *         details like belonging source,sink name,topology,class property,registration type etc.
 *         & gateway element is created & configured and enter in db, CAmSystemActionRegister is initialized.
 *         & set the enterGatewayDB function to return the "E_OK" & sink,source, gateway action parameter is set for
 *         class CAmSystemActionRegister then will check whether _execute function based on parameter set invokes the
 *         routing manager API to register the element or not, without any gmock error.
 *
 * @result : "Pass" when _execute function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmSystemActionRegisterTest, executeNegativeClass)
{
    // Class Element
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 79;
    sinkClass.name                  = "AnySinkClass9";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 80;
    sourceClass.name                = "AnySourceClass9";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classProperty.classProperty = CP_PER_SINK_CLASS_VOLUME_SUPPORT;
    classProperty.value         = 10;
    listClassProperties.push_back(classProperty);
    // source Element belongs to class
    classElement.name                = "AnyClass9";
    classElement.type                = C_PLAYBACK;
    classElement.priority            = 1;
    classElement.listClassProperties = listClassProperties;
    // sink Element belongs to class
    classElement.name     = "AnyClass9";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(79), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(80), Return(E_OK)));
    pClassElement    = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    connectionformat = CF_GENIVI_STEREO;
    listConnectionFormats.push_back(connectionformat);
    gateway.name                  = "AnyGateway5";
    gateway.gatewayID             = 31;
    gatewayInfo.sinkName          = "AnySink9";    // Gateway Sinkname
    gatewayInfo.sourceName        = "AnySource9";  // Gateway Sourcename
    gatewayInfo.controlDomainName = "AnyDomain13"; // Gateway controlling Domainname
    listGateways.push_back(gatewayInfo);
    mListGateways.setParam(listGateways);
    pGatewayElement = CAmGatewayFactory::createElement(gatewayInfo, mpCAmControlReceive);

    systemConfiguration.name = "SystemProp4";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    pSystem                   = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    mpCAmSystemActionRegister = new CAmSystemActionRegister(pSystem);

    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SOURCE_INFO, &mListSources);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SINK_INFO, &mListSinks);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_GATEWAY_INFO, &mListGateways);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    EXPECT_EQ(mpCAmSystemActionRegister->execute(), E_OK);
}

/**
 * @brief : Test to verify the _execute functions for with sink details incorrect
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive,CAmDomainElement,CAmRouteElement classes,
 *        Source, sink, domain, route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc.
 *        source,sink element details is enter in db for behavior set.
 *        and list of domain structure is created and details also push in that list about domain details.
 *
 * @test : verify the "_execute" function when sink details are incorrect & domain element created & is enter in the db,class element is configured with all
 *         details like belonging source,sink name,topology,class property,registration type etc.
 *         & gateway element is created & configured and enter in db, CAmSystemActionRegister is initialized.
 *         & set the enterGatewayDB function to return the "E_OK" & sink,source, gateway action parameter is set for
 *         class CAmSystemActionRegister then will check whether _execute function based on parameter set invokes the
 *         routing manager API to register the element or not, without any gmock error.
 *
 * @result : "Pass" when _execute function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmSystemActionRegisterTest, executeNegativeSink)
{
    for (uint16_t i = 1; i < 5; i++)
    {
        domain.domainID = i;
        listDomains.push_back(domain);
        sink.sinkID   = i;
        sink.domainID = i;
        listSinks.push_back(sinkInfo);
        mListSinks.setParam(listSinks);
        source.sourceID = i;
        source.domainID = i;
        listSources.push_back(sourceInfo);
        mListSources.setParam(listSources);
        gateway.controlDomainID = i;
        listGateways.push_back(gatewayInfo);
        mListGateways.setParam(listGateways);
    }

    systemConfiguration.name = "SystemProp5";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    pSystem                   = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    mpCAmSystemActionRegister = new CAmSystemActionRegister(pSystem);

    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SOURCE_INFO, &mListSources);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_SINK_INFO, &mListSinks);
    mpCAmSystemActionRegister->setParam(ACTION_PARAM_GATEWAY_INFO, &mListGateways);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_NOT_POSSIBLE)));
    EXPECT_EQ(mpCAmSystemActionRegister->execute(), E_OK);
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
