/*
 * CAmActionContainerTest.cpp
 *
 *  Created on: 20-May-2016
 *      Author: dipesh
 */

#include <map>
#include <time.h>
#include "MockCAmActionContainerTest.h"
#include "CAmActionCommandTest.h"
#include "CAmActionContainer.h"
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"
#include "CAmActionCommand.h"
#include "CAmSocketHandler.h"
using namespace testing;
using namespace am;
using namespace gc;

DltContext GenericControllerDlt; /*This added only for compilation purpose*/

/*
 * Here the  test CAmActionContainerTest get executed for the following three test cases
 * 1.updateTest
 * 2.undoTest
 * 3.executeTest
 */

/*TEST(CAmActionContainerTest,undoTest)
 * {
 *  using ::testing::InSequence;
 *
 *
 *  //*created a child of type MockCAmActionCommandChild
 *  MockCAmActionCommandChild MockCAmActionCommandChild1, MockCAmActionCommandChild2;
 *
 *  //*created a object of  MockCAmActionContainer class*
 *  MockCAmActionContainer pMockCAmActionContainer;
 *
 *  //*appending the child
 *  pMockCAmActionContainer.insert(&MockCAmActionCommandChild1);
 *  pMockCAmActionContainer.insert(&MockCAmActionCommandChild2);
 *  //pMockCAmActionContainer.append(&MockCAmActionCommandChild1);
 *
 *  //pMockCAmActionContainer.append(&MockCAmActionCommandChild2);
 *
 *  InSequence s;
 *
 *  //*set the expectation on method of MockCAmActionContainer class
 *  EXPECT_CALL(pMockCAmActionContainer,getStatus()).WillOnce(Return(AS_ERROR_STOPPED));
 *  EXPECT_CALL(pMockCAmActionContainer,getUndoRequired()).WillOnce(Return(true));
 *  EXPECT_CALL(pMockCAmActionContainer,_undo()).WillOnce(Return(0));
 *  EXPECT_CALL(pMockCAmActionContainer,getStatus()).WillOnce(Return(AS_UNDOING));
 *
 *  //*set the expectation on method of child class i.e MockCAmActionCommandChild
 *  EXPECT_CALL(MockCAmActionCommandChild1,getStatus()).WillOnce(Return(AS_ERROR_STOPPED));
 *  EXPECT_CALL(MockCAmActionCommandChild1,getError()).WillOnce(Return(0));
 *  EXPECT_CALL(MockCAmActionCommandChild1,undo()).WillOnce(Return(0));
 *  EXPECT_CALL(MockCAmActionCommandChild1,getStatus()).WillOnce(Return(AS_UNDO_COMPLETE));
 *
 *  EXPECT_CALL(pMockCAmActionContainer,getError()).WillOnce(Return(0));
 *  EXPECT_CALL(pMockCAmActionContainer,update(0)).WillOnce(Return(0));
 *
 *
 *  /*EXPECT_CALL(MockCAmActionCommandChild2,getStatus()).WillOnce(Return(AS_ERROR_STOPPED));
 *  EXPECT_CALL(MockCAmActionCommandChild2,getError()).WillOnce(Return(0));
 *  EXPECT_CALL(MockCAmActionCommandChild2,undo()).WillOnce(Return(0));
 *  EXPECT_CALL(MockCAmActionCommandChild2,getStatus()).WillOnce(Return(AS_UNDO_COMPLETE));
 *
 *  EXPECT_CALL(pMockCAmActionContainer,getError()).WillOnce(Return(0));
 *  EXPECT_CALL(pMockCAmActionContainer,update(0)).WillOnce(Return(0));
 *
 *  EXPECT_EQ(0,pMockCAmActionContainer.undo());
 *
 * }*/

/*
 * In this test case two child actions are appended to the child1 is success and the child2 is
 * resulting in an error. when undo is initiated only the undo of child1 should get called
 */
ACTION(DltFunctionEntry)
{
    // LOG_FN_ERROR("ABC");
}

TEST(CAmActionContainerTest, undoTest)
{
    using ::testing::InSequence;

    // Preparation phase
    MockCAmActionCommandChild MockCAmActionCommandChild1("Child1");
    MockCAmActionCommandChild MockCAmActionCommandChild2("Child2");
    CAmActionContainer        IUT;
    CAmSocketHandler          socketHandler;
    CAmTimerEvent::getInstance()->setSocketHandle(&socketHandler, NULL);
    // appending the child
    EXPECT_CALL(MockCAmActionCommandChild1, _execute())
    .WillOnce(DoAll(DltFunctionEntry(),
            Return(E_OK)));
    EXPECT_CALL(MockCAmActionCommandChild2, _execute()).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_CALL(MockCAmActionCommandChild1, _undo()).WillOnce(Return(E_OK));
    EXPECT_CALL(MockCAmActionCommandChild2, _undo()).Times(0);

    /*
     * As part of the test preparation following steps are needed
     * 1. Create two child actions, set the undo required to true
     * 2. Append the two child actions to the IUT which is CAmActionContainer
     * 3. Call the execute of the IUT
     */
    MockCAmActionCommandChild1.setUndoRequried(true);
    MockCAmActionCommandChild2.setUndoRequried(true);
    IUT.append(&MockCAmActionCommandChild1);
    IUT.append(&MockCAmActionCommandChild2);
    IUT.setUndoRequried(true);
    EXPECT_EQ(IUT.execute(), 0);
    /*
     * After the execution of the IUT, expectation is
     * 1. The _execute of the MockCAmActionCommandChild1 gets called , mock it to return 0
     * 2. The _execute of the MockCAmActionCommandChild1 gets called , mock it to return error
     * 3. check if the IUT.getStatus = AS_ERROR_STOPPED
     * 4. check if the IUT.getError() = E_NOT_POSSIBLE
     * 5. FInally call the undo method of IUT and expect the _undo of child1 getting called.
     */
    EXPECT_EQ(IUT.getStatus(), AS_ERROR_STOPPED);
    EXPECT_EQ(IUT.getError(), E_NOT_POSSIBLE);
    EXPECT_EQ(IUT.undo(), E_OK);
    EXPECT_EQ(IUT.getStatus(), AS_UNDO_COMPLETE);

}

/*TEST(CAmActionContainerTest,updateTest)
 * {
 *  using ::testing::InSequence;
 *
 *  const int error =1;
 *  //created a object of  MockCAmActionContainer class
 *  MockCAmActionContainer pMockCAmActionContainer;
 *  InSequence s;
 *  //*set the expectation on method of MockCAmActionContainer class
 *  EXPECT_CALL(pMockCAmActionContainer,getStatus()).WillOnce(Return(AS_EXECUTING));
 *  EXPECT_CALL(pMockCAmActionContainer,getError()).WillOnce(Return(1));
 *  EXPECT_CALL(pMockCAmActionContainer,setStatus(AS_ERROR_STOPPED));
 *  EXPECT_CALL(pMockCAmActionContainer,getError()).WillOnce(Return(1));
 *  EXPECT_CALL(pMockCAmActionContainer,_update(error)).WillOnce(Return(0));
 *  EXPECT_CALL(pMockCAmActionContainer,getError()).WillOnce(Return(1));
 *
 *
 *
 *
 *
 *
 *
 *  EXPECT_EQ(0,pMockCAmActionContainer.update(error));
 *
 * }*/
int main(int argc, char * *argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    DLT_REGISTER_APP("TEST", "TEST for CAmActionContainer");
    CAmDltWrapper::instance()->registerContext(GenericControllerDlt, "AMCO",
        "Generic Controller Context",
        DLT_LOG_DEBUG, DLT_TRACE_STATUS_OFF);
    LOG_FN_ENTRY();
    /*uncomment the test cases which you wants to run*/
    // ::testing::GTEST_FLAG(filter) = "*undoTest*";

    /*All the test cases for CAmActionCommandTest test are run */
    LOG_FN_EXIT();
    return RUN_ALL_TESTS();
}
