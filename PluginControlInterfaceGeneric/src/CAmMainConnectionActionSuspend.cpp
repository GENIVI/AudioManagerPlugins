/******************************************************************************
 * @file: CAmMainConnectionActionSuspend.cpp
 *
 * This file contains the definition of main connection action suspend class
 * (member functions and data members) used to implement the logic of suspend
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
#include "CAmMainConnectionActionSuspend.h"
#include "CAmSourceElement.h"
#include "CAmRouteActionConnect.h"
#include "CAmLogger.h"
#include "CAmRouteActionConnect.h"
#include "CAmSourceActionSetState.h"

namespace am {
namespace gc {

CAmMainConnectionActionSuspend::CAmMainConnectionActionSuspend(
    std::shared_ptr<CAmMainConnectionElement > pMainConnection)
    : CAmActionContainer(std::string("CAmMainConnectionActionSuspend"))
    , mpMainConnection(pMainConnection)
{
    this->_registerParam(ACTION_PARAM_SET_SOURCE_STATE_DIRECTION, &mSetSourceStateDirectionParam);

    if (mpMainConnection != nullptr)
    {
        switch (mpMainConnection->getState())
        {
            // target reached or in ongoing transition
            case CS_CONNECTED:
            case CS_SUSPENDED:
            case CS_DISCONNECTING:
            case CS_CONNECTING:
                break;

            // starting points
            case CS_UNKNOWN:
            case CS_DISCONNECTED:
                mpMainConnection->setState(CS_CONNECTING);
                mpMainConnection->setStateChangeTrigger(static_cast<am_Error_e>(getError()));
                break;
        }
        mpMainConnection->registerTransitionAction(this);
    }
}

CAmMainConnectionActionSuspend::~CAmMainConnectionActionSuspend()
{
}

int CAmMainConnectionActionSuspend::_execute(void)
{
    if (nullptr == mpMainConnection)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "  Parameter not set");
        return E_NOT_POSSIBLE;
    }

    am_ConnectionState_e                            state = mpMainConnection->getState();
    std::vector<std::shared_ptr<CAmRouteElement > > listRouteElements;
    mpMainConnection->getListRouteElements(listRouteElements);
    gc_SetSourceStateDirection_e setSourceStateDirection = SD_MAINSINK_TO_MAINSOURCE;
    mSetSourceStateDirectionParam.getParam(setSourceStateDirection);
    if (setSourceStateDirection == SD_MAINSINK_TO_MAINSOURCE)
    {
        // reverse the vector
        std::reverse(listRouteElements.begin(), listRouteElements.end());
    }

    if (state == CS_DISCONNECTED || state == CS_CONNECTING)
    {
        if (_createListActionsRouteConnect(listRouteElements) != E_OK)
        {
            return E_NOT_POSSIBLE;
        }

        if (state != CS_CONNECTING)
        {
            mpMainConnection->setState(CS_CONNECTING);
            mpMainConnection->setStateChangeTrigger(static_cast<am_Error_e>(getError()));
        }
    }

    if (_createListActionsSetSourceState(listRouteElements, SS_PAUSED) != E_OK )
    {
        return E_NOT_POSSIBLE;
    }

    return E_OK;
}

int CAmMainConnectionActionSuspend::_update(const int result)
{
    ActionState_e status = getStatus();
    if ((status == AS_COMPLETED) || (status == AS_UNDO_COMPLETE))
    {
        mpMainConnection->updateState();
        mpMainConnection->setStateChangeTrigger(static_cast<am_Error_e>(getError()));

        mpMainConnection->unregisterTransitionAction(this);
    }

    return E_OK;
}

bool CAmMainConnectionActionSuspend::_sharedConnectionCheck(const CAmRouteElement &routeElement)
{
    am_SourceState_e                           sourceState = routeElement.getSource()->getState();
    std::vector<std::shared_ptr<CAmElement > > listConnections;
    int                                        count = routeElement.getObserverCount(ET_CONNECTION, &listConnections);
    if ((sourceState == SS_OFF) || (listConnections.size() <= 1))
    {
        return true;
    }

    for (auto it : listConnections)
    {
        am_ConnectionState_e connectionState = (std::static_pointer_cast< CAmMainConnectionElement > (it))->getState();
        if ((connectionState == CS_CONNECTED) && ((std::static_pointer_cast< CAmMainConnectionElement > (it))->getName() != mpMainConnection->getName()))
        {
            LOG_FN_ERROR("error");
            return false;
        }
    }

    return true;
}

am_Error_e CAmMainConnectionActionSuspend::_createListActionsRouteConnect(
    std::vector< std::shared_ptr< CAmRouteElement > > &listRouteElements)
{
    am_Error_e error = E_OK;
    for (auto elem : listRouteElements)
    {
        error = _createActionRouteConnect(elem);
        if (error != E_OK)
        {
            break;
        }
    }

    return error;
}

am_Error_e CAmMainConnectionActionSuspend::_createActionRouteConnect(
    std::shared_ptr<CAmRouteElement > routeElement)
{
    if (routeElement == nullptr)
    {
        return E_NOT_POSSIBLE;
    }

    // create router connect action for each element
    IAmActionCommand *pAction = new CAmRouteActionConnect(routeElement);
    if (NULL != pAction)
    {
        CAmActionParam<am_CustomConnectionFormat_t> connectionFormatparam(
            routeElement->getConnectionFormat());
        pAction->setParam(ACTION_PARAM_CONNECTION_FORMAT, &connectionFormatparam);
        pAction->setUndoRequried(getUndoRequired());
        append(pAction);
        return E_OK;
    }

    return E_NOT_POSSIBLE;
}

am_Error_e CAmMainConnectionActionSuspend::_createListActionsSetSourceState(std::vector< std::shared_ptr< CAmRouteElement > > &listRouteElements,
    const am_SourceState_e requestedSourceState)
{
    am_Error_e error = E_OK;
    for (auto elem : listRouteElements)
    {
        if (elem == nullptr)
        {
            error = E_NOT_POSSIBLE;
            break;
        }

        if (false == _sharedConnectionCheck(*elem))
        {
            continue;
        }

        error = _createActionSetSourceState(elem->getSource(), requestedSourceState);
        if (error != E_OK)
        {
            break;
        }
    }

    return error;
}

am_Error_e CAmMainConnectionActionSuspend::_createActionSetSourceState(
    std::shared_ptr<CAmSourceElement > pSource,
    const am_SourceState_e requestedSourceState)
{
    am_Error_e        error = E_NOT_POSSIBLE;
    IAmActionCommand *pAction(NULL);
    if (pSource != nullptr)
    {
        am_SourceState_e sourceState = pSource->getState();
        if (static_cast<am_SourceState_e>(sourceState) == SS_UNKNNOWN)
        {
            return E_OK;
        }

        if ((pSource->getObserverCount(ET_ROUTE) <= 1) || ((static_cast<am_SourceState_e>(sourceState) == SS_OFF)))
        {
            pAction = new CAmSourceActionSetState(pSource);
            if (pAction != NULL)
            {
                CAmActionParam<am_SourceState_e > sourceStateParam(requestedSourceState);
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
