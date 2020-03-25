/*******************************************************************************
 *  \copyright (c) 2018 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com> 2018
 *           Kapildev Patel, Satish Dwivedi <kpatel@jp.adit-jv.com> 2018
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

#ifndef CAMCLASSACTIONINTERRUPTTEST_H_
#define CAMCLASSACTIONINTERRUPTTEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am
{
namespace gc {
class CAmClassActionInterruptTest : public ::testing::Test
{
public:
    CAmClassActionInterruptTest();
    ~CAmClassActionInterruptTest();

protected:
    void SetUp() final;
    void TearDown() final;

    MockIAmControlReceive                    *mpMockControlReceiveInterface;
    IAmControlReceive                        *mpControlReceive;
    std::shared_ptr<CAmSinkElement>           mpSinkElement;
    std::shared_ptr<CAmSourceElement>         mpSourceElement;
    std::shared_ptr<CAmClassElement >         mpClassElement;
    std::shared_ptr<CAmMainConnectionElement> mpMainConnection;
    std::shared_ptr<CAmRouteElement>          mpRouteElement;
    CAmConnectionListFilter                  *mpFilterObject;
    CAmActionInterrupt                       *mpClassActionInterrupt;
    am_MainConnection_s                       mMainConnectionData;
    gc_Class_s                         mGcClassConnection;
    am_Handle_s                        mHandle;
    CAmActionParam<std::string >       mSourceNameParam;
    CAmActionParam<std::string >       mSinkNameParam;
    std::vector<am_Route_s >           mRouteList;
    std::vector<am_ConnectionState_e > mListConnectionStates;
    am_ConnectionState_e               mConnectionstate;
    ut_gc_Sink_s                       sinkInfo;
    ut_gc_Source_s                     sourceInfo;
    am_RoutingElement_s                RoutingElement;
    gc_Route_s                         GCRoute;
    am_ClassProperty_s                 classPropertyData;
    am_SinkClass_s                     sinkClass;
    am_SourceClass_s                   sourceClass;
    am_SystemProperty_s                systemproperty;

};

}
}

#endif /* CAMCLASSACTIONINTERRUPTTEST_H_ */
