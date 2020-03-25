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
#include "CAmRouteElement.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmRouteElementTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;
#define DOMAINID 2;

/**
 *@Class : CAmRouteElementTest
 *@brief : This class is used to test the CAmRouteElement class functionality.
 */
CAmRouteElementTest::CAmRouteElementTest()
    : mpMockControlReceiveInterface(NULL)
    , mpControlReceive(NULL)
    , mpSinkElement(NULL)
    , mpSourcmeElement(NULL)
    , mpCAmRouteElement(NULL)
    , priority(0)
{

}

CAmRouteElementTest::~CAmRouteElementTest()
{
}

void CAmRouteElementTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpControlReceive              = mpMockControlReceiveInterface;
    // This will check whether controller is initialized or not
    ASSERT_THAT(mpControlReceive, NotNull()) << " Controller Not Initialized";
    // Initialize Sink Element
    ut_gc_Sink_s sinkInfo("AnySink1");
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSinkElement = CAmSinkFactory::createElement(sinkInfo, mpControlReceive);
    ASSERT_THAT(mpSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpSinkElement->getID());
    // Initialize Source Element
    ut_gc_Source_s sourceInfo("AnySource1");
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSourcmeElement = CAmSourceFactory::createElement(sourceInfo, mpControlReceive);
    ASSERT_THAT(mpSourcmeElement, NotNull()) << " Source Element Is not Created";
    sourceIDList.push_back(mpSourcmeElement->getID());
    // creating the routing element
    gc_RoutingElement_s gcRoutingElement;
    gcRoutingElement.name             = mpSourcmeElement->getName() + mpSinkElement->getName();
    gcRoutingElement.sourceID         = mpSourcmeElement->getID();
    gcRoutingElement.sinkID           = mpSinkElement->getID();
    gcRoutingElement.domainID         = DOMAINID;
    gcRoutingElement.connectionFormat = CF_GENIVI_STEREO;
    mpCAmRouteElement                 = new CAmRouteElement(gcRoutingElement, mpControlReceive);
    ASSERT_THAT(mpCAmRouteElement, NotNull()) << " Route Element Is not Created";
}

void CAmRouteElementTest::TearDown()
{
    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }

    if (mpCAmRouteElement != NULL)
    {
        delete (mpCAmRouteElement);
        mpCAmRouteElement = NULL;
    }
}

/**
 * @brief : Test to verify the getPriority function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *
 * @test : verify the "getPriority" function when priority is given as input parameters
 *         and along with other configuration parameters then will check whether getPriority function
 *         will return the priority which is addition of sink Priority and source Priority or not.
 *         ,without any gmock error.
 *
 * @result : "Pass" when getPriority function return "E_OK" without any Gmock error message
 */
TEST_F(CAmRouteElementTest, getPriorityPositive)
{
    EXPECT_EQ(2, mpCAmRouteElement->getPriority());
}

/**
 * @brief : Test to verify the getPriority function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *
 * @test : verify the "getPriority" function when priority is given as input parameters
 *         and along with other configuration parameters then will check whether getPriority function
 *         will return the addition of sink Priority and source Priority or not.
 *         ,without any gmock error.
 *
 * @result : "Pass" when getPriority function will not return value 10 [any other value
 *           expect 2 as sink priority and source priority is 1] without any Gmock error message.
 */

TEST_F(CAmRouteElementTest, getPriorityNegative)
{
    EXPECT_NE(10, mpCAmRouteElement->getPriority());
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
