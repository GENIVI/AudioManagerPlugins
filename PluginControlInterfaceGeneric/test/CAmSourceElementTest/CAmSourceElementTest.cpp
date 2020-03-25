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
#include "CAmSourceElementTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmSourceElementTest
 *@brief : This class is used to test the CAmSourceElement class functionality.
 */
CAmSourceElementTest::CAmSourceElementTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmSinkElement(NULL)
    , mpCAmSourceElement(NULL)
    , mpCAmRouteElement(NULL)
    , sourceInfo("AnySource1")
{
}

CAmSourceElementTest::~CAmSourceElementTest()
{
}

void CAmSourceElementTest::InitializeGCCommonStruct()
{
    /// Initialize Source Element
    // GC Sound property
    gcSoundProperty.type     = MSP_GENIVI_TREBLE;
    gcSoundProperty.minValue = 2;
    gcSoundProperty.maxValue = 10;
    listGCSoundProperties.push_back(gcSoundProperty);
    gcSoundProperty.type     = MSP_GENIVI_MID;
    gcSoundProperty.minValue = 5;
    gcSoundProperty.maxValue = 20;
    listGCSoundProperties.push_back(gcSoundProperty);
    sourceInfo.listGCSoundProperties = listGCSoundProperties;
    // GC Main Sound property
    gcMainSoundProperty.type     = MSP_GENIVI_TREBLE;
    gcMainSoundProperty.minValue = 2;
    gcMainSoundProperty.maxValue = 10;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    gcMainSoundProperty.type     = MSP_GENIVI_MID;
    gcMainSoundProperty.minValue = 5;
    gcMainSoundProperty.maxValue = 20;
    listGCMainSoundProperties.push_back(gcMainSoundProperty);
    sourceInfo.listGCMainSoundProperties = listGCMainSoundProperties;
    // Map for storing MSP to SP data
    mapMSPTOSP[MD_SP_TO_MSP].insert(std::make_pair(1, 15));
    mapMSPTOSP[MD_MSP_TO_SP].insert(std::make_pair(2, 15));
    sourceInfo.mapMSPTOSP = mapMSPTOSP;
    mapSourceVolume.insert(pair<float, float>(12.0, 5.0));
    mapSourceVolume.insert(pair<float, float>(10.0, 4.5));
    mapSourceVolume.insert(pair<float, float>(9.0, 3.5));
    sourceInfo.mapRoutingVolume = mapSourceVolume;
}

void CAmSourceElementTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;

    // Initialize Sink Element
    ut_gc_Sink_s sinkInfo("AnySink1");
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    // InitializeGenericControllerCommonStrutureOnly
    InitializeGCCommonStruct();
    source.name               = "AnySource1";
    soundProperties.type      = SP_GENIVI_TREBLE;
    soundProperties.value     = 15;
    mainsoundProperties.type  = MSP_GENIVI_MID;
    mainsoundProperties.value = 10;
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

void CAmSourceElementTest::TearDown()
{
    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }

    if (mpCAmRouteElement != NULL)
    {
        delete (mpCAmRouteElement);
        mpCAmRouteElement = NULL;
    }
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
 *         and along with other configuration parameters and we mock the changeMainSourceSoundPropertyDB then will check
 *         whether setMainSoundPropertyValue function will  set the main sound property value of source element in database or not without any Gmock error message.
 *
 * @result : "Pass" when setMainSoundPropertyValue function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, setMainSoundPropertyValuePositive)
{
    am_CustomMainSoundPropertyType_t type  = MSP_GENIVI_TREBLE;
    int16_t                          value = 10;
    am_MainSoundProperty_s           amMainsoundProperty;
    amMainsoundProperty.type  = MSP_GENIVI_TREBLE;
    amMainsoundProperty.value = 10;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSourceSoundPropertyDB(setMainSoundproperty(amMainsoundProperty), sourceID)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSourceElement->setMainSoundPropertyValue(type, value));
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
 *         and along with other configuration parameters and we mock the changeMainSourceSoundPropertyDB return DB error then will check
 *         whether setMainSoundPropertyValue function will set not the main sound property value of sink element in database or not without any Gmock error message.
 *
 * @result : "Pass" when setMainSoundPropertyValue function return "E_DATABASE_ERROR", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, setMainSoundPropertyValueNegative)
{
    am_CustomMainSoundPropertyType_t type  = MSP_GENIVI_TREBLE;
    int16_t                          value = 10;
    am_MainSoundProperty_s           amMainsoundProperty;
    amMainsoundProperty.type  = MSP_GENIVI_TREBLE;
    amMainsoundProperty.value = 10;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSourceSoundPropertyDB(setMainSoundproperty(amMainsoundProperty), sourceID)).WillOnce(Return(E_DATABASE_ERROR));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSourceElement->setMainSoundPropertyValue(type, value));
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
TEST_F(CAmSourceElementTest, setMainNotificationConfigurationPositive)
{
    am_NotificationConfiguration_s notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSourceNotificationConfigurationDB(sourceID, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSourceElement->setMainNotificationConfiguration(notificationConfigurations));
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
TEST_F(CAmSourceElementTest, setMainNotificationConfigurationNegative)
{
    am_NotificationConfiguration_s notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSourceNotificationConfigurationDB(sourceID, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_DATABASE_ERROR));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSourceElement->setMainNotificationConfiguration(notificationConfigurations));
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
 *         notificationDataUpdate function will update the notification data or not, by send the Main Source Notification payload by using
 *         sendMainSourceNotificationPayload call or not, without any Gmock error message.
 *
 * @result : "Pass" when notificationDataUpdate function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, notificationDataUpdatePositive)
{
    am_NotificationPayload_s payload;
    payload.type  = NT_OVER_TEMPERATURE;
    payload.value = 20;
    EXPECT_CALL(*mpMockControlReceiveInterface, sendMainSourceNotificationPayload(sourceID, setNotificationApyload(payload))).WillOnce(Return());
    EXPECT_EQ(E_OK, mpCAmSourceElement->notificationDataUpdate(payload));
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
 *         as input parameters and along with other configuration parameters details and also the audio manager source structure with details of
 *         Notification configuration is set then will check whether getListMainNotificationConfigurations function will get the list of
 *         main notification configuration of source element from database or not, without any Gmock error message.
 *
 * @result : "Pass" when getListMainNotificationConfigurations function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getListMainNotificationConfigurationsPositive)
{
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Source_s sourcedata;
    sourcedata.listMainNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSourceElement->getListMainNotificationConfigurations(listMainNotificationConfigurations));
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
 *         as input parameters and along with other configuration parameters details and also the audio manager souce structure with details of
 *         Notification configuration is set & mock the getSinkInfoDB with return data base error then will check whether getListMainNotificationConfigurations function will not
 *         get the list of main notification configuration of source element from database or not, without any Gmock error message.
 *
 * @result : "Pass" when getListMainNotificationConfigurations function return "E_DATABASE_ERROR", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getListMainNotificationConfigurationsNegative)
{
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Source_s sourcedata;
    sourcedata.listMainNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_DATABASE_ERROR)));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSourceElement->getListMainNotificationConfigurations(listMainNotificationConfigurations));
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
TEST_F(CAmSourceElementTest, getNotificationConfigurationsPositive)
{
    am_CustomNotificationType_t                  type = NT_OVER_TEMPERATURE;
    std::vector<am_NotificationConfiguration_s > listNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Source_s sourcedata;
    sourcedata.listNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSourceElement->getNotificationConfigurations(type, notificationConfigurations));
}

/**
 * @brief : Test to verify the getNotificationConfigurations function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getNotificationConfigurations" function when type and Notification configurations are given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         getNotificationConfigurations function will not get Notification Configuration data on base of type from list
 *         Notification configuration without any Gmock error message.
 *
 * @result : "Pass" when getNotificationConfigurations function return "E_UNKNOWN", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getNotificationConfigurationsNegative)
{
    am_CustomNotificationType_t                  type = NT_UNKNOWN;
    std::vector<am_NotificationConfiguration_s > listNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Source_s sourcedata;
    sourcedata.listNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_OK)));
    EXPECT_EQ(E_UNKNOWN, mpCAmSourceElement->getNotificationConfigurations(type, notificationConfigurations));
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
TEST_F(CAmSourceElementTest, getMainNotificationConfigurationsPositive)
{
    am_CustomNotificationType_t                  type = NT_OVER_TEMPERATURE;
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Source_s sourcedata;
    sourcedata.listMainNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSourceElement->getMainNotificationConfigurations(type, notificationConfigurations));
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
TEST_F(CAmSourceElementTest, getMainNotificationConfigurationsNegative)
{
    am_CustomNotificationType_t                  type = NT_UNKNOWN;
    std::vector<am_NotificationConfiguration_s > listMainNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Source_s sourcedata;
    sourcedata.listMainNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_OK)));
    EXPECT_EQ(E_UNKNOWN, mpCAmSourceElement->getMainNotificationConfigurations(type, notificationConfigurations));
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
 *         as input parameters and along with other configuration parameters details and source details of AM then will check whether
 *         getListNotificationConfigurations function will get list of Main Notification Configuration data or not,
 *         without any Gmock error message.
 *
 * @result : "Pass" when getListNotificationConfigurations function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getListNotificationConfigurationsPositive)
{
    std::vector<am_NotificationConfiguration_s > listNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Source_s sourcedata;
    sourcedata.listNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSourceElement->getListNotificationConfigurations(listNotificationConfigurations));
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
 *         as input parameters and along with other configuration parameters details & also mock the getSinkInfoDB function for
 *         data base error then will check whether getListNotificationConfigurations function will not get list of Main Notification
 *         Configuration data or not,without any Gmock error message.
 *
 * @result : "Pass" when getListNotificationConfigurations function return "E_NOT_POSSIBLE", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getListNotificationConfigurationsNegative)
{
    std::vector<am_NotificationConfiguration_s > listNotificationConfigurations;
    am_NotificationConfiguration_s               notificationConfigurations;
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    am_Source_s sourcedata;
    sourcedata.listNotificationConfigurations.push_back(notificationConfigurations);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_DATABASE_ERROR)));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSourceElement->getListNotificationConfigurations(listNotificationConfigurations));
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
 *         getSoundPropertyValue function will get the sound property value of source element as in database or not
 *         without any Gmock error message.
 *
 * @result : "Pass" when getSoundPropertyValue function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getSoundPropertyValuePositive)
{
    am_CustomSoundPropertyType_t type = SP_GENIVI_TREBLE;
    int16_t                      value = -100;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceSoundPropertyValue(sourceID, type, _))
        .WillOnce(DoAll(SetArgReferee<2>(20), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSourceElement->getSoundPropertyValue(type, value));
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
 *         getSoundPropertyValue function will not get the sound property value of source element,
 *         without any Gmock error message.
 *
 * @result : "Pass" when getSoundPropertyValue function return "E_DATABASE_ERROR", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getSoundPropertyValueNegative)
{
    am_CustomSoundPropertyType_t type = SP_GENIVI_TREBLE;
    int16_t                      value = -100;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceSoundPropertyValue(sourceID, type, _))
        .WillOnce(DoAll(SetArgReferee<2>(20), Return(E_DATABASE_ERROR)));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSourceElement->getSoundPropertyValue(type, value));
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
 *         setAvailability function will set the availability of source element in database or not
 *         without any Gmock error message.
 *
 * @result : "Pass" when setAvailability function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, setAvailabilityPositive)
{
    am_Availability_s available;
    available.availability       = A_AVAILABLE;
    available.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSourceAvailabilityDB(setAvailable(available), sourceID)).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSourceElement->setAvailability(available));
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
 *         for DB error for availability of source element then will check whether setAvailability function will not set the availability
 *         of sink element in database or not, without any Gmock error message.
 *
 * @result : "Pass" when setAvailability function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, setAvailabilityNegative)
{
    am_Availability_s available;
    available.availability       = A_AVAILABLE;
    available.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSourceAvailabilityDB(setAvailable(available), sourceID)).WillOnce(Return(E_DATABASE_ERROR));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSourceElement->setAvailability(available));
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
 *         getAvailability function will get the availability of source element in database or not
 *         without any Gmock error message.
 *
 * @result : "Pass" when getAvailability function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getAvailabilityPositive)
{
    am_Availability_s available;
    am_Source_s       sourcedata;
    sourcedata.available.availability       = A_AVAILABLE;
    sourcedata.available.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmSourceElement->getAvailability(available));
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
 *         then will check whether getAvailability function will not get the availability of source element in database or not
 *         without any Gmock error message.
 *
 * @result : "Pass" when getAvailability function return "E_NOT_POSSIBLE", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getAvailabilityNegative)
{
    am_Availability_s available;
    am_Source_s       sourcedata;
    sourcedata.available.availability       = A_AVAILABLE;
    sourcedata.available.availabilityReason = AR_GENIVI_NEWMEDIA;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_DATABASE_ERROR)));
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSourceElement->getAvailability(available));
}

TEST_F(CAmSourceElementTest, getStatePositive)
{
    source.sourceState = SS_ON;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
    EXPECT_EQ(SS_ON, mpCAmSourceElement->getState());
}

TEST_F(CAmSourceElementTest, getStateNegative)
{
    int         state;
    am_Source_s sourcedata;
    sourcedata.sourceState = SS_ON;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(sourcedata), Return(E_DATABASE_ERROR)));
    EXPECT_EQ(SS_UNKNNOWN, mpCAmSourceElement->getState());
}

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
TEST_F(CAmSourceElementTest, mainSoundPropertyToSoundPropertyPositive)
{
    EXPECT_EQ(E_OK, mpCAmSourceElement->mainSoundPropertyToSoundProperty(mainsoundProperties, soundProperties));
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
TEST_F(CAmSourceElementTest, mainSoundPropertyToSoundPropertyNegative)
{
    EXPECT_NE(E_DATABASE_ERROR, mpCAmSourceElement->mainSoundPropertyToSoundProperty(mainsoundProperties, soundProperties));
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
TEST_F(CAmSourceElementTest, soundPropertyToMainSoundPropertyPositive)
{
    EXPECT_EQ(E_OK, mpCAmSourceElement->soundPropertyToMainSoundProperty(soundProperties, mainsoundProperties));
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
TEST_F(CAmSourceElementTest, soundPropertyToMainSoundPropertyNegative)
{
    soundProperties.type      = MSP_GENIVI_BASS;
    soundProperties.value     = 15;
    mainsoundProperties.type  = MSP_GENIVI_BASS;
    mainsoundProperties.value = 10;
    mapMSPTOSP[MD_SP_TO_MSP].insert(std::make_pair(3, 15));
    sourceInfo.mapMSPTOSP = mapMSPTOSP;
    EXPECT_EQ(E_DATABASE_ERROR, mpCAmSourceElement->soundPropertyToMainSoundProperty(soundProperties, mainsoundProperties));
}

/**
 * @brief : Test to verify the saturateMainSoundPropertyRange function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "saturateMainSoundPropertyRange" function when main sound property value annd type is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         saturateMainSoundPropertyRange function will saturate the main sound property value as per the range specified
 *         in configuration file or not,without any Gmock error message.
 *
 * @result : "Pass" when saturateMainSoundPropertyRange function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, saturateMainSoundPropertyRangePositive)
{
    am_MainSoundProperty_s mainsoundProperties;
    mainsoundProperties.type  = MSP_GENIVI_MID;
    mainsoundProperties.value = 10;
    std::vector<am_MainSoundProperty_s > listMainSoundProperties;
    listMainSoundProperties.push_back(mainsoundProperties);
    am_CustomMainSoundPropertyType_t mainpropertyType = MSP_GENIVI_TREBLE;
    int16_t                          mainsoundPropertyValue;
    source.listMainSoundProperties = listMainSoundProperties;
    EXPECT_EQ(E_OK, mpCAmSourceElement->saturateMainSoundPropertyRange(mainpropertyType, mainsoundPropertyValue));
}

/**
 * @brief : Test to verify the saturateMainSoundPropertyRangePositive function with MAX value for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "saturateMainSoundPropertyRange" function when main sound property with MAX value  and type is given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         saturateMainSoundPropertyRange function will saturate the main sound property value as per the range specified
 *         in configuration file or not,without any Gmock error message.
 *
 * @result : "Pass" when saturateMainSoundPropertyRange function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, saturateMainSoundPropertyRangePositiveMaxValue)
{
    am_MainSoundProperty_s mainsoundPropertiesMax;
    mainsoundPropertiesMax.type  = MSP_GENIVI_TREBLE;
    mainsoundPropertiesMax.value = 25;
    std::vector<am_MainSoundProperty_s > listMainSoundPropertiesMax;
    listMainSoundPropertiesMax.push_back(mainsoundPropertiesMax);
    am_CustomMainSoundPropertyType_t mainpropertyType = MSP_GENIVI_TREBLE;
    int16_t                          mainsoundPropertyValue;
    source.listMainSoundProperties = listMainSoundPropertiesMax;
    EXPECT_EQ(E_OK, mpCAmSourceElement->saturateMainSoundPropertyRange(mainpropertyType, mainsoundPropertyValue));
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
TEST_F(CAmSourceElementTest, saturateMainSoundPropertyRangeNegative)
{
    am_MainSoundProperty_s mainsoundProperties;
    mainsoundProperties.type  = MSP_GENIVI_TREBLE;
    mainsoundProperties.value = 10;
    std::vector<am_MainSoundProperty_s > listMainSoundProperties;
    listMainSoundProperties.push_back(mainsoundProperties);
    am_CustomMainSoundPropertyType_t mainpropertyType = MSP_UNKNOWN;
    int16_t                          mainsoundPropertyValue;
    source.listMainSoundProperties = listMainSoundProperties;
    EXPECT_EQ(E_UNKNOWN, mpCAmSourceElement->saturateMainSoundPropertyRange(mainpropertyType, mainsoundPropertyValue));
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
TEST_F(CAmSourceElementTest, saturateSoundPropertyRangePositive)
{
    am_SoundProperty_s soundProperties;
    soundProperties.type  = MSP_GENIVI_MID;
    soundProperties.value = 15;
    std::vector<am_SoundProperty_s > listSoundProperties;
    listSoundProperties.push_back(soundProperties);
    am_CustomSoundPropertyType_t propertyType = MSP_GENIVI_MID;
    int16_t                      soundPropertyValue;
    source.listSoundProperties = listSoundProperties;
    EXPECT_EQ(E_OK, mpCAmSourceElement->saturateSoundPropertyRange(propertyType, soundPropertyValue));
}

/**
 * @brief : Test to verify the saturateMainSoundPropertyRange function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "saturateSoundPropertyRange" function when sound property value and property type are given
 *         as input parameters and along with other configuration parameters details then will check whether
 *         saturateSoundPropertyRange function will saturate the sound property value as per the range specified
 *         in configuration file or not,without any Gmock error message.
 *
 * @result : "Pass" when saturateMainSoundPropertyRange function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, saturateSoundPropertyRangePositiveMaxValue)
{
    am_SoundProperty_s soundProperties;
    soundProperties.type  = MSP_GENIVI_MID;
    soundProperties.value = 30;
    std::vector<am_SoundProperty_s > listSoundProperties;
    listSoundProperties.push_back(soundProperties);
    am_CustomSoundPropertyType_t propertyType = MSP_GENIVI_MID;
    int16_t                      soundPropertyValue;
    source.listSoundProperties = listSoundProperties;
    EXPECT_EQ(E_OK, mpCAmSourceElement->saturateSoundPropertyRange(propertyType, soundPropertyValue));
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
TEST_F(CAmSourceElementTest, saturateSoundPropertyRangeNegative)
{
    am_CustomSoundPropertyType_t propertyType = SP_GENIVI_BASS;
    int16_t                      soundPropertyValue;
    EXPECT_EQ(E_UNKNOWN, mpCAmSourceElement->saturateSoundPropertyRange(propertyType, soundPropertyValue));
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
 *         change the source DB or not with help of changeSourceDB function, without any Gmock error message.
 *
 * @result : "Pass" when upadateDB function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, upadateDB)
{

    am_SoundProperty_s soundProperties;
    soundProperties.type  = SP_GENIVI_TREBLE;
    soundProperties.value = 10;
    std::vector<am_SoundProperty_s >listSoundProperties;
    listSoundProperties.push_back(soundProperties);

    am_CustomConnectionFormat_t               connectionformat = CF_GENIVI_STEREO;
    std::vector<am_CustomConnectionFormat_t > listConnectionFormats;
    listConnectionFormats.push_back(connectionformat);

    am_MainSoundProperty_s mainsoundProperties;
    mainsoundProperties.type  = MSP_GENIVI_MID;
    mainsoundProperties.value = 10;
    std::vector<am_MainSoundProperty_s > listMainSoundProperties;
    listMainSoundProperties.push_back(mainsoundProperties);

    am_CustomMainSoundPropertyType_t mainpropertyType       = MSP_GENIVI_MID;
    int16_t                          mainsoundPropertyValue = 10;
    am_CustomSoundPropertyType_t     propertyType           = SP_GENIVI_TREBLE;
    int16_t                          soundPropertyValue     = 15;
    source.sourceClassID           = 78;
    source.listSoundProperties     = listSoundProperties;
    source.listConnectionFormats   = listConnectionFormats;
    source.listMainSoundProperties = listMainSoundProperties;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeSourceDB(sourceID, source.sourceClassID, setMainSoundproperty(soundProperties), setConnectionFormat(connectionformat), setMainSoundproperty(mainsoundProperties))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpCAmSourceElement->upadateDB(source.sourceClassID, listSoundProperties, listConnectionFormats, listMainSoundProperties));
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
 *         as input parameters to function and connection format details is push in GC source structure
 *         along with other configuration parameters details then will check whether getListConnectionFormats function
 *         will get the list of connection format or not, without any Gmock error message.
 *
 * @result : "Pass" when getListConnectionFormats function return "E_OK", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getListConnectionFormats)
{
    am_CustomConnectionFormat_t               connectionformat = CF_GENIVI_STEREO;
    std::vector<am_CustomConnectionFormat_t > listConnectionFormats;
    listConnectionFormats.push_back(connectionformat);
    source.listConnectionFormats = listConnectionFormats;
    EXPECT_EQ(E_OK, mpCAmSourceElement->getListConnectionFormats(listConnectionFormats));
}

/**
 * @brief : Test to verify the getClassID function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getClassID" function when configuration parameters details then
 *         will check whether getClassID function will return the source class ID or not, without any Gmock error message.
 *
 * @result : "Pass" when getClassID function return "sourceClassID", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getClassID)
{
    source.sourceClassID = 78;
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
    EXPECT_EQ(source.sourceClassID, mpCAmSourceElement->getClassID());
}

/**
 * @brief : Test to verify the getRoutingSideVolume function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getRoutingSideVolume" function when volume is given as input parameter to function and along with other configuration parameters details
 *         then will check whether getRoutingSideVolume function will return the routing side volume or not, without any Gmock error message.
 *
 * @result : "Pass" when getRoutingSideVolume function return will "secondvalue", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getRoutingSideVolumePositive)
{
    float       secondvalue    = 4;
    am_volume_t internalVolume = 9;
    EXPECT_EQ(secondvalue, mpCAmSourceElement->getRoutingSideVolume(internalVolume));
}

/**
 * @brief : Test to verify the getRoutingSideVolume function scenario second for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getRoutingSideVolume" function when different volume is given as input parameter to function and along with other configuration parameters details
 *         then will check whether getRoutingSideVolume function will return the routing side volume or not, without any Gmock error message.
 *
 * @result : "Pass" when getRoutingSideVolume function return will "secondvalue", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getRoutingSideVolumePositive1)
{
    float       secondvalue    = 5;
    am_volume_t internalVolume = 12;
    EXPECT_EQ(secondvalue, mpCAmSourceElement->getRoutingSideVolume(internalVolume));
}

/**
 * @brief : Test to verify the getRoutingSideVolume function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source and Route Elements are created with required input structure parameters
 *        like name,domainID,class name, registration type,volume,sinkID,sourceID etc
 *        Main Sound Property and Sound Property is also initialized and set in the lists.
 *
 * @test : verify the "getRoutingSideVolume" function when volume is given as input parameter to function and along with other configuration parameters details
 *         then will check whether getRoutingSideVolume function will not return the expected routing side volume or not, without any Gmock error message.
 *
 * @result : "Pass" when getRoutingSideVolume function return will "secondvalue", without any Gmock error message
 */
TEST_F(CAmSourceElementTest, getRoutingSideVolumeNegative)
{
    float       secondvalue    = 3;
    am_volume_t internalVolume = 12;
    EXPECT_NE(secondvalue, mpCAmSourceElement->getRoutingSideVolume(internalVolume));
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
