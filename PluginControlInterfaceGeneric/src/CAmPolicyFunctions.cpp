/******************************************************************************
 * @file CAmPolicyFunctions.cpp
 *
 * This class encapsulates the evaluation of conditions as defined in policy configuration
 * (see page @ref cond). It is used by the policy engine alone.
 *
 * Internally it maintains a map of member functions implementing the available
 * @ref functionTable, which are selected by function name string as given in the
 * policy rule.
 *
 * @component AudioManager Generic Controller
 *
 * @authors Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2019 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 *****************************************************************************/

#include "CAmPolicyFunctions.h"
#include "IAmPolicyReceive.h"
#include "CAmConfigurationReader.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmDomainElement.h"
#include "CAmMainConnectionElement.h"
#include "CAmPolicyEngine.h"

#include <cstring>

namespace am {
namespace gc {

CAmPolicyFunctions::CAmPolicyFunctions(IAmPolicyReceive *pPolicyReceive)
    : mpPolicyReceive(pPolicyReceive)
{
    // function (as supported in conditions of policy ) mapping to returns string or integer
    mMapFunctionReturnValue[FUNCTION_NAME]                                   = true;
    mMapFunctionReturnValue[FUNCTION_ELEMENTS]                               = true;
    mMapFunctionReturnValue[FUNCTION_PRIORITY]                               = false;
    mMapFunctionReturnValue[FUNCTION_CONNECTION_STATE]                       = false;
    mMapFunctionReturnValue[FUNCTION_VOLUME]                                 = false;
    mMapFunctionReturnValue[FUNCTION_MAIN_VOLUME]                            = false;
    mMapFunctionReturnValue[FUNCTION_MAIN_VOLUME_STEP]                       = false;
    mMapFunctionReturnValue[FUNCTION_VOLUME_LIMIT]                           = false;
    mMapFunctionReturnValue[FUNCTION_SOUND_PROPERTY]                         = false;
    mMapFunctionReturnValue[FUNCTION_MAIN_SOUND_PROPERTY_TYPE]               = false;
    mMapFunctionReturnValue[FUNCTION_SYSTEM_PROPERTY_TYPE]                   = false;
    mMapFunctionReturnValue[FUNCTION_MAIN_SOUND_PROPERTY_VALUE]              = false;
    mMapFunctionReturnValue[FUNCTION_SYSTEM_PROPERTY_VALUE]                  = false;
    mMapFunctionReturnValue[FUNCTION_MUTE_STATE]                             = false;
    mMapFunctionReturnValue[FUNCTION_IS_REGISTRATION_COMPLETE]               = false;
    mMapFunctionReturnValue[FUNCTION_AVAILABILITY]                           = false;
    mMapFunctionReturnValue[FUNCTION_AVAILABILITY_REASON]                    = false;
    mMapFunctionReturnValue[FUNCTION_CONNECTION_FORMAT]                      = false;
    mMapFunctionReturnValue[FUNCTION_INTERRUPT_STATE]                        = false;
    mMapFunctionReturnValue[FUNCTION_IS_REGISTERED]                          = false;
    mMapFunctionReturnValue[FUNCTION_STATE]                                  = false;
    mMapFunctionReturnValue[FUNCTION_PEEK]                                   = true;
    mMapFunctionReturnValue[FUNCTION_ERROR]                                  = false;
    mMapFunctionReturnValue[FUNCTION_NOTIFICATION_CONFIGURATION_STATUS]      = false;
    mMapFunctionReturnValue[FUNCTION_NOTIFICATION_CONFIGURATION_PARAM]       = false;
    mMapFunctionReturnValue[FUNCTION_NOTIFICATION_DATA_VALUE]                = false;
    mMapFunctionReturnValue[FUNCTION_NOTIFICATION_DATA_TYPE]                 = false;
    mMapFunctionReturnValue[FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_TYPE]   = false;
    mMapFunctionReturnValue[FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_STATUS] = false;
    mMapFunctionReturnValue[FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_PARAM]  = false;
    mMapFunctionReturnValue[FUNCTION_SCALE]                                  = false;
    mMapFunctionReturnValue[FUNCTION_COUNT]                                  = false;

    // name of the functions as supported in condition tag in configuration
    mMapFunctionNameToFunctionMaps[FUNCTION_NAME]                                   = &CAmPolicyFunctions::_findName;
    mMapFunctionNameToFunctionMaps[FUNCTION_ELEMENTS]                               = &CAmPolicyFunctions::_findSubItems;
    mMapFunctionNameToFunctionMaps[FUNCTION_PRIORITY]                               = &CAmPolicyFunctions::_findPriority;
    mMapFunctionNameToFunctionMaps[FUNCTION_CONNECTION_STATE]                       = &CAmPolicyFunctions::_findConnectionState;
    mMapFunctionNameToFunctionMaps[FUNCTION_VOLUME]                                 = &CAmPolicyFunctions::_findDeviceVolume;
    mMapFunctionNameToFunctionMaps[FUNCTION_MAIN_VOLUME]                            = &CAmPolicyFunctions::_findMainVolume;
    mMapFunctionNameToFunctionMaps[FUNCTION_MAIN_VOLUME_STEP]                       = &CAmPolicyFunctions::_findIsMainVolumeStep;
    mMapFunctionNameToFunctionMaps[FUNCTION_VOLUME_LIMIT]                           = &CAmPolicyFunctions::_findLimit;
    mMapFunctionNameToFunctionMaps[FUNCTION_SOUND_PROPERTY]                         = &CAmPolicyFunctions::_findDevicePropertyValue;
    mMapFunctionNameToFunctionMaps[FUNCTION_MAIN_SOUND_PROPERTY_VALUE]              = &CAmPolicyFunctions::_findMainSoundPropertyValue;
    mMapFunctionNameToFunctionMaps[FUNCTION_MAIN_SOUND_PROPERTY_TYPE]               = &CAmPolicyFunctions::_findMainSoundPropertyType;
    mMapFunctionNameToFunctionMaps[FUNCTION_SYSTEM_PROPERTY_TYPE]                   = &CAmPolicyFunctions::_findSystemPropertyType;
    mMapFunctionNameToFunctionMaps[FUNCTION_SYSTEM_PROPERTY_VALUE]                  = &CAmPolicyFunctions::_findSystemPropertyValue;
    mMapFunctionNameToFunctionMaps[FUNCTION_MUTE_STATE]                             = &CAmPolicyFunctions::_findMuteState;
    mMapFunctionNameToFunctionMaps[FUNCTION_IS_REGISTRATION_COMPLETE]               = &CAmPolicyFunctions::_findIsDomainRegistrationComplete;
    mMapFunctionNameToFunctionMaps[FUNCTION_AVAILABILITY]                           = &CAmPolicyFunctions::_findAvailability;
    mMapFunctionNameToFunctionMaps[FUNCTION_AVAILABILITY_REASON]                    = &CAmPolicyFunctions::_findAvailabilityReason;
    mMapFunctionNameToFunctionMaps[FUNCTION_INTERRUPT_STATE]                        = &CAmPolicyFunctions::_findInterruptState;
    mMapFunctionNameToFunctionMaps[FUNCTION_IS_REGISTERED]                          = &CAmPolicyFunctions::_findIsRegistered;
    mMapFunctionNameToFunctionMaps[FUNCTION_STATE]                                  = &CAmPolicyFunctions::_findState;
    mMapFunctionNameToFunctionMaps[FUNCTION_CONNECTION_FORMAT]                      = &CAmPolicyFunctions::_findConnectionFormat;
    mMapFunctionNameToFunctionMaps[FUNCTION_PEEK]                                   = &CAmPolicyFunctions::_findPeek;
    mMapFunctionNameToFunctionMaps[FUNCTION_ERROR]                                  = &CAmPolicyFunctions::_findUserErrorValue;
    mMapFunctionNameToFunctionMaps[FUNCTION_NOTIFICATION_CONFIGURATION_STATUS]      = &CAmPolicyFunctions::_findNTStatus;
    mMapFunctionNameToFunctionMaps[FUNCTION_NOTIFICATION_CONFIGURATION_PARAM]       = &CAmPolicyFunctions::_findNTParam;
    mMapFunctionNameToFunctionMaps[FUNCTION_NOTIFICATION_DATA_VALUE]                = &CAmPolicyFunctions::_findUserNotificationValue;
    mMapFunctionNameToFunctionMaps[FUNCTION_NOTIFICATION_DATA_TYPE]                 = &CAmPolicyFunctions::_findNotificationType;
    mMapFunctionNameToFunctionMaps[FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_TYPE]   = &CAmPolicyFunctions::_findUserNotificationType;
    mMapFunctionNameToFunctionMaps[FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_STATUS] = &CAmPolicyFunctions::_findMainNTStatus;
    mMapFunctionNameToFunctionMaps[FUNCTION_MAIN_NOTIFICATION_CONFIGURATION_PARAM]  = &CAmPolicyFunctions::_findMainNTParam;
    mMapFunctionNameToFunctionMaps[FUNCTION_SCALE]                                  = &CAmPolicyFunctions::_findScale;
    mMapFunctionNameToFunctionMaps[FUNCTION_COUNT]                                  = &CAmPolicyFunctions::_findCount;
}

bool CAmPolicyFunctions::evaluateConditionSet(
    const std::vector<gc_ConditionStruct_s > &listConditionSets,
    const gc_triggerParams_s &parameters)
{
    for (const auto &condition : listConditionSets)
    {
        if (!_isConditionTrue(condition, parameters))
        {
            return false;
        }
    }

    return true;
}

void CAmPolicyFunctions::evaluateParameter(const gc_triggerParams_s &trigger, std::string &parameter)
{
    CAmPolicyEngine::evaluateMacro(trigger, parameter);

    // if parameter is a nested function, resolve it
    const char *str = parameter.c_str();
    if (strchr(str, '(') == NULL)
    {
        return;
    }

    std::string orig(parameter);
    gc_FunctionElement_s      nested;
    std::vector<std::string > innerList;
    unsigned int              position = 0;
    if (E_OK == CAmXmlConfigParser::parsePolicyFunction(str, position, nested))
    {

        bool success = _executeFunction(nested, trigger, innerList);
        if (!success)
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "FAILED evaluating:", parameter);
            return;
        }

        parameter.clear();
        bool first = true;
        for (const auto &value : innerList)
        {
            parameter += (first ? "" : " ") + value;
            first      = false;
        }
    }

    if (parameter != orig)
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, trigger.triggerType, orig, "-->", parameter);
    }
}

bool CAmPolicyFunctions::_executeFunction(gc_FunctionElement_s function
    , const gc_triggerParams_s &trigger, std::vector<std::string > &listOutputs)
{
    // resolve nested functions
    evaluateParameter(trigger, function.mandatoryParameter);
    evaluateParameter(trigger, function.optionalParameter);
    evaluateParameter(trigger, function.optionalParameter2);

    if (mMapFunctionNameToFunctionMaps.count(function.functionName) == 0)
    {
        LOG_FN_ERROR("condition", function.functionName, "(", function.category, ", ...) not allowed");
        return false;
    }

    return (this->*mMapFunctionNameToFunctionMaps.at(function.functionName))(function, trigger, listOutputs) == E_OK;
}

// file-local helper function
template <typename Toperand>
bool _conditionResult(Toperand lhsData, gc_Operator_e opType, Toperand rhdData)
{
    bool result;
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

    if (result)
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "matches at", lhsData, opType, rhdData);
    }

    return result;
}

bool CAmPolicyFunctions::_isConditionTrue(const gc_ConditionStruct_s &condition,
    const gc_triggerParams_s &parameters)
{
    std::string                         LHSString, RHSString;
    int32_t                             LHSInt, RHSInt;
    std::vector<std::string >           listLHSOutputs;
    std::vector<std::string >           listRHSOutputs;
    std::vector<std::string >::iterator itListOutputs;
    bool                                result;
    const gc_Operator_e                &operation = condition.operation;

    // main evaluation logic comes here!!!!!!!!

    // convert operator from string to enum type
    // invoke the function to get the LHS string from condition
    result = _executeFunction(condition.leftObject, parameters, listLHSOutputs);
    if (false == result)
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, " LHS function return error. function->category 1",
            condition.leftObject.functionName, condition.leftObject.category);
        return result;
    }

    /*
     * Even if the returned list is empty, the condition should be evaluated
     * the reason is EXC condition would pass for empty list.
     */

    // get the RHS string from condition
    if (true == condition.rightObject.isValue) // direct value is provided by user
    {
        RHSString = condition.rightObject.directValue;
    }
    else
    {
        // invoke the function to get the RHS string from condition
        result = _executeFunction(condition.rightObject.functionObject, parameters, listRHSOutputs);
        if (false == result)
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, " RHS function return error. function->category 3",
                condition.rightObject.functionObject.functionName,
                condition.rightObject.functionObject.category);
            return result;
        }

        if (true == listRHSOutputs.empty())
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, " RHS function does not return any value function->category 4",
                condition.rightObject.functionObject.functionName,
                condition.rightObject.functionObject.category,
                condition.rightObject.functionObject.mandatoryParameter,
                condition.rightObject.functionObject.optionalParameter);
            return false;
        }

        RHSString = listRHSOutputs[0];
    }

    if ((INC == operation) || (EXC == operation)) // operator is include or exclude operator
    {
        // if LHS function returns integer, convert RHS symbolic constant to numeric value
        if (!mMapFunctionReturnValue[condition.leftObject.functionName])
        {
            int value;
            CAmXmlConfigParser::getEnumerationValue(RHSString, value);
            RHSString = to_string(value);
        }

        result = (INC == operation) ? false : true;
        for (itListOutputs = listLHSOutputs.begin(); itListOutputs != listLHSOutputs.end();
             itListOutputs++)
        {
            if (*itListOutputs == RHSString)
            {
                result = !result;
                break;
            }
        }
    }
    else    // mathematical operator
    {
        result = false;
        if (false == listLHSOutputs.empty())
        {
            if (mMapFunctionReturnValue[condition.leftObject.functionName])
            {
                // function if of type string
                LHSString = listLHSOutputs[0];
                result    = _conditionResult(LHSString, operation, RHSString);
            }
            else if (E_OK == CAmXmlConfigParser::getEnumerationValue(RHSString, RHSInt))
            {
                // function is of type integer and RHS value can be interpreted as numerical
                LHSInt = atoi(listLHSOutputs[0].data());
                result = _conditionResult(LHSInt, operation, RHSInt);
            }
            else
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "FAILED comparing", LHSString, operation, RHSString);
            }
        }
    }

    return result;
}

// return the value of mandatory parameter
void CAmPolicyFunctions::_getValueOfParameter(const gc_FunctionElement_s &function
    , std::string &mandatoryParameter) const
{
    std::string tempMandatoryParameter = function.mandatoryParameter;
    if ((FUNCTION_MACRO_SUPPORTED_REQUESTING != tempMandatoryParameter) || (false == function.isValueMacro))
    {
        mandatoryParameter = tempMandatoryParameter;
    }
}

// return weather all domain are requested or single domain is requested
bool CAmPolicyFunctions::_isSingleDomainRequest(const gc_FunctionElement_s &function,
    std::string &mandatoryParameter) const
{
    bool        result = false;
    bool        isValueMacro;
    std::string tempMandatoryParameter;
    isValueMacro           = function.isValueMacro;
    tempMandatoryParameter = function.mandatoryParameter;
    if (FUNCTION_MACRO_SUPPORTED_REQUESTING == tempMandatoryParameter)
    {
        result = true;
    }
    else if ((FUNCTION_MACRO_SUPPORTED_ALL != tempMandatoryParameter) || (false == isValueMacro))
    {
        mandatoryParameter = tempMandatoryParameter;
        result             = true;
    }

    return result;
}

am_Error_e CAmPolicyFunctions::_getNameList(const std::string &mandatoryParameter,
    const std::string &optionalParameter,
    std::vector<std::string > &listOutputs,
    const gc_triggerParams_s &parameters,
    const bool isSinkRequired)
{
    std::vector<gc_ConnectionInfo_s >           listConnectionInfo;
    std::vector<gc_ConnectionInfo_s >::iterator itListConnectionInfo;
    am_Error_e                                  result = E_UNKNOWN;
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

// get the connection list based on element type,name and optional parameter passed in condition in configuration
am_Error_e CAmPolicyFunctions::_getConnectionList(
    const gc_Element_e elementType, const std::string &elementName,
    const std::string &optionalParameter,
    std::vector<gc_ConnectionInfo_s > &listConnectionInfo,
    const gc_triggerParams_s &parameters) const
{
    std::vector<gc_ConnectionInfo_s >           listLocalConnectionInfo;
    std::vector<gc_ConnectionInfo_s >::iterator itListConnectionInfo;
    am_Error_e                                  result = E_UNKNOWN;
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
        else    // for macro like CS_CONNECTED/CS_DISCONNECTED etc as specified in configuration
        {
            for (itListConnectionInfo = listLocalConnectionInfo.begin();
                 itListConnectionInfo != listLocalConnectionInfo.end();
                 itListConnectionInfo++)
            {
                // convert the macro in value based on schema and store in std::string format
                if (to_string((*itListConnectionInfo).connectionState) == optionalParameter)
                {
                    listConnectionInfo.push_back((*itListConnectionInfo));
                }
            }
        }

        result = E_OK;
    }

    return result;
}

am_Error_e CAmPolicyFunctions::_findUserNotificationType(FUNCTION_PARAMS)
{
    (void)function.category;
    listOutputs.push_back(to_string(parameters.notificatonConfiguration.type));

    return E_OK;
}

am_Error_e CAmPolicyFunctions::_findNotificationType(FUNCTION_PARAMS)
{
    (void)function.category;
    listOutputs.push_back(to_string(parameters.notificatonPayload.type));

    return E_OK;
}

am_Error_e CAmPolicyFunctions::_findUserNotificationValue(FUNCTION_PARAMS)
{
    (void)function.category;
    listOutputs.push_back(to_string(parameters.notificatonPayload.value));

    return E_OK;
}

am_Error_e CAmPolicyFunctions::_findMainNTStatusParam(const gc_FunctionElement_s &function,
    std::vector<std::string > &listOutputs,
    std::string mandatoryParameter, gc_Element_e elementType,
    am_CustomNotificationType_t ntType, const bool isStatusReq)
{
    am_Error_e                                  result = E_UNKNOWN;
    std::vector<am_NotificationConfiguration_s> listNotificationConfigurations;
    _getValueOfParameter(function, mandatoryParameter);
    if (E_OK == mpPolicyReceive->getListMainNotificationConfigurations(elementType, mandatoryParameter, listNotificationConfigurations))
    {
        // override notification type if specified in function element
        if ((FUNCTION_MACRO_SUPPORTED_REQUESTING != function.optionalParameter)
            && (E_OK != CAmXmlConfigParser::getEnumerationValue(function.optionalParameter, reinterpret_cast<int &>(ntType))))
        {
            return E_NOT_POSSIBLE;
        }

        std::vector<am_NotificationConfiguration_s>::iterator itListNotificationConfigurations;
        for (itListNotificationConfigurations = listNotificationConfigurations.begin(); itListNotificationConfigurations != listNotificationConfigurations.end(); itListNotificationConfigurations++)
        {
            if (ntType == itListNotificationConfigurations->type)
            {
                if ( true == isStatusReq)
                {
                    listOutputs.push_back(to_string((*itListNotificationConfigurations).status));
                }
                else
                {
                    listOutputs.push_back(to_string((*itListNotificationConfigurations).parameter));
                }

                result = E_OK;
                break;
            }
        }
    }

    return result;
}

am_Error_e CAmPolicyFunctions::_findMainNTStatus(FUNCTION_PARAMS)
{
    switch (function.category)
    {
    case OT_SINK:
        return _findMainNTStatusParam(function, listOutputs, parameters.sinkName, ET_SINK, parameters.notificatonConfiguration.type, true);

    case OT_SOURCE:
        return _findMainNTStatusParam(function, listOutputs, parameters.sourceName, ET_SOURCE, parameters.notificatonConfiguration.type, true);

    default:
        return E_NOT_POSSIBLE;
    }
}

am_Error_e CAmPolicyFunctions::_findMainNTParam(FUNCTION_PARAMS)
{
    switch (function.category)
    {
    case OT_SINK:
        return _findMainNTStatusParam(function, listOutputs, parameters.sinkName, ET_SINK, parameters.notificatonConfiguration.type, false);

    case OT_SOURCE:
        return _findMainNTStatusParam(function, listOutputs, parameters.sourceName, ET_SOURCE, parameters.notificatonConfiguration.type, false);

    default:
        return E_NOT_POSSIBLE;
    }
}

am_Error_e CAmPolicyFunctions::_findNTStatusParam(const gc_FunctionElement_s &function,
    std::vector<std::string > &listOutputs,
    std::string mandatoryParameter, gc_Element_e elementType,
    am_CustomNotificationType_t ntType, const bool isStatusReq)
{
    am_Error_e                                  result = E_UNKNOWN;
    std::vector<am_NotificationConfiguration_s> listNotificationConfigurations;
    _getValueOfParameter(function, mandatoryParameter);

    // override notification type if specified in function element
    if (!function.optionalParameter.empty())
    {
        // resolve notification type
        if (E_OK != CAmXmlConfigParser::getEnumerationValue(function.optionalParameter, reinterpret_cast<int &>(ntType)))
        {
            return E_NOT_POSSIBLE;
        }
    }

    if (E_OK == mpPolicyReceive->getListNotificationConfigurations(elementType, mandatoryParameter, listNotificationConfigurations))
    {
        std::vector<am_NotificationConfiguration_s>::iterator itListNotificationConfigurations;
        for (itListNotificationConfigurations = listNotificationConfigurations.begin(); itListNotificationConfigurations != listNotificationConfigurations.end(); itListNotificationConfigurations++)
        {
            if (ntType == itListNotificationConfigurations->type)
            {
                if ( true == isStatusReq)
                {
                    listOutputs.push_back(to_string((*itListNotificationConfigurations).status));
                }
                else
                {
                    listOutputs.push_back(to_string((*itListNotificationConfigurations).parameter));
                }

                result = E_OK;
                break;
            }
        }
    }

    return result;
}

am_Error_e CAmPolicyFunctions::_findNTStatus(FUNCTION_PARAMS)
{
    switch (function.category)
    {
    case OT_SINK:
        return _findNTStatusParam(function, listOutputs, parameters.sinkName, ET_SINK, parameters.notificatonConfiguration.type, true);

    case OT_SOURCE:
        return _findNTStatusParam(function, listOutputs, parameters.sourceName, ET_SOURCE, parameters.notificatonConfiguration.type, true);

    case OT_USER:
        listOutputs.push_back(to_string(parameters.notificatonConfiguration.status));
        return E_OK;

    default:
        return E_NOT_POSSIBLE;
    }
}

am_Error_e CAmPolicyFunctions::_findNTParam(FUNCTION_PARAMS)
{
    switch (function.category)
    {
    case OT_SINK:
        return _findNTStatusParam(function, listOutputs, parameters.sinkName, ET_SINK, parameters.notificatonConfiguration.type, false);

    case OT_SOURCE:
        return _findNTStatusParam(function, listOutputs, parameters.sourceName, ET_SOURCE, parameters.notificatonConfiguration.type, false);

    case OT_USER:
        listOutputs.push_back(to_string(parameters.notificatonConfiguration.parameter));
        return E_OK;

    default:
        return E_NOT_POSSIBLE;
    }
}

// find the device i.e. router side volume based on element type and name
am_Error_e CAmPolicyFunctions::_findDeviceVolume(FUNCTION_PARAMS)
{
    switch (function.category)
    {
    case OT_SINK:
        return _findDeviceVolume(ET_SINK, function, parameters.sinkName, listOutputs);

    case OT_SOURCE:
        return _findDeviceVolume(ET_SOURCE, function, parameters.sourceName, listOutputs);

    case OT_CONNECTION:
    {
        std::string connectionName = parameters.sourceName + ":" + parameters.sinkName;
        return _findDeviceVolume(ET_CONNECTION, function, connectionName, listOutputs);
    }

    case OT_CLASS:
    {
        std::string mandatoryParameter = parameters.className;
        _getValueOfParameter(function, mandatoryParameter);

        // get the list of connections belonging to class based on optional parameter
        std::vector<gc_ConnectionInfo_s > listConnectionInfo;
        if (E_OK == _getConnectionList(ET_CLASS, mandatoryParameter, function.optionalParameter,
                listConnectionInfo, parameters))
        {
            for (const auto &connection : listConnectionInfo)
            {
                listOutputs.push_back(to_string(connection.volume));
            }

            return E_OK;
        }

        return E_UNKNOWN;
    }

    default:
        return E_NOT_POSSIBLE;
    }
}

// find the device i.e. router side volume based on element type and name
am_Error_e CAmPolicyFunctions::_findDeviceVolume(const gc_Element_e elementType,
    const gc_FunctionElement_s &function,
    std::string elementName,
    std::vector<std::string > &listOutputs)
{
    am_volume_t deviceVolume;
    am_Error_e  result = E_UNKNOWN;
    _getValueOfParameter(function, elementName);
    // get device volume based on element name
    if (E_OK == mpPolicyReceive->getVolume(elementType, elementName, deviceVolume))
    {
        // store the volume in std::string format
        listOutputs.push_back(to_string(deviceVolume));
        result = E_OK;
    }

    return result;
}

// find the user i.e main volume based on element type and name
am_Error_e CAmPolicyFunctions::_findMainVolume(const gc_Element_e elementType,
    const gc_FunctionElement_s &function,
    std::string elementName,
    std::vector<std::string > &listOutputs)
{
    am_mainVolume_t volume;
    am_Error_e      result = E_UNKNOWN;
    _getValueOfParameter(function, elementName);
    // get main volume based on element name
    if (E_OK == mpPolicyReceive->getMainVolume(elementType, elementName, volume))
    {
        // store the volume in std::string format
        listOutputs.push_back(to_string(volume));
        result = E_OK;
    }

    return result;
}

// find the device i.e. router side property value based on element type and name
am_Error_e CAmPolicyFunctions::_findDevicePropertyValue(FUNCTION_PARAMS)
{
    gc_Element_e elementType;
    std::string  elementName;
    switch (function.category)
    {
    case OT_SINK:
        elementType = ET_SINK;
        elementName = parameters.sinkName;
        break;

    case OT_SOURCE:
        elementType = ET_SOURCE;
        elementName = parameters.sourceName;
        break;

    default:
        return E_NOT_POSSIBLE;
    }

    _getValueOfParameter(function, elementName);
    am_CustomSoundPropertyType_t property;
    if (E_OK != CAmXmlConfigParser::getEnumerationValue(function.optionalParameter, reinterpret_cast<int &>(property)))
    {
        return E_NOT_POSSIBLE;
    }

    // get device property based on element name
    int16_t value;
    if (E_OK == mpPolicyReceive->getSoundProperty(elementType, elementName, property, value))
    {
        // store the value in std::string format
        listOutputs.push_back(to_string(value));
        return E_OK;
    }

    return E_UNKNOWN;
}

// find the user i.e main property value based on element type and name
am_Error_e CAmPolicyFunctions::_findMainSoundPropertyValue(FUNCTION_PARAMS)
{
    gc_Element_e elementType;
    std::string  elementName;
    switch (function.category)
    {
    case OT_SINK:
        elementType = ET_SINK;
        elementName = parameters.sinkName;
        break;

    case OT_SOURCE:
        elementType = ET_SOURCE;
        elementName = parameters.sourceName;
        break;

    case OT_USER:
        listOutputs.push_back(to_string(parameters.mainSoundProperty.value));
        return E_OK;

    default:
        return E_NOT_POSSIBLE;
    }

    _getValueOfParameter(function, elementName);
    am_CustomMainSoundPropertyType_t property;
    if (E_OK != CAmXmlConfigParser::getEnumerationValue(function.optionalParameter, reinterpret_cast<int &>(property)))
    {
        return E_NOT_POSSIBLE;
    }

    // get user property based on element name
    int16_t value;
    if (E_OK == mpPolicyReceive->getMainSoundProperty(elementType, elementName, property, value))
    {
        // store the value in std::string format
        listOutputs.push_back(to_string(value));
        return E_OK;
    }

    return E_UNKNOWN;
}

am_Error_e CAmPolicyFunctions::_findName(FUNCTION_PARAMS)
{
    const std::string &optionalParameter = function.optionalParameter;
    gc_Source_s        sourceInstance;
    gc_Sink_s          sinkInstance;
    switch (function.category)
    {
    case OT_SINK:
        return _findName(function, listOutputs, parameters.sinkName);

    case OT_SOURCE:
        return _findName(function, listOutputs, parameters.sourceName);

    case OT_CLASS:      // = &CAmPolicyFunctions::_findClassName;
        return _findName(function, listOutputs, parameters.className);

    case OT_CONNECTION:     // = &CAmPolicyFunctions::_findConnectionName;
        if (parameters.connectionName.empty())
        {
            return _findName(function, listOutputs, parameters.sourceName + ":" + parameters.sinkName);
        }
        else
        {
            return _findName(function, listOutputs, parameters.connectionName);
        }

    case OT_DOMAIN_OF_SOURCE:     // &CAmPolicyFunctions::_findDomainOfSourceName;
        return _findDomainOrClassOfElementName(sourceInstance, function, listOutputs, parameters.sourceName, false);

    case OT_DOMAIN_OF_SINK:      // = &CAmPolicyFunctions::_findDomainOfSinkName;
    {
        return _findDomainOrClassOfElementName(sinkInstance, function, listOutputs, parameters.sinkName, false);
    }
    case OT_CLASS_OF_SOURCE:      // = &CAmPolicyFunctions::_findClassOfSourceName;
    {
        return _findDomainOrClassOfElementName(sourceInstance, function, listOutputs, parameters.sourceName, true);
    }
    case OT_CLASS_OF_SINK:      // = &CAmPolicyFunctions::_findClassOfSinkName;
    {
        return _findDomainOrClassOfElementName(sinkInstance, function, listOutputs, parameters.sinkName, true);
    }
    case OT_SOURCE_OF_CLASS:      // = &CAmPolicyFunctions::_findSourceOfClassName;
    {
        std::string mandatoryParameter = parameters.className;
        _getValueOfParameter(function, mandatoryParameter);

        // if parameter is ALL or empty means all the source of this class is needed
        if ((FUNCTION_MACRO_SUPPORTED_ALL == optionalParameter) || (true == optionalParameter.empty()))
        {
            std::vector<gc_Source_s > listSources;
            if (E_OK == CAmConfigurationReader::instance().getListSources(listSources))
            {
                for (const auto &source : listSources)
                {
                    if (source.className == mandatoryParameter)
                    {
                        listOutputs.push_back(source.name);
                    }
                }

                return E_OK;
            }
        }
        else        // get the connection list and find the source name involved in connection
        {
            return _getNameList(mandatoryParameter, optionalParameter, listOutputs, parameters, false);
        }

        return E_UNKNOWN;
    }
    case OT_SINK_OF_CLASS:      // = &CAmPolicyFunctions::_findSinkOfClassName;
    {
        std::string mandatoryParameter = parameters.className;
        _getValueOfParameter(function, mandatoryParameter);

        // if parameter is ALL or empty means all the sink of this class is needed
        if ((FUNCTION_MACRO_SUPPORTED_ALL == optionalParameter) || (optionalParameter.empty()))
        {
            std::vector<gc_Sink_s > listSinks;
            if (E_OK == CAmConfigurationReader::instance().getListSinks(listSinks))
            {
                for (const auto &sink : listSinks)
                {
                    if (sink.className == mandatoryParameter)
                    {
                        listOutputs.push_back(sink.name);
                    }
                }

                return E_OK;
            }
        }
        else        // get the connection list and find the sink name involved in connection
        {
            return _getNameList(mandatoryParameter, optionalParameter, listOutputs, parameters, true);
        }

        return E_UNKNOWN;
    }
    case OT_DOMAIN:      // = &CAmPolicyFunctions::_findDomainName;
        return _findName(function, listOutputs, parameters.domainName);
    default:
        return E_NOT_POSSIBLE;
    }
}

am_Error_e CAmPolicyFunctions::_findName(const gc_FunctionElement_s &function,
    std::vector<std::string > &listOutputs, const std::string &name)
{
    am_Error_e  result = E_UNKNOWN;
    bool        isValueMacro;
    std::string mandatoryParameter;
    isValueMacro       = function.isValueMacro;
    mandatoryParameter = function.mandatoryParameter;
    if ((FUNCTION_MACRO_SUPPORTED_REQUESTING == mandatoryParameter) && (true == isValueMacro))
    {
        result = E_OK;
        listOutputs.push_back(name);
    }

    return result;
}

// find all contained elements
am_Error_e CAmPolicyFunctions::_findSubItems(FUNCTION_PARAMS)
{
    am_Error_e success = E_OK;
    switch (function.category)
    {
    case OT_CLASS:
    {
        std::string className = function.mandatoryParameter;
        if (FUNCTION_MACRO_SUPPORTED_REQUESTING == function.mandatoryParameter)
        {
            className = parameters.className;
        }

        if (function.optionalParameter == CATEGORY_CONNECTION)
        {
            std::vector<gc_ConnectionInfo_s > listConnectionInfos;
            success = mpPolicyReceive->getListMainConnections(ET_CLASS, className, listConnectionInfos);

            for (const auto &conn : listConnectionInfos)
            {
                listOutputs.push_back(conn.connectionName);
            }
        }
        else if (function.optionalParameter == CATEGORY_SOURCE)
        {
            success = _getNameList(className, FUNCTION_MACRO_SUPPORTED_ALL
                    , listOutputs, parameters, false);
        }
        else if (function.optionalParameter == CATEGORY_SINK)
        {
            success = _getNameList(className, FUNCTION_MACRO_SUPPORTED_ALL
                    , listOutputs, parameters, true);
        }

        break;
    }

    case OT_DOMAIN:
    {
        const auto &domain = CAmDomainFactory::getElement(function.mandatoryParameter);
        if (domain == nullptr)
        {
            LOG_FN_WARN(__FILENAME__, __func__, "Unable to find domain with name", function.mandatoryParameter);
            break;
        }

        if (function.optionalParameter == CATEGORY_SOURCE)
        {
            std::vector<std::shared_ptr<CAmSourceElement > > listSources;
            CAmSourceFactory::getListElements(listSources);
            for (const auto &src : listSources)
            {
                if (src && (src->getDomainID() == domain->getID()))
                {
                    listOutputs.push_back(src->getName());
                }
            }
        }
        else if (function.optionalParameter == CATEGORY_SINK)
        {
            std::vector<std::shared_ptr<CAmSinkElement > > listSinks;
            CAmSinkFactory::getListElements(listSinks);
            for (const auto &snk : listSinks)
            {
                if (snk && (snk->getDomainID() == domain->getID()))
                {
                    listOutputs.push_back(snk->getName());
                }
            }
        }
        else
        {
            success = E_NOT_POSSIBLE;
        }

        break;
    }

    default:
        success = E_NOT_POSSIBLE;
        break;
    }

    if (success != E_OK)
    {
        LOG_FN_WARN(__FILENAME__, __func__, "Failed evaluating"
            , function.category, function.mandatoryParameter, function.optionalParameter);
    }

    return success;
}

// find the name of domain from source name
template <typename Telement>
am_Error_e CAmPolicyFunctions::_findDomainOrClassOfElementName(
    Telement &elementInstance, const gc_FunctionElement_s &function,
    std::vector<std::string > &listOutputs, std::string mandatoryParameter,
    const bool isClassRequired)
{
    am_Error_e result = E_UNKNOWN;
    _getValueOfParameter(function, mandatoryParameter);
    if (E_OK == CAmConfigurationReader::instance().getElementByName(mandatoryParameter, elementInstance))
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

am_Error_e CAmPolicyFunctions::_findPeek(FUNCTION_PARAMS)
{
    switch (function.category)
    {
    case OT_SOURCE_OF_CLASS:
    case OT_SINK_OF_CLASS:
        break;

    default:
        return E_NOT_POSSIBLE;
    }

    std::string className = parameters.className;
    _getValueOfParameter(function, className);

    gc_Order_e order = O_HIGH_PRIORITY;
    if (!function.optionalParameter.empty())
    {
        if (E_OK != CAmXmlConfigParser::getEnumerationValue(function.optionalParameter, reinterpret_cast<int &>(order)))
        {
            return E_NOT_POSSIBLE;
        }
    }

    unsigned index = 0;
    if (!function.optionalParameter2.empty())
    {
        index = atoi(function.optionalParameter2.data());
    }

    std::vector<gc_ConnectionInfo_s > listConnectionInfo;
    if (E_OK == mpPolicyReceive->getListMainConnections(className, listConnectionInfo, order))
    {
        if (listConnectionInfo.size() > index)
        {
            if (function.category == OT_SINK_OF_CLASS)
            {
                listOutputs.push_back(listConnectionInfo[index].sinkName);
            }
            else
            {
                listOutputs.push_back(listConnectionInfo[index].sourceName);
            }

            return E_OK;
        }
    }

    return E_UNKNOWN;
}

am_Error_e CAmPolicyFunctions::_findScale(FUNCTION_PARAMS)
{
    (void)parameters;   // no dependency on trigger

    std::map<float, float > mapValues;
    CAmConfigurationReader::instance().populateScalingMap(mapValues, function.optionalParameter);

    am_Error_e retVal = E_UNKNOWN;
    float      lhs    = strtof(function.mandatoryParameter.c_str(), 0);
    float      rhs    = lhs;
    if (!mapValues.empty())
    {
        retVal = E_OK;
        auto upper = mapValues.upper_bound(lhs);
        if (upper == mapValues.end())  // at or beyond highest end
        {
            rhs = (--upper)->second;
        }
        else if (upper == mapValues.begin())  // at or beyond lowest end
        {
            rhs = upper->second;
        }
        else    // within range - use linear interpolation
        {
            auto lower = std::prev(upper);
            rhs = (upper->second - lower->second) / (upper->first - lower->first)
                * (lhs - lower->first) + lower->second;
        }
    }

    int nearestInt = (rhs >= 0) ? (int)(rhs + 0.5) : (int)(rhs - 0.5);
    listOutputs.push_back(to_string(nearestInt));

    return retVal;
}

// count the number of words in given string, separated by
// any non-zero number of space, tab, newline, comma or semicolon
am_Error_e CAmPolicyFunctions::_findCount(FUNCTION_PARAMS)
{
    (void)parameters;   // no dependency on trigger

    const char *str       = function.mandatoryParameter.c_str();
    unsigned    wordCount = 0;
    bool        inWord    = false;
    for (size_t pos = 0; pos < strlen(str); ++pos)
    {
        const char delimiters[] = { ' ', '\t', '\n', ',', ';' };
        if (strchr(delimiters, str[pos]))
        {
            inWord = false;
        }
        else if (!inWord)
        {
            wordCount++;
            inWord = true;
        }
    }

    listOutputs.push_back(to_string(wordCount));

    return E_OK;
}

// get the priority of sink by sink name
template <typename Telement>
am_Error_e CAmPolicyFunctions::_findPriority(const gc_FunctionElement_s &function,
    std::string mandatoryParameter, std::vector<std::string > &listOutputs)
{
    Telement elementInstance;
    _getValueOfParameter(function, mandatoryParameter);
    if (E_OK == CAmConfigurationReader::instance().getElementByName(mandatoryParameter, elementInstance))
    {
        listOutputs.push_back(to_string(elementInstance.priority));
        return E_OK;
    }

    return E_UNKNOWN;
}

am_Error_e CAmPolicyFunctions::_findPriority(FUNCTION_PARAMS)
{
    switch (function.category)
    {
    case OT_SINK:
    {
        return _findPriority<gc_Sink_s>(function, parameters.sinkName, listOutputs);
    }

    case OT_SOURCE:
    {
        return _findPriority<gc_Source_s>(function, parameters.sourceName, listOutputs);
    }

    case OT_CLASS:
    {
        return _findPriority<gc_Class_s>(function, parameters.className, listOutputs);
    }

    case OT_CONNECTION:
    {
        std::vector<gc_ConnectionInfo_s > listConnectionInfo;
        std::string                       mandatoryParameter = parameters.sourceName + ":" + parameters.sinkName;
        _getValueOfParameter(function, mandatoryParameter);

        // get list of connection based on connection name
        if (E_OK != mpPolicyReceive->getListMainConnections(ET_CONNECTION, mandatoryParameter, listConnectionInfo))
        {
            return E_UNKNOWN;
        }

        for (const auto &connection : listConnectionInfo)
        {
            listOutputs.push_back(to_string(connection.priority));
        }

        return E_OK;
    }

    case OT_CONNECTION_OF_CLASS:
    {
        std::string mandatoryParameter = parameters.className;
        _getValueOfParameter(function, mandatoryParameter);

        std::vector<gc_ConnectionInfo_s > listConnectionInfo;
        if (E_OK == _getConnectionList(ET_CLASS, mandatoryParameter, function.optionalParameter,
                listConnectionInfo, parameters))
        {
            for (const auto &connection : listConnectionInfo)
            {
                listOutputs.push_back(to_string(connection.priority));
            }

            return E_OK;
        }

        return E_UNKNOWN;
    }

    default:
        return E_NOT_POSSIBLE;
    }
}

// get the list of connection state of connections using class name
am_Error_e CAmPolicyFunctions::_findConnectionState(FUNCTION_PARAMS)
{
    switch (function.category)
    {
    case OT_CONNECTION_OF_CLASS:
    {
        std::string mandatoryParameter = parameters.className;
        _getValueOfParameter(function, mandatoryParameter);

        // get list of connection based on class name
        std::vector<gc_ConnectionInfo_s > listConnectionInfo;
        if (E_OK == _getConnectionList(ET_CLASS, mandatoryParameter, function.optionalParameter,
                listConnectionInfo, parameters))
        {
            for (const auto &connection : listConnectionInfo)
            {
                listOutputs.push_back(to_string(connection.connectionState));
            }

            return E_OK;
        }

        return E_UNKNOWN;
    }

    case OT_CONNECTION_OF_SOURCE:
        return _findConnectionState(function, listOutputs, parameters.sourceName, ET_SOURCE);

    case OT_CONNECTION_OF_SINK:
        return _findConnectionState(function, listOutputs, parameters.sinkName, ET_SINK);

    case OT_CONNECTION:
        return _findConnectionState(function, listOutputs, "", ET_CONNECTION);
// case OT_CONNECTION:
// {
// if (parameters.sourceName.empty() || parameters.sinkName.empty())
// {
// return _findConnectionState(function, listOutputs, parameters.connectionName, ET_CONNECTION);
// }
// else
// {
// std::string connectionName = parameters.sourceName + ":" + parameters.sinkName;
// return _findConnectionState(function, listOutputs, connectionName, ET_CONNECTION);
// }
// }

    case OT_USER:
        listOutputs.push_back(to_string(parameters.connectionState));
        return E_OK;

    default:
        return E_NOT_POSSIBLE;
    }
}

am_Error_e CAmPolicyFunctions::_findConnectionState(
    const gc_FunctionElement_s &function,
    std::vector<std::string > &listOutputs, std::string mandatoryParameter,
    const gc_Element_e elementType)
{
    _getValueOfParameter(function, mandatoryParameter);

    // get list of connections based on element name
    std::vector<gc_ConnectionInfo_s > listConnectionInfo;
    if (E_OK != mpPolicyReceive->getListMainConnections(elementType, mandatoryParameter, listConnectionInfo))
    {
        return E_DATABASE_ERROR;
    }

    // define selection function
    auto select = [&] (const gc_ConnectionInfo_s &current) -> bool {
            switch (elementType)
            {
            case ET_SOURCE:
                return (mandatoryParameter == current.sourceName);
            case ET_SINK:
                return (mandatoryParameter == current.sinkName);
            case ET_CONNECTION:
                return ((mandatoryParameter == current.connectionName)
                        || (mandatoryParameter == current.sourceName + ":" + current.sinkName));
            default:
                return false;
            }
        };

    // populate outgoing list from states of gathered main connections
    for (const auto &current : listConnectionInfo)
    {
        if (select(current))
        {
            listOutputs.push_back(to_string(current.connectionState));

            LOG_FN_INFO(__FILENAME__, __func__, "adding", current.connectionState, "from", current.connectionName);
        }
    }

    return E_OK;
}

// find the user volume
am_Error_e CAmPolicyFunctions::_findMainVolume(FUNCTION_PARAMS)
{
    switch (function.category)
    {
    case OT_SINK:
        return _findMainVolume(ET_SINK, function, parameters.sinkName, listOutputs);

    case OT_SOURCE:
        return _findMainVolume(ET_SOURCE, function, parameters.sourceName, listOutputs);

    case OT_USER:
        listOutputs.push_back(to_string(parameters.mainVolume));
        return E_OK;

    default:
        return E_NOT_POSSIBLE;
    }
}

am_Error_e CAmPolicyFunctions::_findIsMainVolumeStep(FUNCTION_PARAMS)
{
    (void)function.category;
    listOutputs.push_back(to_string(parameters.isVolumeStep));

    return E_OK;
}

am_Error_e CAmPolicyFunctions::_findLimit(FUNCTION_PARAMS)
{
    am_volume_t limit = AM_VOLUME_NO_LIMIT;
    switch (function.category)
    {
    case OT_SOURCE:
    {
        const auto &src = CAmSourceFactory::getElement(function.mandatoryParameter);
        if (src != nullptr)
        {
            listOutputs.push_back(to_string(src->getOffsetVolume()));
            return E_OK;
        }

        break;
    }

    case OT_SINK:
    {
        const auto &snk = CAmSinkFactory::getElement(function.mandatoryParameter);
        if (snk != nullptr)
        {
            listOutputs.push_back(to_string(snk->getOffsetVolume()));
            return E_OK;
        }

        break;
    }

    case OT_CONNECTION:
    {
        const auto &conn = CAmMainConnectionFactory::getElement(function.mandatoryParameter);
        if (conn != nullptr)
        {
            listOutputs.push_back(to_string(conn->getOffsetVolume()));
            return E_OK;
        }

        break;
    }
    }

    return E_NOT_POSSIBLE;
}

// find the trigger error value
am_Error_e CAmPolicyFunctions::_findUserErrorValue(FUNCTION_PARAMS)
{
    (void)function.category;
    listOutputs.push_back(to_string(parameters.status));

    return E_OK;
}

// find the trigger user property type
am_Error_e CAmPolicyFunctions::_findMainSoundPropertyType(FUNCTION_PARAMS)
{
    listOutputs.push_back(to_string(parameters.mainSoundProperty.type));

    return E_OK;
}

// find the trigger user property type
am_Error_e CAmPolicyFunctions::_findSystemPropertyType(FUNCTION_PARAMS)
{
    (void)function.category;
    listOutputs.push_back(to_string(parameters.systemProperty.type));

    return E_OK;
}

// find the system property value
am_Error_e CAmPolicyFunctions::_findSystemPropertyValue(FUNCTION_PARAMS)
{
    switch (function.category)
    {
    case OT_SYSTEM:
    {
        am_Error_e                    result = E_UNKNOWN;
        am_CustomSystemPropertyType_t property;
        if (function.mandatoryParameter.empty())
        {
            property = parameters.systemProperty.type;
        }
        else if (E_OK != CAmXmlConfigParser::getEnumerationValue(function.mandatoryParameter, reinterpret_cast<int &>(property)))
        {
            return E_NOT_POSSIBLE;
        }

        // get system property
        int16_t value;
        if (E_OK == mpPolicyReceive->getSystemProperty(property, value))
        {
            listOutputs.push_back(to_string(value));
            result = E_OK;
        }

        return result;
    }

    case OT_USER:
        listOutputs.push_back(to_string(parameters.systemProperty.value));
        return E_OK;

    default:
        return E_NOT_POSSIBLE;
    }
}

am_Error_e CAmPolicyFunctions::_findMuteState(FUNCTION_PARAMS)
{
    gc_Element_e elementType;
    std::string  elementName;
    switch (function.category)
    {
    case OT_SINK:
        elementType = ET_SINK;
        elementName = parameters.sinkName;
        break;

    case OT_CLASS:
        elementType = ET_CLASS;
        elementName = parameters.className;
        break;

    case OT_CONNECTION:
        elementType = ET_CONNECTION;
        elementName = parameters.sourceName + ":" + parameters.sinkName;
        break;

    case OT_USER:
        listOutputs.push_back(to_string(parameters.muteState));
        return E_OK;

    default:
        return E_NOT_POSSIBLE;
    }

    am_MuteState_e muteState;
    _getValueOfParameter(function, elementName);

    if (E_OK == mpPolicyReceive->getMuteState(elementType, elementName, muteState))
    {
        listOutputs.push_back(to_string(muteState));
        return E_OK;
    }

    return E_UNKNOWN;
}

am_Error_e CAmPolicyFunctions::_findAvailability(FUNCTION_PARAMS)
{
    // special case CONNECTION which has no corresponding availability reason
    if (function.category == OT_CONNECTION)
    {
        const auto pConnection = CAmMainConnectionFactory::getElement(function.mandatoryParameter);
        if (pConnection)
        {
            listOutputs.push_back(to_string(pConnection->getRouteAvailability()));
            return E_OK;
        }

        return E_NON_EXISTENT;
    }

    // SOURCE and SINK
    am_Availability_s availability;
    am_Error_e        retVal = _findAvailability(function, parameters, availability);
    if (retVal == E_OK)
    {
        listOutputs.push_back(to_string(availability.availability));
    }

    return retVal;
}

am_Error_e CAmPolicyFunctions::_findAvailabilityReason(FUNCTION_PARAMS)
{
    am_Availability_s availability;
    am_Error_e        retVal = _findAvailability(function, parameters, availability);
    if (retVal == E_OK)
    {
        listOutputs.push_back(to_string(availability.availabilityReason));
    }

    return retVal;
}

am_Error_e CAmPolicyFunctions::_findAvailability(const gc_FunctionElement_s &function
    , const gc_triggerParams_s &parameters, am_Availability_s &availability)
{
    gc_Element_e elementType;
    std::string  elementName;
    switch (function.category)
    {
    case OT_SINK:
        elementType = ET_SINK;
        elementName = parameters.sinkName;
        break;

    case OT_SOURCE:
        elementType = ET_SOURCE;
        elementName = parameters.sourceName;
        break;

    case OT_USER:
        availability = parameters.availability;
        return E_OK;

    default:
        return E_NOT_POSSIBLE;
    }

    _getValueOfParameter(function, elementName);

    return mpPolicyReceive->getAvailability(elementType, elementName, availability);
}

// Reserved for future use
am_Error_e CAmPolicyFunctions::_findConnectionFormat(FUNCTION_PARAMS)
{
    (void)parameters;

    switch (function.category)
    {
    case OT_CONNECTION:
    case OT_CLASS:
        listOutputs.push_back(to_string(CF_GENIVI_STEREO));
        return E_OK;

    default:
        return E_NOT_POSSIBLE;
    }
}

// find the interrupt state
am_Error_e CAmPolicyFunctions::_findInterruptState(FUNCTION_PARAMS)
{
    std::string  elementName;
    gc_Element_e elementType;
    switch (function.category)
    {
    case OT_SOURCE:
        elementName = parameters.sourceName;
        elementType = ET_SOURCE;
        break;

    case OT_CONNECTION:
        elementType = ET_CONNECTION;
        break;

    case OT_USER:
        listOutputs.push_back(to_string(parameters.interruptState));
        return E_OK;

    default:
        return E_NOT_POSSIBLE;
    }

    am_InterruptState_e interruptState;
    _getValueOfParameter(function, elementName);
    if (E_OK == mpPolicyReceive->getInterruptState(elementType, elementName, interruptState))
    {
        listOutputs.push_back(to_string(interruptState));
        return E_OK;
    }

    return E_UNKNOWN;
}

am_Error_e CAmPolicyFunctions::_findIsRegistered(FUNCTION_PARAMS)
{
    std::string  elementName;
    gc_Element_e elementType;
    switch (function.category)
    {
    case OT_SINK:
        elementType = ET_SINK;
        elementName = parameters.sinkName;
        break;

    case OT_SOURCE:
        elementType = ET_SOURCE;
        elementName = parameters.sourceName;
        break;

    case OT_DOMAIN:
    {
        bool                                result      = false;
        am_Error_e                          returnValue = E_UNKNOWN;
        std::vector<gc_Domain_s >           listDomains;
        std::vector<gc_Domain_s >::iterator itListDomains;

        std::string mandatoryParameter = parameters.domainName;
        if (true == _isSingleDomainRequest(function, mandatoryParameter))
        {
            gc_Domain_s domain;
            if (E_OK == CAmConfigurationReader::instance().getElementByName(mandatoryParameter, domain))
            {
                listDomains.push_back(domain);
            }
        }
        else        // check for all the domains
        {
            CAmConfigurationReader::instance().getListDomains(listDomains);
        }

        for (const auto &domain : listDomains)
        {
            returnValue = E_OK;
            // get registration complete status
            result = mpPolicyReceive->isRegistered(ET_DOMAIN, domain.name);
            if (false == result)
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "domain not registered:", domain.name);
                break;
            }
        }

        listOutputs.push_back(to_string(result));
        return returnValue;
    }

    default:
        return E_NOT_POSSIBLE;
    }

    // get registration status
    _getValueOfParameter(function, elementName);
    bool result = mpPolicyReceive->isRegistered(elementType, elementName);
    listOutputs.push_back(to_string(result));

    return E_OK;
}

// check domain registration is completed or not
am_Error_e CAmPolicyFunctions::_findIsDomainRegistrationComplete(FUNCTION_PARAMS)
{
    if (function.category != OT_DOMAIN)
    {
        return E_NOT_POSSIBLE;
    }

    bool                                result             = false;
    std::string                         mandatoryParameter = parameters.domainName;
    am_Error_e                          returnValue        = E_UNKNOWN;
    gc_Domain_s                         domain;
    std::vector<gc_Domain_s >           listDomains;
    std::vector<gc_Domain_s >::iterator itListDomains;

    if (true == _isSingleDomainRequest(function, mandatoryParameter))
    {
        if (E_OK == CAmConfigurationReader::instance().getElementByName(mandatoryParameter, domain))
        {
            listDomains.push_back(domain);
        }
    }
    else    // check for all the domains
    {
        CAmConfigurationReader::instance().getListDomains(listDomains);
    }

    for (itListDomains = listDomains.begin(); itListDomains != listDomains.end(); itListDomains++)
    {
        returnValue = E_OK;
        // get registration complete status
        result = mpPolicyReceive->isDomainRegistrationComplete((*itListDomains).name);
        if (false == result)
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "domain registration not complete:", (*itListDomains).name);
            break;
        }
    }

    listOutputs.push_back(to_string(result));
    return returnValue;
}

// get the state
am_Error_e CAmPolicyFunctions::_findState(FUNCTION_PARAMS)
{
    switch (function.category)
    {
    case OT_SOURCE:
    {
        std::string mandatoryParameter = parameters.sourceName;
        _getValueOfParameter(function, mandatoryParameter);

        std::shared_ptr<CAmSourceElement > pSource = CAmSourceFactory::getElement(mandatoryParameter);
        if (pSource != nullptr)
        {
            am_SourceState_e sourceState = pSource->getState();
            listOutputs.push_back(to_string(sourceState));
            return E_OK;
        }

        return E_UNKNOWN;
    }

    case OT_DOMAIN:
    {
        am_Error_e                          result = E_OK;
        std::vector<gc_Domain_s >           listDomains;
        std::vector<gc_Domain_s >::iterator itListDomains;

        std::string mandatoryParameter = parameters.domainName;
        if (true == _isSingleDomainRequest(function, mandatoryParameter))
        {
            gc_Domain_s domain;
            if (E_OK == CAmConfigurationReader::instance().getElementByName(mandatoryParameter, domain))
            {
                listDomains.push_back(domain);
            }
        }
        else        // get state for all the domains
        {
            CAmConfigurationReader::instance().getListDomains(listDomains);
        }

        for (const auto &domain : listDomains)
        {
            // get domain state
            int                                domainState;
            std::shared_ptr<CAmDomainElement > pDomain = CAmDomainFactory::getElement(domain.name);
            if (nullptr == pDomain)
            {
                result      = E_UNKNOWN;
                domainState = DS_UNKNOWN;
                break;
            }
            else
            {
                result = pDomain->getState(domainState);
                break;
            }

            listOutputs.push_back(to_string(static_cast<am_DomainState_e>(domainState)));
        }

        return result;
    }

    default:
        return E_NOT_POSSIBLE;
    }
}

} /* namespace gc */
} /* namespace am */
