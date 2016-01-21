/******************************************************************************
 * @file: CAmClassActionConnect.cpp
 *
 * This file contains the definition of user connection action connect class
 * (member functions and data members) used to implement the logic of disconnect
 * at user level
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

#include "CAmClassElement.h"
#include "CAmClassActionConnect.h"
#include "CAmMainConnectionElement.h"
#include "CAmMainConnectionActionConnect.h"
#include "CAmMainConnectionActionSetLimitState.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmClassActionConnect::CAmClassActionConnect(CAmClassElement *pClassElement) :
                                CAmActionContainer(std::string("CAmClassActionConnect")),
                                mpClassElement(pClassElement),
                                mpMainConnectionElement(NULL),
                                mOrderParam(DEFAULT_CONNECT_ORDER)

{
    this->_registerParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    this->_registerParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    this->_registerParam(ACTION_PARAM_ORDER, &mOrderParam);
    this->_registerParam(ACTION_PARAM_CONNECTION_FORMAT, &mConnectionFormatParam);
}

CAmClassActionConnect::~CAmClassActionConnect()
{
}

int CAmClassActionConnect::_execute(void)
{
    am_Error_e result;
    result = _getMainConnectionElement();
    if ((E_OK != result) && (E_ALREADY_EXISTS != result))
    {
        return result;
    }
    if (NULL != mpMainConnectionElement)
    {
        IAmActionCommand* pAction(NULL);
        if(mpClassElement->getLimitState() == LS_LIMITED)
        {
            pAction = _createActionSetLimitState(mpMainConnectionElement);
            if (NULL != pAction)
            {
                append(pAction);
            }
        }
        pAction = new CAmMainConnectionActionConnect(mpMainConnectionElement);
        if (NULL != pAction)
        {
            pAction->setParam(ACTION_PARAM_CONNECTION_FORMAT, &mConnectionFormatParam);
            pAction->setUndoRequried(true);
            append(pAction);
        }
    }
    return E_OK;
}

int CAmClassActionConnect::_update(const int result)
{
    if (this->getStatus() == AS_UNDO_COMPLETE)
    {
        if (NULL != mpMainConnectionElement)
        {
            mpClassElement->disposeConnection(mpMainConnectionElement->getID());
        }
    }
    mpClassElement->updateMainConnectionQueue();
    return E_OK;
}

am_Error_e CAmClassActionConnect::_getMainConnectionElement()
{
    std::string sourceName;
    std::string sinkName;
    mSourceNameParam.getParam(sourceName);
    mSinkNameParam.getParam(sinkName);
    if (!sourceName.empty() && !sinkName.empty())
    {
        am_mainConnectionID_t mainConnectionID;
        am_Error_e result;
        result = mpClassElement->createMainConnection(sourceName, sinkName, mainConnectionID);
        if ((E_OK != result) && (E_ALREADY_EXISTS != result))
        {
            return result;
        }
    }
    gc_Order_e order;
    mOrderParam.getParam(order);
    std::vector < am_ConnectionState_e > listConnectionStates {CS_DISCONNECTED, CS_SUSPENDED};
    mpMainConnectionElement = mpClassElement->getMainConnection(sourceName, sinkName,
                                                                listConnectionStates, order);
    return E_OK;
}

IAmActionCommand* CAmClassActionConnect::_createActionSetLimitState(
                CAmMainConnectionElement* pMainConnection)
{
    IAmActionCommand* pAction = new CAmMainConnectionActionSetLimitState(pMainConnection);
    if (NULL != pAction)
    {
        gc_LimitVolume_s limitVolume;
        am_volume_t volume;
        pMainConnection->getVolume(volume);
        mpClassElement->getClassLimitVolume(volume, limitVolume);
        pMainConnection->setLimitVolume(limitVolume);
        CAmActionParam < am_volume_t > limitVolumeParam(limitVolume.limitVolume);
        CAmActionParam < gc_LimitType_e > limitTypeParam(limitVolume.limitType);
        pAction->setParam(ACTION_PARAM_LIMIT_TYPE, &limitTypeParam);
        pAction->setParam(ACTION_PARAM_LIMIT_VOLUME, &limitVolumeParam);
        pAction->setUndoRequried(true);
    }
    return pAction;
}

}
}
