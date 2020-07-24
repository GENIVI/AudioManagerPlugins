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

/*
 * This file contains the All Elements DataBase. Kind of Mock to AudioManager DataBase.
 */

#ifndef CGMOCKCOMMONFUNCTIONS_H_
#define CGMOCKCOMMONFUNCTIONS_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <vector>
#include <algorithm>
#include <string>

/*
 * Elements name should match in Vector List
 */
std::vector<std::string > sourceNameList = { "AnySource1", "AnySource2", "AnySource3", "AnySource4",
                                             "AnySource5", "AnySource6", "AnySource7", "AnySource8",
                                             "AnySource9", "AnySource10", "AnySource11", "AnySource12",
                                             "AnySource13", "AnySource14", "AnySource15", "AnySource16",
                                             "AnySource17", "AnySource18", "AnySource19", "AnySource20",
                                             "AnySource21", "AnySource22", "AnySource23", "AnySource24",
                                             "AnySource25", "AnySource26", "AnySource27", "AnySource28",
                                             "AnySource29", "AnySource30", "AnySource31", "AnySource32",
                                             "AnySource33", "AnySource34", "AnySource35", "AnySource36",
                                             "AnySource37", "AnySource38", "AnySource39", "AnySource40",
                                             "AnySource41", "AnySource42", "AnySource43", "AnySource44",
                                             "AnySource45", "AnySource46", "AnySource47", "AnySource48",
                                             "AnySource49", "AnySource50", "AnySource51", "AnySource52",
                                             "AnySource53", "AnySource54", "AnySource55", "AnySource56",
                                             "AnySource57", "AnySource58", "AnySource59", "AnySource60",
                                             "AnySource61", "AnySource62", "AnySource63", "AnySource64",
                                             "AnySource65", "AnySource66", "AnySource67", "AnySource68",
                                             "AnySource69", "AnySource70", "AnySource71", "AnySource72",
                                             "AnySource73", "AnySource74", "AnySource75", "AnySource76",
                                             "AnySource77", "AnySource78", "AnySource79", "AnySource80",
                                             "AnySource81", "AnySource82", "Media", "Navi", "TTS", "Phone",
                                             "MediaPlayer", "NAVI", "Gateway0", "Gateway1", "GatewaySource" };

std::vector<std::string > sinkNameList = { "AnySink1", "AnySink2", "AnySink3", "AnySink4", "AnySink5",
                                           "AnySink6", "AnySink7", "AnySink8", "AnySink9", "AnySink10",
                                           "AnySink11", "AnySink12", "AnySink13", "AnySink14", "AnySink15",
                                           "AnySink16", "AnySink17", "AnySink18", "AnySink19", "AnySink20",
                                           "AnySink21", "AnySink22", "AnySink23", "AnySink24", "AnySink25",
                                           "AnySink26", "AnySink27", "AnySink28", "AnySink29", "AnySink30",
                                           "AnySink31", "AnySink32", "AnySink33", "AnySink34", "AnySink35",
                                           "AnySink36", "AnySink37", "AnySink38", "AnySink39", "AnySink40",
                                           "AnySink41", "AnySink42", "AnySink43", "AnySink44", "AnySink45",
                                           "AnySink46", "AnySink47", "AnySink48", "AnySink49", "AnySink50",
                                           "AnySink51", "AnySink52", "AnySink53", "AnySink54", "AnySink55",
                                           "AnySink56", "AnySink57", "AnySink58", "AnySink59", "AnySink60",
                                           "AnySink61", "AnySink62", "AnySink63", "AnySink64", "AnySink65",
                                           "AnySink66", "AnySink67", "AnySink68", "AnySink69", "AnySink70",
                                           "AnySink71", "AnySink72", "AnySink73", "AnySink74", "AnySink75",
                                           "AnySink76", "AnySink77", "AnySink78", "AnySink79", "AnySink80",
                                           "AnySink81", "AnySink82", "AllSpeakers", "FrontSpeakers", "AMP",
                                           "Gateway0", "Gateway1", "Gateway2", };

std::vector<std::string > domainNameList = { "AnyDomain1", "AnyDomain2", "AnyDomain3", "AnyDomain4",
                                             "AnyDomain5", "AnyDomain6", "AnyDomain7", "AnyDomain8",
                                             "AnyDomain9", "AnyDomain10", "AnyDomain11", "AnyDomain12",
                                             "AnyDomain13", "AnyDomain14", "AnyDomain15", "AnyDomain16",
                                             "AnyDomain17", "AnyDomain18", "AnyDomain19", "AnyDomain20",
                                             "AnyDomain21", "AnyDomain22", "PulseAudio", "QAudio", "DSP",
                                             "AMP", "Applications", "VirtDSP", };

std::vector<std::string > gatewayNameList = { "AnyGateway1", "AnyGateway2", "AnyGateway3", "AnyGateway4",
                                              "AnyGateway5", "AnyGateway6", "AnyGateway7", "AnyGateway8",
                                              "AnyGateway9", "AnyGateway10", "AnyGateway11", "AnyGateway12",
                                              "Gateway0", "Gateway1", };

std::vector<std::string > sinkClassNameList = { "AnySinkClass1", "AnySinkClass2", "AnySinkClass3",
                                                "AnySinkClass4", "AnySinkClass5", "AnySinkClass6",
                                                "AnySinkClass7", "AnySinkClass8", "AnySinkClass9",
                                                "AnySinkClass10", "AnySinkClass11", "AnySinkClass12",
                                                "AnySinkClass13", "AnySinkClass14", "AnySinkClass15",
                                                "AnySinkClass16", "AnySinkClass17", "AnySinkClass18",
                                                "AnySinkClass19", "AnySinkClass20", "AnySinkClass21",
                                                "AnySinkClass22", "default", "BASE", "NAVI", };

std::vector<std::string > sourceClassNameList = { "AnySourceClass1", "AnySourceClass2", "AnySourceClass3",
                                                  "AnySourceClass4", "AnySourceClass5", "AnySourceClass6",
                                                  "AnySourceClass7", "AnySourceClass8", "AnySourceClass9",
                                                  "AnySourceClass10", "AnySourceClass11", "AnySourceClass12",
                                                  "AnySourceClass13", "AnySourceClass14", "AnySourceClass15",
                                                  "AnySourceClass16", "AnySourceClass17", "AnySourceClass18",
                                                  "AnySourceClass19", "AnySourceClass20", "AnySourceClass21",
                                                  "AnySourceClass22", "default", "BASE", "NAVI", };

MATCHER_P(IsSourceNamePresent, sourceData, "")
{
    return ((find(sourceNameList.begin(), sourceNameList.end(), sourceData.name) != sourceNameList.end()));
}

MATCHER_P(IsDomainNamePresent, doaminData, "")
{
    return ((find(domainNameList.begin(), domainNameList.end(), doaminData.name) != domainNameList.end()));
}
/*
 * This function compare the both pattern AnyGateway and Gateway based on position and length of std::string
 */
bool compareGatewayName(std::string name)
{
    return ((name.compare(0, 10, "AnyGateway") == 0) || (name.compare(0, 7, "Gateway") == 0));
}

MATCHER_P(IsGatewayNamePresent, gatewayData, "")
{
    return ((find_if(gatewayNameList.begin(), gatewayNameList.end(), compareGatewayName) != gatewayNameList.end()));
}

MATCHER_P(IsSinkNamePresent, sinkData, "")
{
    return ((find(sinkNameList.begin(), sinkNameList.end(), sinkData.name) != sinkNameList.end()));
}

MATCHER_P(IsSinkClassOk, sinkClassData, "")
{
    return ((find(sinkClassNameList.begin(), sinkClassNameList.end(), sinkClassData.name) != sinkClassNameList.end()));
}

MATCHER_P(IsSourceClassOk, sourceClassData, "")
{
    return ((find(sourceClassNameList.begin(), sourceClassNameList.end(), sourceClassData.name) != sourceClassNameList.end()));
}
/*
 * sourceIDList and sinkIDList will be used to collect the SourceId and SinkId at runTime from Test Functions
 */
std::vector<uint16_t > sourceIDList;
std::vector<uint16_t > sinkIDList;

MATCHER_P(IsMainConnSinkSourceOk, mainConnectionData, "")
{
    return ((find(sourceIDList.begin(), sourceIDList.end(), mainConnectionData.sourceID) != sourceIDList.end()) &&
            ((find(sinkIDList.begin(), sinkIDList.end(), mainConnectionData.sinkID)) != sinkIDList.end()));
}

/**
 * Describes the different system properties.
 * Custom System property defined for Test
 */
typedef uint16_t am_CustomSystemPropertyType_t;
static const am_CustomSystemPropertyType_t SYP_DEBUG = 1;

namespace am {
namespace gc {

/*
 * This Matcher will be used to check SystemProperty Type used in Unit Test
 */
std::vector<am_CustomSystemPropertyType_t > systemPropertylist = {
    SYP_GLOBAL_LOG_THRESHOLD, SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES,
    SYP_REGISTRATION_DOMAIN_TIMEOUT, SYP_DEBUG,
};

MATCHER_P(setproperty, propertyData, "")
{
    return ((find(systemPropertylist.begin(), systemPropertylist.end(), propertyData.type) != systemPropertylist.end()));
}

/*
 * This Matcher Will be used to check handle in unit Test
 */

std::vector<am_Handle_e > vectHandle = { H_CONNECT, H_DISCONNECT, H_SETSOURCESTATE, H_SETSINKVOLUME,
                                         H_SETSOURCEVOLUME, H_SETSINKSOUNDPROPERTY,
                                         H_SETSOURCESOUNDPROPERTY, H_SETSINKSOUNDPROPERTIES,
                                         H_SETSOURCESOUNDPROPERTIES, H_CROSSFADE, H_SETVOLUMES,
                                         H_SETSINKNOTIFICATION, H_SETSOURCENOTIFICATION, };

MATCHER_P(IsHandleOk, handleData, "")
{
    return ((find(vectHandle.begin(), vectHandle.end(), handleData.handleType) != vectHandle.end()));
}

/*
 * This Matcher will be used to check SoundProperty in Unit Test
 */

std::vector<am_CustomMainSoundPropertyType_t > mainSoundPropertylist = {
    MSP_GENIVI_TREBLE, MSP_GENIVI_MID, MSP_GENIVI_BASS, MSP_UNKNOWN,
};

MATCHER_P(setSoundProperty, soundPropertyData, "")
{
    return ((find(mainSoundPropertylist.begin(), mainSoundPropertylist.end(), soundPropertyData.type) != mainSoundPropertylist.end()));
}

/*
 * This Matcher will be used to check the Notification Configuration Type
 */
// custom Notification configuration for test
typedef uint16_t am_CustomNotificationType_t;
static const am_CustomNotificationType_t NT_OVER_TEMPERATURE = 5;

std::vector<am_CustomNotificationType_t > notificationConfigTypeList = {
    NT_OVER_TEMPERATURE, NT_UNKNOWN
};

std::vector<am_NotificationStatus_e > notificationConfigStatusList = {
    NS_UNKNOWN, NS_OFF, NS_PERIODIC, NS_MINIMUM, NS_MAXIMUM, NS_CHANGE,
};

std::vector<am_CustomMainSoundPropertyType_t > amCustomMainSoundPropertyType = {
    MSP_UNKNOWN, MSP_GENIVI_TREBLE, MSP_GENIVI_MID, MSP_GENIVI_BASS
};

std::vector< am_Availability_e > amAvailabilityEnumV = {
    A_UNKNOWN, A_AVAILABLE, A_UNAVAILABLE, A_MAX
};

std::vector< am_CustomAvailabilityReason_t> amCustomAvailabilityReasonTypeV = {
    AR_UNKNOWN, AR_GENIVI_NEWMEDIA, AR_GENIVI_SAMEMEDIA, AR_GENIVI_NOMEDIA, AR_GENIVI_TEMPERATURE, AR_GENIVI_VOLTAGE, AR_GENIVI_ERRORMEDIA
};

std::vector< am_CustomConnectionFormat_t > amCustomConnectionFormatTypeV = {
    CF_UNKNOWN, CF_GENIVI_MONO, CF_GENIVI_STEREO, CF_GENIVI_ANALOG, CF_GENIVI_AUTO
};

MATCHER_P(setNotificationConfigurations, notificationConfiguration, "")
{
    return ((find(notificationConfigTypeList.begin(), notificationConfigTypeList.end(), notificationConfiguration.type) != notificationConfigTypeList.end()) &&
            ((find(notificationConfigStatusList.begin(), notificationConfigStatusList.end(), notificationConfiguration.status)) != notificationConfigStatusList.end()));
}

MATCHER_P(setNotificationApyload, notificationpayload, "")
{
    return ((find(notificationConfigTypeList.begin(), notificationConfigTypeList.end(), notificationpayload.type) != notificationConfigTypeList.end()));
}

MATCHER_P(setMainSoundproperty, propertyData, "")
{
    return ((find(amCustomMainSoundPropertyType.begin(), amCustomMainSoundPropertyType.end(), propertyData.type) != amCustomMainSoundPropertyType.end()));
}

MATCHER_P(setAvailable, ElementData, "")
{
    return ((find(amAvailabilityEnumV.begin(), amAvailabilityEnumV.end(), ElementData.availability) != amAvailabilityEnumV.end())
            && (find(amCustomAvailabilityReasonTypeV.begin(), amCustomAvailabilityReasonTypeV.end(), ElementData.availabilityReason) != amCustomAvailabilityReasonTypeV.end()));
}

MATCHER_P(setConnectionFormat, ConnectionFormat, "")
{
    return ((find(amCustomConnectionFormatTypeV.begin(), amCustomConnectionFormatTypeV.end(), ConnectionFormat) != amCustomConnectionFormatTypeV.end()));
}

MATCHER_P(IsSourceHandleOk, handleData, "")
{
    return ((find(vectHandle.begin(), vectHandle.end(), handleData.handleType) != vectHandle.end()));
}

}

}

#endif /* CGMOCKCOMMONFUNCTIONS_H_ */
