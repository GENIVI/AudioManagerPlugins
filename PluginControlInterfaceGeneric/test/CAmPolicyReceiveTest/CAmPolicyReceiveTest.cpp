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
#include "CAmGatewayElement.h"
#include "CAmRouteElement.h"
#include "CAmMainConnectionElement.h"
#include "CAmPolicyReceive.h"
#include "CAmActionConnect.h"
#include "IAmPolicySend.h"
#include "MockIAmControlReceive.h"
#include "MockIAmPolicySend.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmPolicyReceiveTest.h"
#include "CAmSystemElement.h"
#include "stdio.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmPolicyReceiveTest
 *@brief : This class is used to test the CAmPolicyReceive functionality.
 */
CAmPolicyReceiveTest::CAmPolicyReceiveTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmSinkElement(nullptr)
    , mpCAmSourceElement(nullptr)
    , mpCAmRouteElement(NULL)
    , mpMockIAmPolicySend(NULL)
    , mpCAmPolicyReceive(NULL)
    , mpCAmDomainElement(nullptr)
    , domainInfo("AnyDomain1")
    , domainInfo1("AnyDomain2")
    , mpCAmClassElement(nullptr)
    , mpCAmMainConnectionElement(nullptr)
    , mpCAmGatewayElement(nullptr)
    , mpCAmClassActionConnect(NULL)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
{
}

CAmPolicyReceiveTest::~CAmPolicyReceiveTest()
{
}

void CAmPolicyReceiveTest::InitializeCommonStruct()
{
    sink.name                   = "AnySink1";
    source.name                 = "AnySource1";
    sourceID                    = 0;
    sink.domainID               = 25;
    source.domainID             = 26;
    domain.name                 = "AnyDomain1";
    domain.domainID             = 22;
    domainInfo.busname          = "AnyBus";
    domainInfo.nodename         = "AnyNode";
    domainInfo.early            = true;
    domainInfo.complete         = false;
    domainInfo.state            = DS_UNKNOWN;
    domainInfo.registrationType = REG_CONTROLLER;
    listDomains.push_back(domain);

    domain1.name = "AnyDomain2";
    // domain1.domainID             = 26;
    // domainInfo1.busname          = "AnyBus";
    // domainInfo1.nodename         = "AnyNode";
    // domainInfo1.early            = true;
    // domainInfo1.complete         = false;
    // domainInfo1.state            = DS_UNKNOWN;
    // domainInfo1.registrationType = REG_CONTROLLER;
    // listDomains.push_back(domain1);
}

void CAmPolicyReceiveTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;
    // This will check whether controller is initialized or not
    ASSERT_THAT(mpCAmControlReceive, NotNull()) << " Controller Not Initialized";
    InitializeCommonStruct();
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(DoAll(SetArgReferee < 1 > (22), Return(E_OK)));
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    source.name = "AnySource1";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(DoAll(SetArgReferee < 1 > (22), Return(E_OK)));
    mpCAmSourceElement = CAmSourceFactory::createElement(sourceInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSourceElement, NotNull()) << " Source Element Is not Created";
    sourceIDList.push_back(mpCAmSourceElement->getID());
    sourceID = mpCAmSourceElement->getID();

    mpMockIAmPolicySend = new MockIAmPolicySend();
    mpCAmPolicyReceive  = new gc::CAmPolicyReceive(mpCAmControlReceive, mpMockIAmPolicySend);

    // mpCAmDomainElement = CAmDomainFactory::getElement(domain.name);
    // mpCAmDomainElement = new CAmDomainElement(domainInfo, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(IsDomainNamePresent(domain), _)).WillOnce(DoAll(SetArgReferee<1>(25), Return(E_OK)));

    mpCAmDomainElement = CAmDomainFactory::createElement(domainInfo, mpCAmControlReceive);

    mpCAmDomainElement->setID(25);
    //
    // domainID = mpCAmDomainElement->getID();
    //// creating the routing element
    gcRoutingElement.name             = mpCAmSourceElement->getName() + mpCAmSinkElement->getName();
    gcRoutingElement.sourceID         = mpCAmSourceElement->getID();
    gcRoutingElement.sinkID           = mpCAmSinkElement->getID();
    gcRoutingElement.domainID         = 2;
    gcRoutingElement.connectionFormat = CF_GENIVI_STEREO;
    mpCAmRouteElement                 = new CAmRouteElement(gcRoutingElement, mpCAmControlReceive);
    ASSERT_THAT(mpCAmRouteElement, NotNull()) << " Route Element Is not Created";
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(IsDomainNamePresent(domain), _)).WillOnce(DoAll(SetArgReferee<1>(25), Return(E_OK)));
    // mpCAmDomainElement = CAmDomainFactory::createElement(domainInfo, mpCAmControlReceive);
    // mpCAmDomainElement->setID(25);
    listDomains.push_back(domain);

    // EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(IsDomainNamePresent(domain1), _)).WillOnce(DoAll(SetArgReferee<1>(26), Return(E_OK)));
    // mpCAmDomainElement1 = CAmDomainFactory::createElement(domainInfo1, mpCAmControlReceive);
    // listDomains.push_back(domain1);

}

void CAmPolicyReceiveTest::TearDown()
{
    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();
    // listDomains.clear();
    CAmDomainFactory::destroyElement();

    if (mpCAmRouteElement != NULL)
    {
        delete (mpCAmRouteElement);
    }

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }

    if (mpMockIAmPolicySend != NULL)
    {
        delete (mpMockIAmPolicySend);
    }
}

string CAmPolicyReceiveTest::int2string(int x)
{
    value = std::to_string(x);
    return value;
}

/**
 * @brief :  Test to verify the isDomainRegistrationComplete function Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isDomainRegistrationComplete" function when set the behavior of getListDomains function
 *         to return the "E_OK",  and also the domain details is pass as input parameter along with other configuration details
 *         then will check whether isDomainRegistrationComplete function
 *         providing the interface to policy engine to check if domain registration is completed or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isDomainRegistrationComplete function return the "nothing" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsDomainRegistrationCompletePoistive)
{
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    EXPECT_NO_THROW(mpCAmPolicyReceive->isDomainRegistrationComplete("AnyDomain1"));
}

/**
 * @brief :  Test to verify the isDomainRegistrationComplete function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isDomainRegistrationComplete" function when set the behavior of getListDomains function
 *         to return the "E_OK",  and also the domain details is pass as input parameter along with other configuration details
 *         but domain name is not correct one then will check whether isDomainRegistrationComplete function
 *         providing the interface to policy engine to check if domain registration is completed or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isDomainRegistrationComplete function return the "False" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsDomainRegistrationCompleteNegativeDomainNamenotpresent)
{
    string data;
    for (uint16_t i = 1; i < 5; i++)
    {
        domain.domainID = i;
        data            = CAmPolicyReceiveTest::int2string(i);
        domain.name     = "AnyDomain" + data;
        listDomains.push_back(domain);
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    EXPECT_FALSE(mpCAmPolicyReceive->isDomainRegistrationComplete("AnyDomain253"));
}

/**
 * @brief :  Test to verify the isDomainRegistrationComplete function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isDomainRegistrationComplete" function when set the behavior of getListDomains function
 *         to return the "E_NOT_POSSIBLE", and also the domain details is pass as input parameter along with
 *         other configuration details then will check whether isDomainRegistrationComplete function
 *         providing the interface to policy engine to check if domain registration is completed or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isDomainRegistrationComplete function return the "False" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsDomainRegistrationCompleteNegativeDomainListnotupdated)
{
    string data;
    for (uint16_t i = 1; i < 5; i++)
    {
        domain.domainID = i;
        data            = CAmPolicyReceiveTest::int2string(i);
        domain.name     = "AnyDomain" + data;
        listDomains.push_back(domain);
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_NOT_POSSIBLE)));
    EXPECT_FALSE(mpCAmPolicyReceive->isDomainRegistrationComplete("AnyDomain1"));
}

/**
 * @brief :  Test to verify the isRegistered function for sink Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when element type and name details are pass as input parameter
 *         along with other configuration details to isRegistered function  then will check whether isRegistered function
 *         providing the interface to policy engine to check if element is registered or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "true" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredSinkElementPositive)
{
    EXPECT_TRUE(mpCAmPolicyReceive->isRegistered(ET_SINK, "AnySink1"));
}

/**
 * @brief :  Test to verify the isRegistered function for sink Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when element type and name details are pass as input parameter
 *         along with other configuration details to isRegistered function  then will check whether isRegistered function
 *         providing the interface to policy engine to check if element is registered or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "false" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredSinkElementNegative)
{
    EXPECT_FALSE(mpCAmPolicyReceive->isRegistered(ET_SINK, "AnySink123"));
}

/**
 * @brief :  Test to verify the isRegistered function for source Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when element type and name details are pass as input parameter
 *         along with other configuration details to isRegistered function  then will check whether isRegistered function
 *         providing the interface to policy engine to check if element is registered or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "true" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredSourceElementPositive)
{
    EXPECT_TRUE(mpCAmPolicyReceive->isRegistered(ET_SOURCE, "AnySource1"));

}

/**
 * @brief :  Test to verify the isRegistered function for source Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when element type and name details are pass as input parameter
 *         along with other configuration details to isRegistered function  then will check whether isRegistered function
 *         providing the interface to policy engine to check if element is registered or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "false" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredSourceElementNegative)
{
    EXPECT_FALSE(mpCAmPolicyReceive->isRegistered(ET_SOURCE, "AnySource236"));
}

/**
 * @brief :  Test to verify the isRegistered function for domain Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when element type and name details are pass as input parameter
 *         along with other configuration details to isRegistered function  then will check whether isRegistered function
 *         providing the interface to policy engine to check if element is registered or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "true" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredDomainElementPositive)
{
    if (mpCAmDomainElement != NULL)
    {
        EXPECT_TRUE(mpCAmPolicyReceive->isRegistered(ET_DOMAIN, "AnyDomain1"));
    }
}

/**
 * @brief :  Test to verify the isRegistered function for domain Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when element type and name details are pass as input parameter
 *         along with other configuration details to isRegistered function  then will check whether isRegistered function
 *         providing the interface to policy engine to check if element is registered or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "false" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredDomainElementNegative)
{
    if (NULL != mpCAmDomainElement)
    {
        EXPECT_FALSE(mpCAmPolicyReceive->isRegistered(ET_DOMAIN, "AnyDomain5"));
    }
}

/**
 * @brief :  Test to verify the isRegistered function for class Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when class element is created and set the behavior of enterSinkClassDB,
 *         enterSourceClassDB function to return the "E_OK" & also element type and name details are pass as input
 *         parameter along with other configuration details to isRegistered function then will check whether isRegistered function
 *         providing the interface to policy engine to check if element is registered or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "true" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredClassElementPositive)
{
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
    classElement.listTopologies.push_back(GClistTopolologyElement);
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(72), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(73), Return(E_OK)));
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    EXPECT_TRUE(mpCAmPolicyReceive->isRegistered(ET_CLASS, "AnyClass1"));
}

/**
 * @brief :  Test to verify the isRegistered function for class Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when class element is created and set the behavior of enterSinkClassDB,
 *         enterSourceClassDB function to return the "E_OK" & also element type and incorrect name details are pass as input
 *         parameter along with other configuration details to isRegistered function then will check whether isRegistered function
 *         providing the interface to policy engine to check if element is registered or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "false" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredClassElementNegative)
{
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass3";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass3";
    sourceClass.listClassProperties.push_back(classPropertyData);

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class
    classElement.name     = "AnyClass2";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(DoAll(SetArgReferee<1>(72), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(DoAll(SetArgReferee<0>(73), Return(E_OK)));
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    EXPECT_FALSE(mpCAmPolicyReceive->isRegistered(ET_CLASS, "AnyClass256"));
}

/**
 * @brief :  Test to verify the isRegistered function with Main Connection for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when route,main connection element are created and set the behavior of
 *         enterMainConnectionDB function to return the "E_OK" & also element type and route name are pass as input
 *         parameter along with other configuration details to isRegistered function then will check whether isRegistered function
 *         providing the interface to policy engine to check if element is registered or not
 *         ,without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "true" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredMainConnectionElementPositive)
{

    handle.handleType               = H_CONNECT;
    handle.handle                   = 50;
    handleSetSourceState.handleType = H_SETSOURCESTATE;
    handleSetSourceState.handle     = 50;
    am_connectionID_t connectionID  = 10;
    am_connectionID_t connectionID2 = 0;
    int               state         = 0;
    int               result        = 0;
    route1.sinkID   = mpCAmSinkElement->getID();
    route1.sourceID = mpCAmSourceElement->getID();
    route1.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    // Routing element information
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.sourceID         = mpCAmSourceElement->getID();
    route1.route.push_back(routingElement);
    // Main Connection Inforamation
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // out Paramerter
    source_out1.sourceState = SS_UNKNNOWN;
    source_out2.sourceState = SS_ON;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(44), Return(E_OK)));
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(route1, mpCAmControlReceive);
    EXPECT_TRUE(mpCAmPolicyReceive->isRegistered(ET_CONNECTION, route1.name));
}

/**
 * @brief :  Test to verify the isRegistered function with MainConnectionElement for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when route,main connection element are created and element type
 *         and route name are passed as input parameter along with other configuration details to isRegistered function
 *         then will check whether isRegistered function providing the interface to policy engine to check
 *         if element is registered or not, without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "false" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredMainConnectionElemenNegative)
{
    handle.handleType               = H_CONNECT;
    handle.handle                   = 50;
    handleSetSourceState.handleType = H_SETSOURCESTATE;
    handleSetSourceState.handle     = 50;
    am_connectionID_t connectionID  = 10;
    am_connectionID_t connectionID2 = 0;
    int               state         = 0;
    int               result        = 0;
    route1.sinkID   = mpCAmSinkElement->getID();
    route1.sourceID = mpCAmSourceElement->getID();
    // Routing Element information
    route1.name                     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.sourceID         = mpCAmSourceElement->getID();
    route1.route.push_back(routingElement);
    // MainConnection information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // out Parameter
    source_out1.sourceState    = SS_UNKNNOWN;
    source_out2.sourceState    = SS_ON;
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(route1, mpCAmControlReceive);
    EXPECT_FALSE(mpCAmPolicyReceive->isRegistered(ET_CONNECTION, "AnyConnection1"));
}

/**
 * @brief :  Test to verify the isRegistered function with GatewayElement for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when domain elements,gateway element are created and element type
 *         and gateway info are passed as input parameter along with other configuration details to isRegistered function
 *         then will check whether isRegistered function providing the interface to policy engine to check
 *         if element is registered or not, without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "true" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredGatewayElementPositive)
{
    EXPECT_CALL(*mpMockControlReceiveInterface, enterGatewayDB(IsGatewayNamePresent(gateway), _)).WillOnce(DoAll(SetArgReferee<1>(25), Return(E_OK)));
    mpCAmGatewayElement = CAmGatewayFactory::createElement(gatewayInfo, mpCAmControlReceive);
    if (NULL != mpCAmGatewayElement)
    {
        gatewayID = mpCAmGatewayElement->getID();
    }

    ASSERT_NO_THROW(mpCAmPolicyReceive->isRegistered(ET_GATEWAY, "AnyGateway1"));
}

/**
 * @brief : Test to verify the isRegistered function with GatewayElement for negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "isRegistered" function when domain elements,gateway element are created and element type
 *         and gateway info are passed as input parameter along with other configuration details to isRegistered function
 *         then will check whether isRegistered function providing the interface to policy engine to check
 *         if element is registered or not, without any Gmock error.
 *
 * @result : "Pass" when isRegistered function return the "false" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, IsRegisteredGatewayElementNegative)
{
    domain.name  = "AnyDomain8";
    domain1.name = "AnyDomain9";
    if (NULL != mpCAmDomainElement)
    {
        domainID = mpCAmDomainElement->getID();
    }

    if (NULL != mpCAmDomainElement1)
    {
        domainID1 = mpCAmDomainElement1->getID();
    }

    sinkInfo.domainName           = "AnyDomain6";
    sourceInfo.domainName         = "AnyDomain8";
    gateway.name                  = "AnyGateway2";
    gatewayInfo.sinkName          = "AnySink1";
    gatewayInfo.sourceName        = "AnySource1";
    gatewayInfo.controlDomainName = "AnyDomain8";
    gatewayID                     = 0;
    if (NULL != mpCAmControlReceive)
    {
    }

    mpCAmGatewayElement = CAmGatewayFactory::createElement(gatewayInfo, mpCAmControlReceive);
    if (NULL != mpCAmDomainElement)
    {
        gatewayID = mpCAmGatewayElement->getID();
    }

    EXPECT_FALSE(mpCAmPolicyReceive->isRegistered(ET_GATEWAY, "AnyGateway277"));
}

/**
 * @brief : Test to verify the getState function for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getState" function when set the behavior of setSourceState & getSourceInfoDB with parameter like
 *          sourceID, source state, handle etc to return the "E_OK".
 *          & also element type and Name and state info are passed as input parameter along with other configuration
 *          details to getState function then will check whether getState function providing the interface to policy
 *          engine to get the state of source or not, without any Gmock error.
 *
 * @result : "Pass" when getState function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, GetStateSourceElementpositive)
{
    source.sourceState = SS_ON;
    handle.handleType  = H_SETSOURCESTATE;
    handle.handle      = 30;
    int state;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceState(_, sourceID, sourceState))
        .WillOnce(DoAll(SetArgReferee<0>(handle), Return(E_OK)));
    am_Handle_s  handle;
    mpCAmControlReceive->setSourceState(handle, sourceID, sourceState);
    // EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
    // EXPECT_EQ(E_OK, mpCAmPolicyReceive->getState(ET_SOURCE, "AnySource1", state));

}

/// **
// * @brief : Test to verify the getState function for Negative scenario
// *
// * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
// *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
// *        like name,domainID,class name, registration type,volume etc and enter in the db.
// *
// * @test : verify the "getState" function when not set the behavior of setSourceState & getSourceInfoDB with parameter like
// *          sourceID, source state, handle etc to return the "E_OK".
// *          & also element type and Name and state info are passed as input parameter along with other configuration
// *          details to getState function then will check whether getState function providing the interface to policy
// *          engine to get the state of source or not, without any Gmock error.
// *
// * @result : "Pass" when getState function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
// */
// TEST_F(CAmPolicyReceiveTest, GetStateSourceElementNegative)
// {
// source.name = "AnySource3";
// int state = 0;
// if (NULL != mpCAmSourceElement)
// {
// sourceID = mpCAmSourceElement->getID();
// }
//
//// mpCAmSourceElement->setState(state);
//// EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getState(ET_SOURCE, "AnySource3", state));
// }
//
/**
 * @brief : Test to verify the getState function of DomainElement for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getState" function when domain element is created and set the behavior of changDomainStateDB &
 *          getListDomains with parameter like domainID, state etc to return the "E_OK".
 *          & also element type and Name and state info are passed as input parameter along with other configuration
 *          details to getState function then will check whether getState function providing the interface to policy
 *          engine to get the state of domain or not, without any Gmock error.
 *
 * @result : "Pass" when getState function return the "E_OK" without any Gmock error meesage.
 */
// TEST_F(CAmPolicyReceiveTest, GetStateDomainElementpositive)
// {
// domain.name     = "AnyDomain2";
// domain.domainID = 10;
// listDomains.push_back(domain);
// state = DS_CONTROLLED;
// int stateDomain;
// mpCAmDomainElement = CAmDomainFactory::createElement(domainInfo, mpCAmControlReceive);
// if (NULL != mpCAmDomainElement)
// {
// domainID = mpCAmDomainElement->getID();
// }
//
// EXPECT_CALL(*mpMockControlReceiveInterface, changDomainStateDB(state, domainID)).WillOnce(Return(E_OK));
// mpCAmDomainElement->setState(state);
// EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
////This is api remove
// EXPECT_EQ(E_OK, mpCAmPolicyReceive->getState(ET_DOMAIN, "AnyDomain2", stateDomain));
// }

/**
 * @brief : Test to verify the getState function of DomainElement for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getState" function when domain element is created and not set the behavior of changDomainStateDB.
 *         Element type and Name and state info are passed as input parameter along with other configuration
 *         details to getState function then will check whether getState function providing the interface to policy
 *         engine to get the state of domain or not, without any Gmock error.
 *
 * @result : "Pass" when getState function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
// TEST_F(CAmPolicyReceiveTest, GetStateDomainElementNegative)
// {
// domain.name     = "AnyDomain11";
// domain.domainID = 12;
// listDomains.push_back(domain);
// state = DS_CONTROLLED;
// int stateDomain;
// if (NULL != mpCAmDomainElement)
// {
// domainID = mpCAmDomainElement->getID();
// }
//
//// EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getState(ET_DOMAIN, "AnyDomain101", stateDomain));
// }

/**
 * @brief : Test to verify the getInterruptState function for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getInterruptState" function when set the behavior of setInterruptState with parameter like
 *          interruptS to return the "E_OK" and also element type and Name and interruptSt info are passed as input
 *          parameter along with other configuration details to getInterruptState function then will check
 *          whether getInterruptState function providing the interface to policy engine to get the interrupt state
 *          of source or not, without any Gmock error.
 *
 * @result : "Pass" when getInterruptState function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, GetinterruptStateSourceElementpositive)
{
    source.name = "AnySource1";
    am_sourceID_t       sourceID    = 0;
    am_InterruptState_e interruptSt = IS_INTERRUPTED;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    mpCAmSourceElement->setInterruptState(interruptSt);
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getInterruptState(ET_SOURCE, "AnySource1", interruptSt));
}

/**
 * @brief : Test to verify the getInterruptState function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getInterruptState" function when not set the behavior of setInterruptState with parameter like
 *          interruptS to return the "E_OK" and also element type and Name and interruptSt info are passed as input
 *          parameter along with other configuration details to getInterruptState function then will check
 *          whether getInterruptState function providing the interface to policy engine to get the interrupt state
 *          of source or not, without any Gmock error.
 *
 * @result : "Pass" when getInterruptState function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, GetinterruptStateSourceElementNegative)
{
    source.name = "AnySource10";
    am_InterruptState_e interruptSt = IS_INTERRUPTED;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getInterruptState(ET_SOURCE, "AnySource10", interruptSt));
}

/**
 * @brief : Test to verify the getMuteState function for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getMuteState" function when set the behavior of changeSinkMuteStateDB with parameter like
 *         muteState as "MS_UNMUTED", sinkID to return the "E_OK" and also element type and Name and interruptSt info are passed as input
 *         parameter along with other configuration details to getMuteState function then will check
 *         whether getMuteState function providing the interface to policy engine to get the mute state
 *         of sink or not, without any Gmock error.
 *
 * @result : "Pass" when getMuteState function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getMuteStateSinkPositive)
{
    am_MuteState_e muteState = MS_UNMUTED;
    sink.name = "AnySink1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    //EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkMuteStateDB(muteState, sinkID)).WillOnce(Return(E_OK));
    mpCAmSinkElement->setMuteState(muteState);
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getMuteState(ET_SINK, "AnySink1", muteState));
}

/**
 * @brief : Test to verify the getMuteState function for negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getMuteState" function when not set the behavior of changeSinkMuteStateDB with parameter like
 *         muteState as "MS_MUTED", sinkID to return the "E_OK" and also element type and Name and interruptSt info are passed as input
 *         parameter along with other configuration details to getMuteState function then will check
 *         whether getMuteState function providing the interface to policy engine to get the mute state
 *         of sink or not, without any Gmock error.
 *
 * @result : "Pass" when getMuteState function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getMuteStateSinkElementNullNegative)
{
    am_MuteState_e muteState = MS_MUTED;
    sink.name = "AnySink3";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getMuteState(ET_SINK, "AnySink3", muteState));
}

/**
 * @brief : Test to verify the getSoundProperty function for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getSoundProperty" function when set the behavior of getMainSinkSoundPropertyValue,getMainSinkSoundPropertyValue
 *         with parameter like sinkID, propertyType,propertyValue etc to return the "E_OK" and also element type and propertyType
 *         and propertyValue info are passed as input parameter along with other configuration details to getSoundProperty function
 *         then will check whether getSoundProperty function providing the interface to policy engine to get the sound property value
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getSoundProperty function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getSoundPropertySinkPositive)
{
    sink.name = "AnySink1";
    am_CustomSoundPropertyType_t propertyType = 10;
    int16_t                      propertyValue = -100;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkSoundPropertyValue(sinkID, propertyType, _))
        .WillOnce(DoAll(SetArgReferee<2>(20), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getSoundProperty(ET_SINK, "AnySink1", propertyType, propertyValue));
    EXPECT_EQ(20, propertyValue);
}

/**
 * @brief : Test to verify the getSoundProperty function for negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getSoundProperty" function when not set the behavior of getMainSinkSoundPropertyValue,getMainSinkSoundPropertyValue
 *         with parameter like sinkID, propertyType,propertyValue etc to return the "E_OK" and also element type and propertyType
 *         and propertyValue info are passed as input parameter along with other configuration details to getSoundProperty function
 *         then will check whether getSoundProperty function providing the interface to policy engine to get the sound property value
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getSoundProperty function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getSoundPropertySinkNegative)
{
    sink.name = "AnySink11";
    am_CustomSoundPropertyType_t propertyType = 10;
    int16_t                      propertyValue;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getSoundProperty(ET_SINK, "AnySink11", propertyType, propertyValue));
}

/**
 * @brief : Test to verify the getSoundProperty function of source for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getSoundProperty" function when set the behavior of getMainSinkSoundPropertyValue
 *         with parameter like sourceID, propertyType,propertyValue etc to return the "E_OK" and also element type and propertyType
 *         and propertyValue info are passed as input parameter along with other configuration details to getSoundProperty function
 *         then will check whether getSoundProperty function providing the interface to policy engine to get the sound property value
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getSoundProperty function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getSoundPropertySourcePositive)
{
    source.name = "AnySource1";
    am_sourceID_t                sourceID = 0;
    am_Source_s                  source;
    am_CustomSoundPropertyType_t propertyType = 10;
    int16_t                      propertyValue = 77;

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceSoundPropertyValue(sourceID, propertyType, _))
        .WillOnce(DoAll(SetArgReferee<2>(20), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getSoundProperty(ET_SOURCE, "AnySource1", propertyType, propertyValue));
    EXPECT_EQ(20, propertyValue);
}

/**
 * @brief : Test to verify the getSoundProperty function of source for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getSoundProperty" function when element type and propertyType
 *         and propertyValue info are passed as input parameter only to getSoundProperty function
 *         then will check whether getSoundProperty function providing the interface to policy engine to get the sound property value
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getSoundProperty function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getSoundPropertySourceNegative)
{
    am_CustomSoundPropertyType_t propertyType = 10;
    int16_t                      propertyValue;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getSoundProperty(ET_SOURCE, "AnySource11", propertyType, propertyValue));
}

/**
 * @brief : Test to verify the getMainSoundProperty function of sink for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getMainSoundProperty" function when set the behavior of getMainSinkSoundPropertyValue
 *         with parameter like sinkID, propertyType etc to return the "E_OK" and also element type and propertyType
 *         and propertyValue info are passed as input parameter along with other configuration details to getMainSoundProperty function
 *         then will check whether getMainSoundProperty function providing the interface to policy engine to get the main sound property
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getMainSoundProperty function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getMainSoundPropertySinkPositive)
{

    sink.name = "AnySink1";
    am_CustomMainSoundPropertyType_t propertyType = 10;
    int16_t                          propertyValue;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainSinkSoundPropertyValue(sinkID, propertyType, _)).WillOnce(DoAll(SetArgReferee<2>(20), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getMainSoundProperty(ET_SINK, "AnySink1", propertyType, propertyValue));
}

/**
 * @brief : Test to verify the getMainSoundProperty function of sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getMainSoundProperty" function when not set the behavior of getMainSinkSoundPropertyValue
 *         with parameter like sinkID, propertyType etc to return the "E_OK" and also element type and propertyType
 *         and propertyValue info are passed as input parameter along with other configuration details to getMainSoundProperty function
 *         then will check whether getMainSoundProperty function providing the interface to policy engine to get the main sound property
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getMainSoundProperty function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getMainSoundPropertySinkNegative)
{
    sink.name = "AnySink5";
    am_CustomMainSoundPropertyType_t propertyType = 10;
    int16_t                          propertyValue;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getMainSoundProperty(ET_SINK, "AnySink5", propertyType, propertyValue));
}

/**
 * @brief : Test to verify the getMainSoundProperty function of source for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getMainSoundProperty" function when set the behavior of getMainSinkSoundPropertyValue
 *         with parameter like sourceID, propertyType etc to return the "E_OK" and also element type and propertyType
 *         and propertyValue info are passed as input parameter along with other configuration details to getMainSoundProperty function
 *         then will check whether getMainSoundProperty function providing the interface to policy engine to get the main sound property
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getMainSoundProperty function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getMainSoundPropertySourcePositive)
{
    source.name = "AnySource1";
    am_CustomMainSoundPropertyType_t propertyType = 10;
    int16_t                          propertyValue;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainSourceSoundPropertyValue(sourceID, propertyType, _)).WillOnce(DoAll(SetArgReferee<2>(21), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getMainSoundProperty(ET_SOURCE, "AnySource1", propertyType, propertyValue));

}

/**
 * @brief : Test to verify the getMainSoundProperty function of source for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getMainSoundProperty" function when not set the behavior of getMainSinkSoundPropertyValue
 *         with parameter like sourceID, propertyType etc to return the "E_OK" and also element type and propertyType
 *         and propertyValue info are passed as input parameter along with other configuration details to getMainSoundProperty function
 *         then will check whether getMainSoundProperty function providing the interface to policy engine to get the main sound property
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getMainSoundProperty function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getMainSoundPropertySourceNegative)
{
    source.name = "AnySource12";
    am_CustomMainSoundPropertyType_t propertyType = 10;
    int16_t                          propertyValue;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getMainSoundProperty(ET_SOURCE, "AnySource12", propertyType, propertyValue));
}

/**
 * @brief : Test to verify the getAvailability function of source for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getAvailability" function when set the behavior of changeSourceAvailabilityDB,getSourceInfoDB
 *         with parameter like source, sourceID , availability etc to return the "E_OK" and also element type and propertyType
 *         and propertyValue info are passed as input parameter along with other configuration details to getAvailability function
 *         then will check whether getAvailability function providing the interface to policy engine to get the availability of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getAvailability function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getAvailabilitySourcePositive)
{
    source.name = "AnySource1";
    // for assigning data to Availability structure
    available.availability              = A_AVAILABLE;
    available.availabilityReason        = AR_GENIVI_NEWMEDIA;
    source.available.availability       = A_AVAILABLE;
    source.available.availabilityReason = AR_GENIVI_NEWMEDIA;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, changeSourceAvailabilityDB(setAvailable(available), sourceID)).WillOnce(Return(E_OK));
    mpCAmSourceElement->setAvailability(available);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getAvailability(ET_SOURCE, "AnySource1", available));
}

/**
 * @brief : Test to verify the getAvailability function of source for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getAvailability" function when not set the behavior of changeSourceAvailabilityDB,getSourceInfoDB
 *         with parameter like source, sourceID , availability etc to return the "E_OK" and also element type and propertyType
 *         and propertyValue info are passed as input parameter along with other configuration details to getAvailability function
 *         then will check whether getAvailability function providing the interface to policy engine to get the availability of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getAvailability function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getAvailabilitySourceNegative)
{
    source.name                         = "AnySource8";
    available.availability              = A_AVAILABLE;
    available.availabilityReason        = AR_GENIVI_NEWMEDIA;
    source.available.availability       = A_AVAILABLE;
    source.available.availabilityReason = AR_GENIVI_NEWMEDIA;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getAvailability(ET_SOURCE, "AnySource8", available));
}

/**
 * @brief : Test to verify the getAvailability function of sink for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getAvailability" function when set the behavior of changeSinkAvailabilityDB,getSinkInfoDB
 *         with parameter like sink name,ID, availability etc to return the "E_OK" and also element type and propertyType
 *         and propertyValue info are passed as input parameter along with other configuration details to getAvailability function
 *         then will check whether getAvailability function providing the interface to policy engine to get the availability of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getAvailability function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getAvailabilitySinkPositive)
{
    sink.name = "AnySink1";
    // for assigning data to Availability structure
    available.availability            = A_AVAILABLE;
    available.availabilityReason      = AR_GENIVI_NEWMEDIA;
    sink.available.availability       = A_AVAILABLE;
    sink.available.availabilityReason = AR_GENIVI_NEWMEDIA;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkAvailabilityDB(setAvailable(available), sinkID)).WillOnce(Return(E_OK));
    mpCAmSinkElement->setAvailability(available);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sink), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getAvailability(ET_SINK, "AnySink1", available));
}

/**
 * @brief : Test to verify the getAvailability function of source for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getAvailability" function when element type and propertyType
 *         and propertyValue info are passed as input parameter along with other configuration details to getAvailability function
 *         then will check whether getAvailability function providing the interface to policy engine to get the availability of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getAvailability function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getAvailabilitySinkNegative)
{
    sink.name                         = "AnySink7";
    available.availability            = A_AVAILABLE;
    available.availabilityReason      = AR_GENIVI_NEWMEDIA;
    sink.available.availability       = A_AVAILABLE;
    sink.available.availabilityReason = AR_GENIVI_NEWMEDIA;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getAvailability(ET_SINK, "AnySink7", available));
}

/**
 * @brief : Test to verify the getVolume function of source for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getVolume" function when set the behavior of setSourceVolume,setSourceVolume
 *         with parameter like sourceID, LimitVolumeParam, RampTypeParam etc to return the "E_OK" and also element type and LimitVolumeParam
 *         info are passed as input parameter along with other configuration details to getVolume function
 *         then will check whether getVolume function providing the interface to policy engine to get the volume of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getVolume function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getVolumeSourcePositive)
{
    source.name       = "AnySource1";
    handle.handleType = H_SETSOURCEVOLUME;
    handle.handle     = 50;
    am_volume_t         LimitVolumeParam = 50;
    am_CustomRampType_t RampTypeParam    = RAMP_UNKNOWN;
    am_time_t           RampTimeParam    = 0;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceVolume(_, sourceID, LimitVolumeParam, RampTypeParam, RampTimeParam))
        .WillOnce(DoAll(SetArgReferee<0>(handle), Return(E_OK)));
    am_Handle_s handle;
    mpCAmControlReceive->setSourceVolume(handle, sourceID, LimitVolumeParam, RampTypeParam, RampTimeParam);
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getVolume(ET_SOURCE, "AnySource1", LimitVolumeParam));
}

/**
 * @brief : Test to verify the getVolume function of source for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getVolume" function when element type and LimitVolumeParam
 *         info are passed as input parameter along with other configuration details to getVolume function
 *         then will check whether getVolume function providing the interface to policy engine to get the volume of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getVolume function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getVolumeSourceNegative)
{
    source.name       = "AnySource15";
    handle.handleType = H_SETSOURCEVOLUME;
    handle.handle     = 50;
    am_volume_t         LimitVolumeParam = 50;
    am_CustomRampType_t RampTypeParam    = RAMP_UNKNOWN;
    am_time_t           RampTimeParam    = 0;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getVolume(ET_SOURCE, "AnySource15", LimitVolumeParam));
}

/**
 * @brief : Test to verify the getVolume function of sink for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getVolume" function when set the behavior of setSinkVolume,setSinkVolume
 *         with parameter like sinkID, LimitVolumeParam, RampTypeParam etc to return the "E_OK" and also element type and LimitVolumeParam
 *         info are passed as input parameter along with other configuration details to getVolume function
 *         then will check whether getVolume function providing the interface to policy engine to get the volume of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getVolume function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getVolumeSinkPositive)
{
    // Sink element
    sink.name         = "AnySink1";
    handle.handleType = H_SETSINKVOLUME;
    handle.handle     = 50;
    am_volume_t         LimitVolumeParam = 20;
    am_CustomRampType_t RampTypeParam    = RAMP_UNKNOWN;
    am_time_t           RampTimeParam    = 0;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkVolume(_, sinkID, LimitVolumeParam, RampTypeParam, RampTimeParam))
        .WillOnce(DoAll(SetArgReferee<0>(handle), Return(E_OK)));
    am_Handle_s handle;
    mpCAmControlReceive->setSinkVolume(handle, sinkID, LimitVolumeParam, RampTypeParam, RampTimeParam);
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getVolume(ET_SINK, "AnySink1", LimitVolumeParam));
}

/**
 * @brief : Test to verify the getVolume function of sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getVolume" function when element type and LimitVolumeParam, info are passed as input parameter
 *         along with other configuration details to getVolume function then will check whether getVolume function
 *         providing the interface to policy engine to get the volume of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getVolume function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getVolumeSinkNegative)
{
    sink.name         = "AnySink15";
    handle.handleType = H_SETSINKVOLUME;
    handle.handle     = 50;
    am_volume_t         LimitVolumeParam = 20;
    am_CustomRampType_t RampTypeParam    = RAMP_UNKNOWN;
    am_time_t           RampTimeParam    = 0;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getVolume(ET_SINK, "AnySink15", LimitVolumeParam));
}

/**
 * @brief : Test to verify the getMainVolume function of sink for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getMainVolume" function when set the behavior of changeSinkMainVolumeDB
 *         with parameter like sinkID, mainVolume etc to return the "E_OK" and also element type and mainVolume
 *         info are passed as input parameter along with other configuration details to getMainVolume function
 *         then will check whether getMainVolume function providing the interface to policy engine to get the main volume of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getMainVolume function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getMainVolumeSinkPositive)
{
    sink.name = "AnySink1";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    am_mainVolume_t mainVolume = 15;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkMainVolumeDB(mainVolume, sinkID)).WillOnce(Return(E_OK));
    mpCAmSinkElement->setMainVolume(mainVolume);
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getMainVolume(ET_SINK, "AnySink1", mainVolume));
}

/**
 * @brief : Test to verify the getMainVolume function of sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getMainVolume" function when not set the behavior of changeSinkMainVolumeDB
 *         with parameter like sinkID, mainVolume etc to return the "E_OK" and also element type and mainVolume
 *         info are passed as input parameter along with other configuration details to getMainVolume function
 *         then will check whether getMainVolume function providing the interface to policy engine to get the main volume of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getMainVolume function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getMainVolumeSinkNegative)
{
    sink.name = "AnySink10";
    am_mainVolume_t mainVolume;
    sink.mainVolume = 20;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getMainVolume(ET_SINK, "AnySink10", mainVolume));
}

/**
 * @brief : Test to verify the getSystemProperty function for positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getSystemProperty" function when set the behavior of getListSystemProperties, changeSystemPropertyDB
 *         with parameter like listSystemProperties, system property etc to return the "E_OK" and also systype an value
 *         info are passed as input parameter along with other configuration details to getSystemProperty function
 *         then will check whether getSystemProperty function providing the interface to policy engine to get the main volume of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getSystemProperty function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getSystemPropertyPositive)
{
    // SystemProperty creating
    systemproperty.type  = SYP_DEBUG;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    am_CustomSystemPropertyType_t systype = SYP_DEBUG;
    int16_t                       value;
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSystemPropertyDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    mpCAmControlReceive->changeSystemPropertyDB(systemproperty);
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getSystemProperty(systype, value));
}

/**
 * @brief : Test to verify the getSystemProperty function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getSystemProperty" function when set the behavior of getListSystemProperties, changeSystemPropertyDB
 *         with parameter like listSystemProperties, system property etc to return the "E_OK" and also incorrect systype and value
 *         info are passed as input parameter along with other configuration details to getSystemProperty function
 *         then will check whether getSystemProperty function providing the interface to policy engine to get the main volume of element
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getSystemProperty function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getSystemPropertyNegative)
{
    // SystemProperty creating
    systemproperty.type  = SYP_DEBUG + 1;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    am_CustomSystemPropertyType_t systype = 1;
    int16_t                       value;
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listSystemProperties), Return(E_OK)));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getSystemProperty(systype, value));
}

/**
 * @brief : Test to verify the getListMainConnections function with OrderHigh for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and order as "O_HIGH_PRIORITY"
 *         ,class name are passed as input parameter along with other configuration details to getListMainConnections function
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsPositiveOrderHigh)
{

    sink.name                       = "AnySink1";
    sinkInfo.priority               = 0;
    source.name                     = "AnySource1";
    sourceInfo.priority             = 0;
    handle.handleType               = H_CONNECT;
    handle.handle                   = 50;
    handleSetSourceState.handleType = H_SETSOURCESTATE;
    handleSetSourceState.handle     = 50;
    am_connectionID_t connectionID  = 10;
    am_connectionID_t connectionID2 = 0;
    int               state         = 0;
    int               result        = 0;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    route1.sinkID   = mpCAmSinkElement->getID();
    route1.sourceID = mpCAmSourceElement->getID();
    route1.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    classPropertyData.classProperty     = CP_GENIVI_SINK_TYPE;
    classPropertyData.value             = 1;
    sinkClass.sinkClassID               = 73;
    sinkClass.name                      = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 74;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Out Parameter
    source_out1.sourceState = SS_UNKNNOWN;
    source_out2.sourceState = SS_ON;
    classElement.name       = "AnyClass1";
    classElement.type       = C_PLAYBACK;
    classElement.priority   = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    ConnectionInfo.sourceName      = "AnySource1";
    ConnectionInfo.sinkName        = "AnySink1";
    ConnectionInfo.priority        = 1;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;

    ConnectionInfos.sourceName      = "AnySource1";
    ConnectionInfos.sinkName        = "AnySink1";
    ConnectionInfos.priority        = 2;
    ConnectionInfos.connectionState = CS_CONNECTED;
    ConnectionInfos.volume          = 20;
    listConnectionInfos.push_back(ConnectionInfo);
    listConnectionInfos.push_back(ConnectionInfos);
    gc_Order_e order = O_HIGH_PRIORITY;
    mpCAmClassElement          = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(route1, mpCAmControlReceive);
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getListMainConnections("AnyClass1", listConnectionInfos, order));
}

/**
 * @brief : Test to verify the getListMainConnections function with OrderLow for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and order as low
 *         ,class name are passed as input parameter along with other configuration details to getListMainConnections function
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsPositiveOrderLow)
{
    sink.name                       = "AnySink1";
    sinkInfo.priority               = 0;
    source.name                     = "AnySource1";
    sourceInfo.priority             = 0;
    handle.handleType               = H_CONNECT;
    handle.handle                   = 50;
    handleSetSourceState.handleType = H_SETSOURCESTATE;
    handleSetSourceState.handle     = 50;
    am_connectionID_t connectionID  = 10;
    am_connectionID_t connectionID2 = 0;
    int               state         = 0;
    int               result        = 0;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    route1.sinkID   = mpCAmSinkElement->getID();
    route1.sourceID = mpCAmSourceElement->getID();
    route1.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);

    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    classPropertyData.classProperty     = CP_GENIVI_SINK_TYPE;
    classPropertyData.value             = 1;
    sinkClass.sinkClassID               = 73;
    sinkClass.name                      = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 74;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Out Parameter
    source_out1.sourceState = SS_UNKNNOWN;
    source_out2.sourceState = SS_ON;

    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    ConnectionInfo.sourceName      = "AnySource1";
    ConnectionInfo.sinkName        = "AnySink1";
    ConnectionInfo.priority        = 1;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;

    ConnectionInfos.sourceName      = "AnySource1";
    ConnectionInfos.sinkName        = "AnySink1";
    ConnectionInfos.priority        = 2;
    ConnectionInfos.connectionState = CS_CONNECTED;
    ConnectionInfos.volume          = 20;
    listConnectionInfos.push_back(ConnectionInfo);
    listConnectionInfos.push_back(ConnectionInfos);
    gc_Order_e order = O_LOW_PRIORITY;
    mpCAmClassElement          = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(route1, mpCAmControlReceive);

    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getListMainConnections("AnyClass1", listConnectionInfos, order));
}

/**
 * @brief : Test to verify the getListMainConnections function with OrderNew for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and order as "O_NEWEST"
 *         ,class name are passed as input parameter along with other configuration details to getListMainConnections function
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsPositiveOrderNew)
{
    sink.name                       = "AnySink1";
    sinkInfo.priority               = 0;
    source.name                     = "AnySource1";
    sourceInfo.priority             = 0;
    handle.handleType               = H_CONNECT;
    handle.handle                   = 50;
    handleSetSourceState.handleType = H_SETSOURCESTATE;
    handleSetSourceState.handle     = 50;
    am_connectionID_t connectionID  = 10;
    am_connectionID_t connectionID2 = 0;
    int               state         = 0;
    int               result        = 0;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    route1.sinkID                   = mpCAmSinkElement->getID();
    route1.sourceID                 = mpCAmSourceElement->getID();
    route1.name                     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    classPropertyData.classProperty     = CP_GENIVI_SINK_TYPE;
    classPropertyData.value             = 1;
    sinkClass.sinkClassID               = 73;
    sinkClass.name                      = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 74;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Out Parameter
    source_out1.sourceState = SS_UNKNNOWN;
    source_out2.sourceState = SS_ON;

    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    ConnectionInfo.sourceName      = "AnySource1";
    ConnectionInfo.sinkName        = "AnySink1";
    ConnectionInfo.priority        = 1;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;

    ConnectionInfos.sourceName      = "AnySource1";
    ConnectionInfos.sinkName        = "AnySink1";
    ConnectionInfos.priority        = 2;
    ConnectionInfos.connectionState = CS_CONNECTED;
    ConnectionInfos.volume          = 20;

    listConnectionInfos.push_back(ConnectionInfo);
    listConnectionInfos.push_back(ConnectionInfos);
    gc_Order_e order = O_NEWEST;
    mpCAmClassElement          = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(route1, mpCAmControlReceive);

    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getListMainConnections("AnyClass1", listConnectionInfos, order));
}

/**
 * @brief : Test to verify the getListMainConnections function with OrderOld for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and order as "O_OLDEST"
 *         ,class name are passed as input parameter along with other configuration details to getListMainConnections function
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsPositiveOrderOld)
{
    sink.name                       = "AnySink1";
    sinkInfo.priority               = 0;
    source.name                     = "AnySource1";
    sourceInfo.priority             = 0;
    handle.handleType               = H_CONNECT;
    handle.handle                   = 50;
    handleSetSourceState.handleType = H_SETSOURCESTATE;
    handleSetSourceState.handle     = 50;
    am_connectionID_t connectionID  = 10;
    am_connectionID_t connectionID2 = 0;
    int               state         = 0;
    int               result        = 0;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    route1.sinkID                   = mpCAmSinkElement->getID();
    route1.sourceID                 = mpCAmSourceElement->getID();
    route1.name                     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    classPropertyData.classProperty     = CP_GENIVI_SINK_TYPE;
    classPropertyData.value             = 1;
    sinkClass.sinkClassID               = 73;
    sinkClass.name                      = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 74;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Out Parameter
    source_out1.sourceState = SS_UNKNNOWN;
    source_out2.sourceState = SS_ON;

    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    ConnectionInfo.sourceName       = "AnySource1";
    ConnectionInfo.sinkName         = "AnySink1";
    ConnectionInfo.priority         = 1;
    ConnectionInfo.connectionState  = CS_CONNECTED;
    ConnectionInfo.volume           = 10;
    ConnectionInfos.sourceName      = "AnySource1";
    ConnectionInfos.sinkName        = "AnySink1";
    ConnectionInfos.priority        = 2;
    ConnectionInfos.connectionState = CS_CONNECTED;
    ConnectionInfos.volume          = 20;
    listConnectionInfos.push_back(ConnectionInfo);
    listConnectionInfos.push_back(ConnectionInfos);
    gc_Order_e order = O_OLDEST;
    mpCAmClassElement          = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(route1, mpCAmControlReceive);

    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getListMainConnections("AnyClass1", listConnectionInfos, order));
}

/**
 * @brief : Test to verify the getListMainConnections function with Main connection as NULL for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list,set the main connection element as NULL
 *         and also listConnectionInfos and order,class name are passed as input parameter along with
 *         other configuration details to getListMainConnections function
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function will not throw any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsNegativeClassElementNull)
{
    sink.name                       = "AnySink1";
    sinkInfo.priority               = 0;
    source.name                     = "AnySource1";
    sourceInfo.priority             = 0;
    handle.handleType               = H_CONNECT;
    handle.handle                   = 50;
    handleSetSourceState.handleType = H_SETSOURCESTATE;
    handleSetSourceState.handle     = 50;
    am_connectionID_t connectionID  = 10;
    am_connectionID_t connectionID2 = 0;
    int               state         = 0;
    int               result        = 0;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    route1.sinkID                   = mpCAmSinkElement->getID();
    route1.sourceID                 = mpCAmSourceElement->getID();
    route1.name                     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    classPropertyData.classProperty     = CP_GENIVI_SINK_TYPE;
    classPropertyData.value             = 1;
    sinkClass.sinkClassID               = 73;
    sinkClass.name                      = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 74;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Out Parameter
    source_out1.sourceState = SS_UNKNNOWN;
    source_out2.sourceState = SS_ON;

    // source Element belongs to class
    classElement.name     = "AnyClass2";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class

    classElement.name     = "AnyClass2";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    ConnectionInfo.sourceName       = "AnySource1";
    ConnectionInfo.sinkName         = "AnySink1";
    ConnectionInfo.priority         = 1;
    ConnectionInfo.connectionState  = CS_CONNECTED;
    ConnectionInfo.volume           = 10;
    ConnectionInfos.sourceName      = "AnySource1";
    ConnectionInfos.sinkName        = "AnySink1";
    ConnectionInfos.priority        = 2;
    ConnectionInfos.connectionState = CS_CONNECTED;
    ConnectionInfos.volume          = 20;
    listConnectionInfos.push_back(ConnectionInfo);
    listConnectionInfos.push_back(ConnectionInfos);
    gc_Order_e order;
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(route1, mpCAmControlReceive);
    mpCAmMainConnectionElement = NULL;

    EXPECT_NO_THROW(mpCAmPolicyReceive->getListMainConnections("AnyClass1", listConnectionInfos, order));

}

/**
 * @brief : Test to verify the getListNotificationConfigurations function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListNotificationConfigurations" function when notification configuration details are push
 *         in the list of main notification configuration and set the behavior of getSourceInfoDB,setSourceNotificationConfiguration
 *         to return "E_OK" and also element type and listMainNotificationConfigurations and element name are passed as input parameter
 *         along with other configuration details to getListNotificationConfigurations function then will check whether
 *         getListNotificationConfigurations function providing the interface to policy engine to get the
 *         list of notification configurations involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListNotificationConfigurations function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListNotificationConfigurationsSourcePositive)
{
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    // Add above values to MainNotificationConfiguration Vector
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    source.name                               = "AnySource1";
    source.listMainNotificationConfigurations = listMainNotificationConfigurations;
    source.listMainNotificationConfigurations = listMainNotificationConfigurations;
    // output parameter source id and handle type and handle value
    handle.handleType = H_SETSOURCENOTIFICATION;
    handle.handle     = 40;
    am_Error_e result;
    int        result_state;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _))
        .WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceNotificationConfiguration(_, sourceID, setNotificationConfigurations(notificationConfigurations)))
        .WillOnce(DoAll(SetArgReferee<0>(handle), Return(E_OK)));
    am_Handle_s handle;
    mpCAmControlReceive->setSourceNotificationConfiguration(handle, sourceID, notificationConfigurations);
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getListNotificationConfigurations(ET_SOURCE, "AnySource1", listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the getListNotificationConfigurations function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListNotificationConfigurations" function when notification configuration details are push
 *         in the list of main notification configuration and not set the behavior of getSourceInfoDB,setSourceNotificationConfiguration
 *         to return "E_OK" and also element type and listMainNotificationConfigurations and element name are passed as input parameter
 *         along with other configuration details to getListNotificationConfigurations function then will check whether
 *         getListNotificationConfigurations function providing the interface to policy engine to get the
 *         list of notification configurations involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListNotificationConfigurations function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListNotificationConfigurationsSourceNegative)
{
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    // source element
    source.name                               = "AnySource10";
    source.listMainNotificationConfigurations = listMainNotificationConfigurations;
    handle.handleType                         = H_SETSOURCENOTIFICATION;
    handle.handle                             = 40;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getListNotificationConfigurations(ET_SOURCE, "AnySource10", listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the getListNotificationConfigurations function of sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListNotificationConfigurations" function when notification configuration details are push
 *         in the list of main notification configuration and set the behavior of getSinkInfoDB,setSinkNotificationConfiguration
 *         to return "E_OK" and also element type and listMainNotificationConfigurations and element name are passed as input parameter
 *         along with other configuration details to getListNotificationConfigurations function then will check whether
 *         getListNotificationConfigurations function providing the interface to policy engine to get the
 *         list of notification configurations involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListNotificationConfigurations function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListNotificationConfigurationsSinkPositive)
{
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    // Add above values to MainNotificationConfiguration Vector
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    sink.name                               = "AnySink1";
    sink.listMainNotificationConfigurations = listMainNotificationConfigurations;
    sink.listMainNotificationConfigurations = listMainNotificationConfigurations;
    handle.handleType                       = H_SETSINKNOTIFICATION;
    handle.handle                           = 40;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _))
        .WillOnce(DoAll(SetArgReferee<1>(sink), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkNotificationConfiguration(_, sinkID, setNotificationConfigurations(notificationConfigurations)))
        .WillOnce(DoAll(SetArgReferee<0>(handle), Return(E_OK)));
    am_Handle_s handle;
    mpCAmControlReceive->setSinkNotificationConfiguration(handle, sinkID, notificationConfigurations);

    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getListNotificationConfigurations(ET_SINK, "AnySink1", listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the getListNotificationConfigurations function for sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListNotificationConfigurations" function when notification configuration details are push
 *         in the list of main notification configuration and not set the behavior of getSinkInfoDB,setSinkNotificationConfiguration
 *         to return "E_OK" and also element type and listMainNotificationConfigurations and element name are passed as input parameter
 *         along with other configuration details to getListNotificationConfigurations function then will check whether
 *         getListNotificationConfigurations function providing the interface to policy engine to get the
 *         list of notification configurations involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListNotificationConfigurations function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListNotificationConfigurationsSinkNegative)
{
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    // Add above values to MainNotificationConfiguration Vector
    listMainNotificationConfigurations.push_back(notificationConfigurations);

    sink.name                               = "AnySink30";
    sink.listMainNotificationConfigurations = listMainNotificationConfigurations;
    // am_Handle_s handle;
    handle.handleType = H_SETSINKNOTIFICATION;
    handle.handle     = 40;

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getListNotificationConfigurations(ET_SINK, "AnySink30", listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the getListMainNotificationConfigurations function for source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainNotificationConfigurations" function when notification configuration details are push
 *         in the list of main notification configuration  and also push in source main notification configuration list,
 *         Set the behavior of getSourceInfoDB,changeMainSourceNotificationConfigurationDB to return "E_OK"
 *         and also element type and listMainNotificationConfigurations, element name are passed as input parameter
 *         along with other configuration details to getListMainNotificationConfigurations function then will check whether
 *         getListMainNotificationConfigurations function providing the interface to policy engine to get the
 *         list of main notification configurations involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListNotificationConfigurations function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainNotificationConfigurationsSourcePositive)
{
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    // Add above values to MainNotificationConfiguration Vector
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    // source element
    source.name                               = "AnySource29";
    source.listMainNotificationConfigurations = listMainNotificationConfigurations;
    // output parameter source id and handle type and handle value
    handle.handleType = H_SETSOURCENOTIFICATION;
    handle.handle     = 40;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSourceNotificationConfigurationDB(sourceID, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_OK));
    mpCAmSourceElement->setMainNotificationConfiguration(notificationConfigurations);
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getListMainNotificationConfigurations(ET_SOURCE, "AnySource1", listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the getListMainNotificationConfigurations function for source for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainNotificationConfigurations" function when notification configuration details are push
 *         in the list of main notification configuration  and also push in source main notification configuration list,
 *         Also element type and listMainNotificationConfigurations, element name are passed as input parameter
 *         along with other configuration details to getListMainNotificationConfigurations function then will check whether
 *         getListMainNotificationConfigurations function providing the interface to policy engine to get the
 *         list of main notification configurations involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListNotificationConfigurations function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainNotificationConfigurationsSourceNegative)
{
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    // Add above values to MainNotificationConfiguration Vector
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    // source element
    source.name                               = "AnySource31";
    source.listMainNotificationConfigurations = listMainNotificationConfigurations;
    handle.handleType                         = H_SETSOURCENOTIFICATION;
    handle.handle                             = 40;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getListMainNotificationConfigurations(ET_SOURCE, "AnySource31", listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the getListMainNotificationConfigurations function for sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainNotificationConfigurations" function when notification configuration details are push
 *         in the list of main notification configuration  and also push in sink main notification configuration list,
 *         Set the behavior of getSinkInfoDB,changeMainSinkNotificationConfigurationDB to return "E_OK"
 *         and also element type and listMainNotificationConfigurations, element name are passed as input parameter
 *         along with other configuration details to getListMainNotificationConfigurations function then will check whether
 *         getListMainNotificationConfigurations function providing the interface to policy engine to get the
 *         list of main notification configurations involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListNotificationConfigurations function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainNotificationConfigurationsSinkPositive)
{
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    // Add above values to MainNotificationConfiguration Vector
    listMainNotificationConfigurations.push_back(notificationConfigurations);

    sink.name                               = "AnySink1";
    sink.listMainNotificationConfigurations = listMainNotificationConfigurations;
    handle.handleType                       = H_SETSINKNOTIFICATION;
    handle.handle                           = 40;

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sink), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSinkNotificationConfigurationDB(sinkID, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_OK));
    mpCAmSinkElement->setMainNotificationConfiguration(notificationConfigurations);
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getListMainNotificationConfigurations(ET_SINK, "AnySink1", listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the getListMainNotificationConfigurations function for sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainNotificationConfigurations" function when notification configuration details are push
 *         in the list of main notification configuration  and also push in sink main notification configuration list,
 *         and also element type and listMainNotificationConfigurations, element name are passed as input parameter
 *         along with other configuration details to getListMainNotificationConfigurations function then will check whether
 *         getListMainNotificationConfigurations function providing the interface to policy engine to get the
 *         list of main notification configurations involving the given element or not, without any Gmock error.
 *
 * @result : "Pass" when getListNotificationConfigurations function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainNotificationConfigurationsSinkNegative)
{
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    // Add above values to MainNotificationConfiguration Vector
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    sink.name                               = "AnySink31";
    sink.listMainNotificationConfigurations = listMainNotificationConfigurations;
    // output parameter source id and handle type and handle value
    handle.handleType = H_SETSINKNOTIFICATION;
    handle.handle     = 40;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getListMainNotificationConfigurations(ET_SINK, "AnySink31", listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the getListMainConnections function for source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and element name
 *         and type are passed as input parameter along with other configuration details to getListMainConnections function
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given source element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsSourcePositive)
{
    source.name     = "AnySource1";
    sourceInfo.name = "AnySource1";
    sink.name       = "AnySink1";
    sinkInfo.name   = "AnySink1";
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
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

    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    // Routing Element Information
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Information
    route1.sinkID   = mpCAmSinkElement->getID();
    route1.sourceID = mpCAmSourceElement->getID();
    route1.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    // Main Conncetion Information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    // Out Parameter
    source_out1.sourceState            = SS_UNKNNOWN;
    source_out2.sourceState            = SS_ON;
    mainConnectionData.connectionState = CS_CONNECTED;

    // Connection information
    ConnectionInfo.sourceName      = "AnySource1";
    ConnectionInfo.sinkName        = "AnySink1";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;

    ConnectionInfos.sourceName      = "AnySource1";
    ConnectionInfos.sinkName        = "AnySink1";
    ConnectionInfos.priority        = 0;
    ConnectionInfos.connectionState = CS_CONNECTED;
    ConnectionInfos.volume          = 10;
    listConnectionInfos.push_back(ConnectionInfo);
    listConnectionInfos.push_back(ConnectionInfos);
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == mpCAmClassElement)
    {
        ASSERT_THAT(mpCAmClassElement, IsNull()) << " CAmClassElement is not Initialized";
    }

    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(route1, mpCAmControlReceive);
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getListMainConnections(ET_SOURCE, "AnySource1", listConnectionInfos));

}

/**
 * @brief : Test to verify the getListMainConnections function for source for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and incorrect element name
 *         and type are passed as input parameter along with other configuration details to getListMainConnections function
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given source element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsSourceNegative)
{
    // source element
    source.name                     = "AnySource1";
    sink.name                       = "AnySink1";
    sourceID                        = 0;
    sinkID                          = 0;
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass13";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass13";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element

    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class
    // gc_TopologyElement_s topologyElementsink;

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    // SystemProperty creating;
    systemproperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemproperty.value = 5;

    listSystemProperties.push_back(systemproperty);

    // System element
    systemConfiguration.name = "System7";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));

    std::shared_ptr<CAmSystemElement > mpCAmSystemElement;
    mpCAmSystemElement = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);

    // Creating Class Element instancce
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    // attaching the element with class element
    mpCAmClassElement->attach(mpCAmSourceElement);
    mpCAmClassElement->attach(mpCAmSinkElement);
    // Routing Element Information
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Information
    ////gc_Route_s gcRoute;
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    // Main Conncetion Information

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    // Out Parameter
    source_out1.sourceState            = SS_UNKNNOWN;
    source_out2.sourceState            = SS_ON;
    mainConnectionData.connectionState = CS_CONNECTED;

    // Connection information
    ConnectionInfo.sourceName      = "AnySource1";
    ConnectionInfo.sinkName        = "AnySink1";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;

    listConnectionInfos.push_back(ConnectionInfo);

    // creating the class connection element
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == mpCAmClassElement)
    {
        ASSERT_THAT(mpCAmClassElement, IsNull()) << " CAmClassElement is not Initialized";
    }

    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getListMainConnections(ET_SOURCE, "AnySource366", listConnectionInfos));

}

/**
 * @brief : Test to verify the getListMainConnections function for sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and element name
 *         and type are passed as input parameter along with other configuration details to getListMainConnections function
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given sink element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsSinkPositive)
{
    source.name = "AnySource1";
    sink.name   = "AnySink1";
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 20;
    sinkClass.name                  = "AnySinkClass10";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 21;
    sourceClass.name                = "AnySourceClass10";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element

    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Information
    route1.sinkID   = mpCAmSinkElement->getID();
    route1.sourceID = mpCAmSourceElement->getID();
    route1.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    // Main Conncetion Information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    // Out Parameter
    source_out1.sourceState            = SS_UNKNNOWN;
    source_out2.sourceState            = SS_ON;
    mainConnectionData.connectionState = CS_CONNECTED;
    // Connection information
    ConnectionInfo.sourceName      = "AnySource1";
    ConnectionInfo.sinkName        = "AnySink1";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;

    listConnectionInfos.push_back(ConnectionInfo);
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == mpCAmClassElement)
    {
        ASSERT_THAT(mpCAmClassElement, IsNull()) << " CAmClassElement is not Initialized";
    }

    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getListMainConnections(ET_SINK, "AnySink1", listConnectionInfos));
}

/**
 * @brief : Test to verify the getListMainConnections function for sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and incorrect element name
 *         and type are passed as input parameter along with other configuration details to getListMainConnections function
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given sink element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsSinkNegative)
{
    source.name     = "AnySource1";
    sourceInfo.name = "AnySource1";
    sink.name       = "AnySink1";
    sinkInfo.name   = "AnySink1";
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass14";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass14";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    // Routing Element Information
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Information
    route1.sinkID   = mpCAmSinkElement->getID();
    route1.sourceID = mpCAmSourceElement->getID();
    route1.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    // Main Conncetion Information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    // Out Parameter
    source_out1.sourceState            = SS_UNKNNOWN;
    source_out2.sourceState            = SS_ON;
    mainConnectionData.connectionState = CS_CONNECTED;
    // Connection information
    ConnectionInfo.sourceName      = "AnySource1";
    ConnectionInfo.sinkName        = "AnySink1";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;
    listConnectionInfos.push_back(ConnectionInfo);
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == mpCAmClassElement)
    {
        ASSERT_THAT(mpCAmClassElement, IsNull()) << " CAmClassElement is not Initialized";
    }

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getListMainConnections(ET_SINK, "AnySink37", listConnectionInfos));
}

/**
 * @brief : Test to verify the getListMainConnections function for class for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and element name
 *         and type are passed as input parameter along with other configuration details to getListMainConnections function
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given class element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsClassPositive)
{
    source.name = "AnySource1";
    sink.name   = "AnySink1";
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass11";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass11";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    // Routing Element Information
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Information
    route1.sinkID   = mpCAmSinkElement->getID();
    route1.sourceID = mpCAmSourceElement->getID();
    route1.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    // Main Conncetion Information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    // Out Parameter
    source_out1.sourceState            = SS_UNKNNOWN;
    source_out2.sourceState            = SS_ON;
    mainConnectionData.connectionState = CS_CONNECTED;
    // Connection information
    ConnectionInfo.sourceName      = "AnySource1";
    ConnectionInfo.sinkName        = "AnySink1";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;
    listConnectionInfos.push_back(ConnectionInfo);
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);
    if (NULL == mpCAmClassElement)
    {
        ASSERT_THAT(mpCAmClassElement, IsNull()) << " CAmClassElement is not Initialized";
    }

    EXPECT_EQ(E_OK, mpCAmPolicyReceive->getListMainConnections(ET_CLASS, "AnyClass1", listConnectionInfos));
}

/**
 * @brief : Test to verify the getListMainConnections function for class for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and incorrect element name
 *         and type are passed as input parameter along with other configuration details to getListMainConnections function
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given class element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsClassNegative)
{
    source.name = "AnySource1";
    sink.name   = "AnySink1";
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 74;
    sinkClass.name                  = "AnySinkClass15";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 75;
    sourceClass.name                = "AnySourceClass15";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element

    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Information
    route1.sinkID   = mpCAmSinkElement->getID();
    route1.sourceID = mpCAmSourceElement->getID();
    route1.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    // Main Conncetion Information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    // Out Parameter
    source_out1.sourceState            = SS_UNKNNOWN;
    source_out2.sourceState            = SS_ON;
    mainConnectionData.connectionState = CS_CONNECTED;
    // Connection information
    ConnectionInfo.sourceName      = "AnySource1";
    ConnectionInfo.sinkName        = "AnySink1";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;
    listConnectionInfos.push_back(ConnectionInfo);
    mpCAmClassElement = CAmClassFactory::createElement(classElement, mpCAmControlReceive);

    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getListMainConnections(ET_CLASS, "AnyClass11", listConnectionInfos));
}

/**
 * @brief : Test to verify the getListMainConnections function for main connection for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and route name
 *         and type are passed as input parameter along with other configuration details to getListMainConnections function
 *         and also set the behavior of the getMainConnectionInfoDB,getState to return E_OK
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given mian connection element or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function will not throw any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsMainConnectionPositive)
{
    source.name = "AnySource1";
    sink.name   = "AnySink1";
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass11";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass11";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    // Routing Element Information
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Information
    route1.sinkID   = mpCAmSinkElement->getID();
    route1.sourceID = mpCAmSourceElement->getID();
    route1.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    // Main Conncetion Information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    // Out Parameter
    source_out1.sourceState            = SS_UNKNNOWN;
    source_out2.sourceState            = SS_ON;
    mainConnectionData.connectionState = CS_CONNECTED;
    // Connection information
    ConnectionInfo.sourceName       = "AnySource1";
    ConnectionInfo.sinkName         = "AnySink1";
    ConnectionInfo.priority         = 0;
    ConnectionInfo.connectionState  = CS_CONNECTED;
    ConnectionInfo.volume           = 10;
    sink.name                       = "AnySink1";
    source.name                     = "AnySource1";
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    mpCAmMainConnectionElement          = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _)).WillOnce(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    listConnectionInfos.push_back(ConnectionInfo);
    EXPECT_NO_THROW(mpCAmPolicyReceive->getListMainConnections(ET_CONNECTION, gcRoute.name, listConnectionInfos));
}

/**
 * @brief : Test to verify the getListMainConnections function for main connection for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "getListMainConnections" function when sink,source,route,class elements are created
 *         and also push the connection info details in the list, and also listConnectionInfos and route name
 *         and type are passed as input parameter along with other configuration details to getListMainConnections function
 *         and also set the behavior of the getMainConnectionInfoDB,getState to return E_OK
 *         then will check whether getListMainConnections function providing the interface to policy engine to get the
 *         list of main connection involving the given main connection element or not, without any Gmock error.
 *
 *  @result : "Pass" when getListMainConnections function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, getListMainConnectionsMainConnectionNegative)
{

    source.name = "AnySource1";
    sink.name   = "AnySink1";
    am_sourceID_t sourceID = 0;
    am_sinkID_t   sinkID   = 0;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 74;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 75;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    // Class Element
    // source Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    route1.sinkID                   = mpCAmSinkElement->getID();
    route1.sourceID                 = mpCAmSourceElement->getID();
    route1.name                     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    route1.route.push_back(routingElement);
    amRoute.sourceID = mpCAmSourceElement->getID();
    amRoute.sinkID   = mpCAmSinkElement->getID();
    amRoute.route.push_back(routingElement);
    amRouteList.push_back(amRoute);
    // Main Conncetion Information
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    source_out1.sourceState             = SS_UNKNNOWN;
    source_out2.sourceState             = SS_ON;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // Connection information
    ConnectionInfo.sourceName      = "AnySource1";
    ConnectionInfo.sinkName        = "AnySink1";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;
    listConnectionInfos.push_back(ConnectionInfo);
    mpCAmMainConnectionElement = CAmMainConnectionFactory::createElement(route1, mpCAmControlReceive);
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicyReceive->getListMainConnections(ET_CONNECTION, "AnyConnection1", listConnectionInfos));

}

/**
 * @brief : Test to verify the setListActions function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "setListActions" function when sink,source elements are created,
 *         set the action parameter for source,sink,timeout and also set the action details like name,map parameter
 *         and push that action detail in list.
 *         and also pass action list, and type are passed as input parameter along with other configuration details
 *         to setListActions function then will check whether setListActions function providing the interface to
 *         policy engine to set the list of actions to framework or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, setListActionsPositive)
{
    source.name = "AnySource1";
    sink.name   = "AnySink1";
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    CAmActionParam <std::string> sourceNameParam(mpCAmSourceElement->getName());
    CAmActionParam <std::string> sinkNameParam(mpCAmSinkElement->getName());
    CAmActionParam<am_volume_t > timeoutvalue;
    timeoutvalue.setParam(5000);
    mpCAmClassActionConnect = new CAmActionConnect();
    mpCAmClassActionConnect->setParam(ACTION_PARAM_SOURCE_NAME, &sourceNameParam);
    mpCAmClassActionConnect->setParam(ACTION_PARAM_SINK_NAME, &sinkNameParam);
    mpCAmClassActionConnect->setParam(ACTION_PARAM_TIMEOUT, &timeoutvalue);
    Actions.actionType = ACTION_CONNECT;
    Actions.mapParameters.insert(std::pair<string, string>(ACTION_PARAM_SOURCE_NAME, ACTION_PARAM_TIMEOUT));
    Actions.mapParameters.insert(std::pair<string, string>(ACTION_PARAM_SINK_NAME, ACTION_PARAM_TIMEOUT));
    listActions.push_back(Actions);
    gc_ActionList_e actionListType = AL_SYSTEM;
    EXPECT_EQ(E_OK, mpCAmPolicyReceive->setListActions(listActions, actionListType));
    delete mpCAmClassActionConnect;
}

/**
 * @brief : Test to verify the setListActions function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and also initialized the CAmControlReceive,CAmRouteElement
 *        CAmDomainElement class. Sink, Source domain,routing Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume etc and enter in the db.
 *
 * @test : verify the "setListActions" function when sink,source elements are created,
 *         set the action parameter for source,sink,timeout and also set the action details like name,map parameter
 *         and not push that action detail in list.
 *         and also pass action list, and type are passed as input parameter along with other configuration details
 *         to setListActions function then will check whether setListActions function providing the interface to
 *         policy engine to set the list of actions to framework or not, without any Gmock error.
 *
 * @result : "Pass" when getListMainConnections function return the "E_NO_CHANGE" without any Gmock error meesage.
 */
TEST_F(CAmPolicyReceiveTest, setListActionsNegative)
{
    source.name = "AnySource1";
    sink.name   = "AnySink1";
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    CAmActionParam <std::string> sourceNameParam(mpCAmSourceElement->getName());
    CAmActionParam <std::string> sinkNameParam(mpCAmSinkElement->getName());
    CAmActionParam<am_volume_t > timeoutvalue;
    timeoutvalue.setParam(5000);
    // setting paramter
    mpCAmClassActionConnect = new CAmActionConnect();
    mpCAmClassActionConnect->setParam(ACTION_PARAM_SOURCE_NAME, &sourceNameParam);
    mpCAmClassActionConnect->setParam(ACTION_PARAM_SINK_NAME, &sinkNameParam);
    mpCAmClassActionConnect->setParam(ACTION_PARAM_TIMEOUT, &timeoutvalue);
    // Action info
    Actions.actionType = ACTION_CONNECT;
    Actions.mapParameters.insert(std::pair<string, string>(ACTION_PARAM_SOURCE_NAME, ACTION_PARAM_TIMEOUT));
    Actions.mapParameters.insert(std::pair<string, string>(ACTION_PARAM_SOURCE_NAME, ACTION_PARAM_TIMEOUT));
    gc_ActionList_e actionListType = AL_SYSTEM;
    EXPECT_EQ(E_NO_CHANGE, mpCAmPolicyReceive->setListActions(listActions, actionListType));
    delete mpCAmClassActionConnect;
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
