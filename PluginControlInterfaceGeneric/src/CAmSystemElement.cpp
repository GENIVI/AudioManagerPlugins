/******************************************************************************
 * @file: CAmSystemElement.cpp
 *
 * This file contains the definition of system element class (member functions
 * and data members) used as data container to store the information related to
 * system as maintained by controller.
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

#include "CAmSystemElement.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmSystemElement::CAmSystemElement(const gc_System_s &systemData, IAmControlReceive *pControlReceive)
    : CAmElement(ET_SYSTEM, systemData.name, pControlReceive)
    , mSystem(systemData)
{
    setID(SYSTEM_ID);

    am_SystemProperty_s systemProperty;

    for (auto &itlistGCSystemProperties : systemData.listGCSystemProperties)
    {
        systemProperty.type  = itlistGCSystemProperties.type;
        systemProperty.value = itlistGCSystemProperties.value;
        mListSystemProperties.push_back(systemProperty);
    }

    systemProperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemProperty.value = LOG_DEBUG_DEFAULT_VALUE;
    int16_t value;
    if (E_OK != _findSystemProperty(mListSystemProperties, SYP_GLOBAL_LOG_THRESHOLD, value))
    {
        mListSystemProperties.push_back(systemProperty);
    }

    systemProperty.type  = SYP_REGISTRATION_ALLOW_UNKNOWN_ELEMENT;
    systemProperty.value = 0;
    if (E_OK != _findSystemProperty(mListSystemProperties, SYP_REGISTRATION_ALLOW_UNKNOWN_ELEMENT, value))
    {
        mListSystemProperties.push_back(systemProperty);
    }

    systemProperty.type  = SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES;
    systemProperty.value = 1;
    if (E_OK != _findSystemProperty(mListSystemProperties, SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES, value))
    {
        mListSystemProperties.push_back(systemProperty);

    }

    systemProperty.type  = SYP_REGISTRATION_DOMAIN_TIMEOUT;
    systemProperty.value = 10000;
    if (E_OK != _findSystemProperty(mListSystemProperties, SYP_REGISTRATION_DOMAIN_TIMEOUT, value))
    {
        mListSystemProperties.push_back(systemProperty);
    }

    systemProperty.type  = SYP_REGSTRATION_SOUND_PROP_RESTORED;
    systemProperty.value = 0;
    if (E_OK != _findSystemProperty(mListSystemProperties, SYP_REGSTRATION_SOUND_PROP_RESTORED, value))
    {
        mListSystemProperties.push_back(systemProperty);
    }
}

CAmSystemElement::~CAmSystemElement()
{
}

am_Error_e CAmSystemElement::_findSystemProperty(const std::vector<am_SystemProperty_s> &listSystemProperties,
    const uint16_t type, int16_t &value) const
{
    for (auto &itListSystemProperties : listSystemProperties)
    {
        if (itListSystemProperties.type == type)
        {
            value = itListSystemProperties.value;
            return E_OK;
        }
    }

    return E_NOT_POSSIBLE;
}

am_Error_e CAmSystemElement::_register(void)
{
    if (mListSystemProperties.size() > 0)
    {
        mpControlReceive->enterSystemPropertiesListDB(mListSystemProperties);
    }

    int16_t logThresoldLevel = LOG_DEBUG_DEFAULT_VALUE;
    if (E_OK != _findSystemProperty(mListSystemProperties, SYP_GLOBAL_LOG_THRESHOLD, logThresoldLevel))
    {
        LOG_FN_INFO(__FILENAME__, __func__, "Setting default threshold value, failed to get from configuration");
    }

    LOG_FN_CHANGE_LEVEL(static_cast<am_LogLevel_e>(logThresoldLevel));
    return E_OK;
}

am_Error_e CAmSystemElement::_unregister(void)
{
    return E_OK;
}

am_Error_e CAmSystemElement::getSystemProperty(const am_CustomSystemPropertyType_t type,
    int16_t &value) const
{
    am_Error_e                                  result = E_NOT_POSSIBLE;
    std::vector < am_SystemProperty_s >         listSystemProperties;
    std::vector<am_SystemProperty_s >::iterator itListSystemProperties;
    result = mpControlReceive->getListSystemProperties(listSystemProperties);
    if (E_OK == result)
    {
        result = _findSystemProperty(listSystemProperties, type, value);
    }

    return result;
}

am_Error_e CAmSystemElement::setSystemProperties(const std::vector<am_SystemProperty_s> &listSystemProperties)
{
    am_Error_e result = mpControlReceive->changeSystemPropertiesDB(listSystemProperties);

    for (auto &itListSystemProperties : listSystemProperties)
    {
        if (SYP_GLOBAL_LOG_THRESHOLD == itListSystemProperties.type)
        {
            LOG_FN_CHANGE_LEVEL(static_cast<am_LogLevel_e >(itListSystemProperties.value));
        }

        if (E_OK == result)
        {
            if (true == _isPersistenceSupported(itListSystemProperties.type))
            {
                if (false == _updateListLastSystemProperty(itListSystemProperties.type, itListSystemProperties.value))
                {
                    mListLastSystemProperties.push_back(itListSystemProperties);
                }
            }
        }
    }
    return result;
}

am_Error_e CAmSystemElement::setSystemProperty(const am_CustomSystemPropertyType_t type,
    const int16_t value)
{
    am_Error_e          result = E_NOT_POSSIBLE;
    am_SystemProperty_s systemProperty;
    systemProperty.type  = type;
    systemProperty.value = value;
    result               = mpControlReceive->changeSystemPropertyDB(systemProperty);

    if (SYP_GLOBAL_LOG_THRESHOLD == systemProperty.type)
    {
        LOG_FN_CHANGE_LEVEL(static_cast<am_LogLevel_e >(systemProperty.value));
    }

    if (E_OK == result)
    {
        if (true == _isPersistenceSupported(systemProperty.type))
        {
            if (false == _updateListLastSystemProperty(type, value))
            {
                mListLastSystemProperties.push_back(systemProperty);
            }
        }
    }

    return result;
}

bool CAmSystemElement::_updateListLastSystemProperty(const am_CustomSystemPropertyType_t &type,
    const int16_t value)
{
    for (auto &itListLastSystemProperties : mListLastSystemProperties)
    {
        if (itListLastSystemProperties.type == type)
        {
            itListLastSystemProperties.value = value;
            return true;
        }
    }

    return false;
}

bool CAmSystemElement::_isPersistenceSupported(const am_CustomSystemPropertyType_t &type)
{
    for (auto &itSystemProperites : mSystem.listGCSystemProperties)
    {
        if (itSystemProperites.type == type)
        {
            return itSystemProperites.isPersistenceSupported;
        }
    }

    return false;
}

int16_t CAmSystemElement::getDebugLevel(void) const
{
    int16_t value = -1;
    getSystemProperty(SYP_GLOBAL_LOG_THRESHOLD, value);
    return value;
}

bool CAmSystemElement::isUnknownElementRegistrationSupported(void) const
{
    int16_t value = -1;
    getSystemProperty(SYP_REGISTRATION_ALLOW_UNKNOWN_ELEMENT, value);
    return ((value == 0) ? false : true);
}

bool CAmSystemElement::isSystemPropertyReadOnly() const
{
    return mSystem.readOnly;
}

bool CAmSystemElement::isNonTopologyRouteAllowed(void) const
{
    int16_t value;
    getSystemProperty(SYP_CONNECTION_ALLOW_ONLY_TOPOLOGY_ROUTES, value);
    return ((value == 0) ? true : false);
}

std::string CAmSystemElement::getLastSystemPropertiesString()
{
    std::string systemPropertiesString;

    if (mListLastSystemProperties.size() > 0)
    {
        systemPropertiesString += "{";
        for (auto &itListLastSystemProperties : mListLastSystemProperties)
        {
            std::stringstream systemPropertyType;
            systemPropertyType << itListLastSystemProperties.type;
            std::stringstream systemPropertyValue;
            systemPropertyValue << itListLastSystemProperties.value;
            systemPropertiesString += "(" + systemPropertyType.str() + ":"
                + systemPropertyValue.str() + ")";
        }

        systemPropertiesString += "}";
    }

    return systemPropertiesString;
}

} /* namespace gc */
} /* namespace am */
