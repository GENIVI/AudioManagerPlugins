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
        am_mainVolume_t mainVolume = 0;
        am_volume_t volume = 0;
        gc_LimitVolume_s limitVolume;
        mpSource->getLimitVolume(mOldLimitVolume);
        mpSource->getVolume(mOldVolume);
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
           // volume = mpSource->convertMainVolumeToVolume(mainVolume);
        }
        if ((true == mLimitTypeParam.getParam(limitVolume.limitType)) && (true
                        == mLimitVolumeParam.getParam(limitVolume.limitVolume)))
        {
        	mpSource->setLimitVolume(limitVolume);
        }
        mpSource->setVolume(volume);
    return _setRoutingSideVolume();
}

int CAmSourceActionSetVolume::_setRoutingSideVolume()
{
	am_CustomRampType_t rampType;
	am_time_t rampTime;
	int result;
	gc_LimitVolume_s limitVolume;
	am_volume_t volume = 0;
	am_sourceID_t sourceID;
	am_Source_s sourceData;
	CAmControlReceive* pControlReceive = mpSource->getControlReceive();
	mRampTypeParam.getParam(rampType);
	mRampTimeParam.getParam(rampTime);
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
	am_mainVolume_t mainVolume;
	am_volume_t volume;
	gc_LimitVolume_s limitVolume;
	mpSource->getControlReceive()->unregisterObserver(this);
	if (((E_OK == result) && (AS_COMPLETED == getStatus())) || (AS_UNDO_COMPLETE == getStatus()))
	{
		mpSource->getLimitVolume(limitVolume);
		if (limitVolume.limitType == LT_ABSOLUTE)
		{
			if(limitVolume.limitVolume == AM_MUTE)
			{
				mpSource->setMuteState(MS_MUTED);
			}
		}
		else
		{
			if(limitVolume.limitVolume == 34)
			{
				 mpSource->setMuteState(MS_UNMUTED);
			}
			else
			{
				mpSource->setMuteState(MS_UNMUTED);
			}
		}
		mpSource->getVolume(volume);
		mainVolume = mpSource->convertVolumeToMainVolumeSource(volume);
		mpSource->setMainVolume(mainVolume);
}
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
