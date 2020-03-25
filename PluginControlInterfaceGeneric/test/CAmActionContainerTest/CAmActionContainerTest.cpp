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
#include <time.h>
#include "MockCAmActionContainerTest.h"
#include "CAmActionContainerTest.h"
#include "CAmTestConfigurations.h"
#include "CAmActionContainer.h"
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"
#include "CAmActionCommand.h"
#include "CAmTimerEvent.h"
#include "CAmControllerPlugin.h"
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmActionContainerTest
 *@brief : This class is used to test the CAmActionContainer functionality.
 */
CAmActionContainerTest::CAmActionContainerTest()
{
}

CAmActionContainerTest::~CAmActionContainerTest()
{

}

void CAmActionContainerTest::SetUp()
{

    using ::testing::InSequence;

}

void CAmActionContainerTest::TearDown()
{

}

/**
 * @brief : Test to verify the insert function
 *
 * @pre : Mock the CAmActionCommandChild class
 *
 * @test : verify the "insert" function when instance of mockCAmActionCommandChild1 is append in the
 *         CAmActionContainer class then check whether "insert" function will add an action just
 *         after the currently executing action or not,without any gmock error.
 *
 * @result : "Pass" when insert function return the expected output "E_OK".
 */
TEST_F(CAmActionContainerTest, insertTest)
{
    MockCAmActionCommandChild MockCAmActionCommandChild1("Child1");
    MockCAmActionCommandChild MockCAmActionCommandChild2("Child2");
    pCAmActionContainer.append(&MockCAmActionCommandChild1);
    EXPECT_EQ(pCAmActionContainer.insert(&MockCAmActionCommandChild2), E_OK);
}

/**
 * @brief : Test to verify the undo function
 *
 * @pre : Mock the CAmActionCommandChild class
 *
 * @test : verify the "undo" function when instance of mockCAmActionCommandChild1 is append in the
 *         CAmActionContainer class & initialize the CAmControllerPlugin, CAmSocketHandler,CAmTimerEvent class
 *         & set the behavior of _execute,_undo to return E_OK & set the undo flag to true
 *         & set the behavior of getStatus,getError to return the E_OK
 *         then check whether "undo" function will perform the undo for an action.
 *         or not,without any gmock error.
 *
 * @result : "Pass" when undo function return the expected output "E_OK".
 */
TEST_F(CAmActionContainerTest, undoTest)
{
    MockCAmActionCommandChild MockCAmActionCommandChild1("Child1");
    MockCAmActionCommandChild MockCAmActionCommandChild2("Child2");
    TCLAP::CmdLine           *cmd(CAmCommandLineSingleton::instanciateOnce("The team of the AudioManager wishes you a nice day!", ' ', "7.5", true));
    CAmControllerPlugin      *pPlugin        = new CAmControllerPlugin();
    CAmSocketHandler         *pSocketHandler = new CAmSocketHandler();
    CAmTimerEvent            *mpTimerEvent   = CAmTimerEvent::getInstance();
    if (NULL == mpTimerEvent)
    {
        ASSERT_THAT(mpTimerEvent, IsNull()) << "CAmActionContainer class: Not able to create Timer object";
    }

    mpTimerEvent->setSocketHandle(pSocketHandler, pPlugin);
    EXPECT_CALL(MockCAmActionCommandChild1, _execute()).WillOnce(Return(E_OK));
    EXPECT_CALL(MockCAmActionCommandChild2, _execute()).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_CALL(MockCAmActionCommandChild1, _undo()).WillOnce(Return(E_OK));
    EXPECT_CALL(MockCAmActionCommandChild2, _undo()).Times(0);
    MockCAmActionCommandChild1.setUndoRequried(true);
    MockCAmActionCommandChild2.setUndoRequried(true);
    pCAmActionContainer.append(&MockCAmActionCommandChild1);
    pCAmActionContainer.append(&MockCAmActionCommandChild2);
    pCAmActionContainer.setUndoRequried(true);
    EXPECT_EQ(pCAmActionContainer.execute(), 0);
    EXPECT_EQ(pCAmActionContainer.getStatus(), AS_ERROR_STOPPED);
    EXPECT_EQ(pCAmActionContainer.getError(), E_NOT_POSSIBLE);
    EXPECT_EQ(pCAmActionContainer.undo(), E_OK);
    EXPECT_EQ(pCAmActionContainer.getStatus(), AS_UNDO_COMPLETE);
    CAmCommandLineSingleton::deleteInstance();

}

/**
 * @brief : Test to verify the execute function
 *
 * @pre : Mock the CAmActionCommandChild class
 *
 * @test : verify the "execute" function when instance of mockCAmActionCommandChild1 & mockCAmActionCommandChild1
 *         is append in the CAmActionContainer class & initialize the CAmControllerPlugin, CAmSocketHandler,CAmTimerEvent class
 *         & set the behavior of _execute to return E_OK for both instance & set the undo flag to true
 *         then check whether "execute" function will execute an action and all its child actions.
 *         or not,without any Gmock error.
 *
 * @result : "Pass" when execute function return the expected output "zero".
 */
TEST_F(CAmActionContainerTest, executeTest_1)
{
    MockCAmActionCommandChild MockCAmActionCommandChild1("Child1");
    MockCAmActionCommandChild MockCAmActionCommandChild2("Child2");
    // CAmActionContainer pCAmActionContainer;
    ON_CALL(MockCAmActionCommandChild1, _execute()).WillByDefault(Return(E_OK));
    ON_CALL(MockCAmActionCommandChild2, _execute()).WillByDefault(Return(E_OK));
    MockCAmActionCommandChild1.setUndoRequried(true);
    MockCAmActionCommandChild2.setUndoRequried(true);
    pCAmActionContainer.append(&MockCAmActionCommandChild1);
    pCAmActionContainer.append(&MockCAmActionCommandChild2);

    EXPECT_EQ(pCAmActionContainer.execute(), 0);

}

/**
 * @brief : Test to verify the execute function
 *
 * @pre : Mock the CAmActionCommandChild class
 *
 * @test : verify the "execute" function when instance of mockCAmActionCommandChild1 & mockCAmActionCommandChild1
 *         is append in the CAmActionContainer class & initialize the CAmControllerPlugin, CAmSocketHandler,CAmTimerEvent class
 *         & set the behavior of _execute to return E_NOT_POSSIBLE first then E_OK in sequence & set the undo flag to true
 *         then check whether "execute" function will execute an action and all its child actions.
 *         or not,without any Gmock error.
 *
 * @result : "Pass" when execute function return the expected output "zero".
 */
TEST_F(CAmActionContainerTest, executeTest_2)
{
    MockCAmActionCommandChild MockCAmActionCommandChild1("Child1");
    MockCAmActionCommandChild MockCAmActionCommandChild2("Child2");
    ON_CALL(MockCAmActionCommandChild1, _execute()).WillByDefault(Return(E_NOT_POSSIBLE));
    ON_CALL(MockCAmActionCommandChild2, _execute()).WillByDefault(Return(E_OK));
    MockCAmActionCommandChild1.setUndoRequried(true);
    MockCAmActionCommandChild2.setUndoRequried(true);
    pCAmActionContainer.append(&MockCAmActionCommandChild1);
    pCAmActionContainer.append(&MockCAmActionCommandChild2);
    EXPECT_EQ(pCAmActionContainer.execute(), 0);

}

/**
 * @brief : Test to verify the update function
 *
 * @pre : Mock the CAmActionCommandChild class
 *
 * @test : verify the "update" function when instance of mockCAmActionCommandChild1 & mockCAmActionCommandChild1
 *         is append in the CAmActionContainer class & initialize the CAmControllerPlugin,CAmSocketHandler,CAmTimerEvent class
 *         & set the behavior of execute to return zero  & set the undo flag to true
 *         then check whether "update" function with parameter "E_NOT_POSSIBLE" will notify the action about the
 *         child action's execution or undo or not,without any Gmock error.
 *
 * @result : "Pass" when update function return the expected output "zero".
 */
TEST_F(CAmActionContainerTest, updateTest_1)
{
    MockCAmActionCommandChild MockCAmActionCommandChild1("Child1");
    EXPECT_CALL(MockCAmActionCommandChild1, _execute()).WillOnce(Return(E_NOT_POSSIBLE));
    MockCAmActionCommandChild1.setUndoRequried(true);

    TCLAP::CmdLine *cmd(CAmCommandLineSingleton::instanciateOnce("The team of the AudioManager wishes you a nice day!", ' ', "7.5", true));

    CAmControllerPlugin *pPlugin        = new CAmControllerPlugin();
    CAmSocketHandler    *pSocketHandler = new CAmSocketHandler();

    CAmTimerEvent *mpTimerEvent = CAmTimerEvent::getInstance();
    if (NULL == mpTimerEvent)
    {
        ASSERT_THAT(mpTimerEvent, IsNull()) << "CAmActionContainer class: Not able to create Timer object";
    }

    mpTimerEvent->setSocketHandle(pSocketHandler, pPlugin);
    pCAmActionContainer.append(&MockCAmActionCommandChild1);
    EXPECT_EQ(pCAmActionContainer.execute(), 0);
    EXPECT_EQ(0, pCAmActionContainer.update(E_NOT_POSSIBLE));
    EXPECT_EQ(pCAmActionContainer.getError(), E_NOT_POSSIBLE);
    CAmCommandLineSingleton::deleteInstance();
}

/**
 * @brief : Test to verify the cleanup function
 *
 * @pre : Mock the CAmActionCommandChild class
 *
 * @test : verify the "cleanup" function when instance of mockCAmActionCommandChild1 & mockCAmActionCommandChild1
 *         is append in the CAmActionContainer class & set the behavior of execute to return E_OK,
 *         then check whether "cleanup" function will used for cleanup of an action or not
 *        ,without any Gmock error.
 *
 * @result : "Pass" when cleanup function return the expected output "E_OK".
 */
TEST_F(CAmActionContainerTest, cleanupTest)
{
    MockCAmActionCommandChild *pMockCAmActionCommandChild1 = new MockCAmActionCommandChild("Child1");
    MockCAmActionCommandChild *pMockCAmActionCommandChild2 = new MockCAmActionCommandChild("Child1");
    EXPECT_CALL(*pMockCAmActionCommandChild1, _execute()).WillOnce(Return(E_OK));
    EXPECT_CALL(*pMockCAmActionCommandChild2, _execute()).WillOnce(Return(E_OK));
    EXPECT_CALL(*pMockCAmActionCommandChild1, _cleanup()).WillOnce(Return(0));
    EXPECT_CALL(*pMockCAmActionCommandChild2, _cleanup()).WillOnce(Return(0));
    pCAmActionContainer.append(pMockCAmActionCommandChild1);
    pCAmActionContainer.append(pMockCAmActionCommandChild2);
    EXPECT_EQ(pCAmActionContainer.execute(), 0);
    EXPECT_EQ(pCAmActionContainer.cleanup(), E_OK);

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
