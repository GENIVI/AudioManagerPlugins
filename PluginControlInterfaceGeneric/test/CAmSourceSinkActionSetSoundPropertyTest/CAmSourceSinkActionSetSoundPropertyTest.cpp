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

#include <map>
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"
#include "CAmSourceActionSetState.h"
#include "CAmElement.h"
#include "CAmControllerPlugin.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "audiomanagertypes.h"
#include <pthread.h>
#include "CAmSocketHandler.h"
#include "CAmActionContainer.h"
#include "CAmTimerEvent.h"
#include "CAmHandleStore.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmSourceSinkActionSetSoundProperty.h"
#include "CAmSourceSinkActionSetSoundPropertyTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmSourceSinkActionSetSoundPropertyTest
 *@brief : This class is used to test the CAmSourceSinkActionSetSoundProperty class functionality.
 */
CAmSourceSinkActionSetSoundPropertyTest::CAmSourceSinkActionSetSoundPropertyTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmSinkElement(NULL)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
    , sourceInfo1("AnySource5")
    , sourceInfo2("AnySource6")
    , mpCAmSourceElement(NULL)
    , pFrameworkAction(NULL)
{
}

CAmSourceSinkActionSetSoundPropertyTest::~CAmSourceSinkActionSetSoundPropertyTest()
{
}

void CAmSourceSinkActionSetSoundPropertyTest::InitializeCommonStruct()
{
    sink.name    = "AnySink1";
    source.name  = "AnySource1";
    source1.name = "AnySource5";
    source2.name = "AnySource6";

}

void CAmSourceSinkActionSetSoundPropertyTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;

    InitializeCommonStruct();
    EXPECT_CALL(*mpMockControlReceiveInterface,
        enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee < 1 > (22), Return(E_OK)));
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo,
            mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    source.name = "AnySource1";
    EXPECT_CALL(*mpMockControlReceiveInterface,
        enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee < 1 > (23), Return(E_OK)));
    mpCAmSourceElement = CAmSourceFactory::createElement(sourceInfo,
            mpCAmControlReceive);
    ASSERT_THAT(mpCAmSourceElement, NotNull())
        << " Source Element Is not Created";
    sourceIDList.push_back(mpCAmSourceElement->getID());
    sourceID = mpCAmSourceElement->getID();
}

void CAmSourceSinkActionSetSoundPropertyTest::TearDown()
{
    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();
    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

/**
 * @brief : Test to verify the _execute function of sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmSourceActionSetVolume classes,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "_execute" function class when sound property details are set and push in the list of sound property,
 *         & also set the details of sink structure gc_Sink_s, main sound property type, handle type etc.
 *         & set the behavior of getMainSinkSoundPropertyValue & changeMainSinkSoundPropertyDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether _execute function will get the main sound property value or not base on type,
 *         & also set the Routing Side Property using the main sound property or not, without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetSoundPropertyTest, SinkExecutePositiveCase)
{
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 23;
    soundPropertyList.push_back(soundProperty);
    sink.name                = "AnySink1";
    sink.listSoundProperties = soundPropertyList;
    sinkInfo.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    am_sinkID_t sinkID = 0;
    am_Handle_s handle;
    handle.handleType = H_SETSINKSOUNDPROPERTY;
    handle.handle     = 50;
    am_Error_e                                        result;
    int                                               result_state;
    am_sinkID_t                                       sinkID_out   = 8;
    am_CustomMainSoundPropertyType_t                  propertyType = MSP_GENIVI_TREBLE;
    int16_t                                           value;
    CAmActionParam<am_CustomMainSoundPropertyType_t > mPropertyTypeParam;
    CAmActionParam<int16_t >                          mPropertyValueParam;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainSinkSoundPropertyValue(sinkID, propertyType, _)).WillOnce(DoAll(SetArgReferee<2>(4), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSinkSoundPropertyDB(_, _)).WillOnce(Return(E_OK));
    pFrameworkAction = new CAmSourceSinkActionSetSoundProperty<CAmSinkElement >(mpCAmSinkElement);
    mPropertyTypeParam.setParam(MSP_GENIVI_TREBLE);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);
    mPropertyValueParam.setParam(2);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle, E_OK);
    EXPECT_EQ(pFrameworkAction->getError(), E_OK);
}

/**
 * @brief : Test to verify the _execute function for Negative scenario.
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmSourceActionSetVolume classes,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "_execute" function class when sound property details are set and push in the list of sound property,
 *         & also set the details of sink structure gc_Sink_s, main sound property type, handle type etc.
 *         & set the behavior of getMainSinkSoundPropertyValue & changeMainSinkSoundPropertyDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether _execute function will get the main sound property value or not base on type,
 *         & also set the Routing Side Property using the main sound property or not,
 *         & also notify the Async Result with "E_NOT_POSSIBLE" or not  without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetSoundPropertyTest, SinkExecuteNegativeCase)
{
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 23;
    soundPropertyList.push_back(soundProperty);
    sink.name                = "AnySink1";
    sink.listSoundProperties = soundPropertyList;
    sinkInfo.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    am_sinkID_t sinkID = 0;
    am_Handle_s handle;
    handle.handleType = H_SETSINKSOUNDPROPERTY;
    handle.handle     = 50;
    am_Error_e                                        result;
    int                                               result_state;
    am_sinkID_t                                       sinkID_out   = 8;
    am_CustomMainSoundPropertyType_t                  propertyType = MSP_GENIVI_TREBLE;
    int16_t                                           value;
    CAmActionParam<am_CustomMainSoundPropertyType_t > mPropertyTypeParam;
    CAmActionParam<int16_t >                          mPropertyValueParam;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    pFrameworkAction = new CAmSourceSinkActionSetSoundProperty<CAmSinkElement >(mpCAmSinkElement);
    mPropertyValueParam.setParam(2);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle, E_NOT_POSSIBLE);
    EXPECT_EQ(pFrameworkAction->getError(), E_NOT_POSSIBLE);
}

/**
 * @brief : Test to verify the update function for sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmSourceActionSetVolume classes,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "update" function class when sound property details are set and push in the list of sound property,
 *         & also set the details of sink structure gc_Sink_s, main sound property type, handle type etc.
 *         & set the behavior of getMainSinkSoundPropertyValue & changeMainSinkSoundPropertyDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether execute function will get the main sound property value or not base on type,
 *         & also set the Routing Side Property using the main sound property or not,
 *         & also check the whether update function will set Main Sound Property Value with help of setMainSoundPropertyValue function or not
 *         & it also unregister the observer or not  without any Gmock error.
 *
 * @result : "Pass" when update function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetSoundPropertyTest, SinkupdatePositive)
{
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 23;
    soundPropertyList.push_back(soundProperty);
    sink.name                = "AnySink3";
    sink.listSoundProperties = soundPropertyList;
    sinkInfo.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    am_sinkID_t sinkID = 0;
    am_Handle_s handle;
    handle.handleType = H_SETSINKSOUNDPROPERTY;
    handle.handle     = 50;
    am_Error_e                                        result;
    int                                               result_error;
    am_sinkID_t                                       sinkID_out   = 8;
    am_CustomMainSoundPropertyType_t                  propertyType = MSP_GENIVI_TREBLE;
    int16_t                                           value;
    CAmActionParam<am_CustomMainSoundPropertyType_t > mPropertyTypeParam;
    CAmActionParam<int16_t >                          mPropertyValueParam;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    pFrameworkAction = new CAmSourceSinkActionSetSoundProperty<CAmSinkElement >(mpCAmSinkElement);
    mPropertyTypeParam.setParam(MSP_GENIVI_TREBLE);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);
    mPropertyValueParam.setParam(2);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainSinkSoundPropertyValue(sinkID, propertyType, _)).WillOnce(DoAll(SetArgReferee<2>(4), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSinkSoundPropertyDB(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle, E_OK);
    result_error = pFrameworkAction->getError();
    EXPECT_EQ(pFrameworkAction->update(result_error), E_OK);

}

/**
 * @brief : Test to verify the undo function for Source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmSourceActionSetVolume classes,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "undo" function class when sound property details are set and push in the list of sound property,
 *         & also set the details of Source structure gc_Source_s, main sound property type, handle type etc.
 *         & create two source element, store the details of both source element in db,
 *         & set the behavior of getMainSinkSoundPropertyValue & changeMainSinkSoundPropertyDB to return "E_OK" for both.
 *         & initialize the CAmSourceSinkActionSetSoundProperty class with both source elements
 *         & set the action parameter property type and value for both.
 *         then will check whether execute function will get the main sound property value or not base on type,
 *         & also set the Routing Side Property using the main sound property or not,
 *         & also check the whether undo function will set Routing Side sound Property or not,with old main sound property
 *         & registerObserver or not ,without any Gmock error.
 *
 * @result : "Pass" when undo function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetSoundPropertyTest, SourceUndoTest)
{
    CAmActionContainer pCAmActionContainer;
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 23;
    soundPropertyList.push_back(soundProperty);
    soundProperty.type  = SP_GENIVI_BASS;
    soundProperty.value = 26;
    soundPropertyList.push_back(soundProperty);
    source1.name = "AnySource5";
    source2.name = "AnySource6";
    sourceInfo1.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    sourceInfo2.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    am_sourceID_t sourceID1 = 0, sourceID2 = 0;
    am_Handle_s   handle1, handle2;
    handle1.handleType = H_SETSOURCESOUNDPROPERTY;
    handle1.handle     = 50;
    handle2.handleType = H_SETSOURCESOUNDPROPERTY;
    handle2.handle     = 51;
    am_Error_e                       result;
    int                              result_state;
    am_sourceID_t                    sourceID_out1 = 8, sourceID_out2 = 9;
    am_CustomMainSoundPropertyType_t propertyType1 = MSP_GENIVI_MID, propertyType2 = MSP_GENIVI_BASS;
    int16_t                          value1, value2;

    CAmActionParam<am_CustomMainSoundPropertyType_t > mPropertyTypeParam;
    CAmActionParam<int16_t >                          mPropertyValueParam;

// CAmSourceElement *pSourceElement1;
// CAmSourceElement *pSourceElement2;

    std::shared_ptr<CAmSourceElement> pSourceElement1;
    std::shared_ptr<CAmSourceElement> pSourceElement2;

    MockIAmControlReceive *mpMockControlReceiveInterface1 = new MockIAmControlReceive();
    MockIAmControlReceive *mpMockControlReceiveInterface2 = new MockIAmControlReceive();

    EXPECT_CALL(*mpMockControlReceiveInterface1, enterSourceDB(IsSourceNamePresent(sourceInfo1), _)).WillOnce(DoAll(SetArgReferee < 1 > (22), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface2, enterSourceDB(IsSourceNamePresent(sourceInfo2), _)).WillOnce(DoAll(SetArgReferee < 1 > (23), Return(E_OK)));

    pSourceElement1 = CAmSourceFactory::createElement(sourceInfo1, mpMockControlReceiveInterface1);
    ASSERT_THAT(mpCAmSourceElement, NotNull()) << " Source Element Is not Created";
    sourceIDList.push_back(pSourceElement1->getID());

    pSourceElement2 = CAmSourceFactory::createElement(sourceInfo2, mpMockControlReceiveInterface2);
    ASSERT_THAT(mpCAmSourceElement, NotNull()) << " Source Element Is not Created";
    sourceIDList.push_back(pSourceElement2->getID());
    if (NULL != pSourceElement1)
    {
        sourceID1 = pSourceElement1->getID();
    }

    if (NULL != pSourceElement2)
    {
        sourceID2 = pSourceElement2->getID();
    }

    IAmActionCommand *pFrameworkAction1 = new CAmSourceSinkActionSetSoundProperty<CAmSourceElement >(pSourceElement1);
    IAmActionCommand *pFrameworkAction2 = new CAmSourceSinkActionSetSoundProperty<CAmSourceElement >(pSourceElement2);
    ASSERT_THAT(pFrameworkAction1, NotNull()) << " IAmActionCommand1 Is not Created";
    ASSERT_THAT(pFrameworkAction2, NotNull()) << " IAmActionCommand2 Is not Created";
    mPropertyTypeParam.setParam(MSP_GENIVI_MID);
    pFrameworkAction1->setParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);

    mPropertyValueParam.setParam(2);
    pFrameworkAction1->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);

    mPropertyTypeParam.setParam(MSP_GENIVI_BASS);
    pFrameworkAction2->setParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);

    mPropertyValueParam.setParam(2);
    pFrameworkAction2->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);

    TCLAP::CmdLine *cmd(CAmCommandLineSingleton::instanciateOnce("The team of the AudioManager wishes you a nice day!", ' ', "7.5", true));

    CAmControllerPlugin *pPlugin        = new CAmControllerPlugin();
    CAmSocketHandler    *pSocketHandler = new CAmSocketHandler();

    CAmTimerEvent *mpTimerEvent = CAmTimerEvent::getInstance();
    ASSERT_THAT(mpTimerEvent, NotNull()) << " CAmTimerEvent Is not Created";
    mpTimerEvent->setSocketHandle(pSocketHandler, pPlugin);
    pFrameworkAction1->setUndoRequried(true);
    pFrameworkAction2->setUndoRequried(true);
    pCAmActionContainer.append(pFrameworkAction1);
    pCAmActionContainer.append(pFrameworkAction2);
    pCAmActionContainer.setUndoRequried(true);

    EXPECT_CALL(*mpMockControlReceiveInterface1, getMainSourceSoundPropertyValue(sourceID1, propertyType1, _)).WillOnce(DoAll(SetArgReferee<2>(4), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface2, getMainSourceSoundPropertyValue(sourceID2, _, _)).WillOnce(DoAll(SetArgReferee<2>(4), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface2, changeMainSourceSoundPropertyDB(_, _)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_CALL(*mpMockControlReceiveInterface1, changeMainSourceSoundPropertyDB(_, _)).Times(2).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    EXPECT_EQ(pCAmActionContainer.execute(), 0);
    EXPECT_EQ(pCAmActionContainer.undo(), 0);

    delete (mpMockControlReceiveInterface1);
    delete (mpMockControlReceiveInterface2);

}

/**
 * @brief : Test to verify the _execute function of Source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmSourceActionSetVolume classes,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "_execute" function class when sound property details are set and push in the list of sound property,
 *         & also set the details of Source structure gc_Source_s, main sound property type, handle type etc.
 *         & set the behavior of getMainSourceSoundPropertyValue & changeMainSourceSoundPropertyDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether _execute function will get the main sound property value or not base on type,
 *         & also set the Routing Side Property using the main sound property or not, without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetSoundPropertyTest, SourceExecutePositive)
{
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 23;
    soundPropertyList.push_back(soundProperty);
    source.name                = "AnySource1";
    source.listSoundProperties = soundPropertyList;
    sourceInfo.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    am_sourceID_t sourceID = 0;
    am_Handle_s   handle;
    handle.handleType = H_SETSOURCESOUNDPROPERTY;
    handle.handle     = 50;
    am_Error_e                                        result;
    int                                               result_state;
    am_sourceID_t                                     sourceID_out = 8;
    am_CustomMainSoundPropertyType_t                  propertyType = MSP_GENIVI_TREBLE;
    int16_t                                           value;
    CAmActionParam<am_CustomMainSoundPropertyType_t > mPropertyTypeParam;
    CAmActionParam<int16_t >                          mPropertyValueParam;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainSourceSoundPropertyValue(sourceID, propertyType, _)).WillOnce(DoAll(SetArgReferee<2>(4), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSourceSoundPropertyDB(_, _)).WillOnce(Return(E_OK));
    pFrameworkAction = new CAmSourceSinkActionSetSoundProperty<CAmSourceElement >(mpCAmSourceElement);
    mPropertyTypeParam.setParam(MSP_GENIVI_TREBLE);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);
    mPropertyValueParam.setParam(2);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle, E_OK);
    EXPECT_EQ(pFrameworkAction->getError(), E_OK);
}

/**
 * @brief : Test to verify the _execute function of Source for Negative scenario.
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmSourceActionSetVolume classes,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "_execute" function class when sound property details are set and push in the list of sound property,
 *         & also set the details of Source structure gc_Source_s, main sound property type, handle type etc.
 *         & set the behavior of getMainSourceSoundPropertyValue & changeMainSourceSoundPropertyDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether _execute function will get the main sound property value or not base on type,
 *         & also set the Routing Side Property using the main sound property or not,
 *         & also notify the Async Result with "E_NOT_POSSIBLE" or not  without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetSoundPropertyTest, SourceExecuteNegative)
{
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 23;
    soundPropertyList.push_back(soundProperty);
    source.name                = "AnySource1";
    source.listSoundProperties = soundPropertyList;
    sourceInfo.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    am_sourceID_t sourceID = 0;
    am_Handle_s   handle;
    handle.handleType = H_SETSOURCESOUNDPROPERTY;
    handle.handle     = 50;
    am_Error_e                                        result;
    int                                               result_state;
    am_sourceID_t                                     sourceID_out = 8;
    am_CustomMainSoundPropertyType_t                  propertyType = MSP_GENIVI_BASS;
    int16_t                                           value;
    CAmActionParam<am_CustomMainSoundPropertyType_t > mPropertyTypeParam;
    CAmActionParam<int16_t >                          mPropertyValueParam;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainSourceSoundPropertyValue(sourceID, propertyType, _)).WillOnce(DoAll(SetArgReferee<2>(4), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSourceSoundPropertyDB(_, sourceID)).WillOnce(Return(E_OK));
    pFrameworkAction = new CAmSourceSinkActionSetSoundProperty<CAmSourceElement >(mpCAmSourceElement);
    mPropertyTypeParam.setParam(SP_GENIVI_BASS);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);
    mPropertyValueParam.setParam(2);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    EXPECT_EQ(pFrameworkAction->getError(), E_OK);
}

/**
 * @brief : Test to verify the update function for Source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmSourceActionSetVolume classes,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "update" function class when sound property details are set and push in the list of sound property,
 *         & also set the details of Source structure gc_Source_s, main sound property type, handle type etc.
 *         & set the behavior of getMainSourceSoundPropertyValue & changeMainSourceSoundPropertyDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether execute function will get the main sound property value or not base on type,
 *         & also set the Routing Side Property using the main sound property or not,
 *         & also check the whether update function will set Main Sound Property Value with help of setMainSoundPropertyValue function or not
 *         & it also unregister the observer or not  without any Gmock error.
 *
 * @result : "Pass" when update function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetSoundPropertyTest, SourceupdatePositive)
{
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 23;
    soundPropertyList.push_back(soundProperty);
    source.name                = "AnySource1";
    source.listSoundProperties = soundPropertyList;
    sourceInfo.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    am_sourceID_t sourceID = 0;
    am_Handle_s   handle;
    handle.handleType = H_SETSOURCESOUNDPROPERTY;
    handle.handle     = 50;
    am_Error_e                                        result;
    int                                               result_error;
    am_sourceID_t                                     sourceID_out = 8;
    am_CustomMainSoundPropertyType_t                  propertyType = MSP_GENIVI_TREBLE;
    int16_t                                           value;
    CAmActionParam<am_CustomMainSoundPropertyType_t > mPropertyTypeParam;
    CAmActionParam<int16_t >                          mPropertyValueParam;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainSourceSoundPropertyValue(sourceID, propertyType, _)).WillOnce(DoAll(SetArgReferee<2>(4), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSourceSoundPropertyDB(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    pFrameworkAction = new CAmSourceSinkActionSetSoundProperty<CAmSourceElement >(mpCAmSourceElement);
    mPropertyTypeParam.setParam(MSP_GENIVI_TREBLE);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);
    mPropertyValueParam.setParam(2);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle, E_OK);
    result_error = pFrameworkAction->getError();
    EXPECT_EQ(pFrameworkAction->update(result_error), E_OK);

}

/**
 * @brief : Test to verify the update function for Source for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmSourceActionSetVolume classes,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "update" function class when sound property details are set and push in the list of sound property,
 *         & also set the details of Source structure gc_Source_s, main sound property type, handle type etc.
 *         & set the behavior of getMainSourceSoundPropertyValue & changeMainSourceSoundPropertyDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether execute function will get the main sound property value or not base on type,
 *         & also set the Routing Side Property using the main sound property or not,
 *         & also check the whether update function will set Main Sound Property Value with help of setMainSoundPropertyValue function or not
 *         & it also unregister the observer or not  without any Gmock error.
 *
 * @result : "Pass" when update function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetSoundPropertyTest, SourceupdateNegative)
{
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 23;
    soundPropertyList.push_back(soundProperty);
    source.name                = "AnySource1";
    source.listSoundProperties = soundPropertyList;
    sourceInfo.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    am_sourceID_t sourceID = 0;
    am_Handle_s   handle;
    handle.handleType = H_SETSOURCESOUNDPROPERTY;
    handle.handle     = 50;
    am_Error_e                       result;
    int                              result_error;
    am_sourceID_t                    sourceID_out = 8;
    am_CustomMainSoundPropertyType_t propertyType = MSP_GENIVI_TREBLE;
    int16_t                          value;

    CAmActionParam<am_CustomMainSoundPropertyType_t > mPropertyTypeParam;
    CAmActionParam<int16_t >                          mPropertyValueParam;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainSourceSoundPropertyValue(sourceID, propertyType, _)).WillOnce(DoAll(SetArgReferee<2>(4), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSourceSoundPropertyDB(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    pFrameworkAction = new CAmSourceSinkActionSetSoundProperty<CAmSourceElement >(mpCAmSourceElement);
    mPropertyTypeParam.setParam(MSP_GENIVI_TREBLE);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);
    mPropertyValueParam.setParam(2);
    pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle, E_NOT_POSSIBLE);
    result_error = pFrameworkAction->getError();
    EXPECT_EQ(pFrameworkAction->update(result_error), E_OK);

}

void *WorkerThread(void *data)
{
    CAmSocketHandler *pSocketHandler = (CAmSocketHandler *)data;
    pSocketHandler->start_listenting();
    return NULL;
}

/**
 * @brief : Test to verify the undo function for Source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive,CAmSourceActionSetVolume classes,
 *        Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "undo" function class when sound property details are set and push in the list of sound property,
 *         & also set the details of Source structure gc_Source_s, main sound property type,action, handle type etc.
 *         & create two source element, store the details of both source element in db,
 *         & set the behavior of getMainSinkSoundPropertyValue & changeMainSinkSoundPropertyDB to return "E_OK" for both.
 *         & initialize the CAmSourceSinkActionSetSoundProperty class with both source elements
 *         & set the action parameter property type and value for both and append the action in the action container.
 *         & set the UndoRequried flag to true for both source element instance.
 *         then will check whether execute function will get the main sound property value or not base on type,
 *         & also set the Routing Side Property using the main sound property or not,
 *         & also check the whether undo function will set Routing Side sound Property or not,with old main sound property
 *         & registerObserver or not ,without any Gmock error.
 *
 * @result : "Pass" when undo function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetSoundPropertyTest, SourceTimeoutTest)
{
    // AudioManager source element
    CAmActionContainer pCAmActionContainer;
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 23;
    soundPropertyList.push_back(soundProperty);
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 26;
    soundPropertyList.push_back(soundProperty);
    source1.name = "AnySource7";
    source2.name = "AnySource8";
    sourceInfo1.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    sourceInfo2.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    am_sourceID_t sourceID1 = 0, sourceID2 = 0;
    am_Handle_s   handle1, handle2;
    handle1.handleType = H_SETSOURCESOUNDPROPERTY;
    handle1.handle     = 50;
    handle2.handleType = H_SETSOURCESOUNDPROPERTY;
    handle2.handle     = 51;
    am_Error_e result;
    int        result_state;

    // out parameter for property
    am_sourceID_t                    sourceID_out1 = 8, sourceID_out2 = 9;
    am_CustomMainSoundPropertyType_t propertyType1 = MSP_GENIVI_TREBLE, propertyType2 = MSP_GENIVI_TREBLE;
    int16_t                          value1, value2;

    CAmActionParam<am_CustomMainSoundPropertyType_t > mPropertyTypeParam;
    CAmActionParam<int16_t >                          mPropertyValueParam;
    // timer related
    CAmSocketHandler pSocketHandler;
    pthread_t        thread_id;
    CAmControllerPlugin *pPlugin;

    CAmTimerEvent *mpTimerEvent = CAmTimerEvent::getInstance();
    if (NULL == mpTimerEvent)
    {
        ASSERT_THAT(mpTimerEvent, IsNull()) << "CAmSourceSinkActionSetSoundPropertyTest class: Not able to create Timer object";
    }

    mpTimerEvent->setSocketHandle(&pSocketHandler, pPlugin);
    std::shared_ptr<CAmSourceElement> pSourceElement1;
    std::shared_ptr<CAmSourceElement> pSourceElement2;

    MockIAmControlReceive *mpMockControlReceiveInterface1 = new MockIAmControlReceive();
    MockIAmControlReceive *mpMockControlReceiveInterface2 = new MockIAmControlReceive();

    EXPECT_CALL(*mpMockControlReceiveInterface1, enterSourceDB(IsSourceNamePresent(sourceInfo1), _)).WillOnce(DoAll(SetArgReferee < 1 > (22), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface2, enterSourceDB(IsSourceNamePresent(sourceInfo2), _)).WillOnce(DoAll(SetArgReferee < 1 > (23), Return(E_OK)));
    pSourceElement1 = CAmSourceFactory::createElement(sourceInfo1, mpMockControlReceiveInterface1);
    ASSERT_THAT(mpCAmSourceElement, NotNull()) << " Source Element1 Is not Created";
    sourceIDList.push_back(pSourceElement1->getID());
    pSourceElement2 = CAmSourceFactory::createElement(sourceInfo2, mpMockControlReceiveInterface2);
    ASSERT_THAT(mpCAmSourceElement, NotNull()) << " Source Element2 Is not Created";
    sourceIDList.push_back(pSourceElement2->getID());
    if (NULL != pSourceElement1)
    {
        sourceID1 = pSourceElement1->getID();
    }

    if (NULL != pSourceElement2)
    {
        sourceID2 = pSourceElement2->getID();
    }

    IAmActionCommand *pFrameworkAction1 = new CAmSourceSinkActionSetSoundProperty<CAmSourceElement >(pSourceElement1);
    IAmActionCommand *pFrameworkAction2 = new CAmSourceSinkActionSetSoundProperty<CAmSourceElement >(pSourceElement2);

    if (pFrameworkAction1 != NULL)
    {
        printf("action created is \n");
    }

    mPropertyTypeParam.setParam(MSP_GENIVI_TREBLE);
    pFrameworkAction1->setParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);
    mPropertyValueParam.setParam(2);
    pFrameworkAction1->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);
    mPropertyTypeParam.setParam(MSP_GENIVI_TREBLE);
    pFrameworkAction2->setParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);
    mPropertyValueParam.setParam(2);
    pFrameworkAction2->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);
    pFrameworkAction2->setTimeout(3000);
    pFrameworkAction1->setUndoRequried(true);
    pFrameworkAction2->setUndoRequried(true);
    pCAmActionContainer.append(pFrameworkAction1);
    pCAmActionContainer.append(pFrameworkAction2);
    pCAmActionContainer.setUndoRequried(true);
    EXPECT_CALL(*mpMockControlReceiveInterface1, getMainSourceSoundPropertyValue(sourceID1, _, _)).WillOnce(DoAll(SetArgReferee<2>(4), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface1, changeMainSourceSoundPropertyDB(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface1, setSourceSoundProperty(_, sourceID1, _)).WillOnce(DoAll(SetArgReferee<0>(handle1), Return(E_OK)));
    EXPECT_EQ(pCAmActionContainer.execute(), 0);
    CAmHandleStore::instance().notifyAsyncResult(handle1, E_OK);
    EXPECT_CALL(*mpMockControlReceiveInterface2, getMainSourceSoundPropertyValue(sourceID2, propertyType2, _)).WillOnce(DoAll(SetArgReferee<2>(4), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface2, setSourceSoundProperty(_, sourceID2, _)).WillOnce(DoAll(SetArgReferee<0>(handle2), Return(E_OK)));
    EXPECT_EQ(pCAmActionContainer.execute(), 0);
    pthread_create(&thread_id, NULL, WorkerThread, &pSocketHandler);
    EXPECT_CALL(*mpMockControlReceiveInterface2, abortAction(IsSourceHandleOk(handle2))).WillOnce((Return(E_OK)));
    sleep(5);
    EXPECT_CALL(*mpMockControlReceiveInterface1, setSourceSoundProperty(_, sourceID1, _)).WillOnce(DoAll(SetArgReferee<0>(handle1), Return(E_OK)));
    EXPECT_EQ(pCAmActionContainer.undo(), 0);
    pSocketHandler.exit_mainloop();
    void *ret;
    pthread_join(thread_id, &ret);
    delete (mpMockControlReceiveInterface1);
    delete (mpMockControlReceiveInterface2);

}

int main(int argc, char * *argv)
{
    // initialize logging environment
    am::CAmLogWrapper::instantiateOnce("UTST", "Unit test for generic controller"
            , LS_ON, LOG_SERVICE_STDOUT);
    LOG_FN_CHANGE_LEVEL(LL_WARN);

    // redirect configuration path
    gc_utest::ConfigDocument config(gc_utest::ConfigDocument::Default);

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
