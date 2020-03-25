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
#include "CAmPolicyReceive.h"
#include "CAmPolicySend.h"
#include "MockIAmControlReceive.h"
#include "MockIAmPolicySend.h"
#include "MockIAmPolicyReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmPolicySendTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmPolicySendTest
 *@brief : This class is used to test the CAmPolicySend functionality.
 */
CAmPolicySendTest::CAmPolicySendTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpMockIAmPolicyReceive(NULL)
    , mpCAmPolicySend(NULL)
{
}

CAmPolicySendTest::~CAmPolicySendTest()
{
}

void CAmPolicySendTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;
    // This will check whether controller is initialized or not
    ASSERT_THAT(mpCAmControlReceive, NotNull()) << " Controller Not Initialized";
    mpMockIAmPolicyReceive = new MockIAmPolicyReceive();
    ASSERT_THAT(mpMockIAmPolicyReceive, NotNull()) << " mpMockIAmPolicyReceive is Not Initialized";
    mpCAmPolicySend = new gc::CAmPolicySend();
    mpCAmPolicySend->startupInterface(mpMockIAmPolicyReceive);

}

void CAmPolicySendTest::TearDown()
{
    if (mpCAmPolicySend != NULL)
    {
        delete mpCAmPolicySend;
    }

    if (mpMockIAmPolicyReceive != NULL)
    {
        delete mpMockIAmPolicyReceive;
    }

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

/**
 * @brief :  Test to verify the StatupInterface function Positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "startupInterface" function when instance of the MockIAmPolicyReceive is pass as input parameter
 *         along with other configuration details then will check whether startupInterface function
 *         providing the interface to start the policy engine  or not
 *         & instantiate the policy engine class and configure reader class or not without any Gmock error.
 *
 * @result : "Pass" when startupInterface function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, StatupInterfacePositive)
{
    EXPECT_EQ(E_OK, mpCAmPolicySend->startupInterface(mpMockIAmPolicyReceive));
}

/**
 * @brief :  Test to verify the StatupInterface function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "startupInterface" function when null instance of the MockIAmPolicyReceive is pass as input parameter
 *         along with other configuration details then will check whether startupInterface function
 *         providing the interface to start the policy engine  or not
 *         & instantiate the policy engine class and configure reader class or not without any Gmock error.
 *
 * @result : "Pass" when startupInterface function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, StatupInterfaceNegative)
{
    mpMockIAmPolicyReceive = NULL;
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->startupInterface(mpMockIAmPolicyReceive));
}

/**
 * @brief :  Test to verify the hookRegisterDomain function Positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookRegisterDomain" function when set the behavior of isRegistered,setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & domainName and status are pass as input parameter
 *         along with other configuration details to hookRegisterDomain then will check whether
 *         hookRegisterDomain function providing the interface to framework to pass the hook of
 *         register domain request from routing adaptor to policy engine, or not without any Gmock error.
 *
 * @result : "Pass" when hookRegisterDomain function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookRegisterDomainPositive)
{
    domainName = "VirtDSP";
    gateway0   = "Gateway0";
    gateway1   = "Gateway1";
    am_Error_e status = E_OK;
    EXPECT_CALL(*mpMockIAmPolicyReceive, isRegistered(ET_GATEWAY, gateway0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*mpMockIAmPolicyReceive, isRegistered(ET_GATEWAY, gateway1)).WillRepeatedly(Return(0));
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookRegisterDomain(domainName, status));
}

/**
 * @brief :  Test to verify the hookRegisterDomain function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookRegisterDomain" function when set the behavior of isRegistered,setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & domainName and status as "E_NOT_POSSIBLE" are pass as input parameter
 *         along with other configuration details to hookRegisterDomain then will check whether
 *         hookRegisterDomain function providing the interface to framework to pass the hook of
 *         register domain request from routing adaptor to policy engine, or not without any Gmock error.
 *
 * @result : "Pass" when hookRegisterDomain function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookRegisterDomainNegative)
{

    domainName = "VirtDSP";
    am_Error_e status = E_NOT_POSSIBLE;
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookRegisterDomain(domainName, status));

}

/**
 * @brief :  Test to verify the hookRegisterSource function Positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookRegisterSource" function when set the behavior of isRegistered,setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & sourceName and status are pass as input parameter
 *         along with other configuration details to hookRegisterSource then will check whether it
 *         providing the interface to framework to pass the hook of register source request to policy engine
 *         ,or not without any Gmock error.
 *
 * @result : "Pass" when hookRegisterSource function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookRegisterSourcePositive)
{

    gateway0   = "Gateway0";
    gateway1   = "Gateway1";
    sourceName = "Navigation";
    am_Error_e status = E_OK;
    EXPECT_CALL(*mpMockIAmPolicyReceive, isRegistered(ET_GATEWAY, gateway0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*mpMockIAmPolicyReceive, isRegistered(ET_GATEWAY, gateway1)).WillRepeatedly(Return(0));
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillRepeatedly(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookRegisterSource(sourceName, status));
}

/**
 * @brief :  Test to verify the hookRegisterSource function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookRegisterSource" function when set the behavior of isRegistered,setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & sourceName and status as "E_NOT_POSSIBLE" are pass as input parameter
 *         along with other configuration details to hookRegisterSource then will check whether it
 *         providing the interface to framework to pass the hook of register source request to policy engine
 *         ,or not without any Gmock error.
 *
 * @result : "Pass" when hookRegisterSource function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookRegisterSourceNegative)
{

    gateway0   = "Gateway0";
    gateway1   = "Gateway1";
    sourceName = "Navigation";
    am_Error_e status = E_NOT_POSSIBLE;
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookRegisterSource(sourceName, status));
}

/**
 * @brief :  Test to verify the hookRegisterSink function Positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookRegisterSink" function when set the behavior of isRegistered,setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & sinkName and status are pass as input parameter
 *         along with other configuration details to hookRegisterSink then will check whether it
 *         providing the interface to framework to pass the hook of register sink request to policy engine
 *         ,or not without any Gmock error.
 *
 * @result : "Pass" when hookRegisterSink function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookRegisterSinkPositive)
{

    gateway0 = "Gateway0";
    gateway1 = "Gateway1";
    sinkName = "AMP";
    am_Error_e status = E_OK;
    EXPECT_CALL(*mpMockIAmPolicyReceive, isRegistered(ET_GATEWAY, gateway0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*mpMockIAmPolicyReceive, isRegistered(ET_GATEWAY, gateway1)).WillRepeatedly(Return(0));
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillRepeatedly(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookRegisterSink(sinkName, status));
}

/**
 * @brief :  Test to verify the hookRegisterSink function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookRegisterSink" function when set the behavior of isRegistered,setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & sourceName and status as "E_NOT_POSSIBLE" are pass as input parameter
 *         along with other configuration details to hookRegisterSink then will check whether it
 *         providing the interface to framework to pass the hook of register sink request to policy engine
 *         ,or not without any Gmock error.
 *
 * @result : "Pass" when hookRegisterSink function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookRegisterSinkNegative)
{

    gateway0 = "Gateway0";
    gateway1 = "Gateway1";
    sinkName = "AMP";
    am_Error_e status = E_NOT_POSSIBLE;
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookRegisterSink(sinkName, status));
}

/**
 * @brief :  Test to verify the hookRegisterGateway function Positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookRegisterGateway" function when set the behavior of setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & gatewayName and status are pass as input parameter
 *         along with other configuration details to hookRegisterGateway then will check whether it
 *         providing the interface to framework to pass the hook of register gateway request to policy engine
 *         ,or not without any Gmock error.
 *
 * @result : "Pass" when hookRegisterGateway function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookRegisterGatewayPositive)
{

    gatewayName = "Gateway0";
    am_Error_e status = E_OK;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillRepeatedly(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookRegisterGateway(gatewayName, status));
}

/**
 * @brief :  Test to verify the hookRegisterGateway function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookRegisterGateway" function when set the behavior of setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & gatewayName and status as "E_NOT_POSSIBLE" are pass as input parameters
 *         along with other configuration details to hookRegisterGateway then will check whether it
 *         providing the interface to framework to pass the hook of register gateway request to policy engine
 *         ,or not without any Gmock error.
 *
 * @result : "Pass" when hookRegisterGateway function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookRegisterGatewayNegative)
{

    gatewayName = "AMP";
    am_Error_e status = E_NOT_POSSIBLE;
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookRegisterGateway(gatewayName, status));
}

/**
 * @brief :  Test to verify the hookDeregisterDomain function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDeregisterDomain" function when set the behavior of isRegistered,setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & domain Name and status are pass as input parameter
 *         along with other configuration details to hookDeregisterDomain then will check whether it
 *         providing the interface to framework to pass the hook of unregister domain request from
 *         routing adaptor to policy engine,or not without any Gmock error.
 *
 * @result : "Pass" when hookDeregisterDomain function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookDeregisterDomainPositive)
{

    domainName = "VirtDSP";
    am_Error_e status = E_OK;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookDeregisterDomain(domainName, status));
}

/**
 * @brief :  Test to verify the hookDeregisterDomain function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDeregisterDomain" function when set the behavior of isRegistered,setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & domain Name and status as "E_NOT_POSSIBLE" are pass as input parameter
 *         along with other configuration details to hookDeregisterDomain then will check whether it
 *         providing the interface to framework to pass the hook of unregister domain request from
 *         routing adaptor to policy engine,or not without any Gmock error.
 *
 * @result : "Pass" when hookDeregisterDomain function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookDeregisterDomainNegative)
{

    domainName = "VirtDSP";
    am_Error_e status = E_NOT_POSSIBLE;
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookDeregisterDomain(domainName, status));
}

/**
 * @brief :  Test to verify the hookDeregisterSource function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDeregisterSource" function when set the behavior of setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & source Name and status are pass as input parameter
 *         along with other configuration details to hookDeregisterSource then will check whether it
 *         providing the interface to framework to pass the hook of unregister source request from
 *         routing adaptor to policy engine,or not without any Gmock error.
 *
 * @result : "Pass" when hookDeregisterSource function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookDeregisterSourcePositive)
{

    gateway0   = "Gateway0";
    gateway1   = "Gateway1";
    sourceName = "Navigation";
    am_Error_e status = E_OK;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillRepeatedly(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookDeregisterSource(sourceName, status));
}

/**
 * @brief :  Test to verify the hookDeregisterSource function negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDeregisterSource" function when set the behavior of setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & source Name and status as "E_NOT_POSSIBLE" are pass as input parameter
 *         along with other configuration details to hookDeregisterSource then will check whether it
 *         providing the interface to framework to pass the hook of unregister source request from
 *         routing adaptor to policy engine,or not without any Gmock error.
 *
 * @result : "Pass" when hookDeregisterSource function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookDeregisterSourceNegative)
{

    sourceName = "Navigation";
    am_Error_e status = E_NOT_POSSIBLE;
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookRegisterSource(sourceName, status));
}

/**
 * @brief :  Test to verify the hookDeregisterSink function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDeregisterSink" function when set the behavior of setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & sink Name and status are pass as input parameter
 *         along with other configuration details to hookDeregisterSink then will check whether it
 *         providing the interface to framework to pass the hook of unregister sink request from
 *         routing adaptor to policy engine,or not without any Gmock error.
 *
 * @result : "Pass" when hookDeregisterSink function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookDeregisterSinkPositive)
{

    sinkName = "AMP";
    am_Error_e status = E_OK;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillRepeatedly(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookDeregisterSink(sinkName, status));
}

/**
 * @brief :  Test to verify the hookDeregisterSink function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDeregisterSink" function when set the behavior of setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & sink Name and status as "E_NOT_POSSIBLE" are pass as input parameter
 *         along with other configuration details to hookDeregisterSink then will check whether it
 *         providing the interface to framework to pass the hook of unregister sink request from
 *         routing adaptor to policy engine,or not without any Gmock error.
 *
 * @result : "Pass" when hookDeregisterSink function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookDeregisterSinkNegative)
{

    sinkName = "AMP";
    am_Error_e status = E_NOT_POSSIBLE;
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookDeregisterSink(sinkName, status));
}

/**
 * @brief :  Test to verify the hookDeregisterGateway function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDeregisterGateway" function when set the behavior of setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & gateway Name and status are pass as input parameter
 *         along with other configuration details to hookDeregisterGateway then will check whether it
 *         providing the interface to framework to pass the hook of unregister gateway request
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookDeregisterGateway function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookDeregisterGatewayPositive)
{
    gatewayName = "Gateway0";
    am_Error_e status = E_OK;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillRepeatedly(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookDeregisterGateway(gatewayName, status));
}

/**
 * @brief :  Test to verify the hookDeregisterGateway function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDeregisterGateway" function when set the behavior of setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & gateway Name and status as "E_NOT_POSSIBLE" are pass as input parameter
 *         along with other configuration details to hookDeregisterGateway then will check whether it
 *         providing the interface to framework to pass the hook of unregister gateway request
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookDeregisterGateway function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookDeregisterGatewayNegative)
{
    gatewayName = "Gateway0";
    am_Error_e status = E_NOT_POSSIBLE;
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookDeregisterGateway(gatewayName, status));
}

/**
 * @brief :  Test to verify the hookDomainRegistrationComplete function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDomainRegistrationComplete" function when set the behavior of setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & domainName are pass as input parameter
 *         along with other configuration details to hookDomainRegistrationComplete then will check whether it
 *         providing the interface to framework to pass the hook of domain registration complete request
 *         from routing adaptor to policy engine or not, without any Gmock error.
 *
 * @result : "Pass" when hookDomainRegistrationComplete function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookDomainRegistrationCompletePositive)
{

    domainName = "VirtDSP";
    am_Error_e status = E_OK;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookDomainRegistrationComplete(domainName));
}

/**
 * @brief :  Test to verify the hookAllDomainRegistrationComplete function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookAllDomainRegistrationComplete" function when set the behavior of setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" & status is pass as input parameter
 *         along with other configuration details to hookDomainRegistrationComplete then will check whether it
 *         providing the interface to framework to pass the hook of all domain registration complete request
 *         from routing adaptor to policy engine or not, without any Gmock error.
 *
 * @result : "Pass" when hookAllDomainRegistrationComplete function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookAllDomainRegistrationCompletePositive)
{
    am_Error_e status = E_OK;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookAllDomainRegistrationComplete(status));

}

/**
 * @brief : Test to verify the hookConnectionRequest function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookConnectionRequest" function when set the behavior of getListMainConnections,setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" and listConnectionInfos etc .
 *         & class, source, sink Name are pass as input parameter along with other configuration details
 *         to hookConnectionRequest then will check whether it providing the interface to framework
 *         to pass the hook of connection request from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookConnectionRequest function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookConnectionRequestPositive)
{

    className  = "BASE";
    sourceName = "MediaPlayer";
    sinkName   = "Gateway0";

    gc_Element_e elementType = ET_CLASS;
    // Connection information

    ConnectionInfo.sourceName      = "MediaPlayer";
    ConnectionInfo.sinkName        = "Gateway0";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;
    std::vector<gc_ConnectionInfo_s >listConnectionInfos;
    listConnectionInfos.push_back(ConnectionInfo);
    EXPECT_CALL(*mpMockIAmPolicyReceive, getListMainConnections(elementType, className, _)).WillOnce(DoAll(SetArgReferee<2>(listConnectionInfos), Return(E_OK)));
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookConnectionRequest(className, sourceName, sinkName));
}

/**
 * @brief : Test to verify the hookConnectionRequest function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookConnectionRequest" function when set the behavior of getListMainConnections,setListActions to return "E_OK"
 *         & E_NOT_POSSIBLE with parameters as "AL_NORMAL" and listConnectionInfos etc .
 *         & class, source, sink Name are pass as input parameter along with other configuration details
 *         to hookConnectionRequest then will check whether it providing the interface to framework
 *         to pass the hook of connection request from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookConnectionRequest function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookConnectionRequestNegative)
{

    std::string  className   = "BASE";
    std::string  sourceName  = "MediaPlayer";
    std::string  sinkName    = "Gateway0";
    gc_Element_e elementType = ET_CLASS;

    // Connection information

    ConnectionInfo.sourceName      = "MediaPlayer";
    ConnectionInfo.sinkName        = "Gateway0";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;
    std::vector<gc_ConnectionInfo_s >listConnectionInfos;
    listConnectionInfos.push_back(ConnectionInfo);
    EXPECT_CALL(*mpMockIAmPolicyReceive, getListMainConnections(elementType, className, _)).WillOnce(DoAll(SetArgReferee<2>(listConnectionInfos), Return(E_OK)));
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->hookConnectionRequest(className, "AnySource1", "AnySink1"));
}

/**
 * @brief : Test to verify the hookDisconnectionRequest function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDisconnectionRequest" function when set the behavior of getListMainConnections,setListActions to return "E_OK"
 *         with parameter as "AL_NORMAL" and listConnectionInformation etc .
 *         & class, source, sink Name are pass as input parameter along with other configuration details
 *         to v then will check whether it providing the interface to framework
 *         to pass the hook of disconnect request from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookDisconnectionRequest function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookDisconnectionRequestPositive)
{

    std::string  className   = "BASE";
    std::string  sourceName  = "MediaPlayer";
    std::string  sinkName    = "Gateway0";
    gc_Element_e elementType = ET_CLASS;
    // Connection information

    ConnectionInfo.sourceName      = "MediaPlayer";
    ConnectionInfo.sinkName        = "Gateway0";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;
    std::vector<gc_ConnectionInfo_s >listConnectionInfos;
    listConnectionInfos.push_back(ConnectionInfo);
    EXPECT_CALL(*mpMockIAmPolicyReceive, getListMainConnections(elementType, className, _)).WillOnce(DoAll(SetArgReferee<2>(listConnectionInfos), Return(E_OK)));
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillRepeatedly(Return(E_OK));
    mpCAmPolicySend->hookConnectionRequest(className, sourceName, sinkName);
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookDisconnectionRequest(className, sourceName, sinkName));
}

/**
 * @brief : Test to verify the hookDisconnectionRequest function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDisconnectionRequest" function when set the behavior of setListActions to return "E_NOT_POSSIBLE"
 *         with parameter as "AL_NORMAL" and listConnectionInformation etc .
 *         & class, source, sink Name are pass as input parameter along with other configuration details
 *         to v then will check whether it providing the interface to framework
 *         to pass the hook of disconnect request from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookDisconnectionRequest function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookDisconnectionRequestNegative)
{

    className  = "BASE";
    sourceName = "MediaPlayer";
    sinkName   = "Gateway0";
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->hookDisconnectionRequest(className, "AnySource1", "AnySink1"));
}

/**
 * @brief : Test to verify the hookSourceAvailabilityChange function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDisconnectionRequest" function when set the value for am_Availability_s
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & sourceName, availabilityInstance are pass as input parameter along with other configuration details
 *         to hookSourceAvailabilityChange then will check whether it providing the interface to framework
 *         to pass the hook of source availability change request from routing adaptor to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSourceAvailabilityChange function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSourceAvailabilityChangePositive)
{

    sourceName = "MediaPlayer";
    am_Availability_s availabilityInstance;
    availabilityInstance.availability       = A_AVAILABLE;
    availabilityInstance.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSourceAvailabilityChange(sourceName, availabilityInstance));
}

/**
 * @brief : Test to verify the hookSourceAvailabilityChange function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookDisconnectionRequest" function when set the value for am_Availability_s
 *         & also set the behavior of setListActions to return "E_NOT_POSSIBLE"  with parameter as "AL_NORMAL".
 *         & sourceName, availabilityInstance are pass as input parameter along with other configuration details
 *         to hookSourceAvailabilityChange then will check whether it providing the interface to framework
 *         to pass the hook of source availability change request from routing adaptor to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSourceAvailabilityChange function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSourceAvailabilityChangeNegative)
{

    sourceName = "MediaPlayer";
    am_Availability_s availabilityInstance;
    availabilityInstance.availability       = A_AVAILABLE;
    availabilityInstance.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->hookSourceAvailabilityChange("AnySource1", availabilityInstance));
}

/**
 * @brief : Test to verify the hookSinkAvailabilityChange function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSinkAvailabilityChange" function when set the value for am_Availability_s
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & sinkName, availabilityInstance are pass as input parameter along with other configuration details
 *         to hookSourceAvailabilityChange then will check whether it providing the interface to framework
 *         to pass the hook of sink availability change request from routing adaptor to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSinkAvailabilityChange function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSinkAvailabilityChangePositive)
{

    sinkName = "Gateway0";
    am_Availability_s availabilityInstance;
    availabilityInstance.availability       = A_AVAILABLE;
    availabilityInstance.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSinkAvailabilityChange(sinkName, availabilityInstance));
}

/**
 * @brief : Test to verify the hookSinkAvailabilityChange function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSinkAvailabilityChange" function when set the value for am_Availability_s
 *         & also set the behavior of setListActions to return "E_NOT_POSSIBLE"  with parameter as "AL_NORMAL".
 *         & sinkName, availabilityInstance are pass as input parameter along with other configuration details
 *         to hookSourceAvailabilityChange then will check whether it providing the interface to framework
 *         to pass the hook of sink availability change request from routing adaptor to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSinkAvailabilityChange function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSinkAvailabilityChangeNegative)
{

    sinkName = "Gateway0";
    am_Availability_s availabilityInstance;
    availabilityInstance.availability       = A_AVAILABLE;
    availabilityInstance.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->hookSinkAvailabilityChange("AnySink1", availabilityInstance));
}

/**
 * @brief : Test to verify the hookSourceInterruptStateChange function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSourceInterruptStateChange" function when set the value for interruptState as "IS_INTERRUPTED"
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & sourceName, interruptState are pass as input parameter along with other configuration details
 *         to hookSourceInterruptStateChange then will check whether it providing the interface to framework
 *         to pass the hook of source interrupt change request from routing adaptor to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSourceInterruptStateChange function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSourceInterruptStateChangePositive)
{

    sourceName = "MediaPlayer";
    am_InterruptState_e interruptState = IS_INTERRUPTED;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSourceInterruptStateChange(sourceName, interruptState));
}

/**
 * @brief : Test to verify the hookSourceInterruptStateChange function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSourceInterruptStateChange" function when set the value for interruptState as "IS_INTERRUPTED"
 *         & also set the behavior of setListActions to return "E_UNKNOWN"  with parameter as "AL_NORMAL".
 *         & sourceName, interruptState are pass as input parameter along with other configuration details
 *         to hookSourceInterruptStateChange then will check whether it providing the interface to framework
 *         to pass the hook of source interrupt change request from routing adaptor to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSourceInterruptStateChange function return the "E_UNKNOWN" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSourceInterruptStateChangeNegative)
{

    sourceName = "MediaPlayer";
    am_InterruptState_e interruptState = IS_INTERRUPTED;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_UNKNOWN));
    EXPECT_EQ(E_UNKNOWN, mpCAmPolicySend->hookSourceInterruptStateChange("AnySource1", interruptState));
}

/**
 * @brief : Test to verify the hookSetMainSourceSoundProperty function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSetMainSourceSoundProperty" function when set the value for am_MainSoundProperty_s struct
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & sourceName, soundProperty are pass as input parameter along with other configuration details
 *         to v then will check whether it providing the interface to framework
 *         to pass the hook of setting main source sound property request from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSetMainSourceSoundProperty function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSetMainSourceSoundPropertyPositive)
{

    sourceName = "MediaPlayer";
    am_MainSoundProperty_s soundProperty;
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 10;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSetMainSourceSoundProperty(sourceName, soundProperty));
}

/**
 * @brief : Test to verify the hookSetMainSourceSoundProperty function negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSetMainSourceSoundProperty" function when set the value for am_MainSoundProperty_s struct
 *         & also set the behavior of setListActions to return "E_NOT_POSSIBLE"  with parameter as "AL_NORMAL".
 *         & sourceName, soundProperty are pass as input parameter along with other configuration details
 *         to hookSetMainSourceSoundProperty then will check whether it providing the interface to framework
 *         to pass the hook of setting main source sound property request from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSetMainSourceSoundProperty function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSetMainSourceSoundPropertyNegative)
{

    sourceName = "MediaPlayer";
    am_MainSoundProperty_s soundProperty;
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 10;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->hookSetMainSourceSoundProperty("AnySource1", soundProperty));
}

/**
 * @brief : Test to verify the hookSetMainSinkSoundProperty function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSetMainSinkSoundProperty" function when set the value for interruptState as "IS_INTERRUPTED"
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & sourceName, interruptState are pass as input parameter along with other configuration details
 *         to hookSetMainSinkSoundProperty then will check whether it providing the interface to framework
 *         to pass the hook of setting main sink sound property request from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSetMainSinkSoundProperty function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSetMainSinkSoundPropertyPositive)
{

    std::string            sinkName = "Gateway0";
    am_MainSoundProperty_s soundProperty;
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 10;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSetMainSinkSoundProperty(sinkName, soundProperty));
}

/**
 * @brief : Test to verify the hookSetMainSinkSoundProperty function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSetMainSinkSoundProperty" function when set the value for interruptState as "IS_INTERRUPTED"
 *         & also set the behavior of setListActions to return "E_NOT_POSSIBLE"  with parameter as "AL_NORMAL".
 *         & sourceName, interruptState are pass as input parameter along with other configuration details
 *         to hookSetMainSinkSoundProperty then will check whether it providing the interface to framework
 *         to pass the hook of setting main sink sound property request from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSetMainSinkSoundProperty function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSetMainSinkSoundPropertyNegative)
{

    std::string            sinkName = "Gateway0";
    am_MainSoundProperty_s soundProperty;
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 10;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->hookSetMainSinkSoundProperty("AnySink1", soundProperty));
}

/**
 * @brief : Test to verify the hookSetSystemProperty function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSetSystemProperty" function when set the value for am_SystemProperty_s
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & systemProperty details is pass as input parameter along with other configuration details
 *         to hookSetSystemProperty then will check whether it providing the interface to framework
 *         to pass the hook of setting system property request from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSetSystemProperty function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSetSystemPropertyPositive)
{

    am_SystemProperty_s systemProperty;
    systemProperty.type  = SYP_DEBUG;
    systemProperty.value = 5;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSetSystemProperty(systemProperty));
}

/**
 * @brief : Test to verify the hookVolumeChange function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookVolumeChange" function when set the value for am_mainVolume_t
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & am_mainVolume_t details & sinkName are pass as input parameter along with other configuration details
 *         to hookVolumeChange then will check whether it providing the interface to framework
 *         to pass the hook of setting system property request from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookVolumeChange function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookVolumeChangePositive)
{

    std::string     sinkName   = "Gateway0";
    am_mainVolume_t mainVolume = 10;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookVolumeChange(sinkName, mainVolume, true));
}

/**
 * @brief : Test to verify the hookVolumeChange function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookVolumeChange" function when set the value for am_mainVolume_t
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NONE".
 *         & am_mainVolume_t details & sinkName are pass as input parameter along with other configuration details
 *         to hookVolumeChange then will check whether it providing the interface to framework
 *         to pass the hook of setting system property request from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookVolumeChange function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookVolumeChangeNegative)
{

    std::string     sinkName   = "Gateway0";
    am_mainVolume_t mainVolume = 10;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->hookVolumeChange("AnySink1", mainVolume, false));
}

/**
 * @brief : Test to verify the hookSetMainSourceNotificationConfiguration function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSetMainSourceNotificationConfiguration" function when set the value for am_NotificationConfiguration_s
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & am_NotificationConfiguration_s details & sourceName are pass as input parameter along with other configuration details
 *         to hookSetMainSourceNotificationConfiguration then will check whether it providing the interface to framework
 *         to pass the hook of source notification configuration from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSetMainSourceNotificationConfiguration function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSetMainSourceNotificationConfigurationPositive)
{

    std::string                    sourceName = "MediaPlayer";
    am_NotificationConfiguration_s notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSetMainSourceNotificationConfiguration(sourceName, notificationConfigurations));
}

/**
 * @brief : Test to verify the hookSetMainSourceNotificationConfiguration function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSetMainSourceNotificationConfiguration" function when set the value for am_NotificationConfiguration_s
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & am_NotificationConfiguration_s details & sourceName are pass as input parameter along with other configuration details
 *         to hookSetMainSourceNotificationConfiguration then will check whether it providing the interface to framework
 *         to pass the hook of source notification configuration from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSetMainSourceNotificationConfiguration function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSetMainSourceNotificationConfigurationNegative)
{

    std::string                    sourceName = "MediaPlayer";
    am_NotificationConfiguration_s notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->hookSetMainSourceNotificationConfiguration("AnySource1", notificationConfigurations));
}

/**
 * @brief : Test to verify the hookSetMainSinkNotificationConfiguration function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSetMainSinkNotificationConfiguration" function when set the value for am_NotificationConfiguration_s
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & am_NotificationConfiguration_s details & sinkName are pass as input parameter along with other configuration details
 *         to hookSetMainSinkNotificationConfiguration then will check whether it providing the interface to framework
 *         to pass the hook of sink notification configuration from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSetMainSinkNotificationConfiguration function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSetMainSinkNotificationConfigurationPositive)
{

    std::string                    sinkName = "AMP";
    am_NotificationConfiguration_s notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSetMainSinkNotificationConfiguration(sinkName, notificationConfigurations));
}

/**
 * @brief : Test to verify the hookSetMainSinkNotificationConfiguration function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSetMainSinkNotificationConfiguration" function when set the value for am_NotificationConfiguration_s
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & am_NotificationConfiguration_s details & sinkName are pass as input parameter along with other configuration details
 *         to hookSetMainSinkNotificationConfiguration then will check whether it providing the interface to framework
 *         to pass the hook of sink notification configuration from application to policy engine.
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSetMainSinkNotificationConfiguration function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSetMainSinkNotificationConfigurationNegative)
{

    std::string                    sinkName = "AMP";
    am_NotificationConfiguration_s notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSetMainSinkNotificationConfiguration("AnySink1", notificationConfigurations));
}

/**
 * @brief : Test to verify the hookSourceNotificationDataChanged function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSourceNotificationDataChanged" function when set the value for am_NotificationPayload_s
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & am_NotificationConfiguration_s details & sourceName are pass as input parameter along with other configuration details
 *         to hookSourceNotificationDataChanged then will check whether it providing the interface to framework
 *         to pass the hook of source notification data change from application to policy engine
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSourceNotificationDataChanged function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSourceNotificationDataChangedPositive)
{

    sourceName = "MediaPlayer";
    am_NotificationPayload_s payload;
    payload.type  = NT_OVER_TEMPERATURE;
    payload.value = 20;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSourceNotificationDataChanged(sourceName, payload));
}

/**
 * @brief : Test to verify the hookSourceNotificationDataChanged function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSourceNotificationDataChanged" function when set the value for am_NotificationPayload_s
 *         & also set the behavior of setListActions to return "E_NOT_POSSIBLE"  with parameter as "AL_NORMAL".
 *         & am_NotificationPayload_s details & sourceName are pass as input parameter along with other configuration details
 *         to hookSourceNotificationDataChanged then will check whether it providing the interface to framework
 *         to pass the hook of source notification data change from application to policy engine
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSourceNotificationDataChanged function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSourceNotificationDataChangedNegative)
{

    sourceName = "MediaPlayer";
    am_NotificationPayload_s payload;
    payload.type  = NT_OVER_TEMPERATURE;
    payload.value = 20;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->hookSourceNotificationDataChanged("AnySource1", payload));
}

/**
 * @brief : Test to verify the hookSinkNotificationDataChanged function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSinkNotificationDataChanged" function when set the value for am_NotificationPayload_s
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & am_NotificationConfiguration_s details & sinkName are pass as input parameter along with other configuration details
 *         to hookSinkNotificationDataChanged then will check whether it providing the interface to framework
 *         to pass the hook of sink notification data change from application to policy engine
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSinkNotificationDataChanged function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSinkNotificationDataChangedPositive)
{

    sinkName = "AMP";
    am_NotificationPayload_s payload;
    payload.type  = NT_OVER_TEMPERATURE;
    payload.value = 20;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSinkNotificationDataChanged(sinkName, payload));
}

/**
 * @brief : Test to verify the hookSinkNotificationDataChanged function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSinkNotificationDataChanged" function when set the value for am_NotificationPayload_s
 *         & also set the behavior of setListActions to return "E_NOT_POSSIBLE"  with parameter as "AL_NORMAL".
 *         & am_NotificationConfiguration_s details & sinkName are pass as input parameter along with other configuration details
 *         to hookSinkNotificationDataChanged then will check whether it providing the interface to framework
 *         to pass the hook of sink notification data change from application to policy engine
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when hookSinkNotificationDataChanged function return the "E_NOT_POSSIBLE" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSinkNotificationDataChangedNegative)
{

    sinkName = "AMP";
    am_NotificationPayload_s payload;
    payload.type  = NT_OVER_TEMPERATURE;
    payload.value = 20;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->hookSinkNotificationDataChanged("AnySink1", payload));
}

/**
 * @brief : Test to verify the hookConnectionStateChange function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookConnectionStateChange" function when set the value for am_ConnectionState_e
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & am_ConnectionState_e details & connectionState,status are pass as input parameter along with other configuration details
 *         to hookConnectionStateChange then will check whether it providing the interface to framework
 *         to pass the hook of connect state change request or not, without any Gmock error.
 *
 * @result : "Pass" when hookConnectionStateChange function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookConnectionStateChangePositive)
{

    std::string          connectionName  = "MediaPlayer:AMP";
    am_ConnectionState_e connectionState = CS_CONNECTING;
    am_Error_e           status          = E_OK;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookConnectionStateChange(connectionName, connectionState, status));
}

/**
 * @brief : Test to verify the hookStoredMainConnectionVolume function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookStoredMainConnectionVolume" function when set the value for am_mainVolume_t
 *         & also set the behavior of setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & connectionName & mainVolume are pass as input parameter along with other configuration details
 *         to hookStoredMainConnectionVolume then will check whether it providing the interface to framework
 *         to pass the hook of main connection volume or not, without any Gmock error.
 *
 * @result : "Pass" when hookStoredMainConnectionVolume function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookStoredMainConnectionVolumePositive)
{
    std::string     connectionName = "MediaPlayer:AMP";
    am_mainVolume_t mainVolume     = 10;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookStoredMainConnectionVolume(connectionName, mainVolume));
}

/**
 * @brief : Test to verify the hookSetSinkMuteState function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSetSinkMuteState" function when set the value for gc_Element_e,gc_ConnectionInfo_s & listConnectionInfos
 *         & also set the behavior of getListMainConnections,setListActions to return "E_OK"  with parameter as "AL_NORMAL".
 *         & sinkName & muteState are pass as input parameter along with other configuration details
 *         to hookSetSinkMuteState then will check whether it providing the interface to framework
 *         to pass the hook of change mute state request from application to policy engine or not, without any Gmock error.
 *
 * @result : "Pass" when hookSetSinkMuteState function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSetSinkMuteStatePositive)
{
    std::string    className   = "NAVI";
    std::string    sourceName  = "MediaPlayer";
    std::string    sinkName    = "Gateway1";
    am_MuteState_e muteState   = MS_MUTED;
    gc_Element_e   elementType = ET_CLASS;
    // Connection information

    ConnectionInfo.sourceName      = "MediaPlayer";
    ConnectionInfo.sinkName        = "Gateway1";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;

    std::string    className1  = "BASE";
    std::string    sourceName1 = "Gateway0";
    std::string    sinkName1   = "Gateway0";
    am_MuteState_e muteState1  = MS_MUTED;

    // Connection information
    gc_ConnectionInfo_s ConnectionInfo1;
    ConnectionInfo1.sourceName      = "Gateway0";
    ConnectionInfo1.sinkName        = "Gateway0";
    ConnectionInfo1.priority        = 0;
    ConnectionInfo1.connectionState = CS_CONNECTED;
    ConnectionInfo1.volume          = 10;

    std::vector<gc_ConnectionInfo_s >listConnectionInfos;
    listConnectionInfos.push_back(ConnectionInfo);
    listConnectionInfos.push_back(ConnectionInfo1);

    EXPECT_CALL(*mpMockIAmPolicyReceive, getListMainConnections(elementType, className, _)).WillRepeatedly(DoAll(SetArgReferee<2>(listConnectionInfos), Return(E_OK)));
    EXPECT_CALL(*mpMockIAmPolicyReceive, getListMainConnections(elementType, className1, _)).WillRepeatedly(DoAll(SetArgReferee<2>(listConnectionInfos), Return(E_OK)));
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmPolicySend->hookSetSinkMuteState(sinkName, muteState));
}

/**
 * @brief : Test to verify the hookSetSinkMuteState function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "hookSetSinkMuteState" function when set the value for gc_Element_e,gc_ConnectionInfo_s & listConnectionInfos
 *         & also set the behavior of getListMainConnections,setListActions to return "E_NOT_POSSIBLE"  with parameter as "AL_NORMAL".
 *         & sinkName & muteState are pass as input parameter along with other configuration details
 *         to hookSetSinkMuteState then will check whether it providing the interface to framework
 *         to pass the hook of change mute state request from application to policy engine or not, without any Gmock error.
 *
 * @result : "Pass" when hookSetSinkMuteState function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, hookSetSinkMuteStateNegative)
{

    std::string    className  = "NAVI";
    std::string    sourceName = "MediaPlayer";
    std::string    sinkName   = "Gateway1";
    am_MuteState_e muteState  = MS_MUTED;

    gc_Element_e elementType = ET_CLASS;
    // Connection information

    ConnectionInfo.sourceName      = "MediaPlayer";
    ConnectionInfo.sinkName        = "Gateway1";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;

    std::string    className1  = "BASE";
    std::string    sourceName1 = "Gateway0";
    std::string    sinkName1   = "Gateway0";
    am_MuteState_e muteState1  = MS_MUTED;

    // Connection information
    gc_ConnectionInfo_s ConnectionInfo1;
    ConnectionInfo1.sourceName      = "Gateway0";
    ConnectionInfo1.sinkName        = "Gateway0";
    ConnectionInfo1.priority        = 0;
    ConnectionInfo1.connectionState = CS_CONNECTED;
    ConnectionInfo1.volume          = 10;

    std::vector<gc_ConnectionInfo_s >listConnectionInfos;
    listConnectionInfos.push_back(ConnectionInfo);
    listConnectionInfos.push_back(ConnectionInfo1);

    EXPECT_CALL(*mpMockIAmPolicyReceive, getListMainConnections(elementType, className, _)).WillRepeatedly(DoAll(SetArgReferee<2>(listConnectionInfos), Return(E_NOT_POSSIBLE)));
    EXPECT_CALL(*mpMockIAmPolicyReceive, getListMainConnections(elementType, className1, _)).WillRepeatedly(DoAll(SetArgReferee<2>(listConnectionInfos), Return(E_NOT_POSSIBLE)));
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmPolicySend->hookSetSinkMuteState("AnySink1", muteState));
}

/**
 * @brief : Test to verify the getListElements of source function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "getListElements" function when set the value for listNames of source list
 *         & listNames & listSources are pass as input parameter along with other configuration details
 *         to getListElements then will check whether it returns the list of configuration data of
 *         a source by list of names or not, without any Gmock error.
 *
 * @result : "Pass" when getListElements function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, getListElementspositiveSource)
{

    sourceName = "MediaPlayer";
    std::vector<std::string >listNames;
    listNames.push_back(sourceName);
    std::vector<gc_Source_s >listSources;
    EXPECT_EQ(E_OK, mpCAmPolicySend->getListElements(listNames, listSources));
}

/**
 * @brief : Test to verify the getListElements of source function negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "getListElements" function when set the value for listNames of source list
 *         & listNames & listSources are pass as input parameter along with other configuration details
 *         to getListElements then will check whether it returns the list of configuration data of
 *         a source by list of names or not, without any Gmock error.
 *
 * @result : "Pass" when getListElements function return the "E_UNKNOWN" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, getListElementsnegativeSource)
{

    sourceName = "AnySource1";
    std::vector<std::string >listNames;
    listNames.push_back(sourceName);
    std::vector<gc_Source_s >listSources;
    EXPECT_EQ(E_UNKNOWN, mpCAmPolicySend->getListElements(listNames, listSources));
}

/**
 * @brief : Test to verify the getListElements of sink function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "getListElements" function when set the value for listNames of sink list
 *         & listNames & listSinks are pass as input parameter along with other configuration details
 *         to getListElements then will check whether it returns the list of configuration data of
 *         a sink by list of names or not, without any Gmock error.
 *
 * @result : "Pass" when getListElements function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, getListElementspositiveSink)
{

    sinkName = "Gateway1";
    std::vector<std::string >listNames;
    listNames.push_back(sinkName);
    std::vector<gc_Sink_s >listSinks;
    EXPECT_EQ(E_OK, mpCAmPolicySend->getListElements(listNames, listSinks));
}

/**
 * @brief : Test to verify the getListElements of sink function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "getListElements" function when set the value for listNames of sink list
 *         & listNames & listSinks are pass as input parameter along with other configuration details
 *         to getListElements then will check whether it returns the list of configuration data of
 *         a sink by list of names or not, without any Gmock error.
 *
 * @result : "Pass" when getListElements function return the "E_UNKNOWN" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, getListElementsnegativeSink)
{

    sinkName = "AnySink1";
    std::vector<std::string >listNames;
    listNames.push_back(sinkName);
    std::vector<gc_Sink_s >listSinks;
    EXPECT_EQ(E_UNKNOWN, mpCAmPolicySend->getListElements(listNames, listSinks));

}

/**
 * @brief : Test to verify the getListElements of gateway function positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "getListElements" function when set the value for listNames of gateway list
 *         & listNames & listGateways are pass as input parameter along with other configuration details
 *         to getListElements then will check whether it returns the list of configuration data of
 *         a gateway by list of names or not, without any Gmock error.
 *
 * @result : "Pass" when getListElements function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, getListElementspositiveGateway)
{

    std::string              gatewayName = "Gateway1";
    std::vector<std::string >listNames;
    listNames.push_back(gatewayName);
    std::vector<gc_Gateway_s >listGateways;
    EXPECT_EQ(E_OK, mpCAmPolicySend->getListElements(listNames, listGateways));
}

/**
 * @brief : Test to verify the getListElements of gateway function Negative scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "getListElements" function when set the value for listNames of gateway list
 *         & listNames & listGateways are pass as input parameter along with other configuration details
 *         to getListElements then will check whether it returns the list of configuration data of
 *         a gateway by list of names or not, without any Gmock error.
 *
 * @result : "Pass" when getListElements function return the "E_UNKNOWN" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, getListElementsnegativeGateWay)
{

    std::string              gatewayName = "AnyGateway1";
    std::vector<std::string >listNames;
    listNames.push_back(gatewayName);
    std::vector<gc_Gateway_s >listGateways;
    EXPECT_EQ(E_UNKNOWN, mpCAmPolicySend->getListElements(listNames, listGateways));
}

/**
 * @brief Test to verify the getListDomains function positive scenario
 *
 * @test  verify the "getListDomains()" function of the CAmConfigurationReader class will
 *        populate the given list from configuration without any Gmock error message.
 */
TEST_F(CAmPolicySendTest, getListDomains_positive)
{

    std::string domainName = "Applications";
    gc_Domain_s foundDomain;

    EXPECT_EQ(E_OK, CAmConfigurationReader::instance().getElementByName(domainName, foundDomain));
}

/**
 * @brief  Test to verify the getElementByName of domain function Negative scenario
 *
 * @test  verify the "getElementByName()" function of the CAmConfigurationReader class will
 *        properly report error condition when asked for a non-configured domain.
 */
TEST_F(CAmPolicySendTest, getListElementsnegativedomain)
{

    std::string domainName = "AnnDomain1";
    gc_Domain_s foundDomain;

    EXPECT_EQ(E_UNKNOWN, CAmConfigurationReader::instance().getElementByName(domainName, foundDomain));
}

/**
 * @brief : Test to verify the getListSystemProperties for positive scenario
 *
 * @pre : Mock the IAmControlReceive,CAmControlReceive classes and also initialized the CAmControlReceive,CAmPolicySend class.
 *
 * @test : verify the "getListSystemProperties" function when set the value for listSystemProperties list
 *         & listSystemProperties is pass as input parameter along with other configuration details
 *         to getListSystemProperties then will check whether it returns the list of SystemProperties
 *         or not, without any Gmock error.
 *
 * @result : "Pass" when getListSystemProperties function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicySendTest, getListSystemProperties)
{

    std::vector<gc_SystemProperty_s > listSystemProperties;
    CAmConfigurationReader::instance().getListSystemProperties(listSystemProperties);
    EXPECT_EQ(2, listSystemProperties.size());
}

/**
 * @brief Test to verify the getListClasses for positive scenario
 *
 * @test  verify that "getListClasses()" function of the CAmConfigurationReader class will
 *        populate the given list from configuration without any Gmock error message.
 */
TEST_F(CAmPolicySendTest, getListClasses)
{

    std::vector<gc_Class_s >listClasses;
    EXPECT_EQ(E_OK, CAmConfigurationReader::instance().getListClasses(listClasses));
}

/**
 * @brief Test to verify the getListDomains for positive scenario
 *
 * @test  verify that "getListDomains()" function of the CAmConfigurationReader class will
 *        populate the given list from configuration without any Gmock error message.
 */
TEST_F(CAmPolicySendTest, getListDomains)
{

    std::vector<gc_Domain_s >listDomains;
    EXPECT_EQ(E_OK, CAmConfigurationReader::instance().getListDomains(listDomains));
}

int main(int argc, char * *argv)
{
    am::CAmLogWrapper::instantiateOnce("UTST", "Unit test for generic controller, class CAmPolicySend"
            , LS_ON, LOG_SERVICE_STDOUT);
    LOG_FN_CHANGE_LEVEL(LL_WARN);

    // redirect configuration path
    gc_utest::ConfigDocument config(gc_utest::ConfigDocument::Default);

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
