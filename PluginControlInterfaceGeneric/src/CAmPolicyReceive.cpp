/******************************************************************************
 * @file: CAmPolicyReceive.cpp
 *
 * This file contains the definition of policy engine receive class (member
 * functions and data members) used to provide the interface to policy engine to
 * get the context of controller as per current scenario
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Toshiaki Isogai <tisogai@jp.adit-jv.com>
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>
 *          Prashant Jain   <pjain@jp.adit-jv.com>
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 - 2019 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#include <algorithm>
#include "CAmPolicyReceive.h"
#include "CAmPolicyAction.h"
#include "CAmLogger.h"
#include "CAmSinkElement.h"
#include "CAmSourceElement.h"
#include "CAmGatewayElement.h"
#include "IAmPolicySend.h"
#include "CAmRootAction.h"
#include "CAmMainConnectionElement.h"
#include "CAmClassElement.h"
#include "CAmDomainElement.h"

namespace am {
namespace gc {

CAmPolicyReceive::CAmPolicyReceive(IAmControlReceive *pControlReceive, IAmPolicySend *pPolicySend)
    : mpControlReceive(pControlReceive)
    , mpPolicySend(pPolicySend)
{
}

bool CAmPolicyReceive::isDomainRegistrationComplete(const std::string &domainName)
{
    am_Domain_s info;
    auto        pDomainElement = CAmDomainFactory::getElement(domainName);
    if ((pDomainElement != nullptr) && (E_OK == pDomainElement->getDomainInfo(info)))
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, domainName, info.complete ? "true" : "false");
        return info.complete;
    }

    LOG_FN_ERROR(__FILENAME__, __func__, domainName, "FAILED getting info");
    return false;
}

bool CAmPolicyReceive::isRegistered(const gc_Element_e type, const std::string &name)
{
    return (_getElement(type, name) != nullptr) ? true : false;
}

am_Error_e CAmPolicyReceive::getAvailability(const gc_Element_e type, const std::string &name,
    am_Availability_s &availability)
{
    std::shared_ptr<CAmElement > pElement = _getElement(type, name);
    if (pElement == nullptr)
    {
        return E_NOT_POSSIBLE;
    }

    switch (pElement->getType())
    {
        case ET_SOURCE:
        case ET_SINK:
            return (std::static_pointer_cast<CAmRoutePointElement>(pElement))->getAvailability(availability);
        default:
            availability = {A_UNKNOWN, AR_UNKNOWN};
            return E_NON_EXISTENT;
    }
}

am_Error_e CAmPolicyReceive::getInterruptState(const gc_Element_e type, const std::string &name,
    am_InterruptState_e &interruptState)
{
    std::shared_ptr<CAmElement > pElement = _getElement(type, name);
    if (pElement == nullptr)
    {
        return E_NOT_POSSIBLE;
    }

    if (pElement->getType() == ET_SOURCE) // only defined for sources
    {
        interruptState = (static_pointer_cast<CAmSourceElement>(pElement))->getInterruptState();
        return E_OK;
    }

    interruptState = IS_UNKNOWN;
    return E_NON_EXISTENT;
}

am_Error_e CAmPolicyReceive::getMuteState(const gc_Element_e type, const std::string &name,
    am_MuteState_e &muteState)
{
    std::shared_ptr<CAmElement > pElement = _getElement(type, name);
    if (pElement != nullptr)
    {
        switch (pElement->getType())
        {
            case ET_CLASS:
                muteState = (std::static_pointer_cast<CAmClassElement>(pElement))->getMuteState();
                return E_OK;

            case ET_CONNECTION:
                muteState = (std::static_pointer_cast<CAmMainConnectionElement>(pElement))->getMuteState();
                return E_OK;

            case ET_SOURCE:
            case ET_SINK:
                muteState = (std::static_pointer_cast<CAmRoutePointElement>(pElement))->getMuteState();
                return E_OK;
        }
    }

    return E_NOT_POSSIBLE;
}

am_Error_e CAmPolicyReceive::getSoundProperty(const gc_Element_e elementType,
    const std::string &elementName,
    const am_CustomSoundPropertyType_t propertyType,
    int16_t &propertyValue)
{
    am_Error_e result(E_NOT_POSSIBLE);
    if (elementType == ET_SOURCE)
    {
        std::shared_ptr<CAmSourceElement > pElement = CAmSourceFactory::getElement(elementName);
        if (pElement != nullptr)
        {
            result = pElement->getSoundPropertyValue(propertyType, propertyValue);
        }
    }
    else if (elementType == ET_SINK)
    {
        std::shared_ptr<CAmSinkElement > pElement = CAmSinkFactory::getElement(elementName);
        if (pElement != nullptr)
        {
            result = pElement->getSoundPropertyValue(propertyType, propertyValue);
        }
    }

    return result;
}

am_Error_e CAmPolicyReceive::getMainSoundProperty(const gc_Element_e elementType,
    const std::string &elementName,
    const am_CustomMainSoundPropertyType_t propertyType,
    int16_t &propertyValue)
{
    am_Error_e result(E_NOT_POSSIBLE);
    if (elementType == ET_SOURCE)
    {
        std::shared_ptr<CAmSourceElement > pElement = CAmSourceFactory::getElement(elementName);
        if (pElement != nullptr)
        {
            result = pElement->getMainSoundPropertyValue(propertyType, propertyValue);
        }
    }
    else if (elementType == ET_SINK)
    {
        std::shared_ptr<CAmSinkElement > pElement = CAmSinkFactory::getElement(elementName);
        if (pElement != nullptr)
        {
            result = pElement->getMainSoundPropertyValue(propertyType, propertyValue);
        }
    }

    return result;
}

am_Error_e CAmPolicyReceive::getSystemProperty(const am_CustomSystemPropertyType_t systemPropertyType,
    int16_t &value)
{
    std::vector<am_SystemProperty_s>           listSystemProperties;
    std::vector<am_SystemProperty_s>::iterator itListSystemProperties;
    // get the list of system properties from database
    mpControlReceive->getListSystemProperties(listSystemProperties);
    for (itListSystemProperties = listSystemProperties.begin();
         itListSystemProperties != listSystemProperties.end(); itListSystemProperties++)
    {
        if ((*itListSystemProperties).type == systemPropertyType)
        {
            value = (*itListSystemProperties).value;
            return E_OK;
        }
    }

    return E_NOT_POSSIBLE;
}

am_Error_e CAmPolicyReceive::getVolume(const gc_Element_e elementType,
    const std::string &elementName,
    am_volume_t &volume)
{
    const std::shared_ptr<CAmElement > &pElement = _getElement(elementType, elementName);
    if (pElement != nullptr)
    {
        switch (pElement->getType())
        {
            case ET_CONNECTION:
                volume = (std::static_pointer_cast<CAmMainConnectionElement>(pElement))->getVolume();
                return E_OK;

            case ET_SOURCE:
            case ET_SINK:
                volume = (std::static_pointer_cast<CAmRoutePointElement>(pElement))->getVolume();
                return E_OK;
        }
    }

    return E_NOT_POSSIBLE;
}

am_Error_e CAmPolicyReceive::getMainVolume(const gc_Element_e elementType,
    const std::string &elementName,
    am_mainVolume_t &mainVolume)
{
    const std::shared_ptr<CAmElement > &pElement = _getElement(elementType, elementName);
    if (pElement != nullptr)
    {
        switch (elementType)
        {
            case ET_SINK:
                mainVolume = (std::static_pointer_cast<CAmSinkElement>(pElement))->getMainVolume();
                return E_OK;
    
            case ET_CONNECTION:
                mainVolume = (std::static_pointer_cast<CAmMainConnectionElement>(pElement))->getMainVolume();
                return E_OK;
        }
    }

    return E_NOT_POSSIBLE;
}

bool CAmPolicyReceive::_sortingLowest(gc_ConnectionInfo_s i, gc_ConnectionInfo_s j)
{
    return (i.priority > j.priority);
}

bool CAmPolicyReceive::_sortingHighest(gc_ConnectionInfo_s i, gc_ConnectionInfo_s j)
{
    return (i.priority < j.priority);
}

am_Error_e CAmPolicyReceive::getListMainConnections(const std::string &name,
    std::vector<gc_ConnectionInfo_s > &listConnectionInfos, gc_Order_e order)
{
    am_Error_e                        result        = E_NOT_POSSIBLE;
    std::shared_ptr<CAmClassElement > pClassElement = nullptr;
    pClassElement = CAmClassFactory::getElement(name);
    if (nullptr != pClassElement)
    {
        _getConnectionInfo(pClassElement, listConnectionInfos);
        switch (order)
        {
        case O_HIGH_PRIORITY:
            std::stable_sort(listConnectionInfos.begin(), listConnectionInfos.end(), _sortingHighest);
            result = E_OK;
            break;
        case O_LOW_PRIORITY:
            std::stable_sort(listConnectionInfos.begin(), listConnectionInfos.end(), _sortingLowest);
            result = E_OK;
            break;
        case O_NEWEST:
        {
            std::vector<gc_ConnectionInfo_s >                   listTempConnectionInfos;
            std::vector<gc_ConnectionInfo_s >::reverse_iterator itListRevTempConnectionInfos;
            listTempConnectionInfos = listConnectionInfos;
            listConnectionInfos.clear();
            for (itListRevTempConnectionInfos = listTempConnectionInfos.rbegin(); itListRevTempConnectionInfos != listTempConnectionInfos.rend(); itListRevTempConnectionInfos++)
            {
                listConnectionInfos.push_back(*itListRevTempConnectionInfos);
            }

            result = E_OK;
            break;
        }
        case O_OLDEST:
            result = E_OK;
            break;
        default:
            break;
        }
    }

    return result;
}

am_Error_e CAmPolicyReceive::getListNotificationConfigurations(
    const gc_Element_e elementType, const std::string &name,
    std::vector<am_NotificationConfiguration_s > &listNotificationConfigurations)
{
    am_Error_e result(E_NOT_POSSIBLE);
    switch (elementType)
    {
    case ET_SOURCE:
    {
        std::shared_ptr<CAmSourceElement > pElement = CAmSourceFactory::getElement(name);
        if (pElement != nullptr)
        {
            result = pElement->getListNotificationConfigurations(listNotificationConfigurations);
        }

        break;
    }
    case ET_SINK:
    {
        std::shared_ptr<CAmSinkElement > pElement = CAmSinkFactory::getElement(name);
        if (pElement != nullptr)
        {
            result = pElement->getListNotificationConfigurations(listNotificationConfigurations);
        }

        break;
    }
    default:
        break;
    }

    return result;
}

am_Error_e CAmPolicyReceive::getListMainNotificationConfigurations(
    const gc_Element_e elementType, const std::string &name,
    std::vector<am_NotificationConfiguration_s > &listMainNotificationConfigurations)
{
    am_Error_e result(E_NOT_POSSIBLE);
    switch (elementType)
    {
    case ET_SOURCE:
    {
        std::shared_ptr<CAmSourceElement > pElement = CAmSourceFactory::getElement(name);
        if (pElement != nullptr)
        {
            result = pElement->getListMainNotificationConfigurations(listMainNotificationConfigurations);
        }

        break;
    }
    case ET_SINK:
    {
        std::shared_ptr<CAmSinkElement > pElement = CAmSinkFactory::getElement(name);
        if (pElement != nullptr)
        {
            result = pElement->getListMainNotificationConfigurations(listMainNotificationConfigurations);
        }

        break;
    }
    default:
        break;
    }

    return result;
}

am_Error_e CAmPolicyReceive::getListMainConnections(
    const gc_Element_e elementType, const std::string &elementName,
    std::vector<gc_ConnectionInfo_s > &listConnectionInfos)
{
    std::shared_ptr<CAmClassElement >                         pClassElement = nullptr;
    std::vector<std::shared_ptr<CAmClassElement > >           listClasses;
    std::vector<std::shared_ptr<CAmClassElement > >::iterator itListClasses;
    am_Error_e                                                result   = E_NOT_POSSIBLE;
    std::shared_ptr<CAmElement >                              pElement = nullptr;

    LOG_FN_ENTRY(__FILENAME__, __func__, elementType);
    switch (elementType)
    {
    case ET_CLASS:
        pClassElement = CAmClassFactory::getElement(elementName);
        if (nullptr != pClassElement)
        {
            _getConnectionInfo(pClassElement, listConnectionInfos);
            result = E_OK;
        }

        break;
    case ET_SOURCE:
    case ET_SINK:
        pElement = _getElement(elementType, elementName);
        LOG_FN_INFO(__FILENAME__, __func__, "_getElement element name:", elementName);
        if (nullptr != pElement)
        {
            if (ET_SOURCE == elementType)
            {
                CAmClassFactory::getElementsBySource(pElement->getName(), listClasses);
            }
            else
            {
                CAmClassFactory::getElementsBySink(pElement->getName(), listClasses);
            }

            for (itListClasses = listClasses.begin(); itListClasses != listClasses.end();
                 itListClasses++)
            {
                _getConnectionInfo(*itListClasses, listConnectionInfos);
            }

            result = E_OK;
        }

        break;
    case ET_CONNECTION:
    {
        std::shared_ptr<CAmMainConnectionElement > pMainConnection = CAmMainConnectionFactory::getElement(elementName);
        if (pMainConnection != nullptr)
        {
            gc_ConnectionInfo_s connection(elementType, pMainConnection->getMainSourceName(),
                    pMainConnection->getMainSinkName());
            connection.priority = pMainConnection->getPriority();
            connection.connectionState = pMainConnection->getState();
            connection.volume = pMainConnection->getVolume();
            listConnectionInfos.push_back(connection);
            result = E_OK;
        }

        break;
    }
    default:
        LOG_FN_WARN(__FILENAME__, __func__, "  class not found with given element type: ", elementType);
        break;
    }

    LOG_FN_EXIT(__FILENAME__, __func__, result);
    return result;
}

void CAmPolicyReceive::_getConnectionInfo(std::shared_ptr<CAmClassElement > pClass,
    std::vector<gc_ConnectionInfo_s > &listConnectionInfo)
{
    if (pClass == nullptr)
    {
        return;
    }

    std::vector<std::shared_ptr<CAmMainConnectionElement > >           listConnections;
    CAmConnectionListFilter filter;
    filter.setClassName(pClass->getName());
    CAmMainConnectionFactory::getListElements(listConnections, filter);

    for (auto pMainConnection : listConnections)
    {
        gc_ConnectionInfo_s connection(ET_CONNECTION, pMainConnection->getMainSourceName(),
                pMainConnection->getMainSinkName());
        connection.priority = pMainConnection->getPriority();
        connection.connectionState = pMainConnection->getState();
        connection.volume = pMainConnection->getVolume();
        listConnectionInfo.push_back(connection);
    }
}

am_Error_e CAmPolicyReceive::setListActions(std::vector<gc_Action_s > &listActions,
    gc_ActionList_e actionListType)
{
    if (listActions.empty())
    {
        return E_NO_CHANGE;
    }

    CAmRootAction   *pRootAction   = CAmRootAction::getInstance();
    CAmPolicyAction *pPolicyAction = new CAmPolicyAction(listActions, mpPolicySend,
            mpControlReceive);
    if (NULL == pPolicyAction)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "  bad memory state");
        return E_NOT_POSSIBLE;
    }

// attach the dynamic action pointer to root
    pRootAction->append(pPolicyAction);
    return E_OK;
}

std::shared_ptr<CAmElement > CAmPolicyReceive::_getElement(const gc_Element_e type,
    const std::string &name)
{
    std::shared_ptr<CAmElement > pElement = nullptr;
    if (type == ET_SINK)
    {
        pElement = CAmSinkFactory::getElement(name);
    }
    else if (type == ET_SOURCE)
    {
        pElement = CAmSourceFactory::getElement(name);
    }
    else if (type == ET_CLASS)
    {
        pElement = CAmClassFactory::getElement(name);
    }
    else if (type == ET_GATEWAY)
    {
        pElement = CAmGatewayFactory::getElement(name);
    }
    else if (type == ET_CONNECTION)
    {
        pElement = CAmMainConnectionFactory::getElement(name);
    }
    else if (type == ET_DOMAIN)
    {
        pElement = CAmDomainFactory::getElement(name);
    }

    return pElement;
}

am_Error_e CAmPolicyReceive::getDomainInfoByID(const am_domainID_t domainID, am_Domain_s &domainInfo)
{
    auto pDomain = CAmDomainFactory::getElement(domainID);
    if (pDomain != nullptr)
    {
        return pDomain->getDomainInfo(domainInfo);
    }

    return E_UNKNOWN;
}

am_Error_e CAmPolicyReceive::getListGatewaysOfDomain(
    const am_domainID_t domainID, std::vector<am_gatewayID_t > &listGatewaysIDs) const
{
    return mpControlReceive->getListGatewaysOfDomain(domainID, listGatewaysIDs);
}

am_Error_e CAmPolicyReceive::getListSinksOfDomain(const am_domainID_t domainID,
    std::vector<am_sinkID_t > &listSinkIDs) const
{
    return mpControlReceive->getListSinksOfDomain(domainID, listSinkIDs);
}

am_Error_e CAmPolicyReceive::getListSourcesOfDomain(
    const am_domainID_t domainID, std::vector<am_sourceID_t > &listSourceIDs) const
{
    return mpControlReceive->getListSourcesOfDomain(domainID, listSourceIDs);
}

}
/* namespace gc */
} /* namespace am */
