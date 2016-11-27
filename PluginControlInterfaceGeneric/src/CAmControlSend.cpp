/******************************************************************************
 * @file: CAmControlSend.cpp
 *
 * This file contains the definition of control send class (member functions
 * and data members) used to implement the interface of AM daemon to allow the
 * hook from application and routing adaptor to reach the controller
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

#include "CAmControlSend.h"
#include "CAmRootAction.h"
#include "CAmLogger.h"
#include "CAmDomainElement.h"
#include "CAmSinkElement.h"
#include "CAmGatewayElement.h"
#include "CAmClassElement.h"
#include "CAmMainConnectionElement.h"
#include "CAmTimerEvent.h"
#include <cassert>
#include "CAmSourceElement.h"
#include "IAmPolicySend.h"
#include "CAmPolicyReceive.h"

DltContext GenericControllerDlt;
namespace am {
namespace gc {
extern "C" IAmPolicySend* createPolicySendInterface();

extern "C" IAmControlSend* PluginControlInterfaceGenericFactory()
{
    return (new CAmControlSend());
}

extern "C" void destroyPluginControlInterfaceGeneric(IAmControlSend* controlSendInterface)
{
    delete controlSendInterface;
}

CAmControlSend::CAmControlSend() :
                                mpControlReceive(NULL),
                                mpPolicySend(NULL)
{
}

CAmControlSend::~CAmControlSend()
{
    _freeMemory();
}

void CAmControlSend::getInterfaceVersion(std::string& version) const
{
    version = ControlVersion;
}

am_Error_e CAmControlSend::startupController(IAmControlReceive* controlReceiveInterface)
{
    CAmSocketHandler* pSocketHandler;
    am_Error_e result;

    assert(controlReceiveInterface);
    // Register context with DLT for logging purpose.
    LOG_FN_REGISTER_CONTEXT();

    LOG_FN_ENTRY("A new context registered with DLT");

    // get socket handler context
    result = controlReceiveInterface->getSocketHandler(pSocketHandler);
    if ((NULL == pSocketHandler) || (E_OK != result))
    {
        LOG_FN_ERROR(" Failed to get socket handler from AM", result);
        return (E_NOT_POSSIBLE);
    }

    // create timer object
    CAmTimerEvent* mpTimerEvent = CAmTimerEvent::getInstance();
    if (NULL == mpTimerEvent)
    {
        LOG_FN_ERROR(" Not able to create Timer object");
        return (E_NOT_POSSIBLE);
    }
    mpTimerEvent->setSocketHandle(pSocketHandler, this);

    /**
     * Startup sequence should be as follows
     * 1. Instantiate Control Receive
     * 2. Instantiate Policy Send
     * 3. Instantiate Policy receive
     */
    mpControlReceive = new gc::CAmControlReceive(controlReceiveInterface);
    if (NULL == mpControlReceive)
    {
        _freeMemory();
        return (E_NOT_POSSIBLE);
    }

    mpPolicySend = createPolicySendInterface();
    if (NULL == mpPolicySend)
    {
        _freeMemory();
        LOG_FN_ERROR(" Not able to create Policy Engine Receive object");
        return (E_NOT_POSSIBLE);
    }

    IAmPolicyReceive* pPolicyReceive = new CAmPolicyReceive(mpControlReceive, mpPolicySend);
    if (NULL == pPolicyReceive)
    {
        _freeMemory();
        LOG_FN_ERROR(" Not able to create Policy Engine Receive object");
        return (E_NOT_POSSIBLE);
    }

    result = mpPolicySend->startupInterface(pPolicyReceive);
    if (result != E_OK)
    {
        _freeMemory();
        LOG_FN_ERROR(" failed to start policy interface");
        return (E_NOT_POSSIBLE);
    }
    /*
     * register system properties and the list classes
     */
    std::vector<am_SystemProperty_s> listSystemProperties;
    if (E_OK == mpPolicySend->getListSystemProperties(listSystemProperties))
    {
        if (listSystemProperties.size() > 0)
        {
            mpControlReceive->enterSystemPropertiesListDB(listSystemProperties);
        }
    }
    std::vector<gc_Class_s> listClasses;
    std::vector<gc_Class_s>::iterator itListClasses;
    CAmClassElement* pClassElement;
    if (E_OK == mpPolicySend->getListClasses(listClasses))
    {
        for (itListClasses = listClasses.begin(); itListClasses != listClasses.end();
                        itListClasses++)
        {
            // create new class object for each class
            pClassElement = CAmClassFactory::createElement((*itListClasses),
                                                           mpControlReceive);
            if (NULL == pClassElement)
            {
                LOG_FN_ERROR(" not able to create new class object for class=",
                             itListClasses->name);
                return (E_NOT_POSSIBLE);
            }
        }
    }
    iterateActions();
    return E_OK;
}

void CAmControlSend::setControllerReady()
{
    assert(NULL != mpControlReceive);

    mpControlReceive->setCommandReady();
    mpControlReceive->setRoutingReady();
    LOG_FN_INFO(" Finished");
}

void CAmControlSend::setControllerRundown(const int16_t amSignal)
{
    assert(NULL != mpControlReceive);

    if (2 == amSignal)
    {
        mpControlReceive->confirmControllerRundown(E_UNKNOWN);
    }
    mpControlReceive->confirmControllerRundown(E_OK);

    LOG_FN_INFO("  Finished");
}

am_Error_e CAmControlSend::hookUserConnectionRequest(const am_sourceID_t sourceID,
                                                     const am_sinkID_t sinkID,
                                                     am_mainConnectionID_t& mainConnectionID)
{
    CAmSinkElement* pSinkElement;
    CAmSourceElement* pSourceElement;
    CAmClassElement* pClassElement;
    am_Error_e result;
    std::vector < am_Route_s > listRoutes;
    LOG_FN_ENTRY(sourceID, sinkID);
    pSourceElement = CAmSourceFactory::getElement(sourceID);
    pSinkElement = CAmSinkFactory::getElement(sinkID);
    if (pSinkElement == NULL || pSourceElement == NULL)
    {
        return E_NON_EXISTENT;
    }
    mainConnectionID = 0;
    pClassElement = CAmClassFactory::getClassElement(pSourceElement->getName(),
                                                     pSinkElement->getName());
    if (NULL == pClassElement)
    {
        LOG_FN_ERROR("  got failed.");
        return (E_NOT_POSSIBLE);
    }
    result = mpControlReceive->getRoute(false, pSourceElement->getID(), pSinkElement->getID(),
                                        listRoutes);
    if (E_OK != result)
    {
        LOG_FN_ERROR(" getRoute returned error:", result);
        return result;
    }
    if (true == listRoutes.empty())
    {
        LOG_FN_ERROR("  couldn't getRoute. The list is empty");
        return (E_NOT_POSSIBLE);
    }
    result = pClassElement->createMainConnection(pSourceElement->getName(), pSinkElement->getName(),
                                                 mainConnectionID);
    if ((result != E_OK) && (result != E_ALREADY_EXISTS))
    {
        LOG_FN_ERROR("Failed to create connection");
        return result;
    }
    result = E_OK;
    gc_ConnectTrigger_s* pconnectTrigger = new gc_ConnectTrigger_s;
    if (NULL == pconnectTrigger)
    {
        return E_NOT_POSSIBLE;
    }
    pconnectTrigger->className = pClassElement->getName();
    pconnectTrigger->sinkName = pSinkElement->getName();
    pconnectTrigger->sourceName = pSourceElement->getName();
    mlistTrigger.push_back(std::make_pair(USER_CONNECTION_REQUEST, pconnectTrigger));

    iterateActions();
    return result;
}

am_Error_e CAmControlSend::hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID)
{
    CAmMainConnectionElement* pMainConnection;
    CAmClassElement* pClassElement;

    pMainConnection = CAmMainConnectionFactory::getElement(connectionID);
    if (NULL == pMainConnection)
    {
        LOG_FN_ERROR("  no such connection:", connectionID);
        return E_NOT_POSSIBLE;
    }
    LOG_FN_INFO(pMainConnection->getName(), pMainConnection->getID());
    pClassElement = CAmClassFactory::getClassElement(pMainConnection->getName());
    if (NULL == pClassElement)
    {
        LOG_FN_ERROR("  no such class:", connectionID);
        return E_NOT_POSSIBLE;
    }
    // Store the trigger in a Queue
    gc_DisconnectTrigger_s* triggerData = new gc_DisconnectTrigger_s;
    if (NULL == triggerData)
    {
        LOG_FN_ERROR("  bad memory state:", connectionID);
        return E_NOT_POSSIBLE;
    }
    triggerData->className = pClassElement->getName();
    triggerData->sourceName = pMainConnection->getMainSourceName();
    triggerData->sinkName = pMainConnection->getMainSinkName();
    mlistTrigger.push_back(std::make_pair(USER_DISCONNECTION_REQUEST, triggerData));
    iterateActions();
    return E_OK;
}

am_Error_e CAmControlSend::hookUserSetMainSinkSoundProperty(
                const am_sinkID_t sinkID, const am_MainSoundProperty_s& soundProperty)
{
    CAmSinkElement* pElement;
    am_Error_e result;
    am_MainSoundProperty_s localMainSoundProperty;

    LOG_FN_ENTRY(sinkID);
    localMainSoundProperty = soundProperty;

    // check if the sinkID is valid
    pElement = CAmSinkFactory::getElement(sinkID);
    if (NULL == pElement)
    {
        LOG_FN_ERROR("  Not able to get sink element");
        return (E_OUT_OF_RANGE);
    }

    //Check the SP Type and Range
    // Check the SP range for requested sink from configuration and return with
    // same value if soundProperty is within the range else return with saturated min/max value.
    result = pElement->saturateMainSoundPropertyRange(localMainSoundProperty.type,
                                                      localMainSoundProperty.value);

    if (E_OK != result)
    {
        LOG_FN_ERROR("  Not able to saturate property. Type->value", localMainSoundProperty.type,
                     localMainSoundProperty.value);
        return result;
    }
    // Store the trigger in a Queue
    gc_SinkSoundPropertyTrigger_s* triggerData = new gc_SinkSoundPropertyTrigger_s;
    if (NULL == triggerData)
    {
        LOG_FN_ERROR("  bad memory state:");
        return E_NOT_POSSIBLE;
    }
    triggerData->sinkName = pElement->getName();
    triggerData->mainSoundProperty.type = localMainSoundProperty.type;
    triggerData->mainSoundProperty.value = localMainSoundProperty.value;
    mlistTrigger.push_back(std::make_pair(USER_SET_SINK_MAIN_SOUND_PROPERTY, triggerData));
    LOG_FN_EXIT();
    iterateActions();
    return (E_OK);
}

am_Error_e CAmControlSend::hookUserSetMainSourceSoundProperty(
                const am_sourceID_t sourceID, const am_MainSoundProperty_s& soundProperty)
{
    CAmSourceElement* pElement;
    am_Error_e result;
    am_MainSoundProperty_s localMainSoundProperty;

    LOG_FN_ENTRY(sourceID);
    localMainSoundProperty = soundProperty;

    // check if the sourceID is valid
    pElement = CAmSourceFactory::getElement(sourceID);
    if (NULL == pElement)
    {
        LOG_FN_ERROR("  Not able to get source element");
        return (E_OUT_OF_RANGE);
    }

    //Check the SP Type and Range
    // Check the SP range for requested source from configuration and return with
    // same value if soundProperty is within the range else return with saturated min/max value.
    result = pElement->saturateMainSoundPropertyRange(localMainSoundProperty.type,
                                                      localMainSoundProperty.value);

    if (E_OK != result)
    {
        LOG_FN_ERROR("  Not able to saturate property. Type->value", localMainSoundProperty.type,
                     localMainSoundProperty.value);
        return result;
    }

    // Store the trigger in a Queue
    gc_SourceSoundPropertyTrigger_s* triggerData = new gc_SourceSoundPropertyTrigger_s;
    if (NULL == triggerData)
    {
        LOG_FN_ERROR("  bad memory state:");
        return E_NOT_POSSIBLE;
    }
    triggerData->sourceName = pElement->getName();
    triggerData->mainSoundProperty.type = localMainSoundProperty.type;
    triggerData->mainSoundProperty.value = localMainSoundProperty.value;
    mlistTrigger.push_back(std::make_pair(USER_SET_SOURCE_MAIN_SOUND_PROPERTY, triggerData));
    iterateActions();
    LOG_FN_EXIT();
    return (E_OK);
}

am_Error_e CAmControlSend::hookUserSetSystemProperty(const am_SystemProperty_s& property)
{
    LOG_FN_ENTRY();
    am_Error_e result;

    result = mpControlReceive->changeSystemPropertyDB(property);
    if (result != E_OK)
    {
        return result;
    }
    // Store the trigger in a Queue
    gc_SystemPropertyTrigger_s* triggerData = new gc_SystemPropertyTrigger_s;
    if (NULL == triggerData)
    {
        LOG_FN_ERROR("  bad memory state:");
        return E_NOT_POSSIBLE;
    }
    triggerData->systemProperty = property;
    mlistTrigger.push_back(std::make_pair(USER_SET_SYSTEM_PROPERTY, triggerData));
    iterateActions();
    LOG_FN_EXIT();
    return (E_OK);

}

am_Error_e CAmControlSend::hookUserVolumeChange(const am_sinkID_t sinkID,
                                                const am_mainVolume_t mainVolume)
{
    LOG_FN_ENTRY(sinkID, mainVolume);
    am_Error_e result(E_NOT_POSSIBLE);

    CAmSinkElement* pElement = CAmSinkFactory::getElement(sinkID);
    if (NULL == pElement)
    {
        LOG_FN_ERROR("  parameter error");
        return E_OUT_OF_RANGE;
    }

    // Store the trigger in a Queue
    gc_SinkVolumeChangeTrigger_s* triggerData = new gc_SinkVolumeChangeTrigger_s;
    if (NULL == triggerData)
    {
        LOG_FN_ERROR("  bad memory state:");
        return E_NOT_POSSIBLE;
    }

    triggerData->sinkName = pElement->getName();
    triggerData->volume = mainVolume;
    mlistTrigger.push_back(std::make_pair(USER_SET_VOLUME, triggerData));
    iterateActions();
    LOG_FN_EXIT();
    return E_OK;
}

am_Error_e CAmControlSend::hookUserVolumeStep(const am_sinkID_t sinkID, const int16_t increment)
{
    LOG_FN_ENTRY(sinkID, increment);

    CAmSinkElement* pElement = CAmSinkFactory::getElement(sinkID);
    if (NULL == pElement)
    {
        LOG_FN_ERROR("  parameter error");
        return (E_OUT_OF_RANGE);
    }

    //get the user volume and add increment point in it.
    am_mainVolume_t mainVolume;
    pElement->getMainVolume(mainVolume);
    mainVolume+=increment;
    // Store the trigger in a Queue
    gc_SinkVolumeChangeTrigger_s* triggerData = new gc_SinkVolumeChangeTrigger_s;
    if (NULL == triggerData)
    {
        LOG_FN_ERROR("  bad memory state:");
        return E_NOT_POSSIBLE;
    }
    triggerData->sinkName = pElement->getName();
    triggerData->volume = mainVolume;
    mlistTrigger.push_back(std::make_pair(USER_SET_VOLUME, triggerData));
    iterateActions();
    LOG_FN_EXIT();
    return (E_OK);
}

am_Error_e CAmControlSend::hookUserSetSinkMuteState(const am_sinkID_t sinkID,
                                                    const am_MuteState_e muteState)
{
    CAmElement* pElement;

    LOG_FN_ENTRY(sinkID);
    // check parameter
    if ((MS_MUTED != muteState) && (MS_UNMUTED != muteState))
    {
        LOG_FN_ERROR("  parameter error");
        return (E_OUT_OF_RANGE);
    }

    // check if the sinkID is valid
    pElement = CAmSinkFactory::getElement(sinkID);
    if (NULL == pElement)
    {
        LOG_FN_ERROR("  Not able to get sink element");
        return (E_OUT_OF_RANGE);
    }
    // Store the trigger in a Queue
    gc_SinkMuteTrigger_s* triggerData = new gc_SinkMuteTrigger_s;
    if (NULL == triggerData)
    {
        LOG_FN_ERROR("  bad memory state:");
        return E_NOT_POSSIBLE;
    }
    triggerData->sinkName = pElement->getName();
    triggerData->muteState = muteState;
    //mute state will be taken care at policy send side.
    mlistTrigger.push_back(std::make_pair(USER_SET_SINK_MUTE_STATE, triggerData));
    iterateActions();
    LOG_FN_EXIT();
    return (E_OK);
}

am_Error_e CAmControlSend::hookSystemRegisterDomain(const am_Domain_s& domainData,
                                                    am_domainID_t& domainID)
{
    LOG_FN_ENTRY(domainData.name);
    am_Error_e result(E_NOT_POSSIBLE);
    std::vector < gc_Domain_s > listDomainInfo;
    std::vector < std::string > listNames {domainData.name};
    result = mpPolicySend->getListElements(listNames, listDomainInfo);
    if (E_OK == result)
    {
        gc_Domain_s domainInfo = listDomainInfo.front();
        domainInfo.domainID = 0;
        domainInfo.busname = domainData.busname;
        domainInfo.nodename = domainData.nodename;
        domainInfo.early = domainData.early;
        domainInfo.complete = domainData.complete;
        domainInfo.state = domainData.state;
        CAmElement* pElement = CAmDomainFactory::createElement(domainInfo, mpControlReceive);
        if (pElement != NULL)
        {
            domainID = pElement->getID();
            gc_RegisterElementTrigger_s* pregisterdomainTrigger = new gc_RegisterElementTrigger_s;
            if (pregisterdomainTrigger == NULL)
            {
                return E_NOT_POSSIBLE;
            }
            pregisterdomainTrigger->elementName = domainData.name;
            pregisterdomainTrigger->RegisterationStatus = result;
            mlistTrigger.push_back(std::make_pair(SYSTEM_REGISTER_DOMAIN, pregisterdomainTrigger));
            LOG_FN_INFO("  registered domain Name->result =", domainData.name, result);
            result = E_OK;
        }
    }
    iterateActions();
    LOG_FN_EXIT();
    return result;
}

am_Error_e CAmControlSend::hookSystemDeregisterDomain(const am_domainID_t domainID)
{
    am_Error_e result;
    std::vector < am_sinkID_t > listSinkIDs;
    std::vector<am_sinkID_t >::iterator itListSinkIDs;
    std::vector < am_sourceID_t > listSourceIDs;
    std::vector<am_sourceID_t >::iterator itListSourceIDs;
    std::vector < am_gatewayID_t > listGatewaysIDs;
    std::vector<am_gatewayID_t >::iterator itListGatewayIDs;
    am_Domain_s DomainInfo;
    LOG_FN_ENTRY(domainID);

    result = mpControlReceive->getDomainInfoByID(domainID, DomainInfo);
    if (E_OK == result)
    {
        //Get the list of gatewayID belongs to specific domain and also unregister them from the AM DB.
        result = mpControlReceive->getListGatewaysOfDomain(domainID, listGatewaysIDs);
        if ((E_OK == result) && (false == listGatewaysIDs.empty()))
        {
            for (itListGatewayIDs = listGatewaysIDs.begin();
                            itListGatewayIDs != listGatewaysIDs.end(); itListGatewayIDs++)
            {
                CAmGatewayFactory::destroyElement(*itListGatewayIDs);
            }
        }
        //Get the list of sinkID belongs to specific domain and also unregister them from the AM DB.
        result = mpControlReceive->getListSinksOfDomain(domainID, listSinkIDs);
        if ((E_OK == result) && (false == listSinkIDs.empty()))
        {
            for (itListSinkIDs = listSinkIDs.begin(); itListSinkIDs != listSinkIDs.end();
                            itListSinkIDs++)
            {
                CAmSinkFactory::destroyElement(*itListSinkIDs);
            }
        }
        //Get the list of sourceID belongs to specific domain and also unregister them from the AM DB.
        result = mpControlReceive->getListSourcesOfDomain(domainID, listSourceIDs);
        if ((E_OK == result) && (false == listSourceIDs.empty()))
        {
            for (itListSourceIDs = listSourceIDs.begin(); itListSourceIDs != listSourceIDs.end();
                            itListSourceIDs++)
            {
                CAmSourceFactory::destroyElement(*itListSourceIDs);
            }
        }

        result = CAmDomainFactory::destroyElement(domainID);
        if (E_OK == result)
        {
            gc_UnRegisterElementTrigger_s* unRegisterTrigger = new gc_UnRegisterElementTrigger_s;
            if (NULL == unRegisterTrigger)
            {
                LOG_FN_ERROR("  bad memory state:");
                return E_NOT_POSSIBLE;
            }
            unRegisterTrigger->elementName = DomainInfo.name;
            unRegisterTrigger->unRegisterationStatus = result;
            mlistTrigger.push_back(std::make_pair(SYSTEM_DEREGISTER_DOMAIN, unRegisterTrigger));
        }
    }
    LOG_FN_EXIT(" Error:", result);
    iterateActions();
    return result;

}

void CAmControlSend::hookSystemDomainRegistrationComplete(const am_domainID_t domainID)
{
    LOG_FN_INFO();
    am_Domain_s domainInfo;
    am_Error_e result;
    LOG_FN_ENTRY(domainID);
    if ((NULL == mpControlReceive))
    {
        LOG_FN_ERROR("Database handler error ");
        return;
    }
    result = mpControlReceive->getDomainInfoByID(domainID, domainInfo);
    if (E_OK == result)
    {
        domainInfo.complete = true;
        domainInfo.domainID = 0;
        mpControlReceive->enterDomainDB(domainInfo, domainInfo.domainID);
        gc_DomainRegisterationCompleteTrigger_s* registrationCompleteTrigger = new gc_DomainRegisterationCompleteTrigger_s;
        if (NULL == registrationCompleteTrigger)
        {
            LOG_FN_ERROR("  bad memory state:");
            return;
        }
        registrationCompleteTrigger->domainName = domainInfo.name;
        mlistTrigger.push_back(
                        std::make_pair(SYSTEM_DOMAIN_REGISTRATION_COMPLETE,
                                       registrationCompleteTrigger));
    }
    iterateActions();
    LOG_FN_EXIT();
    return;
}

am_Error_e CAmControlSend::hookSystemRegisterSink(const am_Sink_s& sinkData, am_sinkID_t& sinkID)
{
    am_Error_e result = E_NOT_POSSIBLE;
    CAmElement* pElement;
    std::vector < gc_Sink_s > listSinks;
    std::vector < std::string > listNames;
    gc_Sink_s sinkInfo;

    listNames.push_back(sinkData.name);
    LOG_FN_ENTRY(" sink=", sinkData.name);
    // First check with the policy engine if this sink is allowed
    if (E_OK == mpPolicySend->getListElements(listNames, listSinks))
    {
        /*
         * over write the configuration data with the dynamic data
         */
        sinkInfo = listSinks.front();
        sinkInfo.domainID = sinkData.domainID;
        sinkInfo.name = sinkData.name;
        sinkInfo.volume = sinkData.volume;
        sinkInfo.visible = sinkData.visible;
        sinkInfo.available = sinkData.available;
        sinkInfo.listSoundProperties = sinkData.listSoundProperties;
        sinkInfo.listConnectionFormats = sinkData.listConnectionFormats;
        sinkInfo.listMainSoundProperties = sinkData.listMainSoundProperties;
        sinkInfo.listMainNotificationConfigurations = sinkData.listMainNotificationConfigurations;
        sinkInfo.listNotificationConfigurations = sinkData.listNotificationConfigurations;
        sinkInfo.sinkID = sinkData.sinkID;
        sinkInfo.sinkClassID = sinkData.sinkClassID;
        sinkInfo.muteState = sinkData.muteState;
        sinkInfo.mainVolume = sinkData.mainVolume;
        // This sink might have been created during the gateway registration.Please check locally.
        pElement = CAmSinkFactory::createElement(sinkInfo, mpControlReceive);
        if (NULL != pElement)
        {
            sinkID = pElement->getID();
            result = E_OK;
        }
        else
        {
            LOG_FN_INFO(" Not able to create sink element", sinkData.name);
            result = E_NOT_POSSIBLE;
        }
        gc_RegisterElementTrigger_s* registerTrigger = new gc_RegisterElementTrigger_s;
        if (NULL == registerTrigger)
        {
            LOG_FN_ERROR("  bad memory state:");
            return E_NOT_POSSIBLE;
        }
        registerTrigger->elementName = sinkData.name;
        registerTrigger->RegisterationStatus = result;
        mlistTrigger.push_back(std::make_pair(SYSTEM_REGISTER_SINK, registerTrigger));
        LOG_FN_INFO("  registered sink name:result=", sinkData.name, result);

    }
    LOG_FN_EXIT();
    iterateActions();
    return result;
}

am_Error_e CAmControlSend::hookSystemDeregisterSink(const am_sinkID_t sinkID)
{
    am_Error_e result;
    std::string name;
    CAmElement* pElement;

    LOG_FN_ENTRY(sinkID);
    pElement = CAmSinkFactory::getElement(sinkID);
    if (pElement == NULL)
    {
        LOG_FN_ERROR("Invalid Sink Element ");
        return E_NOT_POSSIBLE;
    }
    name = pElement->getName();
    result = CAmSinkFactory::destroyElement(sinkID);
    // remove the sink from DB
    if (E_OK == result)
    {
        gc_UnRegisterElementTrigger_s* unRegisterTrigger = new gc_UnRegisterElementTrigger_s;
        if (NULL == unRegisterTrigger)
        {
            LOG_FN_ERROR("  bad memory state:");
            return E_NOT_POSSIBLE;
        }
        unRegisterTrigger->elementName = name;
        unRegisterTrigger->unRegisterationStatus = E_OK;
        mlistTrigger.push_back(std::make_pair(SYSTEM_DEREGISTER_SINK, unRegisterTrigger));
        LOG_FN_INFO("  unregistered sink =", sinkID);
    }
    else
    {
        LOG_FN_ERROR(" Error while removing sink from DB. sinkId:Error", sinkID, result);
    }
    LOG_FN_EXIT();
    iterateActions();
    return result;
}

am_Error_e CAmControlSend::hookSystemRegisterSource(const am_Source_s& sourceData,
                                                    am_sourceID_t& sourceID)
{
    am_Error_e result = E_NOT_POSSIBLE;
    CAmElement* pElement;
    gc_Source_s sourceInfo;
    std::vector < gc_Source_s > listSources;
    std::vector < std::string > listNames;
    LOG_FN_ENTRY(" source=", sourceData.name);
    listNames.push_back(sourceData.name);
    // First check with the policy engine if this sink is allowed
    if (E_OK == mpPolicySend->getListElements(listNames, listSources))
    {
        sourceInfo = listSources.front();
        sourceInfo.domainID = sourceData.domainID;
        sourceInfo.name = sourceData.name;
        sourceInfo.volume = sourceData.volume;
        sourceInfo.visible = sourceData.visible;
        sourceInfo.available = sourceData.available;
        sourceInfo.listSoundProperties = sourceData.listSoundProperties;
        sourceInfo.listConnectionFormats = sourceData.listConnectionFormats;
        sourceInfo.listMainSoundProperties = sourceData.listMainSoundProperties;
        sourceInfo.listMainNotificationConfigurations = sourceData.listMainNotificationConfigurations;
        sourceInfo.listNotificationConfigurations = sourceData.listNotificationConfigurations;
        sourceInfo.sourceID = sourceData.sourceID;
        sourceInfo.sourceClassID = sourceData.sourceClassID;
        sourceInfo.sourceState = sourceData.sourceState;
        sourceInfo.interruptState = sourceData.interruptState;

        // This sink might have been created during the gateway registration.Please check locally.
        pElement = CAmSourceFactory::createElement(sourceInfo, mpControlReceive);
        if (NULL != pElement)
        {
            sourceID = pElement->getID();
            result = E_OK;
        }
        else
        {
            LOG_FN_INFO(" Not able to create source element", sourceData.name);
            result = E_NOT_POSSIBLE;
        }
        gc_RegisterElementTrigger_s* registerTrigger = new gc_RegisterElementTrigger_s;
        if (NULL == registerTrigger)
        {
            LOG_FN_ERROR("  bad memory state:");
            return E_NOT_POSSIBLE;
        }
        registerTrigger->elementName = sourceData.name;
        registerTrigger->RegisterationStatus = result;
        mlistTrigger.push_back(std::make_pair(SYSTEM_REGISTER_SOURCE, registerTrigger));
        LOG_FN_INFO("  registered source name:result=", sourceData.name, result);

    }
    LOG_FN_EXIT();
    iterateActions();
    return result;
}

am_Error_e CAmControlSend::hookSystemDeregisterSource(const am_sourceID_t sourceID)
{
    am_Error_e result;
    std::string name;
    CAmElement* pElement;

    LOG_FN_ENTRY(sourceID);
    pElement = CAmSourceFactory::getElement(sourceID);
    if (pElement == NULL)
    {
        LOG_FN_ERROR("Invalid Source Element ");
        return E_NOT_POSSIBLE;
    }
    name = pElement->getName();
    result = CAmSourceFactory::destroyElement(sourceID);
    // remove the sink from DB
    if (E_OK == result)
    {
        gc_UnRegisterElementTrigger_s* unRegisterTrigger = new gc_UnRegisterElementTrigger_s;
        if (NULL == unRegisterTrigger)
        {
            LOG_FN_ERROR("  bad memory state:");
            return E_NOT_POSSIBLE;
        }
        unRegisterTrigger->elementName = name;
        unRegisterTrigger->unRegisterationStatus = E_OK;
        mlistTrigger.push_back(std::make_pair(SYSTEM_DEREGISTER_SOURCE, unRegisterTrigger));
        LOG_FN_INFO("  unregistered source =", sourceID);
    }
    else
    {
        LOG_FN_ERROR(" Error while removing sink from DB. sourceId:Error", sourceID, result);
    }
    LOG_FN_EXIT();
    iterateActions();
    return result;
}

am_Error_e CAmControlSend::hookSystemRegisterGateway(const am_Gateway_s& gatewayData,
                                                     am_gatewayID_t& gatewayID)
{
    am_Error_e result = E_NOT_POSSIBLE;
    CAmElement* pElement;
    gc_Gateway_s gatewayInfo;
    std::vector < gc_Gateway_s > listGateways;
    std::vector < std::string > listNames;
    LOG_FN_ENTRY(" gateway=", gatewayData.name);
    listNames.push_back(gatewayData.name);
    if (E_OK == mpPolicySend->getListElements(listNames, listGateways))
    {
        gatewayInfo = listGateways.front();
        gatewayInfo.gatewayID = gatewayData.gatewayID;
        gatewayInfo.name = gatewayData.name;
        gatewayInfo.sinkID = gatewayData.sinkID;
        gatewayInfo.sourceID = gatewayData.sourceID;
        gatewayInfo.domainSinkID = gatewayData.domainSinkID;
        gatewayInfo.domainSourceID = gatewayData.domainSourceID;
        gatewayInfo.controlDomainID = gatewayData.controlDomainID;
        gatewayInfo.listSourceFormats = gatewayData.listSourceFormats;
        gatewayInfo.listSinkFormats = gatewayData.listSinkFormats;
        gatewayInfo.convertionMatrix = gatewayData.convertionMatrix;
        // This sink might have been created during the gateway registration.Please check locally.
        pElement = CAmGatewayFactory::createElement(gatewayInfo, mpControlReceive);
        if (NULL != pElement)
        {
            gatewayID = pElement->getID();
            result = E_OK;
        }
        else
        {
            LOG_FN_INFO(" Not able to create source element", gatewayData.name);
            result = E_NOT_POSSIBLE;
        }
        gc_RegisterElementTrigger_s* registerTrigger = new gc_RegisterElementTrigger_s;
        if (NULL == registerTrigger)
        {
            LOG_FN_ERROR("  bad memory state:");
            return E_NOT_POSSIBLE;
        }
        registerTrigger->elementName = gatewayData.name;
        registerTrigger->RegisterationStatus = result;
        mlistTrigger.push_back(std::make_pair(SYSTEM_REGISTER_GATEWAY, registerTrigger));
        LOG_FN_INFO("  registered gateway name:result=", gatewayData.name, result);
    }
    LOG_FN_EXIT();
    iterateActions();
    return result;
}

am_Error_e CAmControlSend::hookSystemDeregisterGateway(const am_gatewayID_t gatewayID)
{
    am_Error_e result;
    std::string name;
    CAmElement* pElement;

    LOG_FN_ENTRY(gatewayID);
    pElement = CAmGatewayFactory::getElement(gatewayID);
    if (pElement == NULL)
    {
        LOG_FN_ERROR("Invalid Gateway Element ");
        return E_NOT_POSSIBLE;
    }
    name = pElement->getName();
    result = CAmGatewayFactory::destroyElement(gatewayID);
    if (E_OK == result)
    {
        gc_UnRegisterElementTrigger_s* unRegisterTrigger = new gc_UnRegisterElementTrigger_s;
        if (NULL == unRegisterTrigger)
        {
            LOG_FN_ERROR("  bad memory state:");
            return E_NOT_POSSIBLE;
        }
        unRegisterTrigger->elementName = name;
        unRegisterTrigger->unRegisterationStatus = E_OK;
        mlistTrigger.push_back(std::make_pair(SYSTEM_DEREGISTER_GATEWAY, unRegisterTrigger));
        LOG_FN_INFO("  unregistered gatewayID =", gatewayID);
    }
    else
    {
        LOG_FN_ERROR(" Error while removing sink from DB. gatewayID Error", gatewayID, result);
    }
    LOG_FN_EXIT();
    iterateActions();
    return result;
}

am_Error_e CAmControlSend::hookSystemRegisterCrossfader(const am_Crossfader_s& crossFaderData,
                                                        am_crossfaderID_t& crossFaderID)
{
    LOG_FN_INFO();
    am_Error_e err = mpControlReceive->enterCrossfaderDB(crossFaderData, crossFaderID);
    iterateActions();
    return err;
}

am_Error_e CAmControlSend::hookSystemDeregisterCrossfader(const am_crossfaderID_t crossFaderID)
{
    am_Error_e err = mpControlReceive->removeCrossfaderDB(crossFaderID);
    iterateActions();
    return err;
}

void CAmControlSend::hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID,
                                              const am_volume_t volume)
{
    (void)handle;
    (void)sinkID;
    (void)volume;
    iterateActions();
}

void CAmControlSend::hookSystemSourceVolumeTick(const am_Handle_s handle,
                                                const am_sourceID_t sourceID,
                                                const am_volume_t volume)
{
    (void)handle;
    (void)sourceID;
    (void)volume;
    iterateActions();
}

void CAmControlSend::hookSystemInterruptStateChange(const am_sourceID_t sourceID,
                                                    const am_InterruptState_e interruptState)
{
    LOG_FN_ENTRY(sourceID);
    // get the source element by its ID
    CAmSourceElement* pElement = CAmSourceFactory::getElement(sourceID);
    if (NULL == pElement)
    {
        LOG_FN_ERROR("  invalid sourceID=", sourceID);
        return;
    }
    pElement->setInterruptState(interruptState);
    // Store the trigger in a Queue
    gc_SourceInterruptChangeTrigger_s* triggerData = new gc_SourceInterruptChangeTrigger_s;
    if (NULL == triggerData)
    {
        LOG_FN_ERROR("  bad memory state:");
        return;
    }
    triggerData->sourceName = pElement->getName();
    triggerData->interrptstate = interruptState;
    mlistTrigger.push_back(std::make_pair(SYSTEM_INTERRUPT_STATE_CHANGED, triggerData));
    iterateActions();
    LOG_FN_EXIT();
}

void CAmControlSend::hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID,
                                                          const am_Availability_s& availability)
{
    LOG_FN_ENTRY(sinkID);
    mpControlReceive->changeSinkAvailabilityDB(availability, sinkID);
    // check if the sourceID is valid
    CAmElement* pElement = CAmSinkFactory::getElement(sinkID);
    if (pElement == NULL)
    {
        LOG_FN_ERROR("   parameter error");
        return;
    }

    // Store the trigger in a Queue
    gc_AvailabilityChangeTrigger_s* triggerData = new gc_AvailabilityChangeTrigger_s;
    if (NULL == triggerData)
    {
        LOG_FN_ERROR("  bad memory state:");
        return;
    }

    triggerData->elementName = pElement->getName();
    triggerData->availability.availability = availability.availability;
    triggerData->availability.availabilityReason = availability.availabilityReason;
    mlistTrigger.push_back(std::make_pair(SYSTEM_SINK_AVAILABILITY_CHANGED, triggerData));
    iterateActions();
    LOG_FN_EXIT();
}

void CAmControlSend::hookSystemSourceAvailablityStateChange(
                const am_sourceID_t sourceID, const am_Availability_s& availabilityInstance)
{
    CAmElement* pElement;

    LOG_FN_ENTRY(sourceID);
    mpControlReceive->changeSourceAvailabilityDB(availabilityInstance, sourceID);
    // check if the sourceID is valid
    pElement = CAmSourceFactory::getElement(sourceID);
    if (pElement == NULL)
    {
        LOG_FN_ERROR("   parameter error");
        return;
    }

    // Store the trigger in a Queue
    gc_AvailabilityChangeTrigger_s* triggerData = new gc_AvailabilityChangeTrigger_s;
    if (NULL == triggerData)
    {
        LOG_FN_ERROR("  bad memory state:");
        return;
    }
    triggerData->elementName = pElement->getName();
    triggerData->availability.availability = availabilityInstance.availability;
    triggerData->availability.availabilityReason = availabilityInstance.availabilityReason;
    mlistTrigger.push_back(std::make_pair(SYSTEM_SOURCE_AVAILABILITY_CHANGED, triggerData));
    iterateActions();
    LOG_FN_EXIT();
}

void CAmControlSend::hookSystemDomainStateChange(const am_domainID_t domainID,
                                                 const am_DomainState_e state)
{
    assert(NULL != mpControlReceive);
    LOG_FN_INFO();
    //update the domain state in DB
    mpControlReceive->changDomainStateDB(state, domainID);
}

void CAmControlSend::hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s >& listData)
{
    (void)listData;
    iterateActions();
}

void CAmControlSend::hookSystemSpeedChange(const am_speed_t speed)
{
    (void)speed;
    iterateActions();
}

void CAmControlSend::hookSystemTimingInformationChanged(
                const am_mainConnectionID_t mainConnectionID, const am_timeSync_t syncTime)
{
    (void)mainConnectionID;
    (void)syncTime;
    iterateActions();
}

void CAmControlSend::cbAckConnect(const am_Handle_s handle, const am_Error_e errorID)
{
    LOG_FN_INFO("  IN handle.Type=", (int)handle.handleType, " handle.handle=", (int)handle.handle,
                " errorID=", errorID);
    assert(NULL != mpControlReceive);

    mpControlReceive->cbAckConnect(handle, errorID);
    iterateActions();
}

void CAmControlSend::cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID)
{
    LOG_FN_INFO("  IN  handle.Type=", (int)handle.handleType, " handle.handle=", (int)handle.handle,
                " errorID=", errorID);
    assert(NULL != mpControlReceive);

    mpControlReceive->cbAckDisconnect(handle, errorID);
    iterateActions();
}

void CAmControlSend::cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hotSink,
                                    const am_Error_e errorID)
{
    LOG_FN_INFO(" IN  handle.Type/handle=", (int)handle.handleType, (int)handle.handle,
                " hostsink=", hotSink, " errorID=", errorID);
    assert(NULL != mpControlReceive);

    mpControlReceive->cbAckCrossFade(handle, hotSink, errorID);
    iterateActions();
}

void CAmControlSend::cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume,
                                              const am_Error_e errorID)
{
    LOG_FN_INFO(" IN  handle.Type/handle=", (int)handle.handleType, (int)handle.handle, " volume=",
                volume, " errorID", errorID);
    assert(NULL != mpControlReceive);

    mpControlReceive->cbAckSetSinkVolumeChange(handle, volume, errorID);
    iterateActions();
}

void CAmControlSend::cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume,
                                                const am_Error_e errorID)
{

    LOG_FN_INFO(" IN  handle.Type/handle=", (int)handle.handleType, (int)handle.handle, " volume=",
                volume, " errorID", errorID);
    assert(NULL != mpControlReceive);

    mpControlReceive->cbAckSetSourceVolumeChange(handle, volume, errorID);
    iterateActions();
}

void CAmControlSend::cbAckSetSourceState(const am_Handle_s handle, const am_Error_e errorID)
{

    LOG_FN_INFO("  IN  handle.Type=", (int)handle.handleType, " handle.Type=", (int)handle.handle,
                " errorID=", errorID);
    assert(NULL != mpControlReceive);

    mpControlReceive->cbAckSetSourceState(handle, errorID);
    iterateActions();
}

void CAmControlSend::cbAckSetSourceSoundProperties(const am_Handle_s handle,
                                                   const am_Error_e errorID)
{

    LOG_FN_INFO("  IN  handle.Type=", (int)handle.handleType, " handle.Type=", (int)handle.handle,
                " errorID=", errorID);
    assert(NULL != mpControlReceive);

    mpControlReceive->cbAckSetSourceSoundProperties(handle, errorID);
    iterateActions();
}

void CAmControlSend::cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e errorID)
{
    LOG_FN_INFO("  IN  handle.Type=", (int)handle.handleType, " handle.Type=", (int)handle.handle,
                " errorID=", errorID);
    assert(NULL != mpControlReceive);

    mpControlReceive->cbAckSetSourceSoundProperty(handle, errorID);
    iterateActions();
}

void CAmControlSend::cbAckSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e errorID)
{

    LOG_FN_INFO("  IN  handle.Type=", (int)handle.handleType, " handle.Type=", (int)handle.handle,
                " errorID=", errorID);
    assert(NULL != mpControlReceive);

    mpControlReceive->cbAckSetSinkSoundProperties(handle, errorID);
    iterateActions();
}

void CAmControlSend::cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e errorID)
{

    LOG_FN_INFO("  IN  handle.Type=", (int)handle.handleType, " handle.Type=", (int)handle.handle,
                " errorID=", errorID);
    assert(NULL != mpControlReceive);

    mpControlReceive->cbAckSetSinkSoundProperty(handle, errorID);
    iterateActions();
}

am_Error_e CAmControlSend::getConnectionFormatChoice(
                const am_sourceID_t sourceID, const am_sinkID_t sinkID,
                const am_Route_s routeInstance,
                const std::vector<am_CustomConnectionFormat_t > listPossibleConnectionFormats,
                std::vector<am_CustomConnectionFormat_t >& listPrioConnectionFormats)
{

    (void)sourceID;
    (void)sinkID;
    (void)routeInstance;
    (void)listPossibleConnectionFormats;
    (void)listPrioConnectionFormats;
    listPrioConnectionFormats = listPossibleConnectionFormats;
    return E_OK;
}

void CAmControlSend::confirmCommandReady(const am_Error_e error)
{
    (void)error;
    // have to be implemented if requirements come
}

void CAmControlSend::confirmRoutingReady(const am_Error_e error)
{
    (void)error;
    // have to be implemented if requirements come
}

void CAmControlSend::confirmCommandRundown(const am_Error_e error)
{
    (void)error;
    // have to be implemented if requirements come
}

void CAmControlSend::confirmRoutingRundown(const am_Error_e error)
{
    (void)error;
    // have to be implemented if requirements come
}

am_Error_e CAmControlSend::hookSystemUpdateSink(
                const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID,
                const std::vector<am_SoundProperty_s >& listSoundProperties,
                const std::vector<am_CustomConnectionFormat_t >& listConnectionFormats,
                const std::vector<am_MainSoundProperty_s >& listMainSoundProperties)
{
    am_Error_e result(E_NOT_POSSIBLE);
    assert(NULL != mpControlReceive);
    CAmSinkElement* pSink = CAmSinkFactory::getElement(sinkID);
    if (pSink != NULL)
    {
        result = pSink->upadateDB(sinkClassID, listSoundProperties, listConnectionFormats,
                                  listMainSoundProperties);
    }
    return result;
}

am_Error_e CAmControlSend::hookSystemUpdateSource(
                const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID,
                const std::vector<am_SoundProperty_s >& listSoundProperties,
                const std::vector<am_CustomConnectionFormat_t >& listConnectionFormats,
                const std::vector<am_MainSoundProperty_s >& listMainSoundProperties)
{
    am_Error_e result(E_NOT_POSSIBLE);
    assert(NULL != mpControlReceive);
    CAmSourceElement* pSource = CAmSourceFactory::getElement(sourceID);
    if (pSource != NULL)
    {
        result = pSource->upadateDB(sourceClassID, listSoundProperties, listConnectionFormats,
                                    listMainSoundProperties);
    }
    return result;
}

am_Error_e CAmControlSend::hookSystemUpdateGateway(
                const am_gatewayID_t gatewayID,
                const std::vector<am_CustomConnectionFormat_t >& listSourceConnectionFormats,
                const std::vector<am_CustomConnectionFormat_t >& listSinkConnectionFormats,
                const std::vector<bool >& listConvertionMatrix)
{
    assert(NULL != mpControlReceive);
    //TODO Gateway element should implement updateDB() as well as source, sink
    return mpControlReceive->changeGatewayDB(gatewayID, listSourceConnectionFormats,
                                             listSinkConnectionFormats, listConvertionMatrix);
}

void CAmControlSend::cbAckSetVolumes(const am_Handle_s handle,
                                     const std::vector<am_Volumes_s >& listVolumes,
                                     const am_Error_e error)
{
    (void)handle;
    (void)listVolumes;
    (void)error;
}

void CAmControlSend::cbAckSetSinkNotificationConfiguration(const am_Handle_s handle,
                                                           const am_Error_e error)
{
    (void)handle;
    (void)error;
}

void CAmControlSend::cbAckSetSourceNotificationConfiguration(const am_Handle_s handle,
                                                             const am_Error_e error)
{
    (void)handle;
    (void)error;
}

void CAmControlSend::hookSinkNotificationDataChanged(const am_sinkID_t sinkID,
                                                     const am_NotificationPayload_s& payload)
{
    (void)sinkID;
    (void)payload;
}

void CAmControlSend::hookSourceNotificationDataChanged(const am_sourceID_t sourceID,
                                                       const am_NotificationPayload_s& payload)
{
    (void)sourceID;
    (void)payload;
}

am_Error_e CAmControlSend::hookUserSetMainSinkNotificationConfiguration(
                const am_sinkID_t sinkID,
                const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void)sinkID;
    (void)notificationConfiguration;
    return (E_NOT_USED);
}

am_Error_e CAmControlSend::hookUserSetMainSourceNotificationConfiguration(
                const am_sourceID_t sourceID,
                const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void)sourceID;
    (void)notificationConfiguration;
    return (E_NOT_USED);
}
#ifdef NSM_IFACE_PRESENT
void CAmControlSend::hookSystemNodeStateChanged(const NsmNodeState_e nodeStateId)
{
    (void)nodeStateId;
    //here you can process informations about the nodestate
}

void CAmControlSend::hookSystemNodeApplicationModeChanged(
                const NsmApplicationMode_e applicationModeId)
{
    (void)applicationModeId;
}

void CAmControlSend::hookSystemSessionStateChanged(const std::string& sessionName,
                                                   const NsmSeat_e seatId,
                                                   const NsmSessionState_e sessionStateId)
{
    (void)sessionName;
    (void)seatId;
    (void)sessionStateId;
}

NsmErrorStatus_e CAmControlSend::hookSystemLifecycleRequest(const uint32_t request,
                                                            const uint32_t requestId)
{
    (void)request;
    (void)requestId;
    return (NsmErrorStatus_Error);
}
#endif

void CAmControlSend::hookSystemSingleTimingInformationChanged(const am_connectionID_t connectionID,
                                                              const am_timeSync_t time)
{
    (void)connectionID;
    (void)time;
}

am_Error_e CAmControlSend::hookSystemRegisterConverter(const am_Converter_s& converterData,
                                                       am_converterID_t& converterID)
{
    (void)converterData;
    (void)converterID;
    return E_OK;
}

am_Error_e CAmControlSend::hookSystemDeregisterConverter(const am_converterID_t converterID)
{
    (void)converterID;
    return E_OK;
}

am_Error_e CAmControlSend::hookSystemUpdateConverter(
                const am_converterID_t converterID,
                const std::vector<am_CustomConnectionFormat_t >& listSourceConnectionFormats,
                const std::vector<am_CustomConnectionFormat_t >& listSinkConnectionFormats,
                const std::vector<bool >& listConvertionMatrix)
{
    (void)converterID;
    (void)listConvertionMatrix;
    (void)listSinkConnectionFormats;
    (void)listSourceConnectionFormats;
    return E_OK;
}

//
// private functions
//
void CAmControlSend::_freeMemory(void)
{
    CAmGatewayFactory::destroyElement();
    CAmSourceFactory::destroyElement();
    CAmSinkFactory::destroyElement();
    CAmClassFactory::destroyElement();
    if (NULL != mpPolicySend)
    {
        delete mpPolicySend;
    }
    if (NULL != mpControlReceive)
    {
        delete mpControlReceive;
    }
}

void CAmControlSend::iterateActions(void)
{
    CAmRootAction* pRootAction = CAmRootAction::getInstance();
    gc_Trigger_e triggerType;
    gc_TriggerElement_s* triggerData = NULL;
    if (NULL == pRootAction)
    {
        LOG_FN_ERROR(" Not able to get Root instance");
        return;
    }
    do
    {
        pRootAction->execute();
        if ((pRootAction->getStatus() == AS_ERROR_STOPPED) || (pRootAction->getStatus()
                        == AS_UNDOING))
        {
            pRootAction->undo();
        }
        pRootAction->cleanup();
        if (pRootAction->getStatus() == AS_NOT_STARTED)
        {
            if ((pRootAction->isEmpty()) && (!mlistTrigger.empty()))
            {
                std::pair<gc_Trigger_e, gc_TriggerElement_s* > triggerPair;
                triggerPair = mlistTrigger.front();
                triggerType = triggerPair.first;
                triggerData = triggerPair.second;
                mlistTrigger.erase(mlistTrigger.begin());
                if (triggerData != NULL)
                {
                    _forwardTriggertoPolicyEngine(triggerType, triggerData);
                    delete triggerData;
                }
            }
            else
            {
                // No more actions present in the loop
                LOG_FN_DEBUG("trigger data is null");
                break;
            }
        }
    } while (pRootAction->getStatus() == AS_NOT_STARTED);
}

am_Error_e CAmControlSend::_forwardTriggertoPolicyEngine(gc_Trigger_e triggerType,
                                                         gc_TriggerElement_s* triggerData)
{
    am_Error_e result = E_OK;
    if (NULL == mpPolicySend)
    {
        return E_UNKNOWN;
    }
    switch (triggerType)
    {
    case SYSTEM_REGISTER_SINK:
    {
        gc_RegisterElementTrigger_s* pRegisterElementTrigger_t = (gc_RegisterElementTrigger_s*)triggerData;
        result = mpPolicySend->hookRegisterSink(pRegisterElementTrigger_t->elementName,
                                                pRegisterElementTrigger_t->RegisterationStatus);
    }
        break;
    case SYSTEM_REGISTER_SOURCE:
    {
        gc_RegisterElementTrigger_s* pRegisterElementTrigger_t = (gc_RegisterElementTrigger_s*)triggerData;
        result = mpPolicySend->hookRegisterSource(pRegisterElementTrigger_t->elementName,
                                                  pRegisterElementTrigger_t->RegisterationStatus);
    }
        break;
    case SYSTEM_SINK_AVAILABILITY_CHANGED:
    {
        gc_AvailabilityChangeTrigger_s* pAvailabilityChangeTrigger_t = (gc_AvailabilityChangeTrigger_s*)triggerData;
        result = mpPolicySend->hookSinkAvailabilityChange(
                        pAvailabilityChangeTrigger_t->elementName,
                        pAvailabilityChangeTrigger_t->availability);
    }
        break;
    case SYSTEM_SOURCE_AVAILABILITY_CHANGED:
    {
        gc_AvailabilityChangeTrigger_s* pSourceAvailabilityChangeTrigger_t = (gc_AvailabilityChangeTrigger_s*)triggerData;
        result = mpPolicySend->hookSourceAvailabilityChange(
                        pSourceAvailabilityChangeTrigger_t->elementName,
                        pSourceAvailabilityChangeTrigger_t->availability);
    }
        break;
    case SYSTEM_DEREGISTER_SINK:
    {
        gc_UnRegisterElementTrigger_s* pUnRegisterElementTrigger_t = (gc_UnRegisterElementTrigger_s*)triggerData;
        result = mpPolicySend->hookDeregisterSink(
                        pUnRegisterElementTrigger_t->elementName,
                        pUnRegisterElementTrigger_t->unRegisterationStatus);
    }
        break;
    case SYSTEM_DEREGISTER_SOURCE:
    {
        gc_UnRegisterElementTrigger_s* pUnRegisterElementTrigger_t = (gc_UnRegisterElementTrigger_s*)triggerData;
        result = mpPolicySend->hookDeregisterSource(
                        pUnRegisterElementTrigger_t->elementName,
                        pUnRegisterElementTrigger_t->unRegisterationStatus);
    }
        break;
    case USER_CONNECTION_REQUEST:
    {
        gc_ConnectTrigger_s *pConnectTrigger_t = (gc_ConnectTrigger_s*)triggerData;
        result = mpPolicySend->hookConnectionRequest(pConnectTrigger_t->className,
                                                     pConnectTrigger_t->sourceName,
                                                     pConnectTrigger_t->sinkName);
    }
        break;
    case USER_DISCONNECTION_REQUEST:
    {
        gc_DisconnectTrigger_s* pDisconnectTrigger_t = (gc_DisconnectTrigger_s*)triggerData;
        result = mpPolicySend->hookDisconnectionRequest(pDisconnectTrigger_t->className,
                                                        pDisconnectTrigger_t->sourceName,
                                                        pDisconnectTrigger_t->sinkName);
    }
        break;
    case USER_SET_SINK_MUTE_STATE:
    {
        gc_SinkMuteTrigger_s* pSinkMuteTrigger_t = (gc_SinkMuteTrigger_s*)triggerData;
        result = mpPolicySend->hookSetSinkMuteState(pSinkMuteTrigger_t->sinkName,
                                                    pSinkMuteTrigger_t->muteState);
    }
        break;
    case USER_SET_VOLUME:
    {
        gc_SinkVolumeChangeTrigger_s* pSinkVolumeChangeTrigger_t = (gc_SinkVolumeChangeTrigger_s*)triggerData;
        result = mpPolicySend->hookVolumeChange(pSinkVolumeChangeTrigger_t->sinkName,
                                                pSinkVolumeChangeTrigger_t->volume);
    }
        break;
    case USER_SET_SINK_MAIN_SOUND_PROPERTY:
    {
        gc_SinkSoundPropertyTrigger_s* pSinkSoundPropertyTrigger_t = (gc_SinkSoundPropertyTrigger_s*)triggerData;
        result = mpPolicySend->hookSetMainSinkSoundProperty(
                        pSinkSoundPropertyTrigger_t->sinkName,
                        pSinkSoundPropertyTrigger_t->mainSoundProperty);
    }
        break;
    case USER_SET_SOURCE_MAIN_SOUND_PROPERTY:
    {
        gc_SourceSoundPropertyTrigger_s* pSourceSoundPropertyTrigger_t = (gc_SourceSoundPropertyTrigger_s*)triggerData;
        result = mpPolicySend->hookSetMainSourceSoundProperty(
                        pSourceSoundPropertyTrigger_t->sourceName,
                        pSourceSoundPropertyTrigger_t->mainSoundProperty);
    }
        break;
    case SYSTEM_INTERRUPT_STATE_CHANGED:
    {
        gc_SourceInterruptChangeTrigger_s* pSourceInterruptChangeTrigger_t = (gc_SourceInterruptChangeTrigger_s*)triggerData;
        result = mpPolicySend->hookSourceInterruptStateChange(
                        pSourceInterruptChangeTrigger_t->sourceName,
                        pSourceInterruptChangeTrigger_t->interrptstate);
    }
        break;
    case SYSTEM_REGISTER_DOMAIN:
    {
        gc_RegisterElementTrigger_s* pRegisterElementTrigger_t = (gc_RegisterElementTrigger_s*)triggerData;
        result = mpPolicySend->hookRegisterDomain(pRegisterElementTrigger_t->elementName,
                                                  pRegisterElementTrigger_t->RegisterationStatus);
    }
        break;
    case SYSTEM_DEREGISTER_DOMAIN:
    {
        gc_UnRegisterElementTrigger_s* pUnRegisterElementTrigger_t = (gc_UnRegisterElementTrigger_s*)triggerData;
        result = mpPolicySend->hookDeregisterDomain(
                        pUnRegisterElementTrigger_t->elementName,
                        pUnRegisterElementTrigger_t->unRegisterationStatus);
    }
        break;
    case SYSTEM_REGISTER_GATEWAY:
    {
        gc_RegisterElementTrigger_s* pRegisterElementTrigger_t = (gc_RegisterElementTrigger_s*)triggerData;
        result = mpPolicySend->hookRegisterGateway(pRegisterElementTrigger_t->elementName,
                                                   pRegisterElementTrigger_t->RegisterationStatus);
    }
        break;
    case SYSTEM_DEREGISTER_GATEWAY:
    {
        gc_UnRegisterElementTrigger_s* pUnRegisterElementTrigger_t = (gc_UnRegisterElementTrigger_s*)triggerData;
        result = mpPolicySend->hookDeregisterGateway(
                        pUnRegisterElementTrigger_t->elementName,
                        pUnRegisterElementTrigger_t->unRegisterationStatus);
    }
        break;
    case SYSTEM_DOMAIN_REGISTRATION_COMPLETE:
    {
        gc_DomainRegisterationCompleteTrigger_s* pRegisterElementTrigger_t = (gc_DomainRegisterationCompleteTrigger_s*)triggerData;
        result = mpPolicySend->hookDomainRegistrationComplete(
                        pRegisterElementTrigger_t->domainName);
    }
        break;
    case USER_SET_SYSTEM_PROPERTY:
    {
        gc_SystemPropertyTrigger_s* pSystemPropertyTrigger_t = (gc_SystemPropertyTrigger_s*)triggerData;
        result = mpPolicySend->hookSetSystemProperty(pSystemPropertyTrigger_t->systemProperty);
    }
    break;
    case SYSTEM_CONNECTION_STATE_CHANGE:
    {
        gc_ConnectionStateChangeTrigger_s* pConnectionStateTrigger_t = (gc_ConnectionStateChangeTrigger_s*)triggerData;
        result = mpPolicySend->hookConnectionStateChange(pConnectionStateTrigger_t->connectionName,
                                                         pConnectionStateTrigger_t->connectionState,
                                                         pConnectionStateTrigger_t->status);
    }
        break;
    default:
        break;
    }
    return result;
}

} /* namespace gc */
} /* namespace am */
