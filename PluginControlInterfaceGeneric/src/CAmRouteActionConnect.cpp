/******************************************************************************
 * @file: CAmRouteActionConnect.cpp
 *
 * This file contains the definition of router action connect class (member
 * functions and data members) used to implement the logic of connection at
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

#include "CAmRouteActionConnect.h"
#include "CAmRouteElement.h"
#include "CAmHandleStore.h"
#include "CAmLogger.h"
namespace am {
namespace gc {

CAmRouteActionConnect::CAmRouteActionConnect(std::shared_ptr<CAmRouteElement > pRouteElement)
    : CAmActionCommand(std::string("CAmRouterActionConnect"))
    , mpRouteElement(pRouteElement)
    , mHandle({H_UNKNOWN, 0})
{
    LOG_FN_INFO(__FILENAME__, __func__, pRouteElement->getName());
}

CAmRouteActionConnect::~CAmRouteActionConnect()
{
}

int CAmRouteActionConnect::_execute(void)
{

    if (nullptr == mpRouteElement)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Parameters not set");
        return E_NOT_POSSIBLE;
    }

    int                  result = E_OK;
    am_connectionID_t    connectionID(0);
    am_ConnectionState_e connectionState = mpRouteElement->getState();
    if (connectionState != CS_CONNECTED)
    {
        IAmControlReceive *pControlReceive = mpRouteElement->getControlReceive();

        am_Availability_s sourceAvailability;
        am_Availability_s sinkAvailability;
        mpRouteElement->getSource()->getAvailability(sourceAvailability);
        mpRouteElement->getSink()->getAvailability(sinkAvailability);

        LOG_FN_INFO(__FILENAME__, __func__, "resources availability: source is",
            sourceAvailability.availability, "sink is", sinkAvailability.availability);

        if (sourceAvailability.availability == A_UNAVAILABLE || sinkAvailability.availability == A_UNAVAILABLE)
        {
            LOG_FN_WARN(__FILENAME__, __func__, "refused due to resources availability");
            return E_DATABASE_ERROR;
        }

        result = pControlReceive->connect(mHandle, connectionID,
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
    }

    return result;
}

int CAmRouteActionConnect::_undo(void)
{
    int                  result          = E_OK;
    am_ConnectionState_e connectionState = mpRouteElement->getState();
    if (CS_CONNECTED == connectionState)
    {
        IAmControlReceive *pControlReceive = mpRouteElement->getControlReceive();
        result = pControlReceive->disconnect(mHandle, mpRouteElement->getID());
        if (E_OK == result)
        {
            CAmHandleStore::instance().saveHandle(mHandle, this);
            mpRouteElement->setState(CS_DISCONNECTING);
            result = E_WAIT_FOR_CHILD_COMPLETION;
        }
    }

    return result;
}

int CAmRouteActionConnect::_update(const int result)
{
    if (AS_UNDO_COMPLETE == getStatus())
    {
        // Result need not be checked as, even if undo is failed nothing can be done.
        mpRouteElement->setState(CS_DISCONNECTED);
    }
    else
    {
        if ((E_OK == result) && (getStatus() == AS_COMPLETED))
        {
            mpRouteElement->setState(CS_CONNECTED);
        }
        else
        {
            mpRouteElement->setState(CS_DISCONNECTED);
        }
    }

    // unregister the observer
    CAmHandleStore::instance().clearHandle(mHandle);

    return E_OK;
}

void CAmRouteActionConnect::_timeout()
{
    mpRouteElement->getControlReceive()->abortAction(mHandle);
}

} /* namespace gc */
} /* namespace am */
