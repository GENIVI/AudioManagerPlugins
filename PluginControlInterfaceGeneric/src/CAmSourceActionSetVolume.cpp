/******************************************************************************
 * @file: CAmSourceActionSetVolume.cpp
 *
 * This file contains the definition of router action set volume source
 * (member functions and data members) used to implement the logic of setting
 * the volume of element at router level
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

#include "CAmSourceActionSetVolume.h"
#include "CAmControlReceive.h"
#include "CAmSourceElement.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmSourceActionSetVolume::CAmSourceActionSetVolume(CAmSourceElement* pSource) :
                                CAmActionCommand(std::string("CAmSourceActionSetVolume")),
                                mpSource(pSource),
                                mOldVolume(0),
                                mRampTypeParam(DEFAULT_RAMP_TYPE),
                                mRampTimeParam(DEFAULT_RAMP_TIME)
{
    this->_registerParam(ACTION_PARAM_RAMP_TIME, &mRampTimeParam);
    this->_registerParam(ACTION_PARAM_RAMP_TYPE, &mRampTypeParam);
    this->_registerParam(ACTION_PARAM_VOLUME, &mVolumeParam);
    this->_registerParam(ACTION_PARAM_LIMIT_TYPE, &mLimitTypeParam);
    this->_registerParam(ACTION_PARAM_LIMIT_VOLUME, &mLimitVolumeParam);
}

CAmSourceActionSetVolume::~CAmSourceActionSetVolume()
{
}

int CAmSourceActionSetVolume::_execute(void)
{
    am_volume_t volume(0);
    gc_LimitVolume_s limitVolume;

    if ((false == mLimitTypeParam.getParam(limitVolume.limitType)) && (false
                    == mLimitVolumeParam.getParam(limitVolume.limitVolume))
        && (false == mVolumeParam.getParam(volume)))
    {
        LOG_FN_ERROR("parameters not set properly");
        return E_NOT_POSSIBLE;
    }
    // remember old volume
    mpSource->getVolume(mOldVolume);
    mpSource->getLimitVolume(mOldLimitVolume);

    mpSource->setVolume(volume);
    mpSource->setLimitVolume(limitVolume);
    return _setRoutingSideVolume();
}

int CAmSourceActionSetVolume::_setRoutingSideVolume()
{
    am_CustomRampType_t rampType;
    am_time_t rampTime;
    int result;
    gc_LimitVolume_s limitVolume;
    am_volume_t volume = 0;
    CAmControlReceive* pControlReceive = mpSource->getControlReceive();
    mRampTypeParam.getParam(rampType);
    mRampTimeParam.getParam(rampTime);
    /*
     * based on the volume and limit volume to be set
     */
    mpSource->getVolume(volume);
    mpSource->getLimitVolume(limitVolume);
    if (limitVolume.limitType == LT_ABSOLUTE)
    {
        volume = limitVolume.limitVolume;
    }
    if (limitVolume.limitType == LT_RELATIVE)
    {
        volume += limitVolume.limitVolume;
    }

    result = pControlReceive->setSourceVolume(mpSource->getID(), volume, rampType, rampTime);
    if (result == E_OK)
    {
        pControlReceive->registerObserver(this);
        result = E_WAIT_FOR_CHILD_COMPLETION;
    }
    return result;
}
int CAmSourceActionSetVolume::_update(const int result)
{
    mpSource->getControlReceive()->unregisterObserver(this);
    return E_OK;
}

int CAmSourceActionSetVolume::_undo(void)
{
    mpSource->setLimitVolume(mOldLimitVolume);
    mpSource->setVolume(mOldVolume);
    return _setRoutingSideVolume();
}

} /* namespace gc */
} /* namespace am */
