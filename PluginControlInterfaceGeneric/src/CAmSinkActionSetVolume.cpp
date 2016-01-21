/******************************************************************************
 * @file: CAmSinkActionSetVolume.cpp
 *
 * This file contains the definition of user action set volume sink
 * (member functions and data members) used to implement the logic of setting
 * the volume of element at user level
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

#include "CAmSinkActionSetVolume.h"
#include "CAmControlReceive.h"
#include "CAmSinkElement.h"
#include "CAmSourceActionSetVolume.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmSinkActionSetVolume::CAmSinkActionSetVolume(CAmSinkElement *pSinkElement) :
                                CAmActionCommand(std::string("CAmSinkActionSetVolume")),
                                mpSink(pSinkElement),
                                mOldVolume(0),
                                mRampTimeParam(DEFAULT_RAMP_TIME),
                                mRampTypeParam(DEFAULT_RAMP_TYPE)
{
    mOldLimitVolume.limitType = LT_UNKNOWN;
    this->_registerParam(ACTION_PARAM_MAIN_VOLUME, &mMainVolumeParam);
    this->_registerParam(ACTION_PARAM_VOLUME, &mVolumeParam);
    this->_registerParam(ACTION_PARAM_LIMIT_TYPE, &mLimitTypeParam);
    this->_registerParam(ACTION_PARAM_LIMIT_VOLUME, &mLimitVolumeParam);
    this->_registerParam(ACTION_PARAM_RAMP_TIME, &mRampTimeParam);
    this->_registerParam(ACTION_PARAM_RAMP_TYPE, &mRampTypeParam);
}

CAmSinkActionSetVolume::~CAmSinkActionSetVolume()
{
}

int CAmSinkActionSetVolume::_execute(void)
{
    am_mainVolume_t mainVolume = 0;
    am_volume_t volume = 0;
    gc_LimitVolume_s limitVolume;
    mpSink->getLimitVolume(mOldLimitVolume);
    mpSink->getVolume(mOldVolume);
    mMainVolumeParam.getParam(mainVolume);
    if (false == mVolumeParam.getParam(volume))
    {
        volume = mOldVolume;
    }

    if (true == mMainVolumeParam.getParam(mainVolume))
    {
        /*
         * This is a request to set the mainvolume from user side or policy side
         */
        volume = mpSink->convertMainVolumeToVolume(mainVolume);
    }
    if ((true == mLimitTypeParam.getParam(limitVolume.limitType)) && (true
                    == mLimitVolumeParam.getParam(limitVolume.limitVolume)))
    {
        mpSink->setLimitVolume(limitVolume);
    }
    mpSink->setVolume(volume);
    return _setRoutingSideVolume();
}

int CAmSinkActionSetVolume::_setRoutingSideVolume()
{
    am_CustomRampType_t rampType;
    am_time_t rampTime;
    int result;
    am_volume_t volume = 0;
    gc_LimitVolume_s limitVolume;
    CAmControlReceive* pControlReceive = mpSink->getControlReceive();
    mRampTypeParam.getParam(rampType);
    mRampTimeParam.getParam(rampTime);
    /*
     * based on the volume and limit volume to be set
     */
    mpSink->getVolume(volume);
    mpSink->getLimitVolume(limitVolume);
    if (limitVolume.limitType == LT_ABSOLUTE)
    {
        volume = limitVolume.limitVolume;
    }
    if (limitVolume.limitType == LT_RELATIVE)
    {
        volume += limitVolume.limitVolume;
    }

    result = pControlReceive->setSinkVolume(mpSink->getID(), volume, rampType, rampTime);
    if (result == E_OK)
    {
        pControlReceive->registerObserver(this);
        result = E_WAIT_FOR_CHILD_COMPLETION;
    }
    return result;
}
int CAmSinkActionSetVolume::_update(const int result)
{
    am_mainVolume_t mainVolume;
    am_volume_t volume;
    gc_LimitVolume_s limitVolume;
    mpSink->getControlReceive()->unregisterObserver(this);
    if (((E_OK == result) && (AS_COMPLETED == getStatus())) || (AS_UNDO_COMPLETE == getStatus()))
    {
        mpSink->getLimitVolume(limitVolume);
        if ((limitVolume.limitType == LT_ABSOLUTE) && (limitVolume.limitVolume == AM_MUTE))
        {
            mpSink->setMuteState(MS_MUTED);
        }
        else
        {
            mpSink->setMuteState(MS_UNMUTED);
        }

        mpSink->getVolume(volume);
        mainVolume = mpSink->convertVolumeToMainVolume(volume);
        mpSink->setMainVolume(mainVolume);
    }
    mpSink->getControlReceive()->unregisterObserver(this);
    return E_OK;
}

int CAmSinkActionSetVolume::_undo(void)
{
    mpSink->setLimitVolume(mOldLimitVolume);
    mpSink->setVolume(mOldVolume);
    return _setRoutingSideVolume();
}

} /* namespace gc */
} /* namespace am */
