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

#ifndef CAMCLASSACTIONCONNECTTEST_H_
#define CAMCLASSACTIONCONNECTTEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am
{
namespace gc {
class CAmClassActionConnectTest : public ::testing::Test
{
public:
    CAmClassActionConnectTest();
    ~CAmClassActionConnectTest();

protected:
    void SetUp() final;
    void TearDown() final;

    MockIAmControlReceive *mpMockControlReceiveInterface;
    IAmControlReceive     *mpControlReceive;
// CAmSinkElement                              *mpSinkElement;
// CAmSourceElement                            *mpSourceElement;
// CAmClassElement                             *mpClassElement;

    std::shared_ptr<CAmSinkElement>              mpSinkElement;
    std::shared_ptr<CAmSourceElement>            mpSourceElement;
    std::shared_ptr<CAmClassElement>             mpClassElement;

    CAmActionConnect                            *mpClassActionConnect;
    am_MainConnection_s                          mMainConnectionData;
    am_Handle_s mHandle;
    CAmActionParam<std::string >                 mSourceNameParam;
    CAmActionParam<std::string >                 mSinkNameParam;
    CAmActionParam<gc_Order_e >                  mOrderParam;
    CAmActionParam<am_CustomConnectionFormat_t > mConnectionFormatParam;
    std::vector<am_Route_s>                      mRouteList;
    am_volume_t mLimitVolumeParam;
    am_CustomRampType_t                          mRampTypeParam;
    am_time_t mRampTimeParam;

    // gc_TopologyElement_s topologyElement;
// am::gc::gc_ut_System_s systemConfiguration;
    ut_gc_Sink_s                       sinkInfo;
    ut_gc_Source_s                     sourceInfo;

    gc_System_s                        systemConfiguration;
    am_SystemProperty_s                systemproperty;
    am_CustomSystemPropertyType_t      type;
    std::vector<am_SystemProperty_s >  listSystemProperties;
    int16_t value;
    std::shared_ptr<CAmSystemElement > pSystem;

    am_ClassProperty_s                 classPropertyData;

    am_RoutingElement_s                RoutingElement;
    gc_Route_s GCRoute;
    am_SinkClass_s                     sinkClass;
    am_SourceClass_s                   sourceClass;
    gc_Class_s classConnection;
};

}
}

#endif /* CAMCLASSACTIONCONNECTTEST_H_ */
