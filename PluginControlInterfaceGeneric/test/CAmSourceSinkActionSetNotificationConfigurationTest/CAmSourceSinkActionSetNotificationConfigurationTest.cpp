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
#include "CAmHandleStore.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmSourceSinkActionSetNotificationConfiguration.h"
#include "CAmSourceSinkActionSetNotificationConfigurationTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

void *WorkerThread(void *data)
{
    CAmSocketHandler *pSocketHandler = (CAmSocketHandler *)data;
    pSocketHandler->start_listenting();
    return NULL;
}

/**
 *@Class : CAmSourceSinkActionSetNotificationConfigurationTest
 *@brief : This class is used to test the CAmSourceSinkActionSetNotificationConfiguration class functionality.
 */
CAmSourceSinkActionSetNotificationConfigurationTest::CAmSourceSinkActionSetNotificationConfigurationTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmSinkElement(NULL)
    , sinkInfo("AnySink1")
    , sinkInfo1("AnySink2")
    , sinkInfo2("AnySink3")
    , sourceInfo("AnySource1")
    , sourceInfo1("AnySource2")
    , sourceInfo2("AnySource3")
    , mpCAmSourceElement(NULL)
    , pFrameworkAction(NULL)
    , mpMockControlReceiveInterface1(NULL)
    , mpMockControlReceiveInterface2(NULL)
    , mpCAmControlReceive1(NULL)
    , mpCAmControlReceive2(NULL)
{
}

CAmSourceSinkActionSetNotificationConfigurationTest::~CAmSourceSinkActionSetNotificationConfigurationTest()
{
}

void CAmSourceSinkActionSetNotificationConfigurationTest::InitializeCommonStruct()
{
    sink.name    = "AnySink1";
    source.name  = "AnySource1";
    source1.name = "AnySource2";
    source2.name = "AnySource3";
    sink1.name   = "AnySink2";
    sink2.name   = "AnySink3";
}

void CAmSourceSinkActionSetNotificationConfigurationTest::SetUp()
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

void CAmSourceSinkActionSetNotificationConfigurationTest::TearDown()
{
    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();
    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }

    if (pFrameworkAction != NULL)
    {
        delete (pFrameworkAction);
    }

    if (mpMockControlReceiveInterface1 != NULL)
    {
        delete (mpMockControlReceiveInterface1);
    }

    if (mpMockControlReceiveInterface2 != NULL)
    {
        delete (mpMockControlReceiveInterface2);
    }
}

/**
 * @brief : Test to verify the _execute function of sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "_execute" function class when notification configuration details like type
 *         ,status,parameter are configured and push in the list of main notification configuration
 *         which further assign to sink element notification configuration.
 *         & initialized the CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property, & also set the action parameter of notification
 *         configuration like type,status etc. & set the behavior of getSinkInfoDB & setSinkNotificationConfiguration,
 *         changeMainSinkNotificationConfigurationDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether _execute function will get Main Notification Configurations details or not,
 *         & set the Sink Notification Configuration details using setSinkNotificationConfiguration function or not
 *         & also registerObserver or not, with status as E_WAIT_FOR_CHILD_COMPLETION  & notify Async Result , without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, SinkExecutePositiveCase)
{
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    notificationConfigurations1.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations1.status    = NS_PERIODIC;
    notificationConfigurations1.parameter = 20;

    sink.name                               = "AnySink1";
    sink.listMainNotificationConfigurations = listMainNotificationConfigurations;
    sinkInfo.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    sinkOut.listMainNotificationConfigurations = listMainNotificationConfigurations;
    am_sinkID_t sinkID = 0;
    handle.handleType = H_SETSINKNOTIFICATION;
    handle.handle     = 50;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    pFrameworkAction = new CAmSourceSinkActionSetNotificationConfiguration<CAmSinkElement >(mpCAmSinkElement);
    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_OFF);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);

    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkOut), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkNotificationConfiguration(_, sinkID, setNotificationConfigurations(notificationConfigurations))).WillOnce(DoAll(SetArgReferee<0>(handle), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSinkNotificationConfigurationDB(sinkID, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_OK));
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle, E_OK);
    EXPECT_EQ(pFrameworkAction->getError(), E_OK);
}

/**
 * @brief : Test to verify the _execute function of sink with no configuration set for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "_execute" function class when notification configuration details like type
 *         ,status,parameter are not configured and push in the list of main notification configuration
 *         which further assign to sink element notification configuration.
 *         & initialized the CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property, & also set the action parameter of notification
 *         configuration like type,status etc. & set the behavior of getSinkInfoDB & setSinkNotificationConfiguration,
 *         changeMainSinkNotificationConfigurationDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether _execute function will get Main Notification Configurations details or not,
 *         & set the Sink Notification Configuration details using setSinkNotificationConfiguration function or not
 *         & also registerObserver or not, with status as E_WAIT_FOR_CHILD_COMPLETION, without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" & getError function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, SinkExecuteNegativeCaseCofigurationNotSet)
{
    am_sinkID_t sinkID = 0;
    handle.handleType = H_SETSOURCENOTIFICATION;
    handle.handle     = 40;
    am_Error_e result;
    int        result_state;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    pFrameworkAction = new CAmSourceSinkActionSetNotificationConfiguration<CAmSinkElement >(mpCAmSinkElement);
    mNotificationStatus.setParam(NS_PERIODIC);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    EXPECT_EQ(pFrameworkAction->getError(), E_NOT_POSSIBLE);
}

/**
 * @brief : Test to verify the _execute function of sink with element as null set for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "_execute" function class when sink element is set as NULL
 *         & initialized the CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property, & also set the action parameter of notification
 *         configuration like type,status etc. & set the behavior of getSinkInfoDB & setSinkNotificationConfiguration,
 *         changeMainSinkNotificationConfigurationDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether _execute function will get Main Notification Configurations details or not,
 *         & set the Sink Notification Configuration details using setSinkNotificationConfiguration function or not
 *         & also registerObserver or not, with status as E_WAIT_FOR_CHILD_COMPLETION,
 *         & getError must return "E_NOT_POSSIBLE", without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" & getError function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, SinkExecuteNegativeCaseElementNull)
{
    mpCAmSinkElement = NULL;
    pFrameworkAction = new CAmSourceSinkActionSetNotificationConfiguration<CAmSinkElement >(mpCAmSinkElement);
    mNotificationStatus.setParam(NS_PERIODIC);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    EXPECT_EQ(pFrameworkAction->getError(), E_NOT_POSSIBLE);

}

/**
 * @brief : Test to verify the _execute function of Source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "_execute" function class when notification configuration details like type
 *         ,status,parameter are configured and push in the list of main notification configuration
 *         which further assign to Source element notification configuration.
 *         & initialized the CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property, & also set the action parameter of notification
 *         configuration like type,status etc. & set the behavior of getSourceInfoDB & setSourceNotificationConfiguration,
 *         changeMainSourceNotificationConfigurationDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether _execute function will get Main Notification Configurations details or not,
 *         & set the source Notification Configuration details using setSourceNotificationConfiguration function or not
 *         & also registerObserver or not, with status as E_WAIT_FOR_CHILD_COMPLETION  & notify Async Result ,
 *         & getError must return "E_OK", without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, SourceExecutePositiveCase)
{
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    source.name                               = "AnySource1";
    source.listMainNotificationConfigurations = listMainNotificationConfigurations;
    am_sourceID_t sourceID = 0;
    handle.handleType = H_SETSOURCENOTIFICATION;
    handle.handle     = 40;
    am_Error_e result;
    int        result_state;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceNotificationConfiguration(_, sourceID, setNotificationConfigurations(notificationConfigurations))).WillOnce(DoAll(SetArgReferee<0>(handle), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, changeMainSourceNotificationConfigurationDB(sourceID, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_OK));
    pFrameworkAction = new CAmSourceSinkActionSetNotificationConfiguration<CAmSourceElement >(mpCAmSourceElement);

    // setting the parameter of Notification Configuration
    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_PERIODIC);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle, E_OK);
    EXPECT_EQ(pFrameworkAction->getError(), E_OK);

}

/**
 * @brief : Test to verify the _execute function of Source with no configuration set for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "_execute" function class when notification configuration details like type
 *         ,status,parameter are not configured and push in the list of main notification configuration
 *         which further assign to Source element notification configuration.
 *         & initialized the CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property, & also set the action parameter of notification
 *         configuration like type,status etc. & not set the behavior of changeMainSourceNotificationConfigurationDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether _execute function will get Main Notification Configurations details or not,
 *         & set the Source Notification Configuration details using setSourceNotificationConfiguration function or not
 *         & also registerObserver or not, with status as E_WAIT_FOR_CHILD_COMPLETION, without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" & getError function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, SourceExecuteNegativeCaseCofigurationNotSet)
{
    source.name = "AnySource1";
    am_sourceID_t sourceID = 0;
    handle.handleType = H_SETSOURCENOTIFICATION;
    handle.handle     = 40;
    am_Error_e result;
    int        result_state;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    pFrameworkAction = new CAmSourceSinkActionSetNotificationConfiguration<CAmSourceElement >(mpCAmSourceElement);
    mNotificationStatus.setParam(NS_PERIODIC);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle, E_NOT_POSSIBLE);
    EXPECT_EQ(pFrameworkAction->getError(), E_NOT_POSSIBLE);

}

/**
 * @brief : Test to verify the _execute function of Source with element as null set for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "_execute" function class when source element is configured to set as NULL,
 *         & initialized the CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property, & also set the action parameter of notification
 *         configuration like type,status etc. & not set the behavior of getSourceInfoDB & setSourceNotificationConfiguration,
 *         to return "E_OK" & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property
 *         type and value.then will check whether _execute function will get Main Notification Configurations details or not,
 *         & set the Source Notification Configuration details using setSinkNotificationConfiguration function or not
 *         & also registerObserver or not, with status as E_WAIT_FOR_CHILD_COMPLETION,
 *         & getError must return "E_NOT_POSSIBLE", without any Gmock error.
 *
 * @result : "Pass" when _execute function return "E_OK" & getError function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, SourceExecuteNegativeCaseElementNull)
{
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    source.name                               = "AnySource1";
    source.listMainNotificationConfigurations = listMainNotificationConfigurations;
    am_sourceID_t sourceID = 0;
    handle.handleType = H_SETSOURCENOTIFICATION;
    handle.handle     = 40;
    am_Error_e result;
    int        result_state;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    pFrameworkAction = new CAmSourceSinkActionSetNotificationConfiguration<CAmSourceElement >(mpCAmSourceElement);
    mNotificationStatus.setParam(NS_PERIODIC);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    EXPECT_EQ(pFrameworkAction->getError(), E_NOT_POSSIBLE);
}

/**
 * @brief : Test to verify the update function of sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "update" function class when notification configuration details like type
 *         ,status,parameter are configured and push in the list of main notification configuration
 *         which further assign to sink element notification configuration.
 *         & initialized the CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property, & also set the action parameter of notification
 *         configuration like type,status etc. & set the behavior of getSinkInfoDB & setSinkNotificationConfiguration,
 *         changeMainSinkNotificationConfigurationDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then make sure the _execute function to will return "E_OK" and then will check whether
 *         update function will set the main notification configuration using the setMainNotificationConfiguration or not,
 *         & it unregister the observer or not, without any Gmock error.
 *
 * @result : "Pass" when update function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, SinkUpdateNegativeCase)
{
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    listMainNotificationConfigurations.push_back(notificationConfigurations);

    notificationConfigurations1.type        = NT_OVER_TEMPERATURE;
    notificationConfigurations1.status      = NS_PERIODIC;
    notificationConfigurations1.parameter   = 20;
    sink.name                               = "AnySink1";
    sink.listMainNotificationConfigurations = listMainNotificationConfigurations;
    sinkInfo.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    sinkOut.listMainNotificationConfigurations = listMainNotificationConfigurations;
    int         error  = E_OK;
    am_sinkID_t sinkID = 0;
    handle.handleType = H_SETSINKNOTIFICATION;
    handle.handle     = 50;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    pFrameworkAction = new CAmSourceSinkActionSetNotificationConfiguration<CAmSinkElement >(mpCAmSinkElement);
    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_OFF);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sinkOut), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkNotificationConfiguration(_, sinkID, setNotificationConfigurations(notificationConfigurations))).WillOnce(DoAll(SetArgReferee<0>(handle), Return(E_OK)));
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    error = pFrameworkAction->getError();
    EXPECT_EQ(error, -1);
    EXPECT_EQ(pFrameworkAction->update(error), E_OK);
}

/**
 * @brief : Test to verify the update function of sink for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "update" function class when notification configuration details like type
 *         ,status,parameter are configured and push in the list of main notification configuration
 *         which further assign to sink element notification configuration.
 *         & initialized the CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property, & also set the action parameter of notification
 *         configuration like type,status etc. & set the behavior of getSinkInfoDB & setSinkNotificationConfiguration,
 *         changeMainSinkNotificationConfigurationDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether _execute function will get Main Notification Configurations details or not,
 *         & set the Source Notification Configuration details using setSinkNotificationConfiguration function or not
 *         & also registerObserver or not, with status as E_WAIT_FOR_CHILD_COMPLETION,
 *         & getError must return "E_NOT_POSSIBLE", without any Gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" & getError will return the "E_NOT_POSSIBLE", without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, UpdateSinkNegativeCase)
{

    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    sink.name                               = "AnySink1";
    sink.listMainNotificationConfigurations = listMainNotificationConfigurations;
    am_sinkID_t sinkID = 0;
    handle.handleType = H_SETSOURCENOTIFICATION;
    handle.handle     = 40;
    am_Error_e result;
    int        result_state;
    if (NULL != mpCAmSinkElement)
    {
        sinkID = mpCAmSinkElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(sinkID, _)).WillOnce(DoAll(SetArgReferee<1>(sink), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkNotificationConfiguration(_, sinkID, setNotificationConfigurations(notificationConfigurations))).WillOnce(DoAll(SetArgReferee<0>(handle), Return(E_OK)));
    pFrameworkAction = new CAmSourceSinkActionSetNotificationConfiguration<CAmSinkElement >(mpCAmSinkElement);
    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_PERIODIC);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle, E_NOT_POSSIBLE);
    EXPECT_EQ(pFrameworkAction->getError(), E_NOT_POSSIBLE);
}

/**
 * @brief : Test to verify the update function of source for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "update" function class when notification configuration details like type
 *         ,status,parameter are configured and push in the list of main notification configuration
 *         which further assign to source element notification configuration.
 *         & initialized the CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property, & also set the action parameter of notification
 *         configuration like type,status etc. & set the behavior of getSinkInfoDB & setSinkNotificationConfiguration,
 *         changeMainSinkNotificationConfigurationDB to return "E_OK"
 *         & initialize the CAmSourceSinkActionSetSoundProperty class & set the action parameter property type and value.
 *         then will check whether _execute function will get Main Notification Configurations details or not,
 *         & set the Source Notification Configuration details using setSourceNotificationConfiguration function or not
 *         & also registerObserver or not, with status as E_WAIT_FOR_CHILD_COMPLETION,
 *         & getError must return "E_NOT_POSSIBLE", without any Gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" & getError will return the "E_NOT_POSSIBLE", without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, UpdateSourceNegativeCase)
{
    // Assigning the values to NotificationConfiguration
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    source.name                               = "AnySource1";
    source.listMainNotificationConfigurations = listMainNotificationConfigurations;
    handle.handleType                         = H_SETSOURCENOTIFICATION;
    handle.handle                             = 40;
    am_Error_e result;
    int        result_state;
    if (NULL != mpCAmSourceElement)
    {
        sourceID = mpCAmSourceElement->getID();
    }

    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(sourceID, _)).WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceNotificationConfiguration(_, sourceID, setNotificationConfigurations(notificationConfigurations))).WillOnce(DoAll(SetArgReferee<0>(handle), Return(E_OK)));
    pFrameworkAction = new CAmSourceSinkActionSetNotificationConfiguration<CAmSourceElement >(mpCAmSourceElement);
    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_PERIODIC);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);
    EXPECT_EQ(pFrameworkAction->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle, E_NOT_POSSIBLE);
    EXPECT_EQ(pFrameworkAction->getError(), E_NOT_POSSIBLE);
}

/**
 * @brief : Test to verify the Undo function of sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "Undo" function class when two notification configuration details like type
 *         ,status,parameter are configured and push in the list of main notification configuration
 *         which further assign to two sink element notification configuration.
 *         & initialized the two CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property, & also set the action parameter of notification
 *         configuration like type,status etc. for both instance & set the undo flag to true
 *         & set the behavior for both sink of getSinkInfoDB & setSinkNotificationConfiguration,
 *         changeMainSinkNotificationConfigurationDB to return "E_OK"
 *         then make sure the _execute function to will return "E_OK" and then will check whether
 *         Undo function will set the main notification configuration using the setMainNotificationConfiguration or not,
 *         & it unregister the observer or not, without any Gmock error.
 *
 * @result : "Pass" when Undo function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, SinkUndoPositiveCase)
{
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    notificationConfigurationsNew.type      = NT_OVER_TEMPERATURE;
    notificationConfigurationsNew.status    = NS_PERIODIC;
    notificationConfigurationsNew.parameter = 20;
    notificationConfigurationsOld.type      = NT_OVER_TEMPERATURE;
    notificationConfigurationsOld.status    = NS_OFF;
    notificationConfigurationsOld.parameter = 25;

    sink.listMainNotificationConfigurations = listMainNotificationConfigurations;
    sinkInfo.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];
    sink1.name                                  = "AnySink2";
    sink1.listMainNotificationConfigurations    = listMainNotificationConfigurations;
    sink2.name                                  = "AnySink3";
    sink2.listMainNotificationConfigurations    = listMainNotificationConfigurations;
    sinkOut1.listMainNotificationConfigurations = listMainNotificationConfigurations;
//// output parameter sink id and handle type and handle value
    am_sinkID_t sinkID1 = 0, sinkID2 = 0;
    am_Handle_s handle1, handle2, handle3;
    handle1.handleType = H_SETSINKNOTIFICATION;
    handle1.handle     = 51;
    handle2.handleType = H_SETSINKNOTIFICATION;
    handle2.handle     = 52;
    handle3.handleType = H_SETSINKNOTIFICATION;
    handle3.handle     = 53;
    TCLAP::CmdLine *cmd(CAmCommandLineSingleton::instanciateOnce("The team of the AudioManager wishes you a nice day!", ' ', "7.5", true));
    CAmControllerPlugin *pPlugin = new CAmControllerPlugin();
    pSocketHandler = new CAmSocketHandler();
    mpTimerEvent   = CAmTimerEvent::getInstance();
    if (NULL == mpTimerEvent)
    {
        ASSERT_THAT(mpTimerEvent, IsNull()) << "CAmSourceSinkActionSetNotificationConfigurationTest class: Not able to create Timer object";
    }

    mpTimerEvent->setSocketHandle(pSocketHandler, pPlugin);

    // creating the new sink element
    mpMockControlReceiveInterface1 = new MockIAmControlReceive();
    mpMockControlReceiveInterface2 = new MockIAmControlReceive();

    mpCAmControlReceive1 = mpMockControlReceiveInterface1;
    mpCAmControlReceive2 = mpMockControlReceiveInterface2;

    EXPECT_CALL(*mpMockControlReceiveInterface1, enterSinkDB(IsSinkNamePresent(sinkInfo1), _)).WillOnce(DoAll(SetArgReferee < 1 > (22), Return(E_OK)));
    mpCAmSinkElement1 = CAmSinkFactory::createElement(sinkInfo1, mpCAmControlReceive1);
    ASSERT_THAT(mpCAmSinkElement1, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement1->getID());
    sinkID1 = mpCAmSinkElement1->getID();

    EXPECT_CALL(*mpMockControlReceiveInterface2, enterSinkDB(IsSinkNamePresent(sinkInfo2), _)).WillOnce(DoAll(SetArgReferee < 1 > (23), Return(E_OK)));
    mpCAmSinkElement2 = CAmSinkFactory::createElement(sinkInfo2, mpCAmControlReceive2);
    ASSERT_THAT(mpCAmSinkElement2, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement2->getID());
    sinkID2 = mpCAmSinkElement2->getID();

    if (NULL != mpCAmSinkElement1)
    {
        sinkID1 = mpCAmSinkElement1->getID();
    }

    if (NULL != mpCAmSinkElement2)
    {
        sinkID2 = mpCAmSinkElement2->getID();
    }

    pFrameworkAction1 = new CAmSourceSinkActionSetNotificationConfiguration<CAmSinkElement >(mpCAmSinkElement1);
    pFrameworkAction2 = new CAmSourceSinkActionSetNotificationConfiguration<CAmSinkElement >(mpCAmSinkElement2);

    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_OFF);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(22);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);

    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_OFF);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(23);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);
    pFrameworkAction1->setUndoRequried(true);
    pFrameworkAction2->setUndoRequried(true);
    pCAmActionContainer.append(pFrameworkAction1);
    pCAmActionContainer.append(pFrameworkAction2);
    pCAmActionContainer.setUndoRequried(true);
    EXPECT_CALL(*mpMockControlReceiveInterface1, getSinkInfoDB(sinkID1, _)).WillOnce(DoAll(SetArgReferee<1>(sinkOut1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface1, setSinkNotificationConfiguration(_, sinkID1, setNotificationConfigurations(notificationConfigurations))).WillOnce(DoAll(SetArgReferee<0>(handle1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface1, changeMainSinkNotificationConfigurationDB(sinkID1, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_OK));

    EXPECT_CALL(*mpMockControlReceiveInterface2, getSinkInfoDB(sinkID2, _)).WillOnce(DoAll(SetArgReferee<1>(sinkOut1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface2, setSinkNotificationConfiguration(_, sinkID2, setNotificationConfigurations(notificationConfigurationsNew))).WillOnce(DoAll(SetArgReferee<0>(handle2), Return(E_NOT_POSSIBLE)));

    EXPECT_EQ(pCAmActionContainer.execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle1, E_OK);
    EXPECT_EQ(pCAmActionContainer.execute(), E_OK);

    EXPECT_CALL(*mpMockControlReceiveInterface1, setSinkNotificationConfiguration(_, sinkID1, setNotificationConfigurations(notificationConfigurationsOld))).WillOnce(DoAll(SetArgReferee<0>(handle3), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface1, changeMainSinkNotificationConfigurationDB(sinkID1, setNotificationConfigurations(notificationConfigurationsOld))).WillOnce(Return(E_OK));
    EXPECT_EQ(pCAmActionContainer.undo(), 0);
    CAmHandleStore::instance().notifyAsyncResult(handle3, E_OK);
    CAmCommandLineSingleton::deleteInstance();

}

/**
 * @brief : Test to verify the timeout function of sink for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "Undo" function class when two notification configuration details like type
 *         ,status,parameter are configured and push in the list of main notification configuration
 *         which further assign to two sink element notification configuration.
 *         & initialized the two CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property,& CAmTimerEvent is initialized  & set the time out using
 *         the setTimeout function & also set the action parameter of notification
 *         configuration like type,status etc. for both instance & set the undo flag to true
 *         & set the behavior for both sink of getSinkInfoDB & setSinkNotificationConfiguration,
 *         changeMainSinkNotificationConfigurationDB to return "E_OK" &
 *         then make sure the _execute,undo function to will return "E_OK" and then will check whether
 *         timeout will happen or not, main thread stop or not, without any Gmock error.
 *
 * @result : "Pass" when Undo function return "E_OK" & main thread stop without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, SinkTimeoutPositiveCase)
{
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    notificationConfigurationsNew.type          = NT_OVER_TEMPERATURE;
    notificationConfigurationsNew.status        = NS_PERIODIC;
    notificationConfigurationsNew.parameter     = 20;
    notificationConfigurationsOld.type          = NT_OVER_TEMPERATURE;
    notificationConfigurationsOld.status        = NS_OFF;
    notificationConfigurationsOld.parameter     = 25;
    sink1.name                                  = "AnySink9";
    sink1.listMainNotificationConfigurations    = listMainNotificationConfigurations;
    sink2.name                                  = "AnySink10";
    sink2.listMainNotificationConfigurations    = listMainNotificationConfigurations;
    sinkOut1.listMainNotificationConfigurations = listMainNotificationConfigurations;
    am_sinkID_t sinkID1 = 0, sinkID2 = 0;
    am_Handle_s handle1, handle2, handle3;

    handle1.handleType = H_SETSINKNOTIFICATION;
    handle1.handle     = 51;
    handle2.handleType = H_SETSINKNOTIFICATION;
    handle2.handle     = 52;
    handle3.handleType = H_SETSINKNOTIFICATION;
    handle3.handle     = 53;
    // timer related
    CAmSocketHandler pSocketHandler;
    pthread_t        thread_id;
    CAmControllerPlugin *pPlugin;

    CAmTimerEvent *mpTimerEvent = CAmTimerEvent::getInstance();
    if (NULL == mpTimerEvent)
    {
        ASSERT_THAT(mpTimerEvent, IsNull()) << "CAmSourceSinkActionSetNotificationConfigurationTest class: Not able to create Timer object";
    }

    mpTimerEvent->setSocketHandle(&pSocketHandler, pPlugin);

    mpMockControlReceiveInterface1 = new MockIAmControlReceive();
    mpMockControlReceiveInterface2 = new MockIAmControlReceive();

    mpCAmControlReceive1 = mpMockControlReceiveInterface1;
    mpCAmControlReceive2 = mpMockControlReceiveInterface2;

    EXPECT_CALL(*mpMockControlReceiveInterface1, enterSinkDB(IsSinkNamePresent(sinkInfo1), _)).WillOnce(DoAll(SetArgReferee < 1 > (22), Return(E_OK)));
    mpCAmSinkElement1 = CAmSinkFactory::createElement(sinkInfo1, mpCAmControlReceive1);
    ASSERT_THAT(mpCAmSinkElement1, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement1->getID());
    sinkID1 = mpCAmSinkElement1->getID();

    EXPECT_CALL(*mpMockControlReceiveInterface2, enterSinkDB(IsSinkNamePresent(sinkInfo2), _)).WillOnce(DoAll(SetArgReferee < 1 > (23), Return(E_OK)));
    mpCAmSinkElement2 = CAmSinkFactory::createElement(sinkInfo2, mpCAmControlReceive2);
    ASSERT_THAT(mpCAmSinkElement2, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement2->getID());
    sinkID2 = mpCAmSinkElement2->getID();

    if (NULL != mpCAmSinkElement1)
    {
        sinkID1 = mpCAmSinkElement1->getID();
    }

    if (NULL != mpCAmSinkElement2)
    {
        sinkID2 = mpCAmSinkElement2->getID();
    }

    pFrameworkAction1 = new CAmSourceSinkActionSetNotificationConfiguration<CAmSinkElement >(mpCAmSinkElement1);
    pFrameworkAction2 = new CAmSourceSinkActionSetNotificationConfiguration<CAmSinkElement >(mpCAmSinkElement2);

    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_OFF);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);
    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_OFF);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);

    pFrameworkAction2->setTimeout(3000);
    pFrameworkAction1->setUndoRequried(true);
    pFrameworkAction2->setUndoRequried(true);
    pCAmActionContainer.append(pFrameworkAction1);
    pCAmActionContainer.append(pFrameworkAction2);
    pCAmActionContainer.setUndoRequried(true);

    EXPECT_CALL(*mpMockControlReceiveInterface1, getSinkInfoDB(sinkID1, _)).WillOnce(DoAll(SetArgReferee<1>(sinkOut1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface1, setSinkNotificationConfiguration(_, sinkID1, setNotificationConfigurations(notificationConfigurations))).WillOnce(DoAll(SetArgReferee<0>(handle1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface1, changeMainSinkNotificationConfigurationDB(sinkID1, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_OK));
    EXPECT_EQ(pCAmActionContainer.execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle1, E_OK);
    EXPECT_CALL(*mpMockControlReceiveInterface2, getSinkInfoDB(sinkID2, _)).WillOnce(DoAll(SetArgReferee<1>(sinkOut1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface2, setSinkNotificationConfiguration(_, sinkID2, setNotificationConfigurations(notificationConfigurationsNew))).WillOnce(DoAll(SetArgReferee<0>(handle2), Return(E_OK)));
    EXPECT_EQ(pCAmActionContainer.execute(), E_OK);

    pthread_create(&thread_id, NULL, WorkerThread, &pSocketHandler);
    EXPECT_CALL(*mpMockControlReceiveInterface2, abortAction(IsSourceHandleOk(handle2))).WillOnce((Return(E_OK)));
    sleep(5);
    EXPECT_CALL(*mpMockControlReceiveInterface1, setSinkNotificationConfiguration(_, sinkID1, setNotificationConfigurations(notificationConfigurationsOld))).WillOnce(DoAll(SetArgReferee<0>(handle3), Return(E_OK)));
    EXPECT_EQ(pCAmActionContainer.undo(), 0);
    pSocketHandler.exit_mainloop();
    pthread_join(thread_id, NULL);
}

/**
 * @brief : Test to verify the timeout function of source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "Undo" function class when two notification configuration details like type
 *         ,status,parameter are configured and push in the list of main notification configuration
 *         which further assign to two source element notification configuration.
 *         & initialized the two CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property,& CAmTimerEvent is initialized  & set the time out using
 *         the setTimeout function & also set the action parameter of notification
 *         configuration like type,status etc. for both instance & set the undo flag to true
 *         & set the behavior for both source of getDourceInfoDB & setSourceNotificationConfiguration,
 *         changeMainSourceNotificationConfigurationDB to return "E_OK" &
 *         then make sure the _execute,undo function to will return "E_OK" and then will check whether
 *         timeout will happen or not, main thread stop or not, without any Gmock error.
 *
 * @result : "Pass" when Undo function return "E_OK" & main thread stop without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, SourceTimeoutPositiveCase)
{
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    listMainNotificationConfigurations.push_back(notificationConfigurations);

    notificationConfigurationsNew.type      = NT_OVER_TEMPERATURE;
    notificationConfigurationsNew.status    = NS_PERIODIC;
    notificationConfigurationsNew.parameter = 20;

    notificationConfigurationsOld.type         = NT_OVER_TEMPERATURE;
    notificationConfigurationsOld.status       = NS_OFF;
    notificationConfigurationsOld.parameter    = 25;
    source1.name                               = "AnySource2";
    source1.listMainNotificationConfigurations = listMainNotificationConfigurations;
    source2.name                               = "AnySource3";
    source2.listMainNotificationConfigurations = listMainNotificationConfigurations;
    am_Source_s sourceOut1;
    sourceOut1.listMainNotificationConfigurations = listMainNotificationConfigurations;
    // output parameter sink id and handle type and handle value
    am_sourceID_t sourceID1 = 0, sourceID2 = 0;
    am_Handle_s   handle1, handle2, handle3;

    handle1.handleType = H_SETSINKNOTIFICATION;
    handle1.handle     = 51;
    handle2.handleType = H_SETSINKNOTIFICATION;
    handle2.handle     = 52;
    handle3.handleType = H_SETSINKNOTIFICATION;
    handle3.handle     = 53;
    // timer related
    CAmSocketHandler pSocketHandler;
    pthread_t        thread_id;
    CAmControllerPlugin *pPlugin;

    CAmTimerEvent *mpTimerEvent = CAmTimerEvent::getInstance();
    if (NULL == mpTimerEvent)
    {
        ASSERT_THAT(mpTimerEvent, IsNull()) << "CAmSourceSinkActionSetNotificationConfigurationTest class: Not able to create Timer object";
    }

    mpTimerEvent->setSocketHandle(&pSocketHandler, pPlugin);

    mpMockControlReceiveInterface1 = new MockIAmControlReceive();
    mpMockControlReceiveInterface2 = new MockIAmControlReceive();

    mpCAmControlReceive1 = mpMockControlReceiveInterface1;
    mpCAmControlReceive2 = mpMockControlReceiveInterface2;

    EXPECT_CALL(*mpMockControlReceiveInterface1,
        enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee < 1 > (23), Return(E_OK)));
    mpCAmSourceElement1 = CAmSourceFactory::createElement(sourceInfo1,
            mpCAmControlReceive1);
    ASSERT_THAT(mpCAmSourceElement1, NotNull())
        << " Source Element1 Is not Created";
    sourceIDList.push_back(mpCAmSourceElement1->getID());
    sourceID1 = mpCAmSourceElement1->getID();

    EXPECT_CALL(*mpMockControlReceiveInterface2,
        enterSourceDB(IsSourceNamePresent(sourceInfo2), _)).WillOnce(
        DoAll(SetArgReferee < 1 > (24), Return(E_OK)));
    mpCAmSourceElement2 = CAmSourceFactory::createElement(sourceInfo2,
            mpCAmControlReceive2);
    ASSERT_THAT(mpCAmSourceElement2, NotNull())
        << " Source Element Is not Created";
    sourceIDList.push_back(mpCAmSourceElement2->getID());
    sourceID2 = mpCAmSourceElement2->getID();

    if (NULL != mpCAmSourceElement1)
    {
        sourceID1 = mpCAmSourceElement1->getID();
    }

    if (NULL != mpCAmSourceElement2)
    {
        sourceID2 = mpCAmSourceElement2->getID();
    }

    pFrameworkAction1 = new CAmSourceSinkActionSetNotificationConfiguration<CAmSourceElement >(mpCAmSourceElement1);
    pFrameworkAction2 = new CAmSourceSinkActionSetNotificationConfiguration<CAmSourceElement >(mpCAmSourceElement2);

    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_OFF);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);

    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_OFF);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);

    pFrameworkAction2->setTimeout(3000);
    pFrameworkAction1->setUndoRequried(true);
    pFrameworkAction2->setUndoRequried(true);
    pCAmActionContainer.append(pFrameworkAction1);
    pCAmActionContainer.append(pFrameworkAction2);
    pCAmActionContainer.setUndoRequried(true);

    EXPECT_CALL(*mpMockControlReceiveInterface1, getSourceInfoDB(sourceID1, _)).WillOnce(DoAll(SetArgReferee<1>(sourceOut1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface1, setSourceNotificationConfiguration(_, sourceID1, setNotificationConfigurations(notificationConfigurations))).WillOnce(DoAll(SetArgReferee<0>(handle1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface1, changeMainSourceNotificationConfigurationDB(sourceID1, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_OK));

    EXPECT_EQ(pCAmActionContainer.execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle1, E_OK);

    EXPECT_CALL(*mpMockControlReceiveInterface2, getSourceInfoDB(sourceID2, _)).WillOnce(DoAll(SetArgReferee<1>(sourceOut1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface2, setSourceNotificationConfiguration(_, sourceID2, setNotificationConfigurations(notificationConfigurationsNew))).WillOnce(DoAll(SetArgReferee<0>(handle2), Return(E_OK)));
    EXPECT_EQ(pCAmActionContainer.execute(), E_OK);
    pthread_create(&thread_id, NULL, WorkerThread, &pSocketHandler);
    EXPECT_CALL(*mpMockControlReceiveInterface2, abortAction(IsSourceHandleOk(handle2))).WillOnce((Return(E_OK)));
    sleep(5);
    EXPECT_CALL(*mpMockControlReceiveInterface1, setSourceNotificationConfiguration(_, sourceID1, setNotificationConfigurations(notificationConfigurationsOld))).WillOnce(DoAll(SetArgReferee<0>(handle3), Return(E_OK)));
    EXPECT_EQ(pCAmActionContainer.undo(), 0);
    pSocketHandler.exit_mainloop();
    pthread_join(thread_id, NULL);
}

/**
 * @brief : Test to verify the Undo function of source for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink,Source Element is created with required input structure parameters like name,domainID,
 *        class name, registration type,volume etc, sink,source element ID is pushed in the list.
 *
 * @test : verify the "Undo" function class when two notification configuration details like type
 *         ,status,parameter are configured and push in the list of main notification configuration
 *         which further assign to two source element notification configuration.
 *         & initialized the two CAmSourceSinkActionSetNotificationConfiguration class sound property details
 *         are set and push in the list of sound property, & also set the action parameter of notification
 *         configuration like type,status etc. for both instance & set the undo flag to true
 *         & set the behavior for both source of getsourceInfoDB & setsourceNotificationConfiguration,
 *         changeMainsourceNotificationConfigurationDB to return "E_OK"
 *         then make sure the _execute function to will return "E_OK" and then will check whether
 *         Undo function will set the main notification configuration using the setMainNotificationConfiguration or not,
 *         & it unregister the observer or not for source element, without any Gmock error.
 *
 * @result : "Pass" when Undo function return "E_OK" without any Gmock error message
 */
TEST_F(CAmSourceSinkActionSetNotificationConfigurationTest, SourceUndoPositiveCase)
{
    notificationConfigurations.type      = NT_OVER_TEMPERATURE;
    notificationConfigurations.status    = NS_PERIODIC;
    notificationConfigurations.parameter = 20;
    listMainNotificationConfigurations.push_back(notificationConfigurations);
    notificationConfigurationsNew.type            = NT_OVER_TEMPERATURE;
    notificationConfigurationsNew.status          = NS_PERIODIC;
    notificationConfigurationsNew.parameter       = 20;
    notificationConfigurationsOld.type            = NT_OVER_TEMPERATURE;
    notificationConfigurationsOld.status          = NS_OFF;
    notificationConfigurationsOld.parameter       = 25;
    source1.name                                  = "AnySource2";
    source1.listMainNotificationConfigurations    = listMainNotificationConfigurations;
    source2.name                                  = "AnySource3";
    source2.listMainNotificationConfigurations    = listMainNotificationConfigurations;
    sourceOut1.listMainNotificationConfigurations = listMainNotificationConfigurations;
    am_sourceID_t sourceID1 = 0, sourceID2 = 0;
    am_Handle_s   handle1, handle2, handle3;

    handle1.handleType = H_SETSINKNOTIFICATION;
    handle1.handle     = 51;
    handle2.handleType = H_SETSINKNOTIFICATION;
    handle2.handle     = 52;
    handle3.handleType = H_SETSINKNOTIFICATION;
    handle3.handle     = 53;
    TCLAP::CmdLine *cmd(CAmCommandLineSingleton::instanciateOnce("The team of the AudioManager wishes you a nice day!", ' ', "7.5", true));
    CAmControllerPlugin *pPlugin = new CAmControllerPlugin();
    pSocketHandler = new CAmSocketHandler();
    mpTimerEvent1  = CAmTimerEvent::getInstance();
    if (NULL == mpTimerEvent1)
    {
        ASSERT_THAT(mpTimerEvent, IsNull()) << "CAmSourceSinkActionSetNotificationConfigurationTest class: Not able to create Timer object";
    }

    mpTimerEvent1->setSocketHandle(pSocketHandler, pPlugin);
    mpMockControlReceiveInterface1 = new MockIAmControlReceive();
    mpMockControlReceiveInterface2 = new MockIAmControlReceive();
    mpCAmControlReceive1           = mpMockControlReceiveInterface1;
    mpCAmControlReceive2           = mpMockControlReceiveInterface2;

    EXPECT_CALL(*mpMockControlReceiveInterface1,
        enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee < 1 > (23), Return(E_OK)));
    mpCAmSourceElement1 = CAmSourceFactory::createElement(sourceInfo1,
            mpCAmControlReceive1);
    ASSERT_THAT(mpCAmSourceElement1, NotNull())
        << " Source Element1 Is not Created";
    sourceIDList.push_back(mpCAmSourceElement1->getID());
    sourceID1 = mpCAmSourceElement1->getID();

    EXPECT_CALL(*mpMockControlReceiveInterface2,
        enterSourceDB(IsSourceNamePresent(sourceInfo2), _)).WillOnce(
        DoAll(SetArgReferee < 1 > (24), Return(E_OK)));
    mpCAmSourceElement2 = CAmSourceFactory::createElement(sourceInfo2,
            mpCAmControlReceive2);
    ASSERT_THAT(mpCAmSourceElement2, NotNull())
        << " Source Element Is not Created";
    sourceIDList.push_back(mpCAmSourceElement2->getID());
    sourceID2 = mpCAmSourceElement2->getID();

    if (NULL != mpCAmSourceElement1)
    {
        sourceID1 = mpCAmSourceElement1->getID();
    }

    if (NULL != mpCAmSourceElement2)
    {
        sourceID2 = mpCAmSourceElement2->getID();
    }

    pFrameworkAction1 = new CAmSourceSinkActionSetNotificationConfiguration<CAmSourceElement >(mpCAmSourceElement1);
    pFrameworkAction2 = new CAmSourceSinkActionSetNotificationConfiguration<CAmSourceElement >(mpCAmSourceElement2);

    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_OFF);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction1->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);

    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, &mNotificationType);
    mNotificationStatus.setParam(NS_OFF);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, &mNotificationStatus);
    mNotificationparam.setParam(25);
    pFrameworkAction2->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, &mNotificationparam);

    pFrameworkAction1->setUndoRequried(true);
    pFrameworkAction2->setUndoRequried(true);
    pCAmActionContainer.append(pFrameworkAction1);
    pCAmActionContainer.append(pFrameworkAction2);
    pCAmActionContainer.setUndoRequried(true);

    EXPECT_CALL(*mpMockControlReceiveInterface1, getSourceInfoDB(sourceID1, _)).WillOnce(DoAll(SetArgReferee<1>(sourceOut1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface1, setSourceNotificationConfiguration(_, sourceID1, setNotificationConfigurations(notificationConfigurations))).WillOnce(DoAll(SetArgReferee<0>(handle1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface1, changeMainSourceNotificationConfigurationDB(sourceID1, setNotificationConfigurations(notificationConfigurations))).WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface2, getSourceInfoDB(sourceID2, _)).WillOnce(DoAll(SetArgReferee<1>(sourceOut1), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface2, setSourceNotificationConfiguration(_, sourceID2, setNotificationConfigurations(notificationConfigurationsNew))).WillOnce(DoAll(SetArgReferee<0>(handle2), Return(E_NOT_POSSIBLE)));

    EXPECT_EQ(pCAmActionContainer.execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(handle1, E_OK);
    EXPECT_EQ(pCAmActionContainer.execute(), E_OK);
    EXPECT_CALL(*mpMockControlReceiveInterface1, setSourceNotificationConfiguration(_, sourceID1, setNotificationConfigurations(notificationConfigurationsOld))).WillOnce(DoAll(SetArgReferee<0>(handle3), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface1, changeMainSourceNotificationConfigurationDB(sourceID1, setNotificationConfigurations(notificationConfigurationsOld))).WillOnce(Return(E_OK));

    EXPECT_EQ(pCAmActionContainer.undo(), 0);
    CAmHandleStore::instance().notifyAsyncResult(handle3, E_OK);
    CAmCommandLineSingleton::deleteInstance();
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
