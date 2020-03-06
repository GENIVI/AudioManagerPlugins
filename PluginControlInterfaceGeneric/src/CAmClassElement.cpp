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
#include <sstream>
#include "CAmClassElement.h"
#include "CAmMainConnectionElement.h"
#include "CAmGatewayElement.h"
#include "CAmLogger.h"
#include "CAmSinkElement.h"
#include "CAmSourceElement.h"
#include "CAmSystemElement.h"
#include "CAmRootAction.h"
#include "CAmPersistenceWrapper.h"


namespace am {
namespace gc {

#define CLASS_LEVEL_SINK "*"

CAmClassElement::CAmClassElement(const gc_Class_s &gcClass, IAmControlReceive *pControlReceive)
    : CAmLimitableElement(ET_CLASS, gcClass.name, pControlReceive)
    , mClass(gcClass)
    , mSourceClassID(0)
    , mSinkClassID(0)
{
    mClassLastVolume.className           = gcClass.name;
    mLastMainConnectionsVolume.className = gcClass.name;
    mLastMainSoundProperties.className   = gcClass.name;
}

CAmClassElement::~CAmClassElement()
{
}

bool CAmClassElement::isSourceSinkPairBelongtoClass(const std::string &sinkName,
    const std::string &sourceName) const
{
    bool isSourceBelongingToClass = isElementBelongtoClass(sourceName, ET_SOURCE);
    bool isSinkBelongingToClass   = isElementBelongtoClass(sinkName, ET_SINK);
    if (((true == isSourceBelongingToClass) && (true == isSinkBelongingToClass)) ||
        ((true == isSourceBelongingToClass) && (mClass.type == C_PLAYBACK)) ||
        ((true == isSinkBelongingToClass) && (mClass.type == C_CAPTURE)))
    {
        return true;
    }

    return false;

}

void CAmClassElement::disposeConnection(
    const std::shared_ptr<CAmMainConnectionElement > pMainConnection)
{
    std::vector<std::shared_ptr<CAmMainConnectionElement > >::iterator itListMainConnections;
    std::set<std::string >::iterator                                   itListLastMainConnections;

    if (nullptr != pMainConnection)
    {
        // remove link between class element and main connection element
        detach(std::static_pointer_cast < CAmElement > (pMainConnection));
        pMainConnection->removeRouteElements();
        CAmMainConnectionFactory::destroyElement(pMainConnection->getID());
    }
}

std::shared_ptr<CAmMainConnectionElement > CAmClassElement::getMainConnection(
    const std::string &sourceName, const std::string &sinkName,
    const std::vector<am_ConnectionState_e > &listConnectionStates, const gc_Order_e order)
{
    // compose filter
    CAmConnectionListFilter filterObject;
    filterObject.setSourceName(sourceName);
    filterObject.setSinkName(sinkName);
    filterObject.setClassName(mName);
    filterObject.setListConnectionStates(listConnectionStates);

    // get sorted list of applicable connections
    std::vector<std::shared_ptr<CAmMainConnectionElement > > listMainConnections;
    CAmMainConnectionFactory::getListElements(listMainConnections, filterObject, order);

    if (listMainConnections.size() > 0)
    {
        return listMainConnections[0];
    }
    else
    {
        return nullptr;
    }
}


am_Error_e CAmClassElement::createMainConnection(const std::string &sourceName,
    const std::string &sinkName,
    am_mainConnectionID_t &mainConnectionID)
{
    std::shared_ptr<CAmMainConnectionElement > pMainConnection = nullptr;
    gc_Route_s                                 route;
    am_Error_e                                 result = E_NOT_POSSIBLE;
    std::vector < am_Route_s >                 listRoutes;
    std::shared_ptr<CAmSourceElement >         pSourceElement = CAmSourceFactory::getElement(sourceName);
    std::shared_ptr<CAmSinkElement >           pSinkElement   = CAmSinkFactory::getElement(sinkName);

    if ((nullptr == pSourceElement) || (nullptr == pSinkElement))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "Source or sink doesn't exist");
        return E_NOT_POSSIBLE;
    }

    pMainConnection = CAmMainConnectionFactory::getElement(sourceName + ":" + sinkName);
    if (nullptr != pMainConnection)
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
        if ((E_OK != result) || (true == listRoutes.empty()))
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "getting route list from daemon failed");
        }
        else
        {
            std::shared_ptr<CAmSystemElement > pSystemElement = nullptr;
            pSystemElement = CAmSystemFactory::getElement(SYSTEM_ELEMENT_NAME);
            result         = _getPreferredRoute(sourceName, sinkName, route);
            if ((result != E_OK) && (pSystemElement->isNonTopologyRouteAllowed() == true))
            {
                LOG_FN_INFO(__FILENAME__, __func__, getName(), "failed to get route from topology");
                /*
                 * Preferred route not found from topology could be unknown source or sink !! select
                 * the first route from daemon.
                 */
                route.sinkID   = listRoutes.begin()->sinkID;
                route.sourceID = listRoutes.begin()->sourceID;
                route.route    = listRoutes.begin()->route;
                route.name     = sourceName + ":" + sinkName;
                result         = E_OK;
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

        if (pMainConnection != nullptr)
        {
            // attach the route elements to the main connection
            std::vector<std::shared_ptr<CAmRouteElement > >           listRouteElements;
            std::vector<std::shared_ptr<CAmRouteElement > >::iterator itListRouteElements;
            pMainConnection->getListRouteElements(listRouteElements);
            for (itListRouteElements = listRouteElements.begin();
                 itListRouteElements != listRouteElements.end(); itListRouteElements++)
            {
                pMainConnection->attach(*itListRouteElements);
                if (result != E_OK)
                {
                    // its an error need to decide
                    LOG_FN_DEBUG("route element attach to main connection failed, result is:",
                        result);
                }
            }

            result = pMainConnection->attach(pSourceElement);
            if (result != E_OK)
            {
                /*its an error need to decide */
                LOG_FN_DEBUG("source element attach to main connection failed, result is:", result);
            }

            result = pMainConnection->attach(pSinkElement);
            if (result != E_OK)
            {
                /*its an error need to decide */
                LOG_FN_DEBUG("sink element attach to main connection failed, result is:", result);

            }

            result = attach(pMainConnection);
            if (result != E_OK)
            {
                /*its an error need to decide */
                LOG_FN_DEBUG("main connection element attach to class element failed, result is:",
                    result);
            }
        }
    }

    if (nullptr != pMainConnection)
    {
        // set mainConnectionID to return parameter
        mainConnectionID = pMainConnection->getID();
        CAmMainConnectionFactory::moveToEnd(pMainConnection->getName(), O_NEWEST);
        updateMainConnectionQueue();
        result = E_OK;
    }

    return result;
}

am_Error_e CAmClassElement::_validateRouteFromTopology(std::vector<am_Route_s > &listRoutes,
    gc_Route_s &topologyRoute) const
{
    am_Error_e                                          result = E_NOT_POSSIBLE;
    std::vector<am_Route_s >::iterator                  itListRoutes;
    std::vector<am_RoutingElement_s >::iterator         itdaemonRoute;
    std::vector<am_RoutingElement_s >::reverse_iterator ittopologyRoute;
    LOG_FN_ENTRY(__FILENAME__, __func__, listRoutes.size());
    for (itListRoutes = listRoutes.begin(); itListRoutes != listRoutes.end(); ++itListRoutes)
    {
        if (itListRoutes->route.size() != topologyRoute.route.size())
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, "routes size don't match", itListRoutes->route.size(), "=", topologyRoute.route.size());
            continue;
        }

        for (itdaemonRoute = itListRoutes->route.begin(), ittopologyRoute = topologyRoute.route.rbegin();
             itdaemonRoute != itListRoutes->route.end();
             ++itdaemonRoute, ++ittopologyRoute)
        {
            LOG_FN_INFO(__FILENAME__, __func__, "checking next element");
            if ((itdaemonRoute->sinkID != ittopologyRoute->sinkID) || (itdaemonRoute->sourceID
                                                                       != ittopologyRoute->sourceID))
            {
                break;
            }
        }

        if (itdaemonRoute == itListRoutes->route.end())
        {
            topologyRoute.sinkID   = itListRoutes->sinkID;
            topologyRoute.sourceID = itListRoutes->sourceID;
            topologyRoute.route    = itListRoutes->route;
            result                 = E_OK;
            break;
        }
    }

    return result;
}

am_Error_e CAmClassElement::_checkElementPresentInTopology(const std::vector<gc_TopologyElement_s> &topology,
    const std::string &elementName,
    gc_ClassTopologyCodeID_e type) const
{
    std::vector<gc_TopologyElement_s >::const_iterator itListTopologyElements;
    for (itListTopologyElements = topology.begin();
         itListTopologyElements != topology.end(); ++itListTopologyElements)
    {
        if (type == itListTopologyElements->codeID)
        {
            if (elementName == itListTopologyElements->name)
            {
                return E_OK;
            }

            if (itListTopologyElements->name == TOPOLOGY_SYMBOL_ASTERISK)
            {
                gc_Element_e elementtype = (type == MC_SINK_ELEMENT) ? ET_SINK : ET_SOURCE;
                if (true == _checkElementInSubjectList(elementName, elementtype))
                {
                    return E_OK;
                }
            }
        }
    }

    return E_UNKNOWN;
}

am_Error_e CAmClassElement::_getRouteFromTopology(const std::vector<gc_TopologyElement_s> &topology,
    const std::string &mainSourceName,
    const std::string &mainSinkName,
    gc_Route_s &route) const
{
    am_RoutingElement_s                                routingElement;
    std::shared_ptr<CAmGatewayElement >                pGatewayElement = nullptr;
    std::vector<gc_TopologyElement_s >::const_iterator itListTopologyElements;
    int                                                gatewayFoundCounter = 0;
    bool                                               sourceFound         = false;
    std::shared_ptr<CAmSinkElement >                   pMainSinkElement    = CAmSinkFactory::getElement(mainSinkName);
    std::shared_ptr<CAmSourceElement >                 pMainSourceElement  = CAmSourceFactory::getElement(
            mainSourceName);

    if ((nullptr == pMainSourceElement) || (nullptr == pMainSinkElement))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "main source or main sink doesn't exist");
        return E_NOT_POSSIBLE;
    }

    LOG_FN_INFO(__FILENAME__, __func__, "source name : sink name", mainSourceName, mainSinkName);
    if (0 == isSourceSinkPairBelongtoClass(mainSinkName, mainSourceName))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Not able to get source and sink");
        return E_UNKNOWN;
    }

    /*
     * Verify if the sink name is present in the topology
     */
    if (E_OK != _checkElementPresentInTopology(topology, mainSinkName))
    {
        return E_NOT_POSSIBLE;
    }

    route.sinkID   = pMainSinkElement->getID();
    route.sourceID = pMainSourceElement->getID();
    route.name     = pMainSourceElement->getName() + ":" + pMainSinkElement->getName();
    // This field would be overwritten during topology validation.
    routingElement.connectionFormat = 0;
    routingElement.domainID         = pMainSinkElement->getDomainID();
    routingElement.sinkID           = pMainSinkElement->getID();
    // search for gateway and source element
    for (itListTopologyElements = topology.begin();
         itListTopologyElements != topology.end();
         itListTopologyElements++)
    {
        // if topology element is source
        if (MC_SOURCE_ELEMENT == itListTopologyElements->codeID)
        {
            // get the source element by name
            std::shared_ptr<CAmSourceElement > pSourceElement = CAmSourceFactory::getElement(
                    itListTopologyElements->name);
            if (((nullptr != pSourceElement) &&
                 (mainSourceName == pSourceElement->getName())) ||
                ((itListTopologyElements->name == TOPOLOGY_SYMBOL_ASTERISK) &&
                 (true == _checkElementInSubjectList(mainSourceName, ET_SOURCE))))
            {

                routingElement.sourceID = pMainSourceElement->getID();
                route.route.push_back(routingElement);
                sourceFound = true;
                break;
            }
        }
        // if topology element is gateway
        else if (MC_GATEWAY_ELEMENT == itListTopologyElements->codeID)
        {
            // get the gateway element by name
            pGatewayElement = CAmGatewayFactory::getElement(itListTopologyElements->name);
            if (nullptr == pGatewayElement)
            {
                /*
                 * Gateway does not exist. source is not found. so this gateway not
                 * required in this connection
                 *  delete the source ID, sink ID and gateway element from vector
                 */
                for (int i = 0; i < gatewayFoundCounter; i++)
                {
                    // remove sink ID
                    routingElement = route.route.back();
                    route.route.pop_back();
                }

                gatewayFoundCounter++;
                int loopCount = 0;
                while (loopCount < gatewayFoundCounter)
                {
                    itListTopologyElements++;
                    if (itListTopologyElements == topology.end())
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
                        if (itListTopologyElements == topology.end())
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
            // get the source and sink ID of gateway
            routingElement.sourceID = pGatewayElement->getSourceID();
            route.route.push_back(routingElement);
            routingElement.sinkID   = pGatewayElement->getSinkID();
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

am_Error_e CAmClassElement::_getPreferredRoute(const std::string &mainSourceName,
    const std::string &mainSinkName, gc_Route_s &route) const
{
    am_Error_e                                                       result = E_NOT_POSSIBLE;
    std::vector<std::vector< gc_TopologyElement_s> >::const_iterator itListTopologies;
    for (itListTopologies = mClass.listTopologies.begin();
         itListTopologies != mClass.listTopologies.end();
         ++itListTopologies)
    {
        result = _getRouteFromTopology((*itListTopologies),
                mainSourceName,
                mainSinkName,
                route);
        if (E_OK == result)
        {
            break;
        }
    }

    return result;
}

void CAmClassElement::updateMainConnectionQueue(void)
{
    /*
     * If there is connected or suspended connection change its order to
     * the top of queue
     */
    std::vector<std::shared_ptr<CAmMainConnectionElement > > listMainConnections;
    CAmConnectionListFilter filter;
    filter.setClassName(mName);
    filter.setListConnectionStates({CS_CONNECTED, CS_SUSPENDED});
    CAmMainConnectionFactory::getListElements(listMainConnections, filter);
    if (listMainConnections.size() > 0)
    {
        CAmMainConnectionFactory::moveToEnd(listMainConnections[0]->getName(), O_NEWEST);
    }
}

am_Error_e CAmClassElement::_register(void)
{
    am_SourceClass_s sourceClassInstance;
    am_SinkClass_s   sinkClassInstance;

    if (mClass.classID < DYNAMIC_ID_BOUNDARY)
    {
        sourceClassInstance.sourceClassID = mClass.classID;
        sinkClassInstance.sinkClassID     = mClass.classID;
    }
    else
    {
        sourceClassInstance.sourceClassID = 0;
        sinkClassInstance.sinkClassID     = 0;
    }

    sourceClassInstance.name                = mClass.name;
    sourceClassInstance.listClassProperties = mClass.listClassProperties;

    // store source class in DB
    if (E_OK == mpControlReceive->enterSourceClassDB(sourceClassInstance.sourceClassID,
            sourceClassInstance))
    {
        if (C_PLAYBACK == mClass.type)
        {
            setID(sourceClassInstance.sourceClassID);
        }
    }
    else
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Error while registering source Class");
        return E_DATABASE_ERROR;
    }

    sinkClassInstance.name                = mClass.name;
    sinkClassInstance.listClassProperties = mClass.listClassProperties;
    // store sink class in DB
    if (E_OK == mpControlReceive->enterSinkClassDB(sinkClassInstance,
            sinkClassInstance.sinkClassID))
    {
        if (C_CAPTURE == mClass.type)
        {
            setID(sinkClassInstance.sinkClassID);
            LOG_FN_ERROR(__FILENAME__, __func__, " Error while registering sink Class");
        }
    }
    else
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Error while registering source Class");
        return E_DATABASE_ERROR;
    }

    mSinkClassID   = sinkClassInstance.sinkClassID;
    mSourceClassID = sourceClassInstance.sourceClassID;
    return E_OK;
}

am_Error_e CAmClassElement::_unregister(void)
{
    if (C_PLAYBACK == mClass.type)
    {
        mpControlReceive->removeSourceClassDB(getID());
    }
    else if (C_CAPTURE == mClass.type)
    {
        mpControlReceive->removeSinkClassDB(getID());
    }

    return E_OK;
}

std::shared_ptr<CAmClassElement > CAmClassFactory::getElement(const std::string sourceName,
    const std::string sinkName)
{
    std::shared_ptr<CAmClassElement >                         pClassElement = nullptr;
    std::vector<std::shared_ptr<CAmClassElement > >           listElements;
    std::vector<std::shared_ptr<CAmClassElement > >::iterator itListElements;
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

void CAmClassFactory::getElementsBySource(
    const std::string sourceName,
    std::vector<std::shared_ptr<CAmClassElement > > &listClasses)
{
    std::vector<std::shared_ptr<CAmClassElement > >           listElements;
    std::vector<std::shared_ptr<CAmClassElement > >::iterator itListElements;
    getListElements(listElements);
    for (itListElements = listElements.begin(); itListElements != listElements.end();
         ++itListElements)
    {
        if ((*itListElements)->isElementBelongtoClass(sourceName, ET_SOURCE))
        {
            listClasses.push_back(*itListElements);
        }
    }
}

void CAmClassFactory::getElementsBySink(
    const std::string sinkName,
    std::vector<std::shared_ptr<CAmClassElement > > &listClasses)
{
    std::vector<std::shared_ptr<CAmClassElement > >           listElements;
    std::vector<std::shared_ptr<CAmClassElement > >::iterator itListElements;
    getListElements(listElements);
    for (itListElements = listElements.begin(); itListElements != listElements.end();
         ++itListElements)
    {
        if ((*itListElements)->isElementBelongtoClass(sinkName, ET_SINK))
        {
            listClasses.push_back(*itListElements);
        }
    }
}

am_sourceID_t CAmClassElement::getSourceClassID(void) const
{
    return mSourceClassID;
}

am_sinkID_t CAmClassElement::getSinkClassID(void) const
{
    return mSinkClassID;
}

std::shared_ptr<CAmClassElement > CAmClassFactory::getElementBySourceClassID(
    const am_sourceClass_t sourceClassID)
{
    std::vector<std::shared_ptr<CAmClassElement > >           listElements;
    std::vector<std::shared_ptr<CAmClassElement > >::iterator itListElements;
    getListElements(listElements);
    for (itListElements = listElements.begin(); itListElements != listElements.end();
         ++itListElements)
    {
        if ((*itListElements)->getSourceClassID() == sourceClassID)
        {
            return (*itListElements);
        }
    }

    return NULL;
}

std::shared_ptr<CAmClassElement > CAmClassFactory::getElementBySinkClassID(
    const am_sinkClass_t sinkClassID)
{
    std::vector<std::shared_ptr<CAmClassElement > >           listElements;
    std::vector<std::shared_ptr<CAmClassElement > >::iterator itListElements;
    getListElements(listElements);
    for (itListElements = listElements.begin(); itListElements != listElements.end();
         ++itListElements)
    {
        if ((*itListElements)->getSinkClassID() == sinkClassID)
        {
            return (*itListElements);
        }
    }

    return NULL;
}

std::shared_ptr<CAmClassElement > CAmClassFactory::getElementByConnection(
    const std::string &connectionName)
{
    auto pConnection = CAmMainConnectionFactory::getElement(connectionName);
    if (pConnection != nullptr)
    {
        return CAmClassFactory::getElement(pConnection->getClassName());
    }

    return nullptr;
}

gc_Class_e CAmClassElement::getClassType() const
{
    return mClass.type;
}

bool CAmClassElement::isPerSinkClassVolumeEnabled(void) const
{
    std::vector<am_ClassProperty_s>::const_iterator itMapClassProperty;
    for ( itMapClassProperty = mClass.listClassProperties.begin();
          itMapClassProperty != mClass.listClassProperties.end();
          itMapClassProperty++
          )
    {
        if ((*itMapClassProperty).classProperty == CP_PER_SINK_CLASS_VOLUME_SUPPORT)
        {
            return ((*itMapClassProperty).value == 0) ? false : true;
        }
    }

    return false;
}

bool CAmClassElement::_checkElementInSubjectList(const std::string &elementName, const gc_Element_e type) const
{
    std::vector<std::shared_ptr<CAmElement > >           listOfSubjects;
    std::vector<std::shared_ptr<CAmElement > >::iterator itListSubjects;
    gc_ElementTypeName_s                                 element;

    element.elementName = elementName;
    element.elementType = type;
    getListElements(element, listOfSubjects);
    if (listOfSubjects.size() > 0)
    {
        return true;
    }

    return false;
}

bool CAmClassElement::isElementBelongtoClass(const std::string &elementName, gc_Element_e elementType) const
{
    if (true == _checkElementInSubjectList(elementName, elementType))
    {
        return true;
    }

    /*
     * Verify if both source and sink belong to topology
     */
    gc_ClassTopologyCodeID_e                                         topologyType = (elementType == ET_SINK) ? MC_SINK_ELEMENT : MC_SOURCE_ELEMENT;
    std::vector<std::vector< gc_TopologyElement_s> >::const_iterator itListTopologies;
    for (itListTopologies = mClass.listTopologies.begin();
         itListTopologies != mClass.listTopologies.end();
         ++itListTopologies)
    {
        if ( E_OK == _checkElementPresentInTopology((*itListTopologies), elementName, topologyType))
        {
            return true;
        }
        else
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, "Element :", elementName, "not found for class :",
                getName());
        }
    }

    return false;
}

bool CAmClassElement::isVolumePersistencySupported() const
{
    return mClass.isVolumePersistencySupported;
}

am_Error_e CAmClassElement::restoreVolume(gc_LastClassVolume_s &lastClassVolume)
{
    am_Error_e result = E_UNKNOWN;
    mClassLastVolume = lastClassVolume;
    return result;
}

void CAmClassElement::restoreLastMainConnectionsVolume(gc_LastMainConnectionsVolume_s &lastMainConnectionsVolume)
{
    mLastMainConnectionsVolume = lastMainConnectionsVolume;
}

std::string CAmClassElement::getVolumeString()
{
    std::string                            volumeString;
    std::vector<gc_SinkVolume_s>::iterator itlistSinks;

    if ((!mClassLastVolume.className.empty()) &&
        (!mClassLastVolume.listSinkVolume.empty())
        )
    {
        volumeString = "{" + mClassLastVolume.className + ",";
        // sample format of volume string :: {BASE,[sink1:10][sink2:20][*:30]}
        for (itlistSinks = mClassLastVolume.listSinkVolume.begin();
             itlistSinks != mClassLastVolume.listSinkVolume.end(); itlistSinks++)
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, "sink name is:", itlistSinks->sinkName);
            std::stringstream mainVolumeString;
            mainVolumeString << itlistSinks->mainVolume;
            volumeString += "[" + itlistSinks->sinkName + ":" + mainVolumeString.str() + "]";
        }

        volumeString += "}";
    }

    return volumeString;
}

std::string CAmClassElement::getLastMainConnectionsVolumeString()
{
    std::string                                    mainConnectionsVolumeString;
    std::vector<gc_SinkVolume_s>::iterator         itlistSinks;
    std::vector<gc_LastMainConVolInfo_s>::iterator itlistLastMainConVolInfo;

    if ((!mLastMainConnectionsVolume.className.empty()) &&
        (!mLastMainConnectionsVolume.listLastMainConVolInfo.empty()))
    {
        mainConnectionsVolumeString = "{" + mLastMainConnectionsVolume.className + ",";
        // sample format of last main connection volume string :: {BASE,[source1:sink1=30][source2:sink2=10]}
        for (itlistLastMainConVolInfo = mLastMainConnectionsVolume.listLastMainConVolInfo.begin();
             itlistLastMainConVolInfo != mLastMainConnectionsVolume.listLastMainConVolInfo.end(); itlistLastMainConVolInfo++)
        {
            std::stringstream mainVolumeString;
            mainVolumeString << itlistLastMainConVolInfo->mainVolume;
            mainConnectionsVolumeString += "[" + itlistLastMainConVolInfo->mainConnectionName + "=" + mainVolumeString.str() + "]";
        }

        mainConnectionsVolumeString += "}";
    }

    return mainConnectionsVolumeString;
}

std::string CAmClassElement::getMainConnectionString()
{
    std::vector<std::shared_ptr<CAmMainConnectionElement>> connectionList;
    CAmConnectionListFilter                                filter;
    filter.setClassName(mName);
    filter.setListConnectionStates( { CS_CONNECTED, CS_SUSPENDED, CS_DISCONNECTED } );
    CAmMainConnectionFactory::getListElements(connectionList, filter);

    std::string                                            mainConnectionString;
    for (const auto & pMainConnection : connectionList)
    {
        if (    pMainConnection->getMainSource()->isPersistencySupported()
             || pMainConnection->getMainSink()->isPersistencySupported())
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, "main connection name is:", pMainConnection->getName());
            mainConnectionString += pMainConnection->getName() + ";";
        }
    }
    if (mainConnectionString.length() > 0)
    {
        mainConnectionString = "{" + mName + "," + mainConnectionString + "}";
    }

    return mainConnectionString;
}

std::string CAmClassElement::getLastMainSoundPropertiesString()
{
    std::string                                    mainSoundPropertiesString;
    std::set<gc_LastMainSoundProperty_s>::iterator itlistLastSoundProperty;

    if (!mLastMainSoundProperties.className.empty())
    {
        // sample format of last main sound properties string
        // {BASE,[ET_SINK_sink1=(1:5)(2:6)(3:7)][ET_SOURCE_source1=(1:5)(2:6)(3:7)]}
        if (mLastMainSoundProperties.listLastMainSoundProperties.size() > 0)
        {
            mainSoundPropertiesString = "{" + mLastMainSoundProperties.className + ",";
            for (auto &itListLastMainSoundProperties : mLastMainSoundProperties.listLastMainSoundProperties)
            {
                std::stringstream elementType;
                elementType << itListLastMainSoundProperties.elementInfo.elementType;

                mainSoundPropertiesString += "[" + elementType.str() + "_"
                    + itListLastMainSoundProperties.elementInfo.elementName
                    + "=";
                LOG_FN_ERROR(__FILENAME__, __func__, "element name and Type:",
                    itListLastMainSoundProperties.elementInfo.elementName,
                    itListLastMainSoundProperties.elementInfo.elementType);
                for (auto &itListLastMainElementSoundProperties : itListLastMainSoundProperties.listLastMainSoundProperty)
                {
                    std::stringstream soundPropertyType;
                    soundPropertyType << itListLastMainElementSoundProperties.type;
                    std::stringstream soundPropertyValue;
                    soundPropertyValue << itListLastMainElementSoundProperties.value;

                    mainSoundPropertiesString += "(" + soundPropertyType.str() + ":"
                        + soundPropertyValue.str() + ")";
                }

                mainSoundPropertiesString += "]";
            }

            mainSoundPropertiesString += "}";
        }
    }

    return mainSoundPropertiesString;
}

am_Error_e CAmClassElement::getLastVolume(const std::string &sinkName,
    am_mainVolume_t &mainVolume)
{
    std::string                            strCompare(isPerSinkClassVolumeEnabled() ? sinkName : CLASS_LEVEL_SINK);
    std::vector<gc_SinkVolume_s>::iterator itlistSink;

    for (itlistSink = mClassLastVolume.listSinkVolume.begin();
         itlistSink != mClassLastVolume.listSinkVolume.end(); itlistSink++)
    {
        if (itlistSink->sinkName == strCompare)
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, "class last main volume is:", mainVolume, sinkName);
            mainVolume = itlistSink->mainVolume;
            return E_OK;
        }
    }

    if (mClass.defaultVolume != AM_MUTE)
    {
        mainVolume = mClass.defaultVolume;
        LOG_FN_DEBUG(__FILENAME__, __func__, "class default main volume is:", mainVolume);
    }

    return E_OK;
}

am_Error_e CAmClassElement::getLastVolume(
    std::shared_ptr<CAmMainConnectionElement > pMainConnection,
    am_mainVolume_t &mainVolume)
{
    am_Error_e result = E_UNKNOWN;

    if (pMainConnection != nullptr)
    {
        /* 1. check if last main connection volume is present, if present then return the same
         * 2. if last main connection volume not present then check if last class level volume
         * is present, if present then return the same
         * 3. if last class volume not present then return the default class volume
         */
        std::vector<gc_LastMainConVolInfo_s>::iterator itlistLastMainConVolInfo;
        for ( itlistLastMainConVolInfo = mLastMainConnectionsVolume.listLastMainConVolInfo.begin();
              itlistLastMainConVolInfo != mLastMainConnectionsVolume.listLastMainConVolInfo.end();
              itlistLastMainConVolInfo++)
        {
            if (itlistLastMainConVolInfo->mainConnectionName == pMainConnection->getName())
            {
                mainVolume = itlistLastMainConVolInfo->mainVolume;
                LOG_FN_DEBUG(__FILENAME__, __func__, "last main connection", pMainConnection->getName(), "main volume is:", mainVolume);
                result = E_OK;
                return result;
            }
        }

        std::shared_ptr<CAmSinkElement > pSinkElement = pMainConnection->getMainSink();
        if (pSinkElement != nullptr)
        {
            result = getLastVolume(pSinkElement->getName(), mainVolume);
            return result;
        }
    }

    return E_NOT_POSSIBLE;
}

am_Error_e CAmClassElement::setLastVolume(
    std::shared_ptr<CAmMainConnectionElement > pMainConnection,
    const std::string &sinkName, am_mainVolume_t mainVolume)
{
    am_Error_e result = E_UNKNOWN;

    if (pMainConnection != nullptr)
    {
        result = _setLastMainConnectionVolume(pMainConnection);
        /*check if class level volume to be stored */
        std::shared_ptr<CAmSourceElement > pSourceElement = pMainConnection->getMainSource();
        std::shared_ptr<CAmSinkElement >   pSinkElement   = pMainConnection->getMainSink();
        if ((nullptr != pSinkElement) && (nullptr != pSourceElement))
        {
            result = _setLastClassVolume(pSinkElement->getName(), pMainConnection->getMainVolume());
        }
    }

    if (!sinkName.empty())
    {
        result = _setLastClassVolume(sinkName, mainVolume);
    }

    return result;
}

am_Error_e CAmClassElement::_setLastClassVolume(const std::string &sinkName, am_mainVolume_t mainVolume)
{
    std::string                            localSinkName = sinkName;
    std::vector<gc_SinkVolume_s>::iterator itlistSink;
    am_Error_e                             result = E_UNKNOWN;

    if ((isVolumePersistencySupported() == false) &&
        (isPerSinkClassVolumeEnabled() == false))
    {
        return E_OK;
    }

    LOG_FN_DEBUG(__FILENAME__, __func__, getName(), sinkName, mainVolume);
    if (false == isPerSinkClassVolumeEnabled())
    {
        localSinkName = "*";
    }

    for ( itlistSink = mClassLastVolume.listSinkVolume.begin();
          itlistSink != mClassLastVolume.listSinkVolume.end(); itlistSink++)
    {
        // if sink is already exist then update only its main volume in the list
        if (itlistSink->sinkName == localSinkName)
        {
            itlistSink->mainVolume = mainVolume;
            result                 = E_OK;
            break;
        }
    }

    // if sink is not found in the list then consider it as new sink and insert in to the list
    if (itlistSink == mClassLastVolume.listSinkVolume.end())
    {
        gc_SinkVolume_s sinkVolume;
        sinkVolume.sinkName   = localSinkName;
        sinkVolume.mainVolume = mainVolume;
        mClassLastVolume.listSinkVolume.push_back(sinkVolume);
        result = E_OK;
    }

    if (true == isPerSinkClassVolumeEnabled())
    {
        std::shared_ptr<CAmSinkElement > pSink = CAmSinkFactory::getElement(localSinkName);
        if (nullptr != pSink)
        {
            am_MainSoundProperty_s mainSoundProperty;
            mainSoundProperty.type  = MSP_SINK_PER_CLASS_VOLUME_TYPE(getID());
            mainSoundProperty.value = mainVolume;
            LOG_FN_DEBUG("type:value", mainSoundProperty.type, mainSoundProperty.value);
            int16_t oldValue;
            pSink->getMainSoundPropertyValue(mainSoundProperty.type, oldValue);
            if (oldValue != mainSoundProperty.value)
            {
                result = mpControlReceive->changeMainSinkSoundPropertyDB(
                        mainSoundProperty, pSink->getID());
            }
        }
    }

    return result;
}

am_Error_e CAmClassElement::_setLastMainConnectionVolume(
    std::shared_ptr<CAmMainConnectionElement > pMainConnection)
{
    if (pMainConnection != nullptr)
    {
        std::vector<gc_LastMainConVolInfo_s >::iterator itlistLastMainConVolInfo;
        std::shared_ptr<CAmSourceElement >              pSourceElement = pMainConnection->getMainSource();
        std::shared_ptr<CAmSinkElement >                pSinkElement   = pMainConnection->getMainSink();
        if ((nullptr == pSinkElement) || (nullptr == pSourceElement))
        {
            return E_NOT_POSSIBLE;
        }

        /*set last main connection volume if main source or main sink supports
         * volume persistence
         */
        if (pSourceElement->isVolumePersistencySupported() || pSinkElement->isVolumePersistencySupported())
        {
            for ( itlistLastMainConVolInfo = mLastMainConnectionsVolume.listLastMainConVolInfo.begin();
                  itlistLastMainConVolInfo != mLastMainConnectionsVolume.listLastMainConVolInfo.end(); itlistLastMainConVolInfo++)
            {
                // if main connection  is already exist then update only its main volume in the list
                if (itlistLastMainConVolInfo->mainConnectionName == pMainConnection->getName())
                {
                    LOG_FN_INFO(__FILENAME__, __func__, "main connection is exsist and its volume is :", pMainConnection->getMainVolume());
                    itlistLastMainConVolInfo->mainVolume = pMainConnection->getMainVolume();
                    return E_OK;
                }
            }

            // if main connection is not found in the list then consider it as new main connection and insert in to the list
            if (itlistLastMainConVolInfo == mLastMainConnectionsVolume.listLastMainConVolInfo.end())
            {
                LOG_FN_INFO(__FILENAME__, __func__, "new main connection : ", pMainConnection->getName(), "its volume :", pMainConnection->getMainVolume());
                gc_LastMainConVolInfo_s lastMainConVolInfo;
                lastMainConVolInfo.mainConnectionName = pMainConnection->getName();
                lastMainConVolInfo.mainVolume         = pMainConnection->getMainVolume();
                mLastMainConnectionsVolume.listLastMainConVolInfo.push_back(lastMainConVolInfo);
                return E_OK;
            }
        }
        else
        {
            LOG_FN_INFO(__FILENAME__, __func__, "volume persistence not supported for main source:",
                pSourceElement->getName(), "or main sink", pSinkElement->getName());
            return E_OK;
        }
    }
    else
    {
        LOG_FN_INFO(__FILENAME__, __func__, "invalid main connection");
        return E_NOT_POSSIBLE;
    }
}

void CAmClassElement::setLastSoundProperty(const gc_ElementTypeName_s &elementInfo,
    const am_MainSoundProperty_s &mainSoundProperty)
{
    if (false == _updateListLastMainSoundProperty(elementInfo, mainSoundProperty))
    {
        gc_LastMainSoundProperty_s lastMainSoundProperty;
        lastMainSoundProperty.elementInfo = elementInfo;
        lastMainSoundProperty.listLastMainSoundProperty.push_back(mainSoundProperty);
        mLastMainSoundProperties.listLastMainSoundProperties.push_back(lastMainSoundProperty);
    }
}

am_Error_e CAmClassElement::getLastSoundProperty(
    const gc_ElementTypeName_s &elementInfo,
    std::vector<am_MainSoundProperty_s > &listLastMainSoundProperty)
{
    for (auto &itListLastMainSoundProperties : mLastMainSoundPropertiesFromPersistence.listLastMainSoundProperties)
    {
        if ((itListLastMainSoundProperties.elementInfo.elementType == elementInfo.elementType) &&
            (itListLastMainSoundProperties.elementInfo.elementName == elementInfo.elementName))
        {
            listLastMainSoundProperty = itListLastMainSoundProperties.listLastMainSoundProperty;
            return E_OK;
        }
    }

    return E_NON_EXISTENT;
}

bool CAmClassElement::_updateListLastMainSoundProperty(const gc_ElementTypeName_s &elementInfo,
    const am_MainSoundProperty_s &mainSoundProperty)
{
    for (auto &itListLastMainSoundProperties : mLastMainSoundProperties.listLastMainSoundProperties)
    {
        if ((itListLastMainSoundProperties.elementInfo.elementType == elementInfo.elementType) &&
            (itListLastMainSoundProperties.elementInfo.elementName == elementInfo.elementName))
        {
            for (auto &itListLastMainSoundProperty : itListLastMainSoundProperties.listLastMainSoundProperty )
            {
                if (itListLastMainSoundProperty.type == mainSoundProperty.type)
                {
                    itListLastMainSoundProperty.value = mainSoundProperty.value;
                    return true;
                }
            }

            itListLastMainSoundProperties.listLastMainSoundProperty.push_back(mainSoundProperty);
            return true;
        }
    }

    return false;
}

void CAmClassElement::restoreMainSoundProperties(gc_LastMainSoundProperties_s &lastMainSoundProperties)
{
    mLastMainSoundPropertiesFromPersistence = lastMainSoundProperties;
}

std::shared_ptr<CAmElement > CAmClassElement::getElement()
{
    return CAmClassFactory::getElement(getName());
}

} /* namespace gc */
} /* namespace am */
