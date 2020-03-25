/*
 * MockCAmActionCommandTest.h
 *
 *  Created on: 11-May-2016
 *      Author: dipesh
 */

#ifndef MOCKCAMACTIONCOMMANDTEST_H_
#define MOCKCAMACTIONCOMMANDTEST_H_

#include <string>
#include <map>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmTimerEvent.h"
#include "time.h"

#include "CAmActionCommand.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

/**This is the MockCAmActionCommand class derived from the class CAmActionCommand **/
namespace am
{
namespace gc {
class MockCAmActionCommand : public CAmActionCommand
{
public:

    MockCAmActionCommand(){}
    virtual ~MockCAmActionCommand() {}

    /**Public method mock **/
    MOCK_CONST_METHOD0(getStatus, ActionState_e(void));
    MOCK_CONST_METHOD0(getError, int(void));
    MOCK_METHOD0(getUndoRequired, bool(void));
    MOCK_METHOD0(getTimeout, uint32_t(void));

    /**Protected method mock **/
    MOCK_METHOD1(_update, int(const int));
    MOCK_METHOD0(_execute, int(void));
    MOCK_METHOD0(_undo, int(void));

};

}
}

#endif /* MOCKCAMACTIONCOMMANDTEST_H_ */
