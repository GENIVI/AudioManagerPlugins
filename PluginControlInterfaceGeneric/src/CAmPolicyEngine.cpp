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
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#include "CAmPolicyEngine.h"
#include "IAmPolicyReceive.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

#define isParameterSet(A,B) (1 == B.count(A))
#define AMCO_PRINT_ACTIONS 0
CAmPolicyEngine::CAmPolicyEngine() :
                                mpConfigReader(NULL),
                                mpPolicyReceive(NULL)
{

    // function (as supported in conditions of policy ) mapping to returns string or integer
    mMapFunctionReturnValue[FUNCTION_NAME] = true;
    mMapFunctionReturnValue[FUNCTION_PRIORITY] = false;
    mMapFunctionReturnValue[FUNCTION_CONNECTION_STATE] = false;
    mMapFunctionReturnValue[FUNCTION_VOLUME] = false;
    mMapFunctionReturnValue[FUNCTION_MAIN_VOLUME] = false;
    mMapFunctionReturnValue[FUNCTION_SOUND_PROPERTY] = false;
    mMapFunctionReturnValue[FUNCTION_MAIN_SOUND_PROPERTY_TYPE] = false;
    mMapFunctionReturnValue[FUNCTION_SYSTEM_PROPERTY_TYPE] = false;
    mMapFunctionReturnValue[FUNCTION_MAIN_SOUND_PROPERTY_VALUE] = false;
    mMapFunctionReturnValue[FUNCTION_SYSTEM_PROPERTY_VALUE] = false;
    mMapFunctionReturnValue[FUNCTION_MUTE_STATE] = false;
    mMapFunctionReturnValue[FUNCTION_IS_REGISTRATION_COMPLETE] = false;
    mMapFunctionReturnValue[FUNCTION_AVAILABILITY] = false;
    mMapFunctionReturnValue[FUNCTION_AVAILABILITY_REASON] = false;
    mMapFunctionReturnValue[FUNCTION_CONNECTION_FORMAT] = false;
    mMapFunctionReturnValue[FUNCTION_INTERRUPT_STATE] = false;
    mMapFunctionReturnValue[FUNCTION_IS_REGISTERED] = false;
    mMapFunctionReturnValue[FUNCTION_STATE] = false;
    mMapFunctionReturnValue[FUNCTION_CONNECTION_ERROR] = false;

    mMapNameFunctions[CATEGORY_SINK] = &CAmPolicyEngine::_findSinkName;
    mMapNameFunctions[CATEGORY_SOURCE] = &CAmPolicyEngine::_findSourceName;
    mMapNameFunctions[CATEGORY_CLASS] = &CAmPolicyEngine::_findClassName;
    mMapNameFunctions[CATEGORY_CONNECTION] = &CAmPolicyEngine::_findConnectionName;
    mMapNameFunctions[CATEGORY_DOMAIN_OF_SOURCE] = &CAmPolicyEngine::_findDomainOfSourceName;
    mMapNameFunctions[CATEGORY_DOMAIN_OF_SINK] = &CAmPolicyEngine::_findDomainOfSinkName;
    mMapNameFunctions[CATEGORY_CLASS_OF_SOURCE] = &CAmPolicyEngine::_findClassOfSourceName;
    mMapNameFunctions[CATEGORY_CLASS_OF_SINK] = &CAmPolicyEngine::_findClassOfSinkName;
    mMapNameFunctions[CATEGORY_SOURCE_OF_CLASS] = &CAmPolicyEngine::_findSourceOfClassName;
    mMapNameFunctions[CATEGORY_SINK_OF_CLASS] = &CAmPolicyEngine::_findSinkOfClassName;
    mMapNameFunctions[CATEGORY_DOMAIN] = &CAmPolicyEngine::_findDomainName;

    mMapPriorityFunctions[CATEGORY_SINK] = &CAmPolicyEngine::_findSinkPriority;
    mMapPriorityFunctions[CATEGORY_SOURCE] = &CAmPolicyEngine::_findSourcePriority;
    mMapPriorityFunctions[CATEGORY_CLASS] = &CAmPolicyEngine::_findClassPriority;
    mMapPriorityFunctions[CATEGORY_CONNECTION] = &CAmPolicyEngine::_findConnectionPriority;
    mMapPriorityFunctions[CATEGORY_CONNECTION_OF_CLASS] = &CAmPolicyEngine::_findConnectionOfClassPriority;

    mMapConnectionStateFunctions[CATEGORY_CONNECTION_OF_CLASS] = &CAmPolicyEngine::_findConnectionOfClassState;
    mMapConnectionStateFunctions[CATEGORY_CONNECTION_OF_SOURCE] = &CAmPolicyEngine::_findSourceConnectionState;
    mMapConnectionStateFunctions[CATEGORY_CONNECTION_OF_SINK] = &CAmPolicyEngine::_findSinkConnectionState;

    mMapVolumeFunctions[CATEGORY_SINK] = &CAmPolicyEngine::_findSinkDeviceVolume;
    mMapVolumeFunctions[CATEGORY_SOURCE] = &CAmPolicyEngine::_findSourceDeviceVolume;
    mMapVolumeFunctions[CATEGORY_CONNECTION] = &CAmPolicyEngine::_findConnectionDeviceVolume;
    mMapVolumeFunctions[CATEGORY_CLASS] = &CAmPolicyEngine::_findConnectionOfClassDeviceVolume;

    mMapMainVolumeFunctions[CATEGORY_SINK] = &CAmPolicyEngine::_findSinkMainVolume;
    mMapMainVolumeFunctions[CATEGORY_SOURCE] = &CAmPolicyEngine::_findSourceMainVolume;

    mMapErrorFunctions[CATEGORY_USER] = &CAmPolicyEngine::_findUserErrorValue;

    mMapMainVolumeFunctions[CATEGORY_USER] = &CAmPolicyEngine::_findUserMainVolume;

    mMapSoundPropertyValueFunctions[CATEGORY_SINK] = &CAmPolicyEngine::_findSinkDevicePropertyValue;
    mMapSoundPropertyValueFunctions[CATEGORY_SOURCE] = &CAmPolicyEngine::_findSourceDevicePropertyValue;

    mMapMainSoundPropertyValueFunctions[CATEGORY_SINK] = &CAmPolicyEngine::_findSinkUserPropertyValue;
    mMapMainSoundPropertyValueFunctions[CATEGORY_SOURCE] = &CAmPolicyEngine::_findSourceUserPropertyValue;
    mMapMainSoundPropertyValueFunctions[CATEGORY_USER] = &CAmPolicyEngine::_findUserMainSoundPropertyValue;

    mMapMainSoundPropertyTypeFunctions[CATEGORY_USER] = &CAmPolicyEngine::_findUserMainSoundPropertyType;

    mMapSystemPropertyValueFunctions[CATEGORY_SYSTEM] = &CAmPolicyEngine::_findSystemPropertyValue;
    mMapSystemPropertyValueFunctions[CATEGORY_USER] = &CAmPolicyEngine::_findUserSystemPropertyValue;

    mMapSystemPropertyTypeFunctions[CATEGORY_USER] = &CAmPolicyEngine::_findUserSystemPropertyType;

    mMapMuteStateFunctions[CATEGORY_SINK] = &CAmPolicyEngine::_findSinkMuteState;
    mMapMuteStateFunctions[CATEGORY_CLASS] = &CAmPolicyEngine::_findClassMuteState;
    mMapMuteStateFunctions[CATEGORY_CONNECTION] = &CAmPolicyEngine::_findConnectionMuteState;
    mMapMuteStateFunctions[CATEGORY_USER] = &CAmPolicyEngine::_findUserMuteState;

    mMapIsRegistrationCompleteFunctions[CATEGORY_DOMAIN] = &CAmPolicyEngine::_findIsDomainRegistrationComplete;

    mMapAvailabilityFunctions[CATEGORY_SINK] = &CAmPolicyEngine::_findSinkAvailability;
    mMapAvailabilityFunctions[CATEGORY_SOURCE] = &CAmPolicyEngine::_findSourceAvailability;

    mMapAvailabilityReasonFunctions[CATEGORY_SINK] = &CAmPolicyEngine::_findSinkAvailabilityReason;
    mMapAvailabilityReasonFunctions[CATEGORY_SOURCE] = &CAmPolicyEngine::_findSourceAvailabilityReason;

    mMapInterruptStateFunctions[CATEGORY_SOURCE] = &CAmPolicyEngine::_findSourceInterruptState;
    mMapInterruptStateFunctions[CATEGORY_CONNECTION] = &CAmPolicyEngine::_findConnectionInterruptState;

    mMapIsRegisteredFunctions[CATEGORY_SINK] = &CAmPolicyEngine::_findSinkIsRegistered;
    mMapIsRegisteredFunctions[CATEGORY_SOURCE] = &CAmPolicyEngine::_findSourceIsRegistered;
    mMapIsRegisteredFunctions[CATEGORY_DOMAIN] = &CAmPolicyEngine::_findDomainIsRegistered;

    mMapStateFunctions[CATEGORY_SOURCE] = &CAmPolicyEngine::_findSourceState;
    mMapStateFunctions[CATEGORY_DOMAIN] = &CAmPolicyEngine::_findDomainState;

    //reserved for future use
    mMapConnectionFormatFunctions[CATEGORY_CONNECTION] = &CAmPolicyEngine::_findConnectionFormat;
    mMapConnectionFormatFunctions[CATEGORY_CLASS] = &CAmPolicyEngine::_findConnectionOfClassFormat;

    //name of the functions as supported in condition tag in configuration
    mMapFunctionNameToFunctionMaps[FUNCTION_NAME] = mMapNameFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_PRIORITY] = mMapPriorityFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_CONNECTION_STATE] = mMapConnectionStateFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_VOLUME] = mMapVolumeFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_MAIN_VOLUME] = mMapMainVolumeFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_SOUND_PROPERTY] = mMapSoundPropertyValueFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_MAIN_SOUND_PROPERTY_VALUE] = mMapMainSoundPropertyValueFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_MAIN_SOUND_PROPERTY_TYPE] = mMapMainSoundPropertyTypeFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_SYSTEM_PROPERTY_TYPE] = mMapSystemPropertyTypeFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_SYSTEM_PROPERTY_VALUE] = mMapSystemPropertyValueFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_MUTE_STATE] = mMapMuteStateFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_IS_REGISTRATION_COMPLETE] = mMapIsRegistrationCompleteFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_AVAILABILITY] = mMapAvailabilityFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_AVAILABILITY_REASON] = mMapAvailabilityReasonFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_INTERRUPT_STATE] = mMapInterruptStateFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_IS_REGISTERED] = mMapIsRegisteredFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_STATE] = mMapStateFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_CONNECTION_FORMAT] = mMapConnectionFormatFunctions;
    mMapFunctionNameToFunctionMaps[FUNCTION_CONNECTION_ERROR] = mMapErrorFunctions;

    mMapActions[CONFIG_ACTION_NAME_CONNECT] = ACTION_NAME_CONNECT;
    mMapActions[CONFIG_ACTION_NAME_POP] = ACTION_NAME_CONNECT;
    mMapActions[CONFIG_ACTION_NAME_RESUME] = ACTION_NAME_CONNECT;
    mMapActions[CONFIG_ACTION_NAME_DISCONNECT] = ACTION_NAME_DISCONNECT;
    mMapActions[CONFIG_ACTION_NAME_SET_VOLUME] = ACTION_NAME_SET_VOLUME;
    mMapActions[CONFIG_ACTION_NAME_LIMIT] = ACTION_NAME_LIMIT;
    mMapActions[CONFIG_ACTION_NAME_UNLIMIT] = ACTION_NAME_LIMIT;
    mMapActions[CONFIG_ACTION_NAME_PUSH] = ACTION_NAME_INTERRUPT;
    mMapActions[CONFIG_ACTION_NAME_SUSPEND] = ACTION_NAME_SUSPEND;
    mMapActions[CONFIG_ACTION_NAME_MUTE] = ACTION_NAME_MUTE;
    mMapActions[CONFIG_ACTION_NAME_UNMUTE] = ACTION_NAME_MUTE;
    mMapActions[CONFIG_ACTION_NAME_SET_PROPERTY] = ACTION_NAME_SET_PROPERTY;
    mMapActions[CONFIG_ACTION_NAME_REGISTER] = ACTION_NAME_REGISTER;
    mMapActions[CONFIG_ACTION_NAME_DEBUG] = ACTION_DEBUG;
}

am_Error_e CAmPolicyEngine::startPolicyEngine(IAmPolicyReceive* pPolicyReceive)
{
    std::vector<gc_Class_s > listClasses;
    gc_Class_s classInfoInstance;
    std::vector<am_SystemProperty_s > listSystemProperties;
    std::vector<am_SystemProperty_s >::iterator itListSystemProperties;
    mpPolicyReceive = pPolicyReceive;
    mpConfigReader = new CAmConfigurationReader();
    if (mpConfigReader == NULL)
    {
        return E_UNKNOWN;
    }
    //get list of system properties
    mpConfigReader->getListSystemProperties(listSystemProperties);
    //register system properties to DB
    gc_triggerParams_s triggerParams;
    triggerParams.systemProperty.type = SYP_DEBUGLEVEL;
    triggerParams.systemProperty.value = LOG_DEBUG_DEFAULT_VALUE;

    if (listSystemProperties.size() > 0)
    {
        for (itListSystemProperties = listSystemProperties.begin();
                        itListSystemProperties != listSystemProperties.end();
                        itListSystemProperties++)
        {
            if (itListSystemProperties->type == SYP_DEBUGLEVEL)
            {
                triggerParams.systemProperty.value = itListSystemProperties->value;
            }
        }
    }

    std::vector<gc_Action_s > listActions;
    gc_Action_s action;
    action.actionName = ACTION_DEBUG;
    listActions.push_back(action);
    _updateActionParameters(listActions, triggerParams);
    mpPolicyReceive->setListActions(listActions);
    return E_OK;
}

am_Error_e CAmPolicyEngine::_updateActionParameters(std::vector<gc_Action_s >& listActions,
                                                    gc_triggerParams_s& triggerParams)
{
    char data[5];
    std::vector<gc_Action_s >::iterator itListActions;
    for (itListActions = listActions.begin(); itListActions != listActions.end(); itListActions++)
    {
        if ((*itListActions).actionName == CONFIG_ACTION_NAME_CONNECT)
        {
            if (false == isParameterSet(ACTION_PARAM_SOURCE_NAME, (*itListActions).mapParameters))
            {
                (*itListActions).mapParameters[ACTION_PARAM_SOURCE_NAME] = triggerParams.sourceName;
            }
            if (false == isParameterSet(ACTION_PARAM_SINK_NAME, (*itListActions).mapParameters))
            {
                (*itListActions).mapParameters[ACTION_PARAM_SINK_NAME] = triggerParams.sinkName;
            }
        }
        else if (((*itListActions).actionName == CONFIG_ACTION_NAME_MUTE) || ((*itListActions).actionName
                        == CONFIG_ACTION_NAME_UNMUTE))
        {
            if ((false == isParameterSet(ACTION_PARAM_SINK_NAME, (*itListActions).mapParameters)) && (false
                            == isParameterSet(ACTION_PARAM_CLASS_NAME,
                                              (*itListActions).mapParameters)))
            {
                (*itListActions).mapParameters[ACTION_PARAM_SINK_NAME] = triggerParams.sinkName;
            }
        }
        else if ((*itListActions).actionName == CONFIG_ACTION_NAME_SET_VOLUME)
        {
            if ((false == isParameterSet(ACTION_PARAM_VOLUME, (*itListActions).mapParameters)) && (false
                            == isParameterSet(ACTION_PARAM_VOLUME_STEP,
                                              (*itListActions).mapParameters))
                && (false == isParameterSet(ACTION_PARAM_MAIN_VOLUME,
                                            (*itListActions).mapParameters))
                && (false == isParameterSet(ACTION_PARAM_MAIN_VOLUME_STEP,
                                            (*itListActions).mapParameters)))
            {
                sprintf(data, "%d", triggerParams.mainVolume);
                (*itListActions).mapParameters[ACTION_PARAM_MAIN_VOLUME] = data;
            }
            if (isParameterSet(ACTION_PARAM_SOURCE_NAME, (*itListActions).mapParameters))
            {
                if(isParameterSet(ACTION_PARAM_VOLUME_STEP,(*itListActions).mapParameters))
                {
                    am_volume_t  stepVolume = atoi((*itListActions).mapParameters[ACTION_PARAM_VOLUME_STEP].data());
                    std::string sourceName = (*itListActions).mapParameters[ACTION_PARAM_SOURCE_NAME];
                    am_volume_t volume;
                    mpPolicyReceive->getVolume(ET_SOURCE,sourceName,volume);
                    sprintf(data, "%d", volume + stepVolume);
                    (*itListActions).mapParameters[ACTION_PARAM_VOLUME] = data;
                }
            }
            if (isParameterSet(ACTION_PARAM_SINK_NAME, (*itListActions).mapParameters))
            {
                std::string sinkName = (*itListActions).mapParameters[ACTION_PARAM_SINK_NAME];
                if(isParameterSet(ACTION_PARAM_VOLUME_STEP,(*itListActions).mapParameters))
                {
                    am_volume_t volume;
                    am_volume_t  stepVolume = atoi((*itListActions).mapParameters[ACTION_PARAM_VOLUME_STEP].data());
                    mpPolicyReceive->getVolume(ET_SINK,sinkName,volume);
                    sprintf(data, "%d", volume + stepVolume);
                    (*itListActions).mapParameters[ACTION_PARAM_VOLUME] = data;
                }
                if(isParameterSet(ACTION_PARAM_MAIN_VOLUME_STEP,(*itListActions).mapParameters))
                {
                    am_mainVolume_t mainVolume;
                    am_mainVolume_t  stepVolume = atoi((*itListActions).mapParameters[ACTION_PARAM_MAIN_VOLUME_STEP].data());
                    mpPolicyReceive->getMainVolume(ET_SINK,sinkName,mainVolume);
                    sprintf(data, "%d", mainVolume + stepVolume);
                    (*itListActions).mapParameters[ACTION_PARAM_MAIN_VOLUME] = data;
                }
            }
        }
        else if ((*itListActions).actionName == CONFIG_ACTION_NAME_SET_PROPERTY)
        {
            sprintf(data, "%d", triggerParams.mainSoundProperty.type);
            if (false == isParameterSet(ACTION_PARAM_PROPERTY_TYPE, (*itListActions).mapParameters))
            {
                (*itListActions).mapParameters[ACTION_PARAM_PROPERTY_TYPE] = data;
            }
            sprintf(data, "%d", triggerParams.mainSoundProperty.value);
            if (false == isParameterSet(ACTION_PARAM_PROPERTY_VALUE, (*itListActions).mapParameters))
            {
                (*itListActions).mapParameters[ACTION_PARAM_PROPERTY_VALUE] = data;
            }
        }
        if ((*itListActions).actionName == CONFIG_ACTION_NAME_MUTE)
        {
            sprintf(data, "%d", MS_MUTED);
            (*itListActions).mapParameters[ACTION_PARAM_MUTE_STATE] = data;
        }
        else if ((*itListActions).actionName == CONFIG_ACTION_NAME_UNMUTE)
        {
            sprintf(data, "%d", MS_UNMUTED);
            (*itListActions).mapParameters[ACTION_PARAM_MUTE_STATE] = data;
        }
        else if ((*itListActions).actionName == CONFIG_ACTION_NAME_LIMIT)
        {
            sprintf(data, "%d", LS_LIMITED);
            (*itListActions).mapParameters[ACTION_PARAM_LIMIT_STATE] = data;
        }
        else if ((*itListActions).actionName == CONFIG_ACTION_NAME_UNLIMIT)
        {
            sprintf(data, "%d", LS_UNLIMITED);
            (*itListActions).mapParameters[ACTION_PARAM_LIMIT_STATE] = data;
        }
        else if ((*itListActions).actionName == CONFIG_ACTION_NAME_DEBUG)
        {
            if(false == isParameterSet(ACTION_PARAM_DEBUG_TYPE,(*itListActions).mapParameters))
            {
                sprintf(data, "%d", triggerParams.systemProperty.type);
                (*itListActions).mapParameters[ACTION_PARAM_DEBUG_TYPE] = data;
            }
            if(false == isParameterSet(ACTION_PARAM_DEBUG_VALUE,(*itListActions).mapParameters))
            {
                sprintf(data, "%d", triggerParams.systemProperty.value);
                (*itListActions).mapParameters[ACTION_PARAM_DEBUG_VALUE] = data;
            }
        }

        //convert config actions to framework actions
        (*itListActions).actionName = mMapActions[(*itListActions).actionName];
    }
    return E_OK;
}

am_Error_e CAmPolicyEngine::processTrigger(gc_triggerParams_s& triggerParams)
{
    std::vector<gc_Action_s > listActions;

    am_Error_e result;
    if (triggerParams.triggerType == SYSTEM_SOURCE_AVAILABILITY_CHANGED)
    {
        gc_Source_s source;
        if (E_OK == mpConfigReader->getElementByName(triggerParams.sourceName, source))
        {
            triggerParams.className = source.className;
        }
    }
    else if (triggerParams.triggerType == SYSTEM_SINK_AVAILABILITY_CHANGED)
    {
        gc_Sink_s sink;
        if (E_OK == mpConfigReader->getElementByName(triggerParams.sinkName, sink))
        {
            triggerParams.className = sink.className;
        }
    }
    //get the action based on the trigger
    result = _getActions(triggerParams.triggerType, listActions, triggerParams);
    if (E_OK != result)
    {
        return result;
    }
    _updateActionParameters(listActions, triggerParams);

    return mpPolicyReceive->setListActions(listActions, AL_NORMAL);
}

template <typename Toperand>
bool CAmPolicyEngine::_conditionResult(Toperand lhsData, gc_Operator_e opType, Toperand rhdData)
{
    bool result;
    LOG_FN_INFO(" LHS =", lhsData, " RHS=", rhdData);
    switch (opType)
    {
    case EQ:
        result = (lhsData == rhdData);
        break;
    case GT:
        result = (lhsData > rhdData);
        break;
    case GEQ:
        result = (lhsData >= rhdData);
        break;
    case LT:
        result = (lhsData < rhdData);
        break;
    case LEQ:
        result = (lhsData <= rhdData);
        break;
    case NE:
        result = (lhsData != rhdData);
        break;
    default:
        result = false;
        break;
    }
    return result;
}

bool CAmPolicyEngine::_isConditionTrue(const gc_ConditionStruct_s& condition,
                                       const gc_triggerParams_s& parameters)
{
    std::string LHSString, RHSString;
    int32_t LHSInt, RHSInt;
    std::vector<std::string > listLHSOutputs;
    std::vector<std::string > listRHSOutputs;
    std::vector<std::string >::iterator itListOutputs;
    bool result;
    gc_Operator_e operation;

    // main evaluation logic comes here!!!!!!!!

    //convert operator from string to enum type
    operation = (gc_Operator_e)atoi(condition.operation.data());
    // invoke the function to get the LHS string from condition
    result = _executeFunction(condition.leftObject.functionName, condition.leftObject.category,
                              condition, listLHSOutputs, parameters, true);
    if (false == result)
    {
        LOG_FN_DEBUG(" LHS function return error. function->category 1",
                     condition.leftObject.functionName, condition.leftObject.category);
        return result;
    }
    if (true == listLHSOutputs.empty())
    {
        LOG_FN_DEBUG(" LHS function does not return any value function->category 2",
                     condition.leftObject.functionName, condition.leftObject.category,
                     condition.leftObject.mandatoryParameter,
                     condition.leftObject.optionalParameter);
        return false;
    }
    //get the RHS string from condition
    if (true == condition.rightObject.isValue) // direct value is provided by user
    {
        RHSString = condition.rightObject.directValue;
    }
    else
    {
        // invoke the function to get the RHS string from condition
        result = _executeFunction(condition.rightObject.functionObject.functionName,
                                  condition.rightObject.functionObject.category, condition,
                                  listRHSOutputs, parameters, false);
        if (false == result)
        {
            LOG_FN_DEBUG(" RHS function return error. function->category 3",
                         condition.rightObject.functionObject.functionName,
                         condition.rightObject.functionObject.category);
            return result;
        }
        if (true == listRHSOutputs.empty())
        {
            LOG_FN_DEBUG(" RHS function does not return any value function->category 4",
                         condition.rightObject.functionObject.functionName,
                         condition.rightObject.functionObject.category,
                         condition.rightObject.functionObject.mandatoryParameter,
                         condition.rightObject.functionObject.optionalParameter);
            return false;
        }
        RHSString = listRHSOutputs[0];
    }
    if (INC == operation) //operator is include operator
    {
        result = false;
        for (itListOutputs = listLHSOutputs.begin(); itListOutputs != listLHSOutputs.end();
                        itListOutputs++)
        {
            if (*itListOutputs == RHSString)
            {
                result = true;
                break;
            }
        }
    }
    else if (EXC == operation) //operator is exclude operator
    {
        result = true;
        for (itListOutputs = listLHSOutputs.begin(); itListOutputs != listLHSOutputs.end();
                        itListOutputs++)
        {
            if (*itListOutputs == RHSString)
            {
                result = false;
                break;
            }
        }
    }
    else // mathematical operator
    {
        if (true == mMapFunctionReturnValue[condition.leftObject.functionName]) // function if of type string
        {
            LHSString = listLHSOutputs[0];
            result = _conditionResult(LHSString, operation, RHSString);
        }
        else //function is of type integer
        {
            RHSInt = atoi(RHSString.data());
            LHSInt = atoi(listLHSOutputs[0].data());
            result = _conditionResult(LHSInt, operation, RHSInt);
        }
    }
    return result;
}

am_Error_e CAmPolicyEngine::_evaluateConditionSet(
                const std::vector<gc_ConditionStruct_s >& listConditionSets,
                const gc_triggerParams_s& parameters)
{
    std::vector<gc_ConditionStruct_s >::const_iterator itListConditions;
    for (itListConditions = listConditionSets.begin(); itListConditions != listConditionSets.end();
                    ++itListConditions)
    {
        if (false == _isConditionTrue(*itListConditions, parameters))
        {
            return E_NOT_POSSIBLE;
        }
    }
    return E_OK;
}

bool CAmPolicyEngine::_getActionsfromPolicy(const gc_Process_s& process,
                                            std::vector<gc_Action_s >& listActions,
                                            const gc_triggerParams_s& parameters)
{
    bool returnValue = false;
    listActions.clear();
    if (E_OK == _evaluateConditionSet(process.listConditions, parameters))
    {
        LOG_FN_DEBUG("Policy evaluated to true:", process.comment);
        listActions.insert(listActions.end(), process.listActions.begin(),
                           process.listActions.end());
        returnValue = process.stopEvaluation;
    }
    return returnValue;
}

void CAmPolicyEngine::_removeDoubleQuotes(std::string& inputString,
                                          const std::string& replaceString)
{
    std::size_t found = inputString.find(FUNCTION_MACRO_SUPPORTED_REQUESTING);
    while (std::string::npos != found)
    {
        char * temp = (char *)inputString.c_str();
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

void CAmPolicyEngine::_convertActionParamToValue(std::vector<gc_Action_s >& listActions,
                                                 const gc_triggerParams_s& parameters)
{
    std::vector<gc_Action_s >::iterator itListActions;
    std::map<std::string, std::string >::iterator itMapParameters;
    for (itListActions = listActions.begin(); itListActions != listActions.end(); itListActions++)
    {
        //for param_map of action parameter
        for (itMapParameters = (*itListActions).mapParameters.begin();
                        itMapParameters != (*itListActions).mapParameters.end(); itMapParameters++)
        {
            if (itMapParameters->first == ACTION_PARAM_EXCEPT_SOURCE_NAME)
            {
                _removeDoubleQuotes(itMapParameters->second, parameters.sourceName);
            }
            else if (itMapParameters->first == ACTION_PARAM_EXCEPT_SINK_NAME)
            {
                _removeDoubleQuotes(itMapParameters->second, parameters.sinkName);
            }
            else if (itMapParameters->first == ACTION_PARAM_EXCEPT_CLASS_NAME)
            {
                _removeDoubleQuotes(itMapParameters->second, parameters.className);
            }
            // removing the double quote from action parameter.
            else if (itMapParameters->second.data()[0] == '"')
            {
                _removeDoubleQuotes(itMapParameters->second, "");
            }
            else
            {
                //converting Macro "REQUESTING" to actual value at run time
                if (FUNCTION_MACRO_SUPPORTED_REQUESTING == itMapParameters->second)
                {
                    if (ACTION_PARAM_SOURCE_NAME == itMapParameters->first) //macro is for source name
                    {
                        itMapParameters->second = parameters.sourceName;
                    }
                    else if (ACTION_PARAM_SINK_NAME == itMapParameters->first) //macro is for sink name
                    {
                        itMapParameters->second = parameters.sinkName;
                    }
                    else if (ACTION_PARAM_CLASS_NAME == itMapParameters->first) // macro is for class name
                    {
                        itMapParameters->second = parameters.className;
                    }
                }
            }
        }
    }
}

am_Error_e CAmPolicyEngine::_getActions(const gc_Trigger_e trigger,
                                        std::vector<gc_Action_s >& listActions,
                                        const gc_triggerParams_s& parameters)
{
    std::vector<gc_Process_s > listProcesses;
    std::vector<gc_Process_s >::iterator itListProcesses;
    std::vector<gc_Action_s > listActionSets;
    std::vector<gc_Action_s >::iterator itListActions;
    listActions.clear();

    if (E_OK != mpConfigReader->getListProcess(trigger, listProcesses))
    {
        return E_NOT_POSSIBLE;
    }
    for (itListProcesses = listProcesses.begin(); itListProcesses != listProcesses.end();
                    ++itListProcesses)
    {
        bool stopEvaluation = _getActionsfromPolicy(*itListProcesses, listActionSets, parameters);
        listActions.insert(listActions.end(), listActionSets.begin(), listActionSets.end());
        if (true == stopEvaluation)
        {
            break;
        }
    }
    _convertActionParamToValue(listActions, parameters);
    _getImplicitActions(trigger, listActions, parameters);
    LOG_FN_DEBUG(" ----------------------------------------");
    for (itListActions = listActions.begin(); itListActions != listActions.end(); itListActions++)
    {
        LOG_FN_DEBUG(itListActions->actionName);
    }
    LOG_FN_DEBUG(" ----------------------------------------");
    return E_OK;
}

//used in functions returning the name for source, sink, class, connection and domain category
bool CAmPolicyEngine::_isMacroOfNameAllowed(const gc_ConditionStruct_s& conditionInstance,
                                            const bool isLHS) const
{
    bool result = true;
    bool isValueMacro;
    std::string mandatoryParameter;
    isValueMacro = (isLHS == true) ? conditionInstance.leftObject.isValueMacro : conditionInstance.rightObject.functionObject.isValueMacro;
    mandatoryParameter =
                    (isLHS == true) ? conditionInstance.leftObject.mandatoryParameter : conditionInstance.rightObject.functionObject.mandatoryParameter;
    if ((FUNCTION_MACRO_SUPPORTED_REQUESTING != mandatoryParameter) || (false == isValueMacro))
    {
        result = false;
    }
    return result;
}

// return the value of mandatory and optional parameter
void CAmPolicyEngine::_getValueOfParameter(const gc_ConditionStruct_s& conditionInstance,
                                           const bool isLHS, std::string& mandatoryParameter,
                                           std::string &optionalParameter) const
{
    bool isValueMacro;
    std::string tempMandatoryParameter;
    std::string tempOptionalParameter;
    isValueMacro = (isLHS == true) ? conditionInstance.leftObject.isValueMacro : conditionInstance.rightObject.functionObject.isValueMacro;
    tempMandatoryParameter =
                    (isLHS == true) ? conditionInstance.leftObject.mandatoryParameter : conditionInstance.rightObject.functionObject.mandatoryParameter;
    tempOptionalParameter =
                    (isLHS == true) ? conditionInstance.leftObject.optionalParameter : conditionInstance.rightObject.functionObject.optionalParameter;
    if ((FUNCTION_MACRO_SUPPORTED_REQUESTING != tempMandatoryParameter) || (false == isValueMacro))
    {
        mandatoryParameter = tempMandatoryParameter;
    }
    optionalParameter = tempOptionalParameter;
}

// function overloaded to support 3 parameter functionality
void CAmPolicyEngine::_getValueOfParameter(const gc_ConditionStruct_s &conditionInstance,
                                           const bool isLHS, std::string& mandatoryParameter) const
{
    std::string optionalParameter;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter, optionalParameter);
}

// return weather all domain are requested or single domain is requested
bool CAmPolicyEngine::_isSingleDomainRequest(const gc_ConditionStruct_s &conditionInstance,
                                             const bool isLHS,
                                             std::string& mandatoryParameter) const
{
    bool result = false;
    bool isValueMacro;
    std::string tempMandatoryParameter;
    isValueMacro = (isLHS == true) ? conditionInstance.leftObject.isValueMacro : conditionInstance.rightObject.functionObject.isValueMacro;
    tempMandatoryParameter =
                    (isLHS == true) ? conditionInstance.leftObject.mandatoryParameter : conditionInstance.rightObject.functionObject.mandatoryParameter;
    if (FUNCTION_MACRO_SUPPORTED_REQUESTING == tempMandatoryParameter)
    {
        result = true;
    }
    else if ((FUNCTION_MACRO_SUPPORTED_ALL != tempMandatoryParameter) || (false == isValueMacro))
    {
        mandatoryParameter = tempMandatoryParameter;
        result = true;
    }
    return result;
}

// get the connection list based on element type,name and optional parameter passed in condition in configuration
am_Error_e CAmPolicyEngine::_getConnectionList(
                const gc_Element_e elementType, const std::string& elementName,
                const std::string& optionalParameter,
                std::vector<gc_ConnectionInfo_s > &listConnectionInfo,
                const gc_triggerParams_s &parameters) const
{
    std::vector<gc_ConnectionInfo_s > listLocalConnectionInfo;
    std::vector<gc_ConnectionInfo_s >::iterator itListConnectionInfo;
    am_Error_e result = E_UNKNOWN;
    if (E_OK == mpPolicyReceive->getListMainConnections(elementType, elementName,
                                                        listLocalConnectionInfo))
    {
        // if parameter is ALL or empty means all the connection of this class is needed
        if ((FUNCTION_MACRO_SUPPORTED_ALL == optionalParameter) || (true
                        == optionalParameter.empty()))
        {
            listConnectionInfo = listLocalConnectionInfo;
        }
        else if (FUNCTION_MACRO_SUPPORTED_OTHERS == optionalParameter) // except requesting all connection needed
        {
            for (itListConnectionInfo = listLocalConnectionInfo.begin();
                            itListConnectionInfo != listLocalConnectionInfo.end();
                            itListConnectionInfo++)
            {
                if (((*itListConnectionInfo).sinkName == parameters.sinkName) && ((*itListConnectionInfo).sourceName
                                == parameters.sourceName))
                {
                    continue;
                }
                listConnectionInfo.push_back((*itListConnectionInfo));
            }
        }
        else // for macro like CS_CONNECTED/CS_DISCONNECTED etc as specified in configuration
        {
            char outputData[5];
            for (itListConnectionInfo = listLocalConnectionInfo.begin();
                            itListConnectionInfo != listLocalConnectionInfo.end();
                            itListConnectionInfo++)
            {
                // convert the macro in value based on schema and store in std::string format
                sprintf(outputData, "%d", (*itListConnectionInfo).connectionState);
                if (outputData == optionalParameter)
                {
                    listConnectionInfo.push_back((*itListConnectionInfo));
                }
            }
        }
        result = E_OK;
    }
    return result;
}

//find the device i.e. router side volume based on element type and name
am_Error_e CAmPolicyEngine::_findDeviceVolume(const gc_Element_e elementType,
                                              const gc_ConditionStruct_s &conditionInstance,
                                              std::string& elementName,
                                              std::vector<std::string > &listOutputs,
                                              const bool isLHS)
{
    char outputData[5];
    am_volume_t deviceVolume;
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, elementName);
    // get device volume based on element name
    if (E_OK == mpPolicyReceive->getVolume(elementType, elementName, deviceVolume))
    {
        // store the volume in std::string format
        sprintf(outputData, "%d", deviceVolume);
        listOutputs.push_back(outputData);
        result = E_OK;
    }
    return result;
}

//find the user i.e main volume based on element type and name
am_Error_e CAmPolicyEngine::_findMainVolume(const gc_Element_e elementType,
                                            const gc_ConditionStruct_s &conditionInstance,
                                            std::string& elementName,
                                            std::vector<std::string > &listOutputs,
                                            const bool isLHS)
{
    char outputData[5];
    am_mainVolume_t volume;
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, elementName);
    // get main volume based on element name
    if (E_OK == mpPolicyReceive->getMainVolume(elementType, elementName, volume))
    {
        // store the volume in std::string format
        sprintf(outputData, "%d", volume);
        listOutputs.push_back(outputData);
        result = E_OK;
    }
    return result;
}

//find the device i.e. router side property value based on element type and name
am_Error_e CAmPolicyEngine::_findDevicePropertyValue(const gc_Element_e elementType,
                                                     const gc_ConditionStruct_s &conditionInstance,
                                                     std::string& elementName,
                                                     std::vector<std::string > &listOutputs,
                                                     const bool isLHS)
{
    char outputData[5];
    int16_t value;
    std::string propertyType;
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, elementName, propertyType);
    am_CustomSoundPropertyType_t property = (am_CustomSoundPropertyType_t)atoi(propertyType.data());
    // get device property based on element name
    if (E_OK == mpPolicyReceive->getSoundProperty(elementType, elementName, property, value))
    {
        // store the value in std::string format
        sprintf(outputData, "%d", value);
        listOutputs.push_back(outputData);
        result = E_OK;
    }
    return result;
}

//find the user i.e main property value based on element type and name
am_Error_e CAmPolicyEngine::_findUserPropertyValue(const gc_Element_e elementType,
                                                   const gc_ConditionStruct_s &conditionInstance,
                                                   std::string& elementName,
                                                   std::vector<std::string > &listOutputs,
                                                   const bool isLHS)
{
    char outputData[5];
    int16_t value;
    std::string propertyType;
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, elementName, propertyType);
    am_CustomMainSoundPropertyType_t property = (am_CustomMainSoundPropertyType_t)atoi(
                    propertyType.data());
    // get user property based on element name
    if (E_OK == mpPolicyReceive->getMainSoundProperty(elementType, elementName, property, value))
    {
        // store the value in std::string format
        sprintf(outputData, "%d", value);
        listOutputs.push_back(outputData);
        result = E_OK;
    }
    return result;
}

am_Error_e CAmPolicyEngine::_findElementName(const gc_ConditionStruct_s &conditionInstance,
                                             std::vector<std::string > &listOutputs,
                                             const std::string& name, const bool isLHS)
{
    am_Error_e result = E_UNKNOWN;
    if (true == _isMacroOfNameAllowed(conditionInstance, isLHS))
    {
        result = E_OK;
        listOutputs.push_back(name);
    }
    return result;
}
// find the name of sink
am_Error_e CAmPolicyEngine::_findSinkName(const gc_ConditionStruct_s &conditionInstance,
                                          std::vector<std::string > &listOutputs,
                                          const gc_triggerParams_s &parameters, const bool isLHS)
{
    return _findElementName(conditionInstance, listOutputs, parameters.sinkName, isLHS);
}

// find the name of source
am_Error_e CAmPolicyEngine::_findSourceName(const gc_ConditionStruct_s &conditionInstance,
                                            std::vector<std::string > &listOutputs,
                                            const gc_triggerParams_s &parameters, const bool isLHS)
{
    return _findElementName(conditionInstance, listOutputs, parameters.sourceName, isLHS);
}

am_Error_e CAmPolicyEngine::_findDomainName(const gc_ConditionStruct_s &conditionInstance,
                                            std::vector<std::string > &listOutputs,
                                            const gc_triggerParams_s &parameters, const bool isLHS)
{
    return _findElementName(conditionInstance, listOutputs, parameters.domainName, isLHS);
}

// find the name of class
am_Error_e CAmPolicyEngine::_findClassName(const gc_ConditionStruct_s &conditionInstance,
                                           std::vector<std::string > &listOutputs,
                                           const gc_triggerParams_s &parameters, const bool isLHS)
{
    LOG_FN_ENTRY(parameters.className);
    return _findElementName(conditionInstance, listOutputs, parameters.className, isLHS);
}

// find the name of connection
am_Error_e CAmPolicyEngine::_findConnectionName(const gc_ConditionStruct_s &conditionInstance,
                                                std::vector<std::string > &listOutputs,
                                                const gc_triggerParams_s &parameters,
                                                const bool isLHS)
{
    return _findElementName(conditionInstance, listOutputs,
                            parameters.sourceName + ":" + parameters.sinkName, isLHS);
}

// find the name of domain from source name
template <typename Telement>
am_Error_e CAmPolicyEngine::_findDomainOrClassOfElementName(
                Telement& elementInstance, const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, std::string& mandatoryParameter,
                const bool isLHS, const bool isClassRequired)
{
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter);
    if (E_OK == mpConfigReader->getElementByName(mandatoryParameter, elementInstance))
    {
        if (true == isClassRequired)
        {
            listOutputs.push_back(elementInstance.className);
        }
        else
        {
            listOutputs.push_back(elementInstance.domainName);
        }
        result = E_OK;
    }
    return result;
}

// find the name of domain from source name
am_Error_e CAmPolicyEngine::_findDomainOfSourceName(const gc_ConditionStruct_s &conditionInstance,
                                                    std::vector<std::string > &listOutputs,
                                                    const gc_triggerParams_s &parameters,
                                                    const bool isLHS)
{
    gc_Source_s sourceInstance;
    std::string mandatoryParameter = parameters.sourceName;
    return _findDomainOrClassOfElementName(sourceInstance, conditionInstance, listOutputs,
                                           mandatoryParameter, isLHS, false);
}

// find the name of domain from sink name
am_Error_e CAmPolicyEngine::_findDomainOfSinkName(const gc_ConditionStruct_s &conditionInstance,
                                                  std::vector<std::string > &listOutputs,
                                                  const gc_triggerParams_s &parameters,
                                                  const bool isLHS)
{
    gc_Sink_s sinkInstance;
    std::string mandatoryParameter = parameters.sinkName;
    return _findDomainOrClassOfElementName(sinkInstance, conditionInstance, listOutputs,
                                           mandatoryParameter, isLHS, false);
}

// find the name of class from sink name
am_Error_e CAmPolicyEngine::_findClassOfSinkName(const gc_ConditionStruct_s &conditionInstance,
                                                 std::vector<std::string > &listOutputs,
                                                 const gc_triggerParams_s &parameters,
                                                 const bool isLHS)
{
    gc_Sink_s sinkInstance;
    std::string mandatoryParameter = parameters.sinkName;
    return _findDomainOrClassOfElementName(sinkInstance, conditionInstance, listOutputs,
                                           mandatoryParameter, isLHS, true);
}

// find the name of class from source name
am_Error_e CAmPolicyEngine::_findClassOfSourceName(const gc_ConditionStruct_s &conditionInstance,
                                                   std::vector<std::string > &listOutputs,
                                                   const gc_triggerParams_s &parameters,
                                                   const bool isLHS)
{
    gc_Source_s sourceInstance;
    std::string mandatoryParameter = parameters.sourceName;
    return _findDomainOrClassOfElementName(sourceInstance, conditionInstance, listOutputs,
                                           mandatoryParameter, isLHS, true);
}

am_Error_e CAmPolicyEngine::_getElementNameList(const std::string& mandatoryParameter,
                                                const std::string& optionalParameter,
                                                std::vector<std::string > &listOutputs,
                                                const gc_triggerParams_s &parameters,
                                                const bool isSinkRequired)
{
    std::vector<gc_ConnectionInfo_s > listConnectionInfo;
    std::vector<gc_ConnectionInfo_s >::iterator itListConnectionInfo;
    am_Error_e result = E_UNKNOWN;
    if (E_OK == _getConnectionList(ET_CLASS, mandatoryParameter, optionalParameter,
                                   listConnectionInfo, parameters))
    {
        for (itListConnectionInfo = listConnectionInfo.begin();
                        itListConnectionInfo != listConnectionInfo.end(); itListConnectionInfo++)
        {
            if (true == isSinkRequired)
            {
                listOutputs.push_back((*itListConnectionInfo).sinkName);
            }
            else
            {
                listOutputs.push_back((*itListConnectionInfo).sourceName);
            }
        }
        result = E_OK;
    }
    return result;
}

//get the list of sink belonging to class using class name
am_Error_e CAmPolicyEngine::_findSinkOfClassName(const gc_ConditionStruct_s &conditionInstance,
                                                 std::vector<std::string > &listOutputs,
                                                 const gc_triggerParams_s &parameters,
                                                 const bool isLHS)
{
    std::string optionalParameter;
    std::string mandatoryParameter = parameters.className;
    am_Error_e result = E_UNKNOWN;

    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter, optionalParameter);
    // if parameter is ALL or empty means all the sink of this class is needed
    if ((FUNCTION_MACRO_SUPPORTED_ALL == optionalParameter) || (true == optionalParameter.empty()))
    {
        std::vector<gc_Sink_s > listSinks;
        std::vector<gc_Sink_s >::iterator itListSinks;
        if (E_OK == mpConfigReader->getListElements(listSinks))
        {
            for (itListSinks = listSinks.begin(); itListSinks != listSinks.end(); itListSinks++)
            {
                if ((*itListSinks).className == mandatoryParameter)
                {
                    listOutputs.push_back((*itListSinks).name);
                }
            }
            result = E_OK;
        }
    }
    else //get the connection list and find the sink name involved in connection
    {
        result = _getElementNameList(mandatoryParameter, optionalParameter, listOutputs, parameters,
                                     true);
    }
    return result;
}

//get the list of source belonging to class using class name
am_Error_e CAmPolicyEngine::_findSourceOfClassName(const gc_ConditionStruct_s &conditionInstance,
                                                   std::vector<std::string > &listOutputs,
                                                   const gc_triggerParams_s &parameters,
                                                   const bool isLHS)
{
    std::string optionalParameter;
    std::string mandatoryParameter = parameters.className;
    am_Error_e result = E_UNKNOWN;

    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter, optionalParameter);
    // if parameter is ALL or empty means all the source of this class is needed
    if ((FUNCTION_MACRO_SUPPORTED_ALL == optionalParameter) || (true == optionalParameter.empty()))
    {
        std::vector<gc_Source_s > listSources;
        std::vector<gc_Source_s >::iterator itListSources;
        if (E_OK == mpConfigReader->getListElements(listSources))
        {
            for (itListSources = listSources.begin(); itListSources != listSources.end();
                            itListSources++)
            {
                if ((*itListSources).className == mandatoryParameter)
                {
                    listOutputs.push_back((*itListSources).name);
                }
            }
            result = E_OK;
        }
    }
    else //get the connection list and find the source name involved in connection
    {
        result = _getElementNameList(mandatoryParameter, optionalParameter, listOutputs, parameters,
                                     false);
    }
    return result;
}

template <typename Telement>
am_Error_e CAmPolicyEngine::_findElementPriority(Telement& elementInstance,
                                                 const gc_ConditionStruct_s &conditionInstance,
                                                 std::string& mandatoryParameter, const bool isLHS,
                                                 std::vector<std::string > &listOutputs)
{
    char outputData[5];
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter);
    if (E_OK == mpConfigReader->getElementByName(mandatoryParameter, elementInstance))
    {
        sprintf(outputData, "%d", elementInstance.priority);
        listOutputs.push_back(outputData);
        result = E_OK;
    }
    return result;
}

// get the priority of sink by sink name
am_Error_e CAmPolicyEngine::_findSinkPriority(const gc_ConditionStruct_s &conditionInstance,
                                              std::vector<std::string > &listOutputs,
                                              const gc_triggerParams_s &parameters,
                                              const bool isLHS)
{
    gc_Sink_s sinkInstance;
    std::string mandatoryParameter = parameters.sinkName;
    return _findElementPriority(sinkInstance, conditionInstance, mandatoryParameter, isLHS,
                                listOutputs);
}

// get the priority of source by source name
am_Error_e CAmPolicyEngine::_findSourcePriority(const gc_ConditionStruct_s &conditionInstance,
                                                std::vector<std::string > &listOutputs,
                                                const gc_triggerParams_s &parameters,
                                                const bool isLHS)
{
    gc_Source_s sourceInstance;
    std::string mandatoryParameter = parameters.sourceName;
    return _findElementPriority(sourceInstance, conditionInstance, mandatoryParameter, isLHS,
                                listOutputs);
}

// get the priority of class by class name
am_Error_e CAmPolicyEngine::_findClassPriority(const gc_ConditionStruct_s &conditionInstance,
                                               std::vector<std::string > &listOutputs,
                                               const gc_triggerParams_s &parameters,
                                               const bool isLHS)
{
    gc_Class_s classInstance;
    std::string mandatoryParameter = parameters.className;
    return _findElementPriority(classInstance, conditionInstance, mandatoryParameter, isLHS,
                                listOutputs);
}

// get the priority of connection by connection name
am_Error_e CAmPolicyEngine::_findConnectionPriority(const gc_ConditionStruct_s &conditionInstance,
                                                    std::vector<std::string > &listOutputs,
                                                    const gc_triggerParams_s &parameters,
                                                    const bool isLHS)
{
    std::vector<gc_ConnectionInfo_s > listConnectionInfo;
    std::vector<gc_ConnectionInfo_s >::iterator itListConnectionInfo;
    char outputData[5];
    am_Error_e result = E_UNKNOWN;
    std::string mandatoryParameter = parameters.sourceName + ":" + parameters.sinkName;

    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter);
    // get list of connection based on connection name
    if (E_OK == mpPolicyReceive->getListMainConnections(ET_CONNECTION, mandatoryParameter,
                                                        listConnectionInfo))
    {
        for (itListConnectionInfo = listConnectionInfo.begin();
                        itListConnectionInfo != listConnectionInfo.end(); itListConnectionInfo++)
        {
            // store the priority in std::string format
            sprintf(outputData, "%d", (*itListConnectionInfo).priority);
            listOutputs.push_back(outputData);
        }
        result = E_OK;
    }
    return result;
}

// get the list of connection priority of connections using class name
am_Error_e CAmPolicyEngine::_findConnectionOfClassPriority(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::vector<gc_ConnectionInfo_s > listConnectionInfo;
    std::vector<gc_ConnectionInfo_s >::iterator itListConnectionInfo;
    char outputData[5];
    std::string optionalParameter;
    std::string mandatoryParameter = parameters.className;
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter, optionalParameter);

    if (E_OK == _getConnectionList(ET_CLASS, mandatoryParameter, optionalParameter,
                                   listConnectionInfo, parameters))
    {
        for (itListConnectionInfo = listConnectionInfo.begin();
                        itListConnectionInfo != listConnectionInfo.end(); itListConnectionInfo++)
        {
            // store the priority in std::string format
            sprintf(outputData, "%d", (*itListConnectionInfo).priority);
            listOutputs.push_back(outputData);
        }
        result = E_OK;
    }
    return result;
}

// get the list of connection state of connections using class name
am_Error_e CAmPolicyEngine::_findConnectionOfClassState(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::vector<gc_ConnectionInfo_s > listConnectionInfo;
    std::vector<gc_ConnectionInfo_s >::iterator itlistConnectionInfo;
    char outputData[5];
    std::string optionalParameter;
    std::string mandatoryParameter = parameters.className;
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter, optionalParameter);
    // get list of connection based on class name
    if (E_OK == _getConnectionList(ET_CLASS, mandatoryParameter, optionalParameter,
                                   listConnectionInfo, parameters))
    {
        for (itlistConnectionInfo = listConnectionInfo.begin();
                        itlistConnectionInfo != listConnectionInfo.end(); itlistConnectionInfo++)
        {
            // store the connection state in std::string format
            sprintf(outputData, "%d", (*itlistConnectionInfo).connectionState);
            listOutputs.push_back(outputData);
        }
        result = E_OK;
    }
    return result;
}

am_Error_e CAmPolicyEngine::_findElementConnectionState(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, std::string& mandatoryParameter,
                const bool isLHS, const gc_Element_e elementType)
{
    am_Error_e result = E_UNKNOWN;
    std::vector<gc_ConnectionInfo_s > listConnectionInfo;
    std::vector<gc_ConnectionInfo_s >::iterator itlistConnectionInfo;
    char outputData[5];
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter);

    // get list of connection based on source name
    if (E_OK == mpPolicyReceive->getListMainConnections(elementType, mandatoryParameter,
                                                        listConnectionInfo))
    {
        result = E_OK;
        for (itlistConnectionInfo = listConnectionInfo.begin();
                        itlistConnectionInfo != listConnectionInfo.end(); itlistConnectionInfo++)
        {
            switch (elementType)
            {
            case ET_SOURCE:
                if ((*itlistConnectionInfo).sourceName == mandatoryParameter)
                {
                    // store the connection state in std::string format
                    sprintf(outputData, "%d", (*itlistConnectionInfo).connectionState);
                    listOutputs.push_back(outputData);
                }
                break;
            case ET_SINK:
                if ((*itlistConnectionInfo).sinkName == mandatoryParameter)
                {
                    // store the connection state in std::string format
                    sprintf(outputData, "%d", (*itlistConnectionInfo).connectionState);
                    listOutputs.push_back(outputData);
                }
                break;
            default:
                result = E_UNKNOWN;
            }
        }
    }
    return result;
}
// get the list of connection state of connections using source name
am_Error_e CAmPolicyEngine::_findSourceConnectionState(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::string mandatoryParameter = parameters.sourceName;
    return _findElementConnectionState(conditionInstance, listOutputs, mandatoryParameter, isLHS,
                                       ET_SOURCE);
}

// get the list of connection state of connections using sink name
am_Error_e CAmPolicyEngine::_findSinkConnectionState(const gc_ConditionStruct_s &conditionInstance,
                                                     std::vector<std::string > &listOutputs,
                                                     const gc_triggerParams_s &parameters,
                                                     const bool isLHS)
{
    std::string mandatoryParameter = parameters.sinkName;
    return _findElementConnectionState(conditionInstance, listOutputs, mandatoryParameter, isLHS,
                                       ET_SINK);
}

//find the sink device volume
am_Error_e CAmPolicyEngine::_findSinkDeviceVolume(const gc_ConditionStruct_s &conditionInstance,
                                                  std::vector<std::string > &listOutputs,
                                                  const gc_triggerParams_s &parameters,
                                                  const bool isLHS)
{
    std::string mandatoryParameter = parameters.sinkName;
    return _findDeviceVolume(ET_SINK, conditionInstance, mandatoryParameter, listOutputs, isLHS);
}

//find the source device volume
am_Error_e CAmPolicyEngine::_findSourceDeviceVolume(const gc_ConditionStruct_s &conditionInstance,
                                                    std::vector<std::string > &listOutputs,
                                                    const gc_triggerParams_s &parameters,
                                                    const bool isLHS)
{
    std::string mandatoryParameter = parameters.sourceName;
    return _findDeviceVolume(ET_SOURCE, conditionInstance, mandatoryParameter, listOutputs, isLHS);
}

//find the connection device volume
am_Error_e CAmPolicyEngine::_findConnectionDeviceVolume(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::string mandatoryParameter = parameters.sourceName + ":" + parameters.sinkName;
    return _findDeviceVolume(ET_CONNECTION, conditionInstance, mandatoryParameter, listOutputs,
                             isLHS);
}

//find the list of volume of connection belonging to given class
am_Error_e CAmPolicyEngine::_findConnectionOfClassDeviceVolume(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::vector<gc_ConnectionInfo_s > listConnectionInfo;
    std::vector<gc_ConnectionInfo_s >::iterator itlistConnectionInfo;
    char outputData[5];
    std::string optionalParameter;
    std::string mandatoryParameter = parameters.className;
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter, optionalParameter);

    //get the list of connections belonging to class based on optional parameter
    if (E_OK == _getConnectionList(ET_CLASS, mandatoryParameter, optionalParameter,
                                   listConnectionInfo, parameters))
    {
        for (itlistConnectionInfo = listConnectionInfo.begin();
                        itlistConnectionInfo != listConnectionInfo.end(); itlistConnectionInfo++)
        {
            // store the priority in std::string format
            sprintf(outputData, "%d", (*itlistConnectionInfo).volume);
            listOutputs.push_back(outputData);
        }
        result = E_OK;
    }
    return result;
}

//find the sink user volume
am_Error_e CAmPolicyEngine::_findSinkMainVolume(const gc_ConditionStruct_s &conditionInstance,
                                                std::vector<std::string > &listOutputs,
                                                const gc_triggerParams_s &parameters,
                                                const bool isLHS)
{
    std::string mandatoryParameter = parameters.sinkName;
    return _findMainVolume(ET_SINK, conditionInstance, mandatoryParameter, listOutputs, isLHS);
}

//find the trigger user volume
am_Error_e CAmPolicyEngine::_findUserMainVolume(const gc_ConditionStruct_s &conditionInstance,
                                                std::vector<std::string > &listOutputs,
                                                const gc_triggerParams_s &parameters,
                                                const bool isLHS)
{
    char outputData[5];
    // store the value in std::string format
    sprintf(outputData, "%d", parameters.mainVolume);
    listOutputs.push_back(outputData);
    return E_OK;
}

//find the trigger error value
am_Error_e CAmPolicyEngine::_findUserErrorValue(const gc_ConditionStruct_s &conditionInstance,
                                                std::vector<std::string > &listOutputs,
                                                const gc_triggerParams_s &parameters,
                                                const bool isLHS)
{
    char outputData[5];
    // store the value in std::string format
    sprintf(outputData, "%d", parameters.status);
    listOutputs.push_back(outputData);
    return E_OK;
}

//find the source user volume
am_Error_e CAmPolicyEngine::_findSourceMainVolume(const gc_ConditionStruct_s &conditionInstance,
                                                  std::vector<std::string > &listOutputs,
                                                  const gc_triggerParams_s &parameters,
                                                  const bool isLHS)
{
    std::string mandatoryParameter = parameters.sourceName;
    return _findMainVolume(ET_SOURCE, conditionInstance, mandatoryParameter, listOutputs, isLHS);
}

//find the sink device property value
am_Error_e CAmPolicyEngine::_findSinkDevicePropertyValue(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::string mandatoryParameter = parameters.sinkName;
    return _findDevicePropertyValue(ET_SINK, conditionInstance, mandatoryParameter, listOutputs,
                                    isLHS);
}

//find the source device property value
am_Error_e CAmPolicyEngine::_findSourceDevicePropertyValue(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::string mandatoryParameter = parameters.sourceName;
    return _findDevicePropertyValue(ET_SOURCE, conditionInstance, mandatoryParameter, listOutputs,
                                    isLHS);
}

//find the sink user property value
am_Error_e CAmPolicyEngine::_findSinkUserPropertyValue(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::string mandatoryParameter = parameters.sinkName;
    return _findUserPropertyValue(ET_SINK, conditionInstance, mandatoryParameter, listOutputs,
                                  isLHS);
}

//find the trigger user property value
am_Error_e CAmPolicyEngine::_findUserMainSoundPropertyValue(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    char outputData[5];
    // store the value in std::string format
    sprintf(outputData, "%d", parameters.mainSoundProperty.value);
    listOutputs.push_back(outputData);
    return E_OK;
}

//find the trigger user property type
am_Error_e CAmPolicyEngine::_findUserMainSoundPropertyType(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    char outputData[5];
    // store the value in std::string format
    sprintf(outputData, "%d", parameters.mainSoundProperty.type);
    listOutputs.push_back(outputData);
    return E_OK;
}

//find the source user property value
am_Error_e CAmPolicyEngine::_findSourceUserPropertyValue(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::string mandatoryParameter = parameters.sourceName;
    return _findUserPropertyValue(ET_SOURCE, conditionInstance, mandatoryParameter, listOutputs,
                                  isLHS);
}

//find the trigger user property value
am_Error_e CAmPolicyEngine::_findUserSystemPropertyValue(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    char outputData[5];
    // store the value in std::string format
    sprintf(outputData, "%d", parameters.systemProperty.value);
    listOutputs.push_back(outputData);
    return E_OK;
}

//find the trigger user property type
am_Error_e CAmPolicyEngine::_findUserSystemPropertyType(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    char outputData[5];
    // store the value in std::string format
    sprintf(outputData, "%d", parameters.systemProperty.type);
    listOutputs.push_back(outputData);
    return E_OK;
}

//find the system property value
am_Error_e CAmPolicyEngine::_findSystemPropertyValue(const gc_ConditionStruct_s &conditionInstance,
                                                     std::vector<std::string > &listOutputs,
                                                     const gc_triggerParams_s &parameters,
                                                     const bool isLHS)
{
    char outputData[5];
    int16_t value;
    std::string propertyType;
    am_Error_e result = E_UNKNOWN;
    // convert the macro in value based on schema and store in std::string format
    sprintf(outputData, "%u", parameters.systemProperty.type);
    propertyType = outputData;
    _getValueOfParameter(conditionInstance, isLHS, propertyType);
    am_CustomSystemPropertyType_t property = (am_CustomSystemPropertyType_t)atoi(
                    propertyType.data());
    // get system property
    if (E_OK == mpPolicyReceive->getSystemProperty(property, value))
    {
        // store the value in std::string format
        sprintf(outputData, "%d", value);
        listOutputs.push_back(outputData);
        result = E_OK;
    }
    return result;
}

am_Error_e CAmPolicyEngine::_findUserMuteState(const gc_ConditionStruct_s &conditionInstance,
                                               std::vector<std::string > &listOutputs,
                                               const gc_triggerParams_s &parameters,
                                               const bool isLHS)
{
    char outputData[5];
    sprintf(outputData, "%d", parameters.muteState);
    listOutputs.push_back(outputData);
    return E_OK;
}

am_Error_e CAmPolicyEngine::_findMuteState(const gc_ConditionStruct_s &conditionInstance,
                                           std::vector<std::string > &listOutputs,
                                           std::string& mandatoryParameter, const bool isLHS,
                                           const gc_Element_e elementType)
{
    char outputData[5];
    am_MuteState_e muteState;
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter);

    if (E_OK == mpPolicyReceive->getMuteState(elementType, mandatoryParameter, muteState))
    {
        // store the value in std::string format
        sprintf(outputData, "%d", muteState);
        listOutputs.push_back(outputData);
        result = E_OK;
    }
    return result;
}

//find the mute state of sink
am_Error_e CAmPolicyEngine::_findSinkMuteState(const gc_ConditionStruct_s &conditionInstance,
                                               std::vector<std::string > &listOutputs,
                                               const gc_triggerParams_s &parameters,
                                               const bool isLHS)
{
    std::string mandatoryParameter = parameters.sinkName;
    return _findMuteState(conditionInstance, listOutputs, mandatoryParameter, isLHS, ET_SINK);
}

//find the mute state of active connection of class
am_Error_e CAmPolicyEngine::_findClassMuteState(const gc_ConditionStruct_s &conditionInstance,
                                                std::vector<std::string > &listOutputs,
                                                const gc_triggerParams_s &parameters,
                                                const bool isLHS)
{
    std::string mandatoryParameter = parameters.className;
    return _findMuteState(conditionInstance, listOutputs, mandatoryParameter, isLHS, ET_CLASS);
}

//find the mute state of connection
am_Error_e CAmPolicyEngine::_findConnectionMuteState(const gc_ConditionStruct_s &conditionInstance,
                                                     std::vector<std::string > &listOutputs,
                                                     const gc_triggerParams_s &parameters,
                                                     const bool isLHS)
{
    std::string mandatoryParameter = parameters.sourceName + ":" + parameters.sinkName;
    return _findMuteState(conditionInstance, listOutputs, mandatoryParameter, isLHS, ET_CONNECTION);
}

am_Error_e CAmPolicyEngine::_findAvailability(const gc_ConditionStruct_s &conditionInstance,
                                              std::vector<std::string > &listOutputs,
                                              std::string& mandatoryParameter, const bool isLHS,
                                              const gc_Element_e elementType,
                                              const bool isReasonRequired)
{
    char outputData[5];
    am_Availability_s availability;
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter);
    if (E_OK == mpPolicyReceive->getAvailability(elementType, mandatoryParameter, availability))
    {
        if (true == isReasonRequired)
        {
            // store the value in std::string format
            sprintf(outputData, "%d", availability.availabilityReason);
        }
        else
        {
            // store the value in std::string format
            sprintf(outputData, "%d", availability.availability);
        }
        listOutputs.push_back(outputData);
        result = E_OK;
    }
    return result;
}
//find the sink availability
am_Error_e CAmPolicyEngine::_findSinkAvailability(const gc_ConditionStruct_s &conditionInstance,
                                                  std::vector<std::string > &listOutputs,
                                                  const gc_triggerParams_s &parameters,
                                                  const bool isLHS)
{
    std::string mandatoryParameter = parameters.sinkName;
    return _findAvailability(conditionInstance, listOutputs, mandatoryParameter, isLHS, ET_SINK,
                             false);
}

//find the source availability
am_Error_e CAmPolicyEngine::_findSourceAvailability(const gc_ConditionStruct_s &conditionInstance,
                                                    std::vector<std::string > &listOutputs,
                                                    const gc_triggerParams_s &parameters,
                                                    const bool isLHS)
{
    std::string mandatoryParameter = parameters.sourceName;
    return _findAvailability(conditionInstance, listOutputs, mandatoryParameter, isLHS, ET_SOURCE,
                             false);
}

//find the sink availability reason
am_Error_e CAmPolicyEngine::_findSinkAvailabilityReason(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::string mandatoryParameter = parameters.sinkName;
    return _findAvailability(conditionInstance, listOutputs, mandatoryParameter, isLHS, ET_SINK,
                             true);
}

//find the source availability reason
am_Error_e CAmPolicyEngine::_findSourceAvailabilityReason(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::string mandatoryParameter = parameters.sourceName;
    return _findAvailability(conditionInstance, listOutputs, mandatoryParameter, isLHS, ET_SOURCE,
                             true);
}

//Reserved for future use
am_Error_e CAmPolicyEngine::_findConnectionFormat(const gc_ConditionStruct_s &conditionInstance,
                                                  std::vector<std::string > &listOutputs,
                                                  const gc_triggerParams_s &parameters,
                                                  const bool isLHS)
{
    (void)conditionInstance;
    (void)listOutputs;
    (void)parameters;
    (void)isLHS;
    char outputData[5];
    // store the value in std::string format
    sprintf(outputData, "%d", CF_GENIVI_STEREO);
    listOutputs.push_back(outputData);
    return E_OK;
}

//Reserved for future use
am_Error_e CAmPolicyEngine::_findConnectionOfClassFormat(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    (void)conditionInstance;
    (void)listOutputs;
    (void)parameters;
    (void)isLHS;
    char outputData[5];
    // store the value in std::string format
    sprintf(outputData, "%d", CF_GENIVI_STEREO);
    listOutputs.push_back(outputData);
    return E_OK;
}
am_Error_e CAmPolicyEngine::_findInterruptState(const gc_ConditionStruct_s &conditionInstance,
                                                std::vector<std::string > &listOutputs,
                                                std::string& mandatoryParameter, const bool isLHS,
                                                gc_Element_e elementType)
{
    char outputData[5];
    am_InterruptState_e interruptState;
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter);
    if (E_OK == mpPolicyReceive->getInterruptState(elementType, mandatoryParameter, interruptState))
    {
        // store the value in std::string format
        sprintf(outputData, "%d", interruptState);
        listOutputs.push_back(outputData);
        result = E_OK;
    }
    return result;
}
//find the source interrupt state
am_Error_e CAmPolicyEngine::_findSourceInterruptState(const gc_ConditionStruct_s &conditionInstance,
                                                      std::vector<std::string > &listOutputs,
                                                      const gc_triggerParams_s &parameters,
                                                      const bool isLHS)
{
    std::string mandatoryParameter = parameters.sourceName;
    return _findInterruptState(conditionInstance, listOutputs, mandatoryParameter, isLHS, ET_SOURCE);
}

//find the connection interrupt state
am_Error_e CAmPolicyEngine::_findConnectionInterruptState(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    std::string mandatoryParameter;
    return _findInterruptState(conditionInstance, listOutputs, mandatoryParameter, isLHS,
                               ET_CONNECTION);
}

//check sink is registered or not
am_Error_e CAmPolicyEngine::_findSinkIsRegistered(const gc_ConditionStruct_s &conditionInstance,
                                                  std::vector<std::string > &listOutputs,
                                                  const gc_triggerParams_s &parameters,
                                                  const bool isLHS)
{
    bool result;
    char outputData[5];
    std::string mandatoryParameter = parameters.sinkName;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter);
    // get registration status
    result = mpPolicyReceive->isRegistered(ET_SINK, mandatoryParameter);
    // store the value in std::string format
    sprintf(outputData, "%d", result);
    listOutputs.push_back(outputData);
    return E_OK;
}

//check source is registered or not
am_Error_e CAmPolicyEngine::_findSourceIsRegistered(const gc_ConditionStruct_s &conditionInstance,
                                                    std::vector<std::string > &listOutputs,
                                                    const gc_triggerParams_s &parameters,
                                                    const bool isLHS)
{
    bool result;
    char outputData[5];
    std::string mandatoryParameter = parameters.sourceName;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter);
    // get registration status
    result = mpPolicyReceive->isRegistered(ET_SOURCE, mandatoryParameter);
    // store the value in std::string format
    sprintf(outputData, "%d", result);
    listOutputs.push_back(outputData);
    return E_OK;
}

//check domain is registered or not
am_Error_e CAmPolicyEngine::_findDomainIsRegistered(const gc_ConditionStruct_s &conditionInstance,
                                                    std::vector<std::string > &listOutputs,
                                                    const gc_triggerParams_s &parameters,
                                                    const bool isLHS)
{
    bool result = false;
    am_Error_e returnValue = E_UNKNOWN;
    char outputData[5];
    gc_Domain_s domain;
    std::vector<gc_Domain_s > listDomains;
    std::vector<gc_Domain_s >::iterator itListDomains;

    std::string mandatoryParameter = parameters.domainName;
    if (true == _isSingleDomainRequest(conditionInstance, isLHS, mandatoryParameter))
    {
        if (E_OK == mpConfigReader->getElementByName(mandatoryParameter, domain))
        {
            listDomains.push_back(domain);
        }
    }
    else //check for all the domains
    {
        mpConfigReader->getListElements(listDomains);
    }
    for (itListDomains = listDomains.begin(); itListDomains != listDomains.end(); itListDomains++)
    {
        returnValue = E_OK;
        // get registration complete status
        result = mpPolicyReceive->isRegistered(ET_DOMAIN, (*itListDomains).name);
        if (false == result)
        {
            LOG_FN_DEBUG("domain not registered:", (*itListDomains).name);
            break;
        }
    }
    // store the value in std::string format
    sprintf(outputData, "%d", result);
    listOutputs.push_back(outputData);
    return returnValue;
}

//check domain registration is completed or not
am_Error_e CAmPolicyEngine::_findIsDomainRegistrationComplete(
                const gc_ConditionStruct_s &conditionInstance,
                std::vector<std::string > &listOutputs, const gc_triggerParams_s &parameters,
                const bool isLHS)
{
    bool result = false;
    char outputData[5];
    std::string mandatoryParameter = parameters.domainName;
    am_Error_e returnValue = E_UNKNOWN;
    gc_Domain_s domain;
    std::vector<gc_Domain_s > listDomains;
    std::vector<gc_Domain_s >::iterator itListDomains;

    if (true == _isSingleDomainRequest(conditionInstance, isLHS, mandatoryParameter))
    {
        if (E_OK == mpConfigReader->getElementByName(mandatoryParameter, domain))
        {
            listDomains.push_back(domain);
        }
    }
    else //check for all the domains
    {
        mpConfigReader->getListElements(listDomains);
    }
    for (itListDomains = listDomains.begin(); itListDomains != listDomains.end(); itListDomains++)
    {
        returnValue = E_OK;
        // get registration complete status
        result = mpPolicyReceive->isDomainRegistrationComplete((*itListDomains).name);
        if (false == result)
        {
            LOG_FN_ERROR("domain registration not complete:", (*itListDomains).name);
            break;
        }
    }
    // store the value in std::string format
    sprintf(outputData, "%d", result);
    listOutputs.push_back(outputData);
    return returnValue;
}

// get the source state
am_Error_e CAmPolicyEngine::_findSourceState(const gc_ConditionStruct_s &conditionInstance,
                                             std::vector<std::string > &listOutputs,
                                             const gc_triggerParams_s &parameters, const bool isLHS)
{
    char outputData[5];
    int sourceState;
    am_Error_e result = E_UNKNOWN;
    std::string mandatoryParameter = parameters.sourceName;
    _getValueOfParameter(conditionInstance, isLHS, mandatoryParameter);
    if (E_OK == mpPolicyReceive->getState(ET_SOURCE, mandatoryParameter, sourceState))
    {
        // store the value in std::string format
        sprintf(outputData, "%d", (am_SourceState_e)sourceState);
        listOutputs.push_back(outputData);
        result = E_OK;
    }
    return result;
}

//get the domain state
am_Error_e CAmPolicyEngine::_findDomainState(const gc_ConditionStruct_s &conditionInstance,
                                             std::vector<std::string > &listOutputs,
                                             const gc_triggerParams_s &parameters, const bool isLHS)
{
    char outputData[5];
    int domainState;
    am_Error_e result = E_OK;
    gc_Domain_s domain;
    std::vector<gc_Domain_s > listDomains;
    std::vector<gc_Domain_s >::iterator itListDomains;

    std::string mandatoryParameter = parameters.domainName;
    if (true == _isSingleDomainRequest(conditionInstance, isLHS, mandatoryParameter))
    {
        if (E_OK == mpConfigReader->getElementByName(mandatoryParameter, domain))
        {
            listDomains.push_back(domain);
        }
    }
    else // get state for all the domains
    {
        mpConfigReader->getListElements(listDomains);
    }
    for (itListDomains = listDomains.begin(); itListDomains != listDomains.end(); itListDomains++)
    {
        // get domain state
        if (E_OK != mpPolicyReceive->getState(ET_DOMAIN, (*itListDomains).name, domainState))
        {
            result = E_UNKNOWN;
            break;
        }
        // store the value in std::string format
        sprintf(outputData, "%d", (am_DomainState_e)domainState);
        listOutputs.push_back(outputData);
    }
    return result;
}
bool CAmPolicyEngine::_executeFunction(const std::string& functionName, const std::string& category,
                                       const gc_ConditionStruct_s &conditionInstance,
                                       std::vector<std::string > &listOutputs,
                                       const gc_triggerParams_s &parameters, const bool isLHS)
{
    am_Error_e error = (this->*mMapFunctionNameToFunctionMaps[functionName][category])(
                    conditionInstance, listOutputs, parameters, isLHS);
    return (error == E_OK) ? true : false;
}

void CAmPolicyEngine::_getListStaticSinks(const std::string& domainName,
                                          std::vector<gc_Sink_s >& listStaticSinks)
{
    std::vector<gc_Sink_s > listSinks;
    std::vector<gc_Sink_s >::iterator itListSinks;
    mpConfigReader->getListElements(listSinks);
    for (itListSinks = listSinks.begin(); itListSinks != listSinks.end(); itListSinks++)
    {
        if ((itListSinks->registrationType == REG_CONTROLLER) && (itListSinks->domainName
                        == domainName))
        {
            listStaticSinks.push_back(*itListSinks);
        }
    }
}
void CAmPolicyEngine::_getListStaticSources(const std::string& domainName,
                                            std::vector<gc_Source_s >& listStaticSources)
{
    std::vector<gc_Source_s > listSources;
    std::vector<gc_Source_s >::iterator itListSources;
    mpConfigReader->getListElements(listSources);
    for (itListSources = listSources.begin(); itListSources != listSources.end(); itListSources++)
    {
        if ((itListSources->registrationType == REG_CONTROLLER) && (itListSources->domainName
                        == domainName))
        {
            listStaticSources.push_back(*itListSources);
        }
    }
}

void CAmPolicyEngine::_getListStaticGateways(std::vector<std::string >& listGateways)
{
    std::vector<gc_Gateway_s >::iterator itListGateways;
    std::vector<gc_Gateway_s > listConfiguredGateways;
    mpConfigReader->getListElements(listConfiguredGateways);
    for (itListGateways = listConfiguredGateways.begin();
                    itListGateways != listConfiguredGateways.end(); itListGateways++)
    {
        if (itListGateways->registrationType == REG_CONTROLLER)
        {
            if (false == mpPolicyReceive->isRegistered(ET_SOURCE, itListGateways->sourceName))
            {
                continue;
            }
            if (false == mpPolicyReceive->isRegistered(ET_SINK, itListGateways->sinkName))
            {
                continue;
            }
            if (true == mpPolicyReceive->isRegistered(ET_GATEWAY, itListGateways->name))
            {
                continue;
            }
            listGateways.push_back(itListGateways->name);
        }
    }
}

void CAmPolicyEngine::_getImplicitActions(gc_Trigger_e trigger,
                                          std::vector<gc_Action_s >& listActions,
                                          const gc_triggerParams_s& parameters)
{
    uint8_t flags = 0;
    std::vector<gc_Sink_s > listStaticSinks;
    std::vector<gc_Sink_s >::iterator itListStaticSinks;
    std::vector<gc_Source_s > listStaticSources;
    std::vector<gc_Source_s >::iterator itListStaticSources;
    std::vector<std::string > listStaticGateways;
    std::vector<std::string >::iterator itListStaticGateways;
    gc_Action_s actionRegister;
    if ((trigger != SYSTEM_REGISTER_DOMAIN) && (trigger != SYSTEM_REGISTER_SINK)
        && (trigger != SYSTEM_REGISTER_SOURCE) && (trigger != SYSTEM_DOMAIN_REGISTRATION_COMPLETE))
    {
        return;
    }
    else if (trigger == SYSTEM_REGISTER_DOMAIN)
    {
        flags = (SEARCH_STATIC_SOURCE | SEARCH_STATIC_SINK | SEARCH_STATIC_GATEWAY);
    }
    else if (trigger == SYSTEM_REGISTER_SINK || trigger == SYSTEM_REGISTER_SOURCE)
    {
        flags = SEARCH_STATIC_GATEWAY;
    }
    if (flags & SEARCH_STATIC_SINK)
    {
        _getListStaticSinks(parameters.domainName, listStaticSinks);
    }
    if (flags & SEARCH_STATIC_SOURCE)
    {
        _getListStaticSources(parameters.domainName, listStaticSources);
    }
    if (flags & SEARCH_STATIC_GATEWAY)
    {
        _getListStaticGateways(listStaticGateways);
    }
    actionRegister.actionName = CONFIG_ACTION_NAME_REGISTER;
    for (itListStaticSources = listStaticSources.begin();
                    itListStaticSources != listStaticSources.end(); ++itListStaticSources)
    {
        actionRegister.mapParameters[ACTION_PARAM_SOURCE_NAME] += (*itListStaticSources).name + " ";
    }
    for (itListStaticSinks = listStaticSinks.begin(); itListStaticSinks != listStaticSinks.end();
                    ++itListStaticSinks)
    {
        actionRegister.mapParameters[ACTION_PARAM_SINK_NAME] += (*itListStaticSinks).name + " ";
    }
    for (itListStaticGateways = listStaticGateways.begin();
                    itListStaticGateways != listStaticGateways.end(); ++itListStaticGateways)
    {
        actionRegister.mapParameters[ACTION_PARAM_GATEWAY_NAME] += *itListStaticGateways + " ";
    }
    // append the action only if some source sink or gateway is found
    if (actionRegister.mapParameters.size() > 0)
    {
        listActions.push_back(actionRegister);
    }
}
am_Error_e CAmPolicyEngine::getListSystemProperties(
                std::vector<am_SystemProperty_s >& listSystemProperties)
{
    mpConfigReader->getListSystemProperties(listSystemProperties);
    return E_OK;
}
am_Error_e CAmPolicyEngine::getListClasses(std::vector<gc_Class_s >& listClasses)
{
    return mpConfigReader->getListElements(listClasses);
}

} /* namespace gc */
} /* namespace am */
