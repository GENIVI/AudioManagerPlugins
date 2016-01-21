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
#include  "CAmControlReceive.h"
#include "IAmPolicySend.h"
#include "CAmPolicyAction.h"

#include "CAmClassActionSetLimitState.h"
#include "CAmClassActionConnect.h"
#include "CAmClassActionDisconnect.h"
#include "CAmClassActionSetVolume.h"
#include "CAmClassActionInterrupt.h"
#include "CAmClassActionSuspend.h"
#include "CAmSourceSinkActionSetSoundProperty.h"
#include "CAmSystemActionRegister.h"
#include "CAmSystemActionDebug.h"
#include "CAmClassElement.h"
#include "CAmLogger.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"

namespace am {
namespace gc {

#define isParameterSet(A,B) (1 == B.count(A))

CAmPolicyAction::CAmPolicyAction(const std::vector<gc_Action_s > &listPolicyActions,
                                 IAmPolicySend* pPolicySend, CAmControlReceive* pControlReceive) :
                                CAmActionContainer(std::string("CAmPolicyAction")),
                                mListActions(listPolicyActions),
                                mpPolicySend(pPolicySend),
                                mpControlReceive(pControlReceive)
{
}

CAmPolicyAction::~CAmPolicyAction()
{
}

int CAmPolicyAction::_execute(void)
{
    std::vector<gc_Action_s >::iterator itListPolicyActions;
    std::vector<IAmActionCommand * > listFrameworkActions;
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

am_Error_e CAmPolicyAction::_getListNames(std::string name, std::vector<std::string >& listNames)
{
    listNames.clear();
    std::string temp;
    std::stringstream ss(name);
    while (ss >> temp)
    {
        listNames.push_back(temp);
    }
    return E_OK;
}

void CAmPolicyAction::_setActionParameters(gc_Action_s &policyAction,
                                           IAmActionCommand *pFrameworkAction)
{
    if (isParameterSet(ACTION_PARAM_SOURCE_NAME, policyAction.mapParameters))
    {
        std::string sourceName = policyAction.mapParameters[ACTION_PARAM_SOURCE_NAME];
        /*
         * If no whitespace is found then its a single name
         */
        if (sourceName.find(" ") == std::string::npos)
        {
            CAmActionParam < std::string > sourceNameParam(sourceName);
            pFrameworkAction->setParam(ACTION_PARAM_SOURCE_NAME, &sourceNameParam);
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
            pFrameworkAction->setParam(ACTION_PARAM_SOURCE_INFO, &listSourcesParameter);
        }
    }
    if (isParameterSet(ACTION_PARAM_SINK_NAME, policyAction.mapParameters))
    {
        std::string sinkName = policyAction.mapParameters[ACTION_PARAM_SINK_NAME];
        /*
         * If no whitespace is found then its a single name
         */
        if (sinkName.find(" ") == std::string::npos)
        {
            CAmActionParam < std::string > sinkNameParam(sinkName);
            pFrameworkAction->setParam(ACTION_PARAM_SINK_NAME, &sinkNameParam);
        }
        else
        {
            std::vector < gc_Sink_s > listSinks;
            std::vector < std::string > listNames;
            /*
             * Find the list of sources based on the whitespace separated names
             */
            _getListNames(sinkName, listNames);
            mpPolicySend->getListElements(listNames, listSinks);
            CAmActionParam < std::vector<gc_Sink_s > > listSinksParameter(listSinks);
            pFrameworkAction->setParam(ACTION_PARAM_SINK_INFO, &listSinksParameter);
        }
    }
    if (isParameterSet(ACTION_PARAM_GATEWAY_NAME, policyAction.mapParameters))
    {
        std::string gatewayName = policyAction.mapParameters[ACTION_PARAM_GATEWAY_NAME];
        /*
         * If no whitespace is found then its a single name
         */
        if (gatewayName.find(" ") == std::string::npos)
        {
            CAmActionParam < std::string > sinkNameParam(gatewayName);
            pFrameworkAction->setParam(ACTION_PARAM_GATEWAY_NAME, &sinkNameParam);
        }
        else
        {
            std::vector < gc_Gateway_s > listGateways;
            std::vector < std::string > listNames;
            /*
             * Find the list of sources based on the whitespace separated names
             */
            _getListNames(gatewayName, listNames);
            mpPolicySend->getListElements(listNames, listGateways);
            CAmActionParam < std::vector<gc_Gateway_s > > listGatewaysParameter(listGateways);
            pFrameworkAction->setParam(ACTION_PARAM_GATEWAY_INFO, &listGatewaysParameter);
        }
    }

    if (isParameterSet(ACTION_PARAM_CLASS_NAME, policyAction.mapParameters))
    {
        CAmActionParam < std::string > className(
                        policyAction.mapParameters[ACTION_PARAM_CLASS_NAME]);
        pFrameworkAction->setParam(ACTION_PARAM_CLASS_NAME, &className);
    }
    if (isParameterSet(ACTION_PARAM_RAMP_TYPE, policyAction.mapParameters))
    {
        CAmActionParam<int > rampType(
                        atoi(policyAction.mapParameters[ACTION_PARAM_RAMP_TYPE].data()));
        pFrameworkAction->setParam(ACTION_PARAM_RAMP_TYPE, &rampType);
    }
    if (isParameterSet(ACTION_PARAM_RAMP_TIME, policyAction.mapParameters))
    {
        CAmActionParam<int > rampTime(
                        atoi(policyAction.mapParameters[ACTION_PARAM_RAMP_TIME].data()));
        pFrameworkAction->setParam(ACTION_PARAM_RAMP_TIME, &rampTime);
    }
    if (isParameterSet(ACTION_PARAM_PATTERN, policyAction.mapParameters))
    {
        CAmActionParam < uint32_t > pattern(
                        strtol(policyAction.mapParameters[ACTION_PARAM_PATTERN].data(), NULL, 16));
        pFrameworkAction->setParam(ACTION_PARAM_PATTERN, &pattern);
    }
    if (isParameterSet(ACTION_PARAM_TIMEOUT, policyAction.mapParameters))
    {
        pFrameworkAction->setTimeout(atoi(policyAction.mapParameters[ACTION_PARAM_TIMEOUT].data()));

    }
    if (isParameterSet(ACTION_PARAM_MAIN_VOLUME, policyAction.mapParameters))
    {
        CAmActionParam < am_mainVolume_t > mainVolume(
                        atoi(policyAction.mapParameters[ACTION_PARAM_MAIN_VOLUME].data()));
        pFrameworkAction->setParam(ACTION_PARAM_MAIN_VOLUME, &mainVolume);
    }
    if (isParameterSet(ACTION_PARAM_MAIN_VOLUME_STEP, policyAction.mapParameters))
    {
        CAmActionParam < am_mainVolume_t > mainVolume(
                        atoi(policyAction.mapParameters[ACTION_PARAM_MAIN_VOLUME_STEP].data()));
        pFrameworkAction->setParam(ACTION_PARAM_MAIN_VOLUME_STEP, &mainVolume);
    }
    if (isParameterSet(ACTION_PARAM_VOLUME, policyAction.mapParameters))
    {
        CAmActionParam < am_volume_t > targetVolume(
                        atoi(policyAction.mapParameters[ACTION_PARAM_VOLUME].data()));
        pFrameworkAction->setParam(ACTION_PARAM_VOLUME, &targetVolume);
    }
    if (isParameterSet(ACTION_PARAM_VOLUME_STEP, policyAction.mapParameters))
    {
        CAmActionParam < am_volume_t > offsetVolume(
                        atoi(policyAction.mapParameters[ACTION_PARAM_VOLUME_STEP].data()));
        pFrameworkAction->setParam(ACTION_PARAM_VOLUME_STEP, &offsetVolume);
    }
    if (isParameterSet(ACTION_PARAM_ORDER, policyAction.mapParameters))
    {
        CAmActionParam < gc_Order_e > order(
                        (gc_Order_e)atoi(policyAction.mapParameters[ACTION_PARAM_ORDER].data()));
        pFrameworkAction->setParam(ACTION_PARAM_ORDER, &order);
    }
    if (isParameterSet(ACTION_PARAM_PROPERTY_TYPE, policyAction.mapParameters))
    {
        CAmActionParam < am_CustomMainSoundPropertyType_t > propertyType(
                        (am_CustomMainSoundPropertyType_t)atoi(
                                        policyAction.mapParameters[ACTION_PARAM_PROPERTY_TYPE].data()));
        pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_TYPE, &propertyType);
    }
    if (isParameterSet(ACTION_PARAM_PROPERTY_VALUE, policyAction.mapParameters))
    {
        CAmActionParam < int16_t > propertyValue(
                        atoi(policyAction.mapParameters[ACTION_PARAM_PROPERTY_VALUE].data()));
        pFrameworkAction->setParam(ACTION_PARAM_PROPERTY_VALUE, &propertyValue);
    }
    if (isParameterSet(ACTION_PARAM_EXCEPT_SOURCE_NAME, policyAction.mapParameters))
    {
        std::vector < std::string > listNames;
        _getListNames(policyAction.mapParameters[ACTION_PARAM_EXCEPT_SOURCE_NAME], listNames);
        CAmActionParam < std::vector<std::string > > listExceptSourcesParameter(listNames);
        pFrameworkAction->setParam(ACTION_PARAM_EXCEPT_SOURCE_NAME, &listExceptSourcesParameter);
    }
    if (isParameterSet(ACTION_PARAM_EXCEPT_SINK_NAME, policyAction.mapParameters))
    {
        std::vector < std::string > listNames;
        _getListNames(policyAction.mapParameters[ACTION_PARAM_EXCEPT_SINK_NAME], listNames);
        CAmActionParam < std::vector<std::string > > listExceptSinkParameter(listNames);
        pFrameworkAction->setParam(ACTION_PARAM_EXCEPT_SINK_NAME, &listExceptSinkParameter);
    }
    if (isParameterSet(ACTION_PARAM_CONNECTION_STATE, policyAction.mapParameters))
    {
        CAmActionParam < std::string > filter(
                        policyAction.mapParameters[ACTION_PARAM_CONNECTION_STATE]);
        pFrameworkAction->setParam(ACTION_PARAM_CONNECTION_STATE, &filter);
    }
    if (isParameterSet(ACTION_PARAM_MUTE_STATE, policyAction.mapParameters))
    {
        am_MuteState_e muteState;
        muteState = (am_MuteState_e)atoi(
                        policyAction.mapParameters[ACTION_PARAM_MUTE_STATE].c_str());
        CAmActionParam < gc_LimitState_e > limitStateParam;
        CAmActionParam < am_volume_t > targetVolumeParam;
        CAmActionParam < uint32_t > patternParam;
        LOG_FN_INFO("MUTESTATE=", muteState);
        if (muteState == MS_MUTED)
        {
            limitStateParam.setParam(LS_LIMITED);
            targetVolumeParam.setParam(AM_MUTE);
        }
        else
        {
            limitStateParam.setParam(LS_UNLIMITED);
            targetVolumeParam.setParam(0);
        }
        patternParam.setParam(0xFFFF0000);
        pFrameworkAction->setParam(ACTION_PARAM_LIMIT_STATE, &limitStateParam);
        pFrameworkAction->setParam(ACTION_PARAM_VOLUME, &targetVolumeParam);
        pFrameworkAction->setParam(ACTION_PARAM_PATTERN, &patternParam);
    }
    if (isParameterSet(ACTION_PARAM_LIMIT_STATE, policyAction.mapParameters))
    {
        CAmActionParam < uint32_t > patternParam;
        CAmActionParam < gc_LimitState_e > volumeOperationParam(
                        (gc_LimitState_e)atoi(
                                        policyAction.mapParameters[ACTION_PARAM_LIMIT_STATE].c_str()));
        patternParam.setParam(0xFFFF);
        pFrameworkAction->setParam(ACTION_PARAM_LIMIT_STATE, &volumeOperationParam);
        pFrameworkAction->setParam(ACTION_PARAM_PATTERN, &patternParam);
    }
    if (isParameterSet(ACTION_PARAM_DEBUG_TYPE, policyAction.mapParameters))
    {
        CAmActionParam < uint16_t > debugTypeParam(
                       atoi(policyAction.mapParameters[ACTION_PARAM_DEBUG_TYPE].c_str()));

        pFrameworkAction->setParam(ACTION_PARAM_DEBUG_TYPE, &debugTypeParam);
    }
    if (isParameterSet(ACTION_PARAM_DEBUG_VALUE, policyAction.mapParameters))
    {
        CAmActionParam < int16_t > debugValueParam(
                        atoi(policyAction.mapParameters[ACTION_PARAM_DEBUG_VALUE].c_str()));
        pFrameworkAction->setParam(ACTION_PARAM_DEBUG_VALUE, &debugValueParam);
    }
    if (isParameterSet(ACTION_PARAM_CONNECTION_FORMAT, policyAction.mapParameters))
    {
        CAmActionParam < am_CustomConnectionFormat_t > connectionFormatParam(
                        atoi(policyAction.mapParameters[ACTION_PARAM_CONNECTION_FORMAT].c_str()));
        pFrameworkAction->setParam(ACTION_PARAM_CONNECTION_FORMAT, &connectionFormatParam);
    }
}

void CAmPolicyAction::_getClassList(gc_Action_s& policyAction,
                                    std::vector<CAmClassElement * >& listClasses)
{
    std::string sourceName = _getParam(policyAction.mapParameters, ACTION_PARAM_SOURCE_NAME);
    std::string sinkName = _getParam(policyAction.mapParameters, ACTION_PARAM_SINK_NAME);
    std::string className = _getParam(policyAction.mapParameters, ACTION_PARAM_CLASS_NAME);
    std::vector < std::string > listClassNames;
    std::vector<std::string >::iterator itListClassNames;
    std::vector<CAmClassElement * > listTempClasses;
    std::vector<CAmClassElement * >::iterator itlistTempClasses;
    if (isParameterSet(ACTION_PARAM_EXCEPT_CLASS_NAME, policyAction.mapParameters))
    {
        _getListNames(policyAction.mapParameters[ACTION_PARAM_EXCEPT_CLASS_NAME], listClassNames);
    }
    if (((true == sourceName.empty()) && (true == sinkName.empty()) && (true == className.empty())) || ("ALL"
                    == className))
    {
        CAmClassFactory::getListElements(listClasses);
    }
    else if ((false == sourceName.empty()) && (false == sinkName.empty())) // the source and sink is specified explicitly
    {
        CAmClassElement *pClassElement = (CAmClassElement*)CAmClassFactory::getClassElement(
                        sourceName, sinkName);
        if (NULL != pClassElement)
        {
            listClasses.push_back(pClassElement);
        }
    }
    else if (false == className.empty()) // both source and sink information are not available but class name is specified explicitly
    {
        CAmClassElement *pClassElement = (CAmClassElement*)CAmClassFactory::getElement(className);
        if (NULL != pClassElement)
        {
            listClasses.push_back(pClassElement);
        }
    }
    else // find the class which contains given source or sink
    {
        if (true == sinkName.empty())
        {
            CAmClassFactory::getElementsBySource(sourceName, listClasses);
        }
        else if (true == sourceName.empty())
        {
            CAmClassFactory::getElementsBySink(sinkName, listClasses);
        }
    }
    listTempClasses = listClasses;
    listClasses.clear();
    for (itlistTempClasses = listTempClasses.begin(); itlistTempClasses != listTempClasses.end();
                    itlistTempClasses++)
    {
        for (itListClassNames = listClassNames.begin(); itListClassNames != listClassNames.end();
                        ++itListClassNames)
        {
            if (*itListClassNames == (*itlistTempClasses)->getName())
            {
                break;
            }
        }
        if (itListClassNames == listClassNames.end())
        {
            listClasses.push_back(*itlistTempClasses);
        }
    }

}

void CAmPolicyAction::_getActions(gc_Action_s& policyAction,
                                  std::vector<IAmActionCommand * >& listFrameworkActions)
{
    std::vector<CAmClassElement* > listClasses;
    _getClassList(policyAction, listClasses);
    if (false == listClasses.empty())
    {
        IAmActionCommand *pAction = NULL;
        for (std::vector<CAmClassElement* >::iterator itListClasses = listClasses.begin();
                        itListClasses != listClasses.end(); itListClasses++)
        {
            if (policyAction.actionName == ACTION_NAME_CONNECT)
            {
                pAction = new CAmClassActionConnect(*itListClasses);
            }
            else if (policyAction.actionName == ACTION_NAME_SET_VOLUME)
            {
                pAction = new CAmClassActionSetVolume(*itListClasses);
            }
            else if (policyAction.actionName == ACTION_NAME_INTERRUPT)
            {
                pAction = new CAmClassActionInterrupt(*itListClasses);
            }
            else if (policyAction.actionName == ACTION_NAME_SUSPEND)
            {
                pAction = new CAmClassActionSuspend(*itListClasses);
            }
            else if (policyAction.actionName == ACTION_NAME_DISCONNECT)
            {
                pAction = new CAmClassActionDisconnect(*itListClasses);
            }
            else if ((policyAction.actionName == ACTION_NAME_LIMIT) || (policyAction.actionName
                            == ACTION_NAME_MUTE))
            {
                pAction = new CAmClassActionSetLimitState(*itListClasses);
            }
            else
            {
                pAction = NULL;
            }
            if (NULL != pAction)
            {
                _setActionParameters(policyAction, pAction);
                listFrameworkActions.push_back(pAction);
            }
        }
    }
}

void CAmPolicyAction::_createActions(gc_Action_s& policyAction,
                                     std::vector<IAmActionCommand * >& listFrameworkActions)
{
    IAmActionCommand *pFrameworkAction = NULL;

    // instantiate the action
    if ((policyAction.actionName == ACTION_NAME_CONNECT) || (policyAction.actionName
                    == ACTION_NAME_DISCONNECT)
        || (policyAction.actionName == ACTION_NAME_SET_VOLUME)
        || (policyAction.actionName == ACTION_NAME_LIMIT)
        || (policyAction.actionName == ACTION_NAME_INTERRUPT)
        || (policyAction.actionName == ACTION_NAME_SUSPEND)
        || (policyAction.actionName == ACTION_NAME_MUTE))
    {
        _getActions(policyAction, listFrameworkActions);
    }
    else if (policyAction.actionName == ACTION_NAME_REGISTER)
    {
        pFrameworkAction = new CAmSystemActionRegister(mpControlReceive);
        if (pFrameworkAction != NULL)
        {
            _setActionParameters(policyAction, pFrameworkAction);
            listFrameworkActions.push_back(pFrameworkAction);
        }
    }
    else if (policyAction.actionName == ACTION_DEBUG)
    {
        pFrameworkAction = new CAmSystemActionDebug(mpControlReceive);
        if (pFrameworkAction != NULL)
        {
            _setActionParameters(policyAction, pFrameworkAction);
            listFrameworkActions.push_back(pFrameworkAction);
        }
    }
    else if (policyAction.actionName == ACTION_NAME_SET_PROPERTY)
    {
        std::string sourceName = _getParam(policyAction.mapParameters, ACTION_PARAM_SOURCE_NAME);
        std::string sinkName = _getParam(policyAction.mapParameters, ACTION_PARAM_SINK_NAME);
        if (false == sourceName.empty())
        {
            CAmSourceElement* pElement = CAmSourceFactory::getElement(sourceName);
            pFrameworkAction = new CAmSourceSinkActionSetSoundProperty<CAmSourceElement >(pElement);
        }
        else if (false == sinkName.empty())
        {
            CAmSinkElement* pElement = CAmSinkFactory::getElement(sinkName);
            pFrameworkAction = new CAmSourceSinkActionSetSoundProperty<CAmSinkElement >(pElement);
        }
        if (NULL != pFrameworkAction)
        {
            _setActionParameters(policyAction, pFrameworkAction);
            listFrameworkActions.push_back(pFrameworkAction);
        }
    }
    else
    {
        pFrameworkAction = NULL;
    }
}

}
}
