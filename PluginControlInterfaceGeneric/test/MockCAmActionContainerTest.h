/*
 * MockCAmActionContainerTest.h
 *
 *  Created on: 20-May-2016
 *      Author: dipesh
 */

#ifndef MOCKCAMACTIONCONTAINERTEST_H_
#define MOCKCAMACTIONCONTAINERTEST_H_

#include <string>
#include <map>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmTimerEvent.h"
#include "time.h"

#include "CAmActionContainer.h"
#include "CAmActionCommand.h"
#include "CAmEventSubject.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

/*namespace am
 * {
 * namespace gc{
 * class MockCAmActionCommandChild : public CAmActionCommand {
 * public:
 *
 *  MockCAmActionCommandChild(){};
 *  virtual ~MockCAmActionCommandChild() {};
 *
 *  //Public method mock
 *  MOCK_CONST_METHOD0(getStatus, ActionState_e(void));
 *  MOCK_CONST_METHOD0(getError,int (void));
 *  MOCK_METHOD0(getUndoRequired ,bool (void));
 *  MOCK_METHOD0(getTimeout ,uint32_t (void));
 *  MOCK_METHOD0(undo ,int (void));
 *  MOCK_METHOD1(update ,int (const int result));
 *  MOCK_METHOD0(execute ,int (void));
 *  MOCK_METHOD1(setStatus ,void (const ActionState_e state));
 *     MOCK_METHOD1(setError ,void (const int error));
 *
 *
 *  //Protected method mock
 *  MOCK_METHOD1(_update ,int(const int));
 *  MOCK_METHOD0(_execute ,int (void));
 *  MOCK_METHOD0(_undo ,int (void));
 *
 * };
 * }
 * }*/

namespace am
{

namespace gc {
/*
 * The class needs to be derived from the CAmActionCommand class, the reason being we want to reuse
 * the functionality implemented in the CAmActionCommand like notify, setStatus getStatus etc. If we
 * derive from the IAmAction command then we will have to implement i.e mock all the functions.
 * CAmActionCommand is already tested so we can resuse its functionality.
 */
class MockCAmActionCommandChild : public CAmActionCommand
{
public:

    MockCAmActionCommandChild(std::string name)
        : CAmActionCommand(name){}
    virtual ~MockCAmActionCommandChild() {}

    MOCK_METHOD0(_execute, int(void));
    MOCK_METHOD0(_undo, int(void));

};

}
}

/**This is the MockCAmActionContainer class derived from the class CAmActionContainer **/
namespace am
{
namespace gc {
class MockCAmActionContainer : public CAmActionContainer
{
public:

    // MockCAmActionContainer(){};

    // MockCAmActionContainer() : CAmActionContainer ("name",AS_NOT_STARTED,0,0,NULL,false,INFINITE_TIMEOUT,0,0){};
    virtual ~MockCAmActionContainer() {}

    /**Public method mock **/

    MOCK_CONST_METHOD0(getError, int(void));
    MOCK_METHOD1(setError, void(const int error));
    MOCK_METHOD0(cleanup, int(void));
    MOCK_CONST_METHOD0(getName, std::string(void));
    MOCK_CONST_METHOD0(getStatus, ActionState_e(void));
    MOCK_METHOD1(setStatus, void(const ActionState_e state));
    MOCK_METHOD1(setParent, int(IAmActionCommand * command));
    MOCK_METHOD2(setParam, bool(const std::string &paramName, IAmActionParam * pParam));
    MOCK_METHOD1(getParam, IAmActionParam * (const std::string &paramName));
    MOCK_METHOD0(getUndoRequired, bool(void));
    MOCK_METHOD1(setUndoRequried, void(const bool undoRequired));
    MOCK_METHOD1(setTimeout, void(uint32_t timeout));
    MOCK_METHOD0(getExecutionTime, uint32_t(void));
    MOCK_METHOD0(getUndoTime, uint32_t(void));
    MOCK_METHOD0(getTimeout, uint32_t(void));

    MOCK_METHOD1(update, int(const int result));
    MOCK_METHOD0(execute, int(void));

    /**Protected method mock **/
    MOCK_METHOD1(_update, int(const int));
    MOCK_METHOD0(_execute, int(void));
    MOCK_METHOD0(_undo, int(void));
    MOCK_METHOD0(_cleanup, int(void));
// MOCK_METHOD2(_update ,int(const int result, const int index));

    /* TODO chk to do mock or not ?
     *
     * int append(IAmActionCommand* command);
     * int insert(IAmActionCommand* command);
     * bool isEmpty(void);
     *
     */

};

}
}

#endif /* MOCKCAMACTIONCONTAINERTEST_H_ */
