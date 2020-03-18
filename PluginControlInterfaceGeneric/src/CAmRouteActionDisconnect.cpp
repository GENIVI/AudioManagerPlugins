/******************************************************************************
 * @file: CAmRouteActionDisconnect.cpp
 *
 * This file contains the definition of router action disconnect class (member
 * functions and data members) used to implement the logic of disconnection at
 * router level
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

#include "CAmRouteActionDisconnect.h"
#include "CAmRouteElement.h"
#include "CAmHandleStore.h"
#include "CAmLogger.h"

namespace am {

namespace gc {

CAmRouteActionDisconnect::CAmRouteActionDisconnect(std::shared_ptr<CAmRouteElement > pRouteElement)
    : CAmActionCommand(std::string("CAmRouterActionDisconnect"))
    , mpRouteElement(pRouteElement)
    , mHandle({H_UNKNOWN, 0})
{
}

CAmRouteActionDisconnect::~CAmRouteActionDisconnect()
{
}

int CAmRouteActionDisconnect::_execute(void)
{
    if (nullptr == mpRouteElement)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Parameters not set");
        return E_NOT_POSSIBLE;
    }

    IAmControlReceive   *pControlReceive = mpRouteElement->getControlReceive();
    am_ConnectionState_e connectionState = mpRouteElement->getState();
    if ((CS_DISCONNECTED == connectionState) || (CS_UNKNOWN == connectionState))
    {
        return E_OK;
    }

    int result = pControlReceive->disconnect(mHandle, mpRouteElement->getID());
    if (E_OK == result)
    {
        CAmHandleStore::instance().saveHandle(mHandle, this);
        mpRouteElement->setState(CS_DISCONNECTING);
        result = E_WAIT_FOR_CHILD_COMPLETION;
    }

    return result;
}

int CAmRouteActionDisconnect::_undo(void)
{
    am_connectionID_t  connectionID(0);
    IAmControlReceive *pControlReceive = mpRouteElement->getControlReceive();
    int                result          = pControlReceive->connect(mHandle, connectionID,
            mpRouteElement->getConnectionFormat(),
            mpRouteElement->getSourceID(),
            mpRouteElement->getSinkID());
    if (E_OK == result)
    {
        CAmHandleStore::instance().saveHandle(mHandle, this);
        mpRouteElement->setID(connectionID);
        mpRouteElement->setState(CS_CONNECTING);
        result = E_WAIT_FOR_CHILD_COMPLETION;
    }

    return result;
}

int CAmRouteActionDisconnect::_update(const int result)
{
    if (AS_UNDO_COMPLETE == getStatus())
    {
        // Result need not be checked as, even if undo is failed nothing can be done.
        mpRouteElement->setState(CS_CONNECTED);
    }
    else
    {
        if ((E_OK == result) && (getStatus() == AS_COMPLETED))
        {
            mpRouteElement->setState(CS_DISCONNECTED);
        }
        else
        {
            mpRouteElement->setState(CS_CONNECTED);
        }
    }

    // unregister the observer
    CAmHandleStore::instance().clearHandle(mHandle);

    return E_OK;
}

void CAmRouteActionDisconnect::_timeout(void)
{
    mpRouteElement->getControlReceive()->abortAction(mHandle);
}

} /* namespace gc */
} /* namespace am */
