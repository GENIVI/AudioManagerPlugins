#include "CAmXmlConfigParser.h"

#include <CAmDltWrapper.h>

using std::string;
using std::stoi;

namespace am {

CAmXmlConfigParser::CAmXmlConfigParser()
{
}

CAmXmlConfigParser::~CAmXmlConfigParser(void)
{
}

am_Error_e CAmXmlConfigParser::parse(rp_Configuration_s& pConfiguration)
{
    char *path = (char *)getenv(CONFIGURATION_FILE_ENV_VARNAME);
    if (nullptr == path)
    {
        path = (char*)DEFAULT_USER_CONFIGURATION_PATH;
    }

    logInfo("Parse Start");

    if (E_OK != _parseConfiguration(string(path), pConfiguration))
    {
        logError(" Not able to parse configuration");
        return E_UNKNOWN;
    }

    logInfo("Parse End");

    return E_OK;
}

am_Error_e CAmXmlConfigParser::_parseConfiguration(const string& XMLFilename,
                                                   rp_Configuration_s& pConfiguration)
{
    xmlKeepBlanksDefault(0);

    xmlDocPtr pDocument = xmlParseFile(XMLFilename.c_str());
    xmlNodePtr pCurrent;

    if (pDocument == nullptr)
    {
        logError(" Document not parsed successfully");

        return E_NOT_POSSIBLE;
    }

    pCurrent = xmlDocGetRootElement(pDocument);

    if (pCurrent == nullptr)
    {
        logError(" Empty document");

        xmlFreeDoc(pDocument);

        return E_NOT_POSSIBLE;
    }

    if (xmlStrcmp(pCurrent->name, (const xmlChar *)PULSE_NODE_NAME))
    {
        logError(" Wrong document: expected - ", PULSE_NODE_NAME);

        xmlFreeDoc(pDocument);

        return E_NOT_POSSIBLE;
    }

    am_Error_e ret = E_OK;

    pCurrent = pCurrent->xmlChildrenNode;

    while (pCurrent != nullptr)
    {
        if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_ELEMENT_DOMAIN))
        {
            ret = _parseDomain(pCurrent, pConfiguration);

            if (ret != E_OK)
            {
                break;
            }
        }

        if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_ELEMENT_SET_SOURCE))
        {
            ret = _parseSet(pCurrent, (const xmlChar *)CONFIG_ELEMENT_SOURCE, &CAmXmlConfigParser::_parseSource, pConfiguration);

            if (ret != E_OK)
            {
                break;
            }
        }

        if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_ELEMENT_SET_SINK))
        {
            ret = _parseSet(pCurrent, (const xmlChar *)CONFIG_ELEMENT_SINK, &CAmXmlConfigParser::_parseSink, pConfiguration);

            if (ret != E_OK)
            {
                break;
            }
        }

        pCurrent = pCurrent->next;
    }

    xmlFreeDoc(pDocument);

    return ret;
}

am_Error_e CAmXmlConfigParser::_parseDomain(xmlNodePtr node, rp_Configuration_s& pConfiguration)
{
    rp_Domain_s domain {};

    xmlNodePtr pCurrent;

    am_Error_e ret = E_OK;

    pCurrent = node->xmlChildrenNode;

    while (pCurrent != nullptr)
    {
        char *value = (char *)xmlNodeGetContent(pCurrent);

        if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_ID))
        {
            domain.id = stoi(value);
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_NAME))
        {
            domain.name = value;
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_NODE_NAME))
        {
            domain.nodeName = value;
        }

        xmlFree(value);

        pCurrent = pCurrent->next;
    }

    pConfiguration.domain = domain;

    return ret;
}

am_Error_e CAmXmlConfigParser::_parseSet(xmlNodePtr node, const xmlChar *elementName, ParseElement parser,
                                         rp_Configuration_s& pConfiguration)
{
    xmlNodePtr pCurrent;

    am_Error_e ret = E_OK;

    pCurrent = node->xmlChildrenNode;

    while (pCurrent != nullptr)
    {
        if ((!xmlStrcmp(pCurrent->name, elementName)))
        {
            ret = (this->*parser)(pCurrent, pConfiguration);

            if (ret != E_OK)
            {
                break;
            }
        }
        else
        {
            logError(" Wrong set element: ", (const char *)pCurrent->name);

            ret = E_NOT_POSSIBLE;

            break;
        }

        pCurrent = pCurrent->next;
    }

    return ret;
}

am_Error_e CAmXmlConfigParser::_parseSource(xmlNodePtr node, rp_Configuration_s& pConfiguration)
{
    rp_Source_s source {};

    xmlNodePtr pCurrent;

    am_Error_e ret = E_OK;

    pCurrent = node->xmlChildrenNode;

    while (pCurrent != nullptr)
    {
        char *value = (char *)xmlNodeGetContent(pCurrent);

        if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_ID))
        {
            source.id = stoi(value);
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_NAME))
        {
            source.name = value;
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_DEVICE_NAME))
        {
            source.deviceName = value;
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_CLASS_NAME))
        {
            source.className = value;
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_PROPERTY_NAME))
        {
            source.propertyName = value;
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_PROPERTY_VALUE))
        {
            source.propertyValue = value;
        }

        xmlFree(value);

        pCurrent = pCurrent->next;
    }

    pConfiguration.listSources.push_back(source);

    return ret;

}

am_Error_e CAmXmlConfigParser::_parseSink(xmlNodePtr node, rp_Configuration_s& pConfiguration)
{
    rp_Sink_s sink {};

    xmlNodePtr pCurrent;

    am_Error_e ret = E_OK;

    pCurrent = node->xmlChildrenNode;

    while (pCurrent != nullptr)
    {
        char *value = (char *)xmlNodeGetContent(pCurrent);

        if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_ID))
        {
            sink.id = stoi(value);
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_NAME))
        {
            sink.name = value;
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_DEVICE_NAME))
        {
            sink.deviceName = value;
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_CLASS_NAME))
        {
            sink.className = value;
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_PROPERTY_NAME))
        {
            sink.propertyName = value;
        }
        else if (!xmlStrcmp(pCurrent->name, (const xmlChar *)CONFIG_TAG_PROPERTY_VALUE))
        {
            sink.propertyValue = value;
        }

        xmlFree(value);

        pCurrent = pCurrent->next;
    }

    pConfiguration.listSinks.push_back(sink);

    return ret;
}

} /* namespace am */
