/******************************************************************************
 * @file: CAmSourceElement.cpp
 *
 * This file contains the definition of source element class (member functions
 * and data members) used as data container to store the information related to
 * source as maintained by controller.
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Toshiaki Isogai <tisogai@jp.adit-jv.com>
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>
 *          Prashant Jain   <pjain@jp.adit-jv.com>
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#include "CAmSourceElement.h"
#include "CAmLogger.h"
#include "CAmClassElement.h"

namespace am {
namespace gc {

#define DEFAULT_USER_VOLUME (100)
CAmSourceElement::CAmSourceElement(const gc_Source_s &sourceData, IAmControlReceive *pControlReceive)
    : CAmRoutePointElement(ET_SOURCE, sourceData.name, mSource, pControlReceive)
    , mSource(sourceData)
{
    mMuteState = MS_UNMUTED;
    if ( true == mSource.isVolumeChangeSupported )
    {
        mVolume    = mSource.volume;
        mMinVolume = mSource.minVolume;
        mMaxVolume = mSource.maxVolume;
        LOG_FN_DEBUG(__FILENAME__, mName, "volume initialized as", mVolume, "with range"
                , mMinVolume, "...", mMaxVolume);
    }
}

CAmSourceElement::~CAmSourceElement()
{
}

am_SourceState_e CAmSourceElement::getState() const
{
    am_Source_s sourceData;
    // get the source Info from Database
    am_Error_e result = mpControlReceive->getSourceInfoDB(getID(), sourceData);
    if ((result == E_OK) || (result == E_NO_CHANGE))
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "succeeded with", result, ", returning"
            , sourceData.sourceState, "for", getName());
        return sourceData.sourceState;
    }
    else
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "FAILED with", result, "getting state for", getName());
        return SS_UNKNNOWN;
    }
}

am_InterruptState_e CAmSourceElement::getInterruptState() const
{
    return mSource.interruptState;
}

am_Error_e CAmSourceElement::setInterruptState(const am_InterruptState_e interruptState)
{
    mSource.interruptState = interruptState;
    return E_OK;
}

am_Error_e CAmSourceElement::getMainSoundPropertyValue(const am_CustomMainSoundPropertyType_t type,
    int16_t &value) const
{
    return mpControlReceive->getMainSourceSoundPropertyValue(getID(), type, value);
}

am_Error_e CAmSourceElement::setMainSoundPropertiesValue(const std::vector<am_MainSoundProperty_s > &listSoundProperties)
{
    am_Error_e result = mpControlReceive->changeMainSourceSoundPropertiesDB(listSoundProperties, getID());
    if (E_OK == result)
    {
        for (auto &itListSoundProperties : listSoundProperties)
        {
            // first check if sound property has the persistence support
            if (true == isMSPPersistenceSupported(itListSoundProperties.type))
            {
                std::shared_ptr<CAmClassElement > pClassElement = CAmClassFactory::getElement(
                        mSource.className);
                if (pClassElement != nullptr)
                {
                    gc_ElementTypeName_s elementInfo;
                    elementInfo.elementName = getName();
                    elementInfo.elementType = getType();
                    pClassElement->setLastSoundProperty(elementInfo, itListSoundProperties);
                }
            }
        }
    }

    return result;
}

am_Error_e CAmSourceElement::setMainSoundPropertyValue(const am_CustomMainSoundPropertyType_t type,
    const int16_t value)
{
    am_MainSoundProperty_s mainSoundProperty;
    mainSoundProperty.type  = type;
    mainSoundProperty.value = value;

    am_Error_e result = mpControlReceive->changeMainSourceSoundPropertyDB(mainSoundProperty,
            getID());
    if (E_OK == result)
    {
        // first check if sound property has the persistence support
        if (true == isMSPPersistenceSupported(type))
        {
            std::shared_ptr<CAmClassElement > pClassElement = CAmClassFactory::getElement(
                    mSource.className);
            if (pClassElement != nullptr)
            {
                gc_ElementTypeName_s elementInfo;
                elementInfo.elementName = getName();
                elementInfo.elementType = getType();
                pClassElement->setLastSoundProperty(elementInfo, mainSoundProperty);
            }
        }
    }

    return result;
}

am_Error_e CAmSourceElement::getSoundPropertyValue(const am_CustomSoundPropertyType_t type,
    int16_t &value) const
{
    return mpControlReceive->getSourceSoundPropertyValue(getID(), type, value);
}

am_Error_e CAmSourceElement::setAvailability(const am_Availability_s &availability)
{
    return mpControlReceive->changeSourceAvailabilityDB(availability, getID());
}

am_Error_e CAmSourceElement::getAvailability(am_Availability_s &availability) const
{
    am_Source_s sourceData;
    am_Error_e  result;
    // get the source Info from Database
    result                          = mpControlReceive->getSourceInfoDB(getID(), sourceData);
    availability.availability       = sourceData.available.availability;
    availability.availabilityReason = sourceData.available.availabilityReason;
    return result;
}

am_Error_e CAmSourceElement::_register(void)
{
    am_sourceID_t sourceID;
    am_Error_e    result = E_DATABASE_ERROR;
    if (E_OK == mpControlReceive->enterSourceDB(mSource, sourceID))
    {
        setID(sourceID);
        result = E_OK;
    }

    return result;
}

am_Error_e CAmSourceElement::_unregister(void)
{
    am_Error_e result = E_DATABASE_ERROR;
    if (E_OK == mpControlReceive->removeSourceDB(getID()))
    {
        setID(0);
        result = E_OK;
    }

    return result;
}

am_domainID_t CAmSourceElement::getDomainID(void) const
{
    return mSource.domainID;
}

am_Error_e CAmSourceElement::getListConnectionFormats(
    std::vector<am_CustomConnectionFormat_t > &listConnectionFormats)
{
    listConnectionFormats = mSource.listConnectionFormats;
    return E_OK;
}

am_Error_e CAmSourceElement::upadateDB(
    am_sourceClass_t classId, std::vector<am_SoundProperty_s > listSoundProperties,
    std::vector<am_CustomConnectionFormat_t > listConnectionFormats,
    std::vector<am_MainSoundProperty_s > listMainSoundProperties)
{
    am_Error_e                                  result = E_OK;
    am_Source_s                                 sourceData;
    std::vector < am_SoundProperty_s >          listUpdatedSoundProperties;
    std::vector < am_MainSoundProperty_s >      listUpdatedMainSoundProperties;
    std::vector < am_CustomConnectionFormat_t > listUpdatedConnectionFormats;

    std::vector<am_SoundProperty_s >::iterator          itListSoundProperties;
    std::vector<am_MainSoundProperty_s >::iterator      itListMainSoundProperties;
    std::vector<am_CustomConnectionFormat_t >::iterator itListConnectionFormats;

    std::vector<am_SoundProperty_s >::iterator          itListUpdatedSoundProperties;
    std::vector<am_MainSoundProperty_s >::iterator      itListUpdatedMainSoundProperties;
    std::vector<am_CustomConnectionFormat_t >::iterator itListUpdatedConnectionFormats;

    mpControlReceive->getSourceInfoDB(getID(), sourceData);
    /*
     * Initialize the list with the already present sound properties
     */
    listUpdatedConnectionFormats   = sourceData.listConnectionFormats;
    listUpdatedSoundProperties     = sourceData.listSoundProperties;
    listUpdatedMainSoundProperties = sourceData.listMainSoundProperties;

    /*
     * sound properties update. The strategy is as follows
     * 1. Get the sound properties from the audio manager database
     * 2. for each property present in the list from routing side update in the list.
     */
    for (itListSoundProperties = listSoundProperties.begin();
         itListSoundProperties != listSoundProperties.end(); itListSoundProperties++)
    {
        am_SoundProperty_s     soundProperty = *itListSoundProperties;
        am_MainSoundProperty_s mainSoundProperty;
        if (_isSoundPropertyConfigured(soundProperty.type, mSource.listGCSoundProperties))
        {
            for (itListUpdatedSoundProperties = listUpdatedSoundProperties.begin();
                 itListUpdatedSoundProperties != listUpdatedSoundProperties.end();
                 itListUpdatedSoundProperties++)
            {
                if (itListUpdatedSoundProperties->type == soundProperty.type)
                {
                    itListSoundProperties->value = soundProperty.value;
                    break;
                }
            }

            if (itListUpdatedSoundProperties == listUpdatedSoundProperties.end())
            {
                listUpdatedSoundProperties.push_back(soundProperty);
            }

            LOG_FN_INFO(__FILENAME__, __func__, "converting SP TO MSP type:value=", soundProperty.type,
                soundProperty.value);
            if (E_OK != soundPropertyToMainSoundProperty(soundProperty, mainSoundProperty))
            {
                continue;
            }

            for (itListUpdatedMainSoundProperties = listUpdatedMainSoundProperties.begin();
                 itListUpdatedMainSoundProperties != listUpdatedMainSoundProperties.end();
                 itListUpdatedMainSoundProperties++)
            {
                if (itListUpdatedMainSoundProperties->type == mainSoundProperty.type)
                {
                    itListUpdatedMainSoundProperties->value = mainSoundProperty.value;
                    break;
                }
            }

            if (itListUpdatedMainSoundProperties == listUpdatedMainSoundProperties.end())
            {
                listUpdatedMainSoundProperties.push_back(mainSoundProperty);
            }
        }
    }

    /*
     * main sound properties update. The strategy is as follows
     * 1. Get the main sound properties from the audio manager database
     * 2. perform the SP to MSP conversion and update the MSP type:values.
     * 3. For each main sound property present in the new list update the MSP
     */
    for (itListMainSoundProperties = listMainSoundProperties.begin();
         itListMainSoundProperties != listMainSoundProperties.end();
         itListMainSoundProperties++)
    {
        am_MainSoundProperty_s mainSoundProperty = *itListMainSoundProperties;
        if (_isSoundPropertyConfigured(mainSoundProperty.type, mSource.listGCMainSoundProperties))
        {
            for (itListUpdatedMainSoundProperties = listUpdatedMainSoundProperties.begin();
                 itListUpdatedMainSoundProperties != listUpdatedMainSoundProperties.end();
                 itListUpdatedMainSoundProperties++)
            {
                if (itListUpdatedMainSoundProperties->type == mainSoundProperty.type)
                {
                    itListUpdatedMainSoundProperties->value = mainSoundProperty.value;
                    break;
                }
            }

            if (itListUpdatedMainSoundProperties == listUpdatedMainSoundProperties.end())
            {
                listUpdatedMainSoundProperties.push_back(mainSoundProperty);
            }
        }
    }

    /*
     * upadte connection format list
     */
    for (itListConnectionFormats = listConnectionFormats.begin();
         itListConnectionFormats != listConnectionFormats.end();
         ++itListConnectionFormats)
    {
        for (itListUpdatedConnectionFormats = listUpdatedConnectionFormats.begin();
             itListUpdatedConnectionFormats != listUpdatedConnectionFormats.end();
             itListUpdatedConnectionFormats++)
        {
            if (*itListConnectionFormats == *itListUpdatedConnectionFormats)
            {
                break;
            }
        }

        if (itListUpdatedConnectionFormats == listUpdatedConnectionFormats.end())
        {
            listUpdatedConnectionFormats.push_back(*itListConnectionFormats);
        }
    }

    return mpControlReceive->changeSourceDB(getID(), classId, listUpdatedSoundProperties,
        listUpdatedConnectionFormats,
        listUpdatedMainSoundProperties);
}

am_Error_e CAmSourceElement::setMainNotificationConfiguration(
    const am_NotificationConfiguration_s &mainNotificationConfiguraton)
{
    return mpControlReceive->changeMainSourceNotificationConfigurationDB(
        getID(), mainNotificationConfiguraton);
}

am_Error_e CAmSourceElement::notificationDataUpdate(const am_NotificationPayload_s &payload)
{
    mpControlReceive->sendMainSourceNotificationPayload(getID(), payload);
    return E_OK;
}

am_Error_e CAmSourceElement::getListMainNotificationConfigurations(
    std::vector<am_NotificationConfiguration_s > &listMainNotificationConfigurations)
{
    am_Source_s sourceData;
    am_Error_e  result;
    // get the source Info from Database
    result                             = mpControlReceive->getSourceInfoDB(getID(), sourceData);
    listMainNotificationConfigurations = sourceData.listMainNotificationConfigurations;
    return result;
}

am_Error_e CAmSourceElement::getListNotificationConfigurations(
    std::vector<am_NotificationConfiguration_s > &listNotificationConfigurations)
{
    am_Source_s sourceData;
    am_Error_e  result;
    // get the source Info from Database
    result                         = mpControlReceive->getSourceInfoDB(getID(), sourceData);
    listNotificationConfigurations = sourceData.listNotificationConfigurations;
    return result;
}

am_Error_e CAmSourceElement::getNotificationConfigurations(
    am_CustomNotificationType_t type,
    am_NotificationConfiguration_s &notificationConfiguration)
{
    std::vector < am_NotificationConfiguration_s >         listNotificationConfigurations;
    std::vector<am_NotificationConfiguration_s >::iterator itListNotificationConfigurations;
    am_Error_e                                             result = getListNotificationConfigurations(listNotificationConfigurations);
    if (result == E_OK)
    {
        result = E_UNKNOWN;
        for (itListNotificationConfigurations = listNotificationConfigurations.begin();
             itListNotificationConfigurations != listNotificationConfigurations.end();
             ++itListNotificationConfigurations)
        {
            if (itListNotificationConfigurations->type == type)
            {
                notificationConfiguration = *itListNotificationConfigurations;
                result                    = E_OK;
                break;
            }
        }
    }

    return result;
}

am_Error_e CAmSourceElement::getMainNotificationConfigurations(
    am_CustomNotificationType_t type,
    am_NotificationConfiguration_s &mainNotificationConfiguration)
{
    std::vector < am_NotificationConfiguration_s >         listMainNotificationConfigurations;
    std::vector<am_NotificationConfiguration_s >::iterator itListMainNotificationConfigurations;
    am_Error_e                                             result = getListMainNotificationConfigurations(listMainNotificationConfigurations);
    if (result == E_OK)
    {
        result = E_UNKNOWN;
        for (itListMainNotificationConfigurations = listMainNotificationConfigurations.begin();
             itListMainNotificationConfigurations != listMainNotificationConfigurations.end();
             ++itListMainNotificationConfigurations)
        {
            if (itListMainNotificationConfigurations->type == type)
            {
                mainNotificationConfiguration = *itListMainNotificationConfigurations;
                result                        = E_OK;
                break;
            }
        }
    }

    return result;
}

am_sourceClass_t CAmSourceElement::getClassID(void) const
{
    am_Source_s source;
    source.sourceClassID = 0;
    mpControlReceive->getSourceInfoDB(getID(), source);
    return source.sourceClassID;
}

std::shared_ptr<CAmElement > CAmSourceElement::getElement()
{
    return CAmSourceFactory::getElement(getName());
}


} /* namespace gc */
} /* namespace am */
