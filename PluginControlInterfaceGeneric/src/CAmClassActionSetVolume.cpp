/******************************************************************************
 * @file: CAmClassActionSetVolume.cpp
 *
 * This file contains the definition of user action set volume class
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

#include "CAmClassActionSetVolume.h"
#include "CAmClassElement.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmMainConnectionElement.h"
#include "CAmSinkActionSetVolume.h"
#include "CAmSourceActionSetVolume.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmClassActionSetVolume::CAmClassActionSetVolume(CAmClassElement *pClassElement) :
                                CAmActionContainer(std::string("CAmClassActionSetVolume")),
                                mpElement(NULL),
                                mpClassElement(pClassElement),
                                mRampTimeParam(DEFAULT_RAMP_TIME),
                                mRampTypeParam(DEFAULT_RAMP_TYPE)
{
    this->_registerParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    this->_registerParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    this->_registerParam(ACTION_PARAM_VOLUME, &mVolumeParam);
    this->_registerParam(ACTION_PARAM_VOLUME_STEP, &mVolumeStepParam);
    this->_registerParam(ACTION_PARAM_RAMP_TIME, &mRampTimeParam);
    this->_registerParam(ACTION_PARAM_RAMP_TYPE, &mRampTypeParam);
    this->_registerParam(ACTION_PARAM_MAIN_VOLUME, &mMainVolumeParam);
    this->_registerParam(ACTION_PARAM_MAIN_VOLUME_STEP, &mMainVolumeStepParam);
}

CAmClassActionSetVolume::~CAmClassActionSetVolume()
{

}

int CAmClassActionSetVolume::_execute(void)
{
    am_Error_e result = E_NOT_POSSIBLE;
    CAmMainConnectionElement* pMainConnection;
    std::vector < std::pair<CAmElement*, gc_LimitVolume_s > > listLimitElement;
    am_volume_t volume;
    am_mainVolume_t mainVolume;
    pMainConnection = mpClassElement->getMainConnection(CS_CONNECTED);
    std::vector<IAmActionCommand* > listSetVolumeActions;
    std::vector<IAmActionCommand* >::iterator itListSetVolumeActions;
    IAmActionCommand* pRequestElementAction = NULL;
    mVolumeParam.getParam(volume);
    if (E_OK != _getParameters())
    {
        return E_NOT_POSSIBLE;
    }
    if (mpElement->getType() == ET_SOURCE)
    {
        if ((pMainConnection != NULL) && (pMainConnection->getMainSourceName()
                        == mpElement->getName()))
        {
            pMainConnection->getLimitElement(mpElement, volume, listLimitElement);
        }
    }
    else
    {
        if ((pMainConnection != NULL) && (pMainConnection->getMainSinkName() == mpElement->getName()))
        {
            if (true == mMainVolumeParam.getParam(mainVolume))
            {
                volume = ((CAmSinkElement*)mpElement)->convertMainVolumeToVolume(mainVolume);
            }
            else
            {
                mVolumeParam.getParam(volume);
            }
            pMainConnection->getLimitElement(mpElement, volume, listLimitElement);
        }
    }
    if (E_OK == _CreateSetVolumeActionList(listLimitElement, listSetVolumeActions))
    {
        for (itListSetVolumeActions = listSetVolumeActions.begin();
                        itListSetVolumeActions != listSetVolumeActions.end();
                        ++itListSetVolumeActions)
        {
            append(*itListSetVolumeActions);
        }
        result = E_OK;
    }
    return result;
}

am_Error_e CAmClassActionSetVolume::_getParameters()
{
    std::string elementName;
    am_volume_t volume;
    am_mainVolume_t mainVolume;
    if ((false == mSinkNameParam.getParam(elementName)))
    {
        if ((false == mSourceNameParam.getParam(elementName)))
        {
            LOG_FN_ERROR("  elementName not set");
            return E_NOT_POSSIBLE;
        }
    }
    if (mSinkNameParam.getParam(elementName) == true)
    {
        mpElement = CAmSinkFactory::getElement(elementName);
    }
    else
    {
        mpElement = CAmSourceFactory::getElement(elementName);
    }
    if (NULL == mpElement)
    {
        LOG_FN_ERROR("  element name not valid", elementName);
        return E_NOT_POSSIBLE;
    }
    if((false == mVolumeParam.getParam(volume)) &&
       (false == mVolumeStepParam.getParam(volume)) &&
       (false == mMainVolumeParam.getParam(mainVolume))&&
       (false == mMainVolumeStepParam.getParam(mainVolume))
      )
    {
        LOG_FN_ERROR("  volume not set");
        return E_NOT_POSSIBLE;
    }
    return E_OK;
}

int CAmClassActionSetVolume::_deleteSetVolumeActionList(
                std::vector<IAmActionCommand* >& listActions)
{
    std::vector<IAmActionCommand* >::iterator itListSetVolumeActions;
    for (itListSetVolumeActions = listActions.begin(); itListSetVolumeActions != listActions.end();
                    ++itListSetVolumeActions)
    {
        delete (*itListSetVolumeActions);
    }
    listActions.clear();
    return E_OK;
}

int CAmClassActionSetVolume::_CreateSetVolumeActionList(
                std::vector<std::pair<CAmElement*, gc_LimitVolume_s > >& listLimitElement,
                std::vector<IAmActionCommand* >& listActions)
{
    am_mainVolume_t mainVolume;
    am_volume_t volume;
    IAmActionCommand * pRequestedElement = NULL;
    std::vector<std::pair<CAmElement*, gc_LimitVolume_s > >::iterator itListLimitElement;
    for (itListLimitElement = listLimitElement.begin();
                    itListLimitElement != listLimitElement.end(); ++itListLimitElement)
    {
        CAmActionParam < gc_LimitType_e > limitTypeParam;
        CAmActionParam < am_volume_t > limitVolumeParam;
        IAmActionCommand* pSetVolumeAction;
        if (itListLimitElement->first->getType() == ET_SOURCE)
        {
            pSetVolumeAction = new CAmSourceActionSetVolume(
                            (CAmSourceElement*)itListLimitElement->first);
        }
        else
        {
            pSetVolumeAction = new CAmSinkActionSetVolume(
                            (CAmSinkElement*)itListLimitElement->first);
        }
        if (pSetVolumeAction == NULL)
        {
            break;
        }
        if ((mpElement->getName() == itListLimitElement->first->getName()) && (mpElement->getType()
                        == itListLimitElement->first->getType()))
        {
            pRequestedElement = pSetVolumeAction;
        }
        listActions.push_back(pSetVolumeAction);
    }
    if (listActions.size() != listLimitElement.size())
    {
        _deleteSetVolumeActionList(listActions);
        return E_NOT_POSSIBLE;
    }
    if (pRequestedElement == NULL)
    {
        if (mpElement->getType() == ET_SOURCE)
        {
            pRequestedElement = new CAmSourceActionSetVolume((CAmSourceElement*)mpElement);
        }
        else
        {
            pRequestedElement = new CAmSinkActionSetVolume((CAmSinkElement*)mpElement);
        }
        if (pRequestedElement != NULL)
        {
            listActions.push_back(pRequestedElement);
        }
        else
        {
            _deleteSetVolumeActionList(listActions);
            return E_NOT_POSSIBLE;
        }
    }
    if(mpElement->getType()==ET_SINK)
    {
        if (true == mMainVolumeParam.getParam(mainVolume))
        {
            pRequestedElement->setParam(ACTION_PARAM_MAIN_VOLUME, &mMainVolumeParam);
        }
        if (true == mMainVolumeStepParam.getParam(mainVolume))
        {
            am_mainVolume_t actualMainVol=0;
            mpElement->getMainVolume(actualMainVol);
            actualMainVol += mainVolume;
            mMainVolumeParam.setParam(actualMainVol);
            pRequestedElement->setParam(ACTION_PARAM_MAIN_VOLUME, &mMainVolumeParam);
        }
    }
    if (true == mVolumeParam.getParam(volume))
    {
        pRequestedElement->setParam(ACTION_PARAM_VOLUME, &mVolumeParam);
    }
    if (true == mVolumeStepParam.getParam(volume))
    {
        am_volume_t actualVolume=0;
        mpElement->getVolume(actualVolume);
        actualVolume += volume;
        mVolumeParam.setParam(actualVolume);
        pRequestedElement->setParam(ACTION_PARAM_VOLUME, &mVolumeParam);
    }

    return E_OK;
}

}
/* namespace gc */
} /* namespace am */
