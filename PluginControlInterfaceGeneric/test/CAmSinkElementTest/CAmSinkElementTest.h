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

class CAmSinkElementTest : public ::testing::Test
{
public:
    CAmSinkElementTest();
    ~CAmSinkElementTest();
protected:
    void SetUp() final;
    void TearDown() final;
    void InitializeCommonStruct();

    MockIAmControlReceive            *mpMockControlReceiveInterface;
    IAmControlReceive                *mpCAmControlReceive;
    std::shared_ptr<CAmSinkElement>   mpCAmSinkElement;
    std::shared_ptr<CAmSourceElement> mpCAmSourceElement;
    CAmRouteElement                  *mpCAmRouteElement;
    ut_gc_Sink_s                              sinkInfo;
    am_Sink_s                                 sink;
    am_sinkID_t                               sinkID;
    am_MainSoundProperty_s                    mainsoundProperties;
    am_CustomMainSoundPropertyType_t          mainpropertyType;
    am_SoundProperty_s                        soundProperties;
    am_CustomSoundPropertyType_t              propertyType;
    am_CustomConnectionFormat_t               connectionformat;
    am_sourceID_t                             sourceID;
    am_Source_s                               source;
    gc_MainSoundProperty_s                    gcMainSoundProperty;
    gc_SoundProperty_s                        gcSoundProperty;
    ut_gc_Source_s                            sourceInfo;

    std::vector<am_MainSoundProperty_s >      listMainSoundProperties;
    std::vector<gc_MainSoundProperty_s >      listGCMainSoundProperties;
    std::vector<gc_SoundProperty_s >          listGCSoundProperties;
    std::vector<am_CustomConnectionFormat_t > listConnectionFormats;
    std::vector<am_SoundProperty_s >          listSoundProperties;
    std::map<float, float >                 mMapUserToNormalizedVolume;
    std::map<float, float >                   mMapNormalizedToDecibelVolume;
    std::map<gc_MSPMappingDirection_e, std::map<uint16_t, uint16_t > > mapMSPTOSP;

};

}
}

#endif /* PLUGINCONTROLINTERFACEGENERIC_TEST_CAMSINKELEMENTTEST_CAMSINKELEMENTTEST_H_ */
