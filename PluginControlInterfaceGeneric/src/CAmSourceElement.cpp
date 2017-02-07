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
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#include "CAmSourceElement.h"
#include "CAmControlReceive.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

#define DEFAULT_USER_VOLUME   (100)
CAmSourceElement::CAmSourceElement(const gc_Source_s& sourceData,
                                   CAmControlReceive* pControlReceive) :
                                CAmElement(sourceData.name, pControlReceive),
                                mpControlReceive(pControlReceive),
                                mSource(sourceData),
                                mNumInUse(0)
{
    setType (ET_SOURCE);
    setVolume(mSource.volume);
    setPriority(mSource.priority);
    setInterruptState(sourceData.interruptState);
}

CAmSourceElement::~CAmSourceElement()
{
    //nothing to destroy
}

void CAmSourceElement::setInUse(const bool inUse)
{
    // request is for setting in USE the element
    if (true == inUse)
    {
        mNumInUse++;
    }
    // request is for un-setting in use the element
    else
    {
        if (0 == mNumInUse)
        {
            LOG_FN_ERROR("  OUT: already no instance of element in use");
            return;
        }
        mNumInUse--;
    }
    LOG_FN_EXIT (mNumInUse);
}

uint16_t CAmSourceElement::getInUse(void) const
{
    return mNumInUse;
}

am_Error_e CAmSourceElement::getState(int& state) const
{
    am_Source_s sourceData;
    am_Error_e result;
    //get the source Info from Database
    result = mpControlReceive->getSourceInfoDB(getID(), sourceData);
    state = sourceData.sourceState;
    return result;
}

am_Error_e CAmSourceElement::getMainSoundPropertyValue(const am_CustomMainSoundPropertyType_t type,
                                                       int16_t& value) const
{
    return mpControlReceive->getMainSourceSoundPropertyValue(getID(), type, value);
}

am_Error_e CAmSourceElement::setMainSoundPropertyValue(const am_CustomMainSoundPropertyType_t type,
                                                       const int16_t value)
{
    am_MainSoundProperty_s mainSourceProperty;
    mainSourceProperty.type = type;
    mainSourceProperty.value = value;
    return mpControlReceive->changeMainSourceSoundPropertyDB(mainSourceProperty, getID());
}

template <typename TPropertyType, typename Tlisttype>
am_Error_e CAmSourceElement::_saturateSoundProperty(
                const TPropertyType soundPropertyType,
                const std::vector<Tlisttype >& listGCSoundProperties, int16_t& soundPropertyValue)
{
    am_Error_e result = E_UNKNOWN;
    typename std::vector<Tlisttype >::const_iterator itListSoundProperties;
    // if property list is empty return error
    if (true == listGCSoundProperties.empty())
    {
        LOG_FN_ERROR(" List of sound property is empty");
    }
    else
    {
        for (itListSoundProperties = listGCSoundProperties.begin();
                        itListSoundProperties != listGCSoundProperties.end();
                        ++itListSoundProperties)
        {
            if ((*itListSoundProperties).type == soundPropertyType)
            {
                // sound property value is greater than maximum value supported
                if (soundPropertyValue > (*itListSoundProperties).maxValue)
                {
                    soundPropertyValue = (*itListSoundProperties).maxValue;
                }
                else if (soundPropertyValue < (*itListSoundProperties).minValue) // sound property value is less than minimum value supported
                {
                    soundPropertyValue = (*itListSoundProperties).minValue;
                }
                LOG_FN_EXIT(" Value:", soundPropertyValue);
                result = E_OK;
                break;
            }
        }
    }
    return result;
}

am_Error_e CAmSourceElement::saturateMainSoundPropertyRange(
                const am_CustomMainSoundPropertyType_t mainSoundPropertyType,
                int16_t& soundPropertyValue)
{
    std::vector<gc_MainSoundProperty_s >::iterator itListSoundProperties;
    LOG_FN_ENTRY("type:value", mainSoundPropertyType, soundPropertyValue);
    return _saturateSoundProperty<am_CustomMainSoundPropertyType_t, gc_MainSoundProperty_s >(
                    mainSoundPropertyType, mSource.listGCMainSoundProperties, soundPropertyValue);
}

am_Error_e CAmSourceElement::saturateSoundPropertyRange(
                const am_CustomSoundPropertyType_t soundPropertyType, int16_t& soundPropertyValue)
{
    std::vector<gc_MainSoundProperty_s >::iterator itListSoundProperties;
    LOG_FN_ENTRY("type:value", soundPropertyType, soundPropertyValue);
    return _saturateSoundProperty<am_CustomSoundPropertyType_t, gc_SoundProperty_s >(
                    soundPropertyType, mSource.listGCSoundProperties, soundPropertyValue);
}

template <typename TPropertyType, typename Tlisttype>
bool CAmSourceElement::_isSoundPropertyConfigured(
                const TPropertyType soundPropertyType,
                const std::vector<Tlisttype >& listGCSoundProperties)
{
    bool soundPropertyConfigured = false;
    typename std::vector<Tlisttype >::const_iterator itListSoundProperties;
    for (itListSoundProperties = listGCSoundProperties.begin();
                    itListSoundProperties != listGCSoundProperties.end(); ++itListSoundProperties)
    {
        if ((*itListSoundProperties).type == soundPropertyType)
        {
            soundPropertyConfigured = true;
            break;
        }
    }
    return soundPropertyConfigured;
}

am_Error_e CAmSourceElement::getSoundPropertyValue(const am_CustomSoundPropertyType_t type,
                                                   int16_t& value) const
{
    return mpControlReceive->getSourceSoundPropertyValue(getID(), type, value);
}

am_Error_e CAmSourceElement::setAvailability(const am_Availability_s& availability)
{
    return mpControlReceive->changeSourceAvailabilityDB(availability, getID());
}

am_Error_e CAmSourceElement::getAvailability(am_Availability_s& availability) const
{
    am_Source_s sourceData;
    am_Error_e result;
    //get the source Info from Database
    result = mpControlReceive->getSourceInfoDB(getID(), sourceData);
    availability.availability = sourceData.available.availability;
    availability.availabilityReason = sourceData.available.availabilityReason;
    return result;
}

am_Error_e CAmSourceElement::_register(void)
{
    am_sourceID_t sourceID;
    am_Error_e result = E_DATABASE_ERROR;
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

am_domainID_t CAmSourceElement::getDomainID(void)
{
    return mSource.domainID;
}

am_Error_e CAmSourceElement::getListConnectionFormats(
                std::vector<am_CustomConnectionFormat_t >& listConnectionFormats)
{
    listConnectionFormats = mSource.listConnectionFormats;
    return E_OK;
}

am_Error_e CAmSourceElement::mainSoundPropertyToSoundProperty(
                const am_MainSoundProperty_s &mainSoundProperty, am_SoundProperty_s& soundProperty)
{
    am_Error_e error = E_DATABASE_ERROR;
    if (mSource.mapMSPTOSP[MD_MSP_TO_SP].find(mainSoundProperty.type) != mSource.mapMSPTOSP[MD_MSP_TO_SP].end())
    {
        soundProperty.type = mSource.mapMSPTOSP[MD_MSP_TO_SP][mainSoundProperty.type];
        soundProperty.value = mainSoundProperty.value;
        error = E_OK;
    }
    return error;
}

am_Error_e CAmSourceElement::soundPropertyToMainSoundProperty(
                const am_SoundProperty_s &soundProperty, am_MainSoundProperty_s& mainSoundProperty)
{
    am_Error_e error = E_DATABASE_ERROR;
    if (mSource.mapMSPTOSP[MD_SP_TO_MSP].find(soundProperty.type) != mSource.mapMSPTOSP[MD_SP_TO_MSP].end())
    {
        mainSoundProperty.type = mSource.mapMSPTOSP[MD_SP_TO_MSP][soundProperty.type];
        mainSoundProperty.value = soundProperty.value;
        error = E_OK;
    }
    return error;
}

am_Error_e CAmSourceElement::upadateDB(
                am_sourceClass_t classId, std::vector<am_SoundProperty_s > listSoundProperties,
                std::vector<am_CustomConnectionFormat_t > listConnectionFormats,
                std::vector<am_MainSoundProperty_s > listMainSoundProperties)
{
    am_Error_e result = E_OK;
    am_Source_s sourceData;
    std::vector < am_SoundProperty_s > listUpdatedSoundProperties;
    std::vector < am_MainSoundProperty_s > listUpdatedMainSoundProperties;
    std::vector < am_CustomConnectionFormat_t > listUpdatedConnectionFormats;

    std::vector<am_SoundProperty_s >::iterator itListSoundProperties;
    std::vector<am_MainSoundProperty_s >::iterator itListMainSoundProperties;
    std::vector<am_CustomConnectionFormat_t >::iterator itListConnectionFormats;

    std::vector<am_SoundProperty_s >::iterator itListUpdatedSoundProperties;
    std::vector<am_MainSoundProperty_s >::iterator itListUpdatedMainSoundProperties;
    std::vector<am_CustomConnectionFormat_t >::iterator itListUpdatedConnectionFormats;

    mpControlReceive->getSourceInfoDB(getID(), sourceData);
    /*
     * Initialize the list with the already present sound properties
     */
    listUpdatedConnectionFormats = sourceData.listConnectionFormats;
    listUpdatedSoundProperties = sourceData.listSoundProperties;
    listUpdatedMainSoundProperties = sourceData.listMainSoundProperties;

    /*
     * sound properties update. The strategy is as follows
     * 1. Get the sound properties from the audio manager database
     * 2. for each property present in the list from routing side update in the list.
     */
    for (itListSoundProperties = listSoundProperties.begin();
                    itListSoundProperties != listSoundProperties.end(); itListSoundProperties++)
    {
        am_SoundProperty_s soundProperty = *itListSoundProperties;
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
            LOG_FN_INFO("converting SP TO MSP type:value=", soundProperty.type,
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

bool CAmSourceElement::isVolumeChangeSupported() const
{
    return mSource.isVolumeChangeSupported;
}

am_Error_e CAmSourceElement::setMainNotificationConfiguration(
                const am_NotificationConfiguration_s& mainNotificationConfiguraton)
{
    return mpControlReceive->changeMainSourceNotificationConfigurationDB(
                    getID(), mainNotificationConfiguraton);
}

am_Error_e CAmSourceElement::notificationDataUpdate(const am_NotificationPayload_s& payload)
{
    mpControlReceive->sendMainSourceNotificationPayload(getID(), payload);
    return E_OK;
}

am_Error_e CAmSourceElement::getListMainNotificationConfigurations(
                std::vector<am_NotificationConfiguration_s >& listMainNotificationConfigurations)
{
    am_Source_s sourceData;
    am_Error_e result;
    //get the source Info from Database
    result = mpControlReceive->getSourceInfoDB(getID(), sourceData);
    listMainNotificationConfigurations = sourceData.listMainNotificationConfigurations;
    return result;
}

am_Error_e CAmSourceElement::getListNotificationConfigurations(
                std::vector<am_NotificationConfiguration_s >& listNotificationConfigurations)
{
    am_Source_s sourceData;
    am_Error_e result;
    //get the source Info from Database
    result = mpControlReceive->getSourceInfoDB(getID(), sourceData);
    listNotificationConfigurations = sourceData.listNotificationConfigurations;
    return result;
}

am_Error_e CAmSourceElement::getNotificationConfigurations(
                am_CustomNotificationType_t type,
                am_NotificationConfiguration_s& notificationConfiguration)
{
    std::vector < am_NotificationConfiguration_s > listNotificationConfigurations;
    std::vector<am_NotificationConfiguration_s >::iterator itListNotificationConfigurations;
    am_Error_e result = getListNotificationConfigurations(listNotificationConfigurations);
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
                result = E_OK;
                break;
            }
        }
    }
    return result;
}

am_Error_e CAmSourceElement::getMainNotificationConfigurations(
                am_CustomNotificationType_t type,
                am_NotificationConfiguration_s& mainNotificationConfiguration)
{
    std::vector < am_NotificationConfiguration_s > listMainNotificationConfigurations;
    std::vector<am_NotificationConfiguration_s >::iterator itListMainNotificationConfigurations;
    am_Error_e result = getListMainNotificationConfigurations(listMainNotificationConfigurations);
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
                result = E_OK;
                break;
            }
        }
    }
    return result;
}

} /* namespace gc */
} /* namespace am */
