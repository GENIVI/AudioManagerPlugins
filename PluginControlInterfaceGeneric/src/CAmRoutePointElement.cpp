/******************************************************************************
 * @file CAmRoutePointElement.cpp
 *
 * This file contains the definition of route-point element class (member functions and
 * data members) used as base class for source and sink element classes. Volume and 
 * sound-property handling is moved here from former location in class CAmElement.
 *
 * @component AudioManager Generic Controller
 *
 * @authors Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2019 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/


#include "CAmRoutePointElement.h"

#include <cmath>

namespace am {
namespace gc {


// *****************************************************************************
//                 class   R o u t e - P o i n t - E l e m e n t
//
// common base class for sink and source elements
//

CAmRoutePointElement::CAmRoutePointElement(gc_Element_e type, const std::string &name
        , gc_NodePoint_s &config, IAmControlReceive *pControlReceive)
    : CAmLimitableElement(type, name, pControlReceive)
    , mConfig(config)
    , mVolume(0)
    , mMinVolume(0)
    , mMaxVolume(0)
    , mOffsetVolume(AM_VOLUME_NO_LIMIT)
    , mMuteState(MS_UNKNOWN)
{
    // Note: mConfig is a reference to data assigned in constructor of derived class.
    //       Do NOT read from or write to it here!
}

std::string CAmRoutePointElement::getElementClassName() const
{
    return mConfig.className;
}

bool CAmRoutePointElement::isPersistencySupported() const
{
    return mConfig.isPersistencySupported;
}

/*
 * Volume Management stuff
 */
void CAmRoutePointElement::setVolume(const am_volume_t volume)
{
    LOG_FN_DEBUG(__FILENAME__, __func__, mType, mName, "changing", mVolume, "to", volume);

    mVolume = volume;
}

am_volume_t CAmRoutePointElement::getVolume(void) const
{
    return mVolume;
}

am_volume_t CAmRoutePointElement::getMinVolume(void) const
{
    return mMinVolume;
}

am_volume_t CAmRoutePointElement::getMaxVolume(void) const
{
    return mMaxVolume;
}

bool CAmRoutePointElement::getVolumeSupport() const
{
    return mConfig.isVolumeChangeSupported;
}

bool CAmRoutePointElement::isVolumePersistencySupported() const
{
    return mConfig.isVolumePersistencySupported;
}

void CAmRoutePointElement::setOffsetVolume(const am_volume_t limitVolume)
{
    mOffsetVolume = limitVolume;
}

am_volume_t CAmRoutePointElement::getOffsetVolume() const
{
    return mOffsetVolume;
}

void CAmRoutePointElement::setMuteState(const am_MuteState_e muteState)
{
    am_MuteState_e oldMuteState = mMuteState;
    if (oldMuteState != muteState)
    {
        LOG_FN_DEBUG(__FILENAME__, mType, mName, __func__, oldMuteState, "-->", muteState);
        if (mType == ET_SINK)
        {
            am_Error_e result = mpControlReceive->changeSinkMuteStateDB(muteState, getID());
            if ((result == E_OK) || (result == E_NO_CHANGE))
            {
                LOG_FN_ERROR(__FILENAME__, mType, mName, __func__, "FAILED updating database");
            }
        }

        mMuteState = muteState;
    }
}

am_MuteState_e CAmRoutePointElement::getMuteState() const
{
    return mMuteState;
}

am_volume_t CAmRoutePointElement::getRoutingSideVolume(am_volume_t internalVolume)
{
    return lroundf(_lookupForward((float)internalVolume, mConfig.mapRoutingVolume));
}

float CAmRoutePointElement::_lookupForward(float lhs, const std::map<float, float> &mapValues)
{
    if (mapValues.empty())
    {
        return lhs;
    }

    auto upper = mapValues.upper_bound(lhs);
    if (upper == mapValues.end())
    {
        return (--upper)->second;
    }
    else if (upper == mapValues.begin())
    {
        return upper->second;
    }

    auto lower = std::prev(upper);
    return (upper->second - lower->second) / (upper->first - lower->first)
                   * (lhs - lower->first) + lower->second;
}

float CAmRoutePointElement::_lookupReverse(float rhs, const std::map<float, float> &mapValues)
{
    // create a temporary, reversed map
    std::map<float, float> mapReverse;
    for (const auto &pair : mapValues)
    {
        mapReverse[pair.second] = pair.first;
    }

    // re-use above function
    return _lookupForward(rhs, mapReverse);
}

int32_t CAmRoutePointElement::getPriority(void) const
{
    return mConfig.priority;
}

bool CAmRoutePointElement::isMSPPersistenceSupported(const am_CustomMainSoundPropertyType_t &type) const
{
    for (auto &itlistGCMainSoundProperties : mConfig.listGCMainSoundProperties)
    {
        if (itlistGCMainSoundProperties.type == type)
        {
            return itlistGCMainSoundProperties.isPersistenceSupported;
        }
    }

    return false;
}

am_Error_e CAmRoutePointElement::mainSoundPropertyToSoundProperty(
    const am_MainSoundProperty_s &mainSoundProperty, am_SoundProperty_s &soundProperty)
{
    am_Error_e error = E_DATABASE_ERROR;
    if (mConfig.mapMSPTOSP[MD_MSP_TO_SP].find(mainSoundProperty.type) != mConfig.mapMSPTOSP[MD_MSP_TO_SP].end())
    {
        soundProperty.type  = mConfig.mapMSPTOSP[MD_MSP_TO_SP][mainSoundProperty.type];
        soundProperty.value = mainSoundProperty.value;
        error               = E_OK;
    }

    return error;
}

am_Error_e CAmRoutePointElement::soundPropertyToMainSoundProperty(
    const am_SoundProperty_s &soundProperty, am_MainSoundProperty_s &mainSoundProperty)
{
    am_Error_e error = E_DATABASE_ERROR;
    if (mConfig.mapMSPTOSP[MD_SP_TO_MSP].find(soundProperty.type) != mConfig.mapMSPTOSP[MD_SP_TO_MSP].end())
    {
        mainSoundProperty.type  = mConfig.mapMSPTOSP[MD_SP_TO_MSP][soundProperty.type];
        mainSoundProperty.value = soundProperty.value;
        error                   = E_OK;
    }

    return error;
}

am_Error_e CAmRoutePointElement::saturateMainSoundPropertyRange(
    const am_CustomMainSoundPropertyType_t mainSoundPropertyType,
    int16_t &soundPropertyValue)
{
    LOG_FN_ENTRY(__FILENAME__, __func__, "type:value", mainSoundPropertyType, soundPropertyValue);
    return _saturateSoundProperty<am_CustomMainSoundPropertyType_t, gc_MainSoundProperty_s >(
        mainSoundPropertyType, mConfig.listGCMainSoundProperties, soundPropertyValue);
}

am_Error_e CAmRoutePointElement::saturateSoundPropertyRange(
    const am_CustomSoundPropertyType_t soundPropertyType, int16_t &soundPropertyValue)
{
    LOG_FN_ENTRY(__FILENAME__, __func__, "type:value", soundPropertyType, soundPropertyValue);
    return _saturateSoundProperty<am_CustomSoundPropertyType_t, gc_SoundProperty_s >(
        soundPropertyType, mConfig.listGCSoundProperties, soundPropertyValue);
}


} /* namespace gc */
} /* namespace am */
