/******************************************************************************
 * @file: CAmClassActionSetLimitState.cpp
 *
 * This file contains the definition of user action limit volume class
 * (member functions and data members) used to implement the logic of limiting
 * the volume of connection at user level
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

#include "CAmLogger.h"
#include "CAmMainConnectionElement.h"
#include "CAmMainConnectionActionSetLimitState.h"
#include "CAmClassElement.h"
#include "CAmSinkElement.h"
#include "CAmSourceElement.h"
#include "CAmClassActionSetLimitState.h"
#include "CAmSinkActionSetVolume.h"
#include "CAmSourceActionSetVolume.h"

namespace am {
namespace gc {

CAmClassActionSetLimitState::CAmClassActionSetLimitState(CAmClassElement *pClassElement) :
                                CAmActionContainer(std::string("CAmClassActionSetLimitVolume")),
                                mpClassElement(pClassElement),
                                mRampTimeParam(DEFAULT_RAMP_TIME),
                                mRampTypeParam(DEFAULT_RAMP_TYPE),
                                mPatternParam(DEFAULT_LIMIT_PATTERN),
                                mOpRequestedParam(LS_UNLIMITED)
{
    this->_registerParam(ACTION_PARAM_PATTERN, &mPatternParam);
    this->_registerParam(ACTION_PARAM_RAMP_TIME, &mRampTimeParam);
    this->_registerParam(ACTION_PARAM_RAMP_TYPE, &mRampTypeParam);
    this->_registerParam(ACTION_PARAM_VOLUME, &mTargetVolumeParam);
    this->_registerParam(ACTION_PARAM_VOLUME_STEP, &mOffsetVolumeParam);
    this->_registerParam(ACTION_PARAM_LIMIT_STATE, &mOpRequestedParam);
    this->_registerParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    this->_registerParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
}

CAmClassActionSetLimitState::~CAmClassActionSetLimitState()
{
}

int CAmClassActionSetLimitState::_execute(void)
{
    gc_LimitState_e limitState;
    uint32_t limitVolumePattern;
    am_volume_t volume = 0;
    std::string sinkName;
	std::string sourceName;
    mOpRequestedParam.getParam(limitState);
    CAmActionParam < gc_LimitType_e > limitTypeParam;
    CAmActionParam < am_volume_t > limitVolumeParam;
    IAmActionCommand* pAction = NULL;
    bool createMainConnectionLimit = false;
    gc_LimitVolume_s limitVolume;
    CAmMainConnectionElement* pMainConnection = NULL;
    CAmSinkElement* pSinkElement;
	CAmSourceElement* pSourceElement;
    std::vector < am_ConnectionState_e > listConnectionStates {CS_CONNECTED};
    pMainConnection = mpClassElement->getMainConnection("", "", listConnectionStates);

    limitVolume.limitType = LT_UNKNOWN;
    limitVolume.limitVolume = 0;
    mPatternParam.getParam(limitVolumePattern);
    if (limitState == LS_LIMITED)
    {
        if (true == mTargetVolumeParam.getParam(volume))
        {
            limitVolume.limitType = LT_ABSOLUTE;
            limitVolume.limitVolume = volume;
        }
        if (true == mOffsetVolumeParam.getParam(volume))
        {
            limitVolume.limitType = LT_RELATIVE;
            limitVolume.limitVolume = volume;
        }
    }
    else
    {
        LOG_FN_INFO("unlimit action");
    }
    if(mpClassElement->getType()==ET_SINK)
    {
    if (true == mSinkNameParam.getParam(sinkName))
    {
        pSinkElement = CAmSinkFactory::getElement(sinkName);
        if (pSinkElement == NULL)
        {
            return E_NOT_POSSIBLE;
        }
        pSinkElement->setLimitVolume(limitVolume);
    }
    else
    {
        mpClassElement->setLimitState(limitState, limitVolume, limitVolumePattern);
        createMainConnectionLimit = true;
    }
    if (createMainConnectionLimit == true)
    {
        if (pMainConnection != NULL)
        {
            pMainConnection->getVolume(volume);
            mpClassElement->getClassLimitVolume(volume, limitVolume);
            pAction = new CAmMainConnectionActionSetLimitState(pMainConnection);
            if (NULL == pAction)
            {
                LOG_FN_ERROR(" not able to create main connection Action  volume object");
                return E_NOT_POSSIBLE;
            }
        }
    }
    else
    {
        pAction = new CAmSinkActionSetVolume(pSinkElement);
    }
    }
    else
    {

    if (true == mSourceNameParam.getParam(sourceName))
       {
           pSourceElement = CAmSourceFactory::getElement(sourceName);
           if (pSourceElement == NULL)
           {
               return E_NOT_POSSIBLE;
           }
           pSourceElement->setLimitVolume(limitVolume);
       }
       else
       {
           mpClassElement->setLimitState(limitState, limitVolume, limitVolumePattern);
           createMainConnectionLimit = true;
       }
       if (createMainConnectionLimit == true)
       {
           if (pMainConnection != NULL)
           {
               pMainConnection->getVolume(volume);
               mpClassElement->getClassLimitVolume(volume, limitVolume);
               pAction = new CAmMainConnectionActionSetLimitState(pMainConnection);
               if (NULL == pAction)
               {
                   LOG_FN_ERROR(" not able to create main connection Action  volume object");
                   return E_NOT_POSSIBLE;
               }
           }
       }
       else
       {
           pAction = new CAmSourceActionSetVolume(pSourceElement);
       }

    }
    if (pAction != NULL)
    {
        limitTypeParam.setParam(limitVolume.limitType);
        limitVolumeParam.setParam(limitVolume.limitVolume);
        pAction->setParam(ACTION_PARAM_LIMIT_TYPE, &limitTypeParam);
        pAction->setParam(ACTION_PARAM_LIMIT_VOLUME, &limitVolumeParam);
        pAction->setParam(ACTION_PARAM_RAMP_TIME, &mRampTimeParam);
        pAction->setParam(ACTION_PARAM_RAMP_TYPE, &mRampTypeParam);
        append(pAction);
    }
    return E_OK;
}

} /* namespace gc */
} /* namespace am */

