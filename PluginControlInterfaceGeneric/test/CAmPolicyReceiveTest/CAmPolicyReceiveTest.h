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

#ifndef PLUGINCONTROLINTERFACEGENERIC_TEST_CAMPOLICYRECEIVETEST_CAMPOLICYRECEIVETEST_H_
#define PLUGINCONTROLINTERFACEGENERIC_TEST_CAMPOLICYRECEIVETEST_CAMPOLICYRECEIVETEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am
{
namespace gc {

class CAmPolicyReceiveTest : public ::testing::Test
{
public:
    CAmPolicyReceiveTest();
    ~CAmPolicyReceiveTest();

    std::string int2string(int x);
    void SetUp() final;
    void TearDown() final;
    void InitializeCommonStruct();

    MockIAmControlReceive            *mpMockControlReceiveInterface;
    IAmControlReceive                *mpCAmControlReceive;
    std::shared_ptr<CAmSinkElement>   mpCAmSinkElement;
    std::shared_ptr<CAmSourceElement> mpCAmSourceElement;

    CAmRouteElement                  *mpCAmRouteElement;

    MockIAmPolicySend                *mpMockIAmPolicySend;
    CAmPolicyReceive                 *mpCAmPolicyReceive;
    string                                                           data1;
    string                                                           value;
    am_Domain_s                                                      domain, domain1, domain3;
    std::vector<am_Domain_s>                                         listDomains;

    ut_gc_Sink_s                                                     sinkInfo;
    am_Sink_s                                                        sink;
    am_sinkID_t                                                      sinkID;
    am_sourceID_t                                                    sourceID;
    am_Source_s                                                      source;
    ut_gc_Source_s                                                   sourceInfo;
    am_SourceState_e                                                 sourceState;

    std::shared_ptr<CAmDomainElement>                                mpCAmDomainElement, mpCAmDomainElement1;
    ut_gc_Domain_s                                                   domainInfo, domainInfo1;
    am_domainID_t                                                    domainID, domainID1;
    am_ClassProperty_s                                               classPropertyData;
    am_SinkClass_s                                                   sinkClass;
    am_SourceClass_s                                                 sourceClass;
    std::vector<std::vector<gc_TopologyElement_s > >                 listTopologies;
    std::vector<gc_TopologyElement_s >                               GClistTopolologyElement;
    gc_Class_s                                                       classElement;
    std::shared_ptr<CAmClassElement                                 >mpCAmClassElement;
    std::shared_ptr<CAmMainConnectionElement>                        mpCAmMainConnectionElement;
    gc_Route_s                                                       route1;
    gc_Route_s                                                       gcRoute;
    am_RoutingElement_s                                              routingElement;

    am_Source_s                                                      source_out1;
    am_Source_s                                                      source_out2;
    am_Handle_s                                                      handle;
    am_Handle_s                                                      handleSetSourceState;
    am_MainConnection_s                                              mainConnectionData;
    am_Gateway_s                                                     gateway;
    gc_Gateway_s                                                     gatewayInfo;
    std::shared_ptr<CAmGatewayElement>                               mpCAmGatewayElement;
    am_gatewayID_t                                                   gatewayID;
    CAmActionParam<std::vector<gc_Gateway_s > >                      mListGateways;
    std::vector < gc_Gateway_s >                                     listGateways;
    am_DomainState_e                                                 state;
    am_Availability_s                                                available;
    am_SystemProperty_s                                              systemproperty;
    std::vector<am_SystemProperty_s>                                 listSystemProperties;
    std::vector<CAmMainConnectionElement * >                         listConnections;
    std::vector<am_Route_s>                                          amRouteList;
    am_Route_s                                                       amRoute;
    gc_ConnectionInfo_s                                              ConnectionInfo, ConnectionInfos;
    std::vector<gc_ConnectionInfo_s >                                listConnectionInfos;
    gc_RoutingElement_s                                              gcRoutingElement;
    am_mainConnectionID_t                                            mainConnectionID;
    std::vector<am_NotificationConfiguration_s>                      listMainNotificationConfigurations;
    am_NotificationConfiguration_s                                   notificationConfigurations;
    gc_Order_e                                                       order;
    CAmActionConnect                                                *mpCAmClassActionConnect;
    gc_Action_s                                                      Actions;
    std::vector<gc_Action_s >                                        listActions;
    gc_System_s                                                      systemConfiguration;
    int state1;

};

}
}

#endif /* PLUGINCONTROLINTERFACEGENERIC_TEST_CAMPOLICYRECEIVETEST_CAMPOLICYRECEIVETEST_H_ */
