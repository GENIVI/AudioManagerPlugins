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
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"
#include "CAmElement.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmClassElement.h"
#include "CAmMainConnectionElement.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmSinkElementTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmSinkElementTest
 *@brief : This class is used to test the CAmSinkElement class functionality.
 */
CAmSinkElementTest::CAmSinkElementTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmSinkElement(NULL)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
    , mpCAmSourceElement(NULL)
    , mpCAmRouteElement(NULL)
{
}

CAmSinkElementTest::~CAmSinkElementTest()
{
}

void CAmSinkElementTest::InitializeCommonStruct()
{
    sink.name        = "AnySink1";
    sink.sinkClassID = 78;
    mapMSPTOSP[MD_SP_TO_MSP].insert(std::make_pair(1, 15));
    mapMSPTOSP[MD_MSP_TO_SP].insert(std::make_pair(2, 15));
    mapMSPTOSP[MD_MSP_TO_SP].insert(std::make_pair(3, 15));
    sinkInfo.mapMSPTOSP = mapMSPTOSP;

    soundProperties.type  = MSP_GENIVI_MID;
    soundProperties.value = 30;
    listSoundProperties.push_back(soundProperties);
    sink.listSoundProperties = listSoundProperties;

    mainsoundProperties.type  = MSP_GENIVI_TREBLE;
    mainsoundProperties.value = 10;
    listMainSoundProperties.push_back(mainsoundProperties);
    sink.listMainSoundProperties = listMainSoundProperties;
    propertyType                 = MSP_GENIVI_MID;

    gcMainSoundProperty.type     = MSP_GENIVI_TREBLE;
    gcMainSoundProperty.minValue = 2;
    gcMainSoundProperty.maxValue = 10;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    gcMainSoundProperty.type     = MSP_GENIVI_MID;
    gcMainSoundProperty.minValue = 5;
    gcMainSoundProperty.maxValue = 20;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);

    // Main soundProperty type and value
    mainpropertyType = MSP_GENIVI_TREBLE;
    // gc_Sink_s sinkInfo;
    sinkInfo.listGCMainSoundProperties = listGCMainSoundProperties;

    // GC Sound property
    gcSoundProperty.type     = MSP_GENIVI_TREBLE;
    gcSoundProperty.minValue = 2;
    gcSoundProperty.maxValue = 10;
    listGCSoundProperties.push_back(gcSoundProperty);
    gcSoundProperty.type     = MSP_GENIVI_MID;
    gcSoundProperty.minValue = 5;
    gcSoundProperty.maxValue = 20;
    listGCSoundProperties.push_back(gcSoundProperty);
    sinkInfo.listGCSoundProperties = listGCSoundProperties;

    mMapUserToNormalizedVolume.insert(make_pair(5, 0.5));
    mMapUserToNormalizedVolume.insert(make_pair(7, 0.7));
    mMapUserToNormalizedVolume.insert(make_pair(9, 0.9));
    mMapUserToNormalizedVolume.insert(make_pair(11, 0.11));
    mMapNormalizedToDecibelVolume.insert(make_pair(0.05, 1.5));
    mMapNormalizedToDecibelVolume.insert(make_pair(0.09, 1.1));
    mMapNormalizedToDecibelVolume.insert(make_pair(0.07, 1.3));
    mMapNormalizedToDecibelVolume.insert(make_pair(1.0, 0.09));
    sinkInfo.mainVolume                         = 0;
    sinkInfo.mapUserVolumeToNormalizedVolume    = mMapUserToNormalizedVolume;
    sinkInfo.mapNormalizedVolumeToDecibelVolume = mMapNormalizedToDecibelVolume;
}

void CAmSinkElementTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;

    // Initializing Common Struture
    InitializeCommonStruct();
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    source.name = "AnySource1";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpCAmSourceElement = CAmSourceFactory::createElement(sourceInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSourceElement, NotNull()) << " Source Element Is not Created";
    sourceIDList.push_back(mpCAmSourceElement->getID());
    sourceID = mpCAmSourceElement->getID();

    // creating the routing element
    gc_RoutingElement_s gcRoutingElement;
    gcRoutingElement.name             = mpCAmSourceElement->getName() + mpCAmSinkElement->getName();
    gcRoutingElement.sourceID         = mpCAmSourceElement->getID();
    gcRoutingElement.sinkID           = mpCAmSinkElement->getID();
    gcRoutingElement.domainID         = 2;
    gcRoutingElement.connectionFormat = CF_GENIVI_STEREO;
    mpCAmRouteElement                 = new CAmRouteElement(gcRoutingElement, mpCAmControlReceive);
    ASSERT_THAT(mpCAmRouteElement, NotNull()) << " Route Element Is not Created";

}

void CAmSinkElementTest::TearDown()
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
#ifdef VOLUME_RELATED_TEST
/**
 * @brief : Test to verify the setMuteState function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "setMuteState" function when mute state enum is given as input parameters
 *         and along with other configuration parameters then will check whether setMuteState function
 *         will set the mute state of sink element as in database or not without any Gmock error message.
 *
 * @result : "Pass" when setMuteState function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSinkElementTest, setMuteStatePositive)
{
    am_MuteState_e newmuteState = MS_MUTED;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkMuteStateDB(newmuteState, sinkID)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSinkElement->setMuteState(newmuteState));
}

/**
 * @brief : Test to verify the setMuteState function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "setMuteState" function when mute state enum is given as input parameters
 *         and along with other configuration parameters and also we mock changeSinkMuteStateDB function
 *         to return the error message from DB the then will check whether setMuteState function
 *         will not set the mute state of sink element as in database or not without any Gmock error message.
 *
 * @result : "Pass" when setMuteState function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmSinkElementTest, setMuteStateNegative)
{
    am_MuteState_e newmuteState = MS_MUTED;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkMuteStateDB(newmuteState, sinkID)).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmSinkElement->setMuteState(newmuteState));
}
#endif
/**
 * @brief : Test to verify the setMainVolume function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "setMainVolume" function when main volume is given as input parameters
 *         and along with other configuration parameters and we mock the changeSinkMainVolumeDB then will check whether setMainVolume function
 *         will set the main volume of sink element in database or not without any Gmock error message.
 *
 * @result : "Pass" when setMainVolume function call without any Gmock error message
 */
TEST_F(CAmSinkElementTest, setMainVolumePositive)
{
    am_mainVolume_t newmainVolume = 20;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkMainVolumeDB(newmainVolume, sinkID)).WillOnce(Return(E_OK));
    mpCAmSinkElement->setMainVolume(newmainVolume);
}

/**
 * @brief : Test to verify the setMainVolume function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "setMainVolume" function when main volume is given as input parameters
 *         and along with other configuration parameters and we mock the changeSinkMainVolumeDB to return DB error then will check
 *         whether setMainVolume function will set the main volume of sink element in database or not without any Gmock error message.
 *
 * @result : "Pass" when setMainVolume function call without any Gmock error message
 */
TEST_F(CAmSinkElementTest, setMainVolumeNegative)
{
    am_mainVolume_t newmainVolume = 10;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkMainVolumeDB(newmainVolume, sinkID)).WillOnce(Return(E_NOT_POSSIBLE));
    mpCAmSinkElement->setMainVolume(newmainVolume);
}

/**
 * @brief : Test to verify the setMainSoundPropertyValue function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "setMainSoundPropertyValue" function when type and value of sound property are given as input parameters
 *         and along with other configuration parameters and we mock the changeMainSinkSoundPropertyDB then will check
 *         whether setMainSoundPropertyValue function will set the main sound property value of sink element in database or not without any Gmock error message.
 *
 * @result : "Pass" when setMainSoundPropertyValue function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, setMainSoundPropertyValuePositive)
{
    am_CustomMainSoundPropertyType_t type  = MSP_GENIVI_TREBLE;
    int16_t                          value = 10;
    am_MainSoundProperty_s           amMainsoundProperty;
    amMainsoundProperty.type  = MSP_GENIVI_TREBLE;
    amMainsoundProperty.value = 10;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSinkSoundPropertyDB(setMainSoundproperty(amMainsoundProperty), sinkID)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSinkElement->setMainSoundPropertyValue(type, value));
}

/**
 * @brief : Test to verify the setMainSoundPropertyValue function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "setMainSoundPropertyValue" function when type and value of sound property are given as input parameters
 *         and along with other configuration parameters and we mock the changeMainSinkSoundPropertyDB return DB error then will check
 *         whether setMainSoundPropertyValue function will set not the main sound property value of sink element in database or not without any Gmock error message.
 *
 * @result : "Pass" when setMainSoundPropertyValue function return "E_DATABASE_ERROR", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, setMainSoundPropertyValueNegative)
{
    am_CustomMainSoundPropertyType_t type  = MSP_GENIVI_TREBLE;
    int16_t                          value = 10;
    am_MainSoundProperty_s           amMainsoundProperty;
    amMainsoundProperty.type  = MSP_GENIVI_TREBLE;
    amMainsoundProperty.value = 15;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSinkSoundPropertyDB(setMainSoundproperty(amMainsoundProperty), sinkID)).WillOnce(Return(E_DATABASE_ERROR));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSinkElement->setMainSoundPropertyValue(type, value));
}

/**
 * @brief : Test to verify the getMainSoundPropertyValue function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getMainSoundPropertyValue" function when type and value of sound property are given as input parameters
 *         and along with other configuration parameters then will check whether getMainSoundPropertyValue function will
 *         get the main sound property value of sink element as in database or not, without any Gmock error message.
 *
 * @result : "Pass" when getMainSoundPropertyValue function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getMainSoundPropertyValuePositive)
{
    am_CustomMainSoundPropertyType_t type = MSP_GENIVI_TREBLE;
    int16_t                          value;
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainSinkSoundPropertyValue(sinkID, type, _)).WillOnce(DoAll(SetArgReferee<2>(20), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSinkElement->getMainSoundPropertyValue(type, value));
}

/**
 * @brief : Test to verify the getMainSoundPropertyValue function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getMainSoundPropertyValue" function when type and value of sound property are given as input parameters
 *         and along with other configuration parameters & mock the getMainSinkSoundPropertyValue for Database error, then will
 *         check whether getMainSoundPropertyValue function will not get the main sound property value of sink element as in database or not,
 *         without any Gmock error message.
 *
 * @result : "Pass" when getMainSoundPropertyValue function return "E_DATABASE_ERROR", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getMainSoundPropertyValueNegative)
{
    am_CustomMainSoundPropertyType_t type = MSP_GENIVI_TREBLE;
    int16_t                          value;
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainSinkSoundPropertyValue(sinkID, type, _)).WillOnce(DoAll(SetArgReferee<2>(20), Return(E_DATABASE_ERROR)));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSinkElement->getMainSoundPropertyValue(type, value));
}

/**
 * @brief : Test to verify the setMainNotificationConfiguration function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "setMainNotificationConfiguration" function when type,status, parameters of AM notification configuration are given
 *         as input parameters and along with other configuration parameters then will check whether
 *         setMainNotificationConfiguration function will set the main notification configuration of sink element in database.
 *         or not, without any Gmock error message.
 *
 * @result : "Pass" when setMainNotificationConfiguration function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, setMainNotificationConfigurationPositive)
{
    am_NotificationConfiguration_s notificationConfigurations;
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSinkNotificationConfigurationDB(sinkID, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSinkElement->setMainNotificationConfiguration(notificationConfigurations));
}

/**
 * @brief : Test to verify the setMainNotificationConfiguration function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "setMainNotificationConfiguration" function when type,status, parameters of AM notification configuration are given
 *         as input parameters and the  along with other configuration parameters,
 *         And also  and we mock changeMainSinkNotificationConfigurationDB function to return DB error
 *         then will check whether setMainNotificationConfiguration function will set not the main notification configuration
 *         of sink element in database or not, without any Gmock error message.
 *
 * @result : "Pass" when setMainNotificationConfiguration function return "E_NOT_POSSIBLE", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, setMainNotificationConfigurationNegative)
{
    am_NotificationConfiguration_s notificationConfigurations;
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSinkNotificationConfigurationDB(sinkID, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_NOT_POSSIBLE));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmSinkElement->setMainNotificationConfiguration(notificationConfigurations));
}

/**
 * @brief : Test to verify the getListMainNotificationConfigurations function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getListMainNotificationConfigurations" function when empty list of Main Notification configuration is given
 *         as input parameters and along with other configuration parameters details and we will the audio manager sink structure with details of
 *         Notification configuration then will check whether getListMainNotificationConfigurations function will get the list of main notification
 *         configuration of sink element from database or not, without any Gmock error message.
 *
 * @result : "Pass" when getListMainNotificationConfigurations function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getListMainNotificationConfigurationsPositive)
{
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Sink_s sinkdata;
    sinkdata.listMainNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkdata), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSinkElement->getListMainNotificationConfigurations(listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the getListMainNotificationConfigurations function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getListMainNotificationConfigurations" function when empty list of Main Notification configuration is given
 *         as input parameters and along with other configuration parameters details and we will the audio manager sink structure with details of
 *         Notification configuration & we mock the getSinkInfoDB with return data base error then will check whether getListMainNotificationConfigurations function will not get the list of main notification
 *         configuration of sink element from database or not, without any Gmock error message.
 *
 * @result : "Pass" when getListMainNotificationConfigurations function return "E_DATABASE_ERROR", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getListMainNotificationConfigurationsNegative)
{
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Sink_s sinkdata;
    sinkdata.listMainNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkdata), Return(E_DATABASE_ERROR)));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSinkElement->getListMainNotificationConfigurations(listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the notificationDataUpdate function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "notificationDataUpdate" function when type an value of payload are given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         notificationDataUpdate function will update the notification data or not by send the Main Sink Notification payload by using
 *         sendMainSinkNotificationPayload call or not, without any Gmock error message.
 *
 * @result : "Pass" when notificationDataUpdate function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, notificationDataUpdatePositive)
{
    // Notification payload
    am_NotificationPayload_s payload;
    payload.type  = NT_OVER_TEMPERATURE;
    payload.value = 20;
    EXPECT_CALL(*mpMockControlReceiveInterface, sendMainSinkNotificationPayload(sinkID, setNotificationApyload(payload))).WillOnce(Return());
    EXPECT_EQ(E_OK, mpCAmSinkElement->notificationDataUpdate(payload));
}

/**
 * @brief : Test to verify the getListNotificationConfigurations function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getListNotificationConfigurations" function when list of Main Notification is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         getListNotificationConfigurations function will get list of Main Notification Configuration data or not,
 *         without any Gmock error message.
 *
 * @result : "Pass" when getListNotificationConfigurations function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getListNotificationConfigurationsPositive)
{
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Sink_s sinkdata;
    sinkdata.listNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkdata), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSinkElement->getListNotificationConfigurations(listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the getListNotificationConfigurations function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getListNotificationConfigurations" function when list of Main Notification is given
 *         as input parameters and along with other configuration parameters details & we mock the getSinkInfoDB function for
 *         data base error then will check whether getListNotificationConfigurations function will not get list of Main Notification
 *         Configuration data or not,without any Gmock error message.
 *
 * @result : "Pass" when getListNotificationConfigurations function return "E_NOT_POSSIBLE", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getListNotificationConfigurationsNegative)
{
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Sink_s sinkdata;
    sinkdata.listNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkdata), Return(E_NOT_POSSIBLE)));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmSinkElement->getListNotificationConfigurations(listMainNotificationConfigurations));
}

/**
 * @brief : Test to verify the getNotificationConfigurations function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getNotificationConfigurations" function when type and Notification configuration is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         getNotificationConfigurations function will get Notification Configuration data or not from list
 *         Notification configuration without any Gmock error message.
 *
 * @result : "Pass" when getNotificationConfigurations function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getNotificationConfigurationsPositive)
{
    am_CustomNotificationType_t                  type = NT_OVER_TEMPERATURE;
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Sink_s sinkdata;
    sinkdata.listNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkdata), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSinkElement->getNotificationConfigurations(type, notificationConfigurations));
}

/**
 * @brief : Test to verify the getNotificationConfigurations function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getNotificationConfigurations" function when type and Notification configuration is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         getNotificationConfigurations function will not get Notification Configuration data on base of type from list
 *         Notification configuration without any Gmock error message.
 *
 * @result : "Pass" when getNotificationConfigurations function return "E_UNKNOWN", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getNotificationConfigurationsNegative)
{
    am_CustomNotificationType_t                  type = NT_UNKNOWN;
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Sink_s sinkdata;
    sinkdata.listNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkdata), Return(E_OK)));
    EXPECT_EQ(E_UNKNOWN, mpCAmSinkElement->getNotificationConfigurations(type, notificationConfigurations));
}

/**
 * @brief : Test to verify the getMainNotificationConfigurations function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getMainNotificationConfigurations" function when type and Notification configuration is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         getMainNotificationConfigurations function will get Main Notification Configuration data or not from list
 *         Main Notification configuration base on type without any Gmock error message.
 *
 * @result : "Pass" when getMainNotificationConfigurations function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getMainNotificationConfigurationsPositive)
{
    am_CustomNotificationType_t                  type = NT_OVER_TEMPERATURE;
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Sink_s sinkdata;
    sinkdata.listMainNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkdata), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSinkElement->getMainNotificationConfigurations(type, notificationConfigurations));
}

/**
 * @brief : Test to verify the getMainNotificationConfigurations function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getMainNotificationConfigurations" function when type and Notification configuration is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         getMainNotificationConfigurations function will not get Main Notification Configuration data or not from list
 *         Main Notification configuration base on type without any Gmock error message.
 *
 * @result : "Pass" when getMainNotificationConfigurations function return "E_UNKNOWN", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getMainNotificationConfigurationsNegative)
{
    am_CustomNotificationType_t                  type = NT_UNKNOWN;
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Sink_s sinkdata;
    sinkdata.listMainNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkdata), Return(E_OK)));
    EXPECT_EQ(E_UNKNOWN, mpCAmSinkElement->getMainNotificationConfigurations(type, notificationConfigurations));
}

/**
 * @brief : Test to verify the getSoundPropertyValue function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getSoundPropertyValue" function when type and value are given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         getSoundPropertyValue function will get the sound property value of sink element as in database or not
 *         without any Gmock error message.
 *
 * @result : "Pass" when getSoundPropertyValue function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getSoundPropertyValuePositive)
{
    am_CustomSoundPropertyType_t type = SP_GENIVI_TREBLE;
    int16_t                      value = -100;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkSoundPropertyValue(sinkID, type, _))
        .WillOnce(DoAll(SetArgReferee<2>(20), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSinkElement->getSoundPropertyValue(type, value));
    EXPECT_EQ(20, value);
}

/**
 * @brief : Test to verify the getSoundPropertyValue function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getSoundPropertyValue" function when type and value are given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         getSoundPropertyValue function will not get the sound property value of sink element,
 *         without any Gmock error message.
 *
 * @result : "Pass" when getSoundPropertyValue function return "E_DATABASE_ERROR", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getSoundPropertyValueNegative)
{
    am_CustomSoundPropertyType_t type = SP_GENIVI_TREBLE;
    int16_t                      value = -100;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkSoundPropertyValue(sinkID, type, _))
        .WillOnce(DoAll(SetArgReferee<2>(20), Return(E_DATABASE_ERROR)));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSinkElement->getSoundPropertyValue(type, value));
}

/**
 * @brief : Test to verify the setAvailability function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "setAvailability" function when available is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         setAvailability function will set the availability of sink element in database or not
 *         without any Gmock error message.
 *
 * @result : "Pass" when setAvailability function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, setAvailabilityPositive)
{
    am_Availability_s available;
    // for assigning data to Availability structure
    available.availability       = A_AVAILABLE;
    available.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkAvailabilityDB(setAvailable(available), sinkID)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSinkElement->setAvailability(available));
}

/**
 * @brief : Test to verify the setAvailability function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "setAvailability" function when available is given
 *         as input parameters and along with other configuration parameters details and mock the changeSinkAvailabilityDB function
 *         for DB error for availability of sink element then will check whether setAvailability function will not set the availability
 *         of sink element in database or not, without any Gmock error message.
 *
 * @result : "Pass" when setAvailability function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, setAvailabilityNegative)
{
    am_Availability_s available;
    available.availability       = A_AVAILABLE;
    available.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkAvailabilityDB(setAvailable(available), sinkID)).WillOnce(Return(E_DATABASE_ERROR));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSinkElement->setAvailability(available));
}

/**
 * @brief : Test to verify the getAvailability function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getAvailability" function when available is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         getAvailability function will get the availability of sink element in database or not
 *         without any Gmock error message.
 *
 * @result : "Pass" when getAvailability function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getAvailabilityPositive)
{
    am_Availability_s available;
    // for assigning data to Sink structure
    am_Sink_s sinkdata;
    sinkdata.available.availability       = A_AVAILABLE;
    sinkdata.available.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkdata), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSinkElement->getAvailability(available));
}

/**
 * @brief : Test to verify the getAvailability function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getAvailability" function when available is given
 *         as input parameters and along with other configuration parameters details & mock the getsinkInfoDB for return data error
 *         then will check whether getAvailability function will not get the availability of sink element in database or not
 *         without any Gmock error message.
 *
 * @result : "Pass" when getAvailability function return "E_NOT_POSSIBLE", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getAvailabilityNegative)
{
    am_Availability_s available;
    // for assigning data to Sink structure
    am_Sink_s sinkdata;
    sinkdata.available.availability       = A_AVAILABLE;
    sinkdata.available.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkdata), Return(E_NOT_POSSIBLE)));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmSinkElement->getAvailability(available));
}

/**
 * @brief : Test to verify the saturateMainSoundPropertyRange function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "saturateMainSoundPropertyRange" function when main sound property value is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         saturateMainSoundPropertyRange function will saturate the main sound property value as per the range specified
 *         in configuration file or not,without any Gmock error message.
 *
 * @result : "Pass" when saturateMainSoundPropertyRange function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, saturateMainSoundPropertyRangePositive)
{
    int16_t mainsoundPropertyValue;
    EXPECT_EQ(E_OK, mpCAmSinkElement->saturateMainSoundPropertyRange(mainpropertyType, mainsoundPropertyValue));
}

/**
 * @brief : Test to verify the saturateMainSoundPropertyRange function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "saturateMainSoundPropertyRange" function when main sound property value is given
 *         as input parameters & mainpropertyType set as MSP_UNKNOWN and along with other configuration parameters details then will check whether
 *         saturateMainSoundPropertyRange function will saturate the main sound property value as per the range specified
 *         in configuration file or not,without any Gmock error message.
 *
 * @result : "Pass" when saturateMainSoundPropertyRange function return "E_UNKNOWN", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, saturateMainSoundPropertyRangeNegative)
{
    int16_t mainsoundPropertyValue = 10;
    mainpropertyType = MSP_UNKNOWN;
    EXPECT_EQ(E_UNKNOWN, mpCAmSinkElement->saturateMainSoundPropertyRange(mainpropertyType, mainsoundPropertyValue));
}

/**
 * @brief : Test to verify the saturateMainSoundPropertyRange function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "saturateMainSoundPropertyRange" function when sound property value is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         saturateMainSoundPropertyRange function will saturate the sound property value as per the range specified
 *         in configuration file or not,without any Gmock error message.
 *
 * @result : "Pass" when saturateMainSoundPropertyRange function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, saturateSoundPropertyRangePositive)
{
    int16_t soundPropertyValue;
    EXPECT_EQ(E_OK, mpCAmSinkElement->saturateSoundPropertyRange(propertyType, soundPropertyValue));
}

/**
 * @brief : Test to verify the saturateMainSoundPropertyRange function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "saturateMainSoundPropertyRange" function when sound property value is given
 *         as input parameters and set propertyType as MSP_UNKNOWN, along with other configuration parameters details then will check whether
 *         saturateMainSoundPropertyRange function will not saturate the sound property value as per the range specified
 *         in configuration file or not,without any Gmock error message.
 *
 * @result : "Pass" when saturateMainSoundPropertyRange function return "E_UNKNOWN", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, saturateSoundPropertyRangeNegative)
{
    propertyType = MSP_UNKNOWN;
    int16_t soundPropertyValue;
    EXPECT_EQ(E_UNKNOWN, mpCAmSinkElement->saturateSoundPropertyRange(propertyType, soundPropertyValue));
}

/**
 * @brief : Test to verify the saturateMainSoundPropertyRange function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "upadateDB" function when sink class id and lists of sound properties,connection formats, Main Sound
 *         properties are given as input parameters and along with other configuration parameters details then will check whether
 *         upadateDB function will update the local list of sound properties,connection formats, Main Sound properties and then finally
 *         chnage th sink DB or not with help of changeSinkDB functon,without any Gmock error message.
 *
 * @result : "Pass" when upadateDB function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, upadateDB)
{
    connectionformat = CF_GENIVI_STEREO;
    listConnectionFormats.push_back(connectionformat);
    int16_t mainsoundPropertyValue = 10;
    int16_t soundPropertyValue     = 15;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sink), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSinkDB(sinkID, sink.sinkClassID, setMainSoundproperty(soundProperties), setConnectionFormat(connectionformat), setMainSoundproperty(mainsoundProperties))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSinkElement->upadateDB(sink.sinkClassID, listSoundProperties, listConnectionFormats, listMainSoundProperties));
}

#if 0
/**
 * @brief : Test to verify the saturateVolumeRangePositiveMaxVolume function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "saturateVolumeRangePositiveMaxVolume" function when new Max Volume is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         saturateVolumeRangePositiveMaxVolume function will saturate the main volume value as per the range specified in configuration file
 *         or not,without any Gmock error message.
 *
 * @result : "Pass" when saturateVolumeRangePositiveMaxVolume function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, saturateVolumeRangePositiveMaxVolume)
{
    am_mainVolume_t newVolume = 10;
    EXPECT_EQ(E_OK, mpCAmSinkElement->saturateVolumeRange(newVolume));
}

/**
 * @brief : Test to verify the saturateVolumeRangePositiveMinVolume function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "saturateVolumeRangePositiveMinVolume" function when new mini Volume is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         saturateVolumeRangePositiveMaxVolume function will saturate the main volume value as per the range specified in configuration file
 *         or not,without any Gmock error message.
 *
 * @result : "Pass" when saturateVolumeRangePositiveMinVolume function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, saturateVolumeRangePositiveMinVolume)
{
    am_mainVolume_t newVolume = 5;
    EXPECT_EQ(E_OK, mpCAmSinkElement->saturateVolumeRange(newVolume));
}

/**
 * @brief : Test to verify the saturateVolumeRange function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "saturateVolumeRange" function when new main Volume is given
 *         as input parameters and along with other incorrect configuration parameters details  then will check whether
 *         saturateVolumeRange function will not saturate the main volume value as per the range specified in configuration file
 *         or not,without any Gmock error message.
 *
 * @result : "Pass" when saturateVolumeRange function return "E_UNKNOWN", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, saturateVolumeRangeNegative)
{
    sink.name     = "AnySink1";
    sinkInfo.name = "AnySink1";
    mMapUserToNormalizedVolume.empty();
    mMapNormalizedToDecibelVolume.empty();
    sinkInfo.mapUserVolumeToNormalizedVolume.clear();
    sinkInfo.mapNormalizedVolumeToDecibelVolume.clear();// = mMapNormalizedToDecibelVolume;
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    am_mainVolume_t newVolume;
    EXPECT_NE(E_UNKNOWN, mpCAmSinkElement->saturateVolumeRange(newVolume));
}
#endif
/**
 * @brief : Test to verify the mainSoundPropertyToSoundProperty function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "mainSoundPropertyToSoundProperty" function when type and value of main sound property and sound property are given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         mainSoundPropertyToSoundProperty function will assign the value of main sound property to sound property details or not base on type details
 *         without any Gmock error message.
 *
 * @result : "Pass" when mainSoundPropertyToSoundProperty function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, mainSoundPropertyToSoundPropertyPositive)
{
    soundProperties.type      = SP_GENIVI_TREBLE;
    soundProperties.value     = 15;
    mainsoundProperties.type  = MSP_GENIVI_MID;
    mainsoundProperties.value = 10;
    sinkID                    = mpCAmSinkElement->getID();
    EXPECT_EQ(E_OK, mpCAmSinkElement->mainSoundPropertyToSoundProperty(mainsoundProperties, soundProperties));
}

/**
 * @brief : Test to verify the mainSoundPropertyToSoundProperty function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "mainSoundPropertyToSoundProperty" function when type and value of main sound property and sound property are given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         mainSoundPropertyToSoundProperty function will not assign the value of main sound property to sound property details or not as type details given
 *         without any Gmock error message.
 *
 * @result : "Pass" when mainSoundPropertyToSoundProperty function return "E_DATABASE_ERROR", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, mainSoundPropertyToSoundPropertyNegative)
{
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSinkElement->mainSoundPropertyToSoundProperty(mainsoundProperties, soundProperties));
}

/**
 * @brief : Test to verify the soundPropertyToMainSoundProperty function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "soundPropertyToMainSoundProperty" function when type and value of sound property and main sound property are given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         soundPropertyToMainSoundProperty function will assign the value of sound property to main sound property details or not, based on type details
 *         without any Gmock error message.
 *
 * @result : "Pass" when soundPropertyToMainSoundProperty function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, soundPropertyToMainSoundPropertyPositive)
{
    soundProperties.type      = SP_GENIVI_TREBLE;
    soundProperties.value     = 15;
    mainsoundProperties.type  = MSP_GENIVI_MID;
    mainsoundProperties.value = 10;
    EXPECT_EQ(E_OK, mpCAmSinkElement->soundPropertyToMainSoundProperty(soundProperties, mainsoundProperties));
}

/**
 * @brief : Test to verify the soundPropertyToMainSoundProperty function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "soundPropertyToMainSoundProperty" function when type and value of sound property and main sound property are not given
 *         as input parameters and insert the incorrect value inside the map of main sound property along with other configuration parameters
 *         details then will check whether soundPropertyToMainSoundProperty function will not assign the value of sound property
 *         to main sound property details or not.
 *
 * @result : "Pass" when soundPropertyToMainSoundProperty function return "E_DATABASE_ERROR", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, soundPropertyToMainSoundPropertyNegative)
{
    mapMSPTOSP[MD_SP_TO_MSP].insert(std::make_pair(3, 15));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSinkElement->soundPropertyToMainSoundProperty(soundProperties, mainsoundProperties));
}

/**
 * @brief : Test to verify the convertMainVolumeToVolume function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "convertMainVolumeToVolume" function when main volume which is greater than zero is given
 *         as input parameters to function and along with other configuration parameters details then will check whether
 *         convertMainVolumeToVolume function will convert the main volume to volume as per range given in configuration file
 *         or not, based on type details, without any Gmock error message.
 *
 * @result : "Pass" when convertMainVolumeToVolume function will not return any Gmock error message
 */
TEST_F(CAmSinkElementTest, convertMainVolumeToVolumePositivedeciVolumegreater0)
{
    am_mainVolume_t newVolume = 10;
    am_volume_t     DBVolume;
    EXPECT_THAT(mpCAmSinkElement->convertMainVolumeToVolume(newVolume), A<am_volume_t>());
}

/**
 * @brief : Test to verify the convertMainVolumeToVolume function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "convertMainVolumeToVolume" function when main volume which is less than zero is given
 *         as input parameters to function and along with other configuration parameters details then will check whether
 *         convertMainVolumeToVolume function will convert the main volume to volume as per range given in configuration file
 *         or not, based on type details, without any Gmock error message.
 *
 * @result : "Pass" when convertMainVolumeToVolume function will not return any Gmock error message
 */
TEST_F(CAmSinkElementTest, convertMainVolumeToVolumePositivedeciVolumeless0)
{
    am_mainVolume_t newVolume = 10;
    am_volume_t     DBVolume;
    EXPECT_THAT(mpCAmSinkElement->convertMainVolumeToVolume(newVolume), A<am_volume_t>());
}

/**
 * @brief : Test to verify the convertMainVolumeToVolume function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "convertMainVolumeToVolume" function when main volume which is greater than zero is given
 *         as input parameters to function and along with other configuration parameters details then will check whether
 *         convertMainVolumeToVolume function will convert the main volume to volume as per range given in configuration file
 *         or not, based on type details, without any Gmock error message.
 *
 * @result : "Pass" when convertMainVolumeToVolume function will not return any Gmock error message
 */
TEST_F(CAmSinkElementTest, convertMainVolumeToVolumeNegative)
{
    std::map<float, float > mMapUserToNormalizedVolume;
    std::map<float, float >   mMapNormalizedToDecibelVolume;
    sinkInfo.mapUserVolumeToNormalizedVolume    = mMapUserToNormalizedVolume;
    sinkInfo.mapNormalizedVolumeToDecibelVolume = mMapNormalizedToDecibelVolume;
    am_mainVolume_t newVolume = 10;
    am_volume_t     DBVolume;
    EXPECT_THAT(mpCAmSinkElement->convertMainVolumeToVolume(newVolume), A<am_volume_t>());
}

/**
 * @brief : Test to verify the convertMainVolumeToVolume function with Main volume greater for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "convertMainVolumeToVolume" function when volume which is greater than zero is given
 *         as input parameters to function and along with other configuration parameters details then will check whether
 *         convertMainVolumeToVolume function will convert the volume to main volume as per range given in configuration file
 *         or not, based on type details, without any Gmock error message.
 *
 * @result : "Pass" when convertMainVolumeToVolume function will not return any Gmock error message
 */
TEST_F(CAmSinkElementTest, convertVolumeToMainVolumePositiveMainVolumegreater0)
{
    am_volume_t DBVolume = 5;
    EXPECT_THAT(mpCAmSinkElement->convertVolumeToMainVolume(DBVolume), A<am_volume_t>());
}

/**
 * @brief : Test to verify the convertMainVolumeToVolume function with Main volume greater for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "convertMainVolumeToVolume" function when volume which is less than zero is given
 *         as input parameters to function and along with other configuration parameters details then will check whether
 *         convertMainVolumeToVolume function will convert the volume to main volume as per range given in configuration file
 *         or not, based on type details, without any Gmock error message.
 *
 * @result : "Pass" when convertMainVolumeToVolume function will not return any Gmock error message
 */
TEST_F(CAmSinkElementTest, convertVolumeToMainVolumePositiveMainVolumeLess0)
{
    am_volume_t DBVolume = 5;
    EXPECT_THAT(mpCAmSinkElement->convertVolumeToMainVolume(DBVolume), A<am_volume_t>());
}

/**
 * @brief : Test to verify the convertMainVolumeToVolume function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "convertMainVolumeToVolume" function when volume which is greater than zero is given
 *         as input parameters to function and along with other configuration parameters details then will check whether
 *         convertMainVolumeToVolume function will convert the volume to Main volume as per range given in configuration file
 *         or not, based on type details, without any Gmock error message.
 *
 * @result : "Pass" when convertMainVolumeToVolume function will not return any Gmock error message
 */
TEST_F(CAmSinkElementTest, convertVolumeToMainVolumeNegative)
{
    std::map<float, float > mMapUserToNormalizedVolume;
    std::map<float, float >   mMapNormalizedToDecibelVolume;
    sinkInfo.mapUserVolumeToNormalizedVolume    = mMapUserToNormalizedVolume;
    sinkInfo.mapNormalizedVolumeToDecibelVolume = mMapNormalizedToDecibelVolume;
    mpCAmSinkElement->setID(22);
    am_sinkID_t sinkID   = mpCAmSinkElement->getID();
    am_volume_t DBVolume = 5;
    EXPECT_THAT(mpCAmSinkElement->convertVolumeToMainVolume(DBVolume), A<am_volume_t>());
}

/**
 * @brief : Test to verify the getListConnectionFormats function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getListConnectionFormats" function when list of connection format is given
 *         as input parameters to function and connection format details is push in GC sink structure
 *         along with other configuration parameters details then will check whether getListConnectionFormats function
 *         will get the list of connection format or not, without any Gmock error message.
 *
 * @result : "Pass" when getListConnectionFormats function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getListConnectionFormats)
{
    connectionformat = CF_GENIVI_STEREO;
    listConnectionFormats.push_back(connectionformat);
    sinkInfo.listConnectionFormats = listConnectionFormats;
    EXPECT_EQ(E_OK, mpCAmSinkElement->getListConnectionFormats(listConnectionFormats));
}

/**
 * @brief : Test to verify the getClassID function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getClassID" function when configuration parameters details then
 *         will check whether getClassID function will return the sink class ID or not, without any Gmock error message.
 *
 * @result : "Pass" when getClassID function return "sinkClassID", without any Gmock error message
 */
TEST_F(CAmSinkElementTest, getClassID)
{
    sinkID = mpCAmSinkElement->getID();
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sink), Return(E_OK)));
    EXPECT_EQ(sink.sinkClassID, mpCAmSinkElement->getClassID());
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
