/*
 * CAmActionCommandTest.cpp
 *
 *  Created on: 11-May-2016
 *      Author: dipesh
 */

#include "MockCAmActionCommandTest.h"
#include "CAmActionCommandTest.h"
#include "CAmActionCommand.h"
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"
// #include "time.h"

using namespace testing;
using namespace am;
using namespace gc;

DltContext GenericControllerDlt; /*This added only for compilation purpose*/

/*
 * Here the  test CAmActionCommandTest get executed for the following three test cases
 * 1.updateTest
 * 2.undoTest
 * 3.executeTest
 */

/**
 * @brief The test-case  undoTest is used to test undo method of class CAmActionCommand.
 *
 * The update method will expect a call getStatus(),getUndoRequired().
 *
 *
 */
TEST(CAmActionCommandTest, undoTest)
{
    MockCAmActionCommand pMockCAmActionCommand;
    EXPECT_CALL(pMockCAmActionCommand, getStatus()).WillOnce(Return(AS_COMPLETED));
    EXPECT_CALL(pMockCAmActionCommand, getUndoRequired()).WillOnce(Return(false));
    EXPECT_CALL(pMockCAmActionCommand, getError()).WillOnce(Return(0)).WillOnce(Return(0));

    EXPECT_EQ(0, pMockCAmActionCommand.undo());
}

/**
 * @brief The test-case  undoTest1 is used to test undo method of class CAmActionCommand
 *       when action status is other than "AS_COMPLETED".
 *
 * The update method will expect a call getStatus().
 *
 *
 */
TEST(CAmActionCommandTest, undoTest1)
{
    MockCAmActionCommand pMockCAmActionCommand;
    EXPECT_CALL(pMockCAmActionCommand, getStatus()).WillOnce(Return(AS_NOT_STARTED));

    EXPECT_EQ(0, pMockCAmActionCommand.undo());
}

/**
 * @brief The test-case executeTest is used to test execute method of class CAmActionCommand.
 *
 * The update method will expect a call getStatus(),getTimeout() and getError() method.
 *
 *
 */
TEST(CAmActionCommandTest, executeTest)
{
    MockCAmActionCommand pMockCAmActionCommand;
    EXPECT_CALL(pMockCAmActionCommand, getStatus()).WillOnce(Return(AS_NOT_STARTED));
    EXPECT_CALL(pMockCAmActionCommand, getTimeout()).WillOnce(Return(INFINITE_TIMEOUT));
    EXPECT_CALL(pMockCAmActionCommand, getError()).WillOnce(Return(0)).WillOnce(Return(0));
    EXPECT_CALL(pMockCAmActionCommand, _execute()).WillOnce(Return(0));

    EXPECT_EQ(0, pMockCAmActionCommand.execute());

}

/**
 * @brief The test-case executeTest1 is used to test execute method of class CAmActionCommand
 * in which the _execute method should not get call when action status is other than "AS_NOT_STARTED".
 *
 * The update method will expect a call getStatus(),getTimeout() and getError() method.
 *
 *
 */
TEST(CAmActionCommandTest, executeTest1)
{
    MockCAmActionCommand pMockCAmActionCommand;
    EXPECT_CALL(pMockCAmActionCommand, getStatus()).WillOnce(Return(AS_COMPLETED));
    EXPECT_EQ(0, pMockCAmActionCommand.execute());

}

/**
 * @brief The test-case updateTest is used to test update method of class CAmActionCommand.
 *
 * The update method will expect a call getStatus(),getError() and _update() method.
 *
 *
 */

TEST(CAmActionCommandTest, updateTest)
{
    MockCAmActionCommand pMockCAmActionCommand;
    const int            result = 5; /* Any positive value */
    EXPECT_CALL(pMockCAmActionCommand, getStatus()).WillOnce(Return(AS_EXECUTING)).WillOnce(Return(AS_EXECUTING));
    EXPECT_CALL(pMockCAmActionCommand, getError()).WillOnce(Return(result)).WillOnce(Return(result));
    EXPECT_CALL(pMockCAmActionCommand, _update(result)).WillOnce(Return(0));

    EXPECT_EQ(0, pMockCAmActionCommand.update(result));

}

int main(int argc, char * *argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    /*All the test cases for CAmActionCommandTest test are run */
    return RUN_ALL_TESTS();
}
