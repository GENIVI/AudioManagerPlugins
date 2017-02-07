/******************************************************************************
 * @file: CAmMainConnectionElement.cpp
 *
 * This file contains the definition of main connection class (member functions
 * and data members) used as data container to store the information related to
 * main connection as maintained by controller.
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

#include "CAmMainConnectionElement.h"
#include "CAmControlReceive.h"
#include "CAmSinkElement.h"
#include "CAmSourceElement.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmMainConnectionElement::CAmMainConnectionElement(const gc_Route_s& route,
                                                   CAmControlReceive* pControlReceive) :
                                CAmElement(route.name, pControlReceive),
                                mpControlReceive(pControlReceive),
                                mRoute(route)
{
    setType (ET_CONNECTION);
}

CAmMainConnectionElement::~CAmMainConnectionElement()
{
    std::vector<CAmRouteElement* >::iterator itListRouteElements;
    // release route elements
    for (itListRouteElements = mListRouteElements.begin();
                    itListRouteElements != mListRouteElements.end(); itListRouteElements++)
    {
        if (NULL != (*itListRouteElements))
        {
            CAmSourceElement* pSource = (*itListRouteElements)->getSource();
            if(pSource != NULL)
            {
                pSource->setInUse(false);
            }
            CAmSinkElement* pSink = (*itListRouteElements)->getSink();
            if(pSink != NULL)
            {
                pSink->setInUse(false);
            }
            am_Error_e err = CAmRouteFactory::destroyElement((*itListRouteElements)->getName());
        }
    }
    mListRouteElements.clear();
}

std::string CAmMainConnectionElement::getMainSourceName(void) const
{
    std::string mainSourceName;
    CAmSourceElement* pMainSource = getMainSource();
    if (pMainSource != NULL)
    {
        mainSourceName = pMainSource->getName();
    }
    return mainSourceName;
}

std::string CAmMainConnectionElement::getMainSinkName(void) const
{
    std::string mainSinkName;
    CAmSinkElement* pMainSink = getMainSink();
    if (pMainSink != NULL)
    {
        mainSinkName = pMainSink->getName();
    }
    return mainSinkName;

}

CAmSinkElement* CAmMainConnectionElement::getMainSink(void) const
{
    return CAmSinkFactory::getElement(mRoute.sinkID);
}

CAmSourceElement* CAmMainConnectionElement::getMainSource(void) const
{
    return CAmSourceFactory::getElement(mRoute.sourceID);
}

am_Error_e CAmMainConnectionElement::getState(int& state) const
{
    am_MainConnection_s mainConnection;
    am_Error_e result;
    // get the connection info from database
    result = mpControlReceive->getMainConnectionInfoDB(getID(), mainConnection);
    state = mainConnection.connectionState;
    return result;
}

am_Error_e CAmMainConnectionElement::setState(const int state)
{
    int tempState;
    getState(tempState);
    if (tempState != state)
    {
        return mpControlReceive->changeMainConnectionStateDB(getID(), (am_ConnectionState_e)state);
    }
    return E_OK;
}

am_Error_e CAmMainConnectionElement::getPriority(int32_t& priority) const
{
    priority = 0;
    int32_t tempPriority;
    std::vector<CAmRouteElement* >::const_iterator itlistRouteElements;
    for (itlistRouteElements = mListRouteElements.begin();
                    itlistRouteElements != mListRouteElements.end(); ++itlistRouteElements)
    {
        (*itlistRouteElements)->getPriority(tempPriority);
        priority += tempPriority;
    }
    return E_OK;
}

am_Error_e CAmMainConnectionElement::getVolume(am_volume_t& volume) const
{
    volume=0;
    am_volume_t tempVolume = 0;
    std::vector<CAmRouteElement* >::const_iterator itlistRouteElements;
    for (itlistRouteElements = mListRouteElements.begin();
                    itlistRouteElements != mListRouteElements.end(); ++itlistRouteElements)
    {
        (*itlistRouteElements)->getVolume(tempVolume);
        volume += tempVolume;
    }
    return E_OK;
}

void CAmMainConnectionElement::getListRouteElements(
                std::vector<CAmRouteElement* >& listRouteElements) const
{
    listRouteElements = mListRouteElements;
}

void CAmMainConnectionElement::updateState()
{
    int connectionState = CS_DISCONNECTED;
    std::vector<CAmRouteElement* >::iterator itlistRouteElements;
    for (itlistRouteElements = mListRouteElements.begin();
                    itlistRouteElements != mListRouteElements.end(); ++itlistRouteElements)
    {
        (*itlistRouteElements)->getState(connectionState);
        if (connectionState != CS_CONNECTED)
        {
            break;
        }
    }
    CAmSourceElement* pMainSource = CAmSourceFactory::getElement(mRoute.sourceID);
    if (pMainSource != NULL)
    {
        am_SourceState_e sourceState;
        pMainSource->getState((int&)sourceState);
        if ((connectionState == CS_CONNECTED) && (sourceState != SS_ON))
        {
            connectionState = CS_SUSPENDED;
        }
    }
    this->setState(connectionState);
}

am_Error_e CAmMainConnectionElement::_register(void)
{
    am_Error_e result = E_DATABASE_ERROR;
    std::vector<am_RoutingElement_s >::iterator itListRoutingElements;
    for (itListRoutingElements = mRoute.route.begin(); itListRoutingElements != mRoute.route.end();
                    itListRoutingElements++)
    {
        //TODO Don't touch to source/sink directly
        CAmSourceElement* pSource = CAmSourceFactory::getElement((*itListRoutingElements).sourceID);
        CAmSinkElement* pSink = CAmSinkFactory::getElement((*itListRoutingElements).sinkID);
        if ((pSource == NULL) || (pSink == NULL))
        {
            LOG_FN_ERROR("Source or sink not found");
            return result;
        }

        gc_RoutingElement_s gcRoutingElement;
        gcRoutingElement.name = pSource->getName() + ":" + pSink->getName();
        gcRoutingElement.sourceID = (*itListRoutingElements).sourceID;
        gcRoutingElement.sinkID = (*itListRoutingElements).sinkID;
        gcRoutingElement.domainID = (*itListRoutingElements).domainID;
        gcRoutingElement.connectionFormat = (*itListRoutingElements).connectionFormat;
        CAmRouteElement* pRoutingElement = CAmRouteFactory::createElement(gcRoutingElement,
                                                                          mpControlReceive);
        if (pRoutingElement != NULL)
        {
            LOG_FN_DEBUG("sourceID SinkID", pRoutingElement->getSourceID(),
                         pRoutingElement->getSinkID());
            pSource->setInUse(true);
            pSink->setInUse(true);
            mListRouteElements.push_back(pRoutingElement);
        }
        else
        {
            return result;
        }
    }
    am_MainConnection_s mainConnection;
    am_mainConnectionID_t mainConnectionID;
    mainConnection.sourceID = mRoute.sourceID;
    mainConnection.sinkID = mRoute.sinkID;
    mainConnection.connectionState = CS_DISCONNECTED;
    mainConnection.mainConnectionID = 0;
    //register the connection with database
    if (E_OK == mpControlReceive->enterMainConnectionDB(mainConnection, mainConnectionID))
    {
        setID(mainConnectionID);
        result = E_OK;
    }
    return result;
}

am_Error_e CAmMainConnectionElement::_unregister(void)
{
    return mpControlReceive->removeMainConnectionDB(getID());
}

am_volume_t CAmMainConnectionElement::_getNewOffsetVolume(CAmElement* pElement,
                                                          am_volume_t newVolume)
{
    am_volume_t offsetVolume;
    am_volume_t newConnectionVolume;
    getVolume(newConnectionVolume);
    gc_LimitVolume_s limitVolumeInfo;
    getLimitVolume(limitVolumeInfo);

    if (pElement != NULL)
    {
        am_volume_t volume;
        pElement->getVolume(volume);
        newConnectionVolume -= volume;
        newConnectionVolume += newVolume;
    }
    if (limitVolumeInfo.limitType == LT_UNKNOWN)
    {
        offsetVolume = 0;
    }
    else if (limitVolumeInfo.limitType == LT_RELATIVE)
    {
        offsetVolume = limitVolumeInfo.limitVolume;
    }
    else
    {
        if (newConnectionVolume > limitVolumeInfo.limitVolume)
        {
            offsetVolume = limitVolumeInfo.limitVolume - newConnectionVolume;
        }
        else
        {
            offsetVolume = 0;
        }
    }
    return offsetVolume;
}

am_Error_e CAmMainConnectionElement::getLimitElement(
                CAmElement* pChangedVolumeElement, am_volume_t newVolume,
                std::vector<std::pair<CAmElement*, gc_LimitVolume_s > >& listLimitElements)
{
    std::vector<CAmRouteElement* >::iterator itListElement;
    am_volume_t elementVolume;
    CAmElement* pElementToLimit = NULL;
    CAmElement* pLimitedElement = NULL;
    gc_LimitVolume_s elementLimitVolume;
    gc_LimitVolume_s connectionLimitInfo;
    CAmSourceElement* pSourceElement;
    CAmSinkElement* pSinkElement;
    gc_LimitVolume_s limitVolume;
    /*
     * special handling for mute State needed
     */
    getLimitVolume(connectionLimitInfo);
    if ((connectionLimitInfo.limitType == LT_ABSOLUTE) && (connectionLimitInfo.limitVolume
                    == AM_MUTE))
    {
        /*
         * we can select only sink element
         */
        pSinkElement = getMainSink();
        listLimitElements.push_back(std::make_pair(pSinkElement, connectionLimitInfo));
        return E_OK;

    }

    if (connectionLimitInfo.limitType == LT_UNKNOWN)
    {
        elementLimitVolume.limitType = LT_UNKNOWN;
    }
    else
    {
        elementLimitVolume.limitType = LT_RELATIVE;
        elementLimitVolume.limitVolume = _getNewOffsetVolume(pChangedVolumeElement, newVolume);
    }

    for (itListElement = mListRouteElements.begin(); itListElement != mListRouteElements.end();
                    itListElement++)
    {
        pSourceElement = (*itListElement)->getSource();
        if ((pSourceElement != NULL) && (pSourceElement->getLimitVolume(limitVolume) == E_OK))
        {
            if (limitVolume.limitType != LT_UNKNOWN)
            {
                pLimitedElement = pSourceElement;
            }
            //check is source is used by only one connection and supports volume change
            if ((1 >= pSourceElement->getInUse()) && (true
                            == pSourceElement->isVolumeChangeSupported()))
            {
                if ((pElementToLimit == NULL) && (connectionLimitInfo.limitType != LT_UNKNOWN))
                {
                    pElementToLimit = pSourceElement;
                }
            }
        }

        pSinkElement = (*itListElement)->getSink();
        if ((pSinkElement != NULL) && (pSinkElement->getLimitVolume(limitVolume) == E_OK))
        {
            if (limitVolume.limitType != LT_UNKNOWN)
            {
                pLimitedElement = pSinkElement;
            }
            //check is sink is used by only one connection and supports volume change
            if ((1 >= pSinkElement->getInUse()) && (true == pSinkElement->isVolumeChangeSupported()))
            {
                if ((pElementToLimit == NULL) && (connectionLimitInfo.limitType != LT_UNKNOWN))
                {
                    pElementToLimit = pSinkElement;
                }
            }
        }
        if ((pElementToLimit != NULL) && (pLimitedElement != NULL))
        {
            break;
        }
    }
    /*
     * if limit is present but element is not found
     */
    if ((pElementToLimit == NULL) && (connectionLimitInfo.limitType != LT_UNKNOWN))
    {
        LOG_FN_ERROR(" not able to find any element");
        return E_NOT_POSSIBLE;
    }
    if ((pLimitedElement != NULL) && (pLimitedElement != pElementToLimit))
    {
        gc_LimitVolume_s unlimitVolumeData;
        unlimitVolumeData.limitType = LT_UNKNOWN;
        unlimitVolumeData.limitVolume = 0;
        listLimitElements.push_back(std::make_pair(pLimitedElement, unlimitVolumeData));
    }
    if (pElementToLimit != NULL)
    {
        listLimitElements.push_back(std::make_pair(pElementToLimit, elementLimitVolume));
    }
    return E_OK;
}

am_MuteState_e CAmMainConnectionElement::getMuteState() const
{
    am_MuteState_e muteState = MS_UNMUTED;
    gc_LimitVolume_s limitVolume;
    getLimitVolume(limitVolume);
    if ((limitVolume.limitType == LT_ABSOLUTE) && ((limitVolume.limitVolume == AM_MUTE)))
    {
        muteState = MS_MUTED;
    }
    return muteState;

}
} /* namespace gc */
} /* namespace am */
