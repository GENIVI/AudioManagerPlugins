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
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#include "CAmSinkElement.h"
#include "CAmControlReceive.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmSinkElement::CAmSinkElement(const gc_Sink_s& sinkData, CAmControlReceive* pControlReceive) :
                                CAmElement(sinkData.name, pControlReceive),
                                mpControlReceive(pControlReceive),
                                mSink(sinkData),
                                mMapUserToNormalizedVolume(
                                                sinkData.mapUserVolumeToNormalizedVolume),
                                mMapNormalizedToDecibelVolume(
                                                sinkData.mapNormalizedVolumeToDecibelVolume),
                                mNumInUse(0)
{
    setType (ET_SINK);
    setVolume(convertMainVolumeToVolume(mSink.mainVolume));
    setPriority(mSink.priority);
}

CAmSinkElement::~CAmSinkElement()
{
}

void CAmSinkElement::setInUse(const bool inUse)
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
}

uint16_t CAmSinkElement::getInUse(void) const
{
    return mNumInUse;
}

am_Error_e CAmSinkElement::getMuteState(am_MuteState_e& muteState) const
{
    am_Sink_s sinkData;
    am_Error_e result;
    //get the sink Info from Database
    result = mpControlReceive->getSinkInfoDB(getID(), sinkData);
    muteState = sinkData.muteState;
    return result;
}

am_Error_e CAmSinkElement::setMuteState(const am_MuteState_e muteState)
{
    //update the mute state in database
    am_MuteState_e oldMuteState;
    getMuteState(oldMuteState);
    LOG_FN_ENTRY("OLD MS=", oldMuteState, "MS=", muteState);
    if (oldMuteState != muteState)
    {
        return mpControlReceive->changeSinkMuteStateDB(muteState, getID());
    }
    return E_OK;
}

am_Error_e CAmSinkElement::getMainVolume(am_mainVolume_t& mainVolume) const
{
    am_Sink_s sinkData;
    am_Error_e result;
    //get the sink Info from Database
    result = mpControlReceive->getSinkInfoDB(getID(), sinkData);
    mainVolume = sinkData.mainVolume;
    return result;
}

am_Error_e CAmSinkElement::setMainVolume(const am_mainVolume_t mainVolume)
{
    //update the user volume in database
    am_mainVolume_t oldMainVolume;
    getMainVolume(oldMainVolume);
    if (mainVolume != oldMainVolume)
    {
        return mpControlReceive->changeSinkMainVolumeDB(mainVolume, getID());
    }
    return E_OK;
}

am_Error_e CAmSinkElement::saturateVolumeRange(am_mainVolume_t &newVolume)
{
    am_mainVolume_t maxVolume = 0;
    am_mainVolume_t minVolume = 0;
    LOG_FN_ENTRY();
    if (true == mMapUserToNormalizedVolume.empty())
    {
        LOG_FN_ERROR(" List of Volume range is empty");
        return E_UNKNOWN;
    }
    //Get the min/max volume range for requested sink
    minVolume = (am_volume_t)mMapUserToNormalizedVolume.begin()->first;
    maxVolume = (am_volume_t)(--mMapUserToNormalizedVolume.end())->first;
    //Satuarate the min/max volume
    if (newVolume < minVolume)
    {
        newVolume = minVolume;
    }
    else if (newVolume > maxVolume)
    {
        newVolume = maxVolume;
    }
    LOG_FN_EXIT(" Volume:", newVolume);
    return E_OK;
}

am_Error_e CAmSinkElement::getMainSoundPropertyValue(const am_CustomMainSoundPropertyType_t type,
                                                     int16_t& value) const
{
    return mpControlReceive->getMainSinkSoundPropertyValue(getID(), type, value);
}

am_Error_e CAmSinkElement::setMainSoundPropertyValue(const am_CustomMainSoundPropertyType_t type,
                                                     const int16_t value)
{
    am_MainSoundProperty_s property;
    property.type = type;
    property.value = value;
    return mpControlReceive->changeMainSinkSoundPropertyDB(property, getID());
}

template <typename TPropertyType, typename Tlisttype>
am_Error_e CAmSinkElement::_saturateSoundProperty(
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

am_Error_e CAmSinkElement::saturateMainSoundPropertyRange(
                const am_CustomMainSoundPropertyType_t mainSoundPropertyType,
                int16_t& soundPropertyValue)
{
    std::vector<gc_MainSoundProperty_s >::iterator itListSoundProperties;
    LOG_FN_ENTRY("type:value", mainSoundPropertyType, soundPropertyValue);
    return _saturateSoundProperty<am_CustomMainSoundPropertyType_t, gc_MainSoundProperty_s >(
                    mainSoundPropertyType, mSink.listGCMainSoundProperties, soundPropertyValue);
}

am_Error_e CAmSinkElement::saturateSoundPropertyRange(
                const am_CustomSoundPropertyType_t soundPropertyType, int16_t& soundPropertyValue)
{
    std::vector<gc_MainSoundProperty_s >::iterator itListSoundProperties;
    LOG_FN_ENTRY("type:value", soundPropertyType, soundPropertyValue);
    return _saturateSoundProperty<am_CustomSoundPropertyType_t, gc_SoundProperty_s >(
                    soundPropertyType, mSink.listGCSoundProperties, soundPropertyValue);
}

template <typename TPropertyType, typename Tlisttype>
bool CAmSinkElement::_isSoundPropertyConfigured(
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

am_Error_e CAmSinkElement::getSoundPropertyValue(const am_CustomSoundPropertyType_t type,
                                                 int16_t& value) const
{
    return mpControlReceive->getSinkSoundPropertyValue(getID(), type, value);
}

am_Error_e CAmSinkElement::setAvailability(const am_Availability_s& availability)
{
    return mpControlReceive->changeSinkAvailabilityDB(availability, getID());
}

am_Error_e CAmSinkElement::getAvailability(am_Availability_s& availability) const
{
    am_Sink_s sinkData;
    am_Error_e result;
    result = mpControlReceive->getSinkInfoDB(getID(), sinkData);
    availability = sinkData.available;
    return result;
}

am_Error_e CAmSinkElement::_register(void)
{
    am_sinkID_t sinkID(0);
    am_Error_e result = E_DATABASE_ERROR;
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

am_domainID_t CAmSinkElement::getDomainID(void)
{
    return mSink.domainID;
}

am_Error_e CAmSinkElement::getListConnectionFormats(
                std::vector<am_CustomConnectionFormat_t >& listConnectionFormats)
{
    listConnectionFormats = mSink.listConnectionFormats;
    return E_OK;
}

am_volume_t CAmSinkElement::convertMainVolumeToVolume(const am_mainVolume_t mainVolume)
{
    am_volume_t DBVolume;
    float decibelVolume;
    float normalisedVolume;
    int lastUserValue;
    float lastNormalizedVolume;
    std::map<int16_t, float >::iterator itMapUserVolumeToNormalizedVolume;
    std::map<float, float >::iterator itMapNormalizedVolumeToDecibleVolume;

    LOG_FN_ENTRY(" user volume:", mainVolume);
    if ((true == mMapUserToNormalizedVolume.empty()) || (true
                    == mMapNormalizedToDecibelVolume.empty()))
    {
        LOG_FN_DEBUG("  empty mapping", DBVolume);
        return mainVolume;
    }

    // First convert from User Volume to Normalized Volume
    normalisedVolume = mMapUserToNormalizedVolume.begin()->second;
    lastUserValue = mMapUserToNormalizedVolume.begin()->first;
    if (mainVolume > mMapUserToNormalizedVolume.begin()->first)
    {
        for (itMapUserVolumeToNormalizedVolume = mMapUserToNormalizedVolume.begin();
                        itMapUserVolumeToNormalizedVolume != mMapUserToNormalizedVolume.end();
                        ++itMapUserVolumeToNormalizedVolume)
        {
            if (itMapUserVolumeToNormalizedVolume->first > mainVolume)
            {
                if (itMapUserVolumeToNormalizedVolume->first != lastUserValue)
                {
                    normalisedVolume = (((itMapUserVolumeToNormalizedVolume->second
                                    - normalisedVolume)
                                         / ((float)itMapUserVolumeToNormalizedVolume->first - (float)lastUserValue))
                                        * ((float)mainVolume - (float)lastUserValue)
                                        + normalisedVolume);
                    break;
                }
            }
            normalisedVolume = itMapUserVolumeToNormalizedVolume->second;
            lastUserValue = itMapUserVolumeToNormalizedVolume->first;
        }
    }
    // Convert Normalized to Decibel
    decibelVolume = mMapNormalizedToDecibelVolume.begin()->second;
    lastNormalizedVolume = mMapNormalizedToDecibelVolume.begin()->first;
    if (normalisedVolume > mMapNormalizedToDecibelVolume.begin()->first)
    {
        for (itMapNormalizedVolumeToDecibleVolume = mMapNormalizedToDecibelVolume.begin();
                        itMapNormalizedVolumeToDecibleVolume != mMapNormalizedToDecibelVolume.end();
                        ++itMapNormalizedVolumeToDecibleVolume)
        {
            if (itMapNormalizedVolumeToDecibleVolume->first > normalisedVolume)
            {
                if (itMapNormalizedVolumeToDecibleVolume->first != lastNormalizedVolume)
                {
                    decibelVolume = (((itMapNormalizedVolumeToDecibleVolume->second - decibelVolume)
                                    / (itMapNormalizedVolumeToDecibleVolume->first - lastNormalizedVolume))
                                     * (normalisedVolume - lastNormalizedVolume)
                                     + decibelVolume);
                    break;
                }
            }
            decibelVolume = itMapNormalizedVolumeToDecibleVolume->second;
            lastNormalizedVolume = itMapNormalizedVolumeToDecibleVolume->first;
        }
    }
    if (decibelVolume > 0)
    {
        DBVolume = (am_volume_t)(((float)decibelVolume * (float)10.0) + 0.5f);
    }
    else
    {
        DBVolume = (am_volume_t)(((float)decibelVolume * (float)10.0) - 0.5f);
    }
    LOG_FN_EXIT(" decibel volume:", DBVolume);
    return DBVolume;
}

am_mainVolume_t CAmSinkElement::convertVolumeToMainVolume(const am_volume_t DBVolume)
{
    float mainVolume;
    float decibelVolume = (float)DBVolume / 10.0;
    float normalisedVolume;
    float lastDecibelVolume;
    float lastNormalisedVolume;
    std::map<float, float >::iterator itMapNormalizedVolumeToDecibleVolume;
    std::map<int16_t, float >::iterator itMapUserVolumeToNormalizedVolume;

    LOG_FN_ENTRY("-> decibel volume:", DBVolume);
    if ((true == mMapNormalizedToDecibelVolume.empty()) || (true
                    == mMapUserToNormalizedVolume.empty()))
    {
        LOG_FN_DEBUG("  empty mapping", (am_mainVolume_t)mainVolume);
        return DBVolume;
    }
    // Convert Decibel to normalized
    normalisedVolume = mMapNormalizedToDecibelVolume.begin()->first;
    lastDecibelVolume = mMapNormalizedToDecibelVolume.begin()->second;
    if (decibelVolume > mMapNormalizedToDecibelVolume.begin()->second)
    {
        for (itMapNormalizedVolumeToDecibleVolume = mMapNormalizedToDecibelVolume.begin();
                        itMapNormalizedVolumeToDecibleVolume != mMapNormalizedToDecibelVolume.end();
                        ++itMapNormalizedVolumeToDecibleVolume)
        {
            if (itMapNormalizedVolumeToDecibleVolume->second > decibelVolume)
            {
                if (itMapNormalizedVolumeToDecibleVolume->first != (float)normalisedVolume)
                {
                    normalisedVolume = (((float)itMapNormalizedVolumeToDecibleVolume->first
                                    - (float)normalisedVolume)
                                        / ((itMapNormalizedVolumeToDecibleVolume->second - lastDecibelVolume))
                                        * ((float)decibelVolume - (float)lastDecibelVolume)
                                        + normalisedVolume);
                    break;
                }
            }
            normalisedVolume = itMapNormalizedVolumeToDecibleVolume->first;
            lastDecibelVolume = itMapNormalizedVolumeToDecibleVolume->second;
        }
    }
    // Convert Normalized to User volume
    mainVolume = mMapUserToNormalizedVolume.begin()->first;
    lastNormalisedVolume = mMapUserToNormalizedVolume.begin()->second;
    if (normalisedVolume > mMapUserToNormalizedVolume.begin()->second)
    {
        for (itMapUserVolumeToNormalizedVolume = mMapUserToNormalizedVolume.begin();
                        itMapUserVolumeToNormalizedVolume != mMapUserToNormalizedVolume.end();
                        ++itMapUserVolumeToNormalizedVolume)
        {

            if (itMapUserVolumeToNormalizedVolume->first > mainVolume)
            {
                if (itMapUserVolumeToNormalizedVolume->first != (float)mainVolume)
                {
                    mainVolume = (((float)itMapUserVolumeToNormalizedVolume->first - (float)mainVolume) / ((itMapUserVolumeToNormalizedVolume->second
                                    - lastNormalisedVolume))
                                  * ((float)normalisedVolume - (float)lastNormalisedVolume)
                                  + mainVolume);
                    break;
                }
            }
            mainVolume = itMapUserVolumeToNormalizedVolume->first;
            lastNormalisedVolume = itMapUserVolumeToNormalizedVolume->second;
        }
    }
    am_mainVolume_t mainVolumeInt;
    if (mainVolume > 0)
    {
        mainVolumeInt = (am_mainVolume_t)((float)mainVolume + (float)0.5);
    }
    else
    {
        mainVolumeInt = (am_mainVolume_t)((float)mainVolume - (float)0.5);
    }

    LOG_FN_EXIT("  user volume->", (am_mainVolume_t)mainVolumeInt);
    return (am_mainVolume_t)mainVolumeInt;
}

am_Error_e CAmSinkElement::mainSoundPropertyToSoundProperty(
                const am_MainSoundProperty_s &mainSoundProperty, am_SoundProperty_s& soundProperty)
{
    am_Error_e error = E_DATABASE_ERROR;
    if (mSink.mapMSPTOSP[MD_MSP_TO_SP].find(mainSoundProperty.type) != mSink.mapMSPTOSP[MD_MSP_TO_SP].end())
    {
        soundProperty.type = mSink.mapMSPTOSP[MD_MSP_TO_SP][mainSoundProperty.type];
        soundProperty.value = mainSoundProperty.value;
        error = E_OK;
    }
    return error;
}

am_Error_e CAmSinkElement::soundPropertyToMainSoundProperty(
                const am_SoundProperty_s &soundProperty, am_MainSoundProperty_s& mainSoundProperty)
{
    am_Error_e error = E_DATABASE_ERROR;
    if (mSink.mapMSPTOSP[MD_SP_TO_MSP].find(soundProperty.type) != mSink.mapMSPTOSP[MD_SP_TO_MSP].end())
    {
        mainSoundProperty.type = mSink.mapMSPTOSP[MD_SP_TO_MSP][soundProperty.type];
        mainSoundProperty.value = soundProperty.value;
        error = E_OK;
    }
    return error;
}

am_Error_e CAmSinkElement::upadateDB(
                am_sinkClass_t classId, std::vector<am_SoundProperty_s > listSoundProperties,
                std::vector<am_CustomConnectionFormat_t > listConnectionFormats,
                std::vector<am_MainSoundProperty_s > listMainSoundProperties)
{
    am_Error_e result = E_OK;
    am_Sink_s sinkData;
    std::vector < am_SoundProperty_s > listUpdatedSoundProperties;
    std::vector < am_MainSoundProperty_s > listUpdatedMainSoundProperties;
    std::vector < am_CustomConnectionFormat_t > listUpdatedConnectionFormats;

    std::vector<am_SoundProperty_s >::iterator itListSoundProperties;
    std::vector<am_MainSoundProperty_s >::iterator itListMainSoundProperties;
    std::vector<am_CustomConnectionFormat_t >::iterator itListConnectionFormats;

    std::vector<am_SoundProperty_s >::iterator itListUpdatedSoundProperties;
    std::vector<am_MainSoundProperty_s >::iterator itListUpdatedMainSoundProperties;
    std::vector<am_CustomConnectionFormat_t >::iterator itListUpdatedConnectionFormats;

    mpControlReceive->getSinkInfoDB(getID(), sinkData);
    /*
     * Initialize the list with the already present sound properties
     */
    listUpdatedConnectionFormats = sinkData.listConnectionFormats;
    listUpdatedSoundProperties = sinkData.listSoundProperties;
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
        am_SoundProperty_s soundProperty = *itListSoundProperties;
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

bool CAmSinkElement::isVolumeChangeSupported() const
{
    return mSink.isVolumeChangeSupported;
}
} /* namespace gc */
} /* namespace am */
