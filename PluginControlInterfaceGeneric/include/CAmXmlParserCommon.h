/**************************************************************************//**
 * @file CAmXmlParserCommon.h
 *
 * This files exports the xml parsing common classes.
 *
 * @component{AudioManager Generic Controller}
 *
 * @authors Toshiaki Isogai <tisogai@jp.adit-jv.com>,\n
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>,\n
 *          Prashant Jain   <pjain@jp.adit-jv.com>,\n
 *          Martin Koch     <mkoch@de.adit-jv.com>,\n
 *          Yogesh Sharma   <external.anilkanth@jp.adit-jv.com>
 *
 * @copyright (c) 2015 - 2019 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 *//**************************************************************************/

#ifndef GC_CAMXMLPARSERCOMMON_H_
#define GC_CAMXMLPARSERCOMMON_H_

#include <libxml/parser.h>
#include "CAmTypes.h"
#include "CAmLogger.h"

namespace am
{
namespace gc
{

#define PARSE_TAG_MISMATCH -1
#define PARSE_ERROR        -2
#define PARSE_SUCCESS      0

class CAmXmlConfigParserCommon
{
public:
    static am_Error_e getEnumerationValue(const std::string &str, int &enumerationValue);

    // map to store the value of enumeration as given in schema
    static std::map<std::string, int> mMapEnumerations;

    template <typename T>
    static T convert(const std::string &);

};

template <typename T>
T CAmXmlConfigParserCommon::convert(const std::string &data)
{
    if ( !data.empty())
    {
        T                  ret;
        std::istringstream iss(data);
        if ( data.find("0x") != std::string::npos )
        {
            iss >> std::hex >> ret;
        }
        else
        {
            iss >> std::dec >> ret;
        }

        if ( iss.fail())
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, "Convert error: cannot convert string", data);
            return T();
        }

        return ret;
    }

    return T();
}

class CAmXmlNode
{
public:
    virtual ~CAmXmlNode()
    {
    }

    virtual int parse(const xmlDocPtr pDocument, xmlNodePtr *pCurrent) = 0;

    virtual int write(xmlDocPtr pDocument, xmlNodePtr pCurrent) { return 0;}

    xmlNodePtr getNextElementNode(xmlNodePtr pCurrent);

};

class IAmXmlNodeAttribute
{
public:
    virtual ~IAmXmlNodeAttribute()
    {
    }

    virtual int parse(xmlDocPtr pDocument, xmlNodePtr pCurrent) = 0;
    virtual int write(xmlNodePtr pCurrent) = 0;

};

class CAmNodeAttribute : public IAmXmlNodeAttribute
{
public:
    CAmNodeAttribute(const char *attributeName)
        : mpAttributeName(attributeName)
        , mParsed(false)
        , mValue(){}
    virtual ~CAmNodeAttribute(){}
    virtual int parse(xmlDocPtr pDocument, xmlNodePtr pCurrent);
    virtual int write(xmlNodePtr pCurrent);

    const char *getAttributeName();
    bool IsParsed() const;

protected:
    virtual void _saveAttribute(const std::string &str){}
    virtual void _getAttributeValue(std::string &val);

private:

    const char *mpAttributeName;
    bool        mParsed;
    std::string mValue;
};

class CAmStringAttribute : public CAmNodeAttribute
{
public:
    CAmStringAttribute(const char *attributeName, std::string &str);
protected:
    virtual void _saveAttribute(const std::string &str);
    virtual void _getAttributeValue(std::string &val);

private:
    std::string &mParsedString;
};

class CAmBoolAttribute : public CAmNodeAttribute
{
public:
    CAmBoolAttribute(const char *attributeName, bool &boolValue);

protected:
    virtual void _saveAttribute(const std::string &str);
    virtual void _getAttributeValue(std::string &val);

private:
    bool &mBoolValue;
};

class CAmint16Attribute : public CAmNodeAttribute
{
public:
    CAmint16Attribute(const char *attributeName, int16_t &pint16);

protected:
    virtual void _saveAttribute(const std::string &String);
    virtual void _getAttributeValue(std::string &val);

private:
    int16_t &mpInt16;

};

template<typename T>
class CAmIntAttribute : public CAmNodeAttribute
{
public:
    CAmIntAttribute(const char *attributeName, T &parsedInteger)
        : CAmNodeAttribute(attributeName)
        , mInteger(parsedInteger)
    {
    }

protected:
    virtual void _saveAttribute(const std::string &str)
    {
        mInteger = CAmXmlConfigParserCommon::convert<T>(str);
    }

private:
    T &mInteger;
};

class CAmSimpleNode : public CAmXmlNode
{
public:
    CAmSimpleNode(const char *tagname);
    ~CAmSimpleNode();
    int parse(const xmlDocPtr pDocument, xmlNodePtr *pCurrent);
    int write(xmlDocPtr pDocument, xmlNodePtr pCurrent);
    void addAttribute(CAmNodeAttribute *pAttribute);
    void addAttributeList(std::vector<CAmNodeAttribute *> listAttribute);

protected:
    virtual void _getAttributeValue(std::string &val){}
    virtual void _saveData(const std::string &str);

private:
    const char                     *mpTagName;
    std::string                     mValue;
    std::vector<CAmNodeAttribute *> mListAttributes;
};

class CAmStringNode : public CAmSimpleNode
{
public:
    CAmStringNode(const char *tagname, std::string &str, void *data = NULL);
    void push_back(std::vector<std::string > &listString, std::string &str);

protected:
    virtual void _getAttributeValue(std::string &val);
    void _saveData(const std::string &str);

private:
    std::string &mString;
};

class CAmint16Node : public CAmSimpleNode
{
public:
    CAmint16Node(const char *tagname, int16_t *pint16);
protected:
    virtual void _getAttributeValue(std::string &val);
    void _saveData(std::string parsedData);

private:
    int16_t *mpint16;
};

class CAmBoolNode : public CAmSimpleNode
{
public:
    CAmBoolNode(const char *attributeName, bool &parsedBool);

protected:
    virtual void _getAttributeValue(std::string &val);
    void _saveData(const std::string &str);

private:
    bool &mBool;
};

template<typename T1, typename T2>
class CAmListNode : public CAmXmlNode
{

public:
    CAmListNode(const char *tagname, std::vector<T1> &list, void *data = NULL)
        : mpTagName(tagname)
        , mList(list)
        , mpData(data)
    {
    }

    virtual int parse(const xmlDocPtr pDocument, xmlNodePtr *pCurrent)
    {
        int returnValue = PARSE_TAG_MISMATCH;
        do
        {
            T1 t1;
            T2 t2(mpTagName, t1, mpData);
            returnValue = t2.parse(pDocument, pCurrent);
            if (returnValue == PARSE_SUCCESS)
            {
                t2.push_back(mList, t1);
                *pCurrent = getNextElementNode(*pCurrent);
                if (*pCurrent == NULL)
                {
                    break;
                }
            }
        } while (returnValue == PARSE_SUCCESS);

        return returnValue;
    }

    virtual int write(xmlDocPtr pDocument, xmlNodePtr pCurrent)
    {
        int returnValue = PARSE_TAG_MISMATCH;
        for (auto it : mList)
        {
            T1 t1 = it;
            T2 t2(mpTagName, t1);
            t2.write(pDocument, pCurrent);
        }

        while (returnValue == PARSE_SUCCESS) {}

        return returnValue;
    }

private:
    const char      *mpTagName;
    std::vector<T1> &mList;
    void            *mpData;
};

class CAmComplexNode : public CAmXmlNode
{
public:
    CAmComplexNode(const char *tagname);
    virtual ~CAmComplexNode();
    virtual void expand(void) = 0;
    virtual int parse(const xmlDocPtr pDocument, xmlNodePtr *pCurrent);
    virtual int write(xmlDocPtr pDocument, xmlNodePtr pCurrent);
    virtual void addChildNode(CAmXmlNode *xmlNode);
    virtual void addChildNodeList(std::vector<CAmXmlNode *> listxmlNode);
    virtual void addAttribute(CAmNodeAttribute *pAttribute);
    virtual void addAttributeList(std::vector<CAmNodeAttribute *> listAttribute);

protected:
    virtual am_Error_e _copyComplexData(void);
    virtual void _getDefaultFromTemplate(void);

    typedef void (CAmComplexNode::*templateCopy)(void);
    std::vector<std::pair<const char *, templateCopy > > mMapTemplates;
    virtual void _storeCurrentName(void)
    {
    }

    virtual void _getSoundPropertyList(void)
    {
    }

    virtual void _getMainSoundPropertyList(void)
    {
    }

    virtual void _getNotificationConfiguration(void)
    {
    }

    virtual void _getMainNotificationConfiguration(void)
    {
    }

    virtual void _getConnectionFormats(void)
    {
    }

    virtual void _getMapMSPToSP(void)
    {
    }

    virtual void _getConversionMatrix()
    {
    }

private:
    const char                                *mpTagName;
    std::vector<CAmXmlNode *>                  mListChildNodes;
    std::map<const char *, CAmNodeAttribute *> mMapAttributes;

};

} /* namespace gc */
} /* namespace am */
#endif /* GC_CAMXMLPARSERCOMMON_H_ */
