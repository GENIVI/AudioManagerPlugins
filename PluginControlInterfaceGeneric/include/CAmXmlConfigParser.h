/**************************************************************************//**
 * @file CAmXmlConfigParser.h
 *
 * This class reads the system configuration specified as XML document. It validates the
 * document against the schema and parses the default and user configuration.
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

#ifndef GC_XMLCONFIGPARSER_H_
#define GC_XMLCONFIGPARSER_H_

#include "CAmXmlParserCommon.h"

namespace am
{
namespace gc
{

#ifndef GENERIC_CONTROLLER_DEFAULT_CONF_ROOT
# define GENERIC_CONTROLLER_DEFAULT_CONF_ROOT "/etc/audiomanager/controller"
#endif

#define DEFAULT_USER_CONFIGURATION_PATH GENERIC_CONTROLLER_DEFAULT_CONF_ROOT "/generic.xml"
#define DEFAULT_SCHEMA_PATH             GENERIC_CONTROLLER_DEFAULT_CONF_ROOT "/audiomanagertypes.xsd"
#define DEFAULT_CUSTOM_SCHEMA_PATH      GENERIC_CONTROLLER_DEFAULT_CONF_ROOT "/customtypes.xsd"
#define CONFIGURATION_FILE_ENV_VARNAME  "GENERIC_CONTROLLER_CONFIGURATION"

// sysmbols used in topology to seperate elements
#define TOPOLOGY_SYMBOL_CARET         "^"
#define TOPOLOGY_SYMBOL_EQUAL         "="
#define TOPOLOGY_SYMBOL_RIGHT_BRACKET ")"
#define TOPOLOGY_SYMBOL_LEFT_BRACKET  "("
#define TOPOLOGY_SYMBOL_WHITESPACE    " "
#define CONFIGURATION_LIST_SEPERATOR  ';'

// sets of elements of configuration
#define CONFIG_ELEMENT_SET_CLASS           "classes"
#define CONFIG_ELEMENT_SET_POLICY          "policies"
#define CONFIG_ELEMENT_SET_SYSTEM_PROPERTY "properties"
#define CONFIG_ELEMENT_SYSTEM              "system"
#define CONFIG_ELEMENT_TEMPLATE            "templates"

// tags used by system property
#define SYSTEM_PROP_TAG "systemProperty"

// tags used by policyset in configuration
#define POLICY_ATTRIBUTE_NAME      "name"
#define POLICY_TAG_TRIGGER         "trigger"
#define POLICY_TAG_STOP_EVALUATION "break"
#define POLICY_TAG_CONDITION       "condition"
#define POLICY_TAG_ACTION          "action"
#define POLICY_TAG_PRIORITY        "priority"
#define POLICY_TAG_LIST_PROCESS    "process"

// tags used by classset in configuration
#define CLASS_ATTRIBUTE_NAME           "name"
#define CLASS_ATTRIBUTE_TYPE           "type"
#define CLASS_ATTRIBUTE_DEFAULT_VOLUME "defaultVolume"
#define CLASS_TAG_TOPOLOGY_TAG         "topology"
#define CLASS_TAG_PROPERTIES_TAG       "classProperty"

// tags used by domainset in configuration
#define DOMAIN_ATTRIBUTE_BUS_NAME  "busName"
#define DOMAIN_ATTRIBUTE_NODE_NAME "nodeName"
#define DOMAIN_ATTRIBUTE_EARLY     "early"
#define DOMAIN_ATTRIBUTE_COMPLETE  "complete"
#define DOMAIN_ATTRIBUTE_STATE     "state"

// tags used by gatewayset in configuration
#define GATEWAY_ATTRIBUTE_SINK_NAME           "sinkName"
#define GATEWAY_ATTRIBUTE_SOURCE_NAME         "sourceName"
#define GATEWAY_ATTRIBUTE_CONTROL_DOMAIN_NAME "controlDomainName"
#define GATEWAY_TAG_CONVERSION_MATRIX         "conversionMatrix"

// tags used by source/sink/class element set
#define COMMON_ATTRIBUTE_VOLUME_USER_TO_NORM      "listMainVolumesToNormalizedVolumes"
#define COMMON_ATTRIBUTE_VOLUME_NORM_TO_USER      "listNormalizedVolumesToDecibelVolumes"
#define COMMON_ATTRIBUTE_PRIORITY                 "priority"
#define COMMON_ATTRIBUTE_PROPERTY                 "property"
#define COMMON_ATTRIBUTE_TYPE                     "type"
#define COMMON_ATTRIBUTE_VALUE                    "value"
#define COMMON_ATTRIBUTE_IS_PERSISTENCE_SUPPORTED "isPersistenceSupported"

// tags used by sinkset in configuration
#define SINK_ATTRIBUTE_MAIN_VOLUME "mainVolume"
#define SINK_ATTRIBUTE_MUTE_STATE  "muteState"

// tags used by sourceset in configuration
#define SOURCE_ATTRIBUTE_STATE           "sourceState"
#define SOURCE_ATTRIBUTE_INTERRUPT_STATE "interruptState"

// generic scale conversion template
#define TEMPLATE_TAG_SCALE_CONVERSION_MAP "scaleConversionMap"

// post-scaling for routing side
#define SOURCE_TAG_VOLUME_MAP "sourceVolumeMap"
#define SINK_TAG_VOLUME_MAP   "sinkVolumeMap"

// tags used by MSP SP maps
#define CONFIG_ELEMENT_SET_MSP_SP_MAP "listmapMSPToSP"
#define MSPSP_MAP_TAG_TYPE            "mapMSPtoSP"
#define MSPSP_MAP_ATTRIBUTE_TYPE      "mappingDirection"

// tags used by main sound and sound propertyset in configuration
#define SOUND_PROPERTY_ATTRIBUTE_TYPE                     "type"
#define SOUND_PROPERTY_ATTRIBUTE_VALUE                    "value"
#define SOUND_PROPERTY_ATTRIBUTE_MIN_VALUE                "minValue"
#define SOUND_PROPERTY_ATTRIBUTE_MAX_VALUE                "maxValue"
#define SOUND_PROPERTY_ATTRIBUTE_IS_PERSISTENCE_SUPPORTED "isPersistenceSupported"

// tags used by availability set in configuration
#define AVAILABILITY_ATTRIBUTE_AVAILABLE "availability"
#define AVAILABILITY_ATTRIBUTE_REASON    "availabilityReason"

// tags used by source/sink element set
#define ELEMENT_COMMON_ATTRIBUTE_DOMAIN_NAME                     "domainName"
#define ELEMENT_COMMON_ATTRIBUTE_CLASS_NAME                      "className"
#define ELEMENT_COMMON_TAG_AVAILABLE                             "availability"
#define ELEMENT_COMMON_ATTRIBUTE_IS_VOLUME_SUPPORTED             "isVolumeSupported"
#define ELEMENT_COMMON_ATTRIBUTE_IS_PERSISTENCY_SUPPORTED        "isPersistencySupported"
#define ELEMENT_COMMON_ATTRIBUTE_IS_VOLUME_PERSISTENCY_SUPPORTED "isVolumePersistencySupported"
#define ELEMENT_COMMON_ATTRIBUTE_VOLUME                          "volume"
#define ELEMENT_COMMON_ATTRIBUTE_MIN_VOLUME                      "minVolume"
#define ELEMENT_COMMON_ATTRIBUTE_MAX_VOLUME                      "maxVolume"
#define ELEMENT_COMMON_ATTRIBUTE_VISIBLE                         "visible"

// tags used by source/sink/domain/gateway/class element set
#define ELEMENT_COMMON_TAG_REGISTRATION_TYPE   "registrationType"
#define ELEMENT_COMMON_ATTRIBUTE_ID            "id"
#define ELEMENT_COMMON_ATTRIBUTE_NAME          "name"
#define ELEMENT_COMMON_ATTRIBUTE_BASENAME      "baseName"
#define ELEMENT_COMMON_ATTRIBUTE_MSP_SP        "mapMSPToSPTemplateName"
#define ELEMENT_COMMON_ATTRIBUTE_SP_LIST_NAME  "soundPropertiesTemplateName"
#define ELEMENT_COMMON_ATTRIBUTE_MSP_LIST_NAME "mainSoundPropertiesTemplateName"
#define ELEMENT_COMMON_ATTRIBUTE_NC_NAME       "notificationConfigurationsTemplateName"
#define ELEMENT_COMMON_ATTRIBUTE_MNC_NAME      "mainNotificationConfigurationsTemplateName"
#define ELEMENT_COMMON_ATTRIBUTE_CF_NAME       "connectionFormatsTemplateName"

// tags used by action set in configuration
#define ACTION_SET_ATTRIBUTE_TYPE "type"

// tags used by notification set in configuration
#define NOTIFICATION_ATTRIBUTE_TYPE   "type"
#define NOTIFICATION_ATTRIBUTE_STATUS "status"
#define NOTIFICATION_ATTRIBUTE_PARAM  "parameter"

// tags used by enumeration set in configuration
#define ENUM_TAG_ENUMERATION "enumeration"
#define ENUM_TAG_VALUE       "value"

// main tags whose child parameters will be stored
#define XML_ELEMENT_ATTRIBUTE_CONNECTION_FORMAT  "connectionFormat"
#define XML_ELEMENT_TAG_CONNECTION_FORMAT        "connectionFormat"
#define XML_ELEMENT_TAG_SIMPLE_TYPE              "simpleType"
#define XML_ELEMENT_TAG_ANNOTATION               "annotation"
#define XML_ELEMENT_TAG_RESTRICTION              "restriction"
#define XML_ELEMENT_TAG_NOTIFICATION_CONFIG      "notificationConfiguration"
#define XML_ELEMENT_TAG_NOTIFICATION_CONFIGS     "notificationConfigurations"
#define XML_ELEMENT_TAG_MAIN_NOTIFICATION_CONFIG "mainNotificationConfiguration"
#define XML_ELEMENT_TAG_SOUND_PROPERTY           "soundProperty"
#define XML_ELEMENT_TAG_SYSTEM_PROPERTY          "systemProperty"
#define XML_ELEMENT_TAG_SOUND_PROPERTIES         "soundProperties"
#define XML_ELEMENT_TAG_SYSTEM_PROPERTIES        "systemProperties"
#define XML_ELEMENT_TAG_MAIN_SOUND_PROPERTY      "mainSoundProperty"
#define XML_ELEMENT_TAG_MAIN_SOUND_PROPERTIES    "mainSoundProperties"
#define XML_ELEMENT_TAG_GENERIC                  "generic"
#define XML_ELEMENT_TAG_SOURCE                   "source"
#define XML_ELEMENT_TAG_SINK                     "sink"
#define XML_ELEMENT_TAG_GATEWAY                  "gateway"
#define XML_ELEMENT_TAG_DOMAIN                   "domain"
#define XML_ELEMENT_TAG_CLASS                    "class"
#define XML_ELEMENT_TAG_POLICY                   "policy"
#define XML_ELEMENT_TAG_SCHEMA                   "schema"

// extra symbol returned by xmllibrary during parsing
#define XML_ILLEGAL_SYMBOL "text"

#define DEFAULT_CONFIG_PARSED_POLICY_PRIORITY 49
#define DEFAULT_CONFIG_ELEMENT_PRIORITY       50
#define DEFAULT_CONFIG_CLASS_PRIORITY         99

// functions supported by configuration conditions set
#define FUNCTION_NAME                                   "name"
#define FUNCTION_ELEMENTS                               "elements"
#define FUNCTION_PRIORITY                               "priority"
#define FUNCTION_CONNECTION_STATE                       "connectionState"
#define FUNCTION_VOLUME                                 "volume"
#define FUNCTION_MAIN_VOLUME                            "mainVolume"
#define FUNCTION_MAIN_VOLUME_STEP                       "isMainVolumeStep"
#define FUNCTION_VOLUME_LIMIT                           "volumeLimit"
#define FUNCTION_SOUND_PROPERTY                         "soundProperty"
#define FUNCTION_MAIN_SOUND_PROPERTY_TYPE               "mainSoundPropertyType"
#define FUNCTION_MAIN_SOUND_PROPERTY_VALUE              "mainSoundPropertyValue"
#define FUNCTION_SYSTEM_PROPERTY_VALUE                  "systemPropertyValue"
#define FUNCTION_SYSTEM_PROPERTY_TYPE                   "systemPropertyType"
#define FUNCTION_MUTE_STATE                             "muteState"
#define FUNCTION_IS_REGISTRATION_COMPLETE               "isRegistrationComplete"
#define FUNCTION_AVAILABILITY                           "availability"
#define FUNCTION_AVAILABILITY_REASON                    "availabilityReason"
#define FUNCTION_INTERRUPT_STATE                        "interruptState"
#define FUNCTION_IS_REGISTERED                          "isRegistered"
#define FUNCTION_STATE                                  "state"
#define FUNCTION_DOMAIN_STATE                           "domainState"
#define FUNCTION_SOURCE_STATE                           "sourceState"
#define FUNCTION_CONNECTION_FORMAT                      "connectionFormat"
#define FUNCTION_ERROR                                  "error"
#define FUNCTION_NOTIFICATION_CONFIGURATION_STATUS      "notificationConfigurationStatus"
#define FUNCTION_NOTIFICATION_CONFIGURATION_PARAM       "notificationConfigurationParam"
#define FUNCTION_NOTIFICATION_DATA_VALUE                "notificationDataValue"
#define FUNCTION_NOTIFICATION_DATA_TYPE                 "notificationDataType"
#define FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_TYPE   "mainNotificationConfigurationType"
#define FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_STATUS "mainNotificationConfigurationStatus"
#define FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_PARAM  "mainNotificationConfigurationParam"
#define FUNCTION_PEEK                                   "peek"
#define FUNCTION_SCALE                                  "scale"
#define FUNCTION_COUNT                                  "count"

// macros supported in functions
#define FUNCTION_MACRO_SUPPORTED_REQUESTING "REQUESTING"
#define FUNCTION_MACRO_SUPPORTED_ALL        "ALL"
#define FUNCTION_MACRO_SUPPORTED_OTHERS     "OTHERS"
#define FUNCTION_MACRO_REQ_TRIG_TYPE        "REQ_TRIG_TYPE"
#define FUNCTION_MACRO_REQ_SINK_NAME        "REQ_SINK_NAME"
#define FUNCTION_MACRO_REQ_SOURCE_NAME      "REQ_SOURCE_NAME"
#define FUNCTION_MACRO_REQ_DOMAIN_NAME      "REQ_DOMAIN_NAME"
#define FUNCTION_MACRO_REQ_GATEWAY_NAME     "REQ_GATEWAY_NAME"
#define FUNCTION_MACRO_REQ_CLASS_NAME       "REQ_CLASS_NAME"
#define FUNCTION_MACRO_REQ_CONNECTION_NAME  "REQ_CON_NAME"
#define FUNCTION_MACRO_REQ_CONNECTION_STATE "REQ_CON_STATE"
#define FUNCTION_MACRO_REQ_STATUS           "REQ_STATUS"
#define FUNCTION_MACRO_MAIN_VOLUME          "REQ_MAIN_VOL"
#define FUNCTION_MACRO_MSP_TYPE             "REQ_MSP_TYPE"
#define FUNCTION_MACRO_MSP_VAL              "REQ_MSP_VAL"
#define FUNCTION_MACRO_SYP_TYPE             "REQ_SYP_TYPE"
#define FUNCTION_MACRO_SYP_VAL              "REQ_SYP_VAL"
#define FUNCTION_MACRO_AVAIL_STATE          "REQ_AVAIL_STATE"
#define FUNCTION_MACRO_AVAIL_REASON         "REQ_AVAIL_REASON"
#define FUNCTION_MACRO_MUTE_STATE           "REQ_MUTE_STATE"
#define FUNCTION_MACRO_INT_STATE            "REQ_INT_STATE"
#define FUNCTION_MACRO_NP_TYPE              "REQ_NP_TYPE"
#define FUNCTION_MACRO_NP_VAL               "REQ_NP_VAL"
#define FUNCTION_MACRO_NC_TYPE              "REQ_NC_TYPE"
#define FUNCTION_MACRO_NC_STATUS            "REQ_NC_STATUS"
#define FUNCTION_MACRO_NC_PARAM             "REQ_NC_PARAM"

// category by which functions can be separated and evaluated
#define CATEGORY_SINK                 "SINK"
#define CATEGORY_SOURCE               "SOURCE"
#define CATEGORY_CLASS                "CLASS"
#define CATEGORY_CONNECTION           "CONNECTION"
#define CATEGORY_DOMAIN               "DOMAIN"
#define CATEGORY_DOMAIN_OF_SOURCE     "DOMAINOFSOURCE"
#define CATEGORY_DOMAIN_OF_SINK       "DOMAINOFSINK"
#define CATEGORY_CLASS_OF_SOURCE      "CLASSOFSOURCE"
#define CATEGORY_CLASS_OF_SINK        "CLASSOFSINK"
#define CATEGORY_SOURCE_OF_CLASS      "SOURCEOFCLASS"
#define CATEGORY_SINK_OF_CLASS        "SINKOFCLASS"
#define CATEGORY_CONNECTION_OF_CLASS  "CONNECTIONOFCLASS"
#define CATEGORY_CONNECTION_OF_SOURCE "CONNECTIONOFSOURCE"
#define CATEGORY_CONNECTION_OF_SINK   "CONNECTIONOFSINK"
#define CATEGORY_SYSTEM               "SYSTEM"
#define CATEGORY_USER                 "USER"


/// Operators for policy condition evaluation
enum gc_Operator_e
{
    EQ,   ///< left side equals right side
    GT,   ///< left side greater than right side
    GEQ,  ///< left side greater than or equal to right side
    LT,   ///< left side less than right side
    LEQ,  ///< left side less than or equal to right side
    NE,   ///< left side not equal to right side
    INC,  ///< left side collection contains right side item
    EXC   ///< left side collection does not contain right side item
};

// stream non-default members for logging
inline std::ostream &operator<<(std::ostream &out, const gc_Operator_e &op)
{
    switch (op)
    {
    case EQ:
        return out << "EQ";
    case GT:
        return out << "GT";
    case GEQ:
        return out << "GEQ";
    case LT:
        return out << "LT";
    case LEQ:
        return out << "LEQ";
    case NE:
        return out << "NE";
    case INC:
        return out << "INC";
    case EXC:
        return out << "EXC";
    default:
        return out << "(Unknown Operator:" << (int)op << ")";
    }
}

/// Operand type (category) by which functions can be separated and evaluated
enum gc_OperandType_e
{
    OT_Invalid = 0,
    OT_SINK,                 ///< SINK: operate on given sink
    OT_SOURCE,               ///< SOURCE: operate on given source
    OT_CLASS,                ///< CLASS: operate on given class
    OT_CONNECTION,           ///< CONNECTION: operate on given connection, specified as colon-joined source and sink name
    OT_DOMAIN,               ///< DOMAIN: operate on given domain
    OT_DOMAIN_OF_SOURCE,     ///< DOMAINOFSOURCE: operate on domain hosting given source
    OT_DOMAIN_OF_SINK,       ///< DOMAINOFSINK: operate on domain hosting given sink
    OT_CLASS_OF_SOURCE,      ///< CLASSOFSOURCE: operate on class assigned to given source
    OT_CLASS_OF_SINK,        ///< CLASSOFSINK: operate on class assigned to given sink
    OT_SOURCE_OF_CLASS,      ///< SOURCEOFCLASS: operate on source(s) mapped to given class
    OT_SINK_OF_CLASS,        ///< SINKOFCLASS: operate on sink(s) mapped to given class
    OT_CONNECTION_OF_CLASS,  ///< CONNECTIONOFCLASS: operate on connection(s) assigned to given class
    OT_CONNECTION_OF_SOURCE, ///< CONNECTIONOFSOURCE: operate on connection(s) containing given source
    OT_CONNECTION_OF_SINK,   ///< CONNECTIONOFSINK: operate on connection(s) containing given sink
    OT_SYSTEM,               ///< SYSTEM: operate on global settings item
    OT_USER                  ///< operate on item requested by user
};

// stream non-default members for logging
inline std::ostream &operator<<(std::ostream &out, const gc_OperandType_e &ot)
{
    switch (ot)
    {
    case OT_SINK:
        return out << CATEGORY_SINK;
    case OT_SOURCE:
        return out << CATEGORY_SOURCE;
    case OT_CLASS:
        return out << CATEGORY_CLASS;
    case OT_CONNECTION:
        return out << CATEGORY_CONNECTION;
    case OT_DOMAIN:
        return out << CATEGORY_DOMAIN;
    case OT_DOMAIN_OF_SOURCE:
        return out << CATEGORY_DOMAIN_OF_SOURCE;
    case OT_DOMAIN_OF_SINK:
        return out << CATEGORY_DOMAIN_OF_SINK;
    case OT_CLASS_OF_SOURCE:
        return out << CATEGORY_CLASS_OF_SOURCE;
    case OT_CLASS_OF_SINK:
        return out << CATEGORY_CLASS_OF_SINK;
    case OT_SOURCE_OF_CLASS:
        return out << CATEGORY_SOURCE_OF_CLASS;
    case OT_SINK_OF_CLASS:
        return out << CATEGORY_SINK_OF_CLASS;
    case OT_CONNECTION_OF_CLASS:
        return out << CATEGORY_CONNECTION_OF_CLASS;
    case OT_CONNECTION_OF_SOURCE:
        return out << CATEGORY_CONNECTION_OF_SOURCE;
    case OT_CONNECTION_OF_SINK:
        return out << CATEGORY_CONNECTION_OF_SINK;
    case OT_SYSTEM:
        return out << CATEGORY_SYSTEM;
    case OT_USER:
        return out << CATEGORY_USER;
    default:
        return out << "(Invalid OperandType:" << (int)ot << ")";
    }
}

// store the function and its parameter as given in policy
struct gc_FunctionElement_s
{
    std::string functionName;
    gc_OperandType_e category;
    std::string mandatoryParameter;
    std::string optionalParameter;
    std::string optionalParameter2;
    bool isValueMacro;
};

// store the RHS side of condition as given in policy
struct gc_RHSElement_s
{
    bool isValue;
    gc_FunctionElement_s functionObject;
    std::string directValue;
};

// store the condition as given in policy
struct gc_ConditionStruct_s
{
    gc_FunctionElement_s leftObject;
    gc_Operator_e operation;
    gc_RHSElement_s rightObject;
};

struct gc_Process_s
{
    std::string comment;
    int32_t priority;
    bool stopEvaluation;
    std::vector<gc_ConditionStruct_s> listConditions;
    std::vector<gc_Action_s> listActions;
};

struct gc_Policy_s
{
    std::vector<int> listEvents;
    std::vector<gc_Process_s> listProcesses;
};

struct gc_Configuration_s
{
    std::vector<gc_Source_s> listSources;
    std::vector<gc_Sink_s> listSinks;
    std::vector<gc_Gateway_s> listGateways;
    std::vector<gc_Domain_s> listDomains;
    std::vector<gc_Class_s> listClasses;
    std::vector<gc_Policy_s> listPolicies;
    std::vector<gc_SystemProperty_s> listSystemProperties;

    std::vector<gc_Source_s> listTemplateSources;
    std::vector<gc_Sink_s> listTemplateSinks;
    std::vector<gc_Gateway_s> listTemplateGateways;
    std::vector<gc_Domain_s> listTemplateDomains;

    std::map<std::string, std::vector<gc_SoundProperty_s> > listTemplateSoundProperties;
    std::map<std::string, std::vector<gc_SystemProperty_s> > listTemplateSystemProperties;
    std::map<std::string, std::vector<gc_MainSoundProperty_s> > listTemplateMainSoundProperties;
    std::map<std::string, std::vector<am_NotificationConfiguration_s> > listTemplateNotificationConfigurations;
    std::map<std::string, std::vector<am_CustomConnectionFormat_t> > listTemplateConnectionFormats;
    std::map<std::string, std::map<float, float> > listTemplateMapScaleConversions;
    std::map<std::string, std::map<gc_MSPMappingDirection_e, std::map<uint16_t, uint16_t> > > listTemplateMapMSPToSP;
    std::map<std::string, std::vector<std::pair<uint16_t, uint16_t> > > listTemplateConversionMatrix;

    std::string mCurrentDomain;
    std::string mCurrentSource;
    std::string mCurentSink;

};

class CAmXmlConfigParser;

/**************************************************************************//**
 * @class CAmXmlConfigParser
 *
 * @copydoc CAmXmlConfigParser.h
 */
class CAmXmlConfigParser : public CAmXmlConfigParserCommon
{
public:
    CAmXmlConfigParser(void);
    ~CAmXmlConfigParser(void);
    am_Error_e parse(gc_Configuration_s *pConfiguration);

    /// helper to access configured scale conversion maps
    static void populateScalingMap(std::map<float, float > &outMap, const std::string &valuePairString
        , const std::map<std::string, std::map<float, float> > listTemplateMapScaleConversions);

    static am_Error_e parsePolicyFunction(const char *inputString
        , unsigned int &startPosition, gc_FunctionElement_s &function);

private:
    /**
     * @brief It is the internal function use to validate the user configuration against the schema
     * @param XMLFilename: user configuration file name
     *        XSDFilename: schema name
     * @return E_UNKNOWN on error
     *         E_OK on success
     */
    am_Error_e _validateConfiguration(const std::string &XMLFilename, const std::string &XSDFilename);

    /**
     * @brief It is the internal function use to parse the schema.
     * @param XSDFilename: schema file name
     * @return E_UNKNOWN on error
     *         E_OK on success
     */
    am_Error_e _parseXSDFile(const std::string &XSDFilename);

    /**
     * @brief It is the internal function use to parse the simple types as defined in schema.
     * @param pDocument: document pointer
     *        pCurrent: current node to be parsed
     * @return none
     */
    void _parseSimpleType(const xmlDocPtr pDocument, xmlNodePtr pCurrent);

    /**
     * @brief It is the internal function use to parse the enum initializer values as defined in schema.
     * @param pDocument: document pointer
     *        pCurrent: current node to be parsed
     *        value: value of enum
     * @return E_UNKNOWN on error
     *         E_OK on success
     */
    am_Error_e _parseEnumInitialiser(const xmlDocPtr pDocument, xmlNodePtr pCurrent, int &value);

    /**
     * @brief It is the internal function use to parse the enum as defined in schema.
     * @param pDocument: document pointer
     *        pCurrent: current node to be parsed
     *        Name: name of enum
     * @return none
     */
    void _parseEnumeration(const xmlDocPtr pDocument, xmlNodePtr pCurrent);

    /**
     * @brief It is the internal function use to parse the default and user configuration common elements
     * @param XMLFilename: file to be parsed
     * @return E_NOT_POSSIBLE on error
     *         E_OK on success
     */
    am_Error_e _parseConfiguration(const std::string &XMLFilename, gc_Configuration_s *pConfiguration);

    gc_Configuration_s *mpConfiguration;

#if AMCO_DEBUGGING
    void printAllEnums(void) const;
    void printListGateways(gc_Configuration_s *pConfiguration) const;
    void printListDomains(gc_Configuration_s *pConfiguration) const;
    void printListClasses(gc_Configuration_s *pConfiguration) const;
    void printListPolicies(gc_Configuration_s *pConfiguration) const;
    void printListSinks(gc_Configuration_s *pConfiguration) const;
    void printListSources(gc_Configuration_s *pConfiguration) const;
    void printListSystemProperties(gc_Configuration_s *pConfiguration) const;
#endif      // if AMCO_DEBUGGING
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_XMLCONFIGPARSER_H_ */
