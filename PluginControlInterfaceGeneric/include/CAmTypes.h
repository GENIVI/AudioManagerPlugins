/******************************************************************************
 * @file: CAmTypes.h
 *
 * This file contains the declaration of all the constants and enum shared
 * between framework and policy engine.
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Toshiaki Isogai <tisogai@jp.adit-jv.com>
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>
 *          Prashant Jain   <pjain@jp.adit-jv.com>
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#ifndef GC_TYPES_H_
#define GC_TYPES_H_

#include "IAmControlCommon.h"

namespace am {
namespace gc {

#define DEFAULT_UNDO_TIMEOUT        (10000)
#define INFINITE_TIMEOUT            (0xFFFFFFFF)
#define DEFAULT_RAMP_TIME           (200)
#define DEFAULT_ASYNC_ACTION_TIME   (2000)
#define DEFAULT_RAMP_TYPE           (RAMP_GENIVI_DIRECT)
#define DEFAULT_OFFSET_VOLUME       (0)
#define DEFAULT_LIMIT_PATTERN       (0xFFFFFFFF)
#define E_WAIT_FOR_CHILD_COMPLETION    (-1)

//supported parameters of actions
#define ACTION_PARAM_CLASS_NAME             "className"
#define ACTION_PARAM_SOURCE_NAME            "sourceName"
#define ACTION_PARAM_SINK_NAME              "sinkName"
#define ACTION_PARAM_GATEWAY_NAME           "gatewayName"
#define ACTION_PARAM_RAMP_TYPE              "rampType"
#define ACTION_PARAM_RAMP_TIME              "rampTime"
#define ACTION_PARAM_MAIN_VOLUME_STEP       "mainVolumeStep"
#define ACTION_PARAM_MAIN_VOLUME            "mainVolume"
#define ACTION_PARAM_VOLUME_STEP            "volumeStep"
#define ACTION_PARAM_VOLUME                 "volume"
#define ACTION_PARAM_ORDER                  "order"
#define ACTION_PARAM_PROPERTY_TYPE          "propertyType"
#define ACTION_PARAM_PROPERTY_VALUE         "propertyValue"
#define ACTION_PARAM_TIMEOUT                "timeOut"
#define ACTION_PARAM_PATTERN                "pattern"
#define ACTION_PARAM_CONNECTION_STATE       "connectionState"
#define ACTION_PARAM_CONNECTION_FORMAT      "connectionFormat"
#define ACTION_PARAM_MUTE_STATE             "muteState"
#define ACTION_PARAM_EXCEPT_SOURCE_NAME     "exceptSource"
#define ACTION_PARAM_EXCEPT_SINK_NAME       "exceptSink"
#define ACTION_PARAM_EXCEPT_CLASS_NAME      "exceptClass"
#define ACTION_PARAM_SOURCE_INFO            "sourceInfo"
#define ACTION_PARAM_SINK_INFO              "sinkInfo"
#define ACTION_PARAM_GATEWAY_INFO           "gatewayInfo"
#define ACTION_PARAM_SOURCE_STATE           "sourceState"
#define ACTION_PARAM_ELEMENT_TYPE           "elementType"
#define ACTION_PARAM_LIMIT_STATE            "limitState"
#define ACTION_PARAM_DEBUG_TYPE             "debugType"
#define ACTION_PARAM_DEBUG_VALUE            "debugValue"
#define ACTION_PARAM_LIMIT_TYPE             "limitType"
#define ACTION_PARAM_LIMIT_VOLUME           "limitVolume"
#define ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE    "notificationConfigurationType"
#define ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS  "notificationConfigurationStatus"
#define ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM    "notificationConfigurationParam"

#define ACTION_NAME_UNKNOWN             "ACTION_UNKNOWN"
#define ACTION_NAME_CONNECT             "ACTION_CONNECT"
#define ACTION_NAME_DISCONNECT          "ACTION_DISCONNECT"
#define ACTION_NAME_INTERRUPT           "ACTION_INTERRUPT"
#define ACTION_NAME_SUSPEND             "ACTION_SUSPEND"
#define ACTION_NAME_LIMIT               "ACTION_LIMIT"
#define ACTION_NAME_MUTE                "ACTION_MUTE"
#define ACTION_NAME_SET_VOLUME          "ACTION_SET_VOLUME"
#define ACTION_NAME_SET_PROPERTY        "ACTION_SET_PROPERTY"
#define ACTION_NAME_REGISTER            "ACTION_REGISTER"
#define ACTION_DEBUG                    "ACTION_DEBUG"
#define ACTION_NAME_SET_NOTIFICATION_CONFIGURATION  "ACTION_SET_NOTIFICATION_CONFIGURATION"

#define SYP_DEBUGLEVEL  1
enum gc_ActionList_e
{
    AL_NONE,
    // append at the end of Queue
    AL_NORMAL,
    // append after the currently executing action
    AL_SYSTEM

};
// For class Topology
enum gc_ClassTopologyCodeID_e
{
    MC_GENERAL_ELEMENT = 0,
    MC_SINK_ELEMENT,
    MC_SOURCE_ELEMENT,
    MC_GATEWAY_ELEMENT,
    MC_EQUAL_CODE,
    MC_EXCLUSIVE_CODE,
    MC_SHARED_CODE,
    MC_LBRACKET_CODE,
    MC_RBRACKET_CODE,
    MC_NULL_CODE
};

enum gc_Class_e
{
    C_UNKNOWN = 0,
    C_PLAYBACK,
    C_CAPTURE,
    C_MAX
};

enum gc_Trigger_e
{
    TRIGGER_UNKNOWN,
    USER_CONNECTION_REQUEST,
    USER_DISCONNECTION_REQUEST,
    USER_SET_SINK_MUTE_STATE,
    USER_SET_VOLUME,
    USER_SET_SINK_MAIN_SOUND_PROPERTY,
    USER_SET_SOURCE_MAIN_SOUND_PROPERTY,
    USER_SET_SYSTEM_PROPERTY,
    USER_SET_SINK_MAIN_NOTIFICATION_CONFIGURATION,
    USER_SET_SOURCE_MAIN_NOTIFICATION_CONFIGURATION,
    SYSTEM_SOURCE_AVAILABILITY_CHANGED,
    SYSTEM_SINK_AVAILABILITY_CHANGED,
    SYSTEM_INTERRUPT_STATE_CHANGED,
    SYSTEM_SINK_MUTE_STATE_CHANGED,
    SYSTEM_SINK_MAIN_SOUND_PROPERTY_CHANGED,
    SYSTEM_SOURCE_MAIN_SOUND_PROPERTY_CHANGED,
    SYSTEM_VOLUME_CHANGED,
    SYSTEM_SINK_NOTIFICATION_DATA_CHANGED,
    SYSTEM_SOURCE_NOTIFICATION_DATA_CHANGED,
    SYSTEM_REGISTER_DOMAIN,
    SYSTEM_REGISTER_SOURCE,
    SYSTEM_REGISTER_SINK,
    SYSTEM_REGISTER_GATEWAY,
    SYSTEM_DEREGISTER_DOMAIN,
    SYSTEM_DEREGISTER_SOURCE,
    SYSTEM_DEREGISTER_SINK,
    SYSTEM_DEREGISTER_GATEWAY,
    SYSTEM_DOMAIN_REGISTRATION_COMPLETE,
    SYSTEM_CONNECTION_STATE_CHANGE,
    USER_ALL_TRIGGER,
    TRIGGER_MAX
};

enum gc_Element_e
{
    ET_UNKNOWN,
    ET_SOURCE,
    ET_SINK,
    ET_ROUTE,
    ET_DOMAIN,
    ET_CLASS,
    ET_CONNECTION,
    ET_GATEWAY,
    ET_MAX
};

enum gc_Registration_e
{
    REG_NONE,
    REG_CONTROLLER,
    REG_ROUTER,
    REG_TEMPLATE
};

enum gc_LimitState_e
{
    LS_UNKNWON,
    LS_LIMITED,
    LS_UNLIMITED,
    LS_MAX
};

enum gc_Order_e
{
    O_HIGH_PRIORITY,
    O_LOW_PRIORITY,
    O_NEWEST,
    O_OLDEST
};

enum gc_LimitType_e
{
    LT_UNKNOWN,
    LT_RELATIVE,
    LT_ABSOLUTE,
    LT_MAX
};

struct gc_LimitVolume_s
{
    gc_LimitType_e limitType;
    am_volume_t limitVolume;
};

enum gc_MSPMappingDirection_e
{
    MD_MSP_TO_SP,
    MD_SP_TO_MSP,
    MD_BOTH
};

struct gc_Route_s : public am_Route_s
{
    std::string name;
};

struct gc_RoutingElement_s : public am_RoutingElement_s
{
    std::string name;
};

struct gc_TopologyElement_s
{
    std::string name;
    gc_ClassTopologyCodeID_e codeID;
};

struct gc_SoundProperty_s : public am_SoundProperty_s
{
    int16_t minValue;
    int16_t maxValue;
};

struct gc_MainSoundProperty_s : public am_MainSoundProperty_s
{
    int16_t minValue;
    int16_t maxValue;
};

struct gc_Source_s : public am_Source_s
{
    std::string domainName;
    std::string className;
    gc_Registration_e registrationType;
    bool isVolumeChangeSupported;
    int32_t priority;
    std::vector<gc_MainSoundProperty_s > listGCMainSoundProperties;
    std::vector<gc_SoundProperty_s > listGCSoundProperties;
    std::map<gc_MSPMappingDirection_e, std::map<uint16_t, uint16_t > > mapMSPTOSP;
};

struct gc_Sink_s : public am_Sink_s
{
    std::string domainName;
    std::string className;
    gc_Registration_e registrationType;
    bool isVolumeChangeSupported;
    int32_t priority;
    std::vector<gc_MainSoundProperty_s > listGCMainSoundProperties;
    std::vector<gc_SoundProperty_s > listGCSoundProperties;
    std::map<int16_t, float > mapUserVolumeToNormalizedVolume;
    std::map<float, float > mapNormalizedVolumeToDecibelVolume;
    std::map<gc_MSPMappingDirection_e, std::map<uint16_t, uint16_t > > mapMSPTOSP;
};

struct gc_Gateway_s : public am_Gateway_s
{
    std::string sinkName;
    std::string sourceName;
    std::string controlDomainName;
    gc_Registration_e registrationType;
    std::vector<std::pair<am_CustomConnectionFormat_t, am_CustomConnectionFormat_t > > listConvertionmatrix;
};

struct gc_Domain_s : public am_Domain_s
{
    gc_Registration_e registrationType;
};

struct gc_Class_s
{
    std::string name;
    gc_Class_e type;
    int32_t priority;
    std::vector<gc_TopologyElement_s > listTopologies;
    gc_Registration_e registrationType;
};

struct gc_Action_s
{
    std::string actionName;
    std::map<std::string, std::string > mapParameters;
};

struct gc_ConnectionInfo_s
{
    std::string sourceName;
    std::string sinkName;
    int32_t priority;
    am_ConnectionState_e connectionState;
    am_volume_t volume;
};

} /* namespace gc */
} /* namespace am */

#endif /* GC_TYPES_H_ */
