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

#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"

#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmActionCommandTest.h"
#include "CAmActionCommand.h"
#include "MockCAmActionCommandTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmActionCommandTest
 *@brief : This class is used to test the CAmActionCommand functionality.
 */
CAmActionCommandTest::CAmActionCommandTest()
{
}

CAmActionCommandTest::~CAmActionCommandTest()
{

}

void CAmActionCommandTest::SetUp()
{

}

void CAmActionCommandTest::TearDown()
{

}

/**
 * @brief : Test to verify the undo function
 *
 * @pre : Mock the CAmActionCommand class
 *
 * @test : verify the "undo" function when set the behavior of getStatus to return the "AS_COMPLETED".
 *         & set the behavior of getUndoRequired to return the flase.
 *         then check whether "undo" function will be performed undo only for completed actions or not,
 *         & set the action state by using setStatus or not, without any gmock error.
 *
 * @result : "Pass" when undo function return the expected output "zero".
 */
TEST_F(CAmActionCommandTest, undoTest)
{
    MockCAmActionCommand pMockCAmActionCommand;
    EXPECT_CALL(pMockCAmActionCommand, getStatus()).WillOnce(Return(AS_COMPLETED));
    EXPECT_CALL(pMockCAmActionCommand, getUndoRequired()).WillOnce(Return(false));
    EXPECT_CALL(pMockCAmActionCommand, getError()).WillOnce(Return(0)).WillOnce(Return(0));
    EXPECT_EQ(0, pMockCAmActionCommand.undo());

}

/**
 * @brief : Test to verify the undo function
 *
 * @pre : Mock the CAmActionCommand class
 *
 * @test : verify the "undo" function when set the behavior of getStatus to return the "AS_NOT_STARTED",
 *         then check whether "undo" function will be performed undo only for completed actions or not,
 *         & set the action state by using setStatus or not, without any gmock error.
 *
 * @result : "Pass" when undo function return the expected output "zero".
 */
TEST_F(CAmActionCommandTest, undoTest1)
{
    MockCAmActionCommand pMockCAmActionCommand;
    EXPECT_CALL(pMockCAmActionCommand, getStatus()).WillOnce(Return(AS_NOT_STARTED));
    EXPECT_EQ(0, pMockCAmActionCommand.undo());
}

/**
 * @brief : Test to verify the _execute function
 *
 * @pre : Mock the CAmActionCommand class
 *
 * @test : verify the "_execute" function when set the behavior of getStatus to return the "AS_NOT_STARTED".
 *         & set the behavior of getTimeout to return the INFINITE_TIMEOUT.
 *         then check whether "_execute" function will get called or not only for the AS_NOT_STARTED state
 *         & it executes the action or not & set the timer or not by using setTimer & then call the _execute
 *         & set the action state by using setStatus or not, without any gmock error.
 *
 * @result : "Pass" when _execute function return the expected output "zero".
 */
TEST_F(CAmActionCommandTest, executeTest)
{
    MockCAmActionCommand pMockCAmActionCommand;
    EXPECT_CALL(pMockCAmActionCommand, getStatus()).WillOnce(Return(AS_NOT_STARTED));
    EXPECT_CALL(pMockCAmActionCommand, getTimeout()).WillOnce(Return(INFINITE_TIMEOUT));
    EXPECT_CALL(pMockCAmActionCommand, getError()).WillOnce(Return(0)).WillOnce(Return(0));
    EXPECT_CALL(pMockCAmActionCommand, _execute()).WillOnce(Return(0));

    EXPECT_EQ(0, pMockCAmActionCommand.execute());

}

/**
 * @brief : Test to verify the _execute function
 *
 * @pre : Mock the CAmActionCommand class
 *
 * @test : verify the "_execute" function when set the behavior of getStatus to return the "AS_COMPLETED".
 *         then check whether "_execute" function will get called or not, only for the AS_NOT_STARTED & it executes the action or not
 *         & set the timer by using setTimer & then call the _execute
 *         & set the action state by using setStatus or not, without any gmock error.
 *
 * @result : "Pass" when _execute function return the expected output "zero".
 */
TEST_F(CAmActionCommandTest, executeTest1)
{
    MockCAmActionCommand pMockCAmActionCommand;
    EXPECT_CALL(pMockCAmActionCommand, getStatus()).WillOnce(Return(AS_COMPLETED));
    EXPECT_EQ(0, pMockCAmActionCommand.execute());

}

/**
 * @brief : Test to verify the update function
 *
 * @pre : Mock the CAmActionCommand class
 *
 * @test : verify the "update" function when set the behavior of getStatus to return the "AS_EXECUTING".
 *         & set the behavior of getError function to return the positive value
 *         then check whether "update" function will update the completion of the child action or the
 *         completion of undo operation or not, & set the status as AS_COMPLETED or not, without any gmock error.
 *
 * @result : "Pass" when update function return the expected output "zero".
 */
TEST_F(CAmActionCommandTest, updateTest)
{
    MockCAmActionCommand pMockCAmActionCommand;
    const int            result = 5; /* Any positive value */
    EXPECT_CALL(pMockCAmActionCommand, getStatus()).WillOnce(Return(AS_EXECUTING));
    EXPECT_CALL(pMockCAmActionCommand, getError()).WillOnce(Return(result)).WillOnce(Return(result));
    EXPECT_CALL(pMockCAmActionCommand, _update(result)).WillOnce(Return(0));

    EXPECT_EQ(0, pMockCAmActionCommand.update(result));

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
