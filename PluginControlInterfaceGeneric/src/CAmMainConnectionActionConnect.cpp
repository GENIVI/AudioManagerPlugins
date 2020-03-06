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
#include <algorithm>
#include "CAmMainConnectionActionConnect.h"
#include "CAmMainConnectionElement.h"
#include "CAmSourceElement.h"
#include "CAmLogger.h"
#include "CAmTriggerQueue.h"
#include "CAmRouteActionConnect.h"
#include "CAmSourceActionSetState.h"


namespace am {
namespace gc {

CAmMainConnectionActionConnect::CAmMainConnectionActionConnect(
    std::shared_ptr<CAmMainConnectionElement > pMainConnection)
    : CAmActionContainer(std::string("CAmMainConnectionActionConnect"))
    , mpMainConnection(pMainConnection)
{
    this->_registerParam(ACTION_PARAM_CONNECTION_FORMAT, &mConnectionFormatParam);
    this->_registerParam(ACTION_PARAM_SET_SOURCE_STATE_DIRECTION, &mSetSourceStateDirectionParam);

    if (mpMainConnection)
    {
        switch (mpMainConnection->getState())
        {
            // target reached or in transition
            case CS_CONNECTED:
            case CS_DISCONNECTING:
            case CS_CONNECTING:
                break;

            // starting points
            case CS_UNKNOWN:
            case CS_DISCONNECTED:
            case CS_SUSPENDED:
                mpMainConnection->setState(CS_CONNECTING);
                mpMainConnection->setStateChangeTrigger((am_Error_e)getError());
                break;
        }
        mpMainConnection->registerTransitionAction(this);
    }
}

CAmMainConnectionActionConnect::~CAmMainConnectionActionConnect()
{
}

int CAmMainConnectionActionConnect::_execute(void)
{
    if (nullptr == mpMainConnection)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "  Parameters not set.");
        return E_NOT_POSSIBLE;
    }

    LOG_FN_INFO(__FILENAME__, __func__, mpMainConnection->getName(), mpMainConnection->getState());

    am_ConnectionState_e state = mpMainConnection->getState();
    if (CS_CONNECTED == state)
    {
        return E_OK; // already connected
    }

    if (mpMainConnection->getRouteAvailability() != A_AVAILABLE)
    {
        return E_NOT_POSSIBLE;
    }

    std::vector<std::shared_ptr<CAmRouteElement > >           listRouteElements;
    std::vector<std::shared_ptr<CAmRouteElement > >::iterator itListRouteElements;
    mpMainConnection->getListRouteElements(listRouteElements);
    gc_SetSourceStateDirection_e setSourceStateDirection = SD_MAINSINK_TO_MAINSOURCE;
    mSetSourceStateDirectionParam.getParam(setSourceStateDirection);
    if (setSourceStateDirection == SD_MAINSINK_TO_MAINSOURCE)
    {
        // reverse the vector
        std::reverse(listRouteElements.begin(), listRouteElements.end());
    }

    if (E_OK != _createListActionsRouteConnect(listRouteElements))
    {
        return E_NOT_POSSIBLE;
    }

    if (E_OK != _createListActionsSetSourceState(listRouteElements, SS_ON))
    {
        return E_NOT_POSSIBLE;
    }

    if (state != CS_CONNECTING)
    {
        mpMainConnection->setState(CS_CONNECTING);
        mpMainConnection->setStateChangeTrigger(static_cast<am_Error_e>(getError()));
    }
    return E_OK;
}

int CAmMainConnectionActionConnect::_undo(void)
{
    // update main connection state in Audio Manager daemon
    mpMainConnection->setState(CS_DISCONNECTING);
    mpMainConnection->setStateChangeTrigger(static_cast<am_Error_e>(getError()));

    return E_OK;
}

int CAmMainConnectionActionConnect::_update(const int result)
{
    am_ConnectionState_e connectionState(CS_UNKNOWN);
    if ((E_OK == result) && (AS_COMPLETED == getStatus()))
    {
        am_SourceState_e sourceState = mpMainConnection->getMainSource()->getState();
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

    if (connectionState != CS_UNKNOWN)
    {
        // update main connection state in Audio Manager daemon
        mpMainConnection->setState(connectionState);
        mpMainConnection->updateMainVolume();
        mpMainConnection->setStateChangeTrigger(static_cast<am_Error_e>(getError()));

        mpMainConnection->unregisterTransitionAction(this);
    }

    return E_OK;
}

am_Error_e CAmMainConnectionActionConnect::_createListActionsRouteConnect(
    std::vector<std::shared_ptr<CAmRouteElement > > &listRouteElements)
{
    am_Error_e error = E_OK;
    for (auto itListRouteElements : listRouteElements)
    {
        error = _createActionRouteConnect(itListRouteElements);
        if (error != E_OK)
        {
            break;
        }
    }

    return error;
}

am_Error_e CAmMainConnectionActionConnect::_createActionRouteConnect(std::shared_ptr<CAmRouteElement > routeElement)
{
    // create router connect action for each element'
    IAmActionCommand *pAction = new CAmRouteActionConnect(routeElement);
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

    return E_OK;
}

am_Error_e CAmMainConnectionActionConnect::_createListActionsSetSourceState(std::vector<std::shared_ptr<CAmRouteElement > > &listRouteElements, const am_SourceState_e requestedSourceState)
{
    am_Error_e                               error = E_OK;
    std::vector<CAmRouteElement *>::iterator itListRouteElements;
    for (auto itListRouteElements : listRouteElements)
    {
        error = _createActionSetSourceState(itListRouteElements->getSource(), requestedSourceState);
        if (error != E_OK)
        {
            break;
        }
    }

    return error;
}

am_Error_e CAmMainConnectionActionConnect::_createActionSetSourceState(std::shared_ptr<CAmSourceElement > pSource, const am_SourceState_e requestedSourceState)
{
    am_Error_e        error = E_NOT_POSSIBLE;
    IAmActionCommand *pAction(NULL);
    if (pSource != nullptr)
    {
        am_SourceState_e sourceState = pSource->getState();
        if (static_cast<am_SourceState_e>(sourceState) != SS_UNKNNOWN)
        {
            pAction = new CAmSourceActionSetState(pSource);
            if (pAction != NULL)
            {
                CAmActionParam<am_SourceState_e> sourceStateParam(requestedSourceState);
                pAction->setParam(ACTION_PARAM_SOURCE_STATE, &sourceStateParam);

                pAction->setUndoRequried(getUndoRequired());
                append(pAction);
                error = E_OK;
            }
        }
        else
        {
            error = E_OK;
        }
    }

    return error;
}

} /* namespace gc */
} /* namespace am */
