/******************************************************************************
 * @file: CAmPolicyEngine.cpp
 *
 * This file contains the definition of policy engine class (member functions
 * and data members) used to provide the interface to get the actions related
 * to the trigger/hook from framework side
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

#include "CAmPolicyEngine.h"
#include "IAmPolicyReceive.h"
#include "CAmLogger.h"
#include "CAmPolicyFunctions.h"
#include "CAmMainConnectionElement.h"
#include "CAmClassElement.h"
#include "CAmGatewayElement.h"
#include "CAmDomainElement.h"
#include "CAmTriggerQueue.h"

namespace am {
namespace gc {

#define isParameterSet(A, B) (1 == B.count(A))
#define AMCO_PRINT_ACTIONS   0


CAmPolicyEngine::CAmPolicyEngine()
    : mpPolicyReceive(NULL)
    , mpPolicyFunctions(NULL)
{
}

am_Error_e CAmPolicyEngine::startPolicyEngine(IAmPolicyReceive *pPolicyReceive)
{
    mpPolicyReceive = pPolicyReceive;

    mpPolicyFunctions = new CAmPolicyFunctions(mpPolicyReceive);

    return E_OK;
}

void CAmPolicyEngine::stopPolicyEngine()
{
    delete mpPolicyFunctions;
    mpPolicyFunctions = NULL;
}

am_Error_e CAmPolicyEngine::_updateActionParameters(gc_Action_s &action, const gc_triggerParams_s &triggerParams)
{
    if (    (action.actionType == ACTION_MUTE)
         || (action.actionType == ACTION_UNMUTE))
    {
        if (    (false == isParameterSet(ACTION_PARAM_SINK_NAME, action.mapParameters))
             && (false == isParameterSet(ACTION_PARAM_CLASS_NAME, action.mapParameters)))
        {
            action.mapParameters[ACTION_PARAM_SINK_NAME] = triggerParams.sinkName;
        }
    }
    else if (action.actionType == ACTION_SET_VOLUME)
    {
        bool isTargetVolumeSpecified = (isParameterSet(ACTION_PARAM_VOLUME, action.mapParameters)
                || isParameterSet(ACTION_PARAM_VOLUME_STEP, action.mapParameters)
                || isParameterSet(ACTION_PARAM_MAIN_VOLUME, action.mapParameters)
                || isParameterSet(ACTION_PARAM_MAIN_VOLUME_STEP, action.mapParameters));
        if ( ! isTargetVolumeSpecified)
        {
            const std::string targetParam = triggerParams.isVolumeStep ? ACTION_PARAM_MAIN_VOLUME_STEP : ACTION_PARAM_MAIN_VOLUME;
            action.mapParameters[targetParam] = to_string(triggerParams.mainVolume);
        }
    }
    else if (action.actionType == ACTION_SET_PROPERTY)
    {
        if (false == isParameterSet(ACTION_PARAM_PROPERTY_TYPE, action.mapParameters))
        {
            action.mapParameters[ACTION_PARAM_PROPERTY_TYPE] = to_string(triggerParams.mainSoundProperty.type);
        }

        if (false == isParameterSet(ACTION_PARAM_PROPERTY_VALUE, action.mapParameters))
        {
            action.mapParameters[ACTION_PARAM_PROPERTY_VALUE] = to_string(triggerParams.mainSoundProperty.value);
        }
    }

    if (action.actionType == ACTION_MUTE)
    {
        action.mapParameters[ACTION_PARAM_MUTE_STATE] = to_string(MS_MUTED);
    }
    else if (action.actionType == ACTION_UNMUTE)
    {
        action.mapParameters[ACTION_PARAM_MUTE_STATE] = to_string(MS_UNMUTED);
    }
    else if (action.actionType == ACTION_LIMIT)
    {
        action.mapParameters[ACTION_PARAM_LIMIT_STATE] = to_string(LS_LIMITED);
    }
    else if (action.actionType == ACTION_UNLIMIT)
    {
        action.mapParameters[ACTION_PARAM_LIMIT_STATE] = to_string(LS_UNLIMITED);
    }
    else if (action.actionType == ACTION_DEBUG)
    {
        if (false == isParameterSet(ACTION_PARAM_DEBUG_TYPE, action.mapParameters))
        {
            action.mapParameters[ACTION_PARAM_DEBUG_TYPE] = to_string(triggerParams.systemProperty.type);

        }

        if (false == isParameterSet(ACTION_PARAM_DEBUG_VALUE, action.mapParameters))
        {
            action.mapParameters[ACTION_PARAM_DEBUG_VALUE] = to_string(triggerParams.systemProperty.value);

        }
    }
    else if (action.actionType == ACTION_SET_SYSTEM_PROPERTY)
    {
        if (false == isParameterSet(ACTION_PARAM_PROPERTY_TYPE, action.mapParameters))
        {
            action.mapParameters[ACTION_PARAM_PROPERTY_TYPE] = to_string(triggerParams.systemProperty.type);
        }

        if (false == isParameterSet(ACTION_PARAM_PROPERTY_VALUE, action.mapParameters))
        {
            action.mapParameters[ACTION_PARAM_PROPERTY_VALUE] = to_string(triggerParams.systemProperty.value);
        }
    }
    else if (action.actionType == ACTION_SET_NOTIFICATION_CONFIGURATION)
    {
        if (false == isParameterSet(ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE, action.mapParameters))
        {
            action.mapParameters[ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE] = to_string(triggerParams.notificatonConfiguration.type);
        }

        if (false == isParameterSet(ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, action.mapParameters))
        {
            action.mapParameters[ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM] = to_string(triggerParams.notificatonConfiguration.parameter);
        }

        if (false == isParameterSet(ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS, action.mapParameters))
        {
            action.mapParameters[ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS] = to_string(triggerParams.notificatonConfiguration.status);
        }
    }
    else if (action.actionType == ACTION_SET_PROPERTIES)
    {
        if (isParameterSet(ACTION_PARAM_LIST_PROPERTY, action.mapParameters))
        {
            std::string list = action.mapParameters[ACTION_PARAM_LIST_PROPERTY];
        }
    }
    else if (action.actionType == ACTION_SET_SYSTEM_PROPERTIES)
    {
        if (isParameterSet(ACTION_PARAM_LIST_SYSTEM_PROPERTIES, action.mapParameters))
        {
            std::string list = action.mapParameters[ACTION_PARAM_LIST_SYSTEM_PROPERTIES];
        }
    }

    return E_OK;
}

am_Error_e CAmPolicyEngine::processTrigger(gc_triggerParams_s &triggerParams)
{
    std::vector<gc_Action_s > listActions;

    am_Error_e result;
    if (triggerParams.triggerType == SYSTEM_SOURCE_AVAILABILITY_CHANGED)
    {
        gc_Source_s source;
        if (E_OK == CAmConfigurationReader::instance().getElementByName(triggerParams.sourceName, source))
        {
            triggerParams.className = source.className;
        }
    }
    else if (triggerParams.triggerType == SYSTEM_SINK_AVAILABILITY_CHANGED)
    {
        gc_Sink_s sink;
        if (E_OK == CAmConfigurationReader::instance().getElementByName(triggerParams.sinkName, sink))
        {
            triggerParams.className = sink.className;
        }
    }

    LOG_FN_INFO(__FILENAME__, __func__, triggerParams);

    // get the action based on the trigger
    result = _getActions(listActions, triggerParams);
    if (E_OK != result)
    {
        return result;
    }

    return mpPolicyReceive->setListActions(listActions, AL_NORMAL);
}

bool CAmPolicyEngine::_getActionsfromPolicy(const gc_Process_s &process,
    std::vector<gc_Action_s > &listActions,
    const gc_triggerParams_s &parameters)
{
    bool returnValue = false;
    listActions.clear();
    if (mpPolicyFunctions && (mpPolicyFunctions->evaluateConditionSet(process.listConditions, parameters)))
    {
        LOG_FN_INFO(__FILENAME__, __func__, "Policy applied with", process.listActions.size()
            , "actions:", process.comment);
        for (auto processAction : process.listActions)
        {
            listActions.push_back(processAction);
            gc_Action_s &action = listActions.back();

            // strip-off quotes and resolve macro REQUESTING
            _convertActionParamsToValues(action, parameters);

            // add other, trigger-related parameters
            _updateActionParameters(action, parameters);
        }
        returnValue = process.stopEvaluation;
    }

    return returnValue;
}

void CAmPolicyEngine::_removeDoubleQuotes(std::string &inputString,
    const std::string &replaceString)
{
    std::size_t found = inputString.find(FUNCTION_MACRO_SUPPORTED_REQUESTING);
    while (std::string::npos != found)
    {
        char *temp = (char *)inputString.c_str();
        if (temp[found - 1] != '\"')
        {
            inputString.replace(found, SIZE_OF_REQUESTING_MACRO, replaceString);
        }

        found = inputString.find(FUNCTION_MACRO_SUPPORTED_REQUESTING, found + 1);
    }

    found = inputString.find("\"");
    while (std::string::npos != found)
    {
        inputString.erase(found, 1);
        found = inputString.find("\"");
    }
}

void CAmPolicyEngine::evaluateMacro(const gc_triggerParams_s &parameters, std::string &requested)
{
    if (requested.find("REQ_") != 0)
    {
        return;   // not a macro handled by this method -> do not touch parameter
    }

    std::map < std::string, std::string > mapRequesting =
    {
        { FUNCTION_MACRO_REQ_TRIG_TYPE, to_string(parameters.triggerType)},
        { FUNCTION_MACRO_REQ_SINK_NAME, parameters.sinkName},
        { FUNCTION_MACRO_REQ_SOURCE_NAME, parameters.sourceName},
        { FUNCTION_MACRO_REQ_DOMAIN_NAME, parameters.domainName},
        { FUNCTION_MACRO_REQ_GATEWAY_NAME, parameters.gatewayName},
        { FUNCTION_MACRO_REQ_CLASS_NAME, parameters.className},
        { FUNCTION_MACRO_REQ_CONNECTION_NAME, parameters.connectionName},
        { FUNCTION_MACRO_REQ_CONNECTION_STATE, to_string(parameters.connectionState)},
        { FUNCTION_MACRO_REQ_STATUS,  to_string(parameters.status)},
        { FUNCTION_MACRO_MAIN_VOLUME, to_string(parameters.mainVolume)},
        { FUNCTION_MACRO_MSP_TYPE, to_string(parameters.mainSoundProperty.type)},
        { FUNCTION_MACRO_MSP_VAL, to_string(parameters.mainSoundProperty.value)},
        { FUNCTION_MACRO_SYP_TYPE, to_string(parameters.systemProperty.type)},
        { FUNCTION_MACRO_SYP_VAL, to_string(parameters.systemProperty.value)},
        { FUNCTION_MACRO_AVAIL_STATE, to_string(parameters.availability.availability)},
        { FUNCTION_MACRO_AVAIL_REASON, to_string(parameters.availability.availabilityReason)},
        { FUNCTION_MACRO_MUTE_STATE, to_string(parameters.muteState)},
        { FUNCTION_MACRO_INT_STATE, to_string(parameters.interruptState)},
        { FUNCTION_MACRO_NP_TYPE, to_string(parameters.notificatonPayload.type)},
        { FUNCTION_MACRO_NP_VAL, to_string(parameters.notificatonPayload.value)},
        { FUNCTION_MACRO_NC_TYPE, to_string(parameters.notificatonConfiguration.type)},
        { FUNCTION_MACRO_NC_STATUS, to_string(parameters.notificatonConfiguration.status)},
        { FUNCTION_MACRO_NC_PARAM, to_string(parameters.notificatonConfiguration.parameter)}
    };
    if (mapRequesting.count(requested))
    {
        std::string &converted = mapRequesting.at(requested);
        LOG_FN_DEBUG(__FILENAME__, __func__, requested, "-->", converted, "for trigger", parameters.triggerType);
        requested = converted;
    }
    else
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "undefined macro", requested, "for trigger", parameters.triggerType);
    }
}

void CAmPolicyEngine::_convertActionParamsToValues(gc_Action_s &action, const gc_triggerParams_s &parameters)
{
    for (auto& itMapParameters : action.mapParameters)
    {
        // if parameter is a macro requesting dedicated fields, evaluate it
        evaluateMacro(parameters, itMapParameters.second);

        // if parameter is a function, evaluate it
        _evaluateParameterFunction(parameters, itMapParameters.second);

        if (itMapParameters.first == ACTION_PARAM_EXCEPT_SOURCE_NAME)
        {
            _removeDoubleQuotes(itMapParameters.second, parameters.sourceName);
        }
        else if (itMapParameters.first == ACTION_PARAM_EXCEPT_SINK_NAME)
        {
            _removeDoubleQuotes(itMapParameters.second, parameters.sinkName);
        }
        else if (itMapParameters.first == ACTION_PARAM_EXCEPT_CLASS_NAME)
        {
            _removeDoubleQuotes(itMapParameters.second, parameters.className);
        }
        else if (itMapParameters.second.data()[0] == '"')
        {
            _removeDoubleQuotes(itMapParameters.second, "");
        }
        else
        {
            if (FUNCTION_MACRO_SUPPORTED_REQUESTING == itMapParameters.second)
            {
                if (ACTION_PARAM_SOURCE_NAME == itMapParameters.first)
                {
                    itMapParameters.second = parameters.sourceName;
                }
                else if (ACTION_PARAM_SINK_NAME == itMapParameters.first)
                {
                    itMapParameters.second = parameters.sinkName;
                }
                else if (ACTION_PARAM_CLASS_NAME == itMapParameters.first)
                {
                    itMapParameters.second = parameters.className;
                }
                else if (ACTION_PARAM_LIST_PROPERTY == itMapParameters.first)
                {
                    std::string listMainSoundProperty = "";
                    for (auto &itListMainSoundProperty : parameters.listMainSoundProperty )
                    {
                        listMainSoundProperty += "(" + to_string(itListMainSoundProperty.type) + ":"
                        + to_string(itListMainSoundProperty.value) + ")";
                    }

                    itMapParameters.second = listMainSoundProperty;
                }
                else if (ACTION_PARAM_LIST_SYSTEM_PROPERTIES == itMapParameters.first)
                {
                    std::string listSystemProperty = "";
                    for (auto &itListSystemProperty : parameters.listSystemProperties )
                    {
                        listSystemProperty += "(" + to_string(itListSystemProperty.type) + ":"
                        + to_string(itListSystemProperty.value) + ")";
                    }
                    itMapParameters.second = listSystemProperty;
                }
            }
        }
    }
}

void CAmPolicyEngine::_evaluateParameterFunction(const gc_triggerParams_s &trigger, std::string &paramValue)
{
    // check if parameter contains an opening bracket and thus is a function
    size_t pos = paramValue.find('(');
    if (pos != string::npos)
    {
        // inplace substitution of single quotes by double quotes
        for (pos = paramValue.find('\'', pos); pos != string::npos; pos = paramValue.find('\'', pos))
        {
            paramValue[pos] = '"';
        }

        // delegate to nested function evaluation
        mpPolicyFunctions->evaluateParameter(trigger, paramValue);
    }
}

am_Error_e CAmPolicyEngine::_getActions(std::vector<gc_Action_s > &listActions,
    const gc_triggerParams_s &parameters)
{
    std::vector<gc_Process_s > listProcesses;
    std::vector<gc_Action_s >  listActionSets;
    listActions.clear();

    // evaluate policies
    if (E_OK != CAmConfigurationReader::instance().getListProcess(parameters.triggerType, listProcesses))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "ERROR finding process for", parameters);
        return E_NOT_POSSIBLE;
    }

    for (auto &process : listProcesses)
    {
        bool stopEvaluation = _getActionsfromPolicy(process, listActionSets, parameters);
        listActions.insert(listActions.end(), listActionSets.begin(), listActionSets.end());
        if (true == stopEvaluation)
        {
            break;
        }
    }

    LOG_FN_INFO(__FILENAME__, __func__, " >>> ------------------------------------");

    // report actions directly derived from policies
    for (auto &action : listActions)
    {
        LOG_FN_INFO(__FILENAME__, __func__, "appending", action);
    }

    // add dependent actions (report on their own)
    _getImplicitActions(listActions, parameters);

    LOG_FN_INFO(__FILENAME__, __func__, " ------------------------------------ <<<");
    return E_OK;
}

void CAmPolicyEngine::_getListStaticSinks(const std::string &domainName,
    std::vector<gc_Sink_s > &listStaticSinks)
{
    std::vector<gc_Sink_s >           listSinks;
    std::vector<gc_Sink_s >::iterator itListSinks;
    CAmConfigurationReader::instance().getListSinks(listSinks);
    for (itListSinks = listSinks.begin(); itListSinks != listSinks.end(); itListSinks++)
    {
        if ((itListSinks->registrationType == REG_CONTROLLER) && (itListSinks->domainName
                                                                  == domainName))
        {
            listStaticSinks.push_back(*itListSinks);
        }
    }
}

void CAmPolicyEngine::_getListStaticSources(const std::string &domainName,
    std::vector<gc_Source_s > &listStaticSources)
{
    std::vector<gc_Source_s >           listSources;
    std::vector<gc_Source_s >::iterator itListSources;
    CAmConfigurationReader::instance().getListSources(listSources);
    for (itListSources = listSources.begin(); itListSources != listSources.end(); itListSources++)
    {
        if ((itListSources->registrationType == REG_CONTROLLER) && (itListSources->domainName
                                                                    == domainName))
        {
            listStaticSources.push_back(*itListSources);
        }
    }
}

void CAmPolicyEngine::_getListStaticGateways(std::vector<std::string > &listGateways,
    std::string &listSources,
    std::string &listSinks)
{
    std::vector<gc_Gateway_s >::iterator itListGateways;
    std::vector<gc_Gateway_s >           listConfiguredGateways;
    CAmConfigurationReader::instance().getListGateways(listConfiguredGateways);
    for (itListGateways = listConfiguredGateways.begin();
         itListGateways != listConfiguredGateways.end(); itListGateways++)
    {
        if ((false == mpPolicyReceive->isRegistered(ET_GATEWAY, itListGateways->name)) &&
            (itListGateways->registrationType == REG_CONTROLLER))
        {
            if ((true == mpPolicyReceive->isRegistered(ET_SOURCE, itListGateways->sourceName)) ||
                (std::string::npos != listSources.find(itListGateways->sourceName)))
            {
                if ((true == mpPolicyReceive->isRegistered(ET_SINK, itListGateways->sinkName)) ||
                    (std::string::npos != listSinks.find(itListGateways->sinkName)))
                {
                    listGateways.push_back(itListGateways->name);
                }
            }
        }
    }
}

void CAmPolicyEngine::_createDisconnectActions(std::set<std::string > &listActionSet,
    std::vector<gc_Action_s > &listActions)
{
    gc_Action_s actionRegister;

    for (auto itListActionSet : listActionSet)
    {
        std::shared_ptr<CAmMainConnectionElement > pMainConnection = CAmMainConnectionFactory::getElement(itListActionSet);
        if (nullptr != pMainConnection)
        {
            std::shared_ptr<CAmClassElement > pClassElement = CAmClassFactory::getElementByConnection(pMainConnection->getName());
            if (nullptr == pClassElement)
            {
                LOG_FN_WARN(__FILENAME__, __func__,
                    "  no class element found from main connection:",
                    pMainConnection->getName());
                continue;
            }

            actionRegister.actionType                                  = ACTION_DISCONNECT;
            actionRegister.mapParameters[ACTION_PARAM_CONNECTION_NAME] = itListActionSet;
            actionRegister.mapParameters[ACTION_PARAM_CLASS_NAME]      = pClassElement->getName();
            actionRegister.mapParameters[ACTION_PARAM_SOURCE_NAME]     = pMainConnection->getMainSourceName();
            actionRegister.mapParameters[ACTION_PARAM_SINK_NAME]       = pMainConnection->getMainSinkName();
            listActions.push_back(actionRegister);

            LOG_FN_INFO(__FILENAME__, __func__, "appended", actionRegister);
        }
    }
}

void CAmPolicyEngine::_domainDeregister(std::string inElementName, std::vector<gc_Action_s > &listActions)
{
    am_Error_e                   result;
    am_Domain_s                  DomainInfo;
    std::vector<am_gatewayID_t > listGatewaysIDs;
    std::vector<am_sinkID_t >    listSinkIDs;
    std::vector<am_sourceID_t >  listSourceIDs;
    std::set<std::string >       listActionSet;

    std::shared_ptr<CAmDomainElement > pDomainElement = CAmDomainFactory::getElement(inElementName);
    if (nullptr == pDomainElement)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "Invalid domain element", inElementName);
        return;
    }

    result = mpPolicyReceive->getDomainInfoByID(pDomainElement->getID(), DomainInfo);
    if (E_OK == result)
    {
        // Get the list of gatewayID belongs to specific domain and also unregister them from the AM DB.
        result = mpPolicyReceive->getListGatewaysOfDomain(pDomainElement->getID(), listGatewaysIDs);
        if ((E_OK == result) && (false == listGatewaysIDs.empty()))
        {
            for (auto itListGatewayIDs : listGatewaysIDs)
            {
                std::shared_ptr<CAmGatewayElement > pGatewayElement = CAmGatewayFactory::getElement(itListGatewayIDs);
                if (nullptr != pGatewayElement)
                {
                    std::shared_ptr<CAmSinkElement > pSinkElement = CAmSinkFactory::getElement(pGatewayElement->getSinkID());
                    if (nullptr == pSinkElement)
                    {
                        LOG_FN_ERROR(__FILENAME__, __func__, "Invalid sink element in domain", inElementName);
                        continue;
                    }

                    _getMainConnectionName(ET_SINK, pSinkElement->getName(), listActionSet);
                }
            }
        }
    }

    result = mpPolicyReceive->getListSinksOfDomain(pDomainElement->getID(), listSinkIDs);
    if ((E_OK == result) && (false == listSinkIDs.empty()))
    {
        for (auto itListSinkIDs : listSinkIDs)
        {
            std::shared_ptr<CAmSinkElement > pSinkElement = CAmSinkFactory::getElement(itListSinkIDs);
            if (nullptr == pSinkElement)
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "Invalid sink element in domain", inElementName);
                continue;
            }

            _getMainConnectionName(ET_SINK, pSinkElement->getName(), listActionSet);
        }
    }

    // Get the list of sourceID belongs to specific domain and disconnect the connection if any
    result = mpPolicyReceive->getListSourcesOfDomain(pDomainElement->getID(), listSourceIDs);
    if ((E_OK == result) && (false == listSourceIDs.empty()))
    {
        for (auto itListSourceIDs : listSourceIDs)
        {
            std::shared_ptr<CAmSourceElement > pSourceElement = CAmSourceFactory::getElement(itListSourceIDs);
            if (nullptr == pSourceElement)
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "Invalid source element in domain", inElementName);
                continue;
            }

            _getMainConnectionName(ET_SOURCE, pSourceElement->getName(), listActionSet);
        }
    }

    /* clear domain id for all the route elements which are belongs to the main connection whose
     * domain is un-register this is required during route disconnection. Because asynDisconnect
     * action gets failed for the route whose domain is no longer exist hence RA will not respond
     * */
    for (auto itListActionSet : listActionSet)
    {
        std::shared_ptr<CAmMainConnectionElement > pMainConnection = CAmMainConnectionFactory::getElement(itListActionSet);
        if (nullptr != pMainConnection)
        {
            std::vector<std::shared_ptr<CAmRouteElement > > listRouteElements;
            pMainConnection->getListRouteElements(listRouteElements);
            for (auto itListRouteElements : listRouteElements)
            {
                if (itListRouteElements->getDomainId() == pDomainElement->getID())
                {
                    LOG_FN_DEBUG(__FILENAME__, __func__, "route domain id matched ", itListRouteElements->getDomainId());
                    itListRouteElements->setDomainId(0);
                }
            }
        }
    }

    _createDisconnectActions(listActionSet, listActions);

}

void CAmPolicyEngine::_gatewayDeregister(std::string inElementName, std::vector<gc_Action_s > &listActions)
{
    std::set<std::string > listActionSet;

    std::shared_ptr<CAmGatewayElement > pGatewayElement = CAmGatewayFactory::getElement(inElementName);
    if (nullptr == pGatewayElement)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "Invalid gateway element", inElementName);
        return;
    }

    std::shared_ptr<CAmElement > pSourceElement = CAmSourceFactory::getElement(pGatewayElement->getSourceID());
    if (nullptr == pSourceElement)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "Invalid source element in gateway", inElementName);
        return;
    }

    _getMainConnectionName(ET_SOURCE, pSourceElement->getName(), listActionSet);
    std::shared_ptr<CAmElement > pSinkElement = CAmSinkFactory::getElement(pGatewayElement->getSinkID());
    if (nullptr == pSinkElement)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "Invalid sink element in gateway", inElementName);
        return;
    }

    _getMainConnectionName(ET_SINK, pSinkElement->getName(), listActionSet);
    _createDisconnectActions(listActionSet, listActions);
}

void CAmPolicyEngine::_getMainConnectionName(gc_Element_e elementType, std::string inElementName,
    std::set<std::string > &listMainConnection)
{
    std::string                       elementName;
    std::string                       sinkName;
    std::string                       sourceName;
    std::vector<gc_ConnectionInfo_s > listConnectionInfo;

    LOG_FN_INFO(__FILENAME__, __func__, "is called and Element type is =", elementType);
    if (elementType == ET_MAX)
    {
        return;
    }

    mpPolicyReceive->getListMainConnections((am::gc::gc_Element_e)elementType, inElementName,
        listConnectionInfo);
    for (auto &itlistConnectionInfo : listConnectionInfo)
    {
        sinkName    = itlistConnectionInfo.sinkName;
        sourceName  = itlistConnectionInfo.sourceName;
        elementName = (sourceName + ":" + sinkName);
        std::shared_ptr<CAmMainConnectionElement > pMainConnection = CAmMainConnectionFactory::getElement(elementName);
        if (nullptr != pMainConnection)
        {
            // get route elements from main connection class
            std::vector<std::shared_ptr<CAmRouteElement > >           listRouteElements;
            std::vector<std::shared_ptr<CAmRouteElement > >::iterator itListRouteElements;

            pMainConnection->getListRouteElements(listRouteElements);
            for (itListRouteElements = listRouteElements.begin();
                 itListRouteElements != listRouteElements.end(); ++itListRouteElements)
            {
                if ((((*itListRouteElements)->getSource())->getName() == inElementName) || (((*itListRouteElements)->getSink())->getName()
                                                                                            == inElementName))
                {
                    LOG_FN_INFO(__FILENAME__, __func__, "element name matched:", inElementName);
                    LOG_FN_INFO(__FILENAME__, __func__, "main connection to be removed:",
                        pMainConnection->getName());
                    auto result = listMainConnection.insert(pMainConnection->getName());
                    if (!result.second)
                    {
                        LOG_FN_INFO(__FILENAME__, __func__, "duplicate main connection",
                            pMainConnection->getName());
                    }

                    break;
                }
            }
        }
    }
}

void CAmPolicyEngine::_updateSystemProperty(am_Error_e status, std::vector<gc_Action_s > &listActions)
{
    gc_Action_s actionRegister;
    actionRegister.actionType                              = ACTION_DEBUG;
    actionRegister.mapParameters[ACTION_PARAM_DEBUG_TYPE]  = to_string(SYP_REGSTRATION_SOUND_PROP_RESTORED);
    actionRegister.mapParameters[ACTION_PARAM_DEBUG_VALUE] = to_string(1); // 1 indicates all sound properties are set
    listActions.push_back(actionRegister);
}

void CAmPolicyEngine::_scheduleSoundPropertyTriggers(std::shared_ptr<CAmRoutePointElement > pElement)
{
    if (nullptr == pElement)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "Invalid source or sink element ");
        return;
    }

    const auto pClassElement = CAmClassFactory::getElement(pElement->getElementClassName());
    if (nullptr == pClassElement)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "Invalid class element ");
        return;
    }

    gc_ElementTypeName_s                 elementInfo = {pElement->getType(), pElement->getName()};
    std::vector<am_MainSoundProperty_s > listLastMainSoundProperty;
    pClassElement->getLastSoundProperty(elementInfo, listLastMainSoundProperty);

    // launch triggers for applications working on the single property interface
    for (const auto &lastMainSoundProperty : listLastMainSoundProperty)
    {
        if (!pElement->isMSPPersistenceSupported(lastMainSoundProperty.type))
        {
            continue;
        }

        // Create a trigger for each recovered property and append it to the Queue
        if (pElement->getType()== ET_SINK)
        {
            auto *trigger = new gc_SinkSoundPropertyTrigger_s;
            if (trigger)
            {
                trigger->sinkName                = pElement->getName();
                trigger->mainSoundProperty       = lastMainSoundProperty;
                CAmTriggerQueue::getInstance()->queue(USER_SET_SINK_MAIN_SOUND_PROPERTY, trigger);
            }
        }
        else if (pElement->getType()== ET_SOURCE)
        {
            auto *trigger = new gc_SourceSoundPropertyTrigger_s;
            if (trigger)
            {
                trigger->sourceName              = pElement->getName();
                trigger->mainSoundProperty       = lastMainSoundProperty;
                CAmTriggerQueue::getInstance()->queue(USER_SET_SOURCE_MAIN_SOUND_PROPERTY, trigger);
            }
        }
    }

    // launch also a trigger for applications working on the multiple property interface
    if (listLastMainSoundProperty.empty())
    {
        return;
    }
    else if (pElement->getType()== ET_SINK)
    {
        auto *trigger = new gc_SinkSoundPropertiesTrigger_s;
        if (trigger)
        {
            trigger->sinkName                = pElement->getName();
            trigger->listMainSoundProperty   = listLastMainSoundProperty;
            CAmTriggerQueue::getInstance()->queue(USER_SET_SINK_MAIN_SOUND_PROPERTIES, trigger);
        }
    }
    else if (pElement->getType()== ET_SOURCE)
    {
        auto *trigger = new gc_SourceSoundPropertiesTrigger_s;
        if (trigger)
        {
            trigger->sourceName              = pElement->getName();
            trigger->listMainSoundProperty   = listLastMainSoundProperty;
            CAmTriggerQueue::getInstance()->queue(USER_SET_SOURCE_MAIN_SOUND_PROPERTIES, trigger);
        }
    }
}

void CAmPolicyEngine::_restoreMainSoundProperties(std::string domainName)
{
    std::vector<am_sinkID_t >          listSinkIDs;
    std::vector<am_sourceID_t >        listSourceIDs;
    am_Error_e                         result;
    std::shared_ptr<CAmDomainElement > pDomainElement = CAmDomainFactory::getElement(domainName);

    if (nullptr == pDomainElement)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "Invalid domain element ");
        return;
    }

    result = mpPolicyReceive->getListSinksOfDomain(pDomainElement->getID(), listSinkIDs);
    if ((E_OK == result) && (false == listSinkIDs.empty()))
    {
        for (auto itListSinkIDs : listSinkIDs)
        {
            std::shared_ptr<CAmSinkElement > pSinkElement = CAmSinkFactory::getElement(
                    itListSinkIDs);
            if (nullptr == pSinkElement)
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "Invalid sink element ");
                continue;
            }

            _scheduleSoundPropertyTriggers(pSinkElement);
        }
    }

    result = mpPolicyReceive->getListSourcesOfDomain(pDomainElement->getID(), listSourceIDs);
    if ((E_OK == result) && (false == listSourceIDs.empty()))
    {

        for (auto itListSourceIDs : listSourceIDs)
        {
            std::shared_ptr<CAmSourceElement > pSourceElement = CAmSourceFactory::getElement(
                    itListSourceIDs);
            if (nullptr == pSourceElement)
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "Invalid source element ");
                continue;
            }

            _scheduleSoundPropertyTriggers(pSourceElement);
        }
    }
}

void CAmPolicyEngine::_getImplicitActions(std::vector<gc_Action_s > &listActions,
    const gc_triggerParams_s &parameters)
{
    uint8_t                flags = 0;
    std::set<std::string > listActionSet;
    LOG_FN_DEBUG(__FILENAME__, __func__, "is called and New trigger type is =", parameters.triggerType);
    switch (parameters.triggerType)
    {
    case SYSTEM_DEREGISTER_SOURCE:
        /// This is System Generated Trigger for the De-registration of Source from Routing Side. it create the ACTION_DISCONNECT.
        _getMainConnectionName(ET_SOURCE, parameters.sourceName, listActionSet);
        _createDisconnectActions(listActionSet, listActions);
        break;

    case SYSTEM_DEREGISTER_SINK:
        /// This is System Generated Trigger for the De-registration of Sink from Routing Side. it create the ACTION_DISCONNECT.
        _getMainConnectionName(ET_SINK, parameters.sinkName, listActionSet);
        _createDisconnectActions(listActionSet, listActions);
        break;
    case SYSTEM_DOMAIN_REGISTRATION_COMPLETE:
        _restoreMainSoundProperties(parameters.domainName);
        break;

    case SYSTEM_ALL_DOMAIN_REGISTRATION_COMPLETE:
        _updateSystemProperty(parameters.status, listActions);
        break;
    case SYSTEM_DEREGISTER_GATEWAY:
        _gatewayDeregister(parameters.gatewayName, listActions);
        break;

    case SYSTEM_DEREGISTER_DOMAIN:
        LOG_FN_DEBUG(__FILENAME__, __func__, "unregistered domain name is =", parameters.domainName);
        _domainDeregister(parameters.domainName, listActions);
        break;

    case SYSTEM_REGISTER_DOMAIN:
        flags = (SEARCH_STATIC_SOURCE | SEARCH_STATIC_SINK | SEARCH_STATIC_GATEWAY);
        break;

    case SYSTEM_REGISTER_SINK:
    case SYSTEM_REGISTER_SOURCE:
        flags = SEARCH_STATIC_GATEWAY;
        break;

    default:
        // all others don't have implicit actions
        return;
    }

    // prepare a register action according to above above flags
    gc_Action_s actionRegister;
    actionRegister.actionType = ACTION_REGISTER;
    if (flags & SEARCH_STATIC_SINK)
    {
        std::vector<gc_Sink_s > listStaticSinks;
        _getListStaticSinks(parameters.domainName, listStaticSinks);
        for (auto &staticSink : listStaticSinks)
        {
            actionRegister.mapParameters[ACTION_PARAM_SINK_NAME] += staticSink.name + " ";
        }
    }

    if (flags & SEARCH_STATIC_SOURCE)
    {
        std::vector<gc_Source_s > listStaticSources;
        _getListStaticSources(parameters.domainName, listStaticSources);
        for (auto &staticSource : listStaticSources)
        {
            actionRegister.mapParameters[ACTION_PARAM_SOURCE_NAME] += staticSource.name + " ";
        }
    }

    if (flags & SEARCH_STATIC_GATEWAY)
    {
        std::string sinkList("");
        std::string sourceList("");
        auto        itMapParam = actionRegister.mapParameters.find(ACTION_PARAM_SINK_NAME);
        if (itMapParam != actionRegister.mapParameters.end())
        {
            sinkList = itMapParam->second;
        }

        itMapParam = actionRegister.mapParameters.find(ACTION_PARAM_SOURCE_NAME);
        if (itMapParam != actionRegister.mapParameters.end())
        {
            sourceList = itMapParam->second;
        }

        std::vector<std::string > listStaticGateways;
        _getListStaticGateways(listStaticGateways, sourceList, sinkList);
        for (auto &staticGateway : listStaticGateways)
        {
            actionRegister.mapParameters[ACTION_PARAM_GATEWAY_NAME] += staticGateway + " ";
        }
    }

    // append the action only if some source, sink or gateway is found
    if (actionRegister.mapParameters.size() > 0)
    {
        listActions.push_back(actionRegister);
        _updateActionParameters(listActions.back(), parameters);
        LOG_FN_INFO(__FILENAME__, __func__, "appended", actionRegister);
    }
}

} /* namespace gc */
} /* namespace am */
