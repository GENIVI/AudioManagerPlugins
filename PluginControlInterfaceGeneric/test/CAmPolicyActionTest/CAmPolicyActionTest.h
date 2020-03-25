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

#ifndef PLUGINCONTROLINTERFACEGENERIC_TEST_CAMPOLICYACTIONTEST_CAMPOLICYACTIONTEST_H_
#define PLUGINCONTROLINTERFACEGENERIC_TEST_CAMPOLICYACTIONTEST_CAMPOLICYACTIONTEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am
{
namespace gc {

class CAmPolicyActionTest : public ::testing::Test
{
public:
    CAmPolicyActionTest();
    ~CAmPolicyActionTest();

/*
 * This Method will be used in Case of Connect and Volume Action
 */
    void CreateSourceSink();

    /*
     * This method creates the sink with Sound Property
     */
    void CreateSink();

    /*
     * This method creates the Source with NotificationConfiguration
     */
    void CreateSource();

protected:
    void SetUp() final;
    void TearDown() final;

    MockIAmControlReceive *mpMockControlReceiveInterface;
    MockIAmPolicySend     *mpPolicySendInterface;
    IAmControlReceive     *mpControlReceive;

// CAmSinkElement                                                      *mpSinkElement;
// CAmSourceElement                                                    *mpSourceElement;

    std::shared_ptr<CAmSinkElement>   mpSinkElement;
    std::shared_ptr<CAmSourceElement> mpSourceElement;
//
// CAmClassElement                                                     *mpClassElement;
// CAmSystemElement                                                    *mpSystemElement;
    CAmActionConnect                 *mpClassActionConnect;
//

    std::shared_ptr<CAmClassElement>  mpClassElement;
    std::shared_ptr<CAmSystemElement> mpSystemElement;
    // std::shared_ptr<CAmClassActionConnect>mpClassActionConnect;

    gc_System_s                                                          systemConfiguration;

    CAmPolicyAction                                                     *mpPolicyAction;
#ifdef VOLUME_RELATED_TEST
    CAmClassActionSetVolume                                             *mpClassActionSetVolume;
#endif
    CAmSystemActionSetProperty                                          *mpySystemActionSetProperty;
    gc_Action_s                                                          mActions;
    am_MainConnection_s                                                  mMainConnectionData;
    am_Handle_s                                                          mHandle;
    am_SystemProperty_s                                                  mSystemproperty;
    am_SoundProperty_s                                                   mSoundProperty;
    am_NotificationConfiguration_s                                       mNotificationConfigurations;
    CAmActionParam<std::string >                                         mSourceNameParam;
    CAmActionParam<std::string >                                         mSinkNameParam;
    CAmActionParam<gc_Order_e >                                          mOrderParam;
    CAmActionParam<am_CustomConnectionFormat_t >                         mConnectionFormatParam;
    CAmActionParam <std::string>                                         mClassNameParam;
    CAmActionParam<am_volume_t >                                         mTimeoutvalue;
    CAmActionParam<am_mainVolume_t >                                     mMainVolumeParam;
    CAmActionParam<uint16_t >                                            mTypeParam;
    CAmActionParam<int16_t >                                             mValueParam;
    CAmActionParam<am_CustomMainSoundPropertyType_t >                    mPropertyTypeParam;
    CAmActionParam<int16_t >                                             mPropertyValueParam;
    CAmActionParam<am_CustomNotificationType_t >                         mNotificationType;
    CAmActionParam<am_NotificationStatus_e >                             mNotificationStatus;
    CAmActionParam<int16_t >                                             mNotificationparam;
    std::vector<gc_Action_s >                                            mListPolicyActions;
    std::vector<am_NotificationConfiguration_s>                          mListMainNotificationConfigurations;
    CAmSourceSinkActionSetSoundProperty <CAmSinkElement >               *mpActionSinkSoundProperty;
    CAmSourceSinkActionSetNotificationConfiguration <CAmSourceElement > *mpActionSourceNotification;
    ut_gc_Sink_s                                                         sinkInfo;
    ut_gc_Source_s                                                       sourceInfo;

    std::vector<am_Route_s >                                             listRoutes;
    am_RoutingElement_s                                                  routingElement;

};

}
}

#endif /* PLUGINCONTROLINTERFACEGENERIC_TEST_CAMPOLICYACTIONTEST_CAMPOLICYACTIONTEST_H_ */
