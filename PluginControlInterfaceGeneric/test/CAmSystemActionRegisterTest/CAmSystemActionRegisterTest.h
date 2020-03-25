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
#ifndef PLUGINCONTROLINTERFACEGENERIC_TEST_CAMSINKELEMENTTEST_CAMSINKELEMENTTEST_H_
#define PLUGINCONTROLINTERFACEGENERIC_TEST_CAMSINKELEMENTTEST_CAMSINKELEMENTTEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am
{
namespace gc {
class CAmSystemActionRegisterTest : public ::testing::Test
{
public:
    CAmSystemActionRegisterTest();
    ~CAmSystemActionRegisterTest();

protected:
    void SetUp() final;
    void TearDown() final;
    void InitializeCommonStruct();

    MockIAmControlReceive            *mpMockControlReceiveInterface;
    IAmControlReceive                *mpCAmControlReceive;
    CAmDomainElement                 *mpCAmDomainElement;

    std::shared_ptr<CAmSinkElement>   mpCAmSinkElement;
    std::shared_ptr<CAmSourceElement> mpCAmSourceElement;
    ut_gc_Sink_s                                sinkInfo;
    am_Sink_s                                   sink;
    am_sinkID_t                                 sinkID;
    am_MainSoundProperty_s                      mainsoundProperties;
    am_CustomMainSoundPropertyType_t            mainpropertyType;
    am_SoundProperty_s                          soundProperties;
    am_CustomSoundPropertyType_t                propertyType;
    am_CustomConnectionFormat_t                 connectionformat;
    am_sourceID_t                               sourceID;
    am_Source_s                                 source;
    gc_MainSoundProperty_s                      gcMainSoundProperty;
    gc_SoundProperty_s                          gcSoundProperty;
    ut_gc_Source_s                              sourceInfo;
    am_Domain_s                                 domain;
    am_Domain_s                                 domain1;
    gc_Domain_s                                 domainInfo;
    am_domainID_t                               domainID;
    am_DomainState_e                            state;
    am_ClassProperty_s                          classPropertyData;
    std::vector<am_ClassProperty_s>             listClassProperties;
    am_ClassProperty_s                          classProperty;
    std::shared_ptr<CAmClassElement>            pClassElement;
    CAmActionParam<std::vector<gc_Source_s > >  mListSources;
    CAmActionParam<std::vector<gc_Sink_s > >    mListSinks;
    CAmActionParam<std::vector<gc_Gateway_s > > mListGateways;
    std::vector < gc_Source_s >                 listSources;
    std::vector < gc_Sink_s >                   listSinks;
    std::vector < gc_Gateway_s >                listGateways;
    std::vector <am_Domain_s>                   listDomains;
    std::shared_ptr<am::gc::CAmDomainElement>   pDomainElement;
    std::shared_ptr<am::gc::CAmDomainElement>   pDomainElement2;

    gc_Class_s                                  classElement;
    am_SinkClass_s                              sinkClass;
    am_SourceClass_s                            sourceClass;
    am_Gateway_s                                gateway;
    gc_Gateway_s                                gatewayInfo;
    std::shared_ptr<CAmGatewayElement>          pGatewayElement;
    std::vector<am_CustomConnectionFormat_t >   listConnectionFormats;
    // CAmSystemActionRegister                  *mpCAmSystemActionRegister;
    IAmActionCommand                           *mpCAmSystemActionRegister;
    std::shared_ptr<CAmSystemElement >          pSystem;
    gc_RoutingElement_s                         gcRoutingElement;
    CAmRouteElement                            *mpCAmRouteElement;
    gc_System_s                                 systemConfiguration;
    am_SystemProperty_s                         systemproperty;
    am_CustomSystemPropertyType_t               type;
    std::vector<am_SystemProperty_s >           listSystemProperties;
    int16_t value;

};

}
}

#endif /* CAMSYSTEMACTIONREGISTERTEST_H_ */
