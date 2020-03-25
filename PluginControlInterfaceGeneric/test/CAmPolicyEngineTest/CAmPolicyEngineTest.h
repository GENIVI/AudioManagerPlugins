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

#ifndef CAMPOLICYENGINETEST_H_
#define CAMPOLICYENGINETEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace am
{
namespace gc {

class CAmPolicyEngineTest : public ::testing::Test
{
public:
    CAmPolicyEngineTest();
    ~CAmPolicyEngineTest();

    // keep track which actions are created
    std::vector<gc_Action_s >      mListActions;
    am_Error_e mockSetListActions(std::vector<gc_Action_s > &listActions, gc_ActionList_e actionListType);

protected:
    void SetUp() final;
    void TearDown() final;

    MockIAmControlReceive          mMockControlReceiveInterface;
    IAmControlReceive             &mCAmControlReceive;
    MockIAmPolicyReceive          *mpMockIAmPolicyReceive;
    CAmPolicyEngine               *mpCAmPolicyEngine;
    std::string                    sinkName;
    std::string                    sourceName;
    std::string                    className;
    gc_ConnectionInfo_s            ConnectionInfo;
    gc_Element_e                   elementType;
};


}
}

#endif /* CAMPOLICYENGINETEST_H_ */
