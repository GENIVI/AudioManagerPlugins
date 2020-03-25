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
namespace gc
{
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
