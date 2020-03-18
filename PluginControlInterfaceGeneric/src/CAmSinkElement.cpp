/******************************************************************************
 * @file: CAmSinkElement.cpp
 *
 * This file contains the definition of sink element class (member functions
 * and data members) used as data container to store the information related to
 * sink as maintained by controller.
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Toshiaki Isogai <tisogai@jp.adit-jv.com>
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>
 *          Prashant Jain   <pjain@jp.adit-jv.com>
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 - 2019 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#include "CAmSinkElement.h"
#include "CAmLogger.h"
#include "CAmClassElement.h"

#include <cmath>

namespace am {
namespace gc {

CAmSinkElement::CAmSinkElement(const gc_Sink_s &sinkData, IAmControlReceive *pControlReceive)
    : CAmRoutePointElement(ET_SINK, sinkData.name, mSink, pControlReceive)
    , mSink(sinkData)
{
    mMuteState       = MS_UNMUTED;

    if (mSink.visible)
    {
        setVolume(convertMainVolumeToVolume(mSink.mainVolume));
    }
    if (mSink.isVolumeChangeSupported)
    {
        if (false == mSink.mapUserVolumeToNormalizedVolume.empty())
        {
            mMinVolume = mSink.mapNormalizedVolumeToDecibelVolume.begin()->second * 10;
            mMaxVolume = mSink.mapNormalizedVolumeToDecibelVolume.rbegin()->second * 10;
            LOG_FN_DEBUG(__FILENAME__, mName, "volume range set to"
                    , mMinVolume, "...", mMaxVolume);
        }
    }
}

CAmSinkElement::~CAmSinkElement()
{
}

am_Error_e CAmSinkElement::getMainSoundPropertyValue(const am_CustomMainSoundPropertyType_t type,
    int16_t &value) const
{
    return mpControlReceive->getMainSinkSoundPropertyValue(getID(), type, value);
}

am_Error_e CAmSinkElement::setMainSoundPropertiesValue(const std::vector<am_MainSoundProperty_s > &listSoundProperties)
{
    am_Error_e result = mpControlReceive->changeMainSinkSoundPropertiesDB(listSoundProperties, getID());

    if (E_OK == result)
    {
        for (auto &itListSoundProperties : listSoundProperties)
        {
            // first check if sound property has the persistence support
            if (true == isMSPPersistenceSupported(itListSoundProperties.type))
            {
                std::shared_ptr<CAmClassElement > pClassElement = CAmClassFactory::getElement(
                        mSink.className);
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

am_Error_e CAmSinkElement::setMainSoundPropertyValue(const am_CustomMainSoundPropertyType_t type,
    const int16_t value)
{
    am_MainSoundProperty_s property;
    property.type  = type;
    property.value = value;

    am_Error_e result = mpControlReceive->changeMainSinkSoundPropertyDB(property, getID());
    if (E_OK == result)
    {
        // first check if main sound property has the persistence support
        if (true == isMSPPersistenceSupported(type))
        {
            std::shared_ptr<CAmClassElement > pClassElement = CAmClassFactory::getElement(
                    mSink.className);
            if (pClassElement != nullptr)
            {
                gc_ElementTypeName_s elementInfo;
                elementInfo.elementName = getName();
                elementInfo.elementType = getType();
                pClassElement->setLastSoundProperty(elementInfo, property);
            }
        }
    }

    return result;
}

am_Error_e CAmSinkElement::getSoundPropertyValue(const am_CustomSoundPropertyType_t type,
    int16_t &value) const
{
    return mpControlReceive->getSinkSoundPropertyValue(getID(), type, value);
}

am_Error_e CAmSinkElement::setAvailability(const am_Availability_s &availability)
{
    return mpControlReceive->changeSinkAvailabilityDB(availability, getID());
}

am_Error_e CAmSinkElement::getAvailability(am_Availability_s &availability) const
{
    am_Sink_s  sinkData;
    am_Error_e result;
    result       = mpControlReceive->getSinkInfoDB(getID(), sinkData);
    availability = sinkData.available;
    return result;
}

am_Error_e CAmSinkElement::_register(void)
{
    am_sinkID_t sinkID(0);
    am_Error_e  result = E_DATABASE_ERROR;
    if (E_OK == mpControlReceive->enterSinkDB(mSink, sinkID))
    {
        setID(sinkID);
        result = E_OK;
    }

    return result;
}

am_Error_e CAmSinkElement::_unregister(void)
{
    am_Error_e result = E_DATABASE_ERROR;
    if (E_OK == mpControlReceive->removeSinkDB(getID()))
    {
        setID(0);
        result = E_OK;
    }

    return result;
}

am_domainID_t CAmSinkElement::getDomainID(void) const
{
    return mSink.domainID;
}

am_Error_e CAmSinkElement::getListConnectionFormats(
    std::vector<am_CustomConnectionFormat_t > &listConnectionFormats)
{
    listConnectionFormats = mSink.listConnectionFormats;
    return E_OK;
}

am_Error_e CAmSinkElement::upadateDB(
    am_sinkClass_t classId, std::vector<am_SoundProperty_s > listSoundProperties,
    std::vector<am_CustomConnectionFormat_t > listConnectionFormats,
    std::vector<am_MainSoundProperty_s > listMainSoundProperties)
{
    am_Error_e                                  result = E_OK;
    am_Sink_s                                   sinkData;
    std::vector < am_SoundProperty_s >          listUpdatedSoundProperties;
    std::vector < am_MainSoundProperty_s >      listUpdatedMainSoundProperties;
    std::vector < am_CustomConnectionFormat_t > listUpdatedConnectionFormats;

    std::vector<am_SoundProperty_s >::iterator          itListSoundProperties;
    std::vector<am_MainSoundProperty_s >::iterator      itListMainSoundProperties;
    std::vector<am_CustomConnectionFormat_t >::iterator itListConnectionFormats;

    std::vector<am_SoundProperty_s >::iterator          itListUpdatedSoundProperties;
    std::vector<am_MainSoundProperty_s >::iterator      itListUpdatedMainSoundProperties;
    std::vector<am_CustomConnectionFormat_t >::iterator itListUpdatedConnectionFormats;

    mpControlReceive->getSinkInfoDB(getID(), sinkData);
    /*
     * Initialize the list with the already present sound properties
     */
    listUpdatedConnectionFormats   = sinkData.listConnectionFormats;
    listUpdatedSoundProperties     = sinkData.listSoundProperties;
    listUpdatedMainSoundProperties = sinkData.listMainSoundProperties;

    /*
     * sound properties update. The strategy is as follows
     * 1. Get the list of sound properties from the audio manager database
     * 2. for each property present in the list from routing side update in the list returned from
     * AudioManager Daemon.
     * 3. If the sound property is not present in the configuration ignore it
     */
    for (itListSoundProperties = listSoundProperties.begin();
         itListSoundProperties != listSoundProperties.end(); itListSoundProperties++)
    {
        am_SoundProperty_s     soundProperty = *itListSoundProperties;
        am_MainSoundProperty_s mainSoundProperty;
        if (_isSoundPropertyConfigured(soundProperty.type, mSink.listGCSoundProperties))
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

            LOG_FN_INFO(__FILENAME__, __func__, "converting SP TO MSP", soundProperty.type);
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
     * 2. Perform the SP to MSP conversion and update the MSP type:values.
     * 3. For each main sound property present in the new list update the MSP value
     * 4. If the new list has a main sound property type which is not present in the
     * configuration list then ignore it.
     */
    for (itListMainSoundProperties = listMainSoundProperties.begin();
         itListMainSoundProperties != listMainSoundProperties.end();
         itListMainSoundProperties++)
    {
        am_MainSoundProperty_s mainSoundProperty = *itListMainSoundProperties;
        if (_isSoundPropertyConfigured(mainSoundProperty.type, mSink.listGCMainSoundProperties))
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

    return mpControlReceive->changeSinkDB(getID(), classId, listUpdatedSoundProperties,
        listUpdatedConnectionFormats,
        listUpdatedMainSoundProperties);
}

am_Error_e CAmSinkElement::setMainNotificationConfiguration(
    const am_NotificationConfiguration_s &mainNotificationConfiguraton)
{
    return mpControlReceive->changeMainSinkNotificationConfigurationDB(getID(),
        mainNotificationConfiguraton);
}

am_Error_e CAmSinkElement::notificationDataUpdate(const am_NotificationPayload_s &payload)
{
    mpControlReceive->sendMainSinkNotificationPayload(getID(), payload);
    return E_OK;
}

am_Error_e CAmSinkElement::getListMainNotificationConfigurations(
    std::vector<am_NotificationConfiguration_s > &listMainNotificationConfigurations)
{
    am_Sink_s  sinkData;
    am_Error_e result;
    // get the source Info from Database
    result                             = mpControlReceive->getSinkInfoDB(getID(), sinkData);
    listMainNotificationConfigurations = sinkData.listMainNotificationConfigurations;
    return result;
}

am_Error_e CAmSinkElement::getListNotificationConfigurations(
    std::vector<am_NotificationConfiguration_s > &listNotificationConfigurations)
{
    am_Sink_s  sinkData;
    am_Error_e result;
    // get the source Info from Database
    result                         = mpControlReceive->getSinkInfoDB(getID(), sinkData);
    listNotificationConfigurations = sinkData.listNotificationConfigurations;
    return result;
}

am_Error_e CAmSinkElement::getNotificationConfigurations(
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

am_Error_e CAmSinkElement::getMainNotificationConfigurations(
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

am_sinkClass_t CAmSinkElement::getClassID(void) const
{
    am_Sink_s sink;
    sink.sinkClassID = 0;
    mpControlReceive->getSinkInfoDB(getID(), sink);
    return sink.sinkClassID;
}

void CAmSinkElement::setMainVolume(const am_mainVolume_t mainVolume)
{
    am_mainVolume_t oldMainVolume = getMainVolume();
    if (mainVolume != oldMainVolume)
    {
        am_Error_e result = mpControlReceive->changeSinkMainVolumeDB(mainVolume, getID());
        if ((result == E_OK) || (result == E_NO_CHANGE))
        {
            mSink.mainVolume = mainVolume;
        }
        else
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "Failed to update sink volume in DB");
        }
    }
}

am_mainVolume_t CAmSinkElement::getMainVolume() const
{

    return mSink.mainVolume;
}

am_volume_t CAmSinkElement::convertMainVolumeToVolume(const am_mainVolume_t mainVolume)
{
    if (mSink.mapUserVolumeToNormalizedVolume.empty() || mSink.mapNormalizedVolumeToDecibelVolume.empty())
    {
        LOG_FN_WARN(__FILENAME__, __func__, "unable to convert", mainVolume, "to user volume due to empty mapping in sink", mName);

        return mainVolume;
    }

    // First convert from User Volume to Normalized Volume
    float normalisedVolume = _lookupForward((float)mainVolume, mSink.mapUserVolumeToNormalizedVolume);
    // Convert Normalized to Decibel
    float decibelVolume = _lookupForward(normalisedVolume, mSink.mapNormalizedVolumeToDecibelVolume);
    // convert to 1/10 deciBel integer
    am_volume_t DBVolume = (am_volume_t)lroundf(decibelVolume * 10.);

    LOG_FN_DEBUG(__FILENAME__, __func__, "successfully converted user volume", mainVolume
            , "to", DBVolume, "decibel in sink", mName);

    return DBVolume;
}

am_mainVolume_t CAmSinkElement::convertVolumeToMainVolume(const am_volume_t DBVolume)
{
    if (mSink.mapNormalizedVolumeToDecibelVolume.empty() || mSink.mapUserVolumeToNormalizedVolume.empty())
    {
        LOG_FN_WARN(__FILENAME__, __func__, "unable to convert", DBVolume, "decibel due to empty mapping in sink", mName);
        return DBVolume;
    }

    // Convert 1/10 deciBel to normalized
    float normalisedVolume = _lookupReverse((float)DBVolume / 10.0, mSink.mapNormalizedVolumeToDecibelVolume);
    // Convert Normalized to User volume
    float mainVolume = _lookupReverse(normalisedVolume, mSink.mapUserVolumeToNormalizedVolume);
    // convert to integer
    am_mainVolume_t mainVolumeInt = (am_mainVolume_t)lroundf(mainVolume);

    LOG_FN_DEBUG(__FILENAME__, __func__, "successfully converted", DBVolume
            , "decibel to user volume", mainVolumeInt, "for sink", mName);

    return mainVolumeInt;
}

gc_LimitVolume_s CAmSinkElement::getSinkLimit()
{
    return mSinkLimit;
}

void CAmSinkElement::setSinkLimit(gc_LimitVolume_s limit)
{
    mSinkLimit = limit;
}

std::shared_ptr<CAmElement > CAmSinkElement::getElement()
{
    return CAmSinkFactory::getElement(getName());
}

} /* namespace gc */
} /* namespace am */
