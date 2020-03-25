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
#include "CAmMainConnectionElementTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;
/**
 *@Class : CAmMainConnectionElementTest
 *@brief : This class is used to test the CAmMainConnectionElement class functionality.
 */
CAmMainConnectionElementTest::CAmMainConnectionElementTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
    , mpCAmSinkElement(nullptr)
    , mpCAmSourceElement(nullptr)
    , mpCAmRouteElement(NULL)
    , mpCAmMainConnectionElement1(nullptr)
    , mpCAmElement(nullptr)
{
}

CAmMainConnectionElementTest::~CAmMainConnectionElementTest()
{

}

void CAmMainConnectionElementTest::InitializeCommonStruct()
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    state       = 0;
}

void CAmMainConnectionElementTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _))
    .WillRepeatedly(DoAll(SetArgReferee<1 >(gc_ut_MainConnection_s(CS_UNKNOWN)), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(23, _))
    .WillRepeatedly(DoAll(SetArgReferee<1>(sourceInfo), Return(E_OK)));
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
    sourceID = mpCAmSourceElement->getID();
// gcRoutingElement.name = mpCAmSourceElement->getName()
// + mpCAmSinkElement->getName();
//

    gcRoutingElement.name = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();

    gcRoutingElement.sourceID         = mpCAmSourceElement->getID();
    gcRoutingElement.sinkID           = mpCAmSinkElement->getID();
    gcRoutingElement.domainID         = 2;
    gcRoutingElement.connectionFormat = CF_GENIVI_STEREO;
    mpCAmRouteElement                 = new CAmRouteElement(gcRoutingElement, mpCAmControlReceive);
    ASSERT_THAT(mpCAmRouteElement, NotNull())
        << " Route Element Is not Created";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(_, _))
    .WillOnce(DoAll(SetArgReferee<1>((am_mainConnectionID_t)44), Return(E_OK)));

//
    sink.name                       = "AnySink1";
    source.name                     = "AnySource1";
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    gcRoute.sinkID                  = mpCAmSinkElement->getID();
    gcRoute.sourceID                = mpCAmSourceElement->getID();
    gcRoute.name                    = mpCAmSourceElement->getName() + ":"
        + mpCAmSinkElement->getName();

    mainConnectionData.mainConnectionID = 1;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    state                               = 0;

    mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(
            gcRoute, mpCAmControlReceive);

// mpCAmElement = new CAmElement(ET_SINK, (string)sink.name, mpCAmControlReceive);
}

void CAmMainConnectionElementTest::TearDown()
{

    CAmMainConnectionFactory::destroyElement();
    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();

    if (mpCAmRouteElement != NULL)
    {
        delete (mpCAmRouteElement);
    }

    mpCAmMainConnectionElement1 = nullptr;

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

/**
 * @brief : Test to verify the getMainSourceName functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getMainSourceName" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created then will check whether getMainSourceName function will
 *         providing the interface to get the name of main source of the connection or not,without any gmock error.
 *
 * @result : "Pass" when getMainSourceName function return "E_OK" without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getMainSourceNameCheck)
{
    sink.name                       = "AnySink1";
    source.name                     = "AnySource1";
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    gcRoute.sinkID                  = mpCAmSinkElement->getID();
    gcRoute.sourceID                = mpCAmSourceElement->getID();
    gcRoute.name                    = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    // gcRoute.route.push_back(routingElement);
    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // mpCAmMainConnectionElement1 =CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_EQ(source.name, mpCAmMainConnectionElement1->getMainSourceName());
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the getMainSourceName functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getMainSourceName" function when sink,source,route element is configured along with other parameters,
 *         push the incorrect route name in list and main connection element is created then will check whether
 *         getMainSourceName function will providing the interface to get the name of main source of the connection
 *         or not,without any gmock error.
 *
 * @result : "Pass" when getMainSourceName function return empty string without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getMainSourceNameCheckNegative)
{
// sink.name                       = "AnySink1";
// source.name                     = "AnySource1";
// routingElement.sinkID           = mpCAmSinkElement->getID();
// routingElement.domainID         = mpCAmSinkElement->getDomainID();
// routingElement.connectionFormat = CF_GENIVI_STEREO;
// gcRoute.sinkID                  = mpCAmSinkElement->getID();
// gcRoute.name                    = ":" + mpCAmSinkElement->getName();
// gcRoute.route.push_back(routingElement);

    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    EXPECT_NE("AnySource2", mpCAmMainConnectionElement1->getMainSourceName());
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the getMainSinkName functions for sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getMainSinkName" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created then will check whether getMainSinkName function will
 *         providing the interface to get the name of main sink of the connection or not,without any gmock error.
 *
 * @result : "Pass" when getMainSinkName function return "E_OK" without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getMainSinkNameCheck)
{
    sink.name                       = "AnySink1";
    source.name                     = "AnySource1";
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    gcRoute.sinkID                  = mpCAmSinkElement->getID();
    gcRoute.sourceID                = mpCAmSourceElement->getID();
    gcRoute.name                    = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // mpCAmMainConnectionElement1 =CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_EQ(sink.name, mpCAmMainConnectionElement1->getMainSinkName());
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the getMainSinkName functions for sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getMainSinkName" function when sink,source,route element is configured along with other parameters,
 *         push the incorrect route name in list and main connection element is created then will check whether
 *         getMainSinkName function will providing the interface to get the name of main sink of the connection
 *         or not,without any gmock error.
 *
 * @result : "Pass" when getMainSinkName function return empty string without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getMainSinkNameCheckNegative)
{
    source.name             = "AnySource1";
    sink.name               = "AnySink1";
    routingElement.sourceID = mpCAmSourceElement->getID();
    gcRoute.sourceID        = mpCAmSourceElement->getID();
    gcRoute.sinkID          = 0;
    gcRoute.name            = mpCAmSourceElement->getName() + ":";
    gcRoute.route.push_back(routingElement);
    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    EXPECT_NE("AnySink2", mpCAmMainConnectionElement1->getMainSinkName());
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the setState functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "setState" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and pass the state to the setState function then will check whether
 *         setState function will providing the interface to set the connection state of the connection
 *         or not,without any gmock error.
 *
 * @result : "Pass" when setState function return "E_OK" without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, setStatePositive)
{
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
    state                               = 3;
//// mpCAmMainConnectionElement1         = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    //EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(44), Return(E_OK)));
    mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _)).WillOnce(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainConnectionStateDB(44, CS_DISCONNECTING)).WillOnce(Return(E_OK));
    //EXPECT_EQ(E_OK, mpCAmMainConnectionElement1->setState((am_ConnectionState_e)state));
    ASSERT_NO_THROW(mpCAmMainConnectionElement1->setState((am_ConnectionState_e)state));
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the setState functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "setState" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and pass the state as zero to the setState function then will check whether
 *         setState function will providing the interface to set the connection state of the connection
 *         or not,without any gmock error.
 *
 * @result : "Pass" when setState function return "E_OK" without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, setStateNegative)
{
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
    mainConnectionData.connectionState  = CS_UNKNOWN;

    int state = 0;
    //CAmMainConnectionElement *mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _)).WillOnce(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    //EXPECT_CALL(*mpMockControlReceiveInterface, changeMainConnectionStateDB(44, CS_DISCONNECTING)).WillOnce(Return(E_OK));
    //EXPECT_EQ(E_OK, mpCAmMainConnectionElement1->setState((am_ConnectionState_e)state));
    ASSERT_NO_THROW(mpCAmMainConnectionElement1->setState((am_ConnectionState_e)state));
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the getState functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getState" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB, getMainConnectionInfoDB to
 *         return "E_OK" and also pass the state to the getState function then will check whether
 *         getState function will providing the interface to get the connection state of the connection
 *         or not,without any gmock error.
 *
 * @result : "Pass" when getState function return "E_OK" without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getStatePositive)
{
// sink.name = "AnySink1";
// source.name = "AnySource1";
// routingElement.sourceID = mpCAmSourceElement->getID();
// routingElement.sinkID = mpCAmSinkElement->getID();
// routingElement.domainID = mpCAmSinkElement->getDomainID();
// routingElement.connectionFormat = CF_GENIVI_STEREO;
//// Route Infomation
// gcRoute.sinkID = mpCAmSinkElement->getID();
// gcRoute.sourceID = mpCAmSourceElement->getID();
// gcRoute.name = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
// gcRoute.route.push_back(routingElement);
// mainConnectionData.mainConnectionID = 0;
// mainConnectionData.sinkID = mpCAmSinkElement->getID();
// mainConnectionData.sourceID = mpCAmSourceElement->getID();
// mainConnectionData.delay = 0;
    // mainConnectionData.connectionState = CS_CONNECTED;
    // CAmMainConnectionElement *mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(46), Return(E_OK)));
//// mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(1, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    EXPECT_EQ(state, mpCAmMainConnectionElement1->getState());
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the getState functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getState" function when incorrect sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB, getMainConnectionInfoDB to
 *         return "E_OK" and also pass the state to the getState function then will check whether
 *         getState function will providing the interface to get the connection state of the connection
 *         or not,without any gmock error.
 *
 * @result : "Pass" when getState function return "E_OK" without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getStateNegative)
{
// routingElement.sourceID = mpCAmSourceElement->getID();
// routingElement.sinkID = mpCAmSinkElement->getID();
// routingElement.domainID = mpCAmSinkElement->getDomainID();
// routingElement.connectionFormat = CF_GENIVI_STEREO;
//// Route Infomation
// gcRoute.sinkID = mpCAmSinkElement->getID();
// gcRoute.sourceID = mpCAmSourceElement->getID();
// gcRoute.name = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
// gcRoute.route.push_back(routingElement);
// mainConnectionData.mainConnectionID = 0;
// mainConnectionData.sinkID = mpCAmSinkElement->getID();
// mainConnectionData.sourceID = mpCAmSourceElement->getID();
// mainConnectionData.delay = 0;
    mainConnectionData.connectionState = CS_UNKNOWN;
    state                              = 0;
    // CAmMainConnectionElement *mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
// EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(54), Return(E_OK)));
//// mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _)).WillOnce(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    EXPECT_EQ(CS_UNKNOWN, mpCAmMainConnectionElement1->getState());
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the getPriority functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getPriority" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" and also pass the priority to the getPriority function then will check whether
 *         getPriority function will providing the interface to get the priority of the connection
 *         or not,without any gmock error.
 *
 * @result : "Pass" when getPriority function return "E_OK" without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getPriorityPositive)
{
// routingElement.sourceID = mpCAmSourceElement->getID();
// routingElement.sinkID = mpCAmSinkElement->getID();
// routingElement.domainID = mpCAmSinkElement->getDomainID();
// routingElement.connectionFormat = CF_GENIVI_STEREO;
//// Route Infomation
// gcRoute.sinkID = mpCAmSinkElement->getID();
// gcRoute.sourceID = mpCAmSourceElement->getID();
// gcRoute.name = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
// gcRoute.route.push_back(routingElement);
//
// mainConnectionData.mainConnectionID = 0;
// mainConnectionData.sinkID = mpCAmSinkElement->getID();
// mainConnectionData.sourceID = mpCAmSourceElement->getID();
// mainConnectionData.delay = 0;
// mainConnectionData.connectionState = CS_CONNECTED;
//
// int priority;
//// CAmMainConnectionElement *mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
//
// EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(46), Return(E_OK)));
//// mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);

    EXPECT_EQ(0, mpCAmMainConnectionElement1->getPriority());
    CAmMainConnectionFactory::destroyElement();

}

/**
 * @brief : Test to verify the getPriority functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getPriority" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and not set the behavior of enterMainConnectionDB to
 *         return "E_OK" and also pass the priority to the getPriority function then will check whether
 *         getPriority function will providing the interface to get the priority of the connection
 *         or not,without any gmock error.
 *
 * @result : "Pass" when getPriority function return "E_OK" without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getPriorityNegative)
{
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    int priority;
    // CAmMainConnectionElement *mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_EQ(E_OK, mpCAmMainConnectionElement1->getPriority(priority));
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the updateState functions with connection state suspend for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "updateState" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB,getSourceInfoDB,
 *         getMainConnectionInfoDB to return "E_OK" and make connection state as suspend then will check whether
 *         updateState function will calculates the connection state from the actual route elements
 *         or not,without any gmock error.
 *
 * @result : "Pass" when updateState function will return without any Gmock error message

TEST_F(CAmMainConnectionElementTest, updateStatePositiveConnstateSuspend)
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

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // creating the routing element
    gcRoutingElement.name             = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoutingElement.sourceID         = mpCAmSourceElement->getID();
    gcRoutingElement.sinkID           = mpCAmSinkElement->getID();
    gcRoutingElement.domainID         = mpCAmSinkElement->getDomainID();
    gcRoutingElement.connectionFormat = CF_GENIVI_STEREO;

    am_Source_s sourcedata;
    sourcedata.sourceState = SS_OFF;
    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(48), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    // mpCAmRouteElement           = CAmRouteFactory::createElement(gcRoutingElement, mpCAmControlReceive);
    state = 2;
//// mpCAmRouteElement->setState((am_ConnectionState_e)state);
// EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_OK)));
// EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(48, _)).WillOnce(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
// EXPECT_CALL(*mpMockControlReceiveInterface, changeMainConnectionStateDB(48, CS_SUSPENDED)).WillOnce(Return(E_OK));
   ASSERT_NO_THROW(mpCAmMainConnectionElement1->updateState());
    CAmMainConnectionFactory::destroyElement();

}

/**
 * @brief : Test to verify the updateState functions with connection state as connected for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "updateState" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB,getSourceInfoDB,
 *         getMainConnectionInfoDB to return "E_OK" and make connection state as connected then will check whether
 *         updateState function will calculates the connection state from the actual route elements
 *         or not,without any gmock error.
 *
 * @result : "Pass" when updateState function will return without any Gmock error message
 */
/*TEST_F(CAmMainConnectionElementTest, updateStatePositiveConnstateConnect)
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

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    gcRoute.sinkID                  = mpCAmSinkElement->getID();
    gcRoute.sourceID                = mpCAmSourceElement->getID();
    gcRoute.name                    = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTING;

    // creating the routing element
    gcRoutingElement.name             = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoutingElement.sourceID         = mpCAmSourceElement->getID();
    gcRoutingElement.sinkID           = mpCAmSinkElement->getID();
    gcRoutingElement.domainID         = mpCAmSinkElement->getDomainID();
    gcRoutingElement.connectionFormat = CF_GENIVI_STEREO;

    source.sourceState = SS_ON;
    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(49), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    // mpCAmRouteElement           = CAmRouteFactory::createElement(gcRoutingElement, mpCAmControlReceive);
    state = 2;
    // mpCAmRouteElement->setState((am_ConnectionState_e)state);

// EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
// EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(49, _)).WillOnce(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
// EXPECT_CALL(*mpMockControlReceiveInterface, changeMainConnectionStateDB(49, CS_CONNECTED)).WillOnce(Return(E_OK));
    ASSERT_NO_THROW(mpCAmMainConnectionElement1->updateState());
    CAmMainConnectionFactory::destroyElement();
}*/

/**
 * @brief : Test to verify the getMuteState functions with mute state as MS_MUTED for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getMuteState" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" and make connection state as CS_CONNECTED and mute state as MS_MUTED then will check whether
 *         getMuteState function will check the mute state of main connection and then mute the connection state according,
 *         or not,without any gmock error.
 *
 * @result : "Pass" when getMuteState function will return without any Gmock error message
 */
#ifdef VOLUME_RELATED_TEST
TEST_F(CAmMainConnectionElementTest, getMuteStatePositive)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    am_sourceID_t sourceID = 0;
    am_sinkID_t   sinkID   = 0;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(50), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    am_MuteState_e muteState = MS_MUTED;

    gc_LimitVolume_s limitVolume;
    limitVolume.limitType   = LT_ABSOLUTE;
    limitVolume.limitVolume = -3000;

    std::map<uint32_t, gc_LimitVolume_s > maplimit;
    maplimit.insert(std::pair<uint32_t, gc_LimitVolume_s>(1, limitVolume));
    mpCAmMainConnectionElement1->setLimits(maplimit);
    EXPECT_EQ(muteState, mpCAmMainConnectionElement1->getMuteState());
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the getMuteState functions with mute state as MS_UNMUTED for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getMuteState" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" and make connection state as CS_CONNECTED and mute state as MS_UNMUTED then will check whether
 *         getMuteState function will check the mute state of main connection and then mute the connection state according,
 *         or not,without any gmock error.
 *
 * @result : "Pass" when getMuteState function will return without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getMuteStateNegative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource1";
    am_sourceID_t sourceID = 0;
    am_sinkID_t   sinkID   = 0;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(51), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    am_MuteState_e   muteState = MS_UNMUTED;
    gc_LimitVolume_s limitVolume;
    limitVolume.limitType   = LT_ABSOLUTE;
    limitVolume.limitVolume = 20;

    std::map<uint32_t, gc_LimitVolume_s > maplimit;
    maplimit.insert(std::pair<uint32_t, gc_LimitVolume_s>(1, limitVolume));

    mpCAmMainConnectionElement1->setLimits(maplimit);
    EXPECT_EQ(muteState, mpCAmMainConnectionElement1->getMuteState());
    CAmMainConnectionFactory::destroyElement();

}

/**
 * @brief : Test to verify the setLimits functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "setLimits" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" and set the limit volume type and volume details
 *         and insert into the maplimit  then will check whether setLimits function will set the limits volume details
 *         or not, without any gmock error.
 *
 * @result : "Pass" when setLimits function will return "E_OK", without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, SetLimitspositive)
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

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    gcRoute.sinkID                  = mpCAmSinkElement->getID();
    gcRoute.sourceID                = mpCAmSourceElement->getID();
    gcRoute.name                    = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(51), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);

    gc_LimitVolume_s limitVolume;
    limitVolume.limitType   = LT_RELATIVE;
    limitVolume.limitVolume = 15;

    std::map<uint32_t, gc_LimitVolume_s > maplimit;
    maplimit.insert(std::pair<uint32_t, gc_LimitVolume_s>(1, limitVolume));
    EXPECT_EQ(E_OK, mpCAmMainConnectionElement1->setLimits(maplimit));
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the getVolumeChangeElements functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getVolumeChangeElements" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" and set the data of gc_LimitVolume_s,gc_volume_s structures and set the limit type as LT_RELATIVE,
 *         then will check whether getVolumeChangeElements function will get the limits volume details
 *         or not, without any gmock error.
 *
 * @result : "Pass" when getVolumeChangeElements function will throw any without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getVolumeChangeElementspositiveLimitTypeRelative)
{
    sink.name   = "AnySink1";
    source.name = "AnySource15";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(52), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);

    am_volume_t      requestedVolume = 15;
    gc_LimitVolume_s limitVolume;
    limitVolume.limitType   = LT_RELATIVE;
    limitVolume.limitVolume = 15;

    std::map<uint32_t, gc_LimitVolume_s > maplimit;
    maplimit.insert(std::pair<uint32_t, gc_LimitVolume_s>(1, limitVolume));

    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;

// std::map<CAmElement *, gc_volume_s > listVolumeElements;
//// listVolumeElements.insert(std::pair<CAmElement *, gc_volume_s>(mpCAmSinkElement, gcVolume));
// mpCAmMainConnectionElement1->setLimits(maplimit);
//// ASSERT_NO_THROW(mpCAmMainConnectionElement1->getVolumeChangeElements(requestedVolume, listVolumeElements));
//

    std::map<std::shared_ptr<CAmElement >, gc_volume_s > listVolumeElements;
    std::shared_ptr<CAmElement >                         mpCAmElement;
    mpCAmMainConnectionElement1->setLimits(maplimit);
    ASSERT_NO_THROW(mpCAmMainConnectionElement1->getVolumeChangeElements(requestedVolume, listVolumeElements, mpCAmElement));

    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the getVolumeChangeElements functions with limit type as Absolute for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getVolumeChangeElements" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" and set the data of gc_LimitVolume_s,gc_volume_s structures and set the limit type as LT_ABSOLUTE,
 *         then will check whether getVolumeChangeElements function will get the limits volume details
 *         or not, without any gmock error.
 *
 * @result : "Pass" when getVolumeChangeElements function will throw any without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getVolumeChangeElementspositiveLimitTypeAbsolute)
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

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
// EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(53), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);

    am_volume_t      requestedVolume = 15;
    gc_LimitVolume_s limitVolume;
    limitVolume.limitType   = LT_ABSOLUTE;
    limitVolume.limitVolume = 15;

    std::map<uint32_t, gc_LimitVolume_s > maplimit;
    maplimit.insert(std::pair<uint32_t, gc_LimitVolume_s>(1, limitVolume));
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;

// std::map<CAmElement *, gc_volume_s > listVolumeElements;
//// listVolumeElements.insert(std::pair<CAmElement *, gc_volume_s>(mpCAmSinkElement, gcVolume));
//
// mpCAmMainConnectionElement1->setLimits(maplimit);
//// ASSERT_NO_THROW(mpCAmMainConnectionElement1->getVolumeChangeElements(requestedVolume, listVolumeElements));
//

    std::map<std::shared_ptr<CAmElement >, gc_volume_s > listVolumeElements;
    std::shared_ptr<CAmElement >                         mpCAmElement;
    mpCAmMainConnectionElement1->setLimits(maplimit);
    ASSERT_NO_THROW(mpCAmMainConnectionElement1->getVolumeChangeElements(requestedVolume, listVolumeElements, mpCAmElement));

    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the getVolumeChangeElements functions with limit type as Absolute with mute for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getVolumeChangeElements" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" and set the data of gc_LimitVolume_s,gc_volume_s structures and set the limit type as LT_ABSOLUTE,
 *         mute state as mute then will check whether getVolumeChangeElements function will get the limits volume details
 *         or not, without any gmock error.
 *
 * @result : "Pass" when getVolumeChangeElements function will throw any without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getVolumeChangeElementspositiveLimitTypeAbsoluteWithMute)
{
    sink.name   = "AnySink20";
    source.name = "AnySource20";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    std::vector<am_Route_s>amRouteList;
    am_RoutingElement_s    routingElement;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    gcRoute.sinkID                  = mpCAmSinkElement->getID();
    gcRoute.sourceID                = mpCAmSourceElement->getID();
    gcRoute.name                    = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(54), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    am_volume_t      requestedVolume = 15;
    gc_LimitVolume_s limitVolume;
    limitVolume.limitType   = LT_ABSOLUTE;
    limitVolume.limitVolume = -3000;

    std::map<uint32_t, gc_LimitVolume_s > maplimit;
    maplimit.insert(std::pair<uint32_t, gc_LimitVolume_s>(1, limitVolume));
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;
    std::map<CAmElement *, gc_volume_s > listVolumeElements;
    // listVolumeElements.insert(std::pair<CAmElement *, gc_volume_s>(mpCAmSinkElement, gcVolume));
    mpCAmMainConnectionElement1->setLimits(maplimit);
    // ASSERT_NO_THROW(mpCAmMainConnectionElement1->getVolumeChangeElements(requestedVolume, listVolumeElements));
    CAmMainConnectionFactory::destroyElement();

}

/**
 * @brief : Test to verify the getVolumeChangeElements functions with limit type as Absolute with mute for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getVolumeChangeElements" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" and set the data of gc_LimitVolume_s,gc_volume_s structures and set the limit type as LT_ABSOLUTE,
 *         mute state as mute then will check whether getVolumeChangeElements function will get the limits volume details
 *         or not, without any gmock error.
 *
 * @result : "Pass" when getVolumeChangeElements function will throw any without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getVolumeChangeElementspositiveLimitTypeAbsoluteWithMute1)
{
    sink.name   = "AnySink10";
    source.name = "AnySource10";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    std::vector<am_Route_s>amRouteList;
    am_RoutingElement_s    routingElement;
    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    gcRoute.sinkID                  = mpCAmSinkElement->getID();
    gcRoute.sourceID                = mpCAmSourceElement->getID();
    gcRoute.name                    = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // mpCAmMainConnectionElement1         = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(54), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    am_volume_t      requestedVolume = 15;
    gc_LimitVolume_s limitVolume;
    limitVolume.limitType   = LT_ABSOLUTE;
    limitVolume.limitVolume = -3000;
    std::map<uint32_t, gc_LimitVolume_s > maplimit;
    maplimit.insert(std::pair<uint32_t, gc_LimitVolume_s>(1, limitVolume));
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 20;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;
    std::map<std::shared_ptr<CAmElement >, gc_volume_s > listVolumeElements;
    std::shared_ptr<CAmElement >                         mpCAmElement;
    mpCAmMainConnectionElement1->setLimits(maplimit);
    ASSERT_NO_THROW(mpCAmMainConnectionElement1->getVolumeChangeElements(requestedVolume, listVolumeElements, mpCAmElement));

    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the getVolumeChangeElements functions with limit type as Absolute for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "getVolumeChangeElements" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" and set the data of gc_LimitVolume_s,gc_volume_s structures and set the limit type as LT_ABSOLUTE,
 *         mute state as mute then will check whether getVolumeChangeElements function will get the limits volume details
 *         or not, without any gmock error.
 *
 * @result : "Pass" when getVolumeChangeElements function will throw any without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, getVolumeChangeElementsNegative)
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

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(56), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);

    am_volume_t requestedVolume = 15;

    gc_LimitVolume_s limitVolume;
    limitVolume.limitType   = LT_ABSOLUTE;
    limitVolume.limitVolume = -3000;

    std::map<uint32_t, gc_LimitVolume_s > maplimit;
    gc_volume_s                           gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;

    std::map<std::shared_ptr<CAmElement >, gc_volume_s > listVolumeElements;
    std::shared_ptr<CAmElement >                         mpCAmElement;
    mpCAmMainConnectionElement1->setLimits(maplimit);
    ASSERT_NO_THROW(mpCAmMainConnectionElement1->getVolumeChangeElements(requestedVolume, listVolumeElements, mpCAmElement));
    CAmMainConnectionFactory::destroyElement();
}
#endif
/**
 * @brief : Test to verify the isSinkBelongingToMainConnection functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "isSinkBelongingToMainConnection" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" then will check whether isSinkBelongingToMainConnection function will check the sink element is belong to
 *         main connection or not, without any gmock error.
 *
 * @result : "Pass" when isSinkBelongingToMainConnection function will return "true", without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, isSinkBelongingToMainConnectionPositive)
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

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;

    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);

    gcRoutingElement.name = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    gcRoutingElement.sourceID           = mpCAmSourceElement->getID();
    gcRoutingElement.sinkID             = mpCAmSinkElement->getID();
    gcRoutingElement.domainID           = 2;
    gcRoutingElement.connectionFormat   = CF_GENIVI_STEREO;
    vCAmRouteElement.push_back(mpCAmRouteElement1);
    mpCAmMainConnectionElement1->getListRouteElements(vCAmRouteElement);
    ASSERT_NO_THROW(mpCAmMainConnectionElement1->isSinkBelongingToMainConnection(mpCAmSinkElement));
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the isSinkBelongingToMainConnection functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "isSinkBelongingToMainConnection" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and push incorrect route details in list & set the behavior of enterMainConnectionDB to
 *         return "E_OK" then will check whether isSinkBelongingToMainConnection function will check the sink element is belong to
 *         main connection or not, without any gmock error.
 *
 * @result : "Pass" when isSinkBelongingToMainConnection function will return "false", without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, isSinkBelongingToMainConnectionNegative)
{
    sink.name   = "AnySink2";
    source.name = "AnySource2";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    routingElement.sourceID             = mpCAmSourceElement->getID();
    routingElement.connectionFormat     = CF_GENIVI_STEREO;
    gcRoute.sourceID                    = mpCAmSourceElement->getID();
    gcRoute.name                        = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    EXPECT_FALSE(mpCAmMainConnectionElement1->isSinkBelongingToMainConnection(mpCAmSinkElement));
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the isSourceBelongingToMainConnection functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "isSourceBelongingToMainConnection" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" then will check whether isSourceBelongingToMainConnection function will check the source element is belong to
 *         main connection or not, without any gmock error.
 *
 * @result : "Pass" when isSourceBelongingToMainConnection function will return "true", without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, isSourceBelongingToMainConnectionPositive)
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

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    vCAmRouteElement.push_back(mpCAmRouteElement1);
    mpCAmMainConnectionElement1->getListRouteElements(vCAmRouteElement);
    ASSERT_NO_THROW(mpCAmMainConnectionElement1->isSourceBelongingToMainConnection(mpCAmSourceElement));
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the isSourceBelongingToMainConnection functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "isSourceBelongingToMainConnection" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and push incorrect route details in list & set the behavior of enterMainConnectionDB to
 *         return "E_OK" then will check whether isSourceBelongingToMainConnection function will check the source element is belong to
 *         main connection or not, without any gmock error.
 *
 * @result : "Pass" when isSourceBelongingToMainConnection function will return "false", without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, isSourceBelongingToMainConnectionNegative)
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

    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID        = mpCAmSinkElement->getID();
    gcRoute.name          = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoutingElement.name = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // mpCAmMainConnectionElement1         = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(58), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    EXPECT_FALSE(mpCAmMainConnectionElement1->isSourceBelongingToMainConnection(mpCAmSourceElement));
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the isMainConnectionMuted functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "isMainConnectionMuted" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" then will check whether isMainConnectionMuted function will check whether main connection is mute
 *         or not, without any gmock error.
 *
 * @result : "Pass" when isMainConnectionMuted function will return "true", without any Gmock error message
 */
#ifdef VOLUME_RELATED_TEST
TEST_F(CAmMainConnectionElementTest, isMainConnectionMutedPositive)
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

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;

    // mpCAmMainConnectionElement1 = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(59), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    gc_LimitVolume_s limitVolume;
    limitVolume.limitType   = LT_ABSOLUTE;
    limitVolume.limitVolume = -3000;
    std::map<uint32_t, gc_LimitVolume_s > maplimit;
    maplimit.insert(std::pair<uint32_t, gc_LimitVolume_s>(1, limitVolume));
    mpCAmMainConnectionElement1->setLimits(maplimit);
    EXPECT_TRUE(mpCAmMainConnectionElement1->isMainConnectionMuted());
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the isMainConnectionMuted functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "isMainConnectionMuted" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB to
 *         return "E_OK" then will check whether isMainConnectionMuted function will check whether main connection is mute
 *         or not, without any gmock error.
 *
 * @result : "Pass" when isMainConnectionMuted function will return "false", without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, isMainConnectionMutedNegative)
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

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // mpCAmMainConnectionElement1         = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
// EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(60), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    gc_LimitVolume_s limitVolume;
    limitVolume.limitType   = LT_ABSOLUTE;
    limitVolume.limitVolume = 20;
    std::map<uint32_t, gc_LimitVolume_s > maplimit;
    maplimit.insert(std::pair<uint32_t, gc_LimitVolume_s>(1, limitVolume));
    mpCAmMainConnectionElement1->setLimits(maplimit);
    EXPECT_FALSE(mpCAmMainConnectionElement1->isMainConnectionMuted());
    CAmMainConnectionFactory::destroyElement();
}

/**
 * @brief : Test to verify the updateMainVolume functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "updateMainVolume" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB,getMainConnectionInfoDB to
 *         return "E_OK" then will check whether updateMainVolume function will check whether main volume is updated or not
 *         which is the sum of the volume of all the connection in which it is involved,
 *         without any gmock error.
 *
 * @result : "Pass" when updateMainVolume function will return "true", without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, updateMainVolumePositive)
{
    sink.name   = "AnySink29";
    source.name = "AnySource29";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // mpCAmMainConnectionElement1         = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(61), Return(E_OK)));
    // mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
    // EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(61, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmMainConnectionElement1->updateMainVolume());
    CAmMainConnectionFactory::destroyElement();

}

/**
 * @brief : Test to verify the updateMainVolume functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and  initialized the CAmControlReceive class,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db.
 *
 * @test : verify the "updateMainVolume" function when sink,source,route element is configured along with other parameters,
 *         and main connection element is created and set the behavior of enterMainConnectionDB,getMainConnectionInfoDB to
 *         return "E_OK" then will check whether updateMainVolume function will check whether main volume is updated or not
 *         which is the sum of the volume of all the connection in which it is involved,
 *         without any gmock error.
 *
 * @result : "Pass" when updateMainVolume function will return "true", without any Gmock error message
 */
TEST_F(CAmMainConnectionElementTest, updateMainVolumeNegative)
{
    sink.name   = "AnySink29";
    source.name = "AnySource29";
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    routingElement.sourceID         = mpCAmSourceElement->getID();
    routingElement.sinkID           = mpCAmSinkElement->getID();
    routingElement.domainID         = mpCAmSinkElement->getDomainID();
    routingElement.connectionFormat = CF_GENIVI_STEREO;
    // Route Infomation
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    gcRoutingElement.name               = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = mpCAmSinkElement->getID();
    mainConnectionData.sourceID         = mpCAmSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTED;
    // mpCAmMainConnectionElement1         = new CAmMainConnectionElement(gcRoute, mpCAmControlReceive);
// EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(61), Return(E_OK)));
//// mpCAmMainConnectionElement1 = CAmMainConnectionFactory::createElement(gcRoute, mpCAmControlReceive);
// EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(61, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmMainConnectionElement1->updateMainVolume());
    CAmMainConnectionFactory::destroyElement();
}
#endif

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
