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
    MOCK_METHOD0(_cleanup, int(void));

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

};

}
}

#endif /* MOCKCAMACTIONCONTAINERTEST_H_ */
