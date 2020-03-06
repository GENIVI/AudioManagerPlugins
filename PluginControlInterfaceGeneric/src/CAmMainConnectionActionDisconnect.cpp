/******************************************************************************
 * @file: CAmMainConenctionActionDisconnect.cpp
 *
 * This file contains the definition of main connection action disconnect class
 * (member functions and data members) used to implement the logic of disconnect
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
#include "CAmMainConnectionActionDisconnect.h"
#include "CAmMainConnectionElement.h"
#include "CAmSourceElement.h"
#include "CAmLogger.h"
#include "CAmTriggerQueue.h"
#include "CAmSourceActionSetState.h"
#include "CAmRouteActionDisconnect.h"

namespace am {
namespace gc {

CAmMainConnectionActionDisconnect::CAmMainConnectionActionDisconnect(
    std::shared_ptr<CAmMainConnectionElement > pMainConnection)
    : CAmActionContainer(
        std::string("CAmMainConnectionActionDisconnect"))
    , mpMainConnection(pMainConnection)
    , mActionCompleted(false)
{
    this->_registerParam(ACTION_PARAM_SET_SOURCE_STATE_DIRECTION, &mSetSourceStateDirectionParam);

    if (mpMainConnection != nullptr)
    {
        switch (mpMainConnection->getState())
        {
            // target reached or in ongoing transition
            case CS_DISCONNECTED:
            case CS_DISCONNECTING:
            case CS_CONNECTING:
                break;

            // starting points
            case CS_UNKNOWN:
            case CS_CONNECTED:
            case CS_SUSPENDED:
                mpMainConnection->setState(CS_DISCONNECTING);
                mpMainConnection->setStateChangeTrigger((am_Error_e)getError());
                break;
        }
        mpMainConnection->registerTransitionAction(this);
    }
}

CAmMainConnectionActionDisconnect::~CAmMainConnectionActionDisconnect()
{
}

int CAmMainConnectionActionDisconnect::_execute(void)
{
    if (nullptr == mpMainConnection)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "  Parameters not set.");
        return E_NOT_POSSIBLE;
    }

    am_ConnectionState_e state = mpMainConnection->getState();
    LOG_FN_INFO(__FILENAME__, __func__, mpMainConnection->getName(), "current state =", state);

    // get route elements from main connection class
    std::vector<std::shared_ptr<CAmRouteElement > > listRouteElements;
    mpMainConnection->getListRouteElements(listRouteElements);
    gc_SetSourceStateDirection_e setSourceStateDirection = SD_MAINSINK_TO_MAINSOURCE;
    mSetSourceStateDirectionParam.getParam(setSourceStateDirection);
    if (setSourceStateDirection == SD_MAINSINK_TO_MAINSOURCE)
    {
        std::reverse(listRouteElements.begin(), listRouteElements.end());
    }

    if (E_OK != _createListActionsSetSourceState(listRouteElements, SS_OFF))
    {
        return E_NOT_POSSIBLE;
    }

    if (E_OK != _createListActionsRouteDisconnect(listRouteElements))
    {
        return E_NOT_POSSIBLE;
    }

    // update main connection state in Audio Manager daemon
    if (state != CS_DISCONNECTING)
    {
        mpMainConnection->setState(CS_DISCONNECTING);
        mpMainConnection->setStateChangeTrigger((am_Error_e)getError());
    }
    return E_OK;
}

int CAmMainConnectionActionDisconnect::_undo(void)
{
    // update main connection state in Audio Manager daemon
    mpMainConnection->setState(CS_CONNECTING);
    mpMainConnection->setStateChangeTrigger((am_Error_e)getError());
    return E_OK;
}

int CAmMainConnectionActionDisconnect::_update(const int result)
{
    if (AS_COMPLETED == getStatus())
    {
        if (E_OK == result)
        {
            mActionCompleted = true;
        }

        mpMainConnection->updateState();
        mpMainConnection->setStateChangeTrigger((am_Error_e)getError());

        mpMainConnection->unregisterTransitionAction(this);
    }
    else if (AS_UNDO_COMPLETE == getStatus())
    {
        if (mActionCompleted)
        {
            mpMainConnection->updateState();
            mpMainConnection->setStateChangeTrigger((am_Error_e)getError());

            mpMainConnection->unregisterTransitionAction(this);
        }
    }

    return E_OK;
}

bool CAmMainConnectionActionDisconnect::_checkSharedRouteDisconnected(const CAmRouteElement &routeElement)
{
    std::vector< shared_ptr< CAmElement > > listElements;
    if (routeElement.getObserverCount(ET_CONNECTION, &listElements) <= 1)
    {
        return true;
    }

    for (auto it : listElements)
    {
        if (it == mpMainConnection)
        {
            // this one we are going to disconnect
            continue;
        }

        // check if route element is shared with another main connection
        const CAmMainConnectionElement                 &mainConnection = *(std::static_pointer_cast< CAmMainConnectionElement > (it));
        std::vector<std::shared_ptr<CAmRouteElement > > listRoutes;
        mainConnection.getListRouteElements(listRoutes);

        for (const auto &fromList : listRoutes)
        {
            if ((fromList->getID() == routeElement.getID()) && (fromList->getState() != CS_DISCONNECTED))
            {
                LOG_FN_DEBUG(__FILENAME__, __func__, "found shared route", fromList->getName(), fromList->getState());
                return false;
            }
        }
    }

    return true;
}

bool CAmMainConnectionActionDisconnect::_checkSharedSourceDisconnected(const CAmSourceElement &source, const CAmRouteElement &routeElement)
{
    std::vector< shared_ptr< CAmElement > > listElements;
    if (source.getObserverCount(ET_ROUTE, &listElements) <= 1)
    {
        return true;
    }

    for (auto it : listElements)
    {
        am_ConnectionState_e connectionState = (std::static_pointer_cast< CAmRouteElement > (it))->getState();
        if ((connectionState != CS_DISCONNECTED) && (it->getName() != routeElement.getName()))
        {
            return false;
        }
    }

    return true;
}

am_Error_e CAmMainConnectionActionDisconnect::_createListActionsRouteDisconnect
    (std::vector<std::shared_ptr< CAmRouteElement > > &listRouteElements)
{
    am_Error_e error = E_OK;
    for (auto itListRouteElements : listRouteElements)
    {
        /*check if domain id is not present then do not create disconnect action*/
        if (itListRouteElements->getDomainId() == 0)
        {
            LOG_FN_INFO(__FILENAME__, __func__,
                "route domain is not available, don't create action : ",
                itListRouteElements->getName());
            continue;
        }

        error = _createActionRouteDisconnect(itListRouteElements);
        if (error != E_OK)
        {
            break;
        }
    }

    return error;
}

am_Error_e CAmMainConnectionActionDisconnect::_createActionRouteDisconnect(
    std::shared_ptr<CAmRouteElement > routeElement)
{
    if (routeElement == nullptr)
    {
        return E_NOT_POSSIBLE;
    }

    if (_checkSharedRouteDisconnected(*routeElement))
    {
        IAmActionCommand *pAction = new CAmRouteActionDisconnect(routeElement);
        if (NULL != pAction)
        {
            // add the newly created route disconnect action to dynamic action
            pAction->setUndoRequried(getUndoRequired());
            append(pAction);
        }
    }

    return E_OK;
}

am_Error_e CAmMainConnectionActionDisconnect::_createListActionsSetSourceState(
    std::vector<std::shared_ptr<CAmRouteElement > > &listRouteElements,
    const am_SourceState_e requestedSourceState)
{
    am_Error_e error = E_OK;
    for (auto itListRouteElements: listRouteElements)
    {
        if (itListRouteElements == nullptr)
        {
            return E_NOT_POSSIBLE;
        }

        error = _createActionSetSourceState(itListRouteElements->getSource(), requestedSourceState, itListRouteElements);
        if (error != E_OK)
        {
            break;
        }
    }

    return error;
}

am_Error_e CAmMainConnectionActionDisconnect::_createActionSetSourceState(std::shared_ptr<CAmSourceElement > pSource,
    const am_SourceState_e requestedSourceState, std::shared_ptr<CAmRouteElement > pRouteElement)
{
    am_Error_e        error = E_NOT_POSSIBLE;
    IAmActionCommand *pAction(NULL);
    if ((pSource != nullptr) && (pRouteElement != nullptr))
    {
        /*
         * Set the source state to SS_OFF only if source is not shared and current state is not SS_UNKNOWN
         */
        am_SourceState_e sourceState = pSource->getState();
        if ((sourceState != SS_UNKNNOWN) && _checkSharedSourceDisconnected(*pSource, *pRouteElement))
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
