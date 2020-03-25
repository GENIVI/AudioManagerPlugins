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
#include "CAmRouteElement.h"
#include "IAmEventObserver.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmEventSubjectTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmEventSubjectTest
 *@brief : This class is used to test the CAmEventSubject functionality.
 */
CAmEventSubjectTest::CAmEventSubjectTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmEventSubject(NULL)
    , pEventObserver(NULL)
{
}

CAmEventSubjectTest::~CAmEventSubjectTest()
{

}

void CAmEventSubjectTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;
    ASSERT_THAT(mpCAmControlReceive, NotNull()) << " Controller Not Initialized";
    mpCAmEventSubject = new CAmEventSubject();
    ASSERT_THAT(mpCAmControlReceive, NotNull()) << " CAmEventSubject is Not Initialized";

}

void CAmEventSubjectTest::TearDown()
{
    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }

    if (mpCAmEventSubject != NULL)
    {
        delete (mpCAmEventSubject);
    }
}

/**
 * @brief : Test to verify the registerObserver function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        and also it will initialized the CAmEventSubject class.
 *
 * @test : verify the "registerObserver" function when pointer of IAmEventObserver is passed
 *         and along with other configuration parameters then will check whether registerObserver function
 *         will push the event observer in the list with any Gmock error.
 *
 * @result : "Pass" when registerObserver function return "E_OK" without any Gmock error message
 */
TEST_F(CAmEventSubjectTest, registerObserverPositive)
{
    EXPECT_EQ(E_OK, mpCAmEventSubject->registerObserver(pEventObserver));
}

/**
 * @brief : Test to verify the registerObserver function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        and also it will initialized the CAmEventSubject class.
 *
 * @test : verify the "registerObserver" function when pointer of IAmEventObserver is passed to function as Null.
 *         and along with other configuration parameters then will check whether registerObserver function
 *         will not push the event observer in the list with any Gmock error.
 *
 * @result : "Pass" when registerObserver function return "E_OK" without any Gmock error message
 */
TEST_F(CAmEventSubjectTest, registerObserverNegative)
{
    pEventObserver = NULL;
    EXPECT_EQ(E_OK, mpCAmEventSubject->registerObserver(pEventObserver));
}

/**
 * @brief : Test to verify the unregisterObserver function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        and also it will initialized the CAmEventSubject class.
 *
 * @test : verify the "unregisterObserver" function when pointer of IAmEventObserver is passed
 *         and along with other configuration parameters and also it is first call registerObserver then will check whether unregisterObserver function
 *         will erase the event observer in the list with any Gmock error.
 *
 * @result : "Pass" when unregisterObserver function return "E_OK" without any Gmock error message
 */
TEST_F(CAmEventSubjectTest, unregisterObserverPositive)
{
    EXPECT_EQ(E_OK, mpCAmEventSubject->registerObserver(pEventObserver));
    EXPECT_EQ(E_OK, mpCAmEventSubject->unregisterObserver(pEventObserver));
}

/**
 * @brief : Test to verify the unregisterObserver function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        and also it will initialized the CAmEventSubject class.
 *
 * @test : verify the "unregisterObserver" function when pointer of IAmEventObserver is passed to function as Null.
 *         and along with other configuration parameters and also it is first call registerObserver then will check whether unregisterObserver function
 *         will not erase the event observer in the list with any Gmock error.
 *
 * @result : "Pass" when unregisterObserver function return "E_OK" without any Gmock error message
 */
TEST_F(CAmEventSubjectTest, unregisterObserverNegative)
{
    EXPECT_EQ(E_OK, mpCAmEventSubject->registerObserver(pEventObserver));
    pEventObserver = NULL;
    EXPECT_EQ(E_OK, mpCAmEventSubject->unregisterObserver(pEventObserver));
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
