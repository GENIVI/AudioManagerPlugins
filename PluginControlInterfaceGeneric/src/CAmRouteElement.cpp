/******************************************************************************
 * @file: CAmRouteElement.cpp
 *
 * This file contains the definition of route element class (member functions
 * and data members) used as data container to store the information related to
 * route of connection as maintained by controller.
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

#include "CAmRouteElement.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmRouteElement::CAmRouteElement(const gc_RoutingElement_s &routingElement, IAmControlReceive *pControlReceive)
    : CAmElement(ET_ROUTE, routingElement.name, pControlReceive)
    , mRoutingElement(routingElement)
    , mState(CS_DISCONNECTED)
    , mpSinkElement(nullptr)
    , mpSourceElement(nullptr)
{
    mpSinkElement   = CAmSinkFactory::getElement(mRoutingElement.sinkID);
    mpSourceElement = CAmSourceFactory::getElement(mRoutingElement.sourceID);
}

CAmRouteElement::~CAmRouteElement()
{
}

int32_t CAmRouteElement::getPriority() const
{
    int32_t priority = 0;
    if ((mpSourceElement != nullptr) && (mpSinkElement != nullptr))
    {
        priority = mpSourceElement->getPriority() + mpSinkElement->getPriority();
    }
    else
    {
        LOG_FN_WARN(__FILENAME__, __func__, "End points invalid:", mName);
    }

    return priority;
}

am_sourceID_t CAmRouteElement::getSourceID(void) const
{
    return mRoutingElement.sourceID;
}

am_sinkID_t CAmRouteElement::getSinkID(void) const
{
    return mRoutingElement.sinkID;
}

std::shared_ptr<CAmSourceElement > CAmRouteElement::getSource(void) const
{
    return mpSourceElement;
}

std::shared_ptr<CAmSinkElement > CAmRouteElement::getSink(void) const
{
    return mpSinkElement;
}

void CAmRouteElement::setState(am_ConnectionState_e state)
{
    mState = state;
}

am_ConnectionState_e CAmRouteElement::getState() const
{
    return mState;
}

am_CustomConnectionFormat_t CAmRouteElement::getConnectionFormat(void) const
{
    return mRoutingElement.connectionFormat;
}

std::shared_ptr<CAmElement > CAmRouteElement::getElement()
{
    return CAmRouteFactory::getElement(getName());
}

am_domainID_t CAmRouteElement::getDomainId(void) const
{
    return mRoutingElement.domainID;
}

void CAmRouteElement::setDomainId(am_domainID_t domainID)
{
    mRoutingElement.domainID = domainID;
}

} /* namespace gc */
} /* namespace am */
