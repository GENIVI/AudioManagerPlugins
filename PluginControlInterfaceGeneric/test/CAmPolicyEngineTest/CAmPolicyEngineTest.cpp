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
#include <set>
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"
#include "CAmPolicyReceive.h"
#include "CAmPolicyEngine.h"
#include "CAmSinkElement.h"
#include "MockIAmControlReceive.h"
#include "MockIAmPolicyReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmPolicyEngineTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;
using namespace gc_utest;


/**
 *@Class : CAmPolicyEngineTest
 *@brief : This class is used to test the CAmPolicyEngine functionality.
 */
CAmPolicyEngineTest::CAmPolicyEngineTest()
    : mMockControlReceiveInterface()
    , mCAmControlReceive(mMockControlReceiveInterface)
    , mpMockIAmPolicyReceive(NULL)
    , mpCAmPolicyEngine(NULL)
    , elementType(ET_CLASS)
{
}

CAmPolicyEngineTest::~CAmPolicyEngineTest()
{
}

void CAmPolicyEngineTest::SetUp()
{
    mpMockIAmPolicyReceive        = new MockIAmPolicyReceive();
    mpCAmPolicyEngine             = new gc::CAmPolicyEngine();
    // This will check whether controller is initialized or not
    ASSERT_THAT(mpCAmPolicyEngine, NotNull()) << " mpCAmPolicyEngine is Not Initialized";
}

void CAmPolicyEngineTest::TearDown()
{
    mListActions.clear();

    delete mpCAmPolicyEngine;
    delete mpMockIAmPolicyReceive;
}

am_Error_e CAmPolicyEngineTest::mockSetListActions(std::vector<gc_Action_s > &listActions, gc_ActionList_e actionListType)
{
    if (listActions.empty())
    {
        return E_NO_CHANGE;
    }

    mListActions = listActions;
    return E_OK;
}

/**
 * @brief :  Test to verify the startPolicyEngine function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class ,IAmPolicyReceive class and also initialized the CAmControlReceive,CAmPolicyEngine class.
 *
 * @test : verify the "startPolicyEngine" function when IamPolicyReceive mock pointer is given as input parameters
 *         then it will check whether startPolicyEngine function initialized the CAmConfigureRead class or not and also initialized
 *         local pointer of that class or not without any Gmock error.
 *
 * @result : "Pass" when startPolicyEngine function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyEngineTest, startPolicyEnginePositive)
{
    EXPECT_EQ(E_OK, mpCAmPolicyEngine->startPolicyEngine(mpMockIAmPolicyReceive));
}

/**
 * @brief :  Test to verify the getListSystemProperties function Positive scenario
 *
 * @pre :  Mock the IAmControlReceive class ,IAmPolicyReceive class and also initialized the CAmControlReceive,CAmPolicyEngine class.
 *
 * @test : verify the "getListSystemProperties" function when list of system properties is give as input parameter
 *         then it will check whether getListSystemProperties will get the list of system properties from the CAmConfigurationReader class or not
 *         without any Gmock error message.
 *
 * @result : "Pass" when getListSystemProperties function return the "E_OK" without any Gmock error meesage.
 */
TEST_F(CAmPolicyEngineTest, getListSystemProperties)
{
    std::vector<gc_SystemProperty_s > listSystemProperties;
    CAmConfigurationReader::instance().getListSystemProperties(listSystemProperties);

    // validate result
    EXPECT_EQ(2, listSystemProperties.size());
    if (listSystemProperties.size() == 2)
    {
        EXPECT_EQ(SYP_GLOBAL_LOG_THRESHOLD, listSystemProperties[0].type);
        EXPECT_EQ(LL_INFO, listSystemProperties[0].value);

        EXPECT_EQ(SYP_REGISTRATION_ALLOW_UNKNOWN_ELEMENT, listSystemProperties[1].type);
        EXPECT_EQ(1, listSystemProperties[1].value);
    }
}

/**
 * @brief Test to verify the getListClasses function Positive scenario
 *
 * @test  verify that "getListClasses()" of the CAmConfigurationReader class will properly
 *        populate the given list from configuration without any Gmock error message.
 */
TEST_F(CAmPolicyEngineTest, getListClasses)
{
    std::vector<gc_Class_s >listClasses;
    EXPECT_EQ(E_OK, CAmConfigurationReader::instance().getListClasses(listClasses));

    // validate reproduction of configured values
    EXPECT_EQ(2, listClasses.size());
    if (listClasses.size() == 2)
    {
        EXPECT_EQ("BASE", listClasses[0].name);
        EXPECT_EQ(C_PLAYBACK, listClasses[0].type);
        EXPECT_EQ(1, listClasses[0].priority);
        EXPECT_EQ(10, listClasses[0].defaultVolume);

        // t.b.d validate allowed connections: AMP=Gateway0(MediaPlayer)
        EXPECT_EQ(1, listClasses[0].listTopologies.size());

        EXPECT_EQ("NAVI", listClasses[1].name);
        EXPECT_EQ(C_PLAYBACK, listClasses[1].type);
        EXPECT_EQ(0, listClasses[1].priority);
        EXPECT_EQ(25, listClasses[1].defaultVolume);

        // t.b.d validate allowed connections: AMP=Gateway1(Navigation)
        EXPECT_EQ(1, listClasses[1].listTopologies.size());
    }
}

/**
 * @brief Test to verify the getListDomains function Positive scenario
 *
 * @test  verify that "getListDomains()" function of the CAmConfigurationReader class will properly
 *        populate the given list from configuration without any Gmock error message.
 */
TEST_F(CAmPolicyEngineTest, getListDomains)
{
    std::vector<gc_Domain_s >listDomains;
    EXPECT_EQ(E_OK, CAmConfigurationReader::instance().getListDomains(listDomains));

    // validate reproduction of configured values
    EXPECT_EQ(2, listDomains.size());
    if (listDomains.size() == 2)
    {
        EXPECT_EQ("Applications", listDomains[0].name);
        EXPECT_EQ("busName1", listDomains[0].busname);
        EXPECT_EQ("Cpu", listDomains[0].nodename);

        EXPECT_EQ("VirtDSP", listDomains[1].name);
        EXPECT_EQ("busName2", listDomains[1].busname);
        EXPECT_EQ("Cpu", listDomains[1].nodename);
    }
}

/**
 * @brief :  Test to verify the processTrigger function with connect type trigger for Positive scenario
 *
 * @pre :  Mock the IAmControlReceive class ,IAmPolicyReceive class and also initialized the CAmControlReceive,CAmPolicyEngine class.
 *
 * @test : verify the "processTriggerConnect" function when policy engine is started and connection information like source,sink name ,priority, state,volume etc details
 *          are push into the list of connection info and also trigger type,sink,source,class name is set and all this detail is give as input parameter using structure
 *          to processTriggerConnect function then check whether processTriggerConnect function will get Action base on the trigger type.
 *          and also update the Action parameter or not and then finally set the action in the list of actions or not, without any Gmock error message.
 *
 * @result : "Pass" when processTriggerConnect function return the "E_OK" without any Gmock error message.
 */
TEST_F(CAmPolicyEngineTest, processTriggerConnect)
{
    mpCAmPolicyEngine->startPolicyEngine(mpMockIAmPolicyReceive);
    className   = "BASE";
    sourceName  = "MediaPlayer";
    sinkName    = "Gateway0";
    elementType = ET_CLASS;
    // Connection information
    ConnectionInfo.sourceName      = "MediaPlayer";
    ConnectionInfo.sinkName        = "Gateway0";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;
    std::vector<gc_ConnectionInfo_s >listConnectionInfos;
    listConnectionInfos.push_back(ConnectionInfo);

    // USER_CONNECTION_REQUEST:{className:BASE, sourceName:MediaPlayer, sinkName:Gateway0}
    gc_triggerParams_s             triggerParams;
    triggerParams.triggerType = USER_CONNECTION_REQUEST;
    triggerParams.sinkName    = sinkName;
    triggerParams.sourceName  = sourceName;
    triggerParams.className   = className;
    EXPECT_CALL(*mpMockIAmPolicyReceive, getListMainConnections(elementType, className, _)).WillOnce(DoAll(SetArgReferee<2>(listConnectionInfos), Return(E_OK)));
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL))
        .WillOnce(Invoke(this, &CAmPolicyEngineTest::mockSetListActions));
    EXPECT_EQ(E_OK, mpCAmPolicyEngine->processTrigger(triggerParams));

    // validate outcome
    EXPECT_EQ(1, mListActions.size());
    if (mListActions.size())
    {
        // ACTION_CONNECT:{className:BASE, timeOut:5000}
        EXPECT_EQ(ACTION_CONNECT, mListActions[0].actionType);
        EXPECT_EQ("BASE", mListActions[0].mapParameters["className"]);
    }
}

/**
 * @brief  Test to verify the processTrigger function with MuteState type trigger for Positive scenario
 *
 * @test   verify multiple actions created from the "processTrigger" function for trigger:
 *             USER_SET_SINK_MUTE_STATE:{className:BASE, sinkName:AMP, muteState:1}
 */
TEST_F(CAmPolicyEngineTest, processTriggerMuteState)
{
    mpCAmPolicyEngine->startPolicyEngine(mpMockIAmPolicyReceive);
    className  = "NAVI";
    sourceName = "MediaPlayer";
    sinkName   = "Gateway0";
    am_MuteState_e muteState = MS_MUTED;
    elementType = ET_CLASS;
    // Connection information
    ConnectionInfo.sourceName      = "MediaPlayer";
    ConnectionInfo.sinkName        = "Gateway1";
    ConnectionInfo.priority        = 0;
    ConnectionInfo.connectionState = CS_CONNECTED;
    ConnectionInfo.volume          = 10;
    string         className1  = "BASE";
    string         sourceName1 = "Gateway0";
    string         sinkName1   = "AMP";
    am_MuteState_e muteState1  = MS_MUTED;
    // Connection information
    gc_ConnectionInfo_s ConnectionInfo1;
    ConnectionInfo1.sourceName      = "Gateway0";
    ConnectionInfo1.sinkName        = "Gateway0";
    ConnectionInfo1.priority        = 0;
    ConnectionInfo1.connectionState = CS_CONNECTED;
    ConnectionInfo1.volume          = 10;

    std::vector<gc_ConnectionInfo_s >listConnectionInfos;
    listConnectionInfos.push_back(ConnectionInfo);
    listConnectionInfos.push_back(ConnectionInfo1);

    // USER_SET_SINK_MUTE_STATE:{className:BASE, sinkName:AMP, muteState:1}
    gc_triggerParams_s             triggerParams;
    triggerParams.sinkName    = sinkName1;
    triggerParams.className   = className1;
    triggerParams.muteState   = muteState;
    triggerParams.triggerType = USER_SET_SINK_MUTE_STATE;
    EXPECT_CALL(*mpMockIAmPolicyReceive, getListMainConnections(elementType, className, _)).WillRepeatedly(DoAll(SetArgReferee<2>(listConnectionInfos), Return(E_OK)));
    EXPECT_CALL(*mpMockIAmPolicyReceive, getListMainConnections(elementType, className1, _)).WillRepeatedly(DoAll(SetArgReferee<2>(listConnectionInfos), Return(E_OK)));
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL))
        .WillOnce(Invoke(this, &CAmPolicyEngineTest::mockSetListActions));
    EXPECT_EQ(E_OK, mpCAmPolicyEngine->processTrigger(triggerParams));

    // validate outcome
    EXPECT_EQ(3, mListActions.size());
    if (mListActions.size() == 3)
    {
        // ACTION_MUTE:{className:NAVI, muteState:1, timeOut:5000}
        auto& actionMute = mListActions[0];
        EXPECT_EQ(ACTION_MUTE, actionMute.actionType);
        EXPECT_EQ("NAVI", actionMute.mapParameters["className"]);
        EXPECT_EQ("1", actionMute.mapParameters["muteState"]);   // MS_MUTED

        // ACTION_DISCONNECT:{className:NAVI, timeOut:5000}
        auto& actionDisconnect = mListActions[1];
        EXPECT_EQ(ACTION_DISCONNECT, actionDisconnect.actionType);
        EXPECT_EQ("NAVI", actionDisconnect.mapParameters["className"]);

        // ACTION_UNLIMIT:{className:BASE, limitState:2, rampTime:1000, rampType:4, timeOut:5000
        auto& actionUnlimit = mListActions[2];
        EXPECT_EQ(ACTION_UNLIMIT, actionUnlimit.actionType);
        EXPECT_EQ("BASE", actionUnlimit.mapParameters["className"]);
        EXPECT_EQ("2", actionUnlimit.mapParameters["limitState"]);  // LS_UNLIMITED
        EXPECT_EQ("4", actionUnlimit.mapParameters["rampType"]);    // RAMP_GENIVI_LINEAR
        EXPECT_EQ("1000", actionUnlimit.mapParameters["rampTime"]);
    }
}

/**
 * @brief  Test to verify the processTrigger function with MainVolume type trigger for Positive scenario
 *
 * @test   verify the "processTrigger" function for trigger:
 *             USER_SET_VOLUME:{sinkName:Gateway0, mainVolume:10}
 */
TEST_F(CAmPolicyEngineTest, processTriggerMainVolume)
{
    mpCAmPolicyEngine->startPolicyEngine(mpMockIAmPolicyReceive);
    sinkName = "Gateway0";
    am_mainVolume_t mainVolume = 10;

    // USER_SET_VOLUME:{sinkName:Gateway0, mainVolume:10}
    gc_triggerParams_s             triggerParams;
    triggerParams.triggerType = USER_SET_VOLUME;
    triggerParams.sinkName    = sinkName;
    triggerParams.mainVolume  = mainVolume;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL))
        .WillOnce(Invoke(this, &CAmPolicyEngineTest::mockSetListActions));
    EXPECT_EQ(E_OK, mpCAmPolicyEngine->processTrigger(triggerParams));

    // validate outcome
    EXPECT_EQ(1, mListActions.size());
    if (mListActions.size())
    {
        // ACTION_SET_VOLUME:{mainVolume:10, sinkName:Gateway0, timeOut:5000}
        EXPECT_EQ(ACTION_SET_VOLUME, mListActions[0].actionType);
        EXPECT_EQ("Gateway0", mListActions[0].mapParameters["sinkName"]);
        EXPECT_EQ("10", mListActions[0].mapParameters["mainVolume"]);
    }
}

/**
 * @brief  Test to verify the processTrigger function with connect type trigger for Positive scenario
 *
 * @test   verify the "processTriggerConnect" function when policy engine is started and connection information like source,sink name ,priority, state,volume etc details
 *          are push into the list of connection info and also trigger type,sink,source,class name is set and all this detail is give as input parameter using structure
 *          to processTriggerConnect function then check whether processTriggerConnect function will get Action base on the trigger type.
 *          and also update the Action parameter or not and then finally set the action in the list of actions or not, without any Gmock error message.
 *
 * @result : "Pass" when processTriggerConnect function return the "E_OK" without any Gmock error message.
 */
TEST_F(CAmPolicyEngineTest, processTriggerMainVolumeNested)
{
    // prepare configuration with extended policy rules
    vector<ConfigTag> policies = ConfigTag::DefaultPolicies;
    policies[2] = ConfigTag("policy", "trigger=\"USER_SET_VOLUME\""
            , "<process name=\"scaled Sink volume with nested functions policy\">\n"
              "  <condition>name(SINK, REQUESTING) EQ \"Gateway0\"</condition>\n"
              "  <action type=\"ACTION_SET_VOLUME\" sinkName=\"Gateway0\" sourceName=\"Navigation\" timeOut=\"500\"\n"
              "     volume=\"scale(SYSTEM, mainSoundPropertyValue(SINK, 'AMP', 'MSP_GENIVI_TREBLE'), '0,-1500;10,-300')\" />\n"
              "</process>");
    LOG_FN_INFO("\n", policies[2]);

    vector<ConfigTag> config
      = {
              ConfigTag("classes", "", ConfigTag::DefaultClasses)
            , ConfigTag("system", "", ConfigTag::DefaultSystem)
            , ConfigTag("policies", "", policies)
            , ConfigTag("properties", "", ConfigTag::DefaultProperties)
        };
    ConfigDocument xml(config);

    // create sink element
    gc_Sink_s sinkInfo;
    sinkInfo.name = "Gateway0";
    sinkInfo.className = "BASE";
    sinkInfo.available = {A_AVAILABLE, AR_GENIVI_NEWMEDIA};
    gc_MainSoundProperty_s treble;
    treble.type = MSP_GENIVI_TREBLE;
    treble.value = 5;
    treble.minValue = 0;
    treble.maxValue = 10;
    treble.isPersistenceSupported = false;
    sinkInfo.listGCMainSoundProperties = {treble};
    EXPECT_CALL(mMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _))
        .WillOnce(DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    CAmSinkFactory::createElement(sinkInfo, &mCAmControlReceive);

    mpCAmPolicyEngine->startPolicyEngine(mpMockIAmPolicyReceive);
    sinkName = "Gateway0";
    am_mainVolume_t mainVolume = 10;

    // USER_SET_VOLUME:{sinkName:Gateway0, mainVolume:10}
    gc_triggerParams_s             triggerParams;
    triggerParams.triggerType = USER_SET_VOLUME;
    triggerParams.sinkName    = sinkName;
    triggerParams.mainVolume  = mainVolume;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL))
        .WillOnce(Invoke(this, &CAmPolicyEngineTest::mockSetListActions));
    EXPECT_CALL(*mpMockIAmPolicyReceive, getMainSoundProperty(ET_SINK, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(5), Return(E_OK)));
    EXPECT_EQ(E_OK, mpCAmPolicyEngine->processTrigger(triggerParams));

    // validate outcome
    EXPECT_EQ(1, mListActions.size());
    if (mListActions.size())
    {
        // ACTION_SET_VOLUME:{mainVolume:10, sinkName:Gateway0, timeOut:5000}
        EXPECT_EQ(ACTION_SET_VOLUME, mListActions[0].actionType);
        EXPECT_EQ("Navigation", mListActions[0].mapParameters["sourceName"]);
        EXPECT_EQ("Gateway0", mListActions[0].mapParameters["sinkName"]);
        EXPECT_EQ("-900", mListActions[0].mapParameters["volume"]);  // half way between -1500 and,-300
        EXPECT_EQ("500", mListActions[0].mapParameters["timeOut"]);
    }

    CAmSinkFactory::destroyElement();
}

/**
 * @brief  Test to verify the processTrigger function with a function as parameter
 *          to MainVolume trigger for Positive scenario
 *
 * @test   verify the "processTrigger" function for trigger:
 *             USER_SET_VOLUME:{sinkName:Gateway0, mainVolume:10}
 *         with policy rule:
 *             <action type="ACTION_SET_VOLUME" sinkName="Gateway0" 
 *                     volume="scale(SYSTEM,REQ_MAIN_VOL,'limitAbsMainVolToDBVolMap')" />
 */
TEST_F(CAmPolicyEngineTest, processTriggerMainVolumeScaled)
{
    // prepare configuration with extended policy rules
    vector<ConfigTag> templates = ConfigTag::DefaultTemplates;
    templates.insert(templates.begin(), ConfigTag("scaleConversionMap", "name=\"limitAbsMainVolToDBVolMap\""
               , "0,-3000;1,-300;20,0"));
    LOG_FN_INFO("\n", templates.front());

    std::vector<ConfigTag> system = {
            ConfigTag("templates", "", templates)
          , ConfigTag::DefaultSystem[1]
          , ConfigTag::DefaultSystem[2]
    };

    vector<ConfigTag> policies = ConfigTag::DefaultPolicies;
    policies[2] = ConfigTag("policy", "trigger=\"USER_SET_VOLUME\""
            , "<process name=\"scaled Sink volume policy\">\n"
              "  <action type=\"ACTION_SET_VOLUME\" sinkName=\"Gateway0\""
              "     volume=\"scale(SYSTEM,REQ_MAIN_VOL,'limitAbsMainVolToDBVolMap')\" />\n"
              "</process>");

    vector<ConfigTag> config
      = {
              ConfigTag("classes", "", ConfigTag::DefaultClasses)
            , ConfigTag("system", "", system)
            , ConfigTag("policies", "", policies)
            , ConfigTag("properties", "", ConfigTag::DefaultProperties)
        };
    ConfigDocument xml(config);

    // execute test scenario
    mpCAmPolicyEngine->startPolicyEngine(mpMockIAmPolicyReceive);

    // USER_SET_VOLUME:{sinkName:Gateway0, mainVolume:10}
    gc_triggerParams_s             triggerParams;
    triggerParams.triggerType = USER_SET_VOLUME;
    triggerParams.sinkName    = "Gateway0";
    triggerParams.mainVolume  = 10;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL))
        .WillOnce(Invoke(this, &CAmPolicyEngineTest::mockSetListActions));
    EXPECT_EQ(E_OK, mpCAmPolicyEngine->processTrigger(triggerParams));

    // validate outcome
    EXPECT_EQ(1, mListActions.size());
    if (mListActions.size())
    {
        // ACTION_SET_VOLUME:{sinkName:Gateway0, timeOut:5000, volume:-158}
        EXPECT_EQ(ACTION_SET_VOLUME, mListActions[0].actionType);
        EXPECT_EQ("Gateway0", mListActions[0].mapParameters["sinkName"]);
        EXPECT_EQ("-158", mListActions[0].mapParameters["volume"]);  // scaled volume
    }
}

/**
 * @brief  Test to verify the processTrigger function with Source Main Sound Property type trigger for Positive scenario
 *
 * @test   verify the "processTriggerConnect" function for trigger
 *         USER_SET_SOURCE_MAIN_SOUND_PROPERTY:{sourceName:MediaPlayer, mainSoundProperty:{type:1, value:10}}
 */
TEST_F(CAmPolicyEngineTest, processTriggerSourceMainSoundProperty)
{
    mpCAmPolicyEngine->startPolicyEngine(mpMockIAmPolicyReceive);
    sourceName = "MediaPlayer";
    am_MainSoundProperty_s soundProperty;
    soundProperty.type  = MSP_GENIVI_TREBLE;
    soundProperty.value = 10;

    gc_triggerParams_s             triggerParams;
    triggerParams.triggerType             = USER_SET_SOURCE_MAIN_SOUND_PROPERTY;
    triggerParams.sourceName              = sourceName;
    triggerParams.mainSoundProperty.type  = soundProperty.type;
    triggerParams.mainSoundProperty.value = soundProperty.value;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL))
        .WillOnce(Invoke(this, &CAmPolicyEngineTest::mockSetListActions));
    EXPECT_EQ(E_OK, mpCAmPolicyEngine->processTrigger(triggerParams));

    // validate outcome
    EXPECT_EQ(1, mListActions.size());
    if (mListActions.size())
    {
        // ACTION_SET_PROPERTY:{propertyType:1, propertyValue:10, sourceName:MediaPlayer, timeOut:5000}
        EXPECT_EQ(ACTION_SET_PROPERTY, mListActions[0].actionType);
        EXPECT_EQ("MediaPlayer", mListActions[0].mapParameters["sourceName"]);
        EXPECT_EQ("1", mListActions[0].mapParameters["propertyType"]);  // SP_GENIVI_TREBLE
        EXPECT_EQ("10", mListActions[0].mapParameters["propertyValue"]);
    }
}

/**
 * @brief  Test to verify the processTrigger function with Sink Main Notification configuration type trigger for Positive scenario
 *
 * @test   verify the "processTrigger" function for trigger
 *            USER_SET_SINK_MAIN_NOTIFICATION_CONFIGURATION:{sinkName:AMP, notificatonConfiguration:{type:5, status:2, parameter:20}}
 */
TEST_F(CAmPolicyEngineTest, processTriggerNotificationConfiguration)
{
    // setup policy rule
    ConfigTag systemPolicy("policy", "trigger=\"USER_SET_SINK_MAIN_NOTIFICATION_CONFIGURATION\""
            , "<process>\n"
              "  <action type=\"ACTION_SET_NOTIFICATION_CONFIGURATION\" sinkName=\"REQ_SINK_NAME\"\n"
              "          notificationConfigurationType=\"REQ_NC_TYPE\" notificationConfigurationStatus=\"REQ_NC_STATUS\"\n"
              "          notificationConfigurationParam=\"REQ_NC_PARAM\" />\n"
              "</process>");
    LOG_FN_INFO("\n", systemPolicy);
    const std::vector<ConfigTag> conf
      = {
              ConfigTag("classes", "", "")
            , ConfigTag("system", "", "")
            , ConfigTag("policies", "", systemPolicy)
            , ConfigTag("properties", "", "")
        };
    ConfigDocument configdoc(conf);

    mpCAmPolicyEngine->startPolicyEngine(mpMockIAmPolicyReceive);
    sinkName                               = "AMP";
    am_NotificationConfiguration_s notificationConfigurations;
    notificationConfigurations.type        = NT_OVER_TEMPERATURE;
    notificationConfigurations.status      = NS_PERIODIC;
    notificationConfigurations.parameter   = 20;

    gc_triggerParams_s             triggerParams;
    triggerParams.sinkName                 = sinkName;
    triggerParams.notificatonConfiguration = notificationConfigurations;
    triggerParams.triggerType              = USER_SET_SINK_MAIN_NOTIFICATION_CONFIGURATION;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL))
        .WillOnce(Invoke(this, &CAmPolicyEngineTest::mockSetListActions));
    EXPECT_EQ(E_OK, mpCAmPolicyEngine->processTrigger(triggerParams));

    // validate outcome
    EXPECT_EQ(1, mListActions.size());
    if (mListActions.size())
    {
        // ACTION_SET_NOTIFICATION_CONFIGURATION:{notificationConfigurationParam:20
        //    , notificationConfigurationStatus:2, notificationConfigurationType:5, sinkName:AMP, timeOut:5000}
        auto &params = mListActions[0].mapParameters;
        EXPECT_EQ(ACTION_SET_NOTIFICATION_CONFIGURATION, mListActions[0].actionType);
        EXPECT_EQ("AMP", params["sinkName"]);
        EXPECT_EQ("5", params["notificationConfigurationType"]);  // NT_OVER_TEMPERATURE
        EXPECT_EQ("2", params["notificationConfigurationStatus"]);  // NS_PERIODIC
        EXPECT_EQ("20", params["notificationConfigurationParam"]);
    }
}

/**
 * @brief  Test to verify the processTrigger function for trigger
 *
 * @test   verify the "processTrigger" function for trigger
 *            USER_SET_SYSTEM_PROPERTY:{systemProperty:{type:1, value:5}}
 */
TEST_F(CAmPolicyEngineTest, processTriggerSystemProperty)
{
    // setup policy rule
    ConfigTag systemPolicy("policy", "trigger=\"USER_SET_SYSTEM_PROPERTY\""
            , "<process>\n"
              "  <action type=\"ACTION_SET_SYSTEM_PROPERTY\" systemProperty=\"REQUESTING\" />\n"
              "</process>");
    LOG_FN_INFO("\n", ConfigTag("policies", "", systemPolicy));
    const std::vector<ConfigTag> conf
      = {
              ConfigTag("classes", "", ConfigTag::DefaultClasses)
            , ConfigTag("system", "", ConfigTag::DefaultSystem)
            , ConfigTag("policies", "", systemPolicy)
            , ConfigTag("properties", "", ConfigTag::DefaultProperties)
        };
    ConfigDocument configdoc(conf);

    mpCAmPolicyEngine->startPolicyEngine(mpMockIAmPolicyReceive);
    am_SystemProperty_s systemProperty;
    systemProperty.type                = SYP_DEBUG;
    systemProperty.value               = 5;

    gc_triggerParams_s             triggerParams;
    triggerParams.triggerType          = USER_SET_SYSTEM_PROPERTY;
    triggerParams.systemProperty.type  = systemProperty.type;
    triggerParams.systemProperty.value = systemProperty.value;
    EXPECT_CALL(*mpMockIAmPolicyReceive, setListActions(_, AL_NORMAL))
        .WillOnce(Invoke(this, &CAmPolicyEngineTest::mockSetListActions));
    EXPECT_EQ(E_OK, mpCAmPolicyEngine->processTrigger(triggerParams));

    // validate outcome
    EXPECT_EQ(1, mListActions.size());
    if (mListActions.size())
    {
        // ACTION_SET_SYSTEM_PROPERTY:{propertyType:1, propertyValue:5, timeOut:5000}
        EXPECT_EQ(ACTION_SET_SYSTEM_PROPERTY, mListActions[0].actionType);
        EXPECT_EQ("1", mListActions[0].mapParameters["propertyType"]);
        EXPECT_EQ("5", mListActions[0].mapParameters["propertyValue"]);  // scaled volume
    }
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
