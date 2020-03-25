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

#include <map>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmElement.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmClassElement.h"
#include "CAmSystemElement.h"
#include "CAmPolicyAction.h"
#include "CAmActionConnect.h"
#include "CAmMainConnectionElement.h"
#include "CAmSystemActionSetProperty.h"
#include "CAmSourceSinkActionSetSoundProperty.h"
#include "CAmSourceSinkActionSetNotificationConfiguration.h"
#include "IAmPolicySend.h"
#include "MockIAmControlReceive.h"
#include "MockIAmPolicySend.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmPolicyActionTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmPolicyActionTest
 *@brief : This class is used to test the CAmPolicyAction class functionality.
 */
CAmPolicyActionTest::CAmPolicyActionTest()
    : mpMockControlReceiveInterface(NULL)
    , mpPolicySendInterface(NULL)
    , mpControlReceive(NULL)
    , mpSinkElement(nullptr)
    , mpSourceElement(nullptr)
    , mpClassElement(nullptr)
    , mpSystemElement(nullptr)
    , mpClassActionConnect(NULL)
    , mpPolicyAction(NULL)
#ifdef VOLUME_RELATED_TEST
    , mpClassActionSetVolume(NULL)
#endif
    , mpySystemActionSetProperty(NULL)
    , mpActionSinkSoundProperty(NULL)
    , mpActionSourceNotification(NULL)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
{
}

CAmPolicyActionTest::~CAmPolicyActionTest()
{
}

void CAmPolicyActionTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpControlReceive              = mpMockControlReceiveInterface;

    // This will check whether controller is initialized or not
    ASSERT_THAT(mpControlReceive, NotNull()) << " Controller Not Initialized";

    mpPolicySendInterface = new MockIAmPolicySend();
    // This will check whether PolicySendInterface is initialized or not
    ASSERT_THAT(mpPolicySendInterface, NotNull()) << " PolicySend InterFace Not Initialized";

    am_ClassProperty_s classPropertyData;
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    am_SinkClass_s sinkClass;
    sinkClass.sinkClassID = 73;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    am_SourceClass_s sourceClass;
    sourceClass.sourceClassID = 74;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    gc_Class_s classConnection;
    classConnection.name     = "AnyClass1";
    classConnection.type     = C_CAPTURE;
    classConnection.priority = 1;
    // SystemProperty creating
    mSystemproperty.type     = SYP_GLOBAL_LOG_THRESHOLD;
    mSystemproperty.value    = 5;
    systemConfiguration.name = "system";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(mSystemproperty))).WillOnce(Return(E_OK));
    mpSystemElement          = CAmSystemFactory::createElement(systemConfiguration, mpControlReceive);
    ASSERT_THAT(mpSystemElement, NotNull()) << " System Element Is not Created";
    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(
        DoAll(SetArgReferee<1 >(73), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(
        DoAll(SetArgReferee<0 >(74), Return(E_OK)));
    mpClassElement = CAmClassFactory::createElement(classConnection, mpControlReceive);

    ASSERT_THAT(mpClassElement, NotNull()) << " Class Element Is not Created";
}

void CAmPolicyActionTest::TearDown()
{
    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();
    CAmClassFactory::destroyElement();
    CAmSystemFactory::destroyElement();

    mpSystemElement = nullptr;
//
    if (mpPolicyAction != NULL)
    {
        delete (mpPolicyAction);
    }

//
// if (mpClassActionSetVolume != NULL)
// {
// delete (mpClassActionSetVolume);
// }
//
// if (mpClassActionConnect != NULL)
// {
////   delete (mpClassActionConnect);
// }
//
// if (mpPolicySendInterface != NULL)
// {
// delete (mpPolicySendInterface);
// }

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

void CAmPolicyActionTest::CreateSourceSink()
{
    // Initialize Sink Element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSinkElement = CAmSinkFactory::createElement(sinkInfo, mpControlReceive);

    ASSERT_THAT(mpSinkElement, NotNull()) << " Sink Element Is not Created";

    sinkIDList.push_back(mpSinkElement->getID());

    // Initialize Source Element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSourceElement = CAmSourceFactory::createElement(sourceInfo, mpControlReceive);

    ASSERT_THAT(mpSourceElement, NotNull()) << " Source Element Is not Created";

    sourceIDList.push_back(mpSourceElement->getID());

    mpClassElement->attach(mpSourceElement);
    mpClassElement->attach(mpSinkElement);

}

void CAmPolicyActionTest::CreateSink()
{
    std::vector<am_SoundProperty_s > soundPropertyList;
    mSoundProperty.type  = MSP_GENIVI_TREBLE;
    mSoundProperty.value = 23;
    soundPropertyList.push_back(mSoundProperty);

    // Initialize Sink Element
    sinkInfo.mapMSPTOSP[MD_MSP_TO_SP][MSP_GENIVI_TREBLE];

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSinkElement = CAmSinkFactory::createElement(sinkInfo, mpControlReceive);

    ASSERT_THAT(mpSinkElement, NotNull()) << " Sink Element Is not Created";

    mpClassElement->attach(mpSinkElement);
}

void CAmPolicyActionTest::CreateSource()
{
    // Assigning the values to NotificationConfiguration
    mNotificationConfigurations.type      = NT_OVER_TEMPERATURE;
    mNotificationConfigurations.status    = NS_PERIODIC;
    mNotificationConfigurations.parameter = 20;
    // Add above values to MainNotificationConfiguration Vector
    mListMainNotificationConfigurations.push_back(mNotificationConfigurations);

    // Initialize Source Element
    ut_gc_Source_s sourceInfo("AnySource1", mListMainNotificationConfigurations);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSourceElement = CAmSourceFactory::createElement(sourceInfo, mpControlReceive);

    ASSERT_THAT(mpSourceElement, NotNull()) << " Source Element Is not Created";

    sourceIDList.push_back(mpSourceElement->getID());
    mpClassElement->attach(mpSourceElement);
}

/**
 * @brief : Test to verify the execute functions for ConnectAction for Positive scenario
 *
 * @pre : Mock the IAmControlReceive & IAmPolicySend class and  initialized the CAmControlReceive,CAmMainConnectionActionConnect classes,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db for behavior set.
 *        and system element is also created with details of system configuration and mock class details of IAmControlReceive.
 *        and class element is also created and attach the sink,source element to class element using attach function call.
 *
 * @test : verify the "_execute" function when CAmClassActionConnect class is initialize with class element,
 *         & set the action parameter details of source,sink,class,order,time of CAmClassActionConnect.
 *         & push the action information like action name as "ACTION_CONNECT" etc in the policy action list
 *         & create main connection element and set the behavior of enterMainConnectionDB,getMainConnectionInfoDB function
 *         to return the "E_OK", then will check whether _execute function will execute the action or not
 *         it actually, For each policy action, it get the list of framework actions and appends in the list of child action
 *         or not,without any gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmPolicyActionTest, executePositiveConnectAction)
{
    CreateSourceSink();
//
    mSourceNameParam.setParam(mpSourceElement->getName());
    mSinkNameParam.setParam(mpSinkElement->getName());
    mClassNameParam.setParam(mpClassElement->getName());
    mOrderParam.setParam(O_LOW_PRIORITY);
    mConnectionFormatParam.setParam(CF_GENIVI_STEREO);
    mTimeoutvalue.setParam(5000);

    mpClassActionConnect = new CAmActionConnect();

    ASSERT_THAT(mpClassActionConnect, NotNull()) << " Class Action Connect Is not Created";
// setting Parameter
    mpClassActionConnect->setParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    mpClassActionConnect->setParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    mpClassActionConnect->setParam(ACTION_PARAM_CLASS_NAME, &mClassNameParam);
    mpClassActionConnect->setParam(ACTION_PARAM_ORDER, &mOrderParam);
    mpClassActionConnect->setParam(ACTION_PARAM_CONNECTION_FORMAT, &mConnectionFormatParam);
    mpClassActionConnect->setParam(ACTION_PARAM_TIMEOUT, &mTimeoutvalue);

// Action info;
    mActions.actionType = ACTION_CONNECT;
// mActions.mapParameters.insert(
// std::pair<string, string >(ACTION_PARAM_SOURCE_NAME,
// mpSourceElement->getName()));
// mActions.mapParameters.insert(
// std::pair<string, string >(ACTION_PARAM_SINK_NAME, mpSinkElement->getName()));
// mActions.mapParameters.insert(
// std::pair<string, string >(ACTION_PARAM_CLASS_NAME, mpClassElement->getName()));
//
// mListPolicyActions.push_back(mActions);

/*
 * Policy Action Instance is being created in each Test Because the Action needs to verified
 *  is different
 */
    mpPolicyAction = new CAmPolicyAction(mListPolicyActions, mpPolicySendInterface,
            mpControlReceive);

    EXPECT_EQ(E_OK, mpPolicyAction->execute());

}
#ifdef VOLUME_RELATED_TEST
/**
 * @brief : Test to verify the execute functions for SetVolumeAction for Positive scenario
 *
 * @pre : Mock the IAmControlReceive & IAmPolicySend class and  initialized the CAmControlReceive,CAmMainConnectionActionConnect classes,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db for behavior set.
 *        and system element is also created with details of system configuration and mock class details of IAmControlReceive.
 *        and class element is also created and attach the sink,source element to class element using attach function call.
 *
 * @test : verify the "_execute" function when CAmClassActionSetVolume class is initialize with class element,
 *         & set the action parameter details of source,sink,class,main volume parameter of CAmClassActionSetVolume.
 *         & push the action information like action name as "ACTION_SET_VOLUME" etc in the policy action list
 *         & set the RampTypeParam as "RAMP_GENIVI_DIRECT" & set the behavior of setSinkVolume function
 *         to return the "E_OK" with parameter like LimitVolumeParam ,RampTypeParam etc,
 *         then will check whether _execute function will execute the action or not
 *         it actually, For each policy action, it get the list of framework actions and appends in the list of child action
 *         or not,without any gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmPolicyActionTest, executePositiveSetVolumeAction)
{
    CreateSourceSink();

    mSinkNameParam.setParam(mpSinkElement->getName());
    mClassNameParam.setParam(mpClassElement->getName());
    mMainVolumeParam.setParam(25);
    // setting Parameter
    mpClassActionSetVolume = new CAmClassActionSetVolume(mpClassElement);
    ASSERT_THAT(mpClassActionSetVolume, NotNull()) << " Class Action SetVolume Is not Created";

    mpClassActionSetVolume->setParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    mpClassActionSetVolume->setParam(ACTION_PARAM_CLASS_NAME, &mClassNameParam);
    mpClassActionSetVolume->setParam(ACTION_PARAM_MAIN_VOLUME, &mMainVolumeParam);

    mActions.actionType = ACTION_SET_VOLUME;

    mActions.mapParameters.insert(
        std::pair<string, string >(ACTION_PARAM_SINK_NAME, mpSinkElement->getName()));
    mActions.mapParameters.insert(
        std::pair<string, string >(ACTION_PARAM_CLASS_NAME, mpClassElement->getName()));
    mActions.mapParameters.insert(std::pair<string, string >(ACTION_PARAM_MAIN_VOLUME, "25"));

    mListPolicyActions.push_back(mActions);

    /*
     * Policy Action Instance is being created in each Test Because the Action needs to verified
     *  is different
     */
    mpPolicyAction = new CAmPolicyAction(mListPolicyActions, mpPolicySendInterface,
            mpControlReceive);
    ASSERT_THAT(mpPolicyAction, NotNull()) << " Policy Action Connect Is not Created";

    mHandle = { H_SETSINKVOLUME, 40 };

    am_volume_t         LimitVolumeParam = -120;
    am_CustomRampType_t RampTypeParam    = RAMP_GENIVI_DIRECT;
    am_time_t           RampTimeParam    = 200;

    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkVolume(_, mpSinkElement->getID(), LimitVolumeParam, RampTypeParam, RampTimeParam)).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPolicyAction->execute());
}
#endif
/**
 * @brief : Test to verify the execute functions for SystemPropertyction for Positive scenario
 *
 * @pre : Mock the IAmControlReceive & IAmPolicySend class and  initialized the CAmControlReceive,CAmMainConnectionActionConnect classes,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db for behavior set.
 *        and system element is also created with details of system configuration and mock class details of IAmControlReceive.
 *        and class element is also created and attach the sink,source element to class element using attach function call.
 *
 * @test : verify the "_execute" function when CAmSystemActionSetProperty class is initialize with system element,
 *         & set the action parameter details of debug type debug value of CAmSystemActionSetProperty class  .
 *         & push the action information like action name as "ACTION_DEBUG" etc in the policy action list
 *         then will check whether _execute function will execute the action or not
 *         it actually, For each policy action, it get the list of framework actions and appends in the list of child action
 *         or not,without any gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmPolicyActionTest, executePositiveSystemPropertyction)
{
    CreateSourceSink();
    mpySystemActionSetProperty = new CAmSystemActionSetProperty(mpSystemElement);
    mTypeParam.setParam(SYP_DEBUG);
    mValueParam.setParam(10);

// setting paramter
    mpySystemActionSetProperty->setParam(ACTION_PARAM_DEBUG_TYPE, &mTypeParam);
    mpySystemActionSetProperty->setParam(ACTION_PARAM_DEBUG_VALUE, &mValueParam);

// Action info
    mActions.actionType = ACTION_DEBUG;
    mActions.mapParameters.insert(std::pair<string, string >(ACTION_PARAM_DEBUG_TYPE, "SYP_DEBUG"));
    mActions.mapParameters.insert(std::pair<string, string >(ACTION_PARAM_DEBUG_VALUE, "10"));
/*
 * Policy Action Instance is being created in each Test Because the Action needs to verified
 *  is different
 */
    mpPolicyAction = new CAmPolicyAction(mListPolicyActions, mpPolicySendInterface,
            mpControlReceive);
    ASSERT_THAT(mpPolicyAction, NotNull()) << " Policy Action Connect Is not Created";
    EXPECT_EQ(E_OK, mpPolicyAction->execute());

}

/**
 * @brief : Test to verify the execute functions for SetSoundPropertyAction for Positive scenario
 *
 * @pre : Mock the IAmControlReceive & IAmPolicySend class and  initialized the CAmControlReceive,CAmMainConnectionActionConnect classes,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db for behavior set.
 *        and system element is also created with details of system configuration and mock class details of IAmControlReceive.
 *        and class element is also created and attach the sink,source element to class element using attach function call.
 *
 * @test : verify the "_execute" function when CAmSourceSinkActionSetSoundProperty class is initialize with sink element,
 *         & set the action parameter details of sink,class,property type, property value of CAmSourceSinkActionSetSoundProperty.
 *         & push the action information like action name as "ACTION_SET_PROPERTY" etc in the policy action list
 *         & set the behavior of getMainSinkSoundPropertyValue,setSinkSoundProperty function
 *         to return the "E_OK" with parameter like sink id ,property Type etc,
 *         then will check whether _execute function will execute the action or not
 *         it actually, For each policy action, it get the list of framework actions and appends in the list of child action
 *         or not,without any gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmPolicyActionTest, executePositiveSetSoundPropertyAction)
{
    CreateSink();

    am_CustomMainSoundPropertyType_t propertyType = MSP_GENIVI_TREBLE;
    int16_t                          value;
    // handle for setsinkproperty
    am_Handle_s handle;
    handle.handleType = H_SETSINKSOUNDPROPERTY;
    handle.handle     = 50;

    mSinkNameParam.setParam(mpSinkElement->getName());
    mClassNameParam.setParam(mpClassElement->getName());
    mPropertyTypeParam.setParam(MSP_GENIVI_TREBLE);
    mPropertyValueParam.setParam(2);
    // setting Parameter
    mpActionSinkSoundProperty = new CAmSourceSinkActionSetSoundProperty<CAmSinkElement >(
            mpSinkElement);
    ASSERT_THAT(mpActionSinkSoundProperty, NotNull()) << " SinkSoundProperty Action Is not Created";

    mpActionSinkSoundProperty->setParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    mpActionSinkSoundProperty->setParam(ACTION_PARAM_CLASS_NAME, &mClassNameParam);
    mpActionSinkSoundProperty->setParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);
    mpActionSinkSoundProperty->setParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);

    // Action info
    mActions.actionType = ACTION_SET_PROPERTY;
    mActions.mapParameters.insert(
        std::pair<string, string >(ACTION_PARAM_SINK_NAME, mpSinkElement->getName()));
    mActions.mapParameters.insert(
        std::pair<string, string >(ACTION_PARAM_CLASS_NAME, mpClassElement->getName()));
    mActions.mapParameters.insert(std::pair<string, string >(ACTION_PARAM_PROPERTY_TYPE, "1"));
    mActions.mapParameters.insert(std::pair<string, string >(ACTION_PARAM_PROPERTY_VALUE, "2"));

    mListPolicyActions.push_back(mActions);

    /*
     * Policy Action Instance is being created in each Test Because the Action needs to verified
     *  is different
     */
    mpPolicyAction = new CAmPolicyAction(mListPolicyActions, mpPolicySendInterface,
            mpControlReceive);
    ASSERT_THAT(mpPolicyAction, NotNull()) << " Policy Action Connect Is not Created";

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainSinkSoundPropertyValue(mpSinkElement->getID(), propertyType, _)).WillOnce(
        DoAll(SetArgReferee<2 >(4), Return(E_OK)));

    mHandle = { H_SETSINKSOUNDPROPERTY, 50 };
    EXPECT_CALL(*mpMockControlReceiveInterface, setSinkSoundProperty(_, mpSinkElement->getID(), setSoundProperty(mSoundProperty))).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPolicyAction->execute());
}

/**
 * @brief : Test to verify the execute functions for SetNotificationConfigurationAction for Positive scenario
 *
 * @pre : Mock the IAmControlReceive & IAmPolicySend class and  initialized the CAmControlReceive,CAmMainConnectionActionConnect classes,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db for behavior set.
 *        and system element is also created with details of system configuration and mock class details of IAmControlReceive.
 *        and class element is also created and attach the sink,source element to class element using attach function call.
 *
 * @test : verify the "_execute" function when CAmSourceSinkActionSetNotificationConfiguration class is initialize with source element,
 *         & set the action parameter details of source,sink,class,notification configuration type,status
 *         of CAmSourceSinkActionSetNotificationConfiguration.
 *         & push the action information like action name as "ACTION_SET_NOTIFICATION_CONFIGURATION" etc in the policy action list
 *         & set the behavior of getSourceInfoDB,setSourceNotificationConfiguration function
 *         to return the "E_OK" with parameter like sink id ,property Type etc,
 *         then will check whether _execute function will execute the action or not
 *         it actually, For each policy action, it get the list of framework actions and appends in the list of child action
 *         or not,without any gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmPolicyActionTest, executePositiveSetNotificationConfigurationAction)
{
    CreateSource();

    am_Source_s sourcedata;
    sourcedata.listMainNotificationConfigurations = mListMainNotificationConfigurations;
    am_Handle_s handle;
    handle.handleType = H_SETSOURCENOTIFICATION;
    handle.handle     = 40;

    mSourceNameParam.setParam(mpSourceElement->getName());
    mClassNameParam.setParam(mpClassElement->getName());
    mNotificationType.setParam(NT_OVER_TEMPERATURE);
    mNotificationStatus.setParam(NS_PERIODIC);
    mNotificationparam.setParam(25);
    // setting Parameter
    mpActionSourceNotification = new CAmSourceSinkActionSetNotificationConfiguration<
            CAmSourceElement >(mpSourceElement);
    ASSERT_THAT(mpActionSourceNotification, NotNull()) << " SourceNotification Action Is not Created";

    mpActionSourceNotification->setParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    mpActionSourceNotification->setParam(ACTION_PARAM_CLASS_NAME, &mClassNameParam);
    mpActionSourceNotification->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE,
        &mNotificationType);
    mpActionSourceNotification->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS,
        &mNotificationStatus);
    mpActionSourceNotification->setParam(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM,
        &mNotificationparam);

    // Action info
    mActions.actionType = ACTION_SET_NOTIFICATION_CONFIGURATION;
    mActions.mapParameters.insert(
        std::pair<string, string >(ACTION_PARAM_SOURCE_NAME,
            mpSourceElement->getName()));
    mActions.mapParameters.insert(
        std::pair<string, string >(ACTION_PARAM_CLASS_NAME, mpClassElement->getName()));
    mActions.mapParameters.insert(
        std::pair<string, string >(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, "5"));
    mActions.mapParameters.insert(
        std::pair<string, string >(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS,
            "2"));
    mActions.mapParameters.insert(
        std::pair<string, string >(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM,
            "20"));

    mListPolicyActions.push_back(mActions);

    /*
     * Policy Action Instance is being created in each Test Because the Action needs to verified
     *  is different
     */
    mpPolicyAction = new CAmPolicyAction(mListPolicyActions, mpPolicySendInterface,
            mpControlReceive);

    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(mpSourceElement->getID(), _)).WillOnce(
        DoAll(SetArgReferee<1 >(sourcedata), Return(E_OK)));

    mHandle = { H_SETSOURCENOTIFICATION, 50 };
    EXPECT_CALL(*mpMockControlReceiveInterface, setSourceNotificationConfiguration(_, mpSourceElement->getID(), setNotificationConfigurations(mNotificationConfigurations))).WillOnce(
        DoAll(SetArgReferee<0 >(mHandle), Return(E_OK)));
    EXPECT_EQ(E_OK, mpPolicyAction->execute());
}

/**
 * @brief : Test to verify the execute functions for ConnectAction for Negative scenario
 *
 * @pre : Mock the IAmControlReceive & IAmPolicySend class and  initialized the CAmControlReceive,CAmMainConnectionActionConnect classes,
 *        Source, sink ,route elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details in enter in db for behavior set.
 *        and system element is also created with details of system configuration and mock class details of IAmControlReceive.
 *        and class element is also created and attach the sink,source element to class element using attach function call.
 *
 * @test : verify the "_execute" function when CAmClassActionConnect class is initialize with class element,
 *         & set the action parameter details of source,sink,class,timeout of CAmClassActionConnect.
 *         & push the different action information like action name as "ACTION_CONNECT",different sink,source name  etc
 *         in the policy action list then will check whether _execute function will execute the action or not
 *         it actually, For each policy action, it get the list of framework actions and appends in the list of child action
 *         or not,without any gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmPolicyActionTest, executeNegative)
{
    CreateSourceSink();

    mpClassActionConnect = new CAmActionConnect();
    ASSERT_THAT(mpClassActionConnect, NotNull()) << " Class Action Connect Is not Created";

    mSourceNameParam.setParam(mpSourceElement->getName());
    mSinkNameParam.setParam(mpSinkElement->getName());
    mClassNameParam.setParam(mpClassElement->getName());
    mTimeoutvalue.setParam(5000);
    // setting Parameter
    mpClassActionConnect->setParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    mpClassActionConnect->setParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    mpClassActionConnect->setParam(ACTION_PARAM_CLASS_NAME, &mClassNameParam);
    mpClassActionConnect->setParam(ACTION_PARAM_TIMEOUT, &mTimeoutvalue);

    // Action info
    mActions.actionType = ACTION_CONNECT;
    mActions.mapParameters.insert(std::pair<string, string >(ACTION_PARAM_SOURCE_NAME, "Media"));
    mActions.mapParameters.insert(std::pair<string, string >(ACTION_PARAM_SINK_NAME, "Speaker"));
    mActions.mapParameters.insert(std::pair<string, string >(ACTION_PARAM_CLASS_NAME, "Phone"));

    mListPolicyActions.push_back(mActions);

    /*
     * Policy Action Instance is being created in each Test Because the Action needs to verified
     *  is different
     */
    mpPolicyAction = new CAmPolicyAction(mListPolicyActions, mpPolicySendInterface,
            mpControlReceive);
    ASSERT_THAT(mpPolicyAction, NotNull()) << " Policy Action Connect Is not Created";
    EXPECT_EQ(E_OK, mpPolicyAction->execute());
}

int main(int argc, char * *argv)
{
    // initialize logging environment
    am::CAmLogWrapper::instantiateOnce("UTST", "Unit test for generic controller"
            , LS_ON, LOG_SERVICE_STDOUT);
    LOG_FN_CHANGE_LEVEL(LL_WARN);

    // redirect configuration path
    gc_utest::ConfigDocument config(gc_utest::ConfigDocument::Default);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
