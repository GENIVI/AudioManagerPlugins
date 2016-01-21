/******************************************************************************
 * @file: CAmMainConnectionActionSetLimitState.cpp
 *
 * This file contains the definition of main connection action limit volume class
 * (member functions and data members) used to implement the logic of limiting
 * the volume at main connection level
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

#include "CAmMainConnectionActionSetLimitState.h"
#include "CAmMainConnectionElement.h"
#include "CAmSinkActionSetVolume.h"
#include "CAmSourceActionSetVolume.h"
#include "CAmLogger.h"

namespace am {

namespace gc {

CAmMainConnectionActionSetLimitState::CAmMainConnectionActionSetLimitState(
                CAmMainConnectionElement* pMainConnection) :
                                CAmActionContainer(
                                                std::string("CAmMainConnectionActionLimitState")),
                                mpMainConnection(pMainConnection),
                                mRampTimeParam(DEFAULT_RAMP_TIME),
                                mRampTypeParam(DEFAULT_RAMP_TYPE)
{
    this->_registerParam(ACTION_PARAM_RAMP_TIME, &mRampTimeParam);
    this->_registerParam(ACTION_PARAM_RAMP_TYPE, &mRampTypeParam);
    this->_registerParam(ACTION_PARAM_LIMIT_TYPE, &mLimitTypeParam);
    this->_registerParam(ACTION_PARAM_LIMIT_VOLUME, &mLimitVolumeParam);
}

CAmMainConnectionActionSetLimitState::~CAmMainConnectionActionSetLimitState()
{
}

int CAmMainConnectionActionSetLimitState::_execute(void)
{
    am_volume_t dbVolume;
    am_Error_e result;
    gc_LimitVolume_s limitVolume;
    am_volume_t limitVolumeValue;
    std::vector < std::pair<CAmElement*, gc_LimitVolume_s > > listLimitElements;
    std::vector<std::pair<CAmElement*, gc_LimitVolume_s > >::iterator itListLimitElements;
    limitVolume.limitType = LT_UNKNOWN;
    limitVolume.limitVolume = 0;
    mLimitTypeParam.getParam(limitVolume.limitType);
    if (true == mLimitVolumeParam.getParam(limitVolumeValue))
    {
        limitVolume.limitVolume = limitVolumeValue;
    }
    mpMainConnection->setLimitVolume(limitVolume);
    result = mpMainConnection->getLimitElement(NULL, 0, listLimitElements);
    if (result == E_OK)
    {
        for (itListLimitElements = listLimitElements.begin();
                        itListLimitElements != listLimitElements.end(); itListLimitElements++)
        {
            IAmActionCommand* pElementActionSetVolume;
            if (itListLimitElements->first->getType() == ET_SOURCE)
            {
                pElementActionSetVolume = new CAmSourceActionSetVolume(
                                (CAmSourceElement*)itListLimitElements->first);
            }
            else
            {
                pElementActionSetVolume = new CAmSinkActionSetVolume(
                                (CAmSinkElement*)itListLimitElements->first);
            }
            if (NULL == pElementActionSetVolume)
            {
                LOG_FN_ERROR(" not able to create router Action set volume object");
                return E_NOT_POSSIBLE;
            }
            pElementActionSetVolume->setParam(ACTION_PARAM_RAMP_TYPE, &mRampTypeParam);
            pElementActionSetVolume->setParam(ACTION_PARAM_RAMP_TIME, &mRampTimeParam);
            CAmActionParam < gc_LimitType_e > limitTypeParam(itListLimitElements->second.limitType);
            pElementActionSetVolume->setParam(ACTION_PARAM_LIMIT_TYPE, &limitTypeParam);
            CAmActionParam < am_volume_t > limitVolumeParam(
                            itListLimitElements->second.limitVolume);
            pElementActionSetVolume->setParam(ACTION_PARAM_LIMIT_VOLUME, &limitVolumeParam);
            pElementActionSetVolume->setUndoRequried(getUndoRequired());
            // add the newly created router level Set Volume action to dynamic action
            append(pElementActionSetVolume);

        }
    }
    return result;
}

int CAmMainConnectionActionSetLimitState::_update(const int result)
{
    CAmElement* pElement = NULL;
    if ((E_OK == result) && (getStatus() == AS_COMPLETED))
    {
    }
    return E_OK;
}

} /* namespace gc */
} /* namespace am */
