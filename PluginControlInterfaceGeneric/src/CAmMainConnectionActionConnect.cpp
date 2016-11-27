/******************************************************************************
 * @file: CAmMainConnectionActionConnect.cpp
 *
 * This file contains the definition of main connection action connect class
 * (member functions and data members) used to implement the logic of connect
 * at main connection level
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

#include "CAmMainConnectionActionConnect.h"
#include "CAmMainConnectionElement.h"
#include "CAmSourceElement.h"
#include "CAmSourceActionSetState.h"
#include "CAmRouteActionConnect.h"
#include "CAmLogger.h"
#include "CAmTriggerQueue.h"

namespace am {
namespace gc {

CAmMainConnectionActionConnect::CAmMainConnectionActionConnect(
                CAmMainConnectionElement* pMainConnection) :
                                CAmActionContainer(std::string("CAmMainConnectionActionConnect")),
                                mpMainConnection(pMainConnection)
{
    this->_registerParam(ACTION_PARAM_CONNECTION_FORMAT, &mConnectionFormatParam);
}

CAmMainConnectionActionConnect::~CAmMainConnectionActionConnect()
{
}

int CAmMainConnectionActionConnect::_execute(void)
{
    if (NULL == mpMainConnection)
    {
        LOG_FN_ERROR("  Parameters not set.");
        return E_NOT_POSSIBLE;
    }
    int state;
    mpMainConnection->getState(state);
    if(CS_CONNECTED == state)
    {
        return E_OK; // already connected
    }

    IAmActionCommand* pAction(NULL);
    std::vector<CAmRouteElement*> listRouteElements;
    std::vector<CAmRouteElement*>::iterator itListRouteElements;
    mpMainConnection->getListRouteElements(listRouteElements);
    for (itListRouteElements = listRouteElements.begin();
                    itListRouteElements != listRouteElements.end();
                    ++itListRouteElements)
    {
        // create router connect action for each element
        pAction = new CAmRouteActionConnect(*itListRouteElements);
        if (NULL != pAction)
        {
            am_CustomConnectionFormat_t connectionFormat;
            if (mConnectionFormatParam.getParam(connectionFormat))
            {
                pAction->setParam(ACTION_PARAM_CONNECTION_FORMAT, &mConnectionFormatParam);
            }
            pAction->setUndoRequried(getUndoRequired());
            append(pAction);
        }
    }

    /*
     * Finally append the set soruce state action
     */
    pAction = _createActionSetSourceState(mpMainConnection, SS_ON);
    if (NULL != pAction)
    {
        append(pAction);
    }
    mpMainConnection->setState(CS_CONNECTING);
    _setConnectionStateChangeTrigger();
    return E_OK;
}

int CAmMainConnectionActionConnect::_undo(void)
{
    // update main connection state in Audio Manager daemon
    mpMainConnection->setState(CS_DISCONNECTING);
    _setConnectionStateChangeTrigger();
    return E_OK;
}

int CAmMainConnectionActionConnect::_update(const int result)
{
    am_ConnectionState_e connectionState(CS_UNKNOWN);
    if ((E_OK == result) && (AS_COMPLETED == getStatus()))
    {
        am_SourceState_e sourceState;
        mpMainConnection->getMainSource()->getState((int&)sourceState);
        if (sourceState == SS_ON)
        {
            connectionState = CS_CONNECTED;
        }
        else
        {
            connectionState = CS_SUSPENDED;
        }
    }
    else if (AS_UNDO_COMPLETE == getStatus())
    {
        connectionState = CS_DISCONNECTED;
    }

    // update main connection state in Audio Manager daemon
    mpMainConnection->setState(connectionState);
    _setConnectionStateChangeTrigger();
    return E_OK;
}

IAmActionCommand* CAmMainConnectionActionConnect::_createActionSetSourceState(CAmMainConnectionElement *pMainConnection,
                                                                              const am_SourceState_e sourceState)
{
    IAmActionCommand* pAction(NULL);
    CAmSourceElement* pMainSource = pMainConnection->getMainSource();
    if (pMainSource != NULL)
    {
        pAction = new CAmSourceActionSetState(pMainSource);
        if (pAction != NULL)
        {
            CAmActionParam<am_SourceState_e> sourceStateParam(sourceState);
            pAction->setParam(ACTION_PARAM_SOURCE_STATE, &sourceStateParam);
        }
    }
    return pAction;
}

void CAmMainConnectionActionConnect::_setConnectionStateChangeTrigger(void)
{
    gc_ConnectionStateChangeTrigger_s* ptrigger = new gc_ConnectionStateChangeTrigger_s;
    ptrigger->connectionName = mpMainConnection->getName();
    mpMainConnection->getState((int&)(ptrigger->connectionState));
    ptrigger->status = (am_Error_e)getError();
    CAmTriggerQueue::getInstance()->pushTop(SYSTEM_CONNECTION_STATE_CHANGE,ptrigger);
}

} /* namespace gc */
} /* namespace am */
