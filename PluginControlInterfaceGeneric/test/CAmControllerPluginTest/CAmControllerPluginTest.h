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

#ifndef PLUGINCONTROLINTERFACEGENERIC_TEST_CAMCONTROLSENDTEST_CAMCONTROLSENDTEST_H_
#define PLUGINCONTROLINTERFACEGENERIC_TEST_CAMCONTROLSENDTEST_CAMCONTROLSENDTEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am
{
namespace gc {

class CAmControllerPluginTest : public ::testing::Test
{
public:
    CAmControllerPluginTest();
    ~CAmControllerPluginTest();

protected:
    void SetUp() final;
    void TearDown() final;
    void InitializeCommonStruct();

    MockIAmControlReceive            *mpMockControlReceiveInterface;
    IAmControlReceive                *mpCAmControlReceive;
    std::shared_ptr<CAmSinkElement>   mpCAmSinkElement;
    std::shared_ptr<CAmSourceElement> mpCAmSourceElement;
    CAmRouteElement                  *mpCAmRouteElement;
    gc_RoutingElement_s               gcRoutingElement;
    MockIAmPolicySend                *mpMockIAmPolicySend;
    CAmControllerPlugin              *mpPlugin;
    CAmSocketHandler                 *pSocketHandler;
    ut_gc_Sink_s                               sinkInfo;
    am_Sink_s                                  sink, sink1;
    am_sinkID_t                                sinkID, sinkID1;
    am_sourceID_t                              sourceID, sourceID1;
    am_Source_s                                source, source1;
    ut_gc_Source_s                             sourceInfo;

    am_ClassProperty_s                         classPropertyData;
    am_SinkClass_s                             sinkClass;
    am_SourceClass_s                           sourceClass;
    gc_Class_s                                 classElement;
    am_SystemProperty_s                        systemproperty;
    std::vector<am_SystemProperty_s >          listSystemProperties;
    gc_System_s                                systemConfiguration;
    std::shared_ptr<CAmSystemElement>          pSystem;
    std::vector<am_Route_s>                    listRoutes;
    am_RoutingElement_s                        routingElement;
    am_Route_s                                 amRoute;
    gc_Route_s                                 gcRoute;
    am_MainConnection_s                        mainConnectionData;
    am_MainConnection_s                        mainConnectionData1;
    am_mainConnectionID_t                      mainConnectionID;
    am_Handle_s                                handle;
    am_Handle_s                                handleConnect;

    std::shared_ptr<CAmClassElement>           pClassElement;
    //CAmSinkActionSetVolume                          *ActionSetVolume;
    std::shared_ptr<CAmMainConnectionElement                        >pMainConnection;
    std::vector<CAmMainConnectionElement * >   listConnections;
    std::vector<gc_MainSoundProperty_s >       listGCMainSoundProperties;
    gc_MainSoundProperty_s                     gcMainSoundProperty;
    am_MainSoundProperty_s                     amMainsoundProperty;
    am_CustomMainSoundPropertyType_t           propertyType;
    CAmActionParam<am_volume_t >               mVolumeParam;
    CAmActionParam<gc_LimitType_e >            mLimitTypeParam;
    CAmActionParam<am_volume_t >               mLimitVolumeParam;
    CAmActionParam<am_time_t >                 mRampTimeParam;
    CAmActionParam<am_CustomRampType_t >       mRampTypeParam;

    am_MainConnection_s                        mainConnectionDataDisconnect;
    am_MainConnection_s                        mainConnectionDataDisconnect1;
    am_Handle_s                                handleVlumesink;
    am_mainVolume_t                            mainVolume;
    int16_t increment;
    am_Domain_s                                domain;
    ut_gc_Domain_s                             domainInfo;
    am_domainID_t                              domainID;
    std::vector < am_Domain_s >                listDomains;
    am_DomainState_e                           state;
    std::shared_ptr<CAmDomainElement>          mpCAmDomainElement;
    int state1;
    am_CustomSystemPropertyType_t              type;
    int16_t value;
    std::vector<am_ClassProperty_s>            listClassProperties;
    am_ClassProperty_s                         classProperty;
    am_NotificationConfiguration_s             notificationConfiguration;
    gc_Gateway_s                               gatewayInfo;
    TestObserver                              *testObserver;
    am_Gateway_s                               gateway;
    am_gatewayID_t                             gatewayID;
    std::vector<am_gatewayID_t >               listGatewaysIDs;
    am_volume_t                                LimitVolumeParam;
    am_CustomRampType_t                        RampTypeParam;
    am_time_t                                  RampTimeParam;
    gc_SoundProperty_s                         gcSoundProperty;
    std::vector<gc_SoundProperty_s >           listGCSoundProperties;
    std::vector<am_CustomConnectionFormat_t >  listConnectionFormats;
    CAmActionParam<std::vector<gc_Source_s > > mListSources;
    CAmActionParam<std::vector<gc_Sink_s > >   mListSinks;
    std::vector < gc_Source_s >                listSources;
    std::vector < gc_Sink_s >                  listSinks;

};

}
}

#endif /* PLUGINCONTROLINTERFACEGENERIC_TEST_CAMCONTROLSENDTEST_CAMCONTROLSENDTEST_H_ */
