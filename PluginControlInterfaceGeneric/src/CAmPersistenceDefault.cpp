/******************************************************************************
 * @file: CAmPersistenceDefault.cpp
 *
 * This file contains the definition of Persistence default class
 * (member functions and data members) used to implement the logic of
 * reading and writing data for last main connection and last volume
 * from the file system (RFS)
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Naohiro Nishiguchi<nnishiguchi@jp.adit-jv.com>
 *          Kapildev Patel, Yogesh Sharma <kpatel@jp.adit-jv.com>
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#include "CAmPersistenceDefault.h"
#include "CAmLogger.h"
#include <string.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include "CAmCommonUtility.h"
#include <libxml/xmlwriter.h>

namespace am {
namespace gc {


class CAmConnectionNode : public CAmComplexNode
{
public:
    CAmConnectionNode(const char *tagname, gc_persistence_connectionData_s &pConnection, void *data = NULL)
        : CAmComplexNode(tagname)
        , mpConnection(pConnection)
        , mSourceNameAttribute(ATTRIBUTE_SOURCE, mpConnection.sourceName)
        , mVolumeAttribute(ATTRIBUTE_VOLUME, mpConnection.volume)
        , mOrderAttribute(ATTRIBUTE_ORDER, mpConnection.order)
        , mLastConValidAttribute(ATTRIBUTE_LASTCON_VALID, mpConnection.lastConnectionValid)
    {
        (void)data;
    }

    virtual void expand(void)
    {
        addAttribute(&mSourceNameAttribute);
        addAttribute(&mVolumeAttribute);
        addAttribute(&mOrderAttribute);
        addAttribute(&mLastConValidAttribute);
    }

    void push_back(std::vector<gc_persistence_connectionData_s > &listConnection, gc_persistence_connectionData_s &connection)
    {
        listConnection.push_back(connection);
    }

private:
    gc_persistence_connectionData_s &mpConnection;
    CAmStringAttribute               mSourceNameAttribute;
    CAmint16Attribute                mVolumeAttribute;
    CAmint16Attribute                mOrderAttribute;
    CAmint16Attribute                mLastConValidAttribute;
};

class CAmStackNode : public CAmComplexNode
{
public:
    CAmStackNode(const char *tagname, gc_persistence_stackData_s &stack, void *data = NULL)
        : CAmComplexNode(tagname)
        , mpStack(stack)
        , mConnectionListNode(TAG_CONNECTION, mpStack.listConnection)
        , mSinkNameAttribute(ATTRIBUTE_SINK, mpStack.sinkName)
        , mVolumeAttribute(ATTRIBUTE_VOLUME, mpStack.volume)
    {
        (void)data;
    }

    virtual void expand(void)
    {
        addChildNode(&mConnectionListNode);
        addAttribute(&mSinkNameAttribute);
        addAttribute(&mVolumeAttribute);
    }

    void push_back(std::vector<gc_persistence_stackData_s > &listStack, gc_persistence_stackData_s &stack)
    {
        listStack.push_back(stack);
    }

private:
    gc_persistence_stackData_s &mpStack;
    CAmListNode< gc_persistence_connectionData_s, CAmConnectionNode> mConnectionListNode;
    CAmStringAttribute          mSinkNameAttribute;
    CAmint16Attribute           mVolumeAttribute;

};

class CAmPersistenceMainSoundPropertyNode : public CAmComplexNode
{
public:
    CAmPersistenceMainSoundPropertyNode(const char *tagname, gc_persistence_mainSoundPropertyData_s &iMainSoundProperty, void *data = NULL)
        : CAmComplexNode(tagname)
        , mpMainSoundProperty(iMainSoundProperty)
        , mTypeAttribute(ATTRIBUTE_VALUE, mpMainSoundProperty.value)
        , mValueAttribute(ATTRIBUTE_TYPE, mpMainSoundProperty.type)
    {
        (void)data;
    }

    virtual void expand(void)
    {
        addAttribute(&mTypeAttribute);
        addAttribute(&mValueAttribute);
    }

    void push_back(std::vector<gc_persistence_mainSoundPropertyData_s > &listMainSoundProperty, gc_persistence_mainSoundPropertyData_s &rMainSoundProperty)
    {
        listMainSoundProperty.push_back(rMainSoundProperty);
    }

private:
    gc_persistence_mainSoundPropertyData_s &mpMainSoundProperty;
    CAmStringAttribute mTypeAttribute;
    CAmStringAttribute mValueAttribute;
};

class CAmMainSoundPropertiesNode : public CAmComplexNode
{
public:
    CAmMainSoundPropertiesNode(const char *tagname, gc_persistence_mainSoundProData_s &iMainSoundProperties, void *data = NULL)
        : CAmComplexNode(tagname)
        , mpMainSoundPro(iMainSoundProperties)
        , mMainSoundPropertyListNode(TAG_MAIN_SOUND_PROPERTY, mpMainSoundPro.listMainSoundProperty)
        , mSinkAttribute(ATTRIBUTE_SINK, mpMainSoundPro.sinkName)
        , mSourceAttribute(ATTRIBUTE_SOURCE, mpMainSoundPro.sourceName)
    {
        (void)data;
    }

    virtual void expand(void)
    {
        addChildNode(&mMainSoundPropertyListNode);
        addAttribute(&mSinkAttribute);
        addAttribute(&mSourceAttribute);
    }

    void push_back(std::vector<gc_persistence_mainSoundProData_s > &listMainSoundPro, gc_persistence_mainSoundProData_s &mMainSoundProperties)
    {
        listMainSoundPro.push_back(mMainSoundProperties);
    }

private:
    gc_persistence_mainSoundProData_s &mpMainSoundPro;
    CAmListNode<gc_persistence_mainSoundPropertyData_s, CAmPersistenceMainSoundPropertyNode> mMainSoundPropertyListNode;
    CAmStringAttribute                 mSinkAttribute;
    CAmStringAttribute                 mSourceAttribute;

};

class CAmClasNode : public CAmComplexNode
{
public:
    CAmClasNode(const char *tagname, gc_persistence_classData_s &classData, void *data = NULL)
        : CAmComplexNode(tagname)
        , mpClassData(classData)
        , mListStackNode(TAG_STACK, mpClassData.listConnections)
        , mListMainSoundProNode(TAG_MAIN_SOUND_PROPERTIES, mpClassData.listMainSoundProperties)
        , mClassNameAttribute(ATTRIBUTE_NAME, mpClassData.name)
        , mVolumeAttribute(ATTRIBUTE_VOLUME, mpClassData.volume)
    {
        (void)data;
    }

    virtual void expand(void)
    {
        addChildNode(&mListStackNode);
        addChildNode(&mListMainSoundProNode);
        addAttribute(&mClassNameAttribute);
        addAttribute(&mVolumeAttribute);
    }

    void push_back(std::vector<gc_persistence_classData_s > &listClass, gc_persistence_classData_s &clas)
    {
        listClass.push_back(clas);
    }

private:
    gc_persistence_classData_s &mpClassData;
    CAmListNode< gc_persistence_stackData_s, CAmStackNode > mListStackNode;
    CAmListNode< gc_persistence_mainSoundProData_s, CAmMainSoundPropertiesNode > mListMainSoundProNode;
    CAmStringAttribute mClassNameAttribute;
    CAmint16Attribute  mVolumeAttribute;
};

class CAmSystemPropertyPersistenceNode : public CAmSimpleNode
{
public:

    CAmSystemPropertyPersistenceNode(const char *tagname, gc_persistence_systemProperty_s &systemPro, void *data = NULL)
        : CAmSimpleNode(tagname)
        , mptrSystemProperty(systemPro)
        , mTypeAttribute(ATTRIBUTE_TYPE, mptrSystemProperty.type)
        , mValueAttribute(ATTRIBUTE_VALUE, mptrSystemProperty.value)
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "is called", tagname, systemPro.type, systemPro.value);
        addAttribute(&mTypeAttribute);
        addAttribute(&mValueAttribute);
        (void)data;
    }

    void push_back(std::vector<gc_persistence_systemProperty_s> &listSystemProperty, gc_persistence_systemProperty_s &isystemProperty)
    {
        listSystemProperty.push_back(isystemProperty);
    }

private:
    gc_persistence_systemProperty_s &mptrSystemProperty;
    CAmStringAttribute               mTypeAttribute;
    CAmStringAttribute               mValueAttribute;
};

class CAmPersistenceNode : public CAmComplexNode
{
public:
    CAmPersistenceNode(const char *tagName, std::vector<gc_persistence_classData_s > &persistence, std::vector<gc_persistence_systemProperty_s> &rSystemPro)
        : CAmComplexNode(tagName)
        , mPersistence(persistence)
        , mListClassNode(TAG_CLASS, mPersistence)
        , mPersistence1(rSystemPro)
        , mListSystemPropertyNode(TAG_SYSTEM_PROPERTY, mPersistence1)
    {
    }

    virtual void expand(void)
    {
        addChildNode(&mListSystemPropertyNode);
        addChildNode(&mListClassNode);
    }

private:
    std::vector< gc_persistence_classData_s >             &mPersistence;
    std::vector< gc_persistence_systemProperty_s >        &mPersistence1;
    CAmListNode< gc_persistence_classData_s, CAmClasNode > mListClassNode;
    CAmListNode< gc_persistence_systemProperty_s, CAmSystemPropertyPersistenceNode > mListSystemPropertyNode;
};

CAmPersistenceDefault::CAmPersistenceDefault()
    : mFileName(GENERIC_CONTROLLER_PERSISTENCE_FILE)
{
    LOG_FN_DEBUG(__FILENAME__, __func__, "is Called");
    return;
}

CAmPersistenceDefault::~CAmPersistenceDefault()
{
    _createSubDirectories();
    if (_writeTOXML())
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "is called & Write data in the XML file is Fine & filename is ", mFileName);
    }
    else
    {
        LOG_FN_WARN(__FILENAME__, __func__, "is called & Write data in the XML file is Failed & filename is ", mFileName);
    }
}

am_Error_e CAmPersistenceDefault::open(const std::string &/*appName*/)
{
    am_Error_e resRead = _readFromXML();
    if (E_OK != resRead)
    {
        LOG_FN_WARN(__FILENAME__, __func__, "& _readFromXML is FAILED & file name is ", mFileName);
        return E_NON_EXISTENT;
    }

    return E_OK;
}

am_Error_e CAmPersistenceDefault::read(const std::string &keyName,
    std::string &readData, int dataSize)
{
    MapData::iterator itFileData;
    itFileData = mFileData.find(keyName);
    if (itFileData != mFileData.end())
    {
        readData = itFileData->second;
        LOG_FN_INFO(__FILENAME__, __func__, "is called & data read from Map for keyName = ", keyName, " is =", readData);
        return E_OK;
    }
    else
    {
        LOG_FN_WARN(__FILENAME__, __func__, "is called & data read from Map is Failed for keyName = ", keyName);
        return E_DATABASE_ERROR;
    }

    return E_OK;
}

am_Error_e CAmPersistenceDefault::write(const std::string &keyName,
    const std::string &writeData,
    int dataSize)
{

    LOG_FN_DEBUG(__FILENAME__, __func__, "keyName=", keyName, "Value=", writeData);
    mFileData[keyName] = writeData;
    return E_OK;
}

void CAmPersistenceDefault::_updateMainConnectionVolume(std::string className, std::string sourceName, std::string sinkName, int16_t volume, std::vector<gc_persistence_classData_s > &outVClassDataS)
{
    bool                            classFound = false;
    gc_persistence_classData_s      classData;
    gc_persistence_stackData_s      stackData;
    gc_persistence_connectionData_s connData;
    connData.sourceName = sourceName;
    connData.volume     = volume;
    stackData.sinkName  = sinkName;
    stackData.listConnection.push_back(connData);
    classData.name = className;
    classData.listConnections.push_back(stackData);
    for (auto &itClassData : outVClassDataS)
    {
        if (itClassData.name == className)
        {
            classFound = true;
            bool sinkFound = false;
            for ( auto &itStackData : itClassData.listConnections)
            {
                if (sinkName == itStackData.sinkName)
                {
                    sinkFound = true;
                    bool sourceFound = false;
                    for (auto &itSourceData : itStackData.listConnection)
                    {
                        if (sourceName == itSourceData.sourceName)
                        {
                            sourceFound         = true;
                            itSourceData.volume = volume;
                        }
                    }

                    if (sourceFound == false)
                    {
                        itStackData.listConnection.push_back(connData);
                    }
                }
            }

            if (sinkFound == false)
            {
                stackData.listConnection.push_back(connData);
                itClassData.listConnections.push_back(stackData);
            }
        }
    }

    if (classFound == false)
    {
        outVClassDataS.push_back(classData);
    }
}

bool CAmPersistenceDefault::_lastMainConnectionVolumeStruct(std::string inputStr, std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS)
{
    LOG_FN_INFO(__FILENAME__, __func__, "Input String=", inputStr);
    std::vector<std::string> listClassesStr;

    /* E.g. Input String for last main connection volume as below
     * {BASE,[source1:sink1=20][source2:sink2=30]}{PHONE,[source3:sink3=40][source4:sink4=50]}*/

    std::string delimiter = "{";
    if (E_OK != CAmCommonUtility::parseString(delimiter, inputStr, listClassesStr))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "not possible to parse last main connection volume");
        return false;
    }

    for (auto itlistClassesStr : listClassesStr)
    {
        std::vector<std::string> listClassStr;
        delimiter = ",";
        if (E_OK != CAmCommonUtility::parseString(delimiter, itlistClassesStr, listClassStr))
        {
            LOG_FN_WARN(__FILENAME__, __func__, "delimiter , not found");
            continue;
        }

        if ( listClassStr.size() != 2)
        {
            LOG_FN_WARN(__FILENAME__, __func__, "delimiter , wrong format");
            continue;
        }

        std::string className;
        className = listClassStr[0];
        std::vector<std::string> listConVolumeStr;
        delimiter = "[";
        if (E_OK != CAmCommonUtility::parseString(delimiter, listClassStr[1], listConVolumeStr))
        {
            LOG_FN_WARN(__FILENAME__, __func__, "delimiter [ not found");
            continue;
        }

        for (auto itConnVolumeStr : listConVolumeStr)
        {
            std::vector<std::string> connVolumeStr;
            std::string              sourceName;
            std::string              sinkName;
            std::string              tempVol;
            std::vector<std::string> sinkVolume;
            delimiter = ":";
            if (E_OK != CAmCommonUtility::parseString(delimiter, itConnVolumeStr, connVolumeStr))
            {
                LOG_FN_WARN(__FILENAME__, __func__, "delimiter : not found");
                continue;
            }

            if (connVolumeStr.size() != 2)
            {
                LOG_FN_WARN(__FILENAME__, __func__, "delimiter : wrong format");
                continue;
            }

            sourceName = connVolumeStr[0];
            delimiter  = "=";
            if (E_OK != CAmCommonUtility::parseString(delimiter, connVolumeStr[1], sinkVolume))
            {
                LOG_FN_WARN(__FILENAME__, __func__, "delimiter = not found");
                continue;
            }

            if (sinkVolume.size() != 2)
            {
                LOG_FN_WARN(__FILENAME__, __func__, "delimiter = wrong format");
                continue;
            }

            sinkName = sinkVolume[0];
            tempVol  = sinkVolume[1];
            if (!sinkVolume.empty())
            {
                _updateMainConnectionVolume(className, sourceName, sinkName, atoi(tempVol.c_str()), outVClassDataS);
            }
        }
    }

    return true;
}

void CAmPersistenceDefault::_updateClassVolume(gc_persistence_classData_s classData, std::vector<gc_persistence_classData_s > &outVClassDataS)
{
    bool classFound = false;
    for (auto &itClassData : outVClassDataS)
    {
        if (itClassData.name == classData.name)
        {
            classFound = true;
            for (auto itInputStackData : classData.listConnections)
            {
                bool sinkFound = false;
                for ( auto &itStackData : itClassData.listConnections)
                {
                    if (itInputStackData.sinkName == itStackData.sinkName)
                    {
                        sinkFound          = true;
                        itStackData.volume = itInputStackData.volume;
                    }
                }

                if (sinkFound == false)
                {
                    itClassData.listConnections.push_back(itInputStackData);
                }
            }
        }
    }

    if (classFound == false)
    {
        outVClassDataS.push_back(classData);
    }
}

bool CAmPersistenceDefault::_lastClassVolumeStruct(std::string inputStr,
    std::vector<gc_persistence_classData_s> &outVClassDataS,
    std::vector<gc_persistence_systemProperty_s> &outVSystemPropertyS)
{
    LOG_FN_INFO(__FILENAME__, __func__, "Input String=", inputStr);

    /*E.g. Input string for last class volume as below
     * {BASE,[ExclusiveSink1:50][ExclusiveSink2:60]}*/
    std::vector< std::string > listClassesStr;
    std::string                delimiter = "{";
    if (E_OK != CAmCommonUtility::parseString(delimiter, inputStr, listClassesStr))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "Not possible to parse the last class volume");
        return false;
    }

    for (auto itlistClassVolumesStr : listClassesStr)
    {
        gc_persistence_classData_s classData;
        std::vector<std::string>   listClassVolumes;
        delimiter = ",";
        if (E_OK != CAmCommonUtility::parseString(delimiter, itlistClassVolumesStr, listClassVolumes))
        {
            LOG_FN_WARN(__FILENAME__, __func__, "delimiter , not found");
            continue;
        }

        if (listClassVolumes.size() != 2)
        {
            LOG_FN_WARN(__FILENAME__, __func__, "delimiter , not correctly formatted");
            continue;
        }

        classData.name = listClassVolumes[0];
        std::vector<std::string> listVolumeStr;
        delimiter = "[";
        if (E_OK != CAmCommonUtility::parseString(delimiter, listClassVolumes[1], listVolumeStr))
        {
            continue;
        }

        for (auto itSinkVolume : listVolumeStr)
        {
            gc_persistence_stackData_s sinkData;
            std::vector<std::string>   listSinkVolume;
            delimiter = ":";
            if (E_OK != CAmCommonUtility::parseString(delimiter, itSinkVolume, listSinkVolume))
            {
                LOG_FN_WARN(__FILENAME__, __func__, "delimiter : not found");
                continue;
            }

            if ( listSinkVolume.size() != 2)
            {
                LOG_FN_WARN(__FILENAME__, __func__, "delimiter : not correclty formatted");
                continue;
            }

            sinkData.sinkName = listSinkVolume[0];
            sinkData.volume   = atoi(listSinkVolume[1].c_str());
            if (sinkData.sinkName == "*")
            {
                classData.volume = sinkData.volume;
                continue;
            }

            if (!listSinkVolume.empty())
            {
                classData.listConnections.push_back(sinkData);
            }
        }

        _updateClassVolume(classData, outVClassDataS);
    }

    return true;

}

void CAmPersistenceDefault::_updateMainConnection(gc_persistence_classData_s classData, std::vector<gc_persistence_classData_s > &outVClassDataS)
{
    bool classFound = false;
    for (auto &itClassData : outVClassDataS)
    {
        if (itClassData.name == classData.name)
        {
            classFound = true;
            for (auto itInputStackData : classData.listConnections)
            {
                bool sinkFound = false;
                for ( auto &itStackData : itClassData.listConnections)
                {
                    if (itInputStackData.sinkName == itStackData.sinkName)
                    {
                        sinkFound = true;
                        for (auto itInputSourceData : itInputStackData.listConnection)
                        {
                            bool sourceFound = false;
                            for (auto &itSourceData : itStackData.listConnection)
                            {
                                if (itInputSourceData.sourceName == itSourceData.sourceName)
                                {
                                    sourceFound                      = true;
                                    itSourceData.lastConnectionValid = 1;
                                }
                            }

                            if (sourceFound == false)
                            {
                                itStackData.listConnection.push_back(itInputSourceData);
                            }
                        }
                    }
                }

                if (sinkFound == false)
                {
                    itClassData.listConnections.push_back(itInputStackData);
                }
            }
        }
    }

    if (classFound == false)
    {
        outVClassDataS.push_back(classData);
    }
}

bool CAmPersistenceDefault::_lastMainConnectionStruct(std::string inputStr, std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS)
{
    LOG_FN_INFO(__FILENAME__, __func__, "Input String=", inputStr);
    std::vector< std::string> listClassesStr;
    /*E.g. Input string for the last main connection as below
     * {BASE,BaseSource1:ExclusiveSink1;BaseSource2:ExclusiveSink2;}*/

    std::string delimiter = "{";
    if (E_OK != CAmCommonUtility::parseString(delimiter, inputStr, listClassesStr))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "cannot parse last main Connection string");
        return false;
    }

    for (auto itlistClassData : listClassesStr)
    {
        gc_persistence_classData_s classData;
        std::vector<std::string>   listClassConnectionStr;

        delimiter = ",";
        if (E_OK != CAmCommonUtility::parseString(delimiter, itlistClassData, listClassConnectionStr))
        {
            LOG_FN_WARN(__FILENAME__, __func__, "parsing of class not successful still continuing");
            continue;
        }

        if (listClassConnectionStr.size() != 2)
        {
            continue;
        }

        classData.name = listClassConnectionStr[0];
        // Start internal stack/conn tags, first conn
        std::vector<std::string> listConnectionStr;

        delimiter = ";";
        if (E_OK != CAmCommonUtility::parseString(delimiter, listClassConnectionStr[1], listConnectionStr))
        {

            continue;
        }

        for (auto itListMainCon : listConnectionStr)
        {
            std::vector<std::string> listNames;
            delimiter = ":";
            if (E_OK != CAmCommonUtility::parseString(delimiter, itListMainCon, listNames))
            {
                LOG_FN_WARN(__FILENAME__, __func__, "delimiter : not found");
                continue;
            }

            if (listNames.size() != 2)
            {
                LOG_FN_WARN(__FILENAME__, __func__, "wrong formatted last main connection");
                continue;
            }

            if (!itListMainCon.empty())
            {
                gc_persistence_connectionData_s   connData;
                struct gc_persistence_stackData_s stackData;

                connData.sourceName          = listNames[0];
                connData.lastConnectionValid = 1;
                stackData.sinkName           = listNames[1];
                stackData.listConnection.push_back(connData);
                classData.listConnections.clear();
                classData.listConnections.push_back(stackData);
                _updateMainConnection(classData, outVClassDataS);
            }
        }
    }

    return true;
}

void CAmPersistenceDefault::_updateMainSoundProperty(gc_persistence_classData_s classData, std::vector<gc_persistence_classData_s > &outVClassDataS)
{
    bool updated = false;
    for (auto &it : outVClassDataS)
    {
        if (it.name == classData.name)
        {
            it.listMainSoundProperties = classData.listMainSoundProperties;
            updated                    = true;
        }
    }

    if (updated == false)
    {
        outVClassDataS.push_back(classData);
    }
}

bool CAmPersistenceDefault::_lastMainSoundPropertyStruct(std::string inputStr, std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS)
{

    LOG_FN_INFO(__FILENAME__, __func__, "Input String=", inputStr);
    /*E.g. Input string for the last main sound properties
     * {BASE,[ET_SOURCE_BaseSource=(1:3)(2:4)(3:5)]} */
    std::vector< std::string > listClasses;
    std::string                delimiter = "{";
    if (E_OK != CAmCommonUtility::parseString(delimiter, inputStr, listClasses))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "delimiter { not found");
        return false;
    }

    for (auto itlistClasses : listClasses)
    {
        gc_persistence_classData_s classData;
        std::vector<std::string>   listClass;
        delimiter = ",";
        if (E_OK != CAmCommonUtility::parseString(delimiter, itlistClasses, listClass))
        {
            continue;
        }

        if (listClass.size() != 2 )
        {
            LOG_FN_WARN(__FILENAME__, __func__, "Invalid formatted class level string");
            continue;
        }

        classData.name = listClass[0];
        // Start internal stack/conn tags, first conn
        std::vector<std::string> listElementProp;
        gc_SinkVolume_s          sinkInfo;

        delimiter = "[";
        if (E_OK != CAmCommonUtility::parseString(delimiter, listClass[1], listElementProp))
        {
            LOG_FN_WARN(__FILENAME__, __func__, "Delimiter [ not found");
            continue;
        }

        for (auto itListElememntProp : listElementProp)
        {
            gc_persistence_mainSoundProData_s listMainSoundProperties {};
            std::vector<std::string>          listPropertyListStr;
            delimiter = "=";
            if (E_OK != CAmCommonUtility::parseString(delimiter, itListElememntProp, listPropertyListStr))
            {
                continue;
            }

            if (listPropertyListStr.size() != 2)
            {
                LOG_FN_WARN(__FILENAME__, __func__, "Invalid format found property");
                continue;
            }

            std::string elementName, tType, tValue;
            elementName = listPropertyListStr[0];

            if ("ET_SOURCE" == elementName.substr(0, 9))
            {
                listMainSoundProperties.sourceName = elementName.substr(10, std::string::npos);
                listMainSoundProperties.sinkName   = "";
            }
            else
            {
                listMainSoundProperties.sinkName   = elementName.substr(8, std::string::npos);
                listMainSoundProperties.sourceName = "";
            }

            delimiter = "(";
            std::vector<std::string> listMainSoundPropertiesStr;
            if (E_OK != CAmCommonUtility::parseString(delimiter, listPropertyListStr[1], listMainSoundPropertiesStr))
            {
                LOG_FN_WARN(__FILENAME__, __func__, "wrong class level data format in the persistence database");
            }

            for (auto itlistMainSoundPropertyStr : listMainSoundPropertiesStr)
            {
                delimiter = ":";
                gc_persistence_mainSoundPropertyData_s mainSoundProperty;
                std::vector<std::string>               typeValPair;
                if (E_OK != CAmCommonUtility::parseString(delimiter, itlistMainSoundPropertyStr, typeValPair))
                {
                    LOG_FN_WARN(__FILENAME__, __func__, "delimiter : not found");
                    continue;
                }

                if (typeValPair.size() != 2)
                {
                    LOG_FN_WARN(__FILENAME__, __func__, "Invaid formatted type value pair");
                    continue;
                }

                tType                   = typeValPair[0];
                mainSoundProperty.type  = tType;
                tValue                  = typeValPair.at(1);
                mainSoundProperty.value = tValue.substr(0, 1);

                if (!typeValPair.empty())
                {
                    listMainSoundProperties.listMainSoundProperty.push_back(mainSoundProperty);
                }
            }

            if (!listPropertyListStr.empty())
            {
                classData.listMainSoundProperties.push_back(listMainSoundProperties);
            }
        }

        _updateMainSoundProperty(classData, outVClassDataS);
    }

    return true;
}

bool CAmPersistenceDefault::_lastSystemPropertyStruct(std::string inputStr, std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS)
{
    std::vector<std::string>        listSystemPropertyStr;
    gc_persistence_systemProperty_s soundProperty {};

    LOG_FN_INFO(__FILENAME__, __func__, "Input String=", inputStr);
    /*E.g. Input string for the last system property as below
     * {(61440:4)(61569:100)} */

    std::string delimiter = "(";
    if (E_OK != CAmCommonUtility::parseString(delimiter, inputStr, listSystemPropertyStr))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "wrong class level data format in the persistence database");
        return false;
    }

    for (auto itlistSystemProperty : listSystemPropertyStr)
    {
        delimiter = ":";
        std::vector<std::string> typeValPair;
        if (E_OK != CAmCommonUtility::parseString(delimiter, itlistSystemProperty, typeValPair))
        {
            LOG_FN_WARN(__FILENAME__, __func__, "delimiter : not found");
            continue;
        }

        if (typeValPair.size() != 2 )
        {
            LOG_FN_WARN(__FILENAME__, __func__, "wrong formatted string");
            continue;
        }

        soundProperty.type  = typeValPair[0];
        soundProperty.value = typeValPair[1];
        std::size_t found = soundProperty.value.find(")");
        soundProperty.value = soundProperty.value.substr(0, found);
        outVSystemPropertyS.push_back(soundProperty);
    }

    return true;
}

bool CAmPersistenceDefault::_stringToStruct(std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS)
{
    typedef bool (CAmPersistenceDefault::*toStruct)(std::string, std::vector<gc_persistence_classData_s > &, std::vector< gc_persistence_systemProperty_s > &);
    std::map<std::string, toStruct > keyMap;

    keyMap["lastMainConnectionVolume"] = &CAmPersistenceDefault::_lastMainConnectionVolumeStruct;
    keyMap["lastClassVolume"]          = &CAmPersistenceDefault::_lastClassVolumeStruct;
    keyMap["lastMainConnection"]       = &CAmPersistenceDefault::_lastMainConnectionStruct;
    keyMap["lastMainSoundProperty"]    = &CAmPersistenceDefault::_lastMainSoundPropertyStruct;
    keyMap["lastSystemProperty"]       = &CAmPersistenceDefault::_lastSystemPropertyStruct;

    for ( auto it = keyMap.begin(); it != keyMap.end(); it++)
    {
        if (!mFileData[it->first].empty())
        {
            (this->*(it->second))(mFileData[it->first], outVClassDataS, outVSystemPropertyS);
        }
    }

    return true;
}

am_Error_e CAmPersistenceDefault::_structTostring(std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS)
{
    std::string outputString;
    std::string outputStringInside;
    std::string outputStringSystem;
    std::string outputStringMainSoundProTypeValue;
    std::string outputStringLMCV;
    std::string outputStringLCV;
    std::string outputStringLMC;
    std::string outputStringLMSP;
    std::string tvolSink;
    std::string tSinkName;
    std::string tSourceName;
    std::string tType;
    std::string tValue;
    am_Error_e result = E_WRONG_FORMAT;

    if(!outVClassDataS.empty())
    {
    for ( auto itClasses : outVClassDataS)
    {
        outputString  = "{";
        outputString += (itClasses).name;
        outputString += ",";
        if (!itClasses.name.empty())
        {
            // For loop required
            // Sink Name and Volume and order also is required.
            for (auto itStacks : itClasses.listConnections)
            {
                for (auto itConnection : itStacks.listConnection)
                {
                    if (!itConnection.sourceName.empty() && !itStacks.sinkName.empty() && ((std::to_string(itConnection.volume) != std::to_string(MAX_NEG_VOL))))
                    {
                        outputStringInside += "[" + itConnection.sourceName + ":" + itStacks.sinkName + "="
                            + std::to_string(itConnection.volume) + "]";
                    }
                }
            }
        }

        outputString += outputStringInside + "}";
        outputStringInside.clear();
        outputStringLMCV += outputString;
        outputString.clear();
    }

    LOG_FN_INFO(__FILENAME__, __func__, "is called & inserted lastMainConnectionVolume OutputString:", outputStringLMCV);
    // Insert key and string details in the map which is read from Xml file via common structure.
    mFileData.insert(std::pair<std::string, std::string>("lastMainConnectionVolume", outputStringLMCV));
    outputStringInside.clear();
    outputString.clear();
    outputStringLMCV.clear();

    // ---------------LastClassVolume---start-----------
    /* E.g. Output String for last class volume as below
     * {BASE,[ExclusiveSink1:50][ExclusiveSink2:60]}*/
    for (auto itClasses : outVClassDataS)
    {
        outputString  = "{";
        outputString += itClasses.name;
        outputString += ",";
        if (!itClasses.name.empty())
        {
            // Sink Name and Volume and order also is required.
            for (auto itStacks : itClasses.listConnections)
            {
                if (!itStacks.sinkName.empty())
                {
                    tvolSink = std::to_string(itStacks.volume);
                    if (std::to_string(itStacks.volume) != std::to_string(MAX_NEG_VOL))
                    {
                        outputStringInside += "[" + itStacks.sinkName + ":" + std::to_string(itStacks.volume) + "]";     // new
                    }
                }
            }

            tvolSink = std::to_string(itClasses.volume);
            if (std::to_string(itClasses.volume) != std::to_string(MAX_NEG_VOL))
            {
                outputStringInside += "[*:" + std::to_string(itClasses.volume) + "]";     // new
            }
        }

        outputString    += outputStringInside + "}";
        outputStringLCV += outputString;
        outputStringInside.clear();
        outputString.clear();
    }

    LOG_FN_INFO(__FILENAME__, __func__, "is called & inserted lastClassVolume OutputString:", outputStringLCV);
    mFileData.insert(std::pair<std::string, std::string>("lastClassVolume", outputStringLCV));
    outputStringInside.clear();
    outputString.clear();
    outputStringLCV.clear();

    // ------------- LastMainConnection--start----------------
    /* E.g. Output String for last main connection as below
     * {BASE,BaseSource1:ExclusiveSink1;BaseSource2:ExclusiveSink2;}*/

    for (auto itClasses : outVClassDataS)
    {
        outputString  = "{";
        outputString += itClasses.name;
        outputString += ",";
        if (!itClasses.name.empty())
        {
            // Sink Name and Volume and order also is required.
            for (auto itStacks : itClasses.listConnections)
            {
                for (auto itConnections : itStacks.listConnection)
                {
                    if (!itConnections.sourceName.empty() && (itConnections.lastConnectionValid == 1))
                    {
                        if (!itStacks.sinkName.empty())
                        {
                            tvolSink            = std::to_string(itStacks.volume);
                            outputStringInside += itConnections.sourceName + ":" + itStacks.sinkName + ";";
                        }
                    }
                }
            }
        }

        outputString += outputStringInside + "}";
        outputStringInside.clear();
        outputStringLMC += outputString;
        outputStringInside.clear();
    }

    LOG_FN_INFO(__FILENAME__, __func__, "is called ok & inserted lastMainConnection OutputString:", outputStringLMC);
    mFileData.insert(std::pair<std::string, std::string>("lastMainConnection", outputStringLMC));
    outputStringInside.clear();
    outputString.clear();

    // ----------Last Main sound properties-start-----------------------
    /* E.g. Output String for last main connection properties as below
     * {BASE,[ET_SOURCE_BaseSource=(1:3)(2:4)(3:5)]} */

    for (auto itClasses : outVClassDataS)
    {
        outputString  = "{";
        outputString += (itClasses).name;
        outputString += ",";
        if (!itClasses.name.empty())
        {
            // For loop required
            // Sink Name and Volume and order also is required.
            for (auto itElementProperties : itClasses.listMainSoundProperties)
            {
                for (auto itListMainProperties : itElementProperties.listMainSoundProperty)
                {
                    if (!itListMainProperties.type.empty())
                    {
                        outputStringMainSoundProTypeValue += "(" + itListMainProperties.type + ":" + itListMainProperties.value + ")";
                    }
                }

                if (!(itElementProperties).sourceName.empty())
                {
                    outputStringInside += "[ET_SOURCE_" + itElementProperties.sourceName + "=" + outputStringMainSoundProTypeValue + "]";      // new
                }
                else
                {
                    outputStringInside += "[ET_SINK_" + itElementProperties.sinkName + "=" + outputStringMainSoundProTypeValue + "]";      // new
                }
            }
        }

        outputString += outputStringInside + "}";
        outputStringInside.clear();
        outputStringLMSP += outputString;
    }

    LOG_FN_INFO(__FILENAME__, __func__, "is called & inserted lastMainSoundProperties OutputString:", outputStringLMSP);
    // Insert key and string details in the map which is read from Xml file via common structure.
    mFileData.insert(std::pair<std::string, std::string>("lastMainSoundProperty", outputStringLMSP));
    outputString.clear();
    result = E_OK;
    }

    // ---------------LastSystemProperties---start----------------
    /*  E.g. Output String for last system properties as below
     *  {(61440:4)(61569:100)} */
    if(!outVSystemPropertyS.empty())
    {
    for (auto itListSYstemProperties : outVSystemPropertyS)
    {
        outputString = "{";
        if (!itListSYstemProperties.type.empty())
        {
            if (!itListSYstemProperties.value.empty())
            {
                tValue              = itListSYstemProperties.value;
                outputStringSystem += "(" + itListSYstemProperties.type + ":" + itListSYstemProperties.value + ")"; // new
            }
        }

        outputString += outputStringSystem + "}";
    }

    LOG_FN_INFO(__FILENAME__, __func__, "is called & inserted LastSystemProperty OutputString:", outputString);
    mFileData.insert(std::pair<std::string, std::string>("lastSystemProperty", outputString));
    outputStringSystem.clear();
    outputString.clear();
    result = E_OK;
    }
    return result;
}

bool CAmPersistenceDefault::_writeTOXML()
{
    vector<gc_persistence_classData_s >      mvClassDataS;
    vector<gc_persistence_systemProperty_s > mvSystemPropertyS;
    bool                                     resData = _stringToStruct(mvClassDataS, mvSystemPropertyS);
    if (resData == false)
    {
        LOG_FN_WARN(__FILENAME__, __func__, "is called & Write data in the XML file is Failed");
    }

    xmlDocPtr pDocument = xmlNewDoc((const xmlChar *)"1.0");
    if (pDocument == NULL)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "is called & pDocument is NULL");
        return false;
    }

    CAmPersistenceNode(TAG_ROOT_PERSISTENCE, mvClassDataS, mvSystemPropertyS).write(pDocument, NULL);

    xmlSaveFormatFileEnc(mFileName.c_str(), pDocument, "UTF-8", 1);
    xmlFreeDoc(pDocument);
    return true;
}

am_Error_e CAmPersistenceDefault::_readFromXML()
{
    vector<gc_persistence_classData_s >      outVClassDataS;
    vector<gc_persistence_systemProperty_s > outVSystemPropertyS;
    char                                    *persistencePath = (char *)getenv(PERISTENCE_FILE_PATH_ENV_VAR_NAME);

    if (persistencePath != NULL)
    {
        mFileName = persistencePath;
        LOG_FN_INFO(__FILENAME__, __func__, "is called & Input mFileName is", mFileName.c_str());
    }

    if (!mFileName.empty())
    {
        xmlDocPtr pDocument = xmlParseFile(mFileName.c_str());
        if (pDocument == NULL)
        {
            LOG_FN_WARN("failed to parse the persistence xml file & Input mFileName is", mFileName);
            return E_NON_EXISTENT;
        }

        xmlNodePtr pCurrent = xmlDocGetRootElement(pDocument);
        if (pCurrent == NULL)
        {
            LOG_FN_ERROR("failed to get persistence xml file root node");
            return E_NON_EXISTENT;
        }

        CAmPersistenceNode(TAG_ROOT_PERSISTENCE, outVClassDataS, outVSystemPropertyS).parse(pDocument, &pCurrent);

        xmlFreeDoc(pDocument);
    }
    else
    {
        LOG_FN_WARN(__FILENAME__, __func__, "is called and File name is Empty, not able to read persistence xml file ", mFileName);
        return E_NON_EXISTENT;
    }

    am_Error_e resultStructToStringToMap = _structTostring(outVClassDataS, outVSystemPropertyS);
    if (E_OK != resultStructToStringToMap)
    {
        LOG_FN_WARN("failed to get persistence xml file in structure and map details");
        return E_WRONG_FORMAT;
    }

    LOG_FN_INFO("Peristence xml parsed successfully & Xml file  data insert in map successfully");
    return E_OK;
}

am_Error_e CAmPersistenceDefault::close()
{
    LOG_FN_INFO(__FILENAME__, __func__, "is called !!!");
    return E_OK;
}

am_Error_e CAmPersistenceDefault::_createDirectory(const std::string &path, mode_t mode)
{
    struct stat st;

    if (path.empty() || (stat(path.c_str(), &st) == 0))
    {
        return E_OK;
    }

    if (mkdir(path.c_str(), mode) != 0 && errno != EEXIST)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "failed to create directory", path);
        return E_NOT_POSSIBLE;
    }

    return E_OK;
}

void CAmPersistenceDefault::_createSubDirectories()
{
    if (mFileName.empty())
    {
        return;
    }

    for (size_t pos = 0; (pos = mFileName.find("/", pos)) != std::string::npos; ++pos)
    {
        std::string dirName = mFileName.substr(0, pos);
        (void)_createDirectory(dirName, FILE_ACCESS_MODE);
    }
}

} /* namespace gc */
} /* namespace am */
