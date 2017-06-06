#ifndef XMLCONFIGPARSER_H_
#define XMLCONFIGPARSER_H_

#include <libxml/parser.h>

#include <audiomanagertypes.h>

namespace am {

#ifndef PULSE_ROUTING_DEFAULT_CONF_ROOT
#define PULSE_ROUTING_DEFAULT_CONF_ROOT "/etc/routing"
#endif

#define DEFAULT_USER_CONFIGURATION_PATH   PULSE_ROUTING_DEFAULT_CONF_ROOT"/pulse.xml"
#define CONFIGURATION_FILE_ENV_VARNAME    "PULSE_ROUTING_CONFIGURATION"

#define PULSE_NODE_NAME             "amri_pulse"

#define CONFIG_ELEMENT_DOMAIN       "domain"
#define CONFIG_ELEMENT_SET_SOURCE   "listSources"
#define CONFIG_ELEMENT_SET_SINK     "listSinks"

#define CONFIG_ELEMENT_SOURCE       "source"
#define CONFIG_ELEMENT_SINK         "sink"

#define CONFIG_TAG_ID               "id"
#define CONFIG_TAG_NAME             "name"
#define CONFIG_TAG_NODE_NAME        "nodeName"
#define CONFIG_TAG_DEVICE_NAME      "deviceName"
#define CONFIG_TAG_CLASS_NAME       "className"
#define CONFIG_TAG_PROPERTY_NAME    "propertyName"
#define CONFIG_TAG_PROPERTY_VALUE   "propertyValue"

struct rp_ConfigSource_s
{
    am_sourceID_t   id;
    std::string     name;
    std::string     deviceName;
    std::string     className;
    std::string     propertyName;
    std::string     propertyValue;
};

struct rp_ConfigSink_s
{
    am_sinkID_t     id;
    std::string     name;
    std::string     deviceName;
    std::string     className;
    std::string     propertyName;
    std::string     propertyValue;
};

struct rp_ConfigDomain_s
{
    am_domainID_t   id;
    std::string     name;
    std::string     nodeName;
};

struct rp_Configuration_s
{
    rp_ConfigDomain_s                 domain;
    std::vector<rp_ConfigSource_s>    listSources;
    std::vector<rp_ConfigSink_s>      listSinks;
};

class CAmXmlConfigParser
{
public:
    CAmXmlConfigParser(void);
    ~CAmXmlConfigParser(void);

    am_Error_e parse(rp_Configuration_s& pConfiguration);

private:
    typedef am_Error_e(CAmXmlConfigParser::*ParseElement)(xmlNodePtr node, rp_Configuration_s& pConfiguration);

    am_Error_e _parseConfiguration(const std::string& XMLFilename, rp_Configuration_s& pConfiguration);
    am_Error_e _parseDomain(xmlNodePtr node, rp_Configuration_s& pConfiguration);
    am_Error_e _parseSet(xmlNodePtr node, const xmlChar *elementName, ParseElement parser,
                         rp_Configuration_s& pConfiguration);

    am_Error_e _parseSource(xmlNodePtr node, rp_Configuration_s& pConfiguration);
    am_Error_e _parseSink(xmlNodePtr node, rp_Configuration_s& pConfiguration);
};

} /* namespace am */
#endif /* XMLCONFIGPARSER_H_ */
