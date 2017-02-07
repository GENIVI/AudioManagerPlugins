/******************************************************************************
 * @file: CAmClassElement.cpp
 *
 * This file contains the definition of class element (member functions
 * and data members) used as data container to store the information related to
 * class as maintained by controller.
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
#include "CAmClassElement.h"
#include "CAmControlReceive.h"
#include "CAmMainConnectionElement.h"
#include "CAmGatewayElement.h"
#include "CAmLogger.h"
#include "CAmSinkElement.h"
#include "CAmSourceElement.h"

namespace am {
namespace gc {

CAmClassElement::CAmClassElement(const gc_Class_s& gcClass, CAmControlReceive* pControlReceive) :
                                CAmElement(gcClass.name, pControlReceive),
                                mpControlReceive(pControlReceive),
                                mClass(gcClass)
{
    std::vector<gc_TopologyElement_s >::iterator itListTopologyElements;
    for (itListTopologyElements = mClass.listTopologies.begin();
                    itListTopologyElements != mClass.listTopologies.end(); ++itListTopologyElements)
    {
        if ((*itListTopologyElements).codeID == MC_SINK_ELEMENT)
        {
            mListOwnedSinkElements.push_back((*itListTopologyElements).name);
        }
        if ((*itListTopologyElements).codeID == MC_SOURCE_ELEMENT)
        {
            mListOwnedSourceElements.push_back((*itListTopologyElements).name);
        }
        if ((*itListTopologyElements).codeID == MC_GATEWAY_ELEMENT)
        {
            mListOwnedGatewayElements.push_back((*itListTopologyElements).name);
        }
    }
}

CAmClassElement::~CAmClassElement()
{
}

bool CAmClassElement::isSourceSinkPairBelongtoClass(const std::string& sinkName,
                                                    const std::string& sourceName)
{
    return (isSourceBelongtoClass(sourceName) && isSinkBelongtoClass(sinkName));
}

void CAmClassElement::disposeConnection(const am_mainConnectionID_t mainConnectionID)
{
    std::vector<CAmMainConnectionElement* >::iterator itListMainConnections;
    CAmMainConnectionElement* pMainConnnection = CAmMainConnectionFactory::getElement(
                    mainConnectionID);
    if (pMainConnnection != NULL)
    {
        CAmMainConnectionFactory::destroyElement(mainConnectionID);
    }
    for (itListMainConnections = mListMainConnections.begin();
                    itListMainConnections != mListMainConnections.end(); ++itListMainConnections)
    {
        if (*itListMainConnections == pMainConnnection)
        {
            break;
        }
    }
    if (itListMainConnections != mListMainConnections.end())
    {
        mListMainConnections.erase(itListMainConnections);
    }
}

CAmMainConnectionElement* CAmClassElement::getMainConnection(
                const std::string& sourceName, const std::string& sinkName,
                std::vector<am_ConnectionState_e >& listConnectionStates, const gc_Order_e order)
{
    std::vector<CAmMainConnectionElement* > listMainConnections;
    std::vector<CAmMainConnectionElement* >::iterator itListMainConnections;
    std::vector<CAmMainConnectionElement* >::reverse_iterator itListReverseMainConnections;
    CAmMainConnectionElement* pMainConnection = NULL;
    CAmConnectionListFilter filterObject;
    filterObject.setSourceName(sourceName);
    filterObject.setSinkName(sinkName);
    filterObject.setListConnectionStates(listConnectionStates);
    getListMainConnections(listMainConnections,filterObject);
    int32_t tempPriority = 0;
    int32_t tempPriority1;
    switch (order)
    {
    case O_HIGH_PRIORITY:
        for (itListMainConnections = listMainConnections.begin();
                        itListMainConnections != listMainConnections.end(); itListMainConnections++)
        {
            (*itListMainConnections)->getPriority(tempPriority1);
            // store the first connection pointer found in queue which is in given state
            if (pMainConnection == NULL)
            {
                LOG_FN_DEBUG("  connection found with ID:", (*itListMainConnections)->getID());
                pMainConnection = *itListMainConnections;
                pMainConnection->getPriority(tempPriority);
            }
            else if (tempPriority > tempPriority1) // update connection pointer if higher priority connection is found with given state.  Lower number means higher priority.
            {
                pMainConnection = *itListMainConnections;
                pMainConnection->getPriority(tempPriority);
                LOG_FN_DEBUG("  higher priority connection found with ID:Priority:",
                             (*itListMainConnections)->getID(), tempPriority);
            }
        }
        break;
    case O_LOW_PRIORITY:
        for (itListMainConnections = listMainConnections.begin();
                        itListMainConnections != listMainConnections.end(); itListMainConnections++)
        {

            (*itListMainConnections)->getPriority(tempPriority1);
            // store the first connection pointer found in queue which is in given state
            if (pMainConnection == NULL)
            {
                LOG_FN_DEBUG("  connection found with ID:", (*itListMainConnections)->getID());
                pMainConnection = *itListMainConnections;
                pMainConnection->getPriority(tempPriority);
            }
            else if (tempPriority < tempPriority1) // update connection pointer if higher priority connection is found with given state. Higher number means lower priority
            {
                LOG_FN_DEBUG("  lower priority connection found with ID:",
                             (*itListMainConnections)->getID());
                pMainConnection = *itListMainConnections;
                pMainConnection->getPriority(tempPriority);
            }
        }
        break;
    case O_NEWEST:
        itListReverseMainConnections = listMainConnections.rbegin();
        if (itListReverseMainConnections != listMainConnections.rend())
        {
            pMainConnection = *itListReverseMainConnections;
        }
        break;
    case O_OLDEST:
        LOG_FN_DEBUG("Number of connections =", mListMainConnections.size());
        itListMainConnections = listMainConnections.begin();
        if (itListMainConnections != listMainConnections.end())
        {
            pMainConnection = *itListMainConnections;
        }
        break;
    default:
        break;
    }
    LOG_FN_EXIT();
    return pMainConnection;
}

void CAmClassElement::getListMainConnections(std::vector<CAmMainConnectionElement* >& listMainConnections,
                                             const CAmConnectionListFilter& fobject)
{
    listMainConnections = std::for_each(mListMainConnections.begin(), mListMainConnections.end(),
                                        fobject).getListMainConnection();
}

am_Error_e CAmClassElement::createMainConnection(const std::string& sourceName,
                                                 const std::string& sinkName,
                                                 am_mainConnectionID_t& mainConnectionID)
{
    CAmMainConnectionElement* pMainConnection = NULL;
    gc_Route_s route;
    am_Error_e result = E_NOT_POSSIBLE;
    CAmSourceElement* pSourceElement;
    CAmSinkElement* pSinkElement;
    std::vector < am_Route_s > listRoutes;

    pSourceElement = CAmSourceFactory::getElement(sourceName);
    pSinkElement = CAmSinkFactory::getElement(sinkName);
    if ((pSourceElement == NULL) || (pSinkElement == NULL))
    {
        LOG_FN_ERROR("Source or sink doesn't exist");
        return E_NOT_POSSIBLE;
    }
    pMainConnection = CAmMainConnectionFactory::getElement(sourceName + ":" + sinkName);
    if (pMainConnection != NULL)
    {
        result = E_ALREADY_EXISTS;
    }
    else
    {
        /*
         * 1. get the route from daemon
         * 2. get the route from topology
         * 3. validate if the route from topology exist in the list retrieved from daemon
         * 4. if the route is present then use that route for connection creation.
         */
        result = mpControlReceive->getRoute(false, pSourceElement->getID(), pSinkElement->getID(),
                                            listRoutes);
        if (result != E_OK)
        {
            LOG_FN_ERROR("getting route list from daemon failed");
        }
        else
        {
            result = _getRoute(sourceName, sinkName, route);
            if (result != E_OK)
            {
                LOG_FN_ERROR(getName(), "failed to get route");
            }
            else
            {
                result = _validateRouteFromTopology(listRoutes, route);
            }
        }
        if (result == E_OK)
        {
            // create main connection object
            pMainConnection = CAmMainConnectionFactory::createElement(route, mpControlReceive);
        }
    }
    if (pMainConnection != NULL)
    {
        // set mainConnectionID to return parameter
        mainConnectionID = pMainConnection->getID();
        pushMainConnectionInQueue(pMainConnection);
        result = E_OK;
    }
    return result;
}

am_Error_e CAmClassElement::_validateRouteFromTopology(std::vector<am_Route_s >& listRoutes,
                                                       gc_Route_s& topologyRoute)
{
    am_Error_e result = E_NOT_POSSIBLE;
    std::vector<am_Route_s >::iterator itListRoutes;
    std::vector<am_RoutingElement_s >::iterator itdaemonRoute;
    std::vector<am_RoutingElement_s >::reverse_iterator ittopologyRoute;
    LOG_FN_ENTRY(listRoutes.size());
    for (itListRoutes = listRoutes.begin(); itListRoutes != listRoutes.end(); ++itListRoutes)
    {
        if (itListRoutes->route.size() != topologyRoute.route.size())
        {
            LOG_FN_DEBUG("routes size don't match", itListRoutes->route.size(), "=",
                         topologyRoute.route.size());
            continue;
        }
        for (itdaemonRoute = itListRoutes->route.begin(), ittopologyRoute = topologyRoute.route.rbegin();
                        itdaemonRoute != itListRoutes->route.end();
                        ++itdaemonRoute, ++ittopologyRoute)
        {
            LOG_FN_INFO("checking next element");
            if ((itdaemonRoute->sinkID != ittopologyRoute->sinkID) || (itdaemonRoute->sourceID
                            != ittopologyRoute->sourceID))
            {
                break;
            }
        }
        if (itdaemonRoute == itListRoutes->route.end())
        {
            topologyRoute.sinkID = itListRoutes->sinkID;
            topologyRoute.sourceID = itListRoutes->sourceID;
            topologyRoute.route = itListRoutes->route;
            result = E_OK;
            break;
        }
    }
    return result;
}
am_Error_e CAmClassElement::setLimitState(const gc_LimitState_e limitState,
                                          const gc_LimitVolume_s& limitVolume,
                                          const uint32_t pattern)
{
    /*
     * Add the volume limit in the map, in case if the same pattern is already present
     * overwrite the limit.
     */
    if (limitState == LS_LIMITED)
    {
        mMapLimitVolumes[pattern] = limitVolume;
        LOG_FN_DEBUG("ADDED", limitVolume.limitType, limitVolume.limitVolume);
    }
    else if (limitState == LS_UNLIMITED)
    {
        /*
         * The pattern is a bit mask to clear the volume limit, clear the entries
         * which match the pattern
         */
        std::map<uint32_t, gc_LimitVolume_s >::iterator itMapLimitVolumes(mMapLimitVolumes.begin());
        for (; itMapLimitVolumes != mMapLimitVolumes.end();)
        {
            if ((itMapLimitVolumes->first & (~pattern)) == 0)
            {
                itMapLimitVolumes = mMapLimitVolumes.erase(itMapLimitVolumes);
                LOG_FN_DEBUG("ERASED");
            }
            else
            {
                ++itMapLimitVolumes;
            }
        }
    }
    return E_OK;
}

gc_LimitState_e CAmClassElement::getLimitState(void)
{
    return mMapLimitVolumes.empty() ? LS_UNLIMITED : LS_LIMITED;
}

am_Error_e CAmClassElement::getClassLimitVolume(const am_volume_t volume,
                                                gc_LimitVolume_s& limitVolume)
{
    limitVolume.limitType = LT_UNKNOWN;
    limitVolume.limitVolume = 0;

    am_volume_t current(0);
    am_volume_t lowest(volume);
    std::map<uint32_t, gc_LimitVolume_s >::iterator itMapLimitVolumes(mMapLimitVolumes.begin());
    for (; itMapLimitVolumes != mMapLimitVolumes.end(); ++itMapLimitVolumes)
    {
        if (itMapLimitVolumes->second.limitType == LT_ABSOLUTE)
        {
            current = itMapLimitVolumes->second.limitVolume;
            if (itMapLimitVolumes->second.limitVolume == AM_MUTE)
            {
                // in case of mute, no comparison is required.
                limitVolume = itMapLimitVolumes->second;
                break;
            }
        }
        else
        {
            current = volume + itMapLimitVolumes->second.limitVolume;
        }
        if (lowest > current)
        {
            lowest = current;
            limitVolume = itMapLimitVolumes->second;
        }
    }
    return E_OK;
}

void CAmClassElement::getListMainConnections(
                std::vector<CAmMainConnectionElement* >& listConnections)
{
    listConnections = mListMainConnections;
}

am_Error_e CAmClassElement::_getRoute(const std::string& mainSourceName,
                                      const std::string& mainSinkName, gc_Route_s& route)
{
    CAmSinkElement *pMainSinkElement, *pSinkElement;
    CAmSourceElement *pMainSourceElement, *pSourceElement;
    am_RoutingElement_s routingElement;
    CAmGatewayElement* pGatewayElement;
    std::vector<gc_TopologyElement_s >::iterator itListTopologyElements;
    int gatewayFoundCounter = 0;
    bool sourceFound = false;

    LOG_FN_ENTRY(mainSourceName, mainSinkName);
    if (isSourceSinkPairBelongtoClass(mainSinkName, mainSourceName) == 0)
    {
        return E_UNKNOWN;
    }
    //search Main sink ID
    pMainSinkElement = CAmSinkFactory::getElement(mainSinkName);
    pMainSourceElement = CAmSourceFactory::getElement(mainSourceName);
    // check if sink element is found
    if ((NULL == pMainSourceElement) || (NULL == pMainSinkElement))
    {
        LOG_FN_ERROR(" Not able to get source and sink");
        return E_UNKNOWN;
    }
    route.sinkID = pMainSinkElement->getID();
    route.sourceID = pMainSourceElement->getID();
    route.name = pMainSourceElement->getName() + ":" + pMainSinkElement->getName();
    // This field would be overwritten during topology validation.
    routingElement.connectionFormat = 0;
    routingElement.domainID = pMainSinkElement->getDomainID();
    routingElement.sinkID = pMainSinkElement->getID();
    //search for gateway and source element
    for (itListTopologyElements = mClass.listTopologies.begin();
                    itListTopologyElements != mClass.listTopologies.end(); itListTopologyElements++)
    {
        // if topology element is source
        if (MC_SOURCE_ELEMENT == itListTopologyElements->codeID)
        {
            // get the source element by name
            pSourceElement = CAmSourceFactory::getElement(itListTopologyElements->name);
            if (NULL == pSourceElement)
            {
                continue;
            }
            //check if source is as per request
            if (mainSourceName == pSourceElement->getName())
            {
                routingElement.sourceID = pMainSourceElement->getID();
                route.route.push_back(routingElement);
                sourceFound = true;
                break;
            }

        }
        //if topology element is gateway
        else if (MC_GATEWAY_ELEMENT == itListTopologyElements->codeID)
        {
            // get the gateway element by name
            pGatewayElement = CAmGatewayFactory::getElement(itListTopologyElements->name);
            if (NULL == pGatewayElement)
            {
                /*
                 * Gateway does not exist. source is not found. so this gateway not
                 * required in this connection
                 *  delete the source ID, sink ID and gateway element from vector
                 */
                for (int i = 0; i < gatewayFoundCounter; i++)
                {
                    //remove sink ID
                    routingElement = route.route.back();
                    route.route.pop_back();
                }
                gatewayFoundCounter++;
                int loopCount = 0;
                while (loopCount < gatewayFoundCounter)
                {
                    itListTopologyElements++;
                    if (itListTopologyElements == mClass.listTopologies.end())
                    {
                        break;
                    }
                    while (MC_RBRACKET_CODE != itListTopologyElements->codeID)
                    {
                        if (MC_GATEWAY_ELEMENT == itListTopologyElements->codeID)
                        {
                            gatewayFoundCounter++;
                        }
                        itListTopologyElements++;
                        if (itListTopologyElements == mClass.listTopologies.end())
                        {
                            break;
                        }
                    }

                    loopCount++;
                }
                gatewayFoundCounter = 0;
                continue;
            }
            // increment the gatewayfound counter
            gatewayFoundCounter++;
            //get the source and sink ID of gateway
            routingElement.sourceID = pGatewayElement->getSourceID();
            route.route.push_back(routingElement);
            routingElement.sinkID = pGatewayElement->getSinkID();
            routingElement.domainID = pGatewayElement->getSinkDomainID();
        }
        // if topology element is )
        else if (MC_RBRACKET_CODE == itListTopologyElements->codeID)
        {
            // source is not found. so this gateway not required in this connection
            // delete the source ID, sink ID and gateway element from vector
            for (int loopCount = 0; loopCount < gatewayFoundCounter; loopCount++)
            {
                routingElement = route.route.back();
                route.route.pop_back();
            }
            gatewayFoundCounter = 0;
        }
    }
    // if source is found then form the pair of source and sink to be connected
    return (sourceFound == true) ? E_OK : E_UNKNOWN;
}

void CAmClassElement::updateMainConnectionQueue(void)
{
    std::vector<CAmMainConnectionElement* >::iterator itListMainConnections;
    CAmMainConnectionElement* pMainConnection;
    int state;
    /**
     * First find the iterator to the connected connection
     */
    for (itListMainConnections = mListMainConnections.begin();
                    itListMainConnections != mListMainConnections.end(); ++itListMainConnections)
    {
        (*itListMainConnections)->getState(state);
        if (state == CS_CONNECTED)
        {
            break;
        }
    }
    if (itListMainConnections != mListMainConnections.end())
    {
        pMainConnection = *itListMainConnections;
        mListMainConnections.erase(itListMainConnections);
        mListMainConnections.push_back(pMainConnection);
    }
}

void CAmClassElement::pushMainConnectionInQueue(CAmMainConnectionElement* pMainConnection)
{
    std::vector<CAmMainConnectionElement* >::iterator itListMainConnection;
    itListMainConnection = std::find(mListMainConnections.begin(), mListMainConnections.end(),
                                     pMainConnection);
    if (itListMainConnection != mListMainConnections.end())
    {
        mListMainConnections.erase(itListMainConnection);
    }
    mListMainConnections.push_back(pMainConnection);
    updateMainConnectionQueue();
}

am_Error_e CAmClassElement::_register(void)
{
    am_SourceClass_s sourceClassInstance;
    sourceClassInstance.sourceClassID = 0;
    sourceClassInstance.name = mClass.name;

    // store source class in DB
    if (E_OK == mpControlReceive->enterSourceClassDB(sourceClassInstance.sourceClassID,
            sourceClassInstance))
    {
        if (mClass.type == C_PLAYBACK)
        {
            setID(sourceClassInstance.sourceClassID);
        }
    }
    else
    {
        LOG_FN_ERROR(" Error while registering source Class");
        return E_DATABASE_ERROR;
    }
    am_SinkClass_s sinkClassInstance;
    sinkClassInstance.sinkClassID = 0;
    sinkClassInstance.name = mClass.name;

    // store sink class in DB
    if (E_OK == mpControlReceive->enterSinkClassDB(sinkClassInstance,
            sinkClassInstance.sinkClassID))
    {
        if (mClass.type == C_CAPTURE)
        {
            setID(sinkClassInstance.sinkClassID);
        }
    }
    else
    {
        LOG_FN_ERROR(" Error while registering source Class");
        return E_DATABASE_ERROR;
    }
    return E_OK;

}

am_Error_e CAmClassElement::_unregister(void)
{
	if (mClass.type == C_PLAYBACK)
	{
		mpControlReceive->removeSourceClassDB(getID());
	}
	else if (mClass.type == C_CAPTURE)
	{
		mpControlReceive->removeSinkClassDB(getID());
	}
    return E_OK;
}
	
	bool CAmClassElement::isSourceBelongtoClass(const std::string& sourceName)
{
    std::vector<std::string >::iterator itListOwnedSourceElements;
    //check if source belong to this class element
    itListOwnedSourceElements = find(mListOwnedSourceElements.begin(),
                                     mListOwnedSourceElements.end(), sourceName);
    return (itListOwnedSourceElements != mListOwnedSourceElements.end());
}
bool CAmClassElement::isSinkBelongtoClass(const std::string& sinkName)
{
    std::vector<std::string >::iterator itListOwnedSinkElements;
    //check if source belong to this class element
    itListOwnedSinkElements = find(mListOwnedSinkElements.begin(), mListOwnedSinkElements.end(),
                                   sinkName);
    return (itListOwnedSinkElements != mListOwnedSinkElements.end());

}

CAmClassElement* CAmClassFactory::getElement(const std::string sourceName,
                                             const std::string sinkName)
{
    CAmClassElement* pClassElement = NULL;
    std::vector<CAmClassElement* > listElements;
    std::vector<CAmClassElement* >::iterator itListElements;
    getListElements(listElements);
    for (itListElements = listElements.begin(); itListElements != listElements.end();
                    ++itListElements)
    {
        if ((*itListElements)->isSourceSinkPairBelongtoClass(sinkName, sourceName))
        {
            pClassElement = *itListElements;
        }
    }
    return pClassElement;
}

void CAmClassFactory::getElementsBySource(const std::string sourceName,
                                          std::vector<CAmClassElement* >& listClasses)
{
    std::vector<CAmClassElement* > listElements;
    std::vector<CAmClassElement* >::iterator itListElements;
    getListElements(listElements);
    for (itListElements = listElements.begin(); itListElements != listElements.end();
                    ++itListElements)
    {
        if ((*itListElements)->isSourceBelongtoClass(sourceName))
        {
            listClasses.push_back(*itListElements);
        }
    }
}

void CAmClassFactory::getElementsBySink(const std::string sinkName,
                                        std::vector<CAmClassElement* >& listClasses)
{
    std::vector<CAmClassElement* > listElements;
    std::vector<CAmClassElement* >::iterator itListElements;
    getListElements(listElements);
    for (itListElements = listElements.begin(); itListElements != listElements.end();
                    ++itListElements)
    {
        if ((*itListElements)->isSinkBelongtoClass(sinkName))
        {
            listClasses.push_back(*itListElements);
        }
    }
}

CAmClassElement* CAmClassFactory::getElementByConnection(const std::string& connectionName)
{
    CAmClassElement* pClassElement = NULL;
    std::vector<CAmClassElement* > listElements;
    std::vector<CAmClassElement* >::iterator itListElements;
    bool found = false;
    getListElements(listElements);
    for (itListElements = listElements.begin(); itListElements != listElements.end();
                    ++itListElements)
    {
        std::vector<CAmMainConnectionElement* >::iterator itListMainConnections;
        std::vector<CAmMainConnectionElement* > listMainConnections;
        (*itListElements)->getListMainConnections(listMainConnections);
        for (itListMainConnections = listMainConnections.begin();
                        itListMainConnections != listMainConnections.end(); ++itListMainConnections)
        {
            if ((*itListMainConnections)->getName() == connectionName)
            {
                found = true;
                break;
            }
        }
        if (found == true)
        {
            break;
        }
    }
    if (itListElements != listElements.end())
    {
        pClassElement = *itListElements;
    }
    return pClassElement;
}

am_MuteState_e CAmClassElement::getMuteState() const
{
    am_MuteState_e muteState = MS_UNMUTED;
    std::map<uint32_t, gc_LimitVolume_s >::const_iterator itMapLimitVolumes;
    for (itMapLimitVolumes = mMapLimitVolumes.begin(); itMapLimitVolumes != mMapLimitVolumes.end();
                    ++itMapLimitVolumes)
    {
        if ((itMapLimitVolumes->second.limitType == LT_ABSOLUTE) && ((itMapLimitVolumes->second.limitVolume
                        == AM_MUTE)))
        {
            muteState = MS_MUTED;
            break;
        }
    }
    return muteState;
}

CAmConnectionListFilter::CAmConnectionListFilter() :
                                mSourceName(""),
                                mSinkName("")
{
    mListConnectionStates.clear();
    mListExceptSinks.clear();
    mListExceptSources.clear();
}
void CAmConnectionListFilter::setSourceName(std::string sourceName)
{
    mSourceName = sourceName;
}

void CAmConnectionListFilter::setSinkName(std::string sinkName)
{
    mSinkName = sinkName;
}

void CAmConnectionListFilter::setListConnectionStates(
                std::vector<am_ConnectionState_e >& listConnectionStates)
{
    mListConnectionStates = listConnectionStates;
}

void CAmConnectionListFilter::setListExceptSourceNames(std::vector<std::string >& listExceptSources)
{
    mListExceptSources = listExceptSources;
}

void CAmConnectionListFilter::setListExceptSinkNames(std::vector<std::string >& listExceptSinks)
{
    mListExceptSinks = listExceptSinks;
}

std::vector<CAmMainConnectionElement* >& CAmConnectionListFilter::getListMainConnection()
{
    return mListMainConnections;
}

void CAmConnectionListFilter::operator()(CAmMainConnectionElement* pMainConnection)
{
    if (pMainConnection == NULL)
    {
        return;
    }
    int state;
    pMainConnection->getState(state);
    std::string sourceName = pMainConnection->getMainSourceName();
    std::string sinkName = pMainConnection->getMainSinkName();
    if (_stringMatchFilter(mSourceName, sourceName) && _stringMatchFilter(mSinkName, sinkName)
        && _connetionStateFilter((am_ConnectionState_e)state)
        && _exceptionNamesFilter(mListExceptSources, sourceName)
        && _exceptionNamesFilter(mListExceptSinks, sinkName))
    {
        mListMainConnections.push_back(pMainConnection);
    }

}

bool CAmConnectionListFilter::_stringMatchFilter(std::string filterName, std::string inputName)
{
    bool result = false;
    if (filterName == "")
    {
        result = true;
    }
    else
    {
        if (filterName == inputName)
        {
            result = true;
        }
    }
    return result;
}
bool CAmConnectionListFilter::_connetionStateFilter(am_ConnectionState_e connectionState)
{
    bool result = false;
    if (std::find(mListConnectionStates.begin(), mListConnectionStates.end(), connectionState) != mListConnectionStates.end())
    {
        result = true;
    }
    return result;
}
bool CAmConnectionListFilter::_exceptionNamesFilter(std::vector<std::string >& listString,
                                                  std::string input)
{
    bool result = true;
    if (std::find(listString.begin(), listString.end(), input) != listString.end())
    {
        result = false;
    }
    return result;
}

} /* namespace gc */
} /* namespace am */
