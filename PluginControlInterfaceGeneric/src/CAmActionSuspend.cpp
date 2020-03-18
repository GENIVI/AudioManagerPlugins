/******************************************************************************
 * @file: CAmClassActionSuspend.cpp
 *
 * This file contains the definition of user action suspend class
 * (member functions and data members) used to implement the logic of suspending
 * the connected connection at user level
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

#include "CAmActionSuspend.h"

#include "CAmLogger.h"
#include "CAmClassElement.h"
#include "CAmMainConnectionActionSuspend.h"
#include "CAmActionLimit.h"

namespace am {
namespace gc {

CAmActionSuspend::CAmActionSuspend()
    : CAmActionContainer(std::string("CAmActionSuspend"))
    , mOrderParam(DEFAULT_CONNECT_ORDER)
{
    _registerParam(ACTION_PARAM_CLASS_NAME, &mClassNameParam);
    _registerParam(ACTION_PARAM_EXCEPT_CLASS_NAME, &mExceptClassParam);
    _registerParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    _registerParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    _registerParam(ACTION_PARAM_ORDER, &mOrderParam);
}

CAmActionSuspend::~CAmActionSuspend(void)
{
}

int CAmActionSuspend::_execute(void)
{
    _getTargetConnections();
    if (mListMainConnections.empty())
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "No main Connection found with matching criteria");
    }

    // setup child actions
    for (auto pMainConnectionElement : mListMainConnections)
    {
        IAmActionCommand *pAction = new CAmMainConnectionActionSuspend(pMainConnectionElement);
        if (NULL != pAction)
        {
            pAction->setUndoRequried(true);
            auto pClassElement = pMainConnectionElement->getClassElement();
            if (pClassElement)
            {
                CAmActionParam<gc_SetSourceStateDirection_e > setSourceStateDir;
                setSourceStateDir.setParam(
                    classTypeToDisconnectDirectionLUT[pClassElement->getClassType()]);
                pAction->setParam(ACTION_PARAM_SET_SOURCE_STATE_DIRECTION, &setSourceStateDir);
            }
            append(pAction);
        }

        pAction = _createActionSetLimitState(pMainConnectionElement);
        if (NULL != pAction)
        {
            append(pAction);
        }
    }

    return E_OK;
}

int CAmActionSuspend::_update(const int result)
{
    for (auto pMainConnectionElement : mListMainConnections)
    {
        auto pClassElement = pMainConnectionElement->getClassElement();
        if (pClassElement)
        {
            pClassElement->updateMainConnectionQueue();
        }
    }

    return E_OK;
}

void CAmActionSuspend::_getTargetConnections()
{
    std::string                                 className;
    gc_Order_e                                  order;
    std::vector<std::string >                   listExcludedClasses;

    mClassNameParam.getParam(className);
    mExceptClassParam.getParam(listExcludedClasses);
    mOrderParam.getParam(order);

    std::vector<std::shared_ptr<CAmMainConnectionElement>> listConnections;
    CAmConnectionListFilter filter;
    filter.setClassName(className);
    filter.setListExceptClassNames(listExcludedClasses);
    CAmMainConnectionFactory::getListElements(listConnections, filter, order);

    // group found connection elements by class
    map<string, vector<shared_ptr<CAmMainConnectionElement>>> listConnectionsPerClass;
    for (auto &pMainConnectionElement : listConnections)
    {
        listConnectionsPerClass[pMainConnectionElement->getClassName()].push_back(pMainConnectionElement);
    }

    // for each class, select up to a single connection by considering conflicts
    for (auto &listCandidates : listConnectionsPerClass)
    {
        shared_ptr<CAmMainConnectionElement > pSelected;
        _selectConnection(listCandidates.second, pSelected);
        if (pSelected != nullptr)
        {
            mListMainConnections.push_back(pSelected);
        }
    }
}

void CAmActionSuspend::_selectConnection(vector<shared_ptr<CAmMainConnectionElement>> &listConnections
        , shared_ptr<CAmMainConnectionElement> &pSelected)
{
    shared_ptr<CAmMainConnectionElement> pConnected, pPreferred;
    string                               sourceName, sinkName;
    CAmConnectionListFilter              filter;
    mSourceNameParam.getParam(sourceName);
    mSinkNameParam.getParam(sinkName);
    filter.setSourceName(sourceName);
    filter.setSinkName(sinkName);

    // detect possible conflicts
    for (const auto &pCandidate : listConnections)
    {
        if (pCandidate == nullptr)
        {
            continue;
        }

        am_ConnectionState_e state = pCandidate->getState();
        switch (state)
        {
            // First check if suspended connection already exists in the queue
            case CS_SUSPENDED:
                LOG_FN_ERROR(__FILENAME__, __func__, "Suspended connection already present in the queue");
                return;    // nothing to be done

            // Next get the connection in connected state
            case CS_CONNECTED:
                pConnected = pCandidate;
                if ((pPreferred == nullptr) && filter.matches(pCandidate))
                {
                    pPreferred = pCandidate;
                }
                break;

            case CS_DISCONNECTED:
            case CS_UNKNOWN:
            {
                if ((pPreferred == nullptr) && filter.matches(pCandidate))
                {
                    pPreferred = pCandidate;
                }
                break;
            }
        }
    }

    // make our decision
    if (pConnected && pPreferred && (pPreferred != pConnected))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "Not allowed to have CS_CONNECTED as well as CS_SUSPENDED");
    }
    else if (pPreferred)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "picking", pPreferred->getState(), pPreferred->getName());
        mListMainConnections.push_back(pPreferred);
    }
}

IAmActionCommand *CAmActionSuspend::_createActionSetLimitState(
    std::shared_ptr<CAmMainConnectionElement > pMainConnection)
{
    IAmActionCommand *pAction = new CAmActionLimit();
    if (NULL != pAction)
    {
        CAmActionParam<std::string>     connectionNameParam(pMainConnection->getName());
        pAction->setParam(ACTION_PARAM_CONNECTION_NAME, &connectionNameParam);

        CAmActionParam<gc_LimitType_e > limitTypeParam(LT_UNKNOWN);
        pAction->setParam(ACTION_PARAM_LIMIT_STATE, &limitTypeParam);
        pAction->setUndoRequried(true);
    }

    return pAction;
}

} /* namespace gc */
} /* namespace am */
