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
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

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

CAmPolicyReceive::CAmPolicyReceive(CAmControlReceive* pControlReceive, IAmPolicySend* pPolicySend) :
                                mpControlReceive(pControlReceive),
                                mpPolicySend(pPolicySend)
{
}

bool CAmPolicyReceive::isDomainRegistrationComplete(const std::string& domainName)
{
    std::vector < am_Domain_s > listDomains;
    std::vector<am_Domain_s >::iterator itListDomains;
    if (E_OK == mpControlReceive->getListElements(listDomains))
    {
        for (itListDomains = listDomains.begin(); itListDomains != listDomains.end();
                        itListDomains++)
        {
            if ((*itListDomains).name == domainName)
            {
                LOG_FN_DEBUG("  domain found ", (*itListDomains).complete);
                return (*itListDomains).complete;
            }
        }
    }
    return false;
}

bool CAmPolicyReceive::isRegistered(const gc_Element_e type, const std::string& name)
{
    return (_getElement(type, name) != NULL) ? true : false;
}

am_Error_e CAmPolicyReceive::getAvailability(const gc_Element_e type, const std::string& name,
                                             am_Availability_s& availability)
{
    am_Error_e result(E_NOT_POSSIBLE);
    CAmElement* pElement = _getElement(type, name);
    if (pElement != NULL)
    {
        result = pElement->getAvailability(availability);
    }
    return result;
}

am_Error_e CAmPolicyReceive::getState(const gc_Element_e type, const std::string& name, int& state)
{
    am_Error_e result(E_NOT_POSSIBLE);
    CAmElement* pElement = _getElement(type, name);
    if (pElement != NULL)
    {
        result = pElement->getState(state);
    }
    return result;
}

am_Error_e CAmPolicyReceive::getInterruptState(const gc_Element_e type, const std::string& name,
                                               am_InterruptState_e& interruptState)
{
    am_Error_e result(E_NOT_POSSIBLE);
    CAmElement* pElement = _getElement(type, name);
    if (pElement != NULL)
    {
        result = pElement->getInterruptState(interruptState);
    }
    return result;
}

am_Error_e CAmPolicyReceive::getMuteState(const gc_Element_e type, const std::string& name,
                                          am_MuteState_e& muteState)
{
    am_Error_e result(E_NOT_POSSIBLE);
    CAmElement* pElement = _getElement(type, name);
    if (pElement != NULL)
    {
        result = pElement->getMuteState(muteState);
    }
    return result;
}

am_Error_e CAmPolicyReceive::getSoundProperty(const gc_Element_e elementType,
                                              const std::string& elementName,
                                              const am_CustomSoundPropertyType_t propertyType,
                                              int16_t &propertyValue)
{
    am_Error_e result(E_NOT_POSSIBLE);
    if (elementType == ET_SOURCE)
    {
        CAmSourceElement* pElement = CAmSourceFactory::getElement(elementName);
        if (pElement != NULL)
        {
            result = pElement->getSoundPropertyValue(propertyType, propertyValue);
        }
    }
    else if (elementType == ET_SINK)
    {
        CAmSinkElement* pElement = CAmSinkFactory::getElement(elementName);
        if (pElement != NULL)
        {
            result = pElement->getSoundPropertyValue(propertyType, propertyValue);
        }
    }
    return result;
}

am_Error_e CAmPolicyReceive::getMainSoundProperty(const gc_Element_e elementType,
                                                  const std::string& elementName,
                                                  const am_CustomMainSoundPropertyType_t propertyType,
                                                  int16_t& propertyValue)
{
    am_Error_e result(E_NOT_POSSIBLE);
    if (elementType == ET_SOURCE)
    {
        CAmSourceElement* pElement = CAmSourceFactory::getElement(elementName);
        if (pElement != NULL)
        {
            result = pElement->getMainSoundPropertyValue(propertyType, propertyValue);
        }
    }
    else if (elementType == ET_SINK)
    {
        CAmSinkElement* pElement = CAmSinkFactory::getElement(elementName);
        if (pElement != NULL)
        {
            result = pElement->getMainSoundPropertyValue(propertyType, propertyValue);
        }
    }
    return result;
}

am_Error_e CAmPolicyReceive::getSystemProperty(const am_CustomSystemPropertyType_t systemPropertyType,
                                               int16_t& value)
{
    std::vector<am_SystemProperty_s> listSystemProperties;
    std::vector<am_SystemProperty_s>::iterator itListSystemProperties;
    //get the list of system properties from database
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
                                       const std::string& elementName,
                                       am_volume_t& volume)
{
    am_Error_e result(E_NOT_POSSIBLE);
    CAmElement* pElement = _getElement(elementType, elementName);
    if (pElement != NULL)
    {
        result = pElement->getVolume(volume);
    }
    return result;
}

am_Error_e CAmPolicyReceive::getMainVolume(const gc_Element_e elementType,
                                           const std::string& elementName,
                                           am_mainVolume_t& mainVolume)
{
    am_Error_e result(E_NOT_POSSIBLE);
    if(ET_SOURCE != elementType)
    {
        CAmElement* pElement = _getElement(elementType, elementName);
        if (pElement != NULL)
        {
            result = pElement->getMainVolume(mainVolume);
        }
    }
    return result;
}

am_Error_e CAmPolicyReceive::getListMainConnections(
                const gc_Element_e elementType, const std::string& elementName,
                std::vector<gc_ConnectionInfo_s >& listConnectionInfo)
{
    CAmClassElement* pClassElement;
    std::vector<CAmClassElement* > listClasses;
    std::vector<CAmClassElement* >::iterator itListClasses;
    am_Error_e result = E_NOT_POSSIBLE;
    CAmElement* pElement;

    LOG_FN_ENTRY(elementType);
    switch (elementType)
    {
    case ET_CLASS:
        pClassElement = CAmClassFactory::getElement(elementName);
        if (NULL != pClassElement)
        {
            _getConnectionInfo(pClassElement, listConnectionInfo);
            result = E_OK;
        }
        break;
    case ET_SOURCE:
    case ET_SINK:
        pElement = _getElement(elementType, elementName);
        if (NULL != pElement)
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
                _getConnectionInfo(*itListClasses, listConnectionInfo);
            }
            result = E_OK;
        }
        break;
    case ET_CONNECTION:
    {
        std::vector<CAmMainConnectionElement* > listConnections;
        std::vector<CAmMainConnectionElement* >::iterator itListMainConnections;
        pClassElement = CAmClassFactory::getClassElement(elementName);
        if (NULL != pClassElement)
        {
            pClassElement->getListMainConnections(listConnections);
            for (itListMainConnections = listConnections.begin();
                            itListMainConnections != listConnections.end(); itListMainConnections++)
            {
                //compare all the connection with given connection name
                if ((*itListMainConnections)->getName() == elementName)
                {
                    gc_ConnectionInfo_s connection;
                    am_volume_t volume;
                    int32_t priority;
                    int state;
                    connection.sinkName = (*itListMainConnections)->getMainSinkName();
                    connection.sourceName = (*itListMainConnections)->getMainSourceName();
                    (*itListMainConnections)->getPriority(priority);
                    connection.priority = priority;
                    (*itListMainConnections)->getState(state);
                    connection.connectionState = (am_ConnectionState_e)state;
                    (*itListMainConnections)->getVolume(volume);
                    connection.volume = volume;
                    listConnectionInfo.push_back(connection);
                    result = E_OK;
                    break;
                }
            }
        }
    }
        break;
    default:
        LOG_FN_DEBUG("  class not found with given element type: ", elementType);
        break;
    }
    LOG_FN_EXIT(result);
    return result;
}

void CAmPolicyReceive::_getConnectionInfo(CAmClassElement* pClass,
                                          std::vector<gc_ConnectionInfo_s >& listConnectionInfo)
{
    std::vector<CAmMainConnectionElement* > listConnections;
    std::vector<CAmMainConnectionElement* >::iterator itListMainConnections;
    gc_ConnectionInfo_s connection;
    am_volume_t volume;
    int32_t priority;
    int state;
    pClass->getListMainConnections(listConnections);
    for (itListMainConnections = listConnections.begin();
                    itListMainConnections != listConnections.end(); itListMainConnections++)
    {

        connection.sinkName = (*itListMainConnections)->getMainSinkName();
        connection.sourceName = (*itListMainConnections)->getMainSourceName();
        (*itListMainConnections)->getPriority(priority);
        connection.priority = priority;
        (*itListMainConnections)->getState(state);
        connection.connectionState = (am_ConnectionState_e)state;
        (*itListMainConnections)->getVolume(volume);
        connection.volume = volume;
        listConnectionInfo.push_back(connection);
    }
}

am_Error_e CAmPolicyReceive::setListActions(std::vector<gc_Action_s >& listActions,
                                            gc_ActionList_e actionListType)
{
    if (listActions.empty())
    {
        return E_NO_CHANGE;
    }
    CAmRootAction* pRootAction = CAmRootAction::getInstance();
    CAmPolicyAction* pPolicyAction = new CAmPolicyAction(listActions, mpPolicySend,
                                                         mpControlReceive);
    if (NULL == pPolicyAction)
    {
        LOG_FN_ERROR("  bad memory state");
        return E_NOT_POSSIBLE;
    }
//attach the dynamic action pointer to root
    pRootAction->append(pPolicyAction);
    return E_OK;
}

CAmElement* CAmPolicyReceive::_getElement(const gc_Element_e type, const std::string& name)
{
    CAmElement* pElement(NULL);
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

}
/* namespace gc */
} /* namespace am */
