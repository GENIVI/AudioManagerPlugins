/**************************************************************************//**
 * @file CAmTypes.h
 *
 * This file contains the declaration of all the constants and enum shared
 * between framework and policy engine.
 *
 * @component{AudioManager Generic Controller}
 *
 * @authors Toshiaki Isogai <tisogai@jp.adit-jv.com>,\n
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>,\n
 *          Prashant Jain   <pjain@jp.adit-jv.com>,\n
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 - 2019 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 *//**************************************************************************/

#ifndef GC_TYPES_H_
#define GC_TYPES_H_

#include "IAmControlCommon.h"
#include <set>
#include <memory>

namespace am {

/// Container to host all modules composing the generic controller plug-in
/// for the GenIVI Audio-Manager
namespace gc {

#define DEFAULT_UNDO_TIMEOUT        (10000)
#define INFINITE_TIMEOUT            (0xFFFFFFFF)
#define DEFAULT_RAMP_TIME           (200)
/// maximum duration (in milliseconds) an action execution may take before an error action is started.
#define DEFAULT_ASYNC_ACTION_TIME   (5000)   
#define DEFAULT_RAMP_TYPE           (RAMP_GENIVI_DIRECT)
#define DEFAULT_OFFSET_VOLUME       (0)
#define DEFAULT_LIMIT_PATTERN       (0xFFFFFFFF)
#define E_WAIT_FOR_CHILD_COMPLETION (-1)
#define TOPOLOGY_SYMBOL_ASTERISK    "*"

// supported parameters of actions
#define ACTION_PARAM_CLASS_NAME                        "className"
#define ACTION_PARAM_CONNECTION_NAME                   "connectionName"
#define ACTION_PARAM_SOURCE_NAME                       "sourceName"
#define ACTION_PARAM_SINK_NAME                         "sinkName"
#define ACTION_PARAM_GATEWAY_NAME                      "gatewayName"
#define ACTION_PARAM_RAMP_TYPE                         "rampType"
#define ACTION_PARAM_RAMP_TIME                         "rampTime"
#define ACTION_PARAM_MAIN_VOLUME_STEP                  "mainVolumeStep"
#define ACTION_PARAM_MAIN_VOLUME                       "mainVolume"
#define ACTION_PARAM_VOLUME_STEP                       "volumeStep"
#define ACTION_PARAM_VOLUME                            "volume"
#define ACTION_PARAM_ORDER                             "order"
#define ACTION_PARAM_PROPERTY_TYPE                     "propertyType"
#define ACTION_PARAM_PROPERTY_VALUE                    "propertyValue"
#define ACTION_PARAM_TIMEOUT                           "timeOut"
#define ACTION_PARAM_PATTERN                           "pattern"
#define ACTION_PARAM_CONNECTION_STATE                  "connectionState"
#define ACTION_PARAM_CONNECTION_FORMAT                 "connectionFormat"
#define ACTION_PARAM_MUTE_STATE                        "muteState"
#define ACTION_PARAM_EXCEPT_SOURCE_NAME                "exceptSource"
#define ACTION_PARAM_EXCEPT_SINK_NAME                  "exceptSink"
#define ACTION_PARAM_EXCEPT_CLASS_NAME                 "exceptClass"
#define ACTION_PARAM_SOURCE_INFO                       "sourceInfo"
#define ACTION_PARAM_SINK_INFO                         "sinkInfo"
#define ACTION_PARAM_GATEWAY_INFO                      "gatewayInfo"
#define ACTION_PARAM_SOURCE_STATE                      "sourceState"
#define ACTION_PARAM_ELEMENT_TYPE                      "elementType"
#define ACTION_PARAM_LIMIT_STATE                       "limitState"
#define ACTION_PARAM_DEBUG_TYPE                        "debugType"
#define ACTION_PARAM_DEBUG_VALUE                       "debugValue"
#define ACTION_PARAM_LIMIT_TYPE                        "limitType"
#define ACTION_PARAM_LIMIT_VOLUME                      "limitVolume"
#define ACTION_PARAM_LIMIT_MAP                         "mapLimits"
#define ACTION_PARAM_SAVE_LAST_VOLUME                  "saveLastVolume"
#define ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE   "notificationConfigurationType"
#define ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS "notificationConfigurationStatus"
#define ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM  "notificationConfigurationParam"
#define ACTION_PARAM_SET_SOURCE_STATE_DIRECTION        "setSourceStateDirection"
#define ACTION_PARAM_LIST_PROPERTY                     "listMainSoundProperties"
#define ACTION_PARAM_LIST_SYSTEM_PROPERTIES            "listSystemProperties"

#define SYSTEM_ELEMENT_NAME "System"
#define DEFAULT_CLASS_NAME  "default"
#define AM_VOLUME_NO_LIMIT  0
#define CLASS_ID_BASE       101
/*
 * These system properties are reserved for controllers use only. The numbers are allocated using
 * following rules
 * - system property occupies the last 4096 entries of the range i,e from 61440 to 65535
 * - 12 bit System property is made up of 5 bit use case ID and 7 bit system property
 *  In other words it can be calculated as
 *  61440 + 128*X + Y, where 61440 is the reserved system property base
 *                           X is the use case ID, and
 *                           Y is the system property.
 *
 * The use case ID are defined as below
 * 0 -> Global.
 * 1 -> Registration/Unregistration use case
 * 2 -> connect/disconnect use case.
 * 3 -> set Volume use case.
 * 4 -> set Sound property use case
 * 5 -> set system property use case.
 * 6 -> update use case.
 * 7 -> notification configuration use case.
 * 8 -> Related to action execution.
 */
#define RESERVED_PROPERTIES_BASE   ((1 << 16) - (1 << 12))
#define PROPERTY_USE_CASE_ID_SHIFT 7

#define GLOBAL_PROPERTY_BASE       RESERVED_PROPERTIES_BASE + (0 << PROPERTY_USE_CASE_ID_SHIFT)
#define REGISTRATION_PROPERTY_BASE RESERVED_PROPERTIES_BASE + (1 << PROPERTY_USE_CASE_ID_SHIFT)
#define CONNECTION_PROPERTY_BASE   RESERVED_PROPERTIES_BASE + (2 << PROPERTY_USE_CASE_ID_SHIFT)
#define VOLUME_PROPERTY_BASE       RESERVED_PROPERTIES_BASE + (3 << PROPERTY_USE_CASE_ID_SHIFT)
#define SOUND_PROP_PROPERTY_BASE   RESERVED_PROPERTIES_BASE + (4 << PROPERTY_USE_CASE_ID_SHIFT)
#define SYS_PROPERTY_PROPERTY_BASE RESERVED_PROPERTIES_BASE + (5 << PROPERTY_USE_CASE_ID_SHIFT)
#define UPDATE_PROPERTY_BASE       RESERVED_PROPERTIES_BASE + (6 << PROPERTY_USE_CASE_ID_SHIFT)
#define NOTIFICATION_CONFIGURARION_PROPERTY_BASE \
    RESERVED_PROPERTIES_BASE + (7 << PROPERTY_USE_CASE_ID_SHIFT)
#define ACTION_PROPERTY_BASE RESERVED_PROPERTIES_BASE + (8 << PROPERTY_USE_CASE_ID_SHIFT)
static const am_CustomSystemPropertyType_t SYP_GLOBAL_LOG_THRESHOLD               = GLOBAL_PROPERTY_BASE + 0;
static const am_CustomSystemPropertyType_t SYP_REGISTRATION_ALLOW_UNKNOWN_ELEMENT = \
    REGISTRATION_PROPERTY_BASE + 0;
static const am_CustomSystemPropertyType_t SYP_REGISTRATION_DOMAIN_TIMEOUT = \
    REGISTRATION_PROPERTY_BASE + 1;
static const am_CustomSystemPropertyType_t SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES = \
    CONNECTION_PROPERTY_BASE + 0;

static const am_CustomClassProperty_t CP_PER_SINK_CLASS_VOLUME_SUPPORT = \
    GLOBAL_PROPERTY_BASE + 0;
#define MSP_SINK_PER_CLASS_VOLUME_TYPE(CLASS_ID) VOLUME_PROPERTY_BASE - CLASS_ID_BASE + CLASS_ID

static const am_CustomSystemPropertyType_t SYP_REGSTRATION_SOUND_PROP_RESTORED =
    REGISTRATION_PROPERTY_BASE + 2;

/***************************************************************************//**
 * @name Enumeration Types
 *
 *//**@{*/
/**
 *   Types of implemented actions
 * 
 *   For details please refer to page @ref pageActions.
 */
// Note: Order must match corresponding enumeration in XML schema audiomanagertypes.xsd
enum gc_Action_e
{
    ACTION_UNKNOWN,
    ACTION_CONNECT,                      ///< Establish a new or reactivate an existing connection 
    ACTION_DISCONNECT,                   ///< Disconnect source from sink and destroy route segments
    ACTION_TRANSFER,                     ///< Offer active main connections to surviving domains
    ACTION_SET_VOLUME,                   ///< Adjust nominal sound level to given value.
    ACTION_LIMIT,                        ///< Install a maximum value which the sound level may not exceed 
    ACTION_UNLIMIT,                      ///< Release previously installed sound level limitations 
    ACTION_PUSH,                         ///< Disconnect source from sink but retain the connection details on stack
    ACTION_POP,                          ///< Re-activate previously 'pushed' connection from stack. 
    ACTION_SUSPEND,                      ///< Pause streaming, but keep the connection intact 
    ACTION_RESUME,                       ///< Continue previously paused streaming on given connection 
    ACTION_MUTE,                         ///< Reduce sound level to inaudibility 
    ACTION_UNMUTE,                       ///< Make sound audible again 
    ACTION_SET_PROPERTY,                 ///< Modify a sound property of a source or sink 
    ACTION_SET_PROPERTIES,               ///< Modify multiple sound properties at once 
    ACTION_REGISTER,                     ///< Auto-register sources, sinks and/or gateways to the AM 
    ACTION_DEBUG,                        ///< Legacy alias for ACTION_SET_SYSTEM_PROPERTY 
    ACTION_SET_SYSTEM_PROPERTY,          ///< Manipulate a global attribute
    ACTION_SET_SYSTEM_PROPERTIES,        ///< Manipulate multiple global attributes at once
    ACTION_SET_NOTIFICATION_CONFIGURATION///< Specify conditions when sources and / or sinks fire notification events
};

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_Action_e &type)
{
    switch (type)
    {
        case ACTION_UNKNOWN:
            return out << "ACTION_UNKNOWN";
        case ACTION_CONNECT:
            return out << "ACTION_CONNECT";
        case ACTION_DISCONNECT:
            return out << "ACTION_DISCONNECT";
        case ACTION_PUSH:
            return out << "ACTION_PUSH";
        case ACTION_POP:
            return out << "ACTION_POP";
        case ACTION_SUSPEND:
            return out << "ACTION_SUSPEND";
        case ACTION_RESUME:
            return out << "ACTION_RESUME";
        case ACTION_LIMIT:
            return out << "ACTION_LIMIT";
        case ACTION_UNLIMIT:
            return out << "ACTION_UNLIMIT";
        case ACTION_MUTE:
            return out << "ACTION_MUTE";
        case ACTION_UNMUTE:
            return out << "ACTION_UNMUTE";
        case ACTION_SET_VOLUME:
            return out << "ACTION_SET_VOLUME";
        case ACTION_SET_PROPERTY:
            return out << "ACTION_SET_PROPERTY";
        case ACTION_SET_PROPERTIES:
            return out << "ACTION_SET_PROPERTIES";
        case ACTION_REGISTER:
            return out << "ACTION_REGISTER";
        case ACTION_DEBUG:
            return out << "ACTION_DEBUG";
        case ACTION_SET_SYSTEM_PROPERTY:
            return out << "ACTION_SET_SYSTEM_PROPERTY";
        case ACTION_SET_SYSTEM_PROPERTIES:
            return out << "ACTION_SET_SYSTEM_PROPERTIES";
        case ACTION_SET_NOTIFICATION_CONFIGURATION:
            return out << "ACTION_SET_NOTIFICATION_CONFIGURATION";
        default:
            return out << "undefined ACTION type(" << (int)type << ")";
    }
}

enum gc_SetSourceStateDirection_e
{
    SD_MAINSOURCE_TO_MAINSINK,
    SD_MAINSINK_TO_MAINSOURCE
};

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_SetSourceStateDirection_e &dir)
{
    switch (dir)
    {
    case SD_MAINSOURCE_TO_MAINSINK:
        return out << "SD_MAINSOURCE_TO_MAINSINK";
    case SD_MAINSINK_TO_MAINSOURCE:
        return out << "SD_MAINSINK_TO_MAINSOURCE";
    default:
        return out << "SD_undefined(" << (int)dir << ")";
    }
}

enum gc_ActionList_e
{
    AL_NONE,
    // append at the end of Queue
    AL_NORMAL,
    // append after the currently executing action
    AL_SYSTEM

};

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_ActionList_e &al)
{
    switch (al)
    {
    case AL_NONE:
        return out << "AL_NONE";
    case AL_NORMAL:
        return out << "AL_NORMAL";
    case AL_SYSTEM:
        return out << "AL_SYSTEM";
    default:
        return out << "AL_undefined(" << (int)al << ")";
    }
}

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

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_ClassTopologyCodeID_e &t)
{
    switch (t)
    {
    case MC_GENERAL_ELEMENT:
        return out << "MC_GENERAL_ELEMENT";
    case MC_SINK_ELEMENT:
        return out << "MC_SINK_ELEMENT";
    case MC_SOURCE_ELEMENT:
        return out << "MC_SOURCE_ELEMENT";
    case MC_GATEWAY_ELEMENT:
        return out << "MC_GATEWAY_ELEMENT";
    case MC_EQUAL_CODE:
        return out << "MC_EQUAL_CODE";
    case MC_EXCLUSIVE_CODE:
        return out << "MC_EXCLUSIVE_CODE";
    case MC_SHARED_CODE:
        return out << "MC_SHARED_CODE";
    case MC_LBRACKET_CODE:
        return out << "MC_LBRACKET_CODE";
    case MC_RBRACKET_CODE:
        return out << "MC_RBRACKET_CODE";
    case MC_NULL_CODE:
        return out << "MC_NULL_CODE";
    default:
        return out << "MC_undefined(" << (int)t << ")";
    }
}

/// Type of class element
enum gc_Class_e
{
    C_UNKNOWN = 0,
    C_PLAYBACK,  ///< Class suitable for audio-emitting ports
    C_CAPTURE,   ///< Class suitable for audio-accepting ports
    C_MAX
};

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_Class_e &c)
{
    switch (c)
    {
    case C_UNKNOWN:
        return out << "C_UNKNOWN";
    case C_PLAYBACK:
        return out << "C_PLAYBACK";
    case C_CAPTURE:
        return out << "C_CAPTURE";
    case C_MAX:
        return out << "C_MAX";
    default:
        return out << "C_undefined(" << (int)c << ")";
    }
}

/// Trigger identifier
enum gc_Trigger_e
{
    TRIGGER_UNKNOWN,

    // Command-side requests
    USER_CONNECTION_REQUEST,                         ///< connect() requested from command interface
    USER_DISCONNECTION_REQUEST,                      ///< disconnect() requested from command interface
    USER_SET_SINK_MUTE_STATE,                        ///< setSinkMuteState() requested from command interface
    USER_SET_VOLUME,                                 ///< setVolume() or volumeStep() requested from command interface
    USER_SET_SINK_MAIN_SOUND_PROPERTY,               ///< setMainSinkSoundProperty() requested from command interface
    USER_SET_SINK_MAIN_SOUND_PROPERTIES,             ///< setMainSinkSoundProperties() requested from command interface
    USER_SET_SOURCE_MAIN_SOUND_PROPERTY,             ///< setMainSourceSoundProperty() requested from command interface
    USER_SET_SOURCE_MAIN_SOUND_PROPERTIES,           ///< setMainSourceSoundProperties() requested from command interface
    USER_SET_SYSTEM_PROPERTY,                        ///< setSystemProperty() requested from command interface
    USER_SET_SYSTEM_PROPERTIES,                      ///< setSystemProperties() requested from command interface
    USER_SET_SINK_MAIN_NOTIFICATION_CONFIGURATION,   ///< setMainSinkNotificationConfiguration() requested from command interface
    USER_SET_SOURCE_MAIN_NOTIFICATION_CONFIGURATION, ///< setMainSourceNotificationConfiguration() requested from command interface

    // Routing-side notifications
    SYSTEM_SOURCE_AVAILABILITY_CHANGED,              ///< a source has become available / unavailable
    SYSTEM_SINK_AVAILABILITY_CHANGED,                ///< a sink has become available / unavailable
    SYSTEM_INTERRUPT_STATE_CHANGED,                  ///< a source changed to interrupted / streaming state
    SYSTEM_SINK_MUTE_STATE_CHANGED,                  // never fired
    SYSTEM_SINK_MAIN_SOUND_PROPERTY_CHANGED,         // never fired
    SYSTEM_SOURCE_MAIN_SOUND_PROPERTY_CHANGED,       // never fired
    SYSTEM_VOLUME_CHANGED,                           // never fired
    SYSTEM_SINK_NOTIFICATION_DATA_CHANGED,           ///< notification data of a sink has changed
    SYSTEM_SOURCE_NOTIFICATION_DATA_CHANGED,         ///< notification data of a source has changed

    // Routing-side requests, successfully processed
    SYSTEM_REGISTER_DOMAIN,                          ///< announced a domain to database
    SYSTEM_REGISTER_SOURCE,                          ///< announced a source to database
    SYSTEM_REGISTER_SINK,                            ///< announced a sink to database
    SYSTEM_REGISTER_GATEWAY,                         ///< announced a gateway to database
    SYSTEM_REGISTER_EARLY_CONNECTION,                ///< announced a main connection pre-established from routing side
    SYSTEM_DEREGISTER_DOMAIN,                        ///< removed a domain from database
    SYSTEM_DEREGISTER_SOURCE,                        ///< removed a source from database
    SYSTEM_DEREGISTER_SINK,                          ///< removed a sink from database
    SYSTEM_DEREGISTER_GATEWAY,                       ///< removed a gateway from database

    // Miscellaneous notifications
    SYSTEM_DOMAIN_REGISTRATION_COMPLETE,             ///< all children (sources, sinks, gateways) are announced to database
    SYSTEM_CONNECTION_STATE_CHANGE,                  ///< a connection has changed its connection state
    SYSTEM_STORED_SINK_VOLUME,                       ///< the volume of a connection is stored to persistence
    SYSTEM_ALL_DOMAIN_REGISTRATION_COMPLETE,         ///< all configured domains have completed their registration
    USER_ALL_TRIGGER,                                ///< wild-card to any of the above
    TRIGGER_MAX
};

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_Trigger_e &t)
{
    switch (t)
    {
    case TRIGGER_UNKNOWN:
        return out << "TRIGGER_UNKNOWN";
    case USER_CONNECTION_REQUEST:
        return out << "USER_CONNECTION_REQUEST";
    case USER_DISCONNECTION_REQUEST:
        return out << "USER_DISCONNECTION_REQUEST";
    case USER_SET_SINK_MUTE_STATE:
        return out << "USER_SET_SINK_MUTE_STATE";
    case USER_SET_VOLUME:
        return out << "USER_SET_VOLUME";
    case USER_SET_SINK_MAIN_SOUND_PROPERTY:
        return out << "USER_SET_SINK_MAIN_SOUND_PROPERTY";
    case USER_SET_SINK_MAIN_SOUND_PROPERTIES:
        return out << "USER_SET_SINK_MAIN_SOUND_PROPERTIES";
    case USER_SET_SOURCE_MAIN_SOUND_PROPERTY:
        return out << "USER_SET_SOURCE_MAIN_SOUND_PROPERTY";
    case USER_SET_SOURCE_MAIN_SOUND_PROPERTIES:
        return out << "USER_SET_SOURCE_MAIN_SOUND_PROPERTIES";
    case USER_SET_SYSTEM_PROPERTY:
        return out << "USER_SET_SYSTEM_PROPERTY";
    case USER_SET_SYSTEM_PROPERTIES:
        return out << "USER_SET_SYSTEM_PROPERTIES";
    case USER_SET_SINK_MAIN_NOTIFICATION_CONFIGURATION:
        return out << "USER_SET_SINK_MAIN_NOTIFICATION_CONFIGURATION";
    case USER_SET_SOURCE_MAIN_NOTIFICATION_CONFIGURATION:
        return out << "USER_SET_SOURCE_MAIN_NOTIFICATION_CONFIGURATION";
    case SYSTEM_SOURCE_AVAILABILITY_CHANGED:
        return out << "SYSTEM_SOURCE_AVAILABILITY_CHANGED";
    case SYSTEM_SINK_AVAILABILITY_CHANGED:
        return out << "SYSTEM_SINK_AVAILABILITY_CHANGED";
    case SYSTEM_INTERRUPT_STATE_CHANGED:
        return out << "SYSTEM_INTERRUPT_STATE_CHANGED";
    case SYSTEM_SINK_MUTE_STATE_CHANGED:
        return out << "SYSTEM_SINK_MUTE_STATE_CHANGED";
    case SYSTEM_SINK_MAIN_SOUND_PROPERTY_CHANGED:
        return out << "SYSTEM_SINK_MAIN_SOUND_PROPERTY_CHANGED";
    case SYSTEM_SOURCE_MAIN_SOUND_PROPERTY_CHANGED:
        return out << "SYSTEM_SOURCE_MAIN_SOUND_PROPERTY_CHANGED";
    case SYSTEM_VOLUME_CHANGED:
        return out << "SYSTEM_VOLUME_CHANGED";
    case SYSTEM_SINK_NOTIFICATION_DATA_CHANGED:
        return out << "SYSTEM_SINK_NOTIFICATION_DATA_CHANGED";
    case SYSTEM_SOURCE_NOTIFICATION_DATA_CHANGED:
        return out << "SYSTEM_SOURCE_NOTIFICATION_DATA_CHANGED";
    case SYSTEM_REGISTER_DOMAIN:
        return out << "SYSTEM_REGISTER_DOMAIN";
    case SYSTEM_REGISTER_SOURCE:
        return out << "SYSTEM_REGISTER_SOURCE";
    case SYSTEM_REGISTER_SINK:
        return out << "SYSTEM_REGISTER_SINK";
    case SYSTEM_REGISTER_GATEWAY:
        return out << "SYSTEM_REGISTER_GATEWAY";
    case SYSTEM_DEREGISTER_DOMAIN:
        return out << "SYSTEM_DEREGISTER_DOMAIN";
    case SYSTEM_DEREGISTER_SOURCE:
        return out << "SYSTEM_DEREGISTER_SOURCE";
    case SYSTEM_DEREGISTER_SINK:
        return out << "SYSTEM_DEREGISTER_SINK";
    case SYSTEM_DEREGISTER_GATEWAY:
        return out << "SYSTEM_DEREGISTER_GATEWAY";
    case SYSTEM_DOMAIN_REGISTRATION_COMPLETE:
        return out << "SYSTEM_DOMAIN_REGISTRATION_COMPLETE";
    case SYSTEM_CONNECTION_STATE_CHANGE:
        return out << "SYSTEM_CONNECTION_STATE_CHANGE";
    case SYSTEM_STORED_SINK_VOLUME:
        return out << "SYSTEM_STORED_SINK_VOLUME";
    case SYSTEM_ALL_DOMAIN_REGISTRATION_COMPLETE:
        return out << "SYSTEM_ALL_DOMAIN_REGISTRATION_COMPLETE";
    case USER_ALL_TRIGGER:
        return out << "USER_ALL_TRIGGER";
    case TRIGGER_MAX:
        return out << "TRIGGER_MAX";
    default:
        return out << "TRIGGER_undefined(" << (int)t << ")";
    }
}

/// Type of topology element
enum gc_Element_e
{
    ET_UNKNOWN,
    ET_SOURCE,     ///< Audio-emitting port element
    ET_SINK,       ///< Audio-accepting port element
    ET_ROUTE,      ///< Partial audio connection element
    ET_DOMAIN,     ///< Environment for sources and/or sinks
    ET_CLASS,      ///< Collection of attributes common to multiple sources and/or sinks
    ET_CONNECTION, ///< Complete audio connection element, consisting of one or more route elements
    ET_GATEWAY,    ///< Part of connection element, used to span multiple domains
    ET_SYSTEM,     ///< Global attribute element
    ET_MAX
};

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_Element_e &s)
{
    switch (s)
    {
    case ET_UNKNOWN:
        return out << "ET_UNKNOWN";
    case ET_SOURCE:
        return out << "ET_SOURCE";
    case ET_SINK:
        return out << "ET_SINK";
    case ET_ROUTE:
        return out << "ET_ROUTE";
    case ET_DOMAIN:
        return out << "ET_DOMAIN";
    case ET_CLASS:
        return out << "ET_CLASS";
    case ET_CONNECTION:
        return out << "ET_CONNECTION";
    case ET_GATEWAY:
        return out << "ET_GATEWAY";
    case ET_SYSTEM:
        return out << "ET_SYSTEM";
    case ET_MAX:
        return out << "ET_MAX";
    default:
        return out << "ET_undefined(" << (int)s << ")";
    }
}

/// Specification whether the controller or the routing plug-in is responsible to register an element
enum gc_Registration_e
{
    REG_NONE,       ///< Not to be registered
    REG_CONTROLLER, ///< Controller shall register
    REG_ROUTER,     ///< Routing-side shall register
};

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_Registration_e &reg)
{
    switch (reg)
    {
    case REG_NONE:
        return out << "REG_NONE";
    case REG_CONTROLLER:
        return out << "REG_CONTROLLER";
    case REG_ROUTER:
        return out << "REG_ROUTER";
    default:
        return out << "REG_undefined(" << (int)reg << ")";
    }
}

///  Current or requested volume limit state
enum gc_LimitState_e
{
    LS_UNKNWON,
    LS_LIMITED,   ///< one or more limitations are active
    LS_UNLIMITED, ///< not limited
    LS_MAX
};

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_LimitState_e &ls)
{
    switch (ls)
    {
    case LS_UNKNWON:
        return out << "LS_UNKNWON";
    case LS_LIMITED:
        return out << "LS_LIMITED";
    case LS_UNLIMITED:
        return out << "LS_UNLIMITED";
    case LS_MAX:
        return out << "LS_MAX";
    default:
        return out << "LS_undefined(" << (int)ls << ")";
    }
}

/// Criterion for selecting the next inactive connection for re-activation
enum gc_Order_e
{
    O_HIGH_PRIORITY, ///< Sorted by priority attribute, highest value first
    O_LOW_PRIORITY,  ///< Sorted by priority attribute, lowest value first
    O_NEWEST,        ///< Sorted like a stack, last in - first out
    O_OLDEST         ///< Sorted like a queue, first in - first out.
};

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_Order_e &o)
{
    switch (o)
    {
    case O_HIGH_PRIORITY:
        return out << "O_HIGH_PRIORITY";
    case O_LOW_PRIORITY:
        return out << "O_LOW_PRIORITY";
    case O_NEWEST:
        return out << "O_NEWEST";
    case O_OLDEST:
        return out << "O_OLDEST";
    default:
        return out << "O_undefined(" << (int)o << ")";
    }
}

/// Specification whether a volume limit is meant as absolute maximum
/// or relative to the volume of another active connection
enum gc_LimitType_e
{
    LT_UNKNOWN,
    LT_RELATIVE, ///< Audible volume to be balanced against other connection volumes
    LT_ABSOLUTE, ///< Audible volume limited by constant value
    LT_MAX
};

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_LimitType_e &lt)
{
    switch (lt)
    {
    case LT_UNKNOWN:
        return out << "LT_UNKNOWN";
    case LT_RELATIVE:
        return out << "LT_RELATIVE";
    case LT_ABSOLUTE:
        return out << "LT_ABSOLUTE";
    case LT_MAX:
        return out << "LT_MAX";
    default:
        return out << "LT_undefined(" << (int)lt << ")";
    }
}

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

// Resolve numeric values to symbolic names for logging
inline std::ostream &operator<<(std::ostream &out, const gc_MSPMappingDirection_e &md)
{
    switch (md)
    {
    case MD_MSP_TO_SP:
        return out << "MD_MSP_TO_SP";
    case MD_SP_TO_MSP:
        return out << "MD_SP_TO_MSP";
    case MD_BOTH:
        return out << "MD_BOTH";
    default:
        return out << "MD_undefined(" << (int)md << ")";

    }
}

/**@}*/

struct gc_SystemProperty_s : public am_SystemProperty_s
{
    bool isPersistenceSupported;
};

/***************************************************************************//**
 * @name Composite Types (Structures)
 *
 *//**@{*/

struct gc_System_s
{
    bool readOnly;
    std::string name;
    std::vector<gc_SystemProperty_s > listGCSystemProperties;
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
    gc_TopologyElement_s(gc_ClassTopologyCodeID_e elementCode, const std::string &elementName = std::string(""));
    gc_TopologyElement_s(gc_ClassTopologyCodeID_e elementCode,
        const std::string &elementName, size_t pos,
        size_t length = std::string::npos);
    gc_TopologyElement_s(const std::string &elementName);
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
    bool isPersistenceSupported;
};

/// @name Initializers for topology elements
/**@{*/

/// Common initializer for source and sink elements
struct gc_NodePoint_s
{
    inline gc_NodePoint_s()
        : registrationType(REG_NONE)
        , domainName("")
        , className("")
        , isVolumeChangeSupported(false)
        , isPersistencySupported(false)
        , isVolumePersistencySupported(false)
        , priority(0)
    {
    }
    std::string domainName;
    std::string className;
    gc_Registration_e registrationType;
    bool isVolumeChangeSupported;
    bool isPersistencySupported;
    bool isVolumePersistencySupported;
    int32_t priority;
    std::map<float, float > mapRoutingVolume;
    std::vector<gc_MainSoundProperty_s > listGCMainSoundProperties;
    std::vector<gc_SoundProperty_s > listGCSoundProperties;
    std::map<gc_MSPMappingDirection_e, std::map<uint16_t, uint16_t > > mapMSPTOSP;
};

/// initializer for a source element
struct gc_Source_s : public am_Source_s, public gc_NodePoint_s
{
    inline gc_Source_s()
        : minVolume(-3000)
        , maxVolume(0)
    {
        ///initializing the am_Source_s variables
        sourceID = 0;
        domainID = 0;
        name = "";
        sourceClassID = 0;
        sourceState = SS_UNKNNOWN;
        volume = 0;
        visible = false;
        available = {A_UNKNOWN, 0};
        interruptState = IS_UNKNOWN;
    }
    am_volume_t minVolume;
    am_volume_t maxVolume;
};

/// Initializer for a sink element
struct gc_Sink_s : public am_Sink_s, public gc_NodePoint_s
{
    inline gc_Sink_s()
    {
        ///initializing the am_Sink_s variables
        sinkID = 0;
        name = "";
        domainID = 0;
        sinkClassID = 0;
        volume = 0;
        visible = false;
        available = {A_UNKNOWN, 0};
        muteState = MS_UNKNOWN;
        mainVolume = 0;
    }
    std::map<float, float > mapUserVolumeToNormalizedVolume;
    std::map<float, float > mapNormalizedVolumeToDecibelVolume;
};

/// Initializer for a gateway element
struct gc_Gateway_s : public am_Gateway_s
{
    std::string sinkName;
    std::string sourceName;
    std::string controlDomainName;
    gc_Registration_e registrationType;
    std::vector<std::pair<am_CustomConnectionFormat_t, am_CustomConnectionFormat_t > > listConvertionmatrix;
};

/// Initializer for a domain element
struct gc_Domain_s : public am_Domain_s
{
    gc_Registration_e registrationType;
};

/// Initializer for a class element
struct gc_Class_s
{
    uint16_t classID;
    std::string name;
    gc_Class_e type;
    int32_t priority;
    std::vector<std::vector<gc_TopologyElement_s > > listTopologies;
    std::vector<am_ClassProperty_s> listClassProperties;
    bool isVolumePersistencySupported;
    am_volume_t defaultVolume;
};
/**@}*/

struct gc_Action_s
{
    gc_Action_e                         actionType;
    std::map<std::string, std::string > mapParameters;
};

// serialize members for logging
inline std::ostream &operator<<(std::ostream &out, const gc_Action_s &action)
{
    out << action.actionType << ":{";
    int cnt = 0;
    for (auto &param : action.mapParameters)
    {
        out << (cnt++ ? ", " : "") << param.first << ":" << param.second;
    }

    return out << "}" << std::endl;
}

struct gc_ConnectionInfo_s
{
    std::string sourceName;
    std::string sinkName;
    std::string connectionName;
    int32_t priority;
    am_ConnectionState_e connectionState;
    am_volume_t volume;
    gc_Element_e elementType;

    gc_ConnectionInfo_s()
        : priority(0)
        , connectionState(CS_UNKNOWN)
        , volume(0)
        , elementType(ET_UNKNOWN)
    {
    }

    gc_ConnectionInfo_s(gc_Element_e type, const std::string &source, const std::string &sink)
        : sourceName(source)
        , sinkName(sink)
        , connectionName((!source.empty() && !sink.empty()) ? std::string(source) + ":" + sink : "")
        , priority(0)
        , connectionState(CS_UNKNOWN)
        , volume(0)
        , elementType(type)
    {
    }

    gc_ConnectionInfo_s(const gc_Element_e &type, const std::string &name)
        : priority(0)
        , connectionState(CS_UNKNOWN)
        , volume(0)
        , elementType(type)
    {
        switch (elementType)
        {
        case ET_SOURCE:
            sourceName = name;
            break;
        case ET_SINK:
            sinkName = name;
            break;
        case ET_CONNECTION:
            connectionName = name;
            break;
        default:
            break;
        }
    }

};

struct gc_ElementTypeName_s
{

public:
    /**
     * the element type
     */
    gc_Element_e elementType;

    /**
     * the element name
     */
    std::string elementName;

};

struct gc_ElementTypeID_s
{

public:
    /**
     * the element type
     */
    gc_Element_e elementType;
    /**
     * the element name
     */
    uint16_t elementID;

};

struct gc_LastMainConnections_s
{
public:
    std::string className;
    std::set<std::string> listMainConnections;
};

struct gc_LastMainSoundProperty_s
{
public:
    gc_ElementTypeName_s elementInfo;
    std::vector<am_MainSoundProperty_s> listLastMainSoundProperty;
};

struct gc_LastMainSoundProperties_s
{
public:
    std::string className;
    std::vector<gc_LastMainSoundProperty_s> listLastMainSoundProperties;
};

struct gc_SinkVolume_s
{
public:
    std::string sinkName;
    am_mainVolume_t mainVolume;
};

struct gc_LastClassVolume_s
{
public:
    std::string className;
    std::vector<gc_SinkVolume_s> listSinkVolume;
};

struct gc_LastMainConVolInfo_s
{
public:
    std::string mainConnectionName;
    am_mainVolume_t mainVolume;
};

struct gc_LastMainConnectionsVolume_s
{
public:
    std::string className;
    std::vector<gc_LastMainConVolInfo_s> listLastMainConVolInfo;
};

} /* namespace gc */
} /* namespace am */

/**@}*/

#endif /* GC_TYPES_H_ */
