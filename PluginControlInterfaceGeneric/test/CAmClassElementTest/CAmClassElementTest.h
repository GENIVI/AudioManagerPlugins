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

#ifndef PLUGINCONTROLINTERFACEGENERIC_TEST_CAMCLASSELEMENTTEST_CAMCLASSELEMENTTEST_H_
#define PLUGINCONTROLINTERFACEGENERIC_TEST_CAMCLASSELEMENTTEST_CAMCLASSELEMENTTEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am
{
namespace gc {

class CAmClassElementTest : public ::testing::Test
{
public:
    CAmClassElementTest();
    ~CAmClassElementTest();

protected:
    void SetUp() final;
    void TearDown() final;
    void InitializeCommonStruct();

    MockIAmControlReceive                    *mpMockControlReceiveInterface;
    IAmControlReceive                        *mpCAmControlReceive;
    CAmRouteElement                          *mpCAmRouteElement;
    CAmElement                               *mpCAmElement;
    std::shared_ptr<CAmMainConnectionElement> mpCAmMainConnectionElement;
    std::shared_ptr<CAmClassElement>          mpCAmClassElement;
    std::shared_ptr<CAmSinkElement>           mpCAmSinkElement;
    std::shared_ptr<CAmSourceElement>         mpCAmSourceElement;

    ut_gc_Sink_s                              sinkInfo;
    am_Sink_s sink;
    am_sinkID_t                               sinkID;
    am_sourceID_t                             sourceID;
    am_Source_s                               source;
    ut_gc_Source_s                            sourceInfo;
    gc_Class_s classElement;
    am_SystemProperty_s                       systemproperty;
    std::vector<am_SystemProperty_s >         listSystemProperties;
    gc_System_s                               systemConfiguration;
    std::shared_ptr<CAmSystemElement>         mpCAmSystemElement;
    am_MainConnection_s                       mainConnectionData;
    std::vector<am_Route_s>                   listRoutes;
    am_RoutingElement_s                       routingElement;
    am_Route_s                               amRoute;
    gc_Route_s                               gcRoute;
    gc_RoutingElement_s                      gcRoutinElement;
    std::vector<am_ConnectionState_e >       listConnectionStates;
    std::vector<CAmMainConnectionElement * > listMainConnections;
    CAmConnectionListFilter                  filterObject;
    am_ClassProperty_s                       classPropertyData;
    am_SinkClass_s                           sinkClass;
    am_SourceClass_s                         sourceClass;
    gc_RoutingElement_s                      gcRoutingElement;
    std::vector<CAmClassElement * >          listClasses;
    am_sourceClass_t                         sourceClassID;
    am_sinkClass_t                           sinkClassID;
    gc_Element_e                             elementType;
    am_ClassProperty_s                       classProperty;
    std::vector<am_ClassProperty_s>          listClassProperties;
    std::map<uint32_t, gc_LimitVolume_s >    mapLimits;
    gc_LimitVolume_s                         limitVolume;
    std::vector<gc_SinkVolume_s>             listSinkVolume;
    am_ConnectionState_e                     state;
    am_mainConnectionID_t                    mainConnectionID;
    gc_Order_e                               order;

};

}
}

#endif /* PLUGINCONTROLINTERFACEGENERIC_TEST_CAMCLASSELEMENTTEST_CAMCLASSELEMENTTEST_H_ */
