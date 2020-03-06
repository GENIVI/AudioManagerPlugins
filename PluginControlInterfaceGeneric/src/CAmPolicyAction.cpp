/******************************************************************************
 * @file: CAmPolicyAction.cpp
 *
 * This file contains the definition of policy action class
 * (member functions and data members)
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

#include <string.h>
#include "IAmAction.h"
#include "IAmPolicySend.h"
#include "CAmPolicyAction.h"
#include "CAmSourceSinkActionSetSoundProperty.h"
#include "CAmSourceSinkActionSetSoundProperties.h"
#include "CAmSourceSinkActionSetNotificationConfiguration.h"
#include "CAmSystemActionSetProperties.h"
#include "CAmClassElement.h"
#include "CAmLogger.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmSystemElement.h"
#include "CAmActionConnect.h"
#include "CAmActionDisconnect.h"
#include "CAmActionInterrupt.h"
#include "CAmActionSuspend.h"
#include "CAmActionSetVolume.h"
#include "CAmActionMute.h"
#include "CAmActionLimit.h"
#include "CAmSystemActionRegister.h"
#include "CAmSystemActionSetProperty.h"
#include "CAmCommonUtility.h"


namespace am {
namespace gc {

#define isParameterSet(mapParams, paramName) (1 == mapParams.count(paramName))

#define checkAndSetStringParam(mapParams, paramName, pAction)              \
    {                                                                      \
        if (isParameterSet(mapParams, paramName))                          \
        {                                                                  \
            CAmActionParam<std::string> actionParam(mapParams[paramName]); \
            pAction->setParam(paramName, &actionParam);                    \
        }                                                                  \
    }

#define checkAndSetStringListParam(mapParams, paramName, pAction)          \
    {                                                                      \
        if (isParameterSet(mapParams, paramName))                          \
        {                                                                  \
            std::vector < std::string > listNames;                         \
            _getListNames(mapParams[paramName], listNames);                \
            CAmActionParam < std::vector<std::string > >                   \
                 listNamesParameter(listNames);                            \
            pAction->setParam(paramName, &listNamesParameter);             \
        }                                                                  \
    }

#define checkAndSetNumericParam(mapParams, paramName, paramType, pAction)                           \
    {                                                                                               \
        if (isParameterSet(mapParams, paramName))                                                   \
        {                                                                                           \
            CAmActionParam < paramType > actionParam((paramType)atoi(mapParams[paramName].data())); \
            pAction->setParam(paramName, &actionParam);                                             \
        }                                                                                           \
    }

CAmPolicyAction::CAmPolicyAction(const std::vector<gc_Action_s > &listPolicyActions,
    IAmPolicySend *pPolicySend, IAmControlReceive *pControlReceive)
    : CAmActionContainer(std::string("CAmPolicyAction"))
    , mListActions(listPolicyActions)
    , mpPolicySend(pPolicySend)
    , mpControlReceive(pControlReceive)
{
    mMapConnectionStates["ALL"] =
    { CS_UNKNOWN, CS_CONNECTING, CS_CONNECTED, CS_DISCONNECTING, CS_DISCONNECTED, CS_SUSPENDED };
    mMapConnectionStates["CS_CONNECTED"] =
    { CS_CONNECTED };
    mMapConnectionStates[""] =
    { CS_CONNECTED };
    mMapConnectionStates["CS_DISCONNECTED"] =
    { CS_DISCONNECTED };
    mMapConnectionStates["CS_SUSPENDED"] =
    { CS_SUSPENDED };
    mMapConnectionStates["OTHERS"] =
    { CS_UNKNOWN, CS_CONNECTING, CS_DISCONNECTING,
      CS_DISCONNECTED, CS_SUSPENDED };

    size_t size = mListActions.size(), count = 0;
    if (size)
    {
        for (const gc_Action_s &it : mListActions)
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, it.actionType, " (", ++count, "/", size, ")");
        }
    }
    else
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, " (empty)");
    }
}

CAmPolicyAction::~CAmPolicyAction()
{
}

int CAmPolicyAction::_execute(void)
{
    std::vector<gc_Action_s >::iterator        itListPolicyActions;
    std::vector<IAmActionCommand * >           listFrameworkActions;
    std::vector<IAmActionCommand * >::iterator itListFrameworkActions;
    // for each policy action get the list of framework actions and appens
    // gc_Action_s = policy action
    // IAmActionCommand = framework action
    for (itListPolicyActions = mListActions.begin(); itListPolicyActions != mListActions.end();
         ++itListPolicyActions)
    {
        listFrameworkActions.clear();
        _createActions(*itListPolicyActions, listFrameworkActions);
        for (itListFrameworkActions = listFrameworkActions.begin();
             itListFrameworkActions != listFrameworkActions.end();
             itListFrameworkActions++)
        {
            append(*itListFrameworkActions);
        }
    }

    return E_OK;
}

int CAmPolicyAction::_update(const int Result)
{
    // TODO : unclear what to do here ??
    return E_OK;
}

// helper functions to create action and populate action parameters
std::string CAmPolicyAction::_getParam(std::map<std::string, std::string > &map,
    const std::string &paramName)
{
    std::string parameter;
    if (map.find(paramName) != map.end())
    {
        parameter = map[paramName];
    }

    return parameter;
}

am_Error_e CAmPolicyAction::_getListNames(std::string name, std::vector<std::string > &listNames)
{
    listNames.clear();
    std::string       temp;
    std::stringstream ss(name);
    while (ss >> temp)
    {
        listNames.push_back(temp);
    }

    return E_OK;
}

void CAmPolicyAction::_setActionParameters(std::map<std::string, std::string > &mapParams,
    IAmActionCommand *pAction)
{
    if (isParameterSet(mapParams, ACTION_PARAM_CLASS_NAME))
    {
        std::string className = mapParams[ACTION_PARAM_CLASS_NAME];
        if (className != "ALL")  // ALL is unrestricted default
        {
            CAmActionParam < std::string > classNameParam(className);
            pAction->setParam(ACTION_PARAM_CLASS_NAME, &classNameParam);
        }
    }

    if (isParameterSet(mapParams, ACTION_PARAM_SOURCE_NAME))
    {
        std::string sourceName = mapParams[ACTION_PARAM_SOURCE_NAME];
        /*
         * If no whitespace is found then its a single name
         */
        if (sourceName.find(" ") == std::string::npos)
        {
            CAmActionParam < std::string > sourceNameParam(sourceName);
            pAction->setParam(ACTION_PARAM_SOURCE_NAME, &sourceNameParam);
        }
        else
        {
            std::vector < gc_Source_s > listSources;
            std::vector < std::string > listNames;
            /*
             * Find the list of sources based on the whitespace separated names
             */
            _getListNames(sourceName, listNames);
            mpPolicySend->getListElements(listNames, listSources);
            CAmActionParam < std::vector<gc_Source_s > > listSourcesParameter(listSources);
            pAction->setParam(ACTION_PARAM_SOURCE_INFO, &listSourcesParameter);
        }
    }

    if (isParameterSet(mapParams, ACTION_PARAM_SINK_NAME))
    {
        std::string sinkName = mapParams[ACTION_PARAM_SINK_NAME];
        /*
         * If no whitespace is found then its a single name
         */
        if (sinkName.find(" ") == std::string::npos)
        {
            CAmActionParam < std::string > sinkNameParam(sinkName);
            pAction->setParam(ACTION_PARAM_SINK_NAME, &sinkNameParam);
        }
        else
        {
            std::vector < gc_Sink_s >   listSinks;
            std::vector < std::string > listNames;
            /*
             * Find the list of sources based on the whitespace separated names
             */
            _getListNames(sinkName, listNames);
            mpPolicySend->getListElements(listNames, listSinks);
            CAmActionParam < std::vector<gc_Sink_s > > listSinksParameter(listSinks);
            pAction->setParam(ACTION_PARAM_SINK_INFO, &listSinksParameter);
        }
    }

    if (isParameterSet(mapParams, ACTION_PARAM_GATEWAY_NAME))
    {
        std::string gatewayName = mapParams[ACTION_PARAM_GATEWAY_NAME];
        /*
         * If no whitespace is found then its a single name
         */
        if (gatewayName.find(" ") == std::string::npos)
        {
            CAmActionParam < std::string > sinkNameParam(gatewayName);
            pAction->setParam(ACTION_PARAM_GATEWAY_NAME, &sinkNameParam);
        }
        else
        {
            std::vector < gc_Gateway_s > listGateways;
            std::vector < std::string >  listNames;
            /*
             * Find the list of sources based on the whitespace separated names
             */
            _getListNames(gatewayName, listNames);
            mpPolicySend->getListElements(listNames, listGateways);
            CAmActionParam < std::vector<gc_Gateway_s > > listGatewaysParameter(listGateways);
            pAction->setParam(ACTION_PARAM_GATEWAY_INFO, &listGatewaysParameter);
        }
    }

    checkAndSetStringListParam(mapParams, ACTION_PARAM_EXCEPT_CLASS_NAME, pAction);
    checkAndSetStringListParam(mapParams, ACTION_PARAM_EXCEPT_SOURCE_NAME, pAction);
    checkAndSetStringListParam(mapParams, ACTION_PARAM_EXCEPT_SINK_NAME, pAction);

    if (isParameterSet(mapParams, ACTION_PARAM_CONNECTION_STATE))
    {
        /*
         * convert the connection state filter from string to list connection states
         * here
         */
        std::string filter(mapParams[ACTION_PARAM_CONNECTION_STATE]);

        if ((true == isParameterSet(mapParams, ACTION_PARAM_SOURCE_NAME)) &&
            (true == isParameterSet(mapParams, ACTION_PARAM_SINK_NAME)))
        {
            filter = "ALL";
        }

        CAmActionParam < std::vector<am_ConnectionState_e > > listConnectionStatesParam(
            mMapConnectionStates[filter]);
        pAction->setParam(ACTION_PARAM_CONNECTION_STATE, &listConnectionStatesParam);
    }
    else
    {
        if ((true == isParameterSet(mapParams, ACTION_PARAM_SINK_NAME)) &&
            (true == isParameterSet(mapParams, ACTION_PARAM_SOURCE_NAME)))
        {
            CAmActionParam < std::vector<am_ConnectionState_e > > listConnectionStatesParam(
                mMapConnectionStates["ALL"]);
            pAction->setParam(ACTION_PARAM_CONNECTION_STATE, &listConnectionStatesParam);
        }
    }

    if (isParameterSet(mapParams, ACTION_PARAM_TIMEOUT))
    {
        pAction->setTimeout(atoi(mapParams[ACTION_PARAM_TIMEOUT].data()));
    }

    if (isParameterSet(mapParams, ACTION_PARAM_MUTE_STATE))
    {
        am_MuteState_e muteState;
        muteState = (am_MuteState_e)atoi(mapParams[ACTION_PARAM_MUTE_STATE].c_str());

        CAmActionParam < am_MuteState_e >  muteStateParam;
        muteStateParam.setParam(muteState);
        pAction->setParam(ACTION_PARAM_MUTE_STATE, &muteStateParam);

        CAmActionParam < uint32_t >        patternParam;
        patternParam.setParam(0xFFFF0000);
        pAction->setParam(ACTION_PARAM_PATTERN, &patternParam);
    }

    if (isParameterSet(mapParams, ACTION_PARAM_LIMIT_STATE))
    {
        CAmActionParam < uint32_t >        patternParam;
        CAmActionParam < gc_LimitState_e > volumeOperationParam(
            (gc_LimitState_e)atoi(mapParams[ACTION_PARAM_LIMIT_STATE].c_str()));
        patternParam.setParam(0xFFFF);
        pAction->setParam(ACTION_PARAM_LIMIT_STATE, &volumeOperationParam);
        pAction->setParam(ACTION_PARAM_PATTERN, &patternParam);
    }

    if (isParameterSet(mapParams, ACTION_PARAM_PATTERN))
    {
        CAmActionParam < uint32_t > pattern(
            strtol(mapParams[ACTION_PARAM_PATTERN].data(), NULL, 16));
        pAction->setParam(ACTION_PARAM_PATTERN, &pattern);
    }

    if (isParameterSet(mapParams, ACTION_PARAM_LIST_PROPERTY))
    {
        CAmActionParam<std::vector<am_MainSoundProperty_s > > listparam;
        std::string                                           list = mapParams[ACTION_PARAM_LIST_PROPERTY];
        std::vector<string >                                  listProperty;
        std::string                                           delimiter = "(";

        if (E_OK != CAmCommonUtility::parseString(delimiter, list, listProperty))
        {
            LOG_FN_INFO(__FILENAME__, __func__, " wrong sound property data format");
            return;
        }

        std::vector<am_MainSoundProperty_s > listMainSoundProperty;
        for (auto &itlistProperty : listProperty)
        {
            std::vector<string > listData;
            delimiter = ":";

            if (E_OK != CAmCommonUtility::parseString(delimiter, itlistProperty, listData))
            {
                LOG_FN_INFO(__FILENAME__, __func__, " wrong sound property data format");
                return;
            }

            for (auto itlistData = listData.begin(); itlistData != listData.end(); itlistData++)
            {
                string propType = *itlistData;
                int    type     = atoi(propType.c_str());
                itlistData++;
                if (itlistData == listData.end())
                {
                    LOG_FN_INFO(__FILENAME__, __func__, "wrong sound property data format");
                    continue;
                }

                string                 propVal = *itlistData;
                int                    val     = atoi(propVal.c_str());
                am_MainSoundProperty_s property;
                property.type  = (am_CustomMainSoundPropertyType_t)type;
                property.value = val;
                listMainSoundProperty.push_back(property);
            }
        }

        listparam.setParam(listMainSoundProperty);
        pAction->setParam(ACTION_PARAM_LIST_PROPERTY, &listparam);
    }

    if (isParameterSet(mapParams, ACTION_PARAM_LIST_SYSTEM_PROPERTIES))
    {
        CAmActionParam<std::vector<am_SystemProperty_s > > listparam;
        std::string                                           list = mapParams[ACTION_PARAM_LIST_SYSTEM_PROPERTIES];
        std::vector<string >                                  listProperty;
        std::string                                           delimiter = "(";

        if (E_OK != CAmCommonUtility::parseString(delimiter, list, listProperty))
        {
            LOG_FN_INFO(__FILENAME__, __func__, " wrong system property data format");
            return;
        }

        std::vector<am_SystemProperty_s > listSystemProperty;
        for (auto &itlistProperty : listProperty)
        {
            std::vector<string > listData;
            delimiter = ":";

            if (E_OK != CAmCommonUtility::parseString(delimiter, itlistProperty, listData))
            {
                LOG_FN_INFO(__FILENAME__, __func__, " wrong system property data format");
                return;
            }

            for (auto itlistData = listData.begin(); itlistData != listData.end(); itlistData++)
            {
                string propType = *itlistData;
                int    type     = atoi(propType.c_str());
                itlistData++;
                if (itlistData == listData.end())
                {
                    LOG_FN_INFO(__FILENAME__, __func__, "wrong system property data format");
                    continue;
                }

                string                 propVal = *itlistData;
                int                    val     = atoi(propVal.c_str());
                am_SystemProperty_s property;
                property.type  = (am_CustomSystemPropertyType_t)type;
                property.value = val;
                listSystemProperty.push_back(property);
            }
        }

        listparam.setParam(listSystemProperty);
        pAction->setParam(ACTION_PARAM_LIST_SYSTEM_PROPERTIES, &listparam);
    }

    checkAndSetStringParam(mapParams, ACTION_PARAM_CONNECTION_NAME, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_RAMP_TYPE, am_CustomRampType_t, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_RAMP_TIME, am_time_t, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_MAIN_VOLUME, am_mainVolume_t, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_MAIN_VOLUME_STEP, am_mainVolume_t, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_VOLUME, am_volume_t, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_VOLUME_STEP, am_volume_t, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_ORDER, gc_Order_e, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_PROPERTY_TYPE, am_CustomSoundPropertyType_t,
        pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_PROPERTY_VALUE, int16_t, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_DEBUG_TYPE, uint16_t, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_DEBUG_VALUE, int16_t, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_CONNECTION_FORMAT, am_CustomConnectionFormat_t,
        pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_NOTIFICATION_CONFIGURATION_TYPE,
        am_CustomNotificationType_t, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_NOTIFICATION_CONFIGURATION_STATUS,
        am_NotificationStatus_e, pAction);
    checkAndSetNumericParam(mapParams, ACTION_PARAM_NOTIFICATION_CONFIGURATION_PARAM, int16_t,
        pAction);

}

void CAmPolicyAction::_createActions(gc_Action_s &policyAction,
    std::vector<IAmActionCommand * > &listFrameworkActions)
{
    IAmActionCommand *pAction = NULL;
    switch (policyAction.actionType)
    {
        case ACTION_SET_VOLUME:
            pAction = new CAmActionSetVolume();
            break;

        case ACTION_LIMIT:
        case ACTION_UNLIMIT:
            pAction = new CAmActionLimit();
            break;

        case ACTION_MUTE:
        case ACTION_UNMUTE:
            pAction = new CAmActionMute();
            break;

        case ACTION_CONNECT:
        case ACTION_POP:
        case ACTION_RESUME:
            pAction = new CAmActionConnect();
            break;

        case ACTION_DISCONNECT:
            pAction = new CAmActionDisconnect();
            break;

        case ACTION_PUSH:
            pAction = new CAmActionInterrupt();
            break;
        case ACTION_SUSPEND:
            pAction = new CAmActionSuspend();
            break;
        case ACTION_REGISTER:
            pAction = new CAmSystemActionRegister(CAmSystemFactory::getElement(SYSTEM_ELEMENT_NAME));
            break;

        case ACTION_DEBUG:
        case ACTION_SET_SYSTEM_PROPERTY:
        case ACTION_SET_SYSTEM_PROPERTIES:
        {
            std::shared_ptr<CAmSystemElement > pSystem = CAmSystemFactory::getElement(
                    SYSTEM_ELEMENT_NAME);
            if (pSystem != nullptr)
            {
                if ((policyAction.actionType == ACTION_SET_SYSTEM_PROPERTY) || (policyAction.actionType == ACTION_DEBUG))
                    pAction = new CAmSystemActionSetProperty(pSystem);
                else if (policyAction.actionType == ACTION_SET_SYSTEM_PROPERTIES)
                    pAction = new CAmSystemActionSetProperties(pSystem);
            }
            break;
        }

        case ACTION_SET_PROPERTY:
        case ACTION_SET_PROPERTIES:
        case ACTION_SET_NOTIFICATION_CONFIGURATION:
        {
            std::string sourceName = _getParam(policyAction.mapParameters,
                    ACTION_PARAM_SOURCE_NAME);
            std::string sinkName = _getParam(policyAction.mapParameters, ACTION_PARAM_SINK_NAME);
            if (false == sourceName.empty())
            {
                std::shared_ptr<CAmSourceElement > pSourceElement = CAmSourceFactory::getElement(
                        sourceName);
                if (pSourceElement != nullptr)
                {
                    if (policyAction.actionType == ACTION_SET_PROPERTY)
                    {
                        pAction = new CAmSourceSinkActionSetSoundProperty<CAmSourceElement >(
                                pSourceElement);
                    }
                    else if (policyAction.actionType == ACTION_SET_PROPERTIES)
                    {
                        pAction = new CAmSourceSinkActionSetSoundProperties<
                                CAmSourceElement >(pSourceElement);
                    }
                    else
                    {
                        pAction = new CAmSourceSinkActionSetNotificationConfiguration<
                                CAmSourceElement >(pSourceElement);
                    }
                }
            }
            else if (false == sinkName.empty())
            {
                std::shared_ptr<CAmSinkElement > pSinkElement = CAmSinkFactory::getElement(
                        sinkName);
                if (pSinkElement != nullptr)
                {
                    if (policyAction.actionType == ACTION_SET_PROPERTY)
                    {
                        pAction = new CAmSourceSinkActionSetSoundProperty<CAmSinkElement >(
                                pSinkElement);
                    }
                    else if (policyAction.actionType == ACTION_SET_PROPERTIES)
                    {
                        pAction = new CAmSourceSinkActionSetSoundProperties<CAmSinkElement >(
                                pSinkElement);
                    }
                    else
                    {
                        pAction = new CAmSourceSinkActionSetNotificationConfiguration<
                                CAmSinkElement >(pSinkElement);
                    }
                }
            }
            break;
        }
    }

    if (NULL != pAction)
    {
        _setActionParameters(policyAction.mapParameters, pAction);
        listFrameworkActions.push_back(pAction);
    }
}

}
}
