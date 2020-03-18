/******************************************************************************
 * @file: CAmXmlConfigParser.cpp
 *
 * This file contains the definition of xml configuration reader class (member functions
 * and data members) used to implement the logic of parsing the configuration file.
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

#include "CAmXmlConfigParser.h"
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iterator>
#include <string.h>
#include <vector>
#include <map>
#include <algorithm>
#include <limits.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>
#include "CAmLogger.h"

namespace am
{
namespace gc
{

using namespace std;

class CAmNameAttribute : public CAmNodeAttribute
{
public:
    CAmNameAttribute(const char *attributeName, std::string &str)
        : CAmNodeAttribute(attributeName)
        , mParsedString(str)
    {
    }

protected:
    void _saveAttribute(const std::string &str)
    {
        std::istringstream       iss(str);
        std::vector<std::string> vstrings(std::istream_iterator<std::string>
            { iss }, std::istream_iterator<std::string>
            { });
        std::vector<std::string>::iterator it;
        for (it = vstrings.begin(); it != vstrings.end(); ++it)
        {
            if ((*it).find("N:") == std::string::npos)
            {
                mParsedString += *it;
            }
            else
            {
                mParsedString += "\"" + (*it).substr(2) + "\"";
            }

            if (it != vstrings.end() - 1)
            {
                mParsedString += " ";
            }
        }
    }

private:
    std::string &mParsedString;
};

template<typename T1>
class CAmEnumerationAttribute : public CAmNodeAttribute
{
public:
    CAmEnumerationAttribute(const char *attributeName, T1 &enumValue)
        : CAmNodeAttribute(attributeName)
        , mEnumValue(enumValue)
    {
    }

protected:
    void _saveAttribute(const std::string &str)
    {
        int enumeratedValue;
        if (E_OK == CAmXmlConfigParserCommon::getEnumerationValue(str, enumeratedValue))
        {
            mEnumValue = (T1)enumeratedValue;
        }
    }

private:
    T1 &mEnumValue;
};

class CAmStringIDAttribute : public CAmNodeAttribute
{
public:
    CAmStringIDAttribute(const char *name, uint16_t &parsedID)
        : CAmNodeAttribute(name)
        , mParsedID(parsedID)
    {
    }

protected:
    void _saveAttribute(const std::string &str)
    {
        int enumeratedValue;
        if (E_OK == CAmXmlConfigParserCommon::getEnumerationValue(str, enumeratedValue))
        {
            mParsedID = enumeratedValue;
        }
        else
        {
            mParsedID = CAmXmlConfigParserCommon::convert<uint16_t>(str);
        }
    }

private:
    uint16_t &mParsedID;
};

class CAmConfigComplexNode : public CAmComplexNode
{
public:
    CAmConfigComplexNode(const char *tagname)
        : CAmComplexNode(tagname)
    {
        mMapTemplates.push_back(std::make_pair("baseName", &am::gc::CAmConfigComplexNode::_getDefaultFromTemplate));
        mMapTemplates.push_back(std::make_pair("name", &am::gc::CAmConfigComplexNode::_storeCurrentName));
        mMapTemplates.push_back(std::make_pair("soundPropertiesTemplateName", &am::gc::CAmConfigComplexNode::_getSoundPropertyList));
        mMapTemplates.push_back(std::make_pair("mainSoundPropertiesTemplateName", &am::gc::CAmConfigComplexNode::_getMainSoundPropertyList));
        mMapTemplates.push_back(std::make_pair("notificationConfigurationsTemplateName", &am::gc::CAmConfigComplexNode::_getNotificationConfiguration));
        mMapTemplates.push_back(std::make_pair("mainNotificationConfigurationsTemplateName", &am::gc::CAmConfigComplexNode::_getMainNotificationConfiguration));
        mMapTemplates.push_back(std::make_pair("connectionFormatsTemplateName", &am::gc::CAmConfigComplexNode::_getConnectionFormats));
        mMapTemplates.push_back(std::make_pair("mapMSPToSPTemplateName", &am::gc::CAmConfigComplexNode::_getMapMSPToSP));
        mMapTemplates.push_back(std::make_pair("conversionMatrixTemplateName", &am::gc::CAmConfigComplexNode::_getConversionMatrix));
    }

};

template<typename T1, typename T2>
class CAmEncapsulatedListNode : public CAmConfigComplexNode
{

public:
    CAmEncapsulatedListNode(const char *tagName, const char *listTagName, std::vector<T1> &list, void *data = NULL)
        : CAmConfigComplexNode(tagName)
        , mpListTagName(listTagName)
        , mpData(data)
        , mList(list)
        , mListNode(mpListTagName, mList, mpData)
    {
    }

    void expand(void)
    {
        addChildNode(&mListNode);
    }

private:
    const char         *mpListTagName;
    void               *mpData;
    std::vector<T1>    &mList;
    CAmListNode<T1, T2> mListNode;
};

template<typename T1, typename T2>
class CAmEncapsulatedTemplateList : public CAmXmlNode
{

public:
    CAmEncapsulatedTemplateList(const char *listTagName, const char *tagName, std::map<std::string, std::vector<T1> > *pList)
        : mpListTagName(listTagName)
        , mpTagName(tagName)
        , mpMap(pList)
    {
    }

    int parse(const xmlDocPtr pDocument, xmlNodePtr *pCurrent)
    {
        int returnValue = PARSE_TAG_MISMATCH;
        if (((*pCurrent) == NULL) || (xmlStrEqual((*pCurrent)->name, reinterpret_cast<const xmlChar *>(mpListTagName)) == 0))
        {
            return PARSE_TAG_MISMATCH;
        }

        do
        {
            // First parse the list Name
            CAmStringAttribute name(ELEMENT_COMMON_ATTRIBUTE_NAME, mName);
            name.parse(pDocument, *pCurrent);
            xmlNodePtr pChild = (*pCurrent)->children;
            pChild = getNextElementNode(pChild);
            std::vector<T1>     innerlist;
            CAmListNode<T1, T2> listNode(mpTagName, innerlist);
            returnValue = listNode.parse(pDocument, &pChild);
            if (innerlist.size() > 0)
            {
                (*mpMap)[mName] = innerlist;
            }

            xmlNodePtr nextxmlNode = getNextElementNode(*pCurrent);
            if (((nextxmlNode) == NULL) || xmlStrEqual(nextxmlNode->name, reinterpret_cast<const xmlChar *>(mpListTagName)) == 0)
            {
                break;
            }

            *pCurrent = getNextElementNode(*pCurrent);
        } while (returnValue == 0);

        return returnValue;
    }

private:
    const char *mpListTagName;
    const char *mpTagName;
    std::string mName;
    std::map<std::string, std::vector<T1> > *mpMap;
};

template<typename T1, typename T2>
class CAmEncapsulatedTemplateMap : public CAmXmlNode
{

public:
    CAmEncapsulatedTemplateMap(const char *listTagName, const char *tagName, std::map<std::string, T1> *pMap)
        : mpTagName(tagName)
        , mpListTagName(listTagName)
        , mpMap(pMap)
    {
    }

    int parse(const xmlDocPtr pDocument, xmlNodePtr *pCurrent)
    {
        int returnValue = PARSE_TAG_MISMATCH;
        if (((*pCurrent) == NULL) || (xmlStrEqual((*pCurrent)->name, reinterpret_cast<const xmlChar *>(mpListTagName)) == 0))
        {
            return PARSE_TAG_MISMATCH;
        }

        do
        {
            CAmStringAttribute name(ELEMENT_COMMON_ATTRIBUTE_NAME, mName);
            name.parse(pDocument, *pCurrent);
            xmlNodePtr pChild = (*pCurrent)->children;
            pChild = getNextElementNode(pChild);
            T1 innerMap;
            T2 listNode(mpTagName, &innerMap);
            returnValue = listNode.parse(pDocument, &pChild);
            if (innerMap.size() > 0)
            {
                (*mpMap)[mName] = innerMap;
            }

            xmlNodePtr nextxmlNode = getNextElementNode(*pCurrent);
            if (((nextxmlNode) == NULL) || xmlStrEqual(nextxmlNode->name, reinterpret_cast<const xmlChar *>(mpListTagName)) == 0)
            {
                break;
            }

            (*pCurrent) = nextxmlNode;
        } while (returnValue == 0);

        return returnValue;
    }

private:
    const char                *mpTagName;
    const char                *mpListTagName;
    std::string                mName;
    std::map<std::string, T1> *mpMap;
};

class CAmTemplateMap : public CAmXmlNode
{

public:
    CAmTemplateMap(const char *tagName, std::map<std::string, std::string> *pMap)
        : mpTagName(tagName)
        , mpMap(pMap)
    {
    }

    int parse(const xmlDocPtr pDocument, xmlNodePtr *pCurrent)
    {
        int returnValue = PARSE_TAG_MISMATCH;
        if (((*pCurrent) == NULL) || (xmlStrEqual((*pCurrent)->name, reinterpret_cast<const xmlChar *>(mpTagName)) == 0))
        {
            return PARSE_TAG_MISMATCH;
        }

        do
        {
            CAmStringAttribute name(ELEMENT_COMMON_ATTRIBUTE_NAME, mName);
            name.parse(pDocument, *pCurrent);
            std::string   str;
            CAmStringNode stringNode(mpTagName, str);
            returnValue = stringNode.parse(pDocument, pCurrent);
            if (str.size() > 0)
            {
                (*mpMap)[mName] = str;
            }

            xmlNodePtr nextxmlNode = getNextElementNode(*pCurrent);
            if (((nextxmlNode) == NULL) || xmlStrEqual(nextxmlNode->name, reinterpret_cast<const xmlChar *>(mpTagName)) == 0)
            {
                break;
            }

            (*pCurrent) = nextxmlNode;
        } while (returnValue == 0);

        return returnValue;
    }

private:
    const char *mpTagName;
    std::string mName;
    std::map<std::string, std::string> *mpMap;
};

class CAmClassPropertyNode : public CAmConfigComplexNode
{
public:
    CAmClassPropertyNode(const char *tagname, am_ClassProperty_s &soundProperty, void *data = NULL)
        : CAmConfigComplexNode(tagname)
        , mClassProperty(soundProperty)
        , mTypeAttribute(COMMON_ATTRIBUTE_TYPE, mClassProperty.classProperty)
        , mValueAttribute(COMMON_ATTRIBUTE_VALUE, mClassProperty.value)
    {
        mClassProperty = { 0, 0 };
    }

    void push_back(std::vector<am_ClassProperty_s > &listClassProperty, am_ClassProperty_s &soundProperty)
    {
        listClassProperty.push_back(soundProperty);
    }

    void expand(void)
    {
        addAttributeList(std::vector<CAmNodeAttribute *>{ &mTypeAttribute, &mValueAttribute });
    }

private:
    am_ClassProperty_s               &mClassProperty;
    CAmEnumerationAttribute<uint16_t> mTypeAttribute;
    CAmIntAttribute<int16_t>          mValueAttribute;
};

class CAmSoundPropertyNode : public CAmConfigComplexNode
{
public:
    CAmSoundPropertyNode(const char *tagname, gc_SoundProperty_s &soundProperty, void *data = NULL)
        : CAmConfigComplexNode(tagname)
        , mSoundProperty(soundProperty)
        , mTypeAttribute(SOUND_PROPERTY_ATTRIBUTE_TYPE, mSoundProperty.type)
        , mValueAttribute(SOUND_PROPERTY_ATTRIBUTE_VALUE, mSoundProperty.value)
        , mMaxValueAttribute(SOUND_PROPERTY_ATTRIBUTE_MIN_VALUE, mSoundProperty.minValue)
        , mMinValueAttribute(SOUND_PROPERTY_ATTRIBUTE_MAX_VALUE, mSoundProperty.maxValue)
    {
        mSoundProperty.type     = 0;
        mSoundProperty.value    = 0;
        mSoundProperty.minValue = SHRT_MIN;
        mSoundProperty.maxValue = SHRT_MAX;
    }

    void push_back(std::vector<gc_SoundProperty_s > &listSoundProperties, gc_SoundProperty_s &property)
    {
        listSoundProperties.push_back(property);
    }

    void expand(void)
    {
        addAttributeList(std::vector<CAmNodeAttribute *>{ &mTypeAttribute, &mValueAttribute, &mMinValueAttribute, &mMaxValueAttribute });
    }

private:
    gc_SoundProperty_s               &mSoundProperty;
    CAmEnumerationAttribute<uint16_t> mTypeAttribute;
    CAmIntAttribute<int16_t>          mValueAttribute;
    CAmIntAttribute<int16_t>          mMaxValueAttribute;
    CAmIntAttribute<int16_t>          mMinValueAttribute;
};

class CAmSystemPropertyNode : public CAmSimpleNode
{
public:
    CAmSystemPropertyNode(const char *tagname, gc_SystemProperty_s &systemProperty, void *data = NULL)
        : CAmSimpleNode(tagname)
        , mSystemProperty(systemProperty)
        , mTypeAttribute(COMMON_ATTRIBUTE_TYPE, mSystemProperty.type)
        , mValueAttribute(COMMON_ATTRIBUTE_VALUE, mSystemProperty.value)
        , mIsPersistenceSupportedAttribute(COMMON_ATTRIBUTE_IS_PERSISTENCE_SUPPORTED, mSystemProperty.isPersistenceSupported)
    {
        (void)data;
        mSystemProperty.type                   = 0;
        mSystemProperty.value                  = 0;
        mSystemProperty.isPersistenceSupported = false;
        addAttributeList(std::vector<CAmNodeAttribute *>{ &mTypeAttribute, &mValueAttribute, &mIsPersistenceSupportedAttribute });
    }

    void push_back(std::vector<gc_SystemProperty_s> &listSystemProperties, gc_SystemProperty_s &property)
    {
        listSystemProperties.push_back(property);
    }

private:
    gc_SystemProperty_s              &mSystemProperty;
    CAmEnumerationAttribute<uint16_t> mTypeAttribute;
    CAmIntAttribute<int16_t>          mValueAttribute;
    CAmBoolAttribute                  mIsPersistenceSupportedAttribute;
};

class CAmMainSoundPropertyNode : public CAmConfigComplexNode
{
public:
    CAmMainSoundPropertyNode(const char *tagname, gc_MainSoundProperty_s &mainSoundProperty, void *data = NULL)
        : CAmConfigComplexNode(tagname)
        , mMainSoundProperty(mainSoundProperty)
        , mTypeAttribute(SOUND_PROPERTY_ATTRIBUTE_TYPE, mMainSoundProperty.type)
        , mValueAttribute(SOUND_PROPERTY_ATTRIBUTE_VALUE, mMainSoundProperty.value)
        , mMaxValueAttribute(SOUND_PROPERTY_ATTRIBUTE_MIN_VALUE, mMainSoundProperty.minValue)
        , mMinValueAttribute(SOUND_PROPERTY_ATTRIBUTE_MAX_VALUE, mMainSoundProperty.maxValue)
        , mIsPersistenceSupportedAttribute(SOUND_PROPERTY_ATTRIBUTE_IS_PERSISTENCE_SUPPORTED, mMainSoundProperty.isPersistenceSupported)
    {
        mMainSoundProperty.type                   = 0;
        mMainSoundProperty.value                  = 0;
        mMainSoundProperty.minValue               = SHRT_MIN;
        mMainSoundProperty.maxValue               = SHRT_MAX;
        mMainSoundProperty.isPersistenceSupported = false;
    }

    void push_back(std::vector<gc_MainSoundProperty_s > &listMainSoundProperty, gc_MainSoundProperty_s &property)
    {
        bool isTypeFound = false;

        for (auto &itListMainSoundProperty : listMainSoundProperty)
        {
            if (itListMainSoundProperty.type == property.type)
            {
                if (mValueAttribute.IsParsed())
                {
                    itListMainSoundProperty.value = property.value;
                }

                if (mIsPersistenceSupportedAttribute.IsParsed())
                {
                    itListMainSoundProperty.isPersistenceSupported = property.isPersistenceSupported;
                }

                if (mMaxValueAttribute.IsParsed())
                {
                    itListMainSoundProperty.maxValue = property.maxValue;
                }

                if (mMinValueAttribute.IsParsed())
                {
                    itListMainSoundProperty.minValue = property.minValue;
                }

                isTypeFound = true;
                break;
            }
        }

        if (isTypeFound == false)
        {
            listMainSoundProperty.push_back(property);
        }
    }

    void expand(void)
    {
        addAttributeList(std::vector<CAmNodeAttribute *>{ &mTypeAttribute, &mValueAttribute, &mMinValueAttribute, &mMaxValueAttribute,
                                                          &mIsPersistenceSupportedAttribute });
    }

private:
    gc_MainSoundProperty_s           &mMainSoundProperty;
    CAmEnumerationAttribute<uint16_t> mTypeAttribute;
    CAmIntAttribute<int16_t>          mValueAttribute;
    CAmIntAttribute<int16_t>          mMaxValueAttribute;
    CAmIntAttribute<int16_t>          mMinValueAttribute;
    CAmBoolAttribute                  mIsPersistenceSupportedAttribute;

};

class CAmNotificationConfigurationNode : public CAmConfigComplexNode
{
public:
    CAmNotificationConfigurationNode(const char *tagname, am_NotificationConfiguration_s &notificationConfiguration, void *mpData = NULL)
        : CAmConfigComplexNode(tagname)
        , mNotificationConfiguration(notificationConfiguration)
        , mTypeAttribute(NOTIFICATION_ATTRIBUTE_TYPE, mNotificationConfiguration.type)
        , mStatusAttribute(NOTIFICATION_ATTRIBUTE_STATUS, mNotificationConfiguration.status)
        , mValueAttribute(NOTIFICATION_ATTRIBUTE_PARAM, mNotificationConfiguration.parameter)
    {
        mNotificationConfiguration = { NT_UNKNOWN, NS_OFF, 0 };
    }

    void push_back(std::vector<am_NotificationConfiguration_s > &listNotificationConfiguration, am_NotificationConfiguration_s &notificationConfiguration)
    {
        listNotificationConfiguration.push_back(notificationConfiguration);
    }

    void expand(void)
    {
        addAttributeList(std::vector<CAmNodeAttribute *>{ &mTypeAttribute, &mStatusAttribute, &mValueAttribute });
    }

private:
    am_NotificationConfiguration_s                  &mNotificationConfiguration;
    CAmEnumerationAttribute<uint16_t>                mTypeAttribute;
    CAmEnumerationAttribute<am_NotificationStatus_e> mStatusAttribute;
    CAmIntAttribute<int16_t> mValueAttribute;
};

class CAmStringParser
{
public:
    CAmStringParser(const std::string &str, const char delimiter = CONFIGURATION_LIST_SEPERATOR)
    {
        std::string inputString = str;
        inputString.erase(remove_if(inputString.begin(), inputString.end(), [](char ch)
                {   return std::isspace<char>(ch, std::locale::classic());}), inputString.end());
        convertSeperateList(inputString, delimiter, mListStrings);
    }

    void setString(const std::string &str, const char delimiter = CONFIGURATION_LIST_SEPERATOR)
    {
        std::string inputString = str;
        inputString.erase(remove_if(inputString.begin(), inputString.end(), [](char ch)
                {   return std::isspace<char>(ch, std::locale::classic());}), inputString.end());
        convertSeperateList(inputString, delimiter, mListStrings);
    }

    void parseConnectionFormats(std::vector<am_CustomConnectionFormat_t> &listConnectionFormats)
    {
        listConnectionFormats.clear();
        for (auto const &it : mListStrings)
        {
            int enumeratedValue;
            if (E_OK == CAmXmlConfigParserCommon::getEnumerationValue(it, enumeratedValue))
            {
                listConnectionFormats.push_back(static_cast<am_CustomConnectionFormat_t>(enumeratedValue));
            }
        }
    }

    void parseTrigger(std::vector<int> &listTrigger)
    {
        listTrigger.clear();
        for (auto const &it : mListStrings)
        {
            int enumeratedValue;
            if (E_OK == CAmXmlConfigParserCommon::getEnumerationValue(it, enumeratedValue))
            {
                listTrigger.push_back(enumeratedValue);
            }
        }
    }

    void parseVolumeMapList(std::map<float, float> &mapVolume)
    {
        mapVolume.clear();
        for (auto const &it : mListStrings)
        {
            float normalizedVolume;
            float decibleVolume;
            if (_parseVolume(it, normalizedVolume, decibleVolume) == E_OK)
            {
                mapVolume[normalizedVolume] = decibleVolume;
            }
        }
    }

    void parseCommaSeperateEnumerationsList(std::vector<std::pair<uint16_t, uint16_t> > &listConvertionmatrix)
    {
        listConvertionmatrix.clear();
        for (auto const &it : mListStrings)
        {
            uint16_t connectionformat1;
            uint16_t connectionformat2;
            if (_parseCommaSeperatedEnums(it, connectionformat1, connectionformat2) == E_OK)
            {
                listConvertionmatrix.push_back(std::make_pair(connectionformat1, connectionformat2));
            }
        }
    }

    static void convertSeperateList(const std::string &inputString, char seperator, std::vector<std::string> &listString)
    {
        listString.clear();
        std::stringstream ss(inputString);
        std::string       token;
        while (std::getline(ss, token, seperator))
        {
            listString.push_back(token);
        }
    }

    static void populateScalingMap(map<float, float > &outMap, const string &valuePairString
        , const std::map<std::string, std::map<float, float> > &listTemplateMapScaleConversions)
    {
        if (valuePairString.empty())
        {
            // nothing specified - silently ignore
            return;
        }

        if (valuePairString.find(',') != string::npos)
        {
            // separator found - parse specified list
            CAmStringParser(valuePairString).parseVolumeMapList(outMap);
        }
        else if (listTemplateMapScaleConversions.count(valuePairString))
        {
            // template with given name found - use values specified there
            outMap = listTemplateMapScaleConversions.at(valuePairString);
        }
        else
        {
            LOG_FN_ERROR(__FILENAME__, "CAmStringParser::populateScalingMap", "NO scaling map found from", valuePairString);
        }
    }

private:
    am_Error_e _parseStringpairs(const std::string &pString, std::string &str1, std::string &str2)
    {
        std::vector<std::string> listString;
        convertSeperateList(pString, ',', listString);
        if (listString.size() == 2)
        {
            str1 = listString[0];
            str2 = listString[1];
            return E_OK;
        }

        return E_NOT_POSSIBLE;
    }

    am_Error_e _parseVolume(const std::string &pString, float &volumeKey, float &volumeValue)
    {
        std::vector<std::string> listString;
        convertSeperateList(pString, ',', listString);
        if (listString.size() == 2)
        {
            volumeKey   = CAmXmlConfigParserCommon::convert<float>(listString[0]);
            volumeValue = CAmXmlConfigParserCommon::convert<float>(listString[1]);
            return E_OK;
        }

        return E_NOT_POSSIBLE;
    }

    am_Error_e _parseCommaSeperatedEnums(const std::string &pString, uint16_t &enum1, uint16_t &enum2)
    {
        std::vector<std::string> listString;
        convertSeperateList(pString, ',', listString);
        if (listString.size() == 2)
        {
            int enumeratedValue;
            if (CAmXmlConfigParserCommon::getEnumerationValue(listString[0], enumeratedValue) == E_OK)
            {
                enum1 = enumeratedValue;
                if (CAmXmlConfigParserCommon::getEnumerationValue(listString[1], enumeratedValue) == E_OK)
                {
                    enum2 = enumeratedValue;
                    return E_OK;
                }
            }
        }

        return E_NOT_POSSIBLE;
    }

    std::vector<std::string> mListStrings;
};

class CAmAvailabilityNode : public CAmConfigComplexNode
{
public:
    CAmAvailabilityNode(const char *tagname, am_Availability_s *pAvailability)
        : CAmConfigComplexNode(tagname)
        , mpAvailability(pAvailability)
        , mAvailabilityAttribute(AVAILABILITY_ATTRIBUTE_AVAILABLE, mpAvailability->availability)
        , mAvailabilityReasonAttribute(AVAILABILITY_ATTRIBUTE_REASON, mpAvailability->availabilityReason)
    {
        mpAvailability->availability       = A_UNKNOWN;
        mpAvailability->availabilityReason = AR_UNKNOWN;
    }

    void expand(void)
    {
        addAttributeList(std::vector<CAmNodeAttribute *>
            { &mAvailabilityAttribute, &mAvailabilityReasonAttribute });
    }

private:
    am_Availability_s                         *mpAvailability;
    CAmEnumerationAttribute<am_Availability_e> mAvailabilityAttribute;
    CAmEnumerationAttribute<uint16_t>          mAvailabilityReasonAttribute;
};

class CAmMSPSPNode : public CAmSimpleNode
{
public:
    CAmMSPSPNode(const char *tagName, am_CustomMainSoundPropertyType_t *pMainSoundProperty, am_CustomSoundPropertyType_t *pSoundProperty, gc_MSPMappingDirection_e *pMappingDirection)
        : CAmSimpleNode(tagName)
        , mpMainSoundProperty(pMainSoundProperty)
        , mpSoundProperty(pSoundProperty)
        , mpMappingDirection(pMappingDirection)
        , mMapAttribute(MSPSP_MAP_ATTRIBUTE_TYPE, mMappingDirectionString)
    {
        *mpMainSoundProperty = 0;
        *mpSoundProperty     = 0;
        *mpMappingDirection  = MD_BOTH;
        addAttribute(&mMapAttribute);
    }

    void _saveData(const std::string &pString)
    {
        std::vector<std::string> listString;
        CAmStringParser::convertSeperateList(pString, ',', listString);
        if (listString.size() != 2)
        {
            return;
        }

        int enumeratedValue;
        if (E_OK == CAmXmlConfigParserCommon::getEnumerationValue(listString[0], enumeratedValue))
        {
            *mpMainSoundProperty = enumeratedValue;
        }

        if (E_OK == CAmXmlConfigParserCommon::getEnumerationValue(listString[1], enumeratedValue))
        {
            *mpSoundProperty = enumeratedValue;
        }

        if (E_OK == CAmXmlConfigParserCommon::getEnumerationValue(mMappingDirectionString, enumeratedValue))
        {
            *mpMappingDirection = (gc_MSPMappingDirection_e)enumeratedValue;
        }
    }

private:
    am_CustomMainSoundPropertyType_t *mpMainSoundProperty;
    am_CustomSoundPropertyType_t     *mpSoundProperty;
    gc_MSPMappingDirection_e         *mpMappingDirection;
    std::string mMappingDirectionString;
    CAmStringAttribute                mMapAttribute;
}

;

class CAmMSPToSPMapListNode : public CAmXmlNode
{
public:
    CAmMSPToSPMapListNode(const char *tagName, std::map<gc_MSPMappingDirection_e, std::map<uint16_t, uint16_t> > *pMSPMap)
        : mpTagName(tagName)
        , mpMSPMap(pMSPMap)
    {
        mMappingDirection = MD_BOTH;
    }

    int parse(const xmlDocPtr pDocument, xmlNodePtr *pCurrent)
    {
        int                                                               returnValue = PARSE_TAG_MISMATCH;
        std::map<gc_MSPMappingDirection_e, std::map<uint16_t, uint16_t> > MSPMap;
        am_CustomMainSoundPropertyType_t                                  mainSoundProperty;
        am_CustomSoundPropertyType_t                                      soundProperty;
        do
        {
            if ((*pCurrent == NULL) || (xmlStrEqual((*pCurrent)->name, reinterpret_cast<const xmlChar *>(mpTagName)) == 0))
            {
                break;
            }

            CAmMSPSPNode mapEntryNode(MSPSP_MAP_TAG_TYPE, &mainSoundProperty, &soundProperty, &mMappingDirection);
            returnValue = mapEntryNode.parse(pDocument, pCurrent);
            if (returnValue == PARSE_SUCCESS)
            {
                if (mMappingDirection == MD_MSP_TO_SP)
                {
                    MSPMap[MD_MSP_TO_SP].insert(std::make_pair(mainSoundProperty, soundProperty));
                }
                else if (mMappingDirection == MD_SP_TO_MSP)
                {
                    MSPMap[MD_SP_TO_MSP].insert(std::make_pair(soundProperty, mainSoundProperty));
                }
                else
                {
                    MSPMap[MD_MSP_TO_SP].insert(std::make_pair(mainSoundProperty, soundProperty));
                    MSPMap[MD_SP_TO_MSP].insert(std::make_pair(soundProperty, mainSoundProperty));

                }
            }

            *pCurrent = getNextElementNode(*pCurrent);
        } while (returnValue == 0);

        if (MSPMap.size() > 0)
        {
            *mpMSPMap = MSPMap;
        }

        return returnValue;
    }

private:
    const char              *mpTagName;
    gc_MSPMappingDirection_e mMappingDirection;
    std::map<gc_MSPMappingDirection_e, std::map<uint16_t, uint16_t> > *mpMSPMap;
};

class CAmGatewayNode : public CAmConfigComplexNode
{
public:
    CAmGatewayNode(const char *tagName, gc_Gateway_s &gateway, void *data = NULL)
        : CAmConfigComplexNode(tagName)
        , mGateway(gateway)
        , mBaseName("")
        , mpConfig((gc_Configuration_s *)data)
        , mIDAttribute(ELEMENT_COMMON_ATTRIBUTE_ID, mGateway.gatewayID)
        , mNameAttribute(ELEMENT_COMMON_ATTRIBUTE_NAME, mGateway.name)
        , mSinkNameAttribute(GATEWAY_ATTRIBUTE_SINK_NAME, mGateway.sinkName)
        , mSourceNameAttribute(GATEWAY_ATTRIBUTE_SOURCE_NAME, mGateway.sourceName)
        , mControlDomainAttribute(GATEWAY_ATTRIBUTE_CONTROL_DOMAIN_NAME, mGateway.controlDomainName)
        , mRegistrationTypeAttribute(ELEMENT_COMMON_TAG_REGISTRATION_TYPE, mGateway.registrationType)
        , mConversionMatrixAttribute(GATEWAY_TAG_CONVERSION_MATRIX, mConversionMatrix)
        , mBaseNameAttribute(ELEMENT_COMMON_ATTRIBUTE_BASENAME, mBaseName)
    {
        mGateway.gatewayID        = 0;
        mGateway.sinkID           = 0;
        mGateway.sourceID         = 0;
        mGateway.domainSinkID     = 0;
        mGateway.domainSourceID   = 0;
        mGateway.controlDomainID  = 0;
        mGateway.registrationType = REG_ROUTER;
        mGateway.sinkName         = "";
        mGateway.sourceName       = "";
        mGateway.convertionMatrix.clear();
        mGateway.listConvertionmatrix.clear();
    }

    void push_back(std::vector<gc_Gateway_s > &listGateway, gc_Gateway_s &gateway)
    {
        listGateway.push_back(gateway);
    }

    void expand()
    {
        addAttributeList(std::vector<CAmNodeAttribute *>
            { &mIDAttribute, &mNameAttribute, &mSinkNameAttribute, &mSourceNameAttribute, &mControlDomainAttribute, &mRegistrationTypeAttribute, &mConversionMatrixAttribute, &mBaseNameAttribute });
    }

protected:
    virtual am_Error_e _copyComplexData(void)
    {
        if (mConversionMatrix != "")
        {
            CAmStringParser stringParser(mConversionMatrix);
            stringParser.parseCommaSeperateEnumerationsList(mGateway.listConvertionmatrix);
        }

        if (mpConfig != NULL)
        {
            if ((mGateway.sinkName == "") || (mpConfig->mCurentSink != ""))
            {
                mGateway.sinkName = mpConfig->mCurentSink;
            }

            if ((mGateway.sourceName == "") || (mpConfig->mCurrentSource != ""))
            {
                mGateway.sourceName = mpConfig->mCurrentSource;
            }

            if ((mGateway.controlDomainName == "") || (mpConfig->mCurrentDomain != ""))
            {
                mGateway.controlDomainName = mpConfig->mCurrentDomain;
            }
        }

        return E_OK;
    }

    void _getConversionMatrix()
    {
        if ((mpConfig != NULL) && (mConversionMatrixTemplateName != ""))
        {
            mGateway.listConvertionmatrix = mpConfig->listTemplateConversionMatrix[mConversionMatrixTemplateName];
        }
    }

    void _getDefaultFromTemplate()
    {
        if (mpConfig == NULL)
        {
            return;
        }

        for (const auto &itListTemplateGateway : mpConfig->listTemplateGateways)
        {
            if (itListTemplateGateway.name == mBaseName)
            {
                mGateway = itListTemplateGateway;
                break;
            }
        }
    }

private:
    gc_Gateway_s        &mGateway;
    std::string          mConversionMatrix;
    std::string          mBaseName;
    std::string          mConversionMatrixTemplateName;
    gc_Configuration_s  *mpConfig;
    CAmStringIDAttribute mIDAttribute;
    CAmStringAttribute   mNameAttribute;
    CAmStringAttribute   mSinkNameAttribute;
    CAmStringAttribute   mSourceNameAttribute;
    CAmStringAttribute   mControlDomainAttribute;
    CAmEnumerationAttribute<gc_Registration_e> mRegistrationTypeAttribute;
    CAmStringAttribute   mConversionMatrixAttribute;
    CAmStringAttribute   mBaseNameAttribute;
};

class CAmSourceNode : public CAmConfigComplexNode
{
public:
    CAmSourceNode(const char *tagname, gc_Source_s &source, void *data = NULL)
        : CAmConfigComplexNode(tagname)
        , mSource(source)
        , mpConfig((gc_Configuration_s *)data)
        , mNotificationcationConfigurationNode(XML_ELEMENT_TAG_NOTIFICATION_CONFIG, mSource.listNotificationConfigurations)
        , mMainNotificationcationConfigurationNode(XML_ELEMENT_TAG_MAIN_NOTIFICATION_CONFIG, mSource.listMainNotificationConfigurations)
        , mAvailabilityNode(ELEMENT_COMMON_TAG_AVAILABLE, (&mSource.available))
        , mSoundPropertyNode(XML_ELEMENT_TAG_SOUND_PROPERTY, mSource.listGCSoundProperties)
        , mMainSoundPropertyNode(XML_ELEMENT_TAG_MAIN_SOUND_PROPERTY, mSource.listGCMainSoundProperties)
        , mMSPToSPMapNode(MSPSP_MAP_TAG_TYPE, &(mSource.mapMSPTOSP))
        , mListGatewayNode(XML_ELEMENT_TAG_GATEWAY, mpConfig->listGateways, mpConfig)
        , mIDAttribute(ELEMENT_COMMON_ATTRIBUTE_ID, mSource.sourceID)
        , mDomainNameAttribute(ELEMENT_COMMON_ATTRIBUTE_DOMAIN_NAME, mSource.domainName)
        , mNameAttribute(ELEMENT_COMMON_ATTRIBUTE_NAME, mSource.name)
        , mVolumeAttribute(ELEMENT_COMMON_ATTRIBUTE_VOLUME, mSource.volume)
        , mVisibleAttribute(ELEMENT_COMMON_ATTRIBUTE_VISIBLE, mSource.visible)
        , mClassNameAttribute(ELEMENT_COMMON_ATTRIBUTE_CLASS_NAME, mSource.className)
        , mStateAttribute(SOURCE_ATTRIBUTE_STATE, mSource.sourceState)
        , mInterruptStateAttribute(SOURCE_ATTRIBUTE_INTERRUPT_STATE, mSource.interruptState)
        , mConnectionFormatAttribute(XML_ELEMENT_ATTRIBUTE_CONNECTION_FORMAT, mConnectionFormats)
        , mIsVolumeSupportedAttribute(ELEMENT_COMMON_ATTRIBUTE_IS_VOLUME_SUPPORTED, mSource.isVolumeChangeSupported)
        , mIsPersistencySupportedAttribute(ELEMENT_COMMON_ATTRIBUTE_IS_PERSISTENCY_SUPPORTED, mSource.isPersistencySupported)
        , mIsVolumePeristenceSupported(ELEMENT_COMMON_ATTRIBUTE_IS_VOLUME_PERSISTENCY_SUPPORTED, mSource.isVolumePersistencySupported)
        , mPriorityAttribute(COMMON_ATTRIBUTE_PRIORITY, mSource.priority)
        , mRegistrationTypeAttribute(ELEMENT_COMMON_TAG_REGISTRATION_TYPE, mSource.registrationType)
        , mMinVolumeAttribute(ELEMENT_COMMON_ATTRIBUTE_MIN_VOLUME, mSource.minVolume)
        , mMaxVolumeAttribute(ELEMENT_COMMON_ATTRIBUTE_MAX_VOLUME, mSource.maxVolume)
        , mVolumeMapAttribute(SOURCE_TAG_VOLUME_MAP, mSourceVolumeMap)
        , mBaseNameAttribute(ELEMENT_COMMON_ATTRIBUTE_BASENAME, mBaseName)
        , mMSPToSPBaseNameAttribute(ELEMENT_COMMON_ATTRIBUTE_MSP_SP, mMapMSPToSPTemplateName)
        , mSoundPropertiesBaseNameAttribute(ELEMENT_COMMON_ATTRIBUTE_SP_LIST_NAME, mSoundPropertiesTemplateName)
        , mMainSoundPropertiesBaseNameAttribute(ELEMENT_COMMON_ATTRIBUTE_MSP_LIST_NAME, mMainSoundPropertiesTemplateName)
        , mNotificationConfigurationBaseNameAttribute(ELEMENT_COMMON_ATTRIBUTE_NC_NAME, mNotificationConfigurationsTemplateName)
        , mMainNotificationConfigurationBaseNameAttribute(ELEMENT_COMMON_ATTRIBUTE_MNC_NAME, mMainNotificationConfigurationTemplateName)
        , mConnectionFormatBaseNameAttribute(ELEMENT_COMMON_ATTRIBUTE_CF_NAME, mConnectionFormatsTemplateName)
    {
        mSource.sourceClassID                = 0;
        mSource.sourceID                     = 0;
        mSource.sourceState                  = SS_UNKNNOWN;
        mSource.interruptState               = IS_UNKNOWN;
        mSource.priority                     = DEFAULT_CONFIG_ELEMENT_PRIORITY;
        mSource.domainID                     = 0;
        mSource.volume                       = 0;
        mSource.visible                      = true;
        mSource.available.availability       = A_AVAILABLE;
        mSource.available.availabilityReason = AR_UNKNOWN;
        mSource.isVolumeChangeSupported      = true;
        mSource.isPersistencySupported       = false;
        mSource.isVolumePersistencySupported = false;
        mSource.registrationType             = REG_ROUTER;
        mSource.listConnectionFormats.clear();
        mSource.listGCMainSoundProperties.clear();
        mSource.listGCSoundProperties.clear();
        mSource.listMainNotificationConfigurations.clear();
        mSource.listNotificationConfigurations.clear();
        mSource.minVolume = 0;
        mSource.maxVolume = 0;
        mSource.mapRoutingVolume.clear();
    }

    void push_back(std::vector<gc_Source_s > &listSource, gc_Source_s &source)
    {
        listSource.push_back(source);
    }

    void expand()
    {
        addChildNodeList(std::vector<CAmXmlNode *>{ &mNotificationcationConfigurationNode,
                                                    &mNotificationcationConfigurationNode, &mMainNotificationcationConfigurationNode,
                                                    &mAvailabilityNode, &mSoundPropertyNode, &mMainSoundPropertyNode, &mMSPToSPMapNode });
        if (mpConfig != NULL)
        {
            addChildNode(&mListGatewayNode);
        }

        addAttributeList(std::vector<CAmNodeAttribute *>{ &mIDAttribute, &mDomainNameAttribute, &mNameAttribute,
                                                          &mVolumeAttribute, &mVisibleAttribute, &mClassNameAttribute, &mStateAttribute,
                                                          &mInterruptStateAttribute, &mConnectionFormatAttribute, &mIsVolumeSupportedAttribute,
                                                          &mIsVolumePeristenceSupported, &mPriorityAttribute, &mRegistrationTypeAttribute,
                                                          &mIsPersistencySupportedAttribute,
                                                          &mMinVolumeAttribute, &mMaxVolumeAttribute, &mVolumeMapAttribute, &mBaseNameAttribute,
                                                          &mMSPToSPBaseNameAttribute, &mSoundPropertiesBaseNameAttribute,
                                                          &mMainSoundPropertiesBaseNameAttribute, &mNotificationConfigurationBaseNameAttribute,
                                                          &mMainNotificationConfigurationBaseNameAttribute, &mConnectionFormatBaseNameAttribute });
    }

protected:
    void _getDefaultFromTemplate()
    {
        if (mpConfig == NULL)
        {
            return;
        }

        for (const auto &itListTemplateSources : mpConfig->listTemplateSources)
        {
            if (itListTemplateSources.name == mBaseName)
            {
                mSource = itListTemplateSources;
                break;
            }
        }

        if ((mSource.domainName == "") && (mpConfig->mCurrentDomain != ""))
        {
            mSource.domainName = mpConfig->mCurrentDomain;
        }
    }

    void _storeCurrentName()
    {
        if (mpConfig != NULL)
        {
            mpConfig->mCurrentSource = mSource.name;
        }
    }

    void _getSoundPropertyList(void)
    {
        if ((mpConfig != NULL) && (mSoundPropertiesTemplateName != ""))
        {
            mSource.listGCSoundProperties = mpConfig->listTemplateSoundProperties[mSoundPropertiesTemplateName];
        }
    }

    void _getMainSoundPropertyList(void)
    {
        if ((mpConfig != NULL) && (mMainSoundPropertiesTemplateName != ""))
        {
            mSource.listGCMainSoundProperties = mpConfig->listTemplateMainSoundProperties[mMainSoundPropertiesTemplateName];
        }
    }

    void _getNotificationConfiguration(void)
    {
        if ((mpConfig != NULL) && (mNotificationConfigurationsTemplateName != ""))
        {
            mSource.listNotificationConfigurations = mpConfig->listTemplateNotificationConfigurations[mNotificationConfigurationsTemplateName];
        }
    }

    void _getMainNotificationConfiguration(void)
    {
        if ((mpConfig != NULL) && (mMainNotificationConfigurationTemplateName != ""))
        {
            mSource.listMainNotificationConfigurations = mpConfig->listTemplateNotificationConfigurations[mMainNotificationConfigurationTemplateName];
        }
    }

    void _getConnectionFormats(void)
    {
        if ((mpConfig != NULL) && (mConnectionFormatsTemplateName != ""))
        {
            mSource.listConnectionFormats = mpConfig->listTemplateConnectionFormats[mConnectionFormatsTemplateName];
        }
    }

    void _getMapMSPToSP(void)
    {
        if ((mpConfig != NULL) && (mMapMSPToSPTemplateName != ""))
        {
            mSource.mapMSPTOSP = mpConfig->listTemplateMapMSPToSP[mMapMSPToSPTemplateName];
        }
    }

    virtual am_Error_e _copyComplexData(void)
    {
        CAmStringParser stringParser(mConnectionFormats);
        if (mConnectionFormats != "")
        {
            stringParser.parseConnectionFormats(mSource.listConnectionFormats);
        }

        if (mpConfig != NULL)
        {
            CAmStringParser::populateScalingMap(mSource.mapRoutingVolume, mSourceVolumeMap
                    , mpConfig->listTemplateMapScaleConversions);

            mpConfig->mCurrentSource = "";
            mSource.domainName       = mpConfig->mCurrentDomain;
        }

        return E_OK;
    }

private:
    gc_Source_s        &mSource;
    std::string         mConnectionFormats;
    std::string         mSourceVolumeMap;
    gc_Configuration_s *mpConfig;
    std::string         mBaseName;
    std::string         mMapMSPToSPTemplateName;
    std::string         mSoundPropertiesTemplateName;
    std::string         mMainSoundPropertiesTemplateName;
    std::string         mNotificationConfigurationsTemplateName;
    std::string         mMainNotificationConfigurationTemplateName;
    std::string         mConnectionFormatsTemplateName;
    CAmListNode<am_NotificationConfiguration_s, CAmNotificationConfigurationNode> mNotificationcationConfigurationNode;
    CAmListNode<am_NotificationConfiguration_s, CAmNotificationConfigurationNode> mMainNotificationcationConfigurationNode;
    CAmAvailabilityNode mAvailabilityNode;
    CAmListNode<gc_SoundProperty_s, CAmSoundPropertyNode>         mSoundPropertyNode;
    CAmListNode<gc_MainSoundProperty_s, CAmMainSoundPropertyNode> mMainSoundPropertyNode;
    CAmMSPToSPMapListNode mMSPToSPMapNode;
    CAmListNode<gc_Gateway_s, CAmGatewayNode>                     mListGatewayNode;
    CAmStringIDAttribute                         mIDAttribute;
    CAmStringAttribute                           mDomainNameAttribute;
    CAmStringAttribute                           mNameAttribute;
    CAmIntAttribute<int16_t>                     mVolumeAttribute;
    CAmBoolAttribute                             mVisibleAttribute;
    CAmStringAttribute                           mClassNameAttribute;
    CAmEnumerationAttribute<am_SourceState_e>    mStateAttribute;
    CAmEnumerationAttribute<am_InterruptState_e> mInterruptStateAttribute;
    CAmStringAttribute                           mConnectionFormatAttribute;
    CAmBoolAttribute                             mIsVolumeSupportedAttribute;
    CAmBoolAttribute                             mIsPersistencySupportedAttribute;
    CAmBoolAttribute                             mIsVolumePeristenceSupported;
    CAmIntAttribute<int32_t>                     mPriorityAttribute;
    CAmEnumerationAttribute<gc_Registration_e>   mRegistrationTypeAttribute;
    CAmIntAttribute<int16_t>                     mMinVolumeAttribute;
    CAmIntAttribute<int16_t>                     mMaxVolumeAttribute;
    CAmStringAttribute                           mVolumeMapAttribute;
    CAmStringAttribute                           mBaseNameAttribute;
    CAmStringAttribute                           mMSPToSPBaseNameAttribute;
    CAmStringAttribute                           mSoundPropertiesBaseNameAttribute;
    CAmStringAttribute                           mMainSoundPropertiesBaseNameAttribute;
    CAmStringAttribute                           mNotificationConfigurationBaseNameAttribute;
    CAmStringAttribute                           mMainNotificationConfigurationBaseNameAttribute;
    CAmStringAttribute                           mConnectionFormatBaseNameAttribute;

};

class CAmSinkNode : public CAmConfigComplexNode
{
public:
    CAmSinkNode(const char *tagName, gc_Sink_s &sink, void *data = NULL)
        : CAmConfigComplexNode(tagName)
        , mSink(sink)
        , mpConfig((gc_Configuration_s *)data)
        , mListNotificationConfigurationNode(XML_ELEMENT_TAG_NOTIFICATION_CONFIG, mSink.listMainNotificationConfigurations)
        , mListMainNotificationConfigurationNode(XML_ELEMENT_TAG_MAIN_NOTIFICATION_CONFIG, mSink.listNotificationConfigurations)
        , mAvailabilityNode(ELEMENT_COMMON_TAG_AVAILABLE, &(mSink.available))
        , mlistSoundPropertiesNode(XML_ELEMENT_TAG_SOUND_PROPERTY, mSink.listGCSoundProperties)
        , mListMainSoundPropertyNode(XML_ELEMENT_TAG_MAIN_SOUND_PROPERTY, mSink.listGCMainSoundProperties)
        , mListMSPToSPNode(MSPSP_MAP_TAG_TYPE, &(mSink.mapMSPTOSP))
        , mListGatewayNode(XML_ELEMENT_TAG_GATEWAY, mpConfig->listGateways, mpConfig)
        , mIDAttribute(ELEMENT_COMMON_ATTRIBUTE_ID, mSink.sinkID)
        , mDomainNameAttribute(ELEMENT_COMMON_ATTRIBUTE_DOMAIN_NAME, mSink.domainName)
        , mSinkNameAttribute(ELEMENT_COMMON_ATTRIBUTE_NAME, mSink.name)
        , mVolumeAttribute(ELEMENT_COMMON_ATTRIBUTE_VOLUME, mSink.volume)
        , mVisibleAttribute(ELEMENT_COMMON_ATTRIBUTE_VISIBLE, mSink.visible)
        , mMainVolumeAttribute(SINK_ATTRIBUTE_MAIN_VOLUME, mSink.mainVolume)
        , mClassNameAttribute(ELEMENT_COMMON_ATTRIBUTE_CLASS_NAME, mSink.className)
        , mMuteStateAttribute(SINK_ATTRIBUTE_MUTE_STATE, mSink.muteState)
        , mConnectionFormatAttribute(XML_ELEMENT_ATTRIBUTE_CONNECTION_FORMAT, mConnectionFormats)
        , mRegistrationTypeAttribute(ELEMENT_COMMON_TAG_REGISTRATION_TYPE, mSink.registrationType)
        , mIsVolumeSupportedAttribute(ELEMENT_COMMON_ATTRIBUTE_IS_VOLUME_SUPPORTED, mSink.isVolumeChangeSupported)
        , mIsPersistenceSupportedAttribute(ELEMENT_COMMON_ATTRIBUTE_IS_PERSISTENCY_SUPPORTED, mSink.isPersistencySupported)
        , mIsVolumePersistenceSupportedAttribute(ELEMENT_COMMON_ATTRIBUTE_IS_VOLUME_PERSISTENCY_SUPPORTED, mSink.isVolumePersistencySupported)
        , mPriorityAttribute(COMMON_ATTRIBUTE_PRIORITY, mSink.priority)
        , mVolumeMapAttribute(SINK_TAG_VOLUME_MAP, mRoutingVolumeMap)
        , mBaseNameAttribute(ELEMENT_COMMON_ATTRIBUTE_BASENAME, mBaseName)
        , mMapMSPToSPTemplateNameAttribute(ELEMENT_COMMON_ATTRIBUTE_MSP_SP, mMapMSPToSPTemplateName)
        , mMapSPListTemplateNameAttribute(ELEMENT_COMMON_ATTRIBUTE_SP_LIST_NAME, mSoundPropertiesTemplateName)
        , mMapMSPListTemplateNameAttribute(ELEMENT_COMMON_ATTRIBUTE_MSP_LIST_NAME, mMainSoundPropertiesTemplateName)
        , mNotificationConfigurationTemplateNameAttribute(ELEMENT_COMMON_ATTRIBUTE_NC_NAME, mNotificationConfigurationsTemplateName)
        , mMainNotificationConfigurationTemplateNameAttribute(ELEMENT_COMMON_ATTRIBUTE_MNC_NAME, mMainNotificationConfigurationTemplateName)
        , mConnectionFormatTemplateNameAttribute(ELEMENT_COMMON_ATTRIBUTE_CF_NAME, mConnectionFormatsTemplateName)
        , mUserToNormTemplateNameAttribute(COMMON_ATTRIBUTE_VOLUME_USER_TO_NORM, mUserVolumeMap)
        , mNormToUserTemplateNameAttribute(COMMON_ATTRIBUTE_VOLUME_NORM_TO_USER, mNormalizedVolumeMap)
    {
        mSink.sinkID                       = 0;
        mSink.sinkClassID                  = 0;
        mSink.muteState                    = MS_UNMUTED;
        mSink.mainVolume                   = 0;
        mSink.priority                     = DEFAULT_CONFIG_ELEMENT_PRIORITY;
        mSink.domainID                     = 0;
        mSink.volume                       = 0;
        mSink.visible                      = true;
        mSink.available.availability       = A_AVAILABLE;
        mSink.available.availabilityReason = AR_UNKNOWN;
        mSink.isVolumeChangeSupported      = true;
        mSink.isPersistencySupported       = false;
        mSink.isVolumePersistencySupported = false;
        mSink.registrationType             = REG_ROUTER;
        mSink.listConnectionFormats.clear();
        mSink.listGCMainSoundProperties.clear();
        mSink.listGCSoundProperties.clear();
        mSink.listMainNotificationConfigurations.clear();
        mSink.listNotificationConfigurations.clear();
        mSink.mapNormalizedVolumeToDecibelVolume.clear();
        mSink.mapUserVolumeToNormalizedVolume.clear();
        if (mpConfig != NULL)
        {
            mSink.domainName = mpConfig->mCurrentDomain;
        }
    }

    void push_back(std::vector<gc_Sink_s > &listSink, gc_Sink_s &sink)
    {
        listSink.push_back(sink);
    }

    void expand(void)
    {
        addChildNodeList(std::vector<CAmXmlNode *>{ &mListNotificationConfigurationNode,
                                                    &mListMainNotificationConfigurationNode, &mAvailabilityNode, &mlistSoundPropertiesNode,
                                                    &mListMainSoundPropertyNode, &mListMSPToSPNode });
        if (mpConfig != NULL)
        {
            addChildNode(&mListGatewayNode);
        }

        addAttributeList(std::vector<CAmNodeAttribute *>{ &mIDAttribute, &mDomainNameAttribute,
                                                          &mSinkNameAttribute, &mVolumeAttribute, &mVisibleAttribute, &mMainVolumeAttribute,
                                                          &mClassNameAttribute, &mMuteStateAttribute, &mConnectionFormatAttribute, &mRegistrationTypeAttribute,
                                                          &mIsVolumeSupportedAttribute, &mIsPersistenceSupportedAttribute, &mIsVolumePersistenceSupportedAttribute,
                                                          &mPriorityAttribute, &mVolumeMapAttribute, &mBaseNameAttribute, &mMapMSPToSPTemplateNameAttribute,
                                                          &mMapSPListTemplateNameAttribute, &mMapMSPListTemplateNameAttribute,
                                                          &mNotificationConfigurationTemplateNameAttribute, &mMainNotificationConfigurationTemplateNameAttribute,
                                                          &mConnectionFormatTemplateNameAttribute, &mUserToNormTemplateNameAttribute,
                                                          &mNormToUserTemplateNameAttribute });
    }

protected:
    void _storeCurrentName()
    {
        if (mpConfig != NULL)
        {
            mpConfig->mCurentSink = mSink.name;
        }
    }

    void _getDefaultFromTemplate()
    {
        if (mpConfig == NULL)
        {
            return;
        }

        for (const auto &itListTemplateSinks : mpConfig->listTemplateSinks)
        {
            if (itListTemplateSinks.name == mBaseName)
            {
                mSink = itListTemplateSinks;
                break;
            }
        }

        if ((mSink.domainName == "") && (mpConfig->mCurrentDomain != ""))
        {
            mSink.domainName = mpConfig->mCurrentDomain;
        }
    }

    virtual am_Error_e _copyComplexData(void)
    {
        CAmStringParser stringParser(mConnectionFormats);
        if (mConnectionFormats != "")
        {
            stringParser.parseConnectionFormats(mSink.listConnectionFormats);
        }

        if (mpConfig != NULL)
        {
            CAmStringParser::populateScalingMap(mSink.mapNormalizedVolumeToDecibelVolume
                    , mNormalizedVolumeMap, mpConfig->listTemplateMapScaleConversions);

            CAmStringParser::populateScalingMap(mSink.mapUserVolumeToNormalizedVolume
                    , mUserVolumeMap, mpConfig->listTemplateMapScaleConversions);

            CAmStringParser::populateScalingMap(mSink.mapRoutingVolume
                    , mRoutingVolumeMap, mpConfig->listTemplateMapScaleConversions);

            mpConfig->mCurentSink = "";
            mSink.domainName      = mpConfig->mCurrentDomain;
        }

        return E_OK;
    }

    void _getSoundPropertyList(void)
    {
        if ((mpConfig != NULL) && (mSoundPropertiesTemplateName != ""))
        {
            mSink.listGCSoundProperties = mpConfig->listTemplateSoundProperties[mSoundPropertiesTemplateName];
        }
    }

    void _getMainSoundPropertyList(void)
    {
        if ((mpConfig != NULL) && (mMainSoundPropertiesTemplateName != ""))
        {
            mSink.listGCMainSoundProperties = mpConfig->listTemplateMainSoundProperties[mMainSoundPropertiesTemplateName];
        }
    }

    void _getNotificationConfiguration(void)
    {
        if ((mpConfig != NULL) && (mNotificationConfigurationsTemplateName != ""))
        {
            mSink.listNotificationConfigurations = mpConfig->listTemplateNotificationConfigurations[mNotificationConfigurationsTemplateName];
        }
    }

    void _getMainNotificationConfiguration(void)
    {
        if ((mpConfig != NULL) && (mMainNotificationConfigurationTemplateName != ""))
        {
            mSink.listMainNotificationConfigurations = mpConfig->listTemplateNotificationConfigurations[mMainNotificationConfigurationTemplateName];
        }
    }

    void _getConnectionFormats(void)
    {
        if ((mpConfig != NULL) && (mConnectionFormatsTemplateName != ""))
        {
            mSink.listConnectionFormats = mpConfig->listTemplateConnectionFormats[mConnectionFormatsTemplateName];
        }
    }

    void _getMapMSPToSP(void)
    {
        if ((mpConfig != NULL) && (mMapMSPToSPTemplateName != ""))
        {
            mSink.mapMSPTOSP = mpConfig->listTemplateMapMSPToSP[mMapMSPToSPTemplateName];
        }
    }

private:
    gc_Sink_s          &mSink;
    gc_Configuration_s *mpConfig;
    std::string         mConnectionFormats;
    std::string         mNormalizedVolumeMap;
    std::string         mUserVolumeMap;
    std::string         mRoutingVolumeMap;
    std::string         mBaseName;
    std::string         mMapMSPToSPTemplateName;
    std::string         mSoundPropertiesTemplateName;
    std::string         mMainSoundPropertiesTemplateName;
    std::string         mMainVolumesToNormalizedVolumesTemplateName;
    std::string         mNormalizedVolumesToDecibelVolumesTemplateName;
    std::string         mNotificationConfigurationsTemplateName;
    std::string         mMainNotificationConfigurationTemplateName;
    std::string         mConnectionFormatsTemplateName;
    CAmListNode<am_NotificationConfiguration_s, CAmNotificationConfigurationNode> mListNotificationConfigurationNode;
    CAmListNode<am_NotificationConfiguration_s, CAmNotificationConfigurationNode> mListMainNotificationConfigurationNode;
    CAmAvailabilityNode mAvailabilityNode;
    CAmListNode<gc_SoundProperty_s, CAmSoundPropertyNode>         mlistSoundPropertiesNode;
    CAmListNode<gc_MainSoundProperty_s, CAmMainSoundPropertyNode> mListMainSoundPropertyNode;
    //CAmListNode<gc_SystemProperty_s, CAmSystemPropertyNode>     mListSystemPropertiesNode;
    CAmListNode<gc_Gateway_s, CAmGatewayNode>                     mListGatewayNode;
    CAmMSPToSPMapListNode                      mListMSPToSPNode;
    CAmStringIDAttribute                       mIDAttribute;
    CAmStringAttribute                         mDomainNameAttribute;
    CAmStringAttribute                         mSinkNameAttribute;
    CAmIntAttribute<int16_t>                   mVolumeAttribute;
    CAmBoolAttribute                           mVisibleAttribute;
    CAmIntAttribute<int16_t>                   mMainVolumeAttribute;
    CAmStringAttribute                         mClassNameAttribute;
    CAmEnumerationAttribute<am_MuteState_e>    mMuteStateAttribute;
    CAmStringAttribute                         mConnectionFormatAttribute;
    CAmEnumerationAttribute<gc_Registration_e> mRegistrationTypeAttribute;
    CAmBoolAttribute                           mIsVolumeSupportedAttribute;
    CAmBoolAttribute                           mIsPersistenceSupportedAttribute;
    CAmBoolAttribute                           mIsVolumePersistenceSupportedAttribute;
    CAmIntAttribute<int32_t>                   mPriorityAttribute;
    CAmStringAttribute                         mVolumeMapAttribute;
    CAmStringAttribute                         mBaseNameAttribute;
    CAmStringAttribute                         mMapMSPToSPTemplateNameAttribute;
    CAmStringAttribute                         mMapSPListTemplateNameAttribute;
    CAmStringAttribute                         mMapMSPListTemplateNameAttribute;
    CAmStringAttribute                         mNotificationConfigurationTemplateNameAttribute;
    CAmStringAttribute                         mMainNotificationConfigurationTemplateNameAttribute;
    CAmStringAttribute                         mConnectionFormatTemplateNameAttribute;
    CAmStringAttribute                         mUserToNormTemplateNameAttribute;
    CAmStringAttribute                         mNormToUserTemplateNameAttribute;
};

class CAmDomainNode : public CAmConfigComplexNode
{
public:
    CAmDomainNode(const char *tagName, gc_Domain_s &domain, void *data = NULL)
        : CAmConfigComplexNode(tagName)
        , mDomain(domain)
        , mBaseName("")
        , mpConfig((gc_Configuration_s *)data)
        , mListSourceNode(XML_ELEMENT_TAG_SOURCE, mpConfig->listSources, mpConfig)
        , mListSinkNode(XML_ELEMENT_TAG_SINK, mpConfig->listSinks, mpConfig)
        , mListGatewayNode(XML_ELEMENT_TAG_GATEWAY, mpConfig->listGateways, mpConfig)
        , mIDAttribute(ELEMENT_COMMON_ATTRIBUTE_ID, mDomain.domainID)
        , mNameAttribute(ELEMENT_COMMON_ATTRIBUTE_NAME, mDomain.name)
        , mBusNameAttribute(DOMAIN_ATTRIBUTE_BUS_NAME, mDomain.busname)
        , mNodeNameAttribute(DOMAIN_ATTRIBUTE_NODE_NAME, mDomain.nodename)
        , mEarlyAttribute(DOMAIN_ATTRIBUTE_EARLY, mDomain.early)
        , mCompleteAttribute(DOMAIN_ATTRIBUTE_COMPLETE, mDomain.complete)
        , mDomainStateAttribute(DOMAIN_ATTRIBUTE_STATE, mDomain.state)
        , mRegistrationTypeAttribute(ELEMENT_COMMON_TAG_REGISTRATION_TYPE, mDomain.registrationType)
        , mBaseNameAttribute(ELEMENT_COMMON_ATTRIBUTE_BASENAME, mBaseName)
    {
        mDomain.domainID         = 0;
        mDomain.complete         = false;
        mDomain.early            = false;
        mDomain.registrationType = REG_ROUTER;
        mDomain.state            = DS_CONTROLLED;
    }

    void push_back(std::vector<gc_Domain_s > &listDomain, gc_Domain_s &domain)
    {
        listDomain.push_back(domain);
    }

    void expand()
    {
        if (mpConfig != NULL)
        {
            addChildNodeList(std::vector<CAmXmlNode *>{ &mListSourceNode, &mListSinkNode, &mListGatewayNode });
        }

        addAttributeList(std::vector<CAmNodeAttribute *>{ &mNameAttribute, &mIDAttribute, &mNodeNameAttribute, &mBusNameAttribute,
                                                          &mNodeNameAttribute, &mEarlyAttribute, &mCompleteAttribute, &mDomainStateAttribute,
                                                          &mRegistrationTypeAttribute, &mBaseNameAttribute });
    }

protected:
    void _getDefaultFromTemplate()
    {
        if (mpConfig == NULL)
        {
            return;
        }

        for (const auto &itListDomains : mpConfig->listTemplateDomains)
        {
            if (itListDomains.name == mBaseName)
            {
                mDomain = itListDomains;
                break;
            }
        }
    }

    void _storeCurrentName()
    {
        if (mpConfig != NULL)
        {
            mpConfig->mCurrentDomain = mDomain.name;
        }
    }

    am_Error_e _copyComplexData(void)
    {
        if (mpConfig != NULL)
        {
            mpConfig->mCurrentDomain = "";
        }

        return E_OK;
    }

private:

    gc_Domain_s                               &mDomain;
    std::string                                mBaseName;
    gc_Configuration_s                        *mpConfig;
    CAmListNode<gc_Source_s, CAmSourceNode>    mListSourceNode;
    CAmListNode<gc_Sink_s, CAmSinkNode>        mListSinkNode;
    CAmListNode<gc_Gateway_s, CAmGatewayNode>  mListGatewayNode;
    CAmStringIDAttribute                       mIDAttribute;
    CAmStringAttribute                         mNameAttribute;
    CAmStringAttribute                         mBusNameAttribute;
    CAmStringAttribute                         mNodeNameAttribute;
    CAmBoolAttribute                           mEarlyAttribute;
    CAmBoolAttribute                           mCompleteAttribute;
    CAmEnumerationAttribute<am_DomainState_e>  mDomainStateAttribute;
    CAmEnumerationAttribute<gc_Registration_e> mRegistrationTypeAttribute;
    CAmStringAttribute                         mBaseNameAttribute;
};

gc_TopologyElement_s::gc_TopologyElement_s(gc_ClassTopologyCodeID_e elementCode, const std::string &elementName)
    : codeID(elementCode)
{
    switch (codeID)
    {
    case MC_EQUAL_CODE:
        name = TOPOLOGY_SYMBOL_EQUAL;
        break;
    case MC_EXCLUSIVE_CODE:
        name = TOPOLOGY_SYMBOL_CARET;
        break;
    case MC_LBRACKET_CODE:
        name = TOPOLOGY_SYMBOL_LEFT_BRACKET;
        break;
    case MC_RBRACKET_CODE:
        name = TOPOLOGY_SYMBOL_RIGHT_BRACKET;
        break;
    default:
        name = elementName;
        break;
    }
}

gc_TopologyElement_s::gc_TopologyElement_s(gc_ClassTopologyCodeID_e elementCode, const std::string &elementName, size_t pos, size_t length)
    : gc_TopologyElement_s(elementCode, elementName.substr(pos, length))
{
}

gc_TopologyElement_s::gc_TopologyElement_s(const std::string &elementName)
{
    name = elementName;
    if (elementName == "(")
    {
        codeID = MC_LBRACKET_CODE;
    }
    else if (elementName == ")")
    {
        codeID = MC_RBRACKET_CODE;
    }
    else
    {
        codeID = MC_EXCLUSIVE_CODE;
    }
}

class CAmClassNode : public CAmConfigComplexNode
{
public:
    CAmClassNode(const char *tagName, gc_Class_s &classInfo, void *data = NULL)
        : CAmConfigComplexNode(tagName)
        , mClass(classInfo)
        , mListrTopologyNode(CLASS_TAG_TOPOLOGY_TAG, mTopologyStrings)
        , mListClassPropertyNode(COMMON_ATTRIBUTE_PROPERTY, mClass.listClassProperties)
        , mIDAttribute(ELEMENT_COMMON_ATTRIBUTE_ID, mClass.classID)
        , mClassNameAttribute(CLASS_ATTRIBUTE_NAME, mClass.name)
        , mClassTypeAttribute(CLASS_ATTRIBUTE_TYPE, mClass.type)
        , mClassPriorityAttribute(COMMON_ATTRIBUTE_PRIORITY, mClass.priority)
        , mIsVolumePersistenceSupportedAttribute(ELEMENT_COMMON_ATTRIBUTE_IS_VOLUME_PERSISTENCY_SUPPORTED, mClass.isVolumePersistencySupported)
        , mDefaultVolumeAttribute(CLASS_ATTRIBUTE_DEFAULT_VOLUME, mClass.defaultVolume)
    {
        (void)data;
        mClass.classID  = 0;
        mClass.name     = "";
        mClass.type     = C_PLAYBACK;
        mClass.priority = DEFAULT_CONFIG_CLASS_PRIORITY;
        mClass.listTopologies.clear();
        mClass.listClassProperties.clear();
        mClass.isVolumePersistencySupported = false;
        mClass.defaultVolume                = AM_MUTE;
    }

    void push_back(std::vector<gc_Class_s > &listClass, gc_Class_s &classInfo)
    {
        listClass.push_back(classInfo);
    }

    void expand()
    {
        addChildNodeList(std::vector<CAmXmlNode *>{ &mListrTopologyNode, &mListClassPropertyNode });
        addAttributeList(std::vector<CAmNodeAttribute *>{ &mIDAttribute, &mClassNameAttribute, &mClassTypeAttribute,
                                                          &mClassPriorityAttribute, &mIsVolumePersistenceSupportedAttribute,
                                                          &mDefaultVolumeAttribute });
    }

protected:

    virtual am_Error_e _copyComplexData(void)
    {
        am_Error_e error = E_OK;
        for (const auto &itListStringTopologies : mTopologyStrings)
        {
            std::vector<gc_TopologyElement_s> topology;
            error = _parseTopologyString(itListStringTopologies, topology);
            if (error == E_OK)
            {
                mClass.listTopologies.push_back(topology);
            }
            else
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "Topology parsing failed for", itListStringTopologies);
            }
        }

        return error;
    }

private:

    am_Error_e _parseElementList(gc_ClassTopologyCodeID_e elementType, const std::string &elementList, size_t currentIndex, const size_t endPosition, std::vector<gc_TopologyElement_s> &listTopologies)
    {
        size_t caretPosition = elementList.find_first_of(TOPOLOGY_SYMBOL_CARET, currentIndex);
        if ((elementList.find_first_of(TOPOLOGY_SYMBOL_LEFT_BRACKET, currentIndex) < endPosition) || (elementList.find_first_of(TOPOLOGY_SYMBOL_RIGHT_BRACKET, currentIndex) < endPosition))
        {
            return E_NOT_POSSIBLE;
        }

        while (currentIndex < endPosition)
        {
            if (caretPosition < endPosition)
            {
                listTopologies.push_back(gc_TopologyElement_s(elementType, elementList, currentIndex, caretPosition - currentIndex));
                listTopologies.push_back(gc_TopologyElement_s(MC_EXCLUSIVE_CODE));
                currentIndex = caretPosition + 1;
            }
            else
            {
                listTopologies.push_back(gc_TopologyElement_s(elementType, elementList, currentIndex, endPosition - currentIndex));
                currentIndex = caretPosition;
            }

            caretPosition = elementList.find_first_of(TOPOLOGY_SYMBOL_CARET, currentIndex);
        }

        return E_OK;
    }

    am_Error_e _parseTopologyString(const std::string &topologyString, std::vector<gc_TopologyElement_s> &listTopologies)
    {
        size_t     currentPosition = 0;
        size_t     totalLength     = topologyString.length();
        size_t     equalsPosition  = topologyString.find_first_of(TOPOLOGY_SYMBOL_EQUAL, currentPosition);
        am_Error_e result          = E_OK;
        if ((std::count(topologyString.begin(), topologyString.end(), '=') != 1) || (topologyString.find_first_of(TOPOLOGY_SYMBOL_WHITESPACE) != std::string::npos))
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "invalid character found");
            return E_NOT_POSSIBLE;
        }

        if (std::count(topologyString.begin(), topologyString.end(), '(') != std::count(topologyString.begin(), topologyString.end(), ')'))
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "mismatch in brackets");
            return E_NOT_POSSIBLE;
        }

        if (E_OK != _parseElementList(MC_SINK_ELEMENT, topologyString, currentPosition, equalsPosition, listTopologies))
        {
            return E_NOT_POSSIBLE;
        }

        listTopologies.push_back(gc_TopologyElement_s(MC_EQUAL_CODE));
        currentPosition = equalsPosition + 1;

        while (currentPosition < totalLength)
        {
            size_t caretPosition        = topologyString.find_first_of(TOPOLOGY_SYMBOL_CARET, currentPosition);
            size_t leftBracketPosition  = topologyString.find_first_of(TOPOLOGY_SYMBOL_LEFT_BRACKET, currentPosition);
            size_t rightBracketPosition = topologyString.find_first_of(TOPOLOGY_SYMBOL_RIGHT_BRACKET, currentPosition);

            if ((caretPosition == std::string::npos) && (leftBracketPosition == std::string::npos) && (rightBracketPosition == std::string::npos))
            {
                // No caret no brackets so only one source case 0
                listTopologies.push_back(gc_TopologyElement_s(MC_SOURCE_ELEMENT, topologyString, currentPosition));
                break;
            }
            else
            {
                size_t                   lowestPosition = std::min(caretPosition, std::min(leftBracketPosition, rightBracketPosition));
                gc_ClassTopologyCodeID_e elementType    = (topologyString[lowestPosition] == '(') ? MC_GATEWAY_ELEMENT : MC_SOURCE_ELEMENT;
                if (lowestPosition != currentPosition)
                {
                    listTopologies.push_back(gc_TopologyElement_s(elementType, topologyString, currentPosition, lowestPosition - currentPosition));
                }

                listTopologies.push_back(gc_TopologyElement_s(topologyString.substr(lowestPosition, 1)));
                currentPosition = lowestPosition + 1;
            }
        }

        return result;
    }

    std::vector<std::string>                              mTopologyStrings;
    gc_Class_s                                           &mClass;
    CAmListNode<std::string, CAmStringNode>               mListrTopologyNode;
    CAmListNode<am_ClassProperty_s, CAmClassPropertyNode> mListClassPropertyNode;
    CAmStringIDAttribute                                  mIDAttribute;
    CAmStringAttribute                                    mClassNameAttribute;
    CAmEnumerationAttribute<gc_Class_e>                   mClassTypeAttribute;
    CAmIntAttribute<int32_t>                              mClassPriorityAttribute;
    CAmBoolAttribute                                      mIsVolumePersistenceSupportedAttribute;
    CAmIntAttribute<int16_t>                              mDefaultVolumeAttribute;
};

class CAmActionNode : public CAmConfigComplexNode
{
public:
    CAmActionNode(const char *tagName, gc_Action_s &action, void *data = NULL)
        : CAmConfigComplexNode(tagName)
        , mAction(action)
        , mActionNameAttribute(ACTION_SET_ATTRIBUTE_TYPE, mAction.actionType)
        , mClassNameAttribute(ACTION_PARAM_CLASS_NAME, mClassName)
        , mConnectionNameAttribute(ACTION_PARAM_CONNECTION_NAME, mConnectionName)
        , mSourceNameAttribute(ACTION_PARAM_SOURCE_NAME, mSourceName)
        , mSinkNameAttribute(ACTION_PARAM_SINK_NAME, mSinkName)
        , mGatewayNameAttribute(ACTION_PARAM_GATEWAY_NAME, mGatewayName)
        , mRampAttribute(ACTION_PARAM_RAMP_TYPE, mRampType)
        , mRampTimeAttribute(ACTION_PARAM_RAMP_TIME, mRampTime)
        , mMuteStateAttribute(ACTION_PARAM_MUTE_STATE, mMuteState)
        , mMainVolumeStepAttribute(ACTION_PARAM_MAIN_VOLUME_STEP, mMainVolumeStep)
        , mMainVolumeAttribute(ACTION_PARAM_MAIN_VOLUME, mMainVolume)
        , mVolumeStepAttribute(ACTION_PARAM_VOLUME_STEP, mVolumeStep)
        , mVolumeAttribute(ACTION_PARAM_VOLUME, mVolume)
        , mOrderAttribute(ACTION_PARAM_ORDER, mOrder)
        , mPropertyTypeAttribute(ACTION_PARAM_PROPERTY_TYPE, mPropertyType)
        , mPropertyValueAtrribute(ACTION_PARAM_PROPERTY_VALUE, mPropertyValue)
        , mTimeoutAttribute(ACTION_PARAM_TIMEOUT, mTimeOut)
        , mPatternAttribute(ACTION_PARAM_PATTERN, mPattern)
        , mConnectionStateAttribute(ACTION_PARAM_CONNECTION_STATE, mConnectionState)
        , mDebugTypeAttribute(ACTION_PARAM_DEBUG_TYPE, mDebugType)
        , mDebugValueAttribute(ACTION_PARAM_DEBUG_VALUE, mDebugValue)
        , mConnectionFormatAttribute(ACTION_PARAM_CONNECTION_FORMAT, mConnectionFormat)
        , mExceptSourceAttribute(ACTION_PARAM_EXCEPT_SOURCE_NAME, mExceptSource)
        , mExceptSinkAttribute(ACTION_PARAM_EXCEPT_SINK_NAME, mExceptSink)
        , mExceptClassAttribute(ACTION_PARAM_EXCEPT_CLASS_NAME, mExceptClass)
        , mNotificationTypeAttribute(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, mNotificationType)
        , mNotificationStatusAttributes(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, mNotificationStatus)
        , mNotificationConfigurationAttribute(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, mNotificationParam)
        , mListMainSoundPropertiesAttribute(ACTION_PARAM_LIST_PROPERTY, mListMainSoundProperties)
        , mListSystemPropertiesAttribute(ACTION_PARAM_LIST_SYSTEM_PROPERTIES, mListSystemProperties)
    {
        mAction.actionType  = ACTION_UNKNOWN;
        mAction.mapParameters.clear();
        mRampType           = -1;
        mMuteState          = -1;
        mOrder              = -1;
        mPropertyType       = -1;
        mDebugType          = -1;
        mNotificationType   = -1;
        mNotificationStatus = -1;
    }

    void push_back(std::vector<gc_Action_s > &listAction, gc_Action_s &action)
    {
        listAction.push_back(action);
    }

    void expand()
    {
        addAttributeList(std::vector<CAmNodeAttribute *>{ &mActionNameAttribute, &mClassNameAttribute, &mConnectionNameAttribute,
                                                          &mSourceNameAttribute, &mSinkNameAttribute, &mGatewayNameAttribute, &mRampAttribute,
                                                          &mRampTimeAttribute, &mMuteStateAttribute, &mMainVolumeStepAttribute, &mMainVolumeAttribute,
                                                          &mVolumeStepAttribute, &mVolumeAttribute, &mOrderAttribute, &mPropertyTypeAttribute,
                                                          &mPropertyValueAtrribute, &mTimeoutAttribute, &mPatternAttribute, &mConnectionStateAttribute,
                                                          &mDebugTypeAttribute, &mDebugValueAttribute, &mConnectionFormatAttribute, &mExceptSourceAttribute,
                                                          &mExceptSinkAttribute, &mExceptClassAttribute, &mNotificationTypeAttribute, &mNotificationStatusAttributes,
                                                          &mNotificationConfigurationAttribute, &mListMainSoundPropertiesAttribute, &mListSystemPropertiesAttribute });
    }

protected:
    am_Error_e _copyComplexData(void)
    {
        copyStringInMap(ACTION_PARAM_CLASS_NAME, mClassName);
        copyStringInMap(ACTION_PARAM_CONNECTION_NAME, mConnectionName);
        copyStringInMap(ACTION_PARAM_CONNECTION_STATE, mConnectionState);
        copyStringInMap(ACTION_PARAM_SOURCE_NAME, mSourceName);
        copyStringInMap(ACTION_PARAM_SINK_NAME, mSinkName);
        copyStringInMap(ACTION_PARAM_GATEWAY_NAME, mGatewayName);
        if (mRampType != -1)
        {
            copyStringInMap(ACTION_PARAM_RAMP_TYPE, to_string(mRampType));
        }

        copyStringInMap(ACTION_PARAM_RAMP_TIME, mRampTime);
        if (mMuteState != -1)
        {
            copyStringInMap(ACTION_PARAM_MUTE_STATE, to_string(mMuteState));
        }

        copyStringInMap(ACTION_PARAM_MAIN_VOLUME_STEP, mMainVolumeStep);
        copyStringInMap(ACTION_PARAM_MAIN_VOLUME, mMainVolume);
        copyStringInMap(ACTION_PARAM_VOLUME_STEP, mVolumeStep);
        copyStringInMap(ACTION_PARAM_VOLUME, mVolume);
        if (mOrder != -1)
        {
            copyStringInMap(ACTION_PARAM_ORDER, to_string(mOrder));
        }

        if (mPropertyType != -1)
        {
            copyStringInMap(ACTION_PARAM_PROPERTY_TYPE, to_string(mPropertyType));
        }

        copyStringInMap(ACTION_PARAM_PROPERTY_VALUE, mPropertyValue);
        if ( mTimeOut != "")
        {
            copyStringInMap(ACTION_PARAM_TIMEOUT, mTimeOut);
        }
        else
        {
            copyStringInMap(ACTION_PARAM_TIMEOUT, to_string(DEFAULT_ASYNC_ACTION_TIME));
        }

        copyStringInMap(ACTION_PARAM_PATTERN, mPattern);
        if (mDebugType != -1)
        {
            copyStringInMap(ACTION_PARAM_DEBUG_TYPE, to_string(mDebugType));
        }

        copyStringInMap(ACTION_PARAM_DEBUG_VALUE, mDebugValue);
        copyStringInMap(ACTION_PARAM_CONNECTION_FORMAT, mConnectionFormat);
        copyStringInMap(ACTION_PARAM_EXCEPT_SOURCE_NAME, mExceptSource);
        copyStringInMap(ACTION_PARAM_EXCEPT_SINK_NAME, mExceptSink);
        copyStringInMap(ACTION_PARAM_EXCEPT_CLASS_NAME, mExceptClass);
        if (mNotificationType != -1)
        {
            copyStringInMap(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, to_string(mNotificationType));
        }

        if (mNotificationStatus != -1)
        {
            copyStringInMap(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, to_string(mNotificationStatus));
        }

        copyStringInMap(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, mNotificationParam);
        copyStringInMap(ACTION_PARAM_LIST_PROPERTY, mListMainSoundProperties);
        copyStringInMap(ACTION_PARAM_LIST_SYSTEM_PROPERTIES, mListSystemProperties);
        return E_OK;
    }

private:
    void copyStringInMap(std::string keyName, std::string value)
    {
        if (value != "")
        {
            mAction.mapParameters[keyName] = value;
        }
    }

private:
    gc_Action_s                 &mAction;
    std::string                  mClassName;
    std::string                  mConnectionName;
    std::string                  mSourceName;
    std::string                  mSinkName;
    std::string                  mGatewayName;
    std::string                  mRampTime;
    std::string                  mMainVolumeStep;
    std::string                  mMainVolume;
    std::string                  mVolumeStep;
    std::string                  mVolume;
    std::string                  mPropertyValue;
    std::string                  mTimeOut;
    std::string                  mPattern;
    std::string                  mConnectionFormat;
    std::string                  mDebugValue;
    std::string                  mExceptSource;
    std::string                  mExceptSink;
    std::string                  mExceptClass;
    std::string                  mConnectionState;
    std::string                  mNotificationParam;
    std::string                  mListMainSoundProperties;
    std::string                  mListSystemProperties;

    int                          mDebugType;
    int                          mRampType;
    int                          mMuteState;
    int                          mOrder;
    int                          mPropertyType;
    int                          mNotificationType;
    int                          mNotificationStatus;

    CAmEnumerationAttribute<gc_Action_e> mActionNameAttribute;
    CAmNameAttribute             mClassNameAttribute;
    CAmNameAttribute             mConnectionNameAttribute;
    CAmNameAttribute             mSourceNameAttribute;
    CAmNameAttribute             mSinkNameAttribute;
    CAmNameAttribute             mGatewayNameAttribute;
    CAmEnumerationAttribute<int> mRampAttribute;
    CAmStringAttribute           mRampTimeAttribute;
    CAmEnumerationAttribute<int> mMuteStateAttribute;
    CAmStringAttribute           mMainVolumeStepAttribute;
    CAmStringAttribute           mMainVolumeAttribute;
    CAmStringAttribute           mVolumeStepAttribute;
    CAmStringAttribute           mVolumeAttribute;
    CAmEnumerationAttribute<int> mOrderAttribute;
    CAmEnumerationAttribute<int> mPropertyTypeAttribute;
    CAmStringAttribute           mPropertyValueAtrribute;
    CAmStringAttribute           mTimeoutAttribute;
    CAmStringAttribute           mPatternAttribute;
    CAmStringAttribute           mConnectionStateAttribute;
    CAmEnumerationAttribute<int> mDebugTypeAttribute;
    CAmStringAttribute           mDebugValueAttribute;
    CAmStringAttribute           mConnectionFormatAttribute;
    CAmNameAttribute             mExceptSourceAttribute;
    CAmNameAttribute             mExceptSinkAttribute;
    CAmNameAttribute             mExceptClassAttribute;
    CAmEnumerationAttribute<int> mNotificationTypeAttribute;
    CAmEnumerationAttribute<int> mNotificationStatusAttributes;
    CAmStringAttribute           mNotificationConfigurationAttribute;
    CAmStringAttribute           mListMainSoundPropertiesAttribute;
    CAmStringAttribute           mListSystemPropertiesAttribute;
};

class CAmProcessNode : public CAmConfigComplexNode
{
public:
    static const std::set<std::string >                   mValidFunctionNames;
    static const std::map<std::string, gc_OperandType_e > mValidCategory;

    CAmProcessNode(const char *tagName, gc_Process_s &process, void *data = NULL)
        : CAmConfigComplexNode(tagName)
        , mProcess(process)
        , mListConditionNode(POLICY_TAG_CONDITION, mListConditions)
        , mListActionNode(POLICY_TAG_ACTION, mProcess.listActions)
        , mStopEvaluationNode(POLICY_TAG_STOP_EVALUATION, mProcess.stopEvaluation)
        , mProcessNameAttribute(POLICY_ATTRIBUTE_NAME, mProcess.comment)
        , mProcessPriorityAttribute(POLICY_TAG_PRIORITY, mProcess.priority)
    {
        mProcess.comment        = "";
        mProcess.priority       = DEFAULT_CONFIG_PARSED_POLICY_PRIORITY;
        mProcess.stopEvaluation = false;
        mProcess.listActions.clear();
        mProcess.listConditions.clear();
    }

    void push_back(std::vector<gc_Process_s > &listProcess, gc_Process_s &process)
    {
        listProcess.push_back(process);
    }

    void expand(void) override
    {
        addChildNodeList(std::vector<CAmXmlNode *>{ &mListConditionNode, &mListActionNode, &mStopEvaluationNode });
        addAttributeList(std::vector<CAmNodeAttribute *>{ &mProcessNameAttribute, &mProcessPriorityAttribute });
    }

protected:
    am_Error_e _copyComplexData(void) override
    {
        am_Error_e error = E_OK;
        for (const auto &itListStringConditions : mListConditions)
        {
            gc_ConditionStruct_s conditionInstance;
            error = _parseCondition(itListStringConditions.c_str(),
                    conditionInstance);
            if (error != E_OK)
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "condition parse failure", itListStringConditions);
                break;
            }

            mProcess.listConditions.push_back(conditionInstance);
        }

        return error;
    }

private:
    static void _removeLeadingSpace(const char *inputString, unsigned int &startPosition)
    {
        while (((inputString[startPosition] == ' ') || (inputString[startPosition] == '\t')) && (startPosition
                                                                                                 < strlen((char *)inputString)))
        {
            startPosition++;
        }
    }

    static bool _isFunctionAvailable(const char *inputString, unsigned int startPosition)
    {
        // check if an opening bracket can be found in inputString.
        // If so, check if the given function name is supported

        const char *pStart   = &inputString[startPosition];
        const char *pBracket = strchr(pStart, '(');

        if (pBracket)
        {
            const std::string functionName(pStart, pBracket - pStart);
            if (mValidFunctionNames.end() == mValidFunctionNames.find(functionName))
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "Function", functionName + "()", "not supported");
            }

            return true;
        }

        return false;
    }

// validates whether the input std::string is integer or not.
    static bool _isInteger(const char *inputString, unsigned int startPosition)
    {
        while ((isdigit(inputString[startPosition])) && (startPosition < strlen((char *)inputString)))
        {
            startPosition++;
        }

        if (startPosition == strlen((char *)inputString))
        {
            return true;
        }

        return false;
    }

public:
    static std::string findElement(unsigned int &startPosition, const char token,
        const char *inputString, const char invalidToken)
    {
        _removeLeadingSpace(inputString, startPosition);

        // find first occurrence of either token, invalidToken or string end
        const char *pStart       = &inputString[startPosition];
        unsigned    nestingLevel = 0;
        size_t      offset       = 0;
        bool        isString     = false;
        for (; offset < strlen(pStart); offset++)
        {
            const char chr = pStart[offset];
            if ((nestingLevel == 0) && ((chr == token) || (chr == invalidToken)))
            {
                break;   // found the desired token at top level - stop further searching
            }

            // track nesting
            if (chr == '"')
            {
                isString = !isString;                        // toggle
                isString ? nestingLevel-- : nestingLevel++;  // track nesting level
            }
            else if (chr == '(')
            {
                nestingLevel++;
            }
            else if (chr == ')')
            {
                nestingLevel--;
            }
        }

        startPosition += offset;

        return std::string(pStart, offset);
    }

private:
// Tokenize the RHS part of condition
    static am_Error_e _findRHSFromCondition(const char *inputString, unsigned int &startPosition,
        gc_RHSElement_s &rightObject)
    {
        am_Error_e result = E_OK;
        if (inputString[startPosition] == '"')
        {
            startPosition++;
            rightObject.directValue = findElement(startPosition, '"', inputString, '"');
            rightObject.isValue     = true;
        }
        else if (_isFunctionAvailable(inputString, startPosition))
        {
            rightObject.isValue = false;
            result              = CAmXmlConfigParser::parsePolicyFunction(inputString, startPosition, rightObject.functionObject);
        }
        else
        {
            rightObject.isValue     = true;
            rightObject.directValue = findElement(startPosition, '"', inputString, ' ');
        }

        return result;
    }

    static am_Error_e _parseCondition(const char *stringConditions,
        gc_ConditionStruct_s &conditionInstance)
    {
        am_Error_e   result;
        unsigned int position = 0;
        int          enumeratedValue;
        int          value;

        _removeLeadingSpace(stringConditions, position);

        // parse the attribute of LHS
        result = CAmXmlConfigParser::parsePolicyFunction(stringConditions, position, conditionInstance.leftObject);
        if (E_OK != result)
        {
            return E_UNKNOWN;
        }

        _removeLeadingSpace(stringConditions, position);

        // find the operator
        std::string operatorString = findElement(position, ' ', stringConditions, '"');
        position++;
        if (E_OK == CAmXmlConfigParserCommon::getEnumerationValue(operatorString, enumeratedValue))
        {
            conditionInstance.operation = static_cast<gc_Operator_e>(enumeratedValue);
        }
        else
        {
            LOG_FN_ERROR(__FILENAME__, __func__, " Operator Not in range:", operatorString);
            return E_UNKNOWN;
        }

        // remove the space after operator
        _removeLeadingSpace(stringConditions, position);

        // parse the attribute of RHS
        result = _findRHSFromCondition(stringConditions, position, conditionInstance.rightObject);
        if (E_OK != result)
        {
            return E_UNKNOWN;
        }

        return E_OK;
    }

    std::vector<std::string >                mListConditions;
    gc_Process_s                            &mProcess;
    CAmListNode<std::string, CAmStringNode > mListConditionNode;
    CAmListNode<gc_Action_s, CAmActionNode > mListActionNode;
    CAmBoolNode mStopEvaluationNode;
    CAmStringAttribute                       mProcessNameAttribute;
    CAmIntAttribute<int32_t>                 mProcessPriorityAttribute;
};

const std::set<std::string > CAmProcessNode::mValidFunctionNames
    = { FUNCTION_NAME, FUNCTION_ELEMENTS, FUNCTION_PRIORITY, FUNCTION_CONNECTION_STATE, FUNCTION_VOLUME,
        FUNCTION_MAIN_VOLUME, FUNCTION_MAIN_VOLUME_STEP, FUNCTION_VOLUME_LIMIT, FUNCTION_SOUND_PROPERTY, FUNCTION_MAIN_SOUND_PROPERTY_TYPE,
        FUNCTION_SYSTEM_PROPERTY_TYPE, FUNCTION_MAIN_SOUND_PROPERTY_VALUE, FUNCTION_SYSTEM_PROPERTY_VALUE,
        FUNCTION_MUTE_STATE, FUNCTION_IS_REGISTRATION_COMPLETE, FUNCTION_AVAILABILITY, FUNCTION_AVAILABILITY_REASON,
        FUNCTION_INTERRUPT_STATE, FUNCTION_IS_REGISTERED, FUNCTION_STATE, FUNCTION_CONNECTION_FORMAT, FUNCTION_PEEK,
        FUNCTION_ERROR, FUNCTION_DOMAIN_STATE, FUNCTION_SOURCE_STATE, FUNCTION_NOTIFICATION_CONFIGURATION_STATUS,
        FUNCTION_NOTIFICATION_CONFIGURATION_PARAM, FUNCTION_NOTIFICATION_DATA_VALUE, FUNCTION_NOTIFICATION_DATA_TYPE,
        FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_TYPE, FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_STATUS,
        FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_PARAM, FUNCTION_SCALE, FUNCTION_COUNT };

const std::map<std::string, gc_OperandType_e > CAmProcessNode::mValidCategory
    = {
    { CATEGORY_SINK, OT_SINK },
    { CATEGORY_SOURCE, OT_SOURCE },
    { CATEGORY_CLASS, OT_CLASS },
    { CATEGORY_CONNECTION, OT_CONNECTION },
    { CATEGORY_DOMAIN, OT_DOMAIN },
    { CATEGORY_DOMAIN_OF_SOURCE, OT_DOMAIN_OF_SOURCE },
    { CATEGORY_DOMAIN_OF_SINK, OT_DOMAIN_OF_SINK },
    { CATEGORY_CLASS_OF_SOURCE, OT_CLASS_OF_SOURCE },
    { CATEGORY_CLASS_OF_SINK, OT_CLASS_OF_SINK },
    { CATEGORY_SOURCE_OF_CLASS, OT_SOURCE_OF_CLASS },
    { CATEGORY_SINK_OF_CLASS, OT_SINK_OF_CLASS },
    { CATEGORY_CONNECTION_OF_CLASS, OT_CONNECTION_OF_CLASS },
    { CATEGORY_CONNECTION_OF_SOURCE, OT_CONNECTION_OF_SOURCE },
    { CATEGORY_CONNECTION_OF_SINK, OT_CONNECTION_OF_SINK },
    { CATEGORY_SYSTEM, OT_SYSTEM },
    { CATEGORY_USER, OT_USER }
    };

class CAmPolicyNode : public CAmConfigComplexNode
{
public:
    CAmPolicyNode(const char *tagName, gc_Policy_s &policy, void *data = NULL)
        : CAmConfigComplexNode(tagName)
        , mListTriggerAttributes(POLICY_TAG_TRIGGER, mTriggerAttributes)
        , mPolicy(policy)
        , mListProcessNode(POLICY_TAG_LIST_PROCESS, mPolicy.listProcesses)
    {
        mPolicy.listEvents.clear();
        mPolicy.listProcesses.clear();
    }

    void push_back(std::vector<gc_Policy_s > &listPolicy, gc_Policy_s &policy)
    {
        listPolicy.push_back(policy);
    }

    void expand()
    {
        addChildNode(&mListProcessNode);
        addAttribute(&mListTriggerAttributes);
    }

protected:
    am_Error_e _copyComplexData(void)
    {
        CAmStringParser trigger(mTriggerAttributes, '|');
        trigger.parseTrigger(mPolicy.listEvents);
        return E_OK;
    }

private:
    gc_Policy_s       &mPolicy;
    CAmStringAttribute mListTriggerAttributes;
    std::string        mTriggerAttributes;
    CAmListNode<gc_Process_s, CAmProcessNode> mListProcessNode;
};

class CAmTemplateNode : public CAmConfigComplexNode
{
public:
    CAmTemplateNode(const char *tagname, gc_Configuration_s *pConfig)
        : CAmConfigComplexNode(tagname)
        , mpConfig(pConfig)
        , mScaleConversionNode(TEMPLATE_TAG_SCALE_CONVERSION_MAP, &(mMapTemplateScaleConversionMaps))
        , mListDomainNode(XML_ELEMENT_TAG_DOMAIN, mpConfig->listTemplateDomains, mpConfig)
        , mListSourceNode(XML_ELEMENT_TAG_SOURCE, mpConfig->listTemplateSources, mpConfig)
        , mListSinkNode(XML_ELEMENT_TAG_SINK,     mpConfig->listTemplateSinks,   mpConfig)
        , mListGatewayNode(XML_ELEMENT_TAG_GATEWAY, mpConfig->listTemplateGateways, mpConfig)
        , mlistSoundPropertiesNode(XML_ELEMENT_TAG_SOUND_PROPERTIES, XML_ELEMENT_TAG_SOUND_PROPERTY, &(mpConfig->listTemplateSoundProperties))
        , mlistSystemPropertiesNode(XML_ELEMENT_TAG_SYSTEM_PROPERTIES, XML_ELEMENT_TAG_SYSTEM_PROPERTY, &(mpConfig->listTemplateSystemProperties))
        , mListMainSoundPropertiesNode(XML_ELEMENT_TAG_MAIN_SOUND_PROPERTIES, XML_ELEMENT_TAG_MAIN_SOUND_PROPERTY, &(mpConfig->listTemplateMainSoundProperties))
        , mListNotificationConfigurationNode(XML_ELEMENT_TAG_NOTIFICATION_CONFIGS, XML_ELEMENT_TAG_NOTIFICATION_CONFIG, &(mpConfig->listTemplateNotificationConfigurations))
        , mListConnectionFormatNode(
            XML_ELEMENT_TAG_CONNECTION_FORMAT, &(mMapTemplateConnectionFormats))
        , mListMSPToSPNode(CONFIG_ELEMENT_SET_MSP_SP_MAP, MSPSP_MAP_TAG_TYPE, &(mpConfig->listTemplateMapMSPToSP))
        , mGatewayConvMatrixNode(GATEWAY_TAG_CONVERSION_MATRIX, &(mMapTemplateConversionMatrix))
    {
    }

    virtual void expand(void)
    {
        addChildNodeList(std::vector<CAmXmlNode *>
            { &mScaleConversionNode, &mListDomainNode, &mListSourceNode, &mListSinkNode, &mListGatewayNode
            , &mlistSoundPropertiesNode, &mListMainSoundPropertiesNode, &mlistSystemPropertiesNode, &mListNotificationConfigurationNode
            , &mListConnectionFormatNode, &mListMSPToSPNode, &mGatewayConvMatrixNode});
    }

protected:
    am_Error_e _copyComplexData(void)
    {
        for (const auto &it : mMapTemplateScaleConversionMaps)
        {
            auto &conversionMap = mpConfig->listTemplateMapScaleConversions[it.first];
            CAmStringParser::populateScalingMap(conversionMap, it.second, mpConfig->listTemplateMapScaleConversions);
        }

        _copyStringToMaps<std::vector<am_CustomConnectionFormat_t> >(mMapTemplateConnectionFormats, &am::gc::CAmStringParser::parseConnectionFormats, mpConfig->listTemplateConnectionFormats);
        _copyStringToMaps<std::vector<std::pair<uint16_t, uint16_t> > >(mMapTemplateConversionMatrix, &am::gc::CAmStringParser::parseCommaSeperateEnumerationsList, mpConfig->listTemplateConversionMatrix);
        return E_OK;
    }

    template<typename T1>
    void _copyStringToMaps(std::map<std::string, std::string> &mapString, void (CAmStringParser::*func)(T1 &), std::map<std::string, T1> &output)
    {
        CAmStringParser stringParser("");
        for (const auto &it : mapString)
        {
            T1 list;
            stringParser.setString(it.second);
            (stringParser.*(func))(list);
            output[it.first] = list;
        }
    }

private:

    gc_Configuration_s                       *mpConfig;
    std::map<std::string, std::string>        mMapTemplateConnectionFormats;
    std::map<std::string, std::string>        mMapTemplateScaleConversionMaps;
    std::map<std::string, std::string>        mMapTemplateNormalizedVolume;
    std::map<std::string, std::string>        mMapTemplateDecibelVolume;
    std::map<std::string, std::string>        mMapTemplateConversionMatrix;
    CAmTemplateMap                            mScaleConversionNode;
    CAmListNode<gc_Domain_s, CAmDomainNode>   mListDomainNode;
    CAmListNode<gc_Source_s, CAmSourceNode>   mListSourceNode;
    CAmListNode<gc_Sink_s, CAmSinkNode>       mListSinkNode;
    CAmListNode<gc_Gateway_s, CAmGatewayNode> mListGatewayNode;
    CAmEncapsulatedTemplateList<gc_SoundProperty_s, CAmSoundPropertyNode>                         mlistSoundPropertiesNode;
    CAmEncapsulatedTemplateList<gc_SystemProperty_s, CAmSystemPropertyNode>                       mlistSystemPropertiesNode;
    CAmEncapsulatedTemplateList<gc_MainSoundProperty_s, CAmMainSoundPropertyNode>                 mListMainSoundPropertiesNode;
    CAmEncapsulatedTemplateList<am_NotificationConfiguration_s, CAmNotificationConfigurationNode> mListNotificationConfigurationNode;
    CAmTemplateMap mListConnectionFormatNode;
    CAmEncapsulatedTemplateMap<std::map<gc_MSPMappingDirection_e, std::map<uint16_t, uint16_t> >, CAmMSPToSPMapListNode> mListMSPToSPNode;
    CAmTemplateMap mGatewayConvMatrixNode;
};

class CAmsystemNode : public CAmConfigComplexNode
{
public:
    CAmsystemNode(const char *tagname, gc_Configuration_s *pConfig)
        : CAmConfigComplexNode(tagname)
        , mpConfig(pConfig)
        , mTemplateNode(CONFIG_ELEMENT_TEMPLATE, mpConfig)
        , mListDomainNode(XML_ELEMENT_TAG_DOMAIN, mpConfig->listDomains, mpConfig)
        , mListSourceNode(XML_ELEMENT_TAG_SOURCE, mpConfig->listSources, mpConfig)
        , mListSinkNode(XML_ELEMENT_TAG_SINK, mpConfig->listSinks, mpConfig)
        , mListGatewayNode(XML_ELEMENT_TAG_GATEWAY, mpConfig->listGateways, mpConfig)
    {
    }

    virtual void expand(void)
    {
        addChildNodeList(std::vector<CAmXmlNode *>
            { &mTemplateNode, &mListDomainNode, &mListSourceNode, &mListSinkNode, &mListGatewayNode });
    }

private:
    gc_Configuration_s                       *mpConfig;
    CAmTemplateNode                           mTemplateNode;
    CAmListNode<gc_Domain_s, CAmDomainNode>   mListDomainNode;
    CAmListNode<gc_Source_s, CAmSourceNode>   mListSourceNode;
    CAmListNode<gc_Sink_s, CAmSinkNode>       mListSinkNode;
    CAmListNode<gc_Gateway_s, CAmGatewayNode> mListGatewayNode;
};

class CAmConfigurationNode : public CAmConfigComplexNode
{
public:
    CAmConfigurationNode(const char *tagname, gc_Configuration_s *pConfig)
        : CAmConfigComplexNode(tagname)
        , mpConfig(pConfig)
        , mListClassNode(CONFIG_ELEMENT_SET_CLASS, XML_ELEMENT_TAG_CLASS, mpConfig->listClasses)
        , mSystemNode(CONFIG_ELEMENT_SYSTEM, mpConfig)
        , mListPolicyNode(CONFIG_ELEMENT_SET_POLICY, XML_ELEMENT_TAG_POLICY, mpConfig->listPolicies)
        , mListSystemPropertiesNode(CONFIG_ELEMENT_SET_SYSTEM_PROPERTY, SYSTEM_PROP_TAG, mpConfig->listSystemProperties)
    {
    }

    virtual void expand(void)
    {
        addChildNodeList(std::vector<CAmXmlNode *>
            { &mListClassNode, &mSystemNode, &mListPolicyNode, &mListSystemPropertiesNode });
    }

private:
    gc_Configuration_s *mpConfig;
    CAmEncapsulatedListNode<gc_Class_s, CAmClassNode> mListClassNode;
    CAmsystemNode       mSystemNode;
    CAmEncapsulatedListNode<gc_Policy_s, CAmPolicyNode>                 mListPolicyNode;
    CAmEncapsulatedListNode<gc_SystemProperty_s, CAmSystemPropertyNode> mListSystemPropertiesNode;
};

CAmXmlConfigParser::CAmXmlConfigParser()
    : mpConfiguration(NULL)
{
}

CAmXmlConfigParser::~CAmXmlConfigParser(void)
{
}

am_Error_e CAmXmlConfigParser::parse(gc_Configuration_s *pConfiguration)
{
// check the environment variable to decide the path to be used for user configuration
    const char *path = getenv(CONFIGURATION_FILE_ENV_VARNAME);
    if (NULL == path)
    {
        path = DEFAULT_USER_CONFIGURATION_PATH;
    }

    LOG_FN_INFO(__FILENAME__, __func__, "Parse Start:", path);
#if AMCO_SCHEMA_VALIDATION
    // Validate the XML file against the schema.
    if (E_OK != _validateConfiguration(path, DEFAULT_SCHEMA_PATH))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Not able to validate configuration as per Schema");
        return E_UNKNOWN;
    }
#endif      // if AMCO_SCHEMA_VALIDATION
    if (E_OK != _parseXSDFile(DEFAULT_SCHEMA_PATH))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Not able to parse Schema");
        return E_UNKNOWN;
    }

    if (E_OK != _parseXSDFile(DEFAULT_CUSTOM_SCHEMA_PATH))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Not able to parse Schema");
        return E_UNKNOWN;
    }

    if (E_OK != _parseConfiguration(std::string(path), pConfiguration))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Not able to parse configuration");
        return E_UNKNOWN;
    }

    LOG_FN_DEBUG(__FILENAME__, __func__, "Parse End");
#if AMCO_DEBUGGING
    printAllEnums();
    printListSources(pConfiguration);
    printListSinks(pConfiguration);
    printListGateways(pConfiguration);
    printListDomains(pConfiguration);
    printListClasses(pConfiguration);
    printListPolicies(pConfiguration);
    printListSystemProperties(pConfiguration);
#endif      // if AMCO_DEBUGGING
    return E_OK;
}

void CAmXmlConfigParser::populateScalingMap(std::map<float, float > &outMap, const std::string &valuePairString
    , const std::map<std::string, std::map<float, float> > listTemplateMapScaleConversions)
{
    CAmStringParser::populateScalingMap(outMap, valuePairString, listTemplateMapScaleConversions);
}

// This API is used to parse and store the token of the LHS/RHS part of the condition
am_Error_e CAmXmlConfigParser::parsePolicyFunction(const char *inputString
    , unsigned int &startPosition, gc_FunctionElement_s &function)
{
    std::string functionName;
    std::string category;
    std::string mandatoryParameter;
    std::string optionalParameter;
    std::string optionalParameter2;
    bool        isValueMacro = false;

    // find function name
    functionName = CAmProcessNode::findElement(startPosition, '(', inputString, ',');
    // validate function name
    if (CAmProcessNode::mValidFunctionNames.end() == CAmProcessNode::mValidFunctionNames.find(functionName))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, functionName, "Function name is not supported");
        return E_UNKNOWN;
    }

    startPosition++;

    // find category
    category = CAmProcessNode::findElement(startPosition, ',', inputString, ')');
    if (inputString[startPosition] == ')')// end of condition part
    {
        if (category.empty())
        {
            category = CATEGORY_USER;
        }
    }

    startPosition++;

    // find the mandatory value. Store the element in macro (REQUESTING/ALL) or std::string with ""
    if (inputString[startPosition - 1] == ')')
    {
        mandatoryParameter = FUNCTION_MACRO_SUPPORTED_REQUESTING;
    }
    else
    {
        mandatoryParameter = CAmProcessNode::findElement(startPosition, ')', inputString, ',');
    }

    // check the value if it is macro. Only REQUESTING (ALL -- in some case) Macro is allowed
    if ((mandatoryParameter.data())[0] != '"')
    {
        if (strchr(mandatoryParameter.c_str(), '('))
        {
            // nested function found
        }
        else if (mandatoryParameter.find("REQ_") == 0)
        {
            // macro found
        }
        else if ((FUNCTION_IS_REGISTERED == functionName) || (FUNCTION_IS_REGISTRATION_COMPLETE
                                                              == functionName)
                 || ((FUNCTION_STATE == functionName) && (CATEGORY_DOMAIN == category)))
        {
            if ((CATEGORY_DOMAIN != category) || ((FUNCTION_MACRO_SUPPORTED_ALL
                                                   != mandatoryParameter)
                                                  && (FUNCTION_MACRO_SUPPORTED_REQUESTING != mandatoryParameter)))
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "Mandatory Parameter not valid in condition of policy."
                                                     "Might be double quote is missing. Expected is ALL or"
                                                     " string inside double quote",
                    functionName, category, mandatoryParameter);
                return E_UNKNOWN;
            }
        }
        else if (FUNCTION_MACRO_SUPPORTED_REQUESTING != mandatoryParameter)
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "Mandatory Parameter not valid in condition of policy. "
                                                 "Might be double quote is missing. Expected is REQUESTING or "
                                                 "string inside double quote",
                functionName, category, mandatoryParameter);
            return E_UNKNOWN;
        }

        isValueMacro = true;
    }
    else
    {
        unsigned int temp = 1;
        mandatoryParameter = CAmProcessNode::findElement(temp, '"', mandatoryParameter.data(),
                '"');
    }

    // find the optional value
    if (inputString[startPosition] == ',')
    {
        startPosition++;
        optionalParameter = CAmProcessNode::findElement(startPosition, ')', inputString, ',');

        // strip off enclosing quotes
        if (!optionalParameter.empty() && optionalParameter.data()[0] == '"')
        {
            unsigned int temp = 1;
            optionalParameter = CAmProcessNode::findElement(temp, '"', optionalParameter.data(), '"');
        }
    }
    else
    {
        // this is needed for main notification configuration param/status function because 3 parameter can be left empty.
        // In that case REQUESTING type need to be considered as configuration type
        if ((FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_PARAM == functionName)
            || (FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_STATUS == functionName))
        {
            optionalParameter = FUNCTION_MACRO_SUPPORTED_REQUESTING;
        }
    }

    // find the 2nd optional value
    if (inputString[startPosition] == ',')
    {
        startPosition++;
        optionalParameter2 = CAmProcessNode::findElement(startPosition, ')', inputString, ',');

        // strip off enclosing quotes
        if (!optionalParameter2.empty() && optionalParameter2.data()[0] == '"')
        {
            unsigned int temp = 1;
            optionalParameter2 = CAmProcessNode::findElement(temp, '"', optionalParameter2.data(), '"');
        }
    }

    // validate category name
    const auto &itOperandtype = CAmProcessNode::mValidCategory.find(category);
    if (CAmProcessNode::mValidCategory.end() == itOperandtype)
    {
        return E_UNKNOWN;
    }

    startPosition++;

    function.functionName       = functionName;
    function.category           = itOperandtype->second;
    function.mandatoryParameter = mandatoryParameter;
    function.optionalParameter  = optionalParameter;
    function.optionalParameter2 = optionalParameter2;
    function.isValueMacro       = isValueMacro;

    return E_OK;
}

void _forwardschemaerrortodlt(void *pCtx, const char *pMsg, ...)
{
    (void)pCtx;
    char        buf[1024];
    std::string ErroMsg;
    va_list     args; // va_list();s
// va_start() will initialize args
    va_start(args, pMsg);
    int len = vsnprintf(buf, 1024, pMsg, args);
    va_end(args);
    ErroMsg = buf;
    LOG_FN_ERROR(__FILENAME__, __func__, "Schema Parsing Error, Length=", len, "Message=", ErroMsg);
}

#if AMCO_SCHEMA_VALIDATION
am_Error_e CAmXmlConfigParser::_validateConfiguration(const std::string &XMLFileName,
    const std::string &XSDFileName)
{
    am_Error_e             returnValue = E_UNKNOWN;
    xmlDocPtr              pDocument;
    xmlSchemaPtr           pSchema = NULL;
    xmlSchemaParserCtxtPtr pContext;
    xmlLineNumbersDefault(1);
    pContext = xmlSchemaNewParserCtxt(XSDFileName.c_str());
    if (pContext == NULL)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " schema file not found");
        return returnValue;
    }

# ifndef AMCO_DEBUGGING
    xmlSchemaSetParserErrors(pContext, (xmlSchemaValidityErrorFunc)_forwardschemaerrortodlt,
        (xmlSchemaValidityWarningFunc)_forwardschemaerrortodlt, NULL);
# else
    xmlSchemaSetParserErrors(pContext,
        (xmlSchemaValidityErrorFunc)fprintf,
        (xmlSchemaValidityWarningFunc)fprintf,
        stderr);
# endif     // ifndef AMCO_DEBUGGING
    pSchema = xmlSchemaParse(pContext);
    if (pSchema != NULL)
    {
        xmlSchemaFreeParserCtxt(pContext);
        pDocument = xmlReadFile(XMLFileName.c_str(), NULL, 0);
        if (pDocument != NULL)
        {
            xmlSchemaValidCtxtPtr pValidContext;
            pValidContext = xmlSchemaNewValidCtxt(pSchema);
# ifndef AMCO_DEBUGGING
            xmlSchemaSetValidErrors(pValidContext,
                (xmlSchemaValidityErrorFunc)_forwardschemaerrortodlt,
                (xmlSchemaValidityWarningFunc)_forwardschemaerrortodlt, NULL);
# else
            xmlSchemaSetValidErrors(pValidContext,
                (xmlSchemaValidityErrorFunc)fprintf,
                (xmlSchemaValidityWarningFunc)fprintf,
                stderr);
# endif             // ifndef AMCO_DEBUGGING
            if (0 == xmlSchemaValidateDoc(pValidContext, pDocument))
            {
                returnValue = E_OK;
            }

            xmlSchemaFreeValidCtxt(pValidContext);
            xmlFreeDoc(pDocument);
        }

        // free the resource
        xmlSchemaFree(pSchema);
        xmlSchemaCleanupTypes();
        xmlCleanupParser();
        xmlMemoryDump();
    }

    return returnValue;
}
#endif  // if AMCO_SCHEMA_VALIDATION

am_Error_e CAmXmlConfigParser::_parseConfiguration(const std::string &XMLFilename, gc_Configuration_s *pConfiguration)
{
    xmlDocPtr  pDocument;
    xmlNodePtr pCurrent;
    pDocument = xmlParseFile(XMLFilename.c_str());
    if (pDocument == NULL)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Document not parsed successfully. ");
        return E_NOT_POSSIBLE;
    }

    pCurrent = xmlDocGetRootElement(pDocument);
    if (pCurrent == NULL)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " empty document");
        xmlFreeDoc(pDocument);
        return E_NOT_POSSIBLE;
    }

    CAmConfigurationNode configurationNode(XML_ELEMENT_TAG_GENERIC, pConfiguration);
    mpConfiguration = pConfiguration;
    configurationNode.parse(pDocument, &pCurrent);
    xmlFreeDoc(pDocument);
    return E_OK;
}

am_Error_e CAmXmlConfigParser::_parseXSDFile(const std::string &XSDFilename)
{
    /**
     * This function should parse the xsd file and make an internal data structure for
     * storing the enumerations as below
     * enumLookup[enumeration element][enum value]
     */
    xmlDocPtr  pDocument;
    xmlNodePtr pCurrent;
    pDocument = xmlParseFile(XSDFilename.c_str());
    if (pDocument == NULL)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Document not parsed successfully. ");
        return E_UNKNOWN;
    }

    pCurrent = xmlDocGetRootElement(pDocument);
    if (pCurrent == NULL)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " empty document");
        xmlFreeDoc(pDocument);
        return E_UNKNOWN;
    }

    if (xmlStrcmp(pCurrent->name, (const xmlChar *)XML_ELEMENT_TAG_SCHEMA))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Document of the wrong type, root node != schema");
        xmlFreeDoc(pDocument);
        return E_UNKNOWN;
    }

    _parseSimpleType(pDocument, pCurrent->children);
    xmlFreeDoc(pDocument);
    return E_OK;
}

void CAmXmlConfigParser::_parseSimpleType(const xmlDocPtr pDocument, xmlNodePtr pCurrent)
{
    xmlNodePtr pChildNode;
    while (pCurrent != NULL)
    {
        if (xmlStrcmp(pCurrent->name, (const xmlChar *)XML_ELEMENT_TAG_SIMPLE_TYPE) == 0)
        {
            pChildNode = pCurrent->children;
            _parseEnumeration(pDocument, pChildNode);
        }

        pCurrent = pCurrent->next;
    }
}

am_Error_e CAmXmlConfigParser::_parseEnumInitialiser(const xmlDocPtr pDocument, xmlNodePtr pCurrent, int &value)
{
    am_Error_e result = E_UNKNOWN;
    xmlChar   *pKey;
    while (pCurrent != NULL)
    {
        if (xmlStrcmp(pCurrent->name, (const xmlChar *)XML_ELEMENT_TAG_ANNOTATION) == 0)
        {
            pCurrent = pCurrent->children;
            if (pCurrent != NULL)
            {
                pCurrent = pCurrent->next;
            }

            if (pCurrent != NULL)
            {
                pKey  = xmlNodeListGetString(pDocument, pCurrent->xmlChildrenNode, 1);
                value = CAmXmlConfigParserCommon::convert<int>(std::string((char *)pKey));
                if (value != 0)
                {
                    result = E_OK;
                }

                xmlFree(pKey);
            }
        }

        if (pCurrent != NULL)
        {
            pCurrent = pCurrent->next;
        }
    }

    return result;
}

void CAmXmlConfigParser::_parseEnumeration(const xmlDocPtr pDocument, xmlNodePtr pCurrent)
{
    xmlChar                     *pKey;
    xmlNodePtr                   pChild;
    int                          counter = 0;
    int                          parseValue;
    const std::set <std::string> listMacros =
    {
        "REQUESTING",
        "ALL",
        "OTHERS",
        "REQ_TRIG_TYPE",
        "REQ_SINK_NAME",
        "REQ_SOURCE_NAME",
        "REQ_DOMAIN_NAME",
        "REQ_GATEWAY_NAME",
        "REQ_CLASS_NAME",
        "REQ_CON_NAME",
        "REQ_CON_STATE",
        "REQ_STATUS",
        "REQ_MAIN_VOL",
        "REQ_MSP_TYPE",
        "REQ_MSP_VAL",
        "REQ_SYP_TYPE",
        "REQ_SYP_VAL",
        "REQ_AVAIL_STATE",
        "REQ_AVAIL_REASON",
        "REQ_MUTE_STATE",
        "REQ_INT_STATE",
        "REQ_NP_TYPE",
        "REQ_NP_VAL",
        "REQ_NC_TYPE",
        "REQ_NC_STATUS",
        "REQ_NC_PARAM",
    };
    while (pCurrent != NULL)
    {
        if (xmlStrcmp(pCurrent->name, (const xmlChar *)XML_ELEMENT_TAG_RESTRICTION) == 0)
        {
            pChild = pCurrent->children;
            while (pChild != NULL)
            {
                if (xmlStrcmp(pChild->name, (const xmlChar *)ENUM_TAG_ENUMERATION) == 0)
                {
                    pKey = xmlGetProp(pChild, (const xmlChar *)ENUM_TAG_VALUE);
                    if (E_OK == _parseEnumInitialiser(pDocument, pChild->children, parseValue))
                    {
                        counter = parseValue;
                    }

                    if (listMacros.find(std::string((const char *)pKey)) == listMacros.end())
                    {
                        CAmXmlConfigParserCommon::mMapEnumerations[(char *)pKey] = counter++;
                    }

                    xmlFree(pKey);
                }

                pChild = pChild->next;
            }
        }

        pCurrent = pCurrent->next;
    }
}

#if AMCO_DEBUGGING
void CAmXmlConfigParser::printAllEnums(void) const
{
    for (const auto itMapEnumerations :  mMapEnumerations)
    {
        LOG_FN_INFO(__FILENAME__, __func__, " Enumeration Name : value", itMapEnumerations.first, itMapEnumerations.second);
    }
}

void CAmXmlConfigParser::printListGateways(gc_Configuration_s *pConfiguration) const
{
    LOG_FN_INFO(__FILENAME__, __func__, " Gateways Information start *****");
    for (const auto &itListGateways :  pConfiguration->listGateways)
    {
        LOG_FN_INFO(__FILENAME__, __func__, " Gateway Information begin");
        LOG_FN_INFO(__FILENAME__, __func__, " Gateway Name ", itListGateways.name);
        LOG_FN_INFO(__FILENAME__, __func__, " Source Name ", itListGateways.sourceName);
        LOG_FN_INFO(__FILENAME__, __func__, " SinkName ", itListGateways.sinkName);
        LOG_FN_INFO(__FILENAME__, __func__, " DomainName ", itListGateways.controlDomainName);
        LOG_FN_INFO(__FILENAME__, __func__, " Number of Conversion matrix ", itListGateways.listConvertionmatrix.size());
        for (const auto &it: itListGateways.listConvertionmatrix )
        {
            LOG_FN_INFO(__FILENAME__, __func__, " DomainName ", it.first, it.second);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " Gateway Information end");
    }

    LOG_FN_INFO(__FILENAME__, __func__, " Gateways Information end *****");
}

void CAmXmlConfigParser::printListDomains(gc_Configuration_s *pConfiguration) const
{
    LOG_FN_INFO(__FILENAME__, __func__, " Template Domains Information start *****");
    for (const auto &itListDomains : pConfiguration->listTemplateDomains)
    {
        LOG_FN_INFO(__FILENAME__, __func__, " Domain Information begin");
        LOG_FN_INFO(__FILENAME__, __func__, " DomainID ", itListDomains.domainID);
        LOG_FN_INFO(__FILENAME__, __func__, " Name ", itListDomains.name);
        LOG_FN_INFO(__FILENAME__, __func__, " BusName ", itListDomains.busname);
        LOG_FN_INFO(__FILENAME__, __func__, " NodeName ", itListDomains.nodename);
        LOG_FN_INFO(__FILENAME__, __func__, " RegistrationType ", itListDomains.registrationType);
        LOG_FN_INFO(__FILENAME__, __func__, " complete ", itListDomains.complete);
        LOG_FN_INFO(__FILENAME__, __func__, " early ", itListDomains.early);
        LOG_FN_INFO(__FILENAME__, __func__, " state ", itListDomains.state);
        LOG_FN_INFO(__FILENAME__, __func__, " Domain Information end");
    }

    LOG_FN_INFO(__FILENAME__, __func__, " Template Domains Information end *****");

    LOG_FN_INFO(__FILENAME__, __func__, " Domains Information start *****");
    for (const auto &itListDomains : pConfiguration->listDomains)
    {
        LOG_FN_INFO(__FILENAME__, __func__, " Domain Information begin");
        LOG_FN_INFO(__FILENAME__, __func__, " DomainID ", itListDomains.domainID);
        LOG_FN_INFO(__FILENAME__, __func__, " Name ", itListDomains.name);
        LOG_FN_INFO(__FILENAME__, __func__, " BusName ", itListDomains.busname);
        LOG_FN_INFO(__FILENAME__, __func__, " NodeName ", itListDomains.nodename);
        LOG_FN_INFO(__FILENAME__, __func__, " RegistrationType ", itListDomains.registrationType);
        LOG_FN_INFO(__FILENAME__, __func__, " complete ", itListDomains.complete);
        LOG_FN_INFO(__FILENAME__, __func__, " early ", itListDomains.early);
        LOG_FN_INFO(__FILENAME__, __func__, " state ", itListDomains.state);
        LOG_FN_INFO(__FILENAME__, __func__, " Domain Information end");
    }

    LOG_FN_INFO(__FILENAME__, __func__, " Domains Information end *****");
}

void CAmXmlConfigParser::printListClasses(gc_Configuration_s *pConfiguration) const
{
    LOG_FN_INFO(__FILENAME__, __func__, " Classes Information start *****");
    for (const auto &itListClasses : pConfiguration->listClasses)
    {
        LOG_FN_INFO(__FILENAME__, __func__, " Class Information begin");
        LOG_FN_INFO(__FILENAME__, __func__, " Name ", itListClasses.name);
        LOG_FN_INFO(__FILENAME__, __func__, " Type ", itListClasses.type);
        LOG_FN_INFO(__FILENAME__, __func__, " Priority ", itListClasses.priority);
        LOG_FN_INFO(__FILENAME__, __func__, " default Volume ", itListClasses.defaultVolume);
        LOG_FN_INFO(__FILENAME__, __func__, " isPeristenceSupported ", itListClasses.isVolumePersistencySupported);
        LOG_FN_INFO(__FILENAME__, __func__, " size of topology", itListClasses.listTopologies.size());
        LOG_FN_INFO(__FILENAME__, __func__, " Num Of Properties", itListClasses.listClassProperties.size());
        for (const auto &itListProperties : itListClasses.listClassProperties)
        {
            LOG_FN_INFO(__FILENAME__, __func__, "Type=", itListProperties.classProperty, " Value=", itListProperties.value);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " Class Information end");
    }

    LOG_FN_INFO(__FILENAME__, __func__, " Classes Information end *****");
}

void CAmXmlConfigParser::printListSinks(gc_Configuration_s *pConfiguration) const
{
    LOG_FN_INFO(__FILENAME__, __func__, " Sinks Information start *****");
    for (const auto &itListSinks : pConfiguration->listSinks)
    {
        LOG_FN_INFO(__FILENAME__, __func__, " Sink Information start *****");
        LOG_FN_INFO(__FILENAME__, __func__, " Name ", itListSinks.name);
        LOG_FN_INFO(__FILENAME__, __func__, " ClassID ", itListSinks.sinkClassID);
        LOG_FN_INFO(__FILENAME__, __func__, " SinkID ", itListSinks.sinkID);
        LOG_FN_INFO(__FILENAME__, __func__, " Domain ID ", itListSinks.domainID);
        LOG_FN_INFO(__FILENAME__, __func__, " Volume ", itListSinks.volume);
        LOG_FN_INFO(__FILENAME__, __func__, " Visible ", itListSinks.visible);
        LOG_FN_INFO(__FILENAME__, __func__, " availability: reason ", itListSinks.available.availability, ":", itListSinks.available.availabilityReason);
        LOG_FN_INFO(__FILENAME__, __func__, " Mutestate ", itListSinks.muteState);
        LOG_FN_INFO(__FILENAME__, __func__, " mainVolume ", itListSinks.mainVolume);

        LOG_FN_INFO(__FILENAME__, __func__, " Domain Name ", itListSinks.domainName);
        LOG_FN_INFO(__FILENAME__, __func__, " Class Name ", itListSinks.className);
        LOG_FN_INFO(__FILENAME__, __func__, " RegistrationType", itListSinks.registrationType);
        LOG_FN_INFO(__FILENAME__, __func__, " isVolumeSupported", itListSinks.isVolumeChangeSupported);
        LOG_FN_INFO(__FILENAME__, __func__, " priority", itListSinks.priority);
        /**
         * Print the volume user-> Norm table
         */

        LOG_FN_INFO(__FILENAME__, __func__, " Connection Formats Begin--------------------");
        for (const auto &itlistConnectionFormats : itListSinks.listConnectionFormats)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " Connection Format ", itlistConnectionFormats);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " Connection Formats Ends--------------------");
        LOG_FN_INFO(__FILENAME__, __func__, " User -> Norm Begin---------------------");
        for (const auto &itMapUserVolumeToNormalizedVolume : itListSinks.mapUserVolumeToNormalizedVolume)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " mainVolume:Norm ", itMapUserVolumeToNormalizedVolume.first, ":", itMapUserVolumeToNormalizedVolume.second);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " User -> Norm Ends------------------------");
        LOG_FN_INFO(__FILENAME__, __func__, " Norm 2 Decibel Begin------------------------");
        for (const auto &itMapNormalizedVolumeToDecibelVolume : itListSinks.mapNormalizedVolumeToDecibelVolume)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " Norm:Decibel ", itMapNormalizedVolumeToDecibelVolume.first, ":", itMapNormalizedVolumeToDecibelVolume.second);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " Norm 2 Decibel end------------------------");
        LOG_FN_INFO(__FILENAME__, __func__, " MainSoundProperties begin-------------------");
        for (const auto &itlistMainSoundProperties : itListSinks.listGCMainSoundProperties)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " Main Sound Property Type:Value:Min:Max",
                itlistMainSoundProperties.type,
                itlistMainSoundProperties.value,
                itlistMainSoundProperties.minValue,
                itlistMainSoundProperties.maxValue);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " MainSoundProperties End-------------------");
        LOG_FN_INFO(__FILENAME__, __func__, " Sound Properties begin-------------------");
        for (const auto &itlistSoundProperties : itListSinks.listGCSoundProperties)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " Type:Value:Min:Max ",
                itlistSoundProperties.type,
                itlistSoundProperties.value,
                itlistSoundProperties.minValue,
                itlistSoundProperties.maxValue);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " Sound Properties End------------------------");
        LOG_FN_INFO(__FILENAME__, __func__, " MSP:SP Begin------------------------");
        std::map<gc_MSPMappingDirection_e, std::map<uint16_t, uint16_t > >::const_iterator itMapMSPSP = itListSinks.mapMSPTOSP.find(MD_MSP_TO_SP);
        if (itMapMSPSP != itListSinks.mapMSPTOSP.end())
        {
            for (const auto &itMap : itMapMSPSP->second)
            {
                LOG_FN_INFO(__FILENAME__, __func__, " MSP:SP ", itMap.first, itMap.second);
            }
        }

        LOG_FN_INFO(__FILENAME__, __func__, " MSP:SP End------------------------");
        LOG_FN_INFO(__FILENAME__, __func__, " SP:MSP Begin------------------------");
        itMapMSPSP = itListSinks.mapMSPTOSP.find(MD_SP_TO_MSP);
        if (itMapMSPSP != itListSinks.mapMSPTOSP.end())
        {
            for (auto itMap : itMapMSPSP->second)
            {
                LOG_FN_INFO(__FILENAME__, __func__, " SP:MSP ", itMap.first, itMap.second);
            }
        }

        LOG_FN_INFO(__FILENAME__, __func__, " MSP:SP End------------------------");

        for (const auto itNotifConf : itListSinks.listNotificationConfigurations)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " Notification Configuration ", itNotifConf.type, itNotifConf.status, itNotifConf.parameter);
        }

        for (const auto itNotifConf : itListSinks.listMainNotificationConfigurations)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " Notification Configuration ", itNotifConf.type, itNotifConf.status, itNotifConf.parameter);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " Sink Information end *****");
    }

    LOG_FN_INFO(__FILENAME__, __func__, " Sink Information end *****");
}

void CAmXmlConfigParser::printListSources(gc_Configuration_s *pConfiguration) const
{
    std::vector<gc_Source_s>::iterator                 itSources;
    std::map<int16_t, float>::iterator                 itMapUserVolumeToNormalizedVolume;
    std::map<float, float >::iterator                  itMapNormalizedVolumeToDecibleVolume;
    std::vector<am_CustomConnectionFormat_t>::iterator itlistConnectionFormats;
    std::vector<gc_MainSoundProperty_s>::iterator      itlistMainSoundProperties;
    std::vector<gc_SoundProperty_s>::iterator          itlistSoundProperties;
    std::map<uint16_t, uint16_t>::iterator             itMapMSPSP;
    LOG_FN_INFO(__FILENAME__, __func__, " Source Information start *****");
    for (const auto &itSources : pConfiguration->listSources)
    {
        LOG_FN_INFO(__FILENAME__, __func__, " Source Information begin");
        LOG_FN_INFO(__FILENAME__, __func__, " Name ", itSources.name);
        LOG_FN_INFO(__FILENAME__, __func__, " ClassID ", itSources.sourceClassID);
        LOG_FN_INFO(__FILENAME__, __func__, " SourceID ", itSources.sourceID);
        LOG_FN_INFO(__FILENAME__, __func__, " DomainID ", itSources.domainID);
        LOG_FN_INFO(__FILENAME__, __func__, " Volume ", itSources.volume);
        LOG_FN_INFO(__FILENAME__, __func__, " Visible ", itSources.visible);
        LOG_FN_INFO(__FILENAME__, __func__, " availability: reason ", itSources.available.availability, ":", itSources.available.availabilityReason);
        LOG_FN_INFO(__FILENAME__, __func__, " Domain Name ", itSources.domainName);
        LOG_FN_INFO(__FILENAME__, __func__, " Class Name ", itSources.className);
        LOG_FN_INFO(__FILENAME__, __func__, " RegistrationType", itSources.registrationType);
        LOG_FN_INFO(__FILENAME__, __func__, " isVolumeSupported", itSources.isVolumeChangeSupported);
        LOG_FN_INFO(__FILENAME__, __func__, " priority", itSources.priority);
        LOG_FN_INFO(__FILENAME__, __func__, " Connection Formats Begin-------------------");
        for (const auto &itlistConnectionFormats : itSources.listConnectionFormats)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " Connection Format ", itlistConnectionFormats);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " Connection Formats End-------------------");
        LOG_FN_INFO(__FILENAME__, __func__, " MainSoundProperties Begin----------------");
        for (const auto &itlistMainSoundProperties : itSources.listGCMainSoundProperties)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " Main Sound Property Type:Value:Min:Max",
                itlistMainSoundProperties.type,
                itlistMainSoundProperties.value,
                itlistMainSoundProperties.minValue,
                itlistMainSoundProperties.maxValue);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " MainSoundProperties End----------------");
        LOG_FN_INFO(__FILENAME__, __func__, " Sound Properties Begin-------------------");
        for (const auto &itlistSoundProperties : itSources.listGCSoundProperties)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " Type:Value:Min:Max ",
                itlistSoundProperties.type,
                itlistSoundProperties.value,
                itlistSoundProperties.minValue,
                itlistSoundProperties.maxValue);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " Sound Properties End-------------------");
        std::map<gc_MSPMappingDirection_e, std::map<uint16_t, uint16_t > >::const_iterator itMapMSPSP = itSources.mapMSPTOSP.find(MD_MSP_TO_SP);
        LOG_FN_INFO(__FILENAME__, __func__, " MSP:SP Begin ------------------------");
        if ( itMapMSPSP != itSources.mapMSPTOSP.end())
        {
            for (const auto &itMap : itMapMSPSP->second)
            {
                LOG_FN_INFO(__FILENAME__, __func__, " MSP:SP ", itMap.first, itMap.second);
            }
        }

        LOG_FN_INFO(__FILENAME__, __func__, " MSP:SP End ------------------------");
        itMapMSPSP = itSources.mapMSPTOSP.find(MD_SP_TO_MSP);
        LOG_FN_INFO(__FILENAME__, __func__, " SP:MSP Begin ------------------------");
        if ( itMapMSPSP != itSources.mapMSPTOSP.end())
        {
            for (const auto &itMap : itMapMSPSP->second)
            {
                LOG_FN_INFO(__FILENAME__, __func__, " MSP:SP ", itMap.first, itMap.second);
            }
        }

        for (const auto &itVolumeMap: itSources.mapSourceVolume)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " VolumeMap ", itVolumeMap.first, itVolumeMap.second);
        }

        for (const auto &itNotifConf: itSources.listNotificationConfigurations)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " Notification Configuration ", itNotifConf.type, itNotifConf.status, itNotifConf.parameter);
        }

        for (const auto &itMainNotifConf: itSources.listMainNotificationConfigurations)
        {
            LOG_FN_INFO(__FILENAME__, __func__, " Main Notification Configuration ", itMainNotifConf.type, itMainNotifConf.status, itMainNotifConf.parameter);
        }

        LOG_FN_INFO(__FILENAME__, __func__, " SP:MSP End ------------------------");
        LOG_FN_INFO(__FILENAME__, __func__, " Source Information end");
    }

    LOG_FN_INFO(__FILENAME__, __func__, " Source Information end *****");
}

void CAmXmlConfigParser::printListPolicies(gc_Configuration_s *pConfiguration) const
{
    const char *str_trigger[] =
    {
        "TRIGGER_UNKNOWN",
        "USER_CONNECTION_REQUEST",
        "USER_DISCONNECTION_REQUEST",
        "USER_SET_SINK_MUTE_STATE",
        "USER_SET_VOLUME",
        "USER_SET_SINK_MAIN_SOUND_PROPERTY",
        "USER_SET_SINK_MAIN_SOUND_PROPERTIES",
        "USER_SET_SOURCE_MAIN_SOUND_PROPERTY",
        "USER_SET_SOURCE_MAIN_SOUND_PROPERTIES",
        "USER_SET_SYSTEM_PROPERTY",
        "USER_SET_SYSTEM_PROPERTIES",
        "USER_SET_SINK_MAIN_NOTIFICATION_CONFIGURATION",
        "USER_SET_SOURCE_MAIN_NOTIFICATION_CONFIGURATION",
        "SYSTEM_SOURCE_AVAILABILITY_CHANGED",
        "SYSTEM_SINK_AVAILABILITY_CHANGED",
        "SYSTEM_INTERRUPT_STATE_CHANGED",
        "SYSTEM_SINK_MUTE_STATE_CHANGED",
        "SYSTEM_SINK_MAIN_SOUND_PROPERTY_CHANGED",
        "SYSTEM_SOURCE_MAIN_SOUND_PROPERTY_CHANGED",
        "SYSTEM_VOLUME_CHANGED",
        "SYSTEM_SINK_NOTIFICATION_DATA_CHANGED",
        "SYSTEM_SOURCE_NOTIFICATION_DATA_CHANGED",
        "SYSTEM_REGISTER_DOMAIN",
        "SYSTEM_REGISTER_SOURCE",
        "SYSTEM_REGISTER_SINK",
        "SYSTEM_REGISTER_GATEWAY",
        "SYSTEM_DEREGISTER_DOMAIN",
        "SYSTEM_DEREGISTER_SINK",
        "SYSTEM_DEREGISTER_SOURCE",
        "SYSTEM_DEREGISTER_GATEWAY",
        "SYSTEM_DOMAIN_REGISTRATION_COMPLETE",
        "SYSTEM_CONNECTION_STATE_CHANGE",
        "SYSTEM_STORED_SINK_VOLUME",
        "SYSTEM_ALL_DOMAIN_REGISTRATION_COMPLETE",
        "USER_ALL_TRIGGER",
        "TRIGGER_MAX"
    };

    LOG_FN_INFO(__FILENAME__, __func__, " Policy Information start *****");

    for (const auto &itListPolicies : pConfiguration->listPolicies)
    {
        LOG_FN_INFO(__FILENAME__, __func__, " Policy Information");
        for (const auto &ittrigger : itListPolicies.listEvents)
        {
            LOG_FN_INFO(__FILENAME__, __func__, "TRIGGER=", str_trigger[ittrigger]);
        }

        for (const auto &itprocess : itListPolicies.listProcesses)
        {
            LOG_FN_INFO(__FILENAME__, __func__, "PROCESS START");
            LOG_FN_INFO(__FILENAME__, __func__, "comment", itprocess.comment);
            LOG_FN_INFO(__FILENAME__, __func__, "stopEvaluation", itprocess.stopEvaluation);
            LOG_FN_INFO(__FILENAME__, __func__, "priority", itprocess.priority);
            for (auto const &itconditions : itprocess.listConditions)
            {
                LOG_FN_INFO(__FILENAME__, __func__, "-->LHS function name", itconditions.leftObject.functionName);
                LOG_FN_INFO(__FILENAME__, __func__, "LHS category name<--", itconditions.leftObject.category);
                LOG_FN_INFO(__FILENAME__, __func__, "LHS mandatory name<--", itconditions.leftObject.mandatoryParameter);
                LOG_FN_INFO(__FILENAME__, __func__, "LHS optionalParameter <--", itconditions.leftObject.optionalParameter);
                LOG_FN_INFO(__FILENAME__, __func__, "LHS optionalParameter2<--", itconditions.leftObject.optionalParameter2);
            }

            for (const auto &itactions : itprocess.listActions)
            {
                LOG_FN_INFO(__FILENAME__, __func__, "action:", itactions.actionName);
                for (const auto &itMapParams : itactions.mapParameters)
                {
                    LOG_FN_INFO(__FILENAME__, __func__, itMapParams.first, "=", itMapParams.second);
                }
            }

            LOG_FN_INFO(__FILENAME__, __func__, "PROCESS END");
        }

        LOG_FN_INFO(__FILENAME__, __func__, " END Policy Information");
    }

    LOG_FN_INFO(__FILENAME__, __func__, " Policy Information end *****");

}

void CAmXmlConfigParser::printListSystemProperties(gc_Configuration_s *pConfiguration) const
{
    for (auto it : pConfiguration->listSystemProperties)
    {
        LOG_FN_INFO(__FILENAME__, __func__, "System Property Type:Value", it.type, it.value);
    }
}
#endif  // if AMCO_DEBUGGING

}
/* namespace gc */
} /* namespace am */
