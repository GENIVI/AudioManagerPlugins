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
#include "CAmControlReceive.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmRouteElement::CAmRouteElement(const gc_RoutingElement_s& routingElement,
                                 CAmControlReceive* pControlReceive) :
                CAmElement(routingElement.name, pControlReceive),
                mRoutingElement(routingElement)
{
    setType(ET_ROUTE);
    setState(CS_DISCONNECTED);
}

CAmRouteElement::~CAmRouteElement()
{
}

std::string CAmRouteElement::getName(void) const
{
    std::string name;
    CAmElement* pSourceElement = getSource();
    CAmElement* pSinkElement = getSink();
    if ((pSourceElement != NULL) && (pSinkElement != NULL))
    {
        name = pSourceElement->getName() + ":" + pSinkElement->getName();
    }
    return name;
}

am_Error_e CAmRouteElement::getPriority(int32_t& priority) const
{
    priority = 0;
    int32_t sinkPriority(0);
    int32_t sourcePriority(0);
    CAmElement* pSourceElement = getSource();
    CAmElement* pSinkElement = getSink();
    if ((pSourceElement != NULL) && (pSinkElement != NULL))
    {
        pSourceElement->getPriority(sourcePriority);
        pSinkElement->getPriority(sinkPriority);
        priority = sourcePriority + sinkPriority;
    }
    return E_OK;
}

am_Error_e CAmRouteElement::getVolume(am_volume_t& volume) const
{
    volume=0;
    am_volume_t sinkVolume = 0;
    am_volume_t sourceVolume = 0;
    CAmElement* pSourceElement = getSource();
    CAmElement* pSinkElement = getSink();
    if ((pSourceElement != NULL) && (pSinkElement != NULL))
    {
         pSourceElement->getVolume(sourceVolume);
         pSinkElement->getVolume(sinkVolume);
         volume = sourceVolume + sinkVolume;
    }
    return E_OK;
}

am_sourceID_t CAmRouteElement::getSourceID(void) const
{
    return mRoutingElement.sourceID;
}

am_sinkID_t CAmRouteElement::getSinkID(void) const
{
    return mRoutingElement.sinkID;
}

CAmSourceElement* CAmRouteElement::getSource(void) const
{
    return CAmSourceFactory::getElement(mRoutingElement.sourceID);
}

CAmSinkElement* CAmRouteElement::getSink(void) const
{
    return CAmSinkFactory::getElement(mRoutingElement.sinkID);
}

am_CustomConnectionFormat_t CAmRouteElement::getConnectionFormat(void) const
{
    return mRoutingElement.connectionFormat;
}

}/* namespace gc */
}/* namespace am */
