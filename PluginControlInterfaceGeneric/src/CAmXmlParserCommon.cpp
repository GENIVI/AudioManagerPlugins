/******************************************************************************
 * @file: CAmXmlParserCommon.cpp
 *
 * This file contains the definition of common XML parsing classes
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

#include "CAmXmlParserCommon.h"
#include "CAmLogger.h"
#include <libxml/xmlwriter.h>

namespace am
{
namespace gc
{

using namespace std;

std::map<std::string, int> CAmXmlConfigParserCommon::mMapEnumerations;

am_Error_e CAmXmlConfigParserCommon::getEnumerationValue(const std::string &str, int &enumerationValue)
{
    // convert directly if input string is given in numeric form
    if (!str.empty())
    {
        char *str_end;
        int   i = strtol(str.c_str(), &str_end, 10);

        if (*str_end == 0)  // terminating zero reached - conversion succeeded
        {
            enumerationValue = i;
            return E_OK;
        }
    }

    if (str == "FALSE")
    {
        // special case boolean value, which is incorrectly mapped with value 2
        // due to 'value != 0' check inside _parseEnumInitialiser() function

        enumerationValue = (int)false;
        return E_OK;
    }

    std::map<std::string, int>::iterator itMapEnumerations = mMapEnumerations.find(str);
    if (itMapEnumerations != mMapEnumerations.end())
    {
        enumerationValue = itMapEnumerations->second;
        return E_OK;
    }

    LOG_FN_WARN(__FILENAME__, __func__, "FAILED to resolve symbolic constant:", str);

    return E_NOT_POSSIBLE;
}

xmlNodePtr CAmXmlNode::getNextElementNode(const xmlNodePtr pCurrent)
{
    xmlNodePtr pNext = (pCurrent != NULL) ? pCurrent->next : NULL;
    while (pNext != NULL && pNext->type != XML_ELEMENT_NODE)
    {
        pNext = pNext->next;
        if (pNext == NULL)
        {
            break;
        }
    }

    return pNext;
}

const char *CAmNodeAttribute::getAttributeName()
{
    return mpAttributeName;
}

int CAmNodeAttribute::parse(const xmlDocPtr pDocument, const xmlNodePtr pCurrent)
{
    xmlChar *pKey = xmlGetProp(pCurrent, reinterpret_cast<const xmlChar *>(mpAttributeName));
    if (pKey)
    {
        std::string parsedData = std::string(reinterpret_cast<const char *>(pKey));
        _saveAttribute(parsedData);
        mParsed = true;
        xmlFree(pKey);
    }

    return 0;
}

bool CAmNodeAttribute::IsParsed() const
{
    return mParsed;
}

void CAmNodeAttribute::_getAttributeValue(std::string &val)
{
    val = std::string("");
}

int CAmNodeAttribute::write(xmlNodePtr pCurrent)
{
    _getAttributeValue(mValue);
    xmlSetProp(pCurrent, (xmlChar *)(mpAttributeName), (xmlChar *)mValue.c_str());
    return 0;
}

CAmStringAttribute::CAmStringAttribute(const char *attributeName, std::string &str)
    : CAmNodeAttribute(attributeName)
    , mParsedString(str)
{
}

void CAmStringAttribute::_getAttributeValue(std::string &val)
{
    val = mParsedString;
}

void CAmStringAttribute::_saveAttribute(const std::string &str)
{
    mParsedString = str;
}

CAmBoolAttribute::CAmBoolAttribute(const char *attributeName, bool &boolValue)
    : CAmNodeAttribute(attributeName)
    , mBoolValue(boolValue)
{
}

void CAmBoolAttribute::_getAttributeValue(std::string &val)
{
    val = (mBoolValue == true) ? std::string("TRUE") : std::string("FALSE");
}

void CAmBoolAttribute::_saveAttribute(const std::string &str)
{
    mBoolValue = (str == "TRUE") ? true : false;
}

CAmint16Attribute::CAmint16Attribute(const char *attributeName, int16_t &pint16)
    : CAmNodeAttribute(attributeName)
    , mpInt16(pint16)
{
}

void CAmint16Attribute::_getAttributeValue(std::string &val)
{

    val = std::to_string(mpInt16);
}

void CAmint16Attribute::_saveAttribute(const std::string &str)
{
    int enumeratedValue;
    // enum value handling for considering the MACRO input.
    if (E_OK == CAmXmlConfigParserCommon::getEnumerationValue(str, enumeratedValue))
    {
        mpInt16 = enumeratedValue;
    }
    else
    {
        mpInt16 = CAmXmlConfigParserCommon::convert<int16_t>(str);
    }
}

CAmSimpleNode::CAmSimpleNode(const char *tagname)
    : mpTagName(tagname)
{
}

CAmSimpleNode::~CAmSimpleNode()
{
    mListAttributes.clear();
}

int CAmSimpleNode::parse(const xmlDocPtr pDocument, xmlNodePtr *pCurrent)
{
    int      returnValue = PARSE_TAG_MISMATCH;
    xmlChar *pKey;
    if (xmlStrEqual((*pCurrent)->name, reinterpret_cast<const xmlChar *>(mpTagName)))
    {
        std::vector<CAmNodeAttribute *>::iterator itListAttributes;
        for (auto &itListAttributes : mListAttributes)
        {
            itListAttributes->parse(pDocument, *pCurrent);
        }

        pKey = xmlNodeListGetString(pDocument, (*pCurrent)->xmlChildrenNode, 1);
        if (pKey)
        {
            _saveData(std::string(reinterpret_cast<const char *>(pKey)));
        }
        else
        {
            LOG_FN_WARN(__FILENAME__, __func__, "There was an error or empty tag was found", mpTagName);
            _saveData(std::string(""));
        }

        xmlFree(pKey);
        returnValue = PARSE_SUCCESS;

    }

    return returnValue;
}

int CAmSimpleNode::write(xmlDocPtr pDocument, xmlNodePtr pCurrent)
{
    xmlNodePtr pNode = xmlNewNode(0, (xmlChar *)mpTagName);
    xmlNodeSetContent(pNode, (const xmlChar *)(mValue.c_str()));
    for (auto itListAttributes : mListAttributes)
    {
        itListAttributes->write(pNode);
    }

    if (pCurrent == NULL)
    {
        xmlDocSetRootElement(pDocument, pNode);
    }
    else
    {
        xmlAddChild(pCurrent, pNode);
    }

    return PARSE_SUCCESS;
}

void CAmSimpleNode::addAttribute(CAmNodeAttribute *pAttribute)
{
    mListAttributes.push_back(pAttribute);
}

void CAmSimpleNode::addAttributeList(std::vector<CAmNodeAttribute *> listAttribute)
{
    mListAttributes = listAttribute;
}

void CAmSimpleNode::_saveData(const std::string &parsedData)
{
}

CAmStringNode::CAmStringNode(const char *tagname, std::string &str, void *data)
    : CAmSimpleNode(tagname)
    , mString(str)
{
}

void CAmStringNode::_getAttributeValue(std::string &val)
{
    val = mString;
}

void CAmStringNode::push_back(std::vector<std::string > &listString, std::string &str)
{
    listString.push_back(str);
}

void CAmStringNode::_saveData(const std::string &str)
{
    mString = str;
}

CAmint16Node::CAmint16Node(const char *tagname, int16_t *pint16)
    : CAmSimpleNode(tagname)
    , mpint16(pint16)
{
}

void CAmint16Node::_getAttributeValue(std::string &val)
{
    val = std::to_string(*mpint16);
}

void CAmint16Node::_saveData(std::string parsedData)
{
    *mpint16 = atoi(parsedData.c_str());
}

CAmBoolNode::CAmBoolNode(const char *attributeName, bool &parsedBool)
    : CAmSimpleNode(attributeName)
    , mBool(parsedBool)
{
}

void CAmBoolNode::_getAttributeValue(std::string &val)
{
    val = (mBool == true) ? "TRUE" : "FALSE";
}

void CAmBoolNode::_saveData(const std::string &str)
{
    mBool = (str == "TRUE" || str == "true" || str == "1") ? true : false;
}

CAmComplexNode::CAmComplexNode(const char *tagname)
    : mpTagName(tagname)
{
}

CAmComplexNode::~CAmComplexNode()
{
    mMapAttributes.clear();

}

int CAmComplexNode::parse(const xmlDocPtr pDocument, xmlNodePtr *pCurrent)
{

    int          returnValue  = PARSE_TAG_MISMATCH;
    unsigned int CurrentChild = 0;
    if (xmlStrEqual((*pCurrent)->name, reinterpret_cast<const xmlChar *>(mpTagName)))
    {

        /*
         * First expand the child nodes
         */
        this->expand();

        /**
         * First check if the baseName is part of the attribute?
         */
        for (const auto &it : mMapTemplates)
        {
            std::map<const char *, CAmNodeAttribute *>::iterator itListAttributes;
            itListAttributes = mMapAttributes.find(it.first);
            if (itListAttributes != mMapAttributes.end())
            {
                itListAttributes->second->parse(pDocument, *pCurrent);
                (this->*(it.second))();
            }
        }

        for (const auto &it : mMapAttributes)
        {
            if (!it.second->IsParsed())
            {
                it.second->parse(pDocument, *pCurrent);
            }
        }

        xmlNodePtr pChild = (*pCurrent)->children;
        pChild = getNextElementNode(pChild);
        /*
         * While parse for each child node
         */
        while (CurrentChild < mListChildNodes.size())
        {
            if (pChild == NULL)
            {
                break;
            }

            returnValue = mListChildNodes[CurrentChild]->parse(pDocument, &pChild);
            if (returnValue == PARSE_SUCCESS)
            {
                pChild = getNextElementNode(pChild);
            }

            CurrentChild++;
        }

        returnValue = _copyComplexData();
        if (returnValue != 0)
        {
            returnValue = PARSE_ERROR;
        }
    }

    return returnValue;
}

int CAmComplexNode::write(xmlDocPtr pDocument, xmlNodePtr pCurrent)
{
    /*
     * First expand the child nodes
     */
    this->expand();
    xmlNodePtr pNode = xmlNewNode(0, (xmlChar *)mpTagName);
    for (auto itListAttributes : mMapAttributes)
    {
        itListAttributes.second->write(pNode);
    }

    /*
     * Write child node
     */

    for (auto it : mListChildNodes)
    {
        it->write(pDocument, pNode);
    }

    if (pCurrent == NULL)
    {
        xmlDocSetRootElement(pDocument, pNode);
    }
    else
    {
        xmlAddChild(pCurrent, pNode);
    }

    return 0;
}

void CAmComplexNode::addChildNode(CAmXmlNode *xmlNode)
{
    mListChildNodes.push_back(xmlNode);
}

void CAmComplexNode::addChildNodeList(std::vector<CAmXmlNode *> listxmlNode)
{
    mListChildNodes = listxmlNode;
}

void CAmComplexNode::addAttribute(CAmNodeAttribute *pAttribute)
{
    mMapAttributes[pAttribute->getAttributeName()] = pAttribute;
}

void CAmComplexNode::addAttributeList(std::vector<CAmNodeAttribute *> listAttribute)
{
    for (const auto &it : listAttribute)
    {
        mMapAttributes[it->getAttributeName()] = it;
    }
}

am_Error_e CAmComplexNode::_copyComplexData(void)
{
    return E_OK;
}

void CAmComplexNode::_getDefaultFromTemplate(void)
{
}

} /* namespace gc */
} /* namespace am */
