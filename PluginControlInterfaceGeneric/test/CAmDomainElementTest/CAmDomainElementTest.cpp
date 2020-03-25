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
#include "CAmDomainElement.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmDomainElementTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmDomainElementTest
 *@brief : This class is used to test the CAmDomainElement class functionality.
 */
CAmDomainElementTest::CAmDomainElementTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmDomainElement(NULL)
    , domainInfo("AnyDomain1")
{
}

CAmDomainElementTest::~CAmDomainElementTest()
{
}

void CAmDomainElementTest::InitializeCommonStruct()
{
    domain.name                 = "AnyDomain1";
    domain.domainID             = 22;
    state                       = DS_CONTROLLED;
    domainInfo.busname          = "AnyBus";
    domainInfo.nodename         = "AnyNode";
    domainInfo.early            = true;
    domainInfo.complete         = false;
    domainInfo.state            = DS_UNKNOWN;
    domainInfo.registrationType = REG_CONTROLLER;
    listDomains.push_back(domain);

}

void CAmDomainElementTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;
    // This will check whether controller is initialized or not
    ASSERT_THAT(mpCAmControlReceive, NotNull()) << " Controller Not Initialized";
    InitializeCommonStruct();
    mpCAmDomainElement = new CAmDomainElement(domainInfo, mpCAmControlReceive);
    mpCAmDomainElement->setID(22);
    domainID = mpCAmDomainElement->getID();

}

void CAmDomainElementTest::TearDown()
{
    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }

    if (mpCAmDomainElement != NULL)
    {
        delete mpCAmDomainElement;
        mpCAmDomainElement = NULL;
    }
}

/**
 * @brief : Test to verify the setState function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Domain Elements are created with required input structure parameters
 *        like name,domainID,bus name ,state, node name,registration type etc.
 *
 * @test : verify the "setState" function when state is given as input parameters
 *         and along with other configuration parameters and here we have mock the changeDominStateDB with state
 *         and domainID and to return E_OK. then will check whether setState function
 *         will set the state or not, with out any gmock error.
 *
 * @result : "Pass" when setState function return "E_OK" without any Gmock error message
 */
TEST_F(CAmDomainElementTest, setStatePositive)
{

    EXPECT_CALL(*mpMockControlReceiveInterface, changDomainStateDB(state, domainID)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmDomainElement->setState(state));
}

/**
 * @brief : Test to verify the setState function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Domain Elements are created with required input structure parameters
 *        like name,domainID,bus name ,state, node name,registration type etc.
 *
 * @test : verify the "setState" function when state is given as input parameters
 *         and along with other configuration parameters and Here we have mock the changeDominStateDB function
 *         to return data base error messgae.then will check whether setState function
 *         will set the state or not with out any gmock error.
 *
 * @result : "Pass" when setState function return "E_DATABASE_ERROR" without any Gmock error message
 */
TEST_F(CAmDomainElementTest, setStateNegative)
{

    EXPECT_CALL(*mpMockControlReceiveInterface, changDomainStateDB(state, domainID)).WillOnce(Return(E_DATABASE_ERROR));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmDomainElement->setState(state));
}

/**
 * @brief : Test to verify the getState function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Domain Elements are created with required input structure parameters
 *        like name,domainID,bus name ,state, node name,registration type etc.
 *
 * @test : verify the "getState" function when state is given as input parameters
 *         and along with other configuration parameters and here we have mock the
 *         changeDominStateDB with state and domainID and to return E_OK.
 *         then will check whether getState function
 *         will get the state or not with out any gmock error.
 *
 * @result : "Pass" when getState function return "E_OK" without any Gmock error message
 */
TEST_F(CAmDomainElementTest, getStatePositive)
{
    am_DomainState_e setstate = DS_CONTROLLED;
    EXPECT_CALL(*mpMockControlReceiveInterface, changDomainStateDB(setstate, domainID)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmDomainElement->setState(setstate));
    int state;
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmDomainElement->getState(state));
}

/**
 * @brief : Test to verify the getState function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Domain Elements are created with required input structure parameters
 *        like name,domainID,bus name ,state, node name,registration type etc.
 *
 * @test : verify the "getState" function when state is given as input parameters
 *         and along with other configuration parameters then will check whether getState function
 *         will get the state or not with out any gmock error.
 *
 * @result : "Pass" when getState function return "E_UNKNOWN" without any Gmock error message
 */
TEST_F(CAmDomainElementTest, getStateNegative)
{
    am_DomainState_e setstate = DS_CONTROLLED;
    mpCAmDomainElement->setID(21);
    domainID = mpCAmDomainElement->getID();
    EXPECT_CALL(*mpMockControlReceiveInterface, changDomainStateDB(setstate, domainID)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmDomainElement->setState(setstate));
    int state;
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_)).WillRepeatedly(DoAll(SetArgReferee<0>(listDomains), Return(E_OK)));
    EXPECT_EQ(E_UNKNOWN, mpCAmDomainElement->getState(state));
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
