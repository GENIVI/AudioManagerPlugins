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
#include "CAmClassElement.h"
#include "CAmSinkElement.h"
#include "CAmSourceElement.h"
#include "CAmLogger.h"
#include "CAmTriggerQueue.h"
#include "IAmAction.h"
#include <algorithm>

namespace am {
namespace gc {

CAmMainConnectionElement::CAmMainConnectionElement(const gc_Route_s &route, IAmControlReceive *pControlReceive)
    : CAmElement(ET_CONNECTION, route.name, pControlReceive)
    , mRoute(route)
    , mpClassElement(CAmClassFactory::getElement(getMainSourceName(), getMainSinkName()))
    , mOngoingTransitions()
    , mMainVolume(0)
{
    LOG_FN_DEBUG(__FILENAME__, __func__, getName());

    CAmMainConnectionFactory::mOrderedList.push_back(this);
}

CAmMainConnectionElement::~CAmMainConnectionElement()
{
    for (auto iter = CAmMainConnectionFactory::mOrderedList.begin(); iter != CAmMainConnectionFactory::mOrderedList.end(); ++iter)
    {
        if ((*iter) == this)
        {
            CAmMainConnectionFactory::mOrderedList.erase(iter);
            break;
        }
    }
}

std::shared_ptr<CAmClassElement> CAmMainConnectionElement::getClassElement(void) const
{
    return mpClassElement;
}

std::string CAmMainConnectionElement::getClassName(void) const
{
    if (mpClassElement != nullptr)
    {
        return mpClassElement->getName();
    }

    return "";
}

std::string CAmMainConnectionElement::getMainSourceName(void) const
{
    std::string                        mainSourceName;
    std::shared_ptr<CAmSourceElement > pMainSource = getMainSource();

    if (pMainSource != nullptr)
    {
        mainSourceName = pMainSource->getName();
    }

    return mainSourceName;
}

std::string CAmMainConnectionElement::getMainSinkName(void) const
{
    std::string                      mainSinkName;
    std::shared_ptr<CAmSinkElement > pMainSink = getMainSink();

    if (pMainSink != nullptr)
    {
        mainSinkName = pMainSink->getName();
    }

    return mainSinkName;
}

std::shared_ptr<CAmSinkElement > CAmMainConnectionElement::getMainSink(void) const
{
    return CAmSinkFactory::getElement(mRoute.sinkID);
}

std::shared_ptr<CAmSourceElement > CAmMainConnectionElement::getMainSource(void) const
{
    return CAmSourceFactory::getElement(mRoute.sourceID);
}

void CAmMainConnectionElement::registerTransitionAction(const IAmActionCommand *pTransitionAction)
{
    if (pTransitionAction)
    {
        mOngoingTransitions.push_back(pTransitionAction);

        LOG_FN_DEBUG(__FILENAME__, __func__, getName(), "ID =", getID(), " appended",
            pTransitionAction->getName(), "size =", mOngoingTransitions.size());
    }
}

void CAmMainConnectionElement::unregisterTransitionAction(const IAmActionCommand *pTransitionAction)
{
    std::vector<const IAmActionCommand *>::iterator itAction;
    itAction = std::find(mOngoingTransitions.begin(), mOngoingTransitions.end(), pTransitionAction);
    if (itAction != mOngoingTransitions.end())
    {
        mOngoingTransitions.erase(itAction);

        LOG_FN_DEBUG(__FILENAME__, __func__, getName(), "ID =", getID(), " removed",
            pTransitionAction->getName(), "size =", mOngoingTransitions.size());
    }
}

void CAmMainConnectionElement::setStateChangeTrigger(am_Error_e actionResult)
{
    gc_ConnectionStateChangeTrigger_s *pTrigger = new gc_ConnectionStateChangeTrigger_s;
    pTrigger->connectionName  = getName();
    pTrigger->connectionState = getState();
    pTrigger->status          = actionResult;
    CAmTriggerQueue::getInstance()->queueWithPriority(SYSTEM_CONNECTION_STATE_CHANGE, pTrigger);

    LOG_FN_INFO(__FILENAME__, __func__, pTrigger->connectionName, "ID =", getID(),
        pTrigger->connectionState, "SYSTEM_CONNECTION_STATE_CHANGE appended to queue");
}

bool CAmMainConnectionElement::permitsDispose()
{
    // first check whether an executing transition has registered with us
    size_t               ongoingTransitions = mOngoingTransitions.size();
    am_ConnectionState_e state              = getState();

    // check whether another Connect action is latent in the trigger-queue
    std::vector<std::pair<gc_Trigger_e, const gc_TriggerElement_s *> > queueSnapShot;
    CAmTriggerQueue::getInstance()->getSnapShot(queueSnapShot);
    for (const auto &trigger : queueSnapShot)
    {
        if ((trigger.first == USER_CONNECTION_REQUEST) && (trigger.second != NULL))
        {
            auto pTrigger = static_cast<const gc_ConnectTrigger_s *>(trigger.second);
            if (pTrigger->sinkName == getMainSinkName() && (pTrigger->sourceName == getMainSourceName()))
            {
                ++ongoingTransitions;
            }
        }
    }

    // allow deletion only if nothing is found so far and we are in DISCONNECTED state
    bool retVal = (state == CS_DISCONNECTED) && (0 == ongoingTransitions);

    LOG_FN_DEBUG(__FILENAME__, __func__, getName(), "ID =", getID(), state,
        ongoingTransitions, "ongoing transitions, return", retVal);

    return retVal;
}

am_ConnectionState_e CAmMainConnectionElement::getState() const
{
    am_MainConnection_s mainConnection;
    // get the connection info from database
    am_Error_e result = mpControlReceive->getMainConnectionInfoDB(getID(), mainConnection);

    if (result == E_OK)
    {
        return mainConnection.connectionState;
    }
    else
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "FAILED with", result, "for", getName()
            , "with ID", getID());
        return CS_UNKNOWN;
    }
}

void CAmMainConnectionElement::setState(am_ConnectionState_e state)
{
    am_ConnectionState_e tempState = getState();
    if (tempState != state)
    {
        mpControlReceive->changeMainConnectionStateDB(getID(), state);
        notify(state);
    }
}

int32_t CAmMainConnectionElement::getPriority() const
{
    int32_t priority = 0;

    std::vector<std::shared_ptr<CAmElement > > listOfSubjects;
    getListElements(ET_ROUTE, listOfSubjects);

    for (auto itListSubjects : listOfSubjects)
    {
        priority += itListSubjects->getPriority();
    }

    return priority;
}

am_Availability_e CAmMainConnectionElement::getRouteAvailability() const
{
    if (mListRouteElements.empty())
    {
        LOG_FN_WARN(__FILENAME__, __func__, mName, "Route empty");
        return A_UNKNOWN;
    }

    am_Availability_e  retVal = A_AVAILABLE;
    for (const auto &routeElement : mListRouteElements)
    {
        am_Availability_s sourceAvailability;
        am_Availability_s sinkAvailability;
        routeElement->getSource()->getAvailability(sourceAvailability);
        routeElement->getSink()->getAvailability(sinkAvailability);
        if ((sourceAvailability.availability == A_UNKNOWN) || (sinkAvailability.availability == A_UNKNOWN))
        {
            LOG_FN_WARN(__FILENAME__, __func__, mName, "route segment", routeElement->getName()
                    , "has unknown availability");
            return A_UNKNOWN;  // strongest, no need to check other elements
        }
        else if ((sourceAvailability.availability == A_UNAVAILABLE) || (sinkAvailability.availability == A_UNAVAILABLE))
        {
            LOG_FN_WARN(__FILENAME__, __func__, mName, "route segment", routeElement->getName(), "NOT available");
            retVal = A_UNAVAILABLE;
        }
    }

    return retVal;
}

void CAmMainConnectionElement::getListRouteElements(
    std::vector<std::shared_ptr<CAmRouteElement > > &listRouteElements) const
{
    listRouteElements = mListRouteElements;
}

void CAmMainConnectionElement::updateState()
{
    am_ConnectionState_e                                      connectionState = CS_DISCONNECTED;
    std::vector<std::shared_ptr<CAmRouteElement > >::iterator itlistRouteElements;
    for (itlistRouteElements = mListRouteElements.begin();
         itlistRouteElements != mListRouteElements.end(); ++itlistRouteElements)
    {
        connectionState = (*itlistRouteElements)->getState();
        if (connectionState != CS_CONNECTED)
        {
            break;
        }
    }

    std::shared_ptr<CAmSourceElement > pMainSource = CAmSourceFactory::getElement(mRoute.sourceID);
    if (pMainSource != nullptr)
    {
        am_SourceState_e sourceState = pMainSource->getState();
        if ((connectionState == CS_CONNECTED) && (sourceState != SS_ON))
        {
            connectionState = CS_SUSPENDED;
        }
    }

    setState(connectionState);
}

am_Error_e CAmMainConnectionElement::_register(void)
{
    am_Error_e                                  result = E_DATABASE_ERROR;
    std::vector<am_RoutingElement_s >::iterator itListRoutingElements;
    for (itListRoutingElements = mRoute.route.begin(); itListRoutingElements != mRoute.route.end();
         itListRoutingElements++)
    {
        // TODO Don't touch to source/sink directly
        std::shared_ptr<CAmSourceElement > pSource = CAmSourceFactory::getElement(
                (*itListRoutingElements).sourceID);
        std::shared_ptr<CAmSinkElement > pSink = CAmSinkFactory::getElement(
                (*itListRoutingElements).sinkID);

        if ((pSource == nullptr) || (pSink == nullptr))
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "Source or sink not found");
            return result;
        }

        gc_RoutingElement_s gcRoutingElement;
        gcRoutingElement.name             = pSource->getName() + ":" + pSink->getName();
        gcRoutingElement.sourceID         = (*itListRoutingElements).sourceID;
        gcRoutingElement.sinkID           = (*itListRoutingElements).sinkID;
        gcRoutingElement.domainID         = (*itListRoutingElements).domainID;
        gcRoutingElement.connectionFormat = (*itListRoutingElements).connectionFormat;
        std::shared_ptr<CAmRouteElement > pRoutingElement = CAmRouteFactory::createElement(
                gcRoutingElement, mpControlReceive);

        if (pRoutingElement != nullptr)
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, "sourceID SinkID", pRoutingElement->getSourceID(),
                pRoutingElement->getSinkID());
            mListRouteElements.push_back(pRoutingElement);
            result = pRoutingElement->attach(pSource);
            if (result != E_OK)
            {
                // its an error need to decide
                LOG_FN_DEBUG(__FILENAME__, __func__,
                    " source element attach to route element failed, result is:", result);
            }

            result = pRoutingElement->attach(pSink);
            if (result != E_OK)
            {
                // its an error need to decide
                LOG_FN_DEBUG(__FILENAME__, __func__,
                    "sink element attach to route element failed, result is:", result);
            }
        }
        else
        {
            return result;
        }
    }

    am_MainConnection_s   mainConnection;
    am_mainConnectionID_t mainConnectionID;
    mainConnection.sourceID         = mRoute.sourceID;
    mainConnection.sinkID           = mRoute.sinkID;
    mainConnection.connectionState  = CS_UNKNOWN;
    mainConnection.mainConnectionID = 0;
    // register the connection with database
    am_Error_e dbSuccess = mpControlReceive->enterMainConnectionDB(mainConnection, mainConnectionID);
    if ((E_OK == dbSuccess) || (E_ALREADY_EXISTS == dbSuccess))
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

am_volume_t CAmMainConnectionElement::getVolume() const
{
    am_volume_t retVal = AM_VOLUME_NO_LIMIT;

    // local helper function to investigate source and sink elements
    auto apply = [&] (const std::shared_ptr<CAmRoutePointElement> &pElement)
    {
        if ((pElement != nullptr) && (pElement->getVolumeSupport()))
        {
            retVal += pElement->getVolume();
        }
    };

    for (const auto& pRoute : mListRouteElements)
    {
        apply(pRoute->getSource());
        apply(pRoute->getSink());
    }

    return retVal;
}

am_volume_t CAmMainConnectionElement::getOffsetVolume() const
{
    am_volume_t retVal = AM_VOLUME_NO_LIMIT;

    // local helper function to investigate source and sink elements
    auto apply = [&] (const std::shared_ptr<CAmRoutePointElement> &pElement)
    {
        if ((pElement != nullptr) && (pElement->getVolumeSupport()))
        {
            retVal += pElement->getOffsetVolume();
        }
    };

    for (const auto& pRoute : mListRouteElements)
    {
        apply(pRoute->getSource());
        apply(pRoute->getSink());
    }

    return retVal;
}

am_MuteState_e CAmMainConnectionElement::getMuteState() const
{
    am_MuteState_e retVal = MS_UNMUTED;

    // local helper function to investigate source and sink elements
    auto apply = [&] (const std::shared_ptr<CAmRoutePointElement> &pElement)
    {
        if (retVal == MS_UNKNOWN)
        {
            return;
        }

        if ((pElement != nullptr) && (pElement->getVolumeSupport()))
        {
            am_MuteState_e mute = pElement->getMuteState();
            if (mute == MS_UNKNOWN)
            {
                retVal = MS_UNKNOWN;
                return;
            }
            else if (mute == MS_MUTED)
            {
                retVal = MS_MUTED;
            }
        }
    };

    for (const auto& pRoute : mListRouteElements)
    {
        apply(pRoute->getSource());
        apply(pRoute->getSink());
    }

    return retVal;
}

void CAmMainConnectionElement::setMainVolume(const am_mainVolume_t volume)
{
    if (mMainVolume != volume)
    {
        LOG_FN_INFO(__FILENAME__, __func__, mMainVolume, "to", volume, "for", mType, mName);
    }
    else
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, volume, "(unchanged) for", mType, mName);
    }

    mMainVolume = volume;
}

am_mainVolume_t CAmMainConnectionElement::getMainVolume() const
{

    return mMainVolume;
}

void CAmMainConnectionElement::getLimits(std::list<gc_LimitVolume_s > &limits) const
{
    // return limits directly installed on related class element
    if (mpClassElement != nullptr)
    {
        mpClassElement->getLimits(limits);
    }

    // add limits possibly directly installed on route elements
    std::list<std::shared_ptr<CAmRoutePointElement > > listVolumeElements;
    getVolumeChangeElements(listVolumeElements, SD_MAINSINK_TO_MAINSOURCE);
    for (const auto &pElement : listVolumeElements)
    {
        if (pElement != nullptr)
        {
            pElement->getLimits(limits);
        }
    }
}

void CAmMainConnectionElement::getVolumeChangeElements(std::list<std::shared_ptr<CAmRoutePointElement > > &listVolumeElements
        , gc_SetSourceStateDirection_e direction) const
{
    if (listVolumeElements.size() > 0)
    {
        LOG_FN_WARN(__FILENAME__, __func__, "listVolumeElements not empty for", mName);
    }

    // Scan list of route elements for items supporting volume change.
    // Primary order is SD_MAINSINK_TO_MAINSOURCE as preferred for setVolume actions
    for (const auto &pRouteElement : mListRouteElements)
    {
        if (pRouteElement == nullptr)
        {
            continue;
        }

        const auto & pSourceElement = pRouteElement->getSource();
        const auto & pSinkElement = pRouteElement->getSink();
        if ((pSourceElement != nullptr) && pSourceElement->getVolumeSupport())
        {
            listVolumeElements.push_front(pSourceElement);
        }
        if ((pSinkElement != nullptr) && pSinkElement->getVolumeSupport())
        {
            listVolumeElements.push_front(pSinkElement);
        }
    }

    if (direction == SD_MAINSOURCE_TO_MAINSINK)
    {
        listVolumeElements.reverse();
    }
}

bool CAmMainConnectionElement::isSinkBelongingToMainConnection(
    std::shared_ptr<CAmSinkElement > pSink)
{
    bool                                                      found = false;
    std::vector<std::shared_ptr<CAmRouteElement > >::iterator itListRouteElement;
    for (itListRouteElement = mListRouteElements.begin();
         itListRouteElement != mListRouteElements.end(); ++itListRouteElement)
    {
        if ((*itListRouteElement)->getSink() == pSink)
        {
            found = true;
            break;
        }
    }

    return found;
}

bool CAmMainConnectionElement::isSourceBelongingToMainConnection(
    std::shared_ptr<CAmSourceElement > pSource)
{
    bool                                                      found = false;
    std::vector<std::shared_ptr<CAmRouteElement > >::iterator itListRouteElement;
    for (itListRouteElement = mListRouteElements.begin();
         itListRouteElement != mListRouteElements.end(); ++itListRouteElement)
    {
        if ((*itListRouteElement)->getSource() == pSource)
        {
            found = true;
            break;
        }
    }

    return found;
}

int CAmMainConnectionElement::update(std::shared_ptr<CAmElement > pNotifierElement,
    const am_mainVolume_t &mainVolume)
{
    setMainVolume(mainVolume);
    return E_OK;
}

am_Error_e CAmMainConnectionElement::updateMainVolume()
{
    am_volume_t                      volume = 0;
    std::shared_ptr<CAmSinkElement > pSink  = getMainSink();

    if (pSink != nullptr)
    {
        if (getState() == CS_CONNECTED)
        {
            /*
             * The shared sink's main volume is the sum of the volume of all
             * the connection in which it is involved.
             *             mc1                 mc2
             *  {source1} <---> {shared-sink} <---> {source2}
             *  volume = Vsource1 + Vshared-sink + Vsource2
             */
            std::vector<std::shared_ptr<CAmMainConnectionElement > > listMainConnections;
            std::map<std::shared_ptr<CAmElement >, am_volume_t >     mapVolume;
            CAmConnectionListFilter                                  filter;
            filter.setSinkName(pSink->getName());
            filter.setListConnectionStates({CS_CONNECTED});

            CAmMainConnectionFactory::getListElements(listMainConnections, filter);
            for (const auto &pMainConnection : listMainConnections)
            {
                std::list<std::shared_ptr<CAmRoutePointElement > > listVolumeElements;
                pMainConnection->getVolumeChangeElements(listVolumeElements, SD_MAINSINK_TO_MAINSOURCE);
                for (const auto &pElement : listVolumeElements)
                {
                    if (mapVolume.count(pElement) == 0)
                    {
                        am_volume_t elementVolume = pElement->getVolume();
                        volume += elementVolume;
                        mapVolume[pElement] = elementVolume;

                        LOG_FN_INFO(__FILENAME__, __func__, "The volume of", pElement->getType()
                                , pElement->getName(), "is", elementVolume);
                    }
                }
            }

            LOG_FN_INFO(__FILENAME__, __func__, "The total volume in connection", mName, "is", volume);
        }
        else
        {
            volume = getVolume();
        }

        am_mainVolume_t mainVolume = pSink->convertVolumeToMainVolume(volume);
        pSink->setMainVolume(mainVolume);

        setMainVolume(pSink->convertVolumeToMainVolume(getVolume()));
    }

    return E_OK;
}

std::shared_ptr<CAmElement > CAmMainConnectionElement::getElement()
{
    return CAmMainConnectionFactory::getElement(getName());
}

void CAmMainConnectionElement::removeRouteElements()
{
    LOG_FN_ENTRY(__FILENAME__, __func__);
    std::vector<std::shared_ptr<CAmElement > >           listOfSubjects;
    std::vector<std::shared_ptr<CAmElement > >::iterator itListSubjects;
    getListElements(ET_ROUTE, listOfSubjects);

    for (itListSubjects = listOfSubjects.begin(); itListSubjects != listOfSubjects.end();
         ++itListSubjects)
    {
        if (nullptr != (*itListSubjects))
        {
            std::shared_ptr<CAmSourceElement > pSource = (std::static_pointer_cast < CAmRouteElement
                                                                                     > (*itListSubjects))->getSource();
            if (pSource != nullptr)
            {
                detach(pSource);
            }

            std::shared_ptr<CAmSinkElement > pSink = (std::static_pointer_cast < CAmRouteElement
                                                                                 > (*itListSubjects))->getSink();
            if (pSink != nullptr)
            {
                detach(pSink);
            }

            /*if subjects observer count is more than one it means it is shared resource, so don't delete it
             * just detach the observer from the subject list*/
            if ((*itListSubjects)->getObserverCount() > SHARED_COUNT)
            {
                LOG_FN_DEBUG(__FILENAME__,
                    __func__,
                    "route is shared resource so don't delete it and only remove the link");
                detach(*itListSubjects);
            }
            else
            {
                LOG_FN_DEBUG(__FILENAME__, __func__,
                    "mainconnection deleting route element, its name is :",
                    (*itListSubjects)->getName());
                detach(*itListSubjects);
                am_Error_e err = CAmRouteFactory::destroyElement((*itListSubjects)->getName());
            }
        }
    }

    mListRouteElements.clear();
    LOG_FN_EXIT(__FILENAME__, __func__);
}

// =============================================================================
//                  M a i n - C o n n e c t i o n    F a c t o r y

std::list<CAmMainConnectionElement* > CAmMainConnectionFactory::mOrderedList;

void CAmMainConnectionFactory::moveToEnd(const std::string &mainConnectionName, gc_Order_e position)
{
    // helper function to ensure correct output of lists sorted by O_OLDEST or O_NEWEST
    // (will move given element to end of list)

    std::shared_ptr<CAmMainConnectionElement> pElement = getElement(mainConnectionName);
    if (pElement == nullptr)
    {
        LOG_FN_WARN(__FILENAME__, __func__, "Element NOT found with name:", mainConnectionName);
        return;
    }

    // obtain raw pointer
    CAmMainConnectionElement *pMainConnection = pElement.get();
    for (auto iter = mOrderedList.begin(); iter != mOrderedList.end(); ++iter)
    {
        if ((*iter) != pMainConnection)
        {
            continue;
        }

        LOG_FN_DEBUG(__FILENAME__, __func__, "moving", pMainConnection->getName(), "to", position);
        if (position == O_NEWEST)
        {
            mOrderedList.erase(iter);
            mOrderedList.push_back(pMainConnection);
        }
        else if (position == O_OLDEST)
        {
            mOrderedList.erase(iter);
            mOrderedList.push_front(pMainConnection);
        }
        else
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "invalid position", position
                    , "NOT moving", pMainConnection->getName());
        }
        break;
    }
}

void CAmMainConnectionFactory::getListElements(std::vector<std::shared_ptr<CAmMainConnectionElement>> &connectionList
        , const CAmConnectionListFilter &filter, const gc_Order_e order)
{
    if (!connectionList.empty())
    {
        LOG_FN_WARN(__FILENAME__, __func__, "connectionList NOT empty; size is", connectionList.size());
    }

    connectionList.reserve(mMapElements.size());
    for (auto it = mOrderedList.begin(); it != mOrderedList.end(); ++it)
    {
        // get corresponding shared_ptr and check against filter
        std::shared_ptr<CAmMainConnectionElement> pCandidate = getElement((*it)->getName());
        if (filter .matches(pCandidate))
        {
            connectionList.push_back(pCandidate);
        }
    }

    if (connectionList.size() <= 1)
    {
        // no need to think about sorting
        return;
    }

    // sort result according to given order
    typedef std::shared_ptr<CAmMainConnectionElement > Elem;
    switch (order)
    {
        case O_HIGH_PRIORITY:
            {
                auto comp = [] (Elem lhs, Elem rhs) -> bool { return (lhs->getPriority() < rhs->getPriority()); };
                std::stable_sort(connectionList.begin(), connectionList.end(), comp);
            }
            break;
        case O_LOW_PRIORITY:
            {
                auto comp = [] (Elem lhs, Elem rhs) -> bool { return (lhs->getPriority() > rhs->getPriority()); };
                std::stable_sort(connectionList.begin(), connectionList.end(), comp);
            }
            break;
        case O_NEWEST:
            {
                std::reverse(connectionList.begin(), connectionList.end());
            }
            break;

        case O_OLDEST:
        default:
            break;
    }
}

// =============================================================================
//                    C o n n e c t i o n - L i s t   F i l t e r

CAmConnectionListFilter::CAmConnectionListFilter()
    : mClassName("")
    , mSourceName("")
    , mSinkName("")
{
    mListConnectionStates.clear();
    mListExceptSinks.clear();
    mListExceptSources.clear();
}

void CAmConnectionListFilter::setClassName(const std::string &className)
{
    mClassName = className;
}

void CAmConnectionListFilter::setSourceName(const std::string &sourceName)
{
    mSourceName = sourceName;
}

void CAmConnectionListFilter::setSinkName(const std::string &sinkName)
{
    mSinkName = sinkName;
}

void CAmConnectionListFilter::setListConnectionStates(
    const std::vector<am_ConnectionState_e > &listConnectionStates)
{
    mListConnectionStates = listConnectionStates;
}

void CAmConnectionListFilter::setListExceptSourceNames(const std::vector<std::string > &listExceptSources)
{
    mListExceptSources = listExceptSources;
}

void CAmConnectionListFilter::setListExceptSinkNames(const std::vector<std::string > &listExceptSinks)
{
    mListExceptSinks = listExceptSinks;
}

void CAmConnectionListFilter::setListExceptClassNames(const std::vector<std::string > &listExceptClasses)
{
    mListExceptClasses = listExceptClasses;
}

bool CAmConnectionListFilter::matches(const std::shared_ptr<CAmMainConnectionElement > &pCandidate) const
{
    if (    (nullptr == pCandidate)
         || (!matches(mClassName, pCandidate->getClassName()))
         || (!mListExceptClasses.empty() && matchesAny(mListExceptClasses, pCandidate->getClassName()))
         || (!matchesAny(mListConnectionStates, pCandidate->getState())))
    {
        return false;
    }

    // no immediate mismatch found - check source and sink of route elements
    std::vector<std::shared_ptr<CAmRouteElement > > listRouteElements;
    pCandidate->getListRouteElements(listRouteElements);
    bool sourceMatch = false;
    bool sinkMatch = false;
    bool hasSourceExceptions = (mListExceptSources.size() > 0);
    bool hasSinkExceptions   = (mListExceptSinks.size() > 0);
    for (auto routeElement : listRouteElements)
    {
        std::string sourceName = routeElement->getSource()->getName();
        std::string sinkName   = routeElement->getSink()->getName();

        if (    (hasSourceExceptions && matchesAny(mListExceptSources, sourceName))
             || (hasSinkExceptions && matchesAny(mListExceptSinks, sinkName)))
        {
            // either source or sink are on the exception list - ignore this candidate
            return false;
        }

        sourceMatch |= matches(mSourceName, sourceName);
        sinkMatch   |= matches(mSinkName, sinkName);
    }

    if (sourceMatch && sinkMatch)
    {
        // all conditions match
        LOG_FN_DEBUG(__FILENAME__, __func__, pCandidate->getName(), " in state ", pCandidate->getState());

        return true;
    }

    return false;
}

bool CAmConnectionListFilter::matches(const std::string &filterName, const std::string &inputName)
{
    if ("" == filterName)
    {
        return true;
    }

    return (filterName == inputName);
}

bool CAmConnectionListFilter::matchesAny(const std::vector<am_ConnectionState_e > &filterStates, am_ConnectionState_e connectionState)
{
    if (filterStates.empty())  // always match if list is empty
    {
        return true;
    }

    // match if given connection state is contained in the list
    return (std::find(filterStates.begin(), filterStates.end(), connectionState) != filterStates.end());
}

bool CAmConnectionListFilter::matchesAny(const std::vector<std::string > &listString, const std::string &input)
{
    if (listString.empty())
    {
        return true;
    }

    // match if given name is contained in the list
    return (std::find(listString.begin(), listString.end(), input) != listString.end());
}

} /* namespace gc */
} /* namespace am */
