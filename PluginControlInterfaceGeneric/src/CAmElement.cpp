/******************************************************************************
 * @file: CAmElement.cpp
 *
 * This file contains the definition of element class (member functions and
 * data members) used as base class for source, sink & gateway element classes
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

#include <algorithm>
#include <map>
#include "CAmElement.h"
#include "CAmControlReceive.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmElement::CAmElement(const std::string& name, CAmControlReceive* pControReceive) :
                                mpControlRecieve(pControReceive),
                                mName(name),
                                mID(0),
                                mType(ET_UNKNOWN),
                                mPriority(0),
                                mVolume(0),
                                mMainVolume(0),
                                mMuteState(MS_UNKNOWN),
                                mInterruptState(IS_UNKNOWN),
                                mState(0)
{
    mLimitVolume.limitType = LT_UNKNOWN;
    mLimitVolume.limitVolume = 0;
    mAvailability.availability = A_UNKNOWN;
    mAvailability.availabilityReason = AR_UNKNOWN;
}

CAmElement::~CAmElement()
{
    mName = "";
}

CAmControlReceive* CAmElement::getControlReceive(void)
{
    return mpControlRecieve;
}

std::string CAmElement::getName(void) const
{
    return mName;
}

int CAmElement::setType(const gc_Element_e type)
{
    mType = type;
}

gc_Element_e CAmElement::getType(void) const
{
    return mType;
}

void CAmElement::setID(const uint16_t ID)
{
    mID = ID;
}

uint16_t CAmElement::getID(void) const
{
    return mID;
}

am_Error_e CAmElement::setPriority(const int32_t priority)
{
    mPriority = priority;
    return E_OK;
}

am_Error_e CAmElement::getPriority(int32_t & priority) const
{
    priority =  mPriority;
    return E_OK;
}

am_Error_e CAmElement::setVolume(const am_volume_t volume)
{
    LOG_FN_ENTRY(volume);
    mVolume = volume;
    return E_OK;
}

am_Error_e CAmElement::getVolume(am_volume_t& volume) const
{
    volume = mVolume;
    return E_OK;
}

am_Error_e CAmElement::setMainVolume(const am_mainVolume_t volume)
{
    mMainVolume = volume;
    return E_OK;
}

am_Error_e CAmElement::getMainVolume(am_mainVolume_t& mainVolume) const
{
    mainVolume = mMainVolume;
    return E_UNKNOWN;
}

am_Error_e CAmElement::setLimitVolume(const gc_LimitVolume_s& limitVolume)
{
    LOG_FN_ENTRY("limitType", limitVolume.limitType, "limitVolume", limitVolume.limitVolume);
    mLimitVolume = limitVolume;
    return E_OK;
}

am_Error_e CAmElement::getLimitVolume(gc_LimitVolume_s& limitVolume) const
{
    limitVolume = mLimitVolume;
    return E_OK;
}

am_Error_e CAmElement::setMuteState(const am_MuteState_e muteState)
{
    mMuteState = muteState;
    return E_OK;
}

am_Error_e CAmElement::getMuteState(am_MuteState_e& muteState) const
{
    muteState = mMuteState;
    return E_UNKNOWN;
}

am_Error_e CAmElement::setInterruptState(const am_InterruptState_e interruptState)
{
    mInterruptState = interruptState;
    return E_OK;
}

am_Error_e CAmElement::getInterruptState(am_InterruptState_e& interruptState) const
{
    interruptState = mInterruptState;
    return E_OK;
}

am_Error_e CAmElement::setState(int state)
{
    mState = state;
    return E_OK;
}

am_Error_e CAmElement::getState(int& state) const
{
    state = mState;
    return E_OK;
}

am_Error_e CAmElement::setAvailability(const am_Availability_s& availability)
{
    mAvailability = availability;
    return E_NOT_POSSIBLE;
}

am_Error_e CAmElement::getAvailability(am_Availability_s& availability) const
{
    availability = mAvailability;
    return E_UNKNOWN;
}

} /* namespace gc */
} /* namespace am */
