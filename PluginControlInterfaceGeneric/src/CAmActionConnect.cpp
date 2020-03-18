/******************************************************************************
 * @file: CAmClassActionConnect.cpp
 *
 * This file contains the definition of user connection action connect class
 * (member functions and data members) used to implement the logic of disconnect
 * at user level
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

#include "CAmActionConnect.h"

#include "CAmLogger.h"
#include "CAmClassElement.h"
#include "CAmMainConnectionElement.h"
#include "CAmMainConnectionActionConnect.h"
#include "CAmMainConnectionActionDisconnect.h"
#include "CAmActionSetVolume.h"

namespace am {
namespace gc {

CAmActionConnect::CAmActionConnect()
    : CAmActionContainer(std::string("CAmActionConnect"))
    , mOrderParam(DEFAULT_CONNECT_ORDER)
    , mDisposeMainConnection(true)
{
    // target selection parameters
    _registerParam(ACTION_PARAM_CLASS_NAME, &mClassNameParam);
    _registerParam(ACTION_PARAM_EXCEPT_CLASS_NAME, &mExceptClassParam);
    _registerParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    _registerParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    _registerParam(ACTION_PARAM_ORDER, &mOrderParam);

    // optional parameters to pass-on to the implicit CAmMainConnectionActionConnect
    _registerParam(ACTION_PARAM_CONNECTION_FORMAT, &mConnectionFormatParam);
    _registerParam(ACTION_PARAM_MAIN_VOLUME, &mMainVolumeParam);
    _registerParam(ACTION_PARAM_MAIN_VOLUME_STEP, &mMainVolumeStepParam);
    _registerParam(ACTION_PARAM_VOLUME, &mVolumeParam);
    _registerParam(ACTION_PARAM_VOLUME_STEP, &mVolumeStepParam);
}

CAmActionConnect::~CAmActionConnect()
{
}

int CAmActionConnect::_execute(void)
{
    // obtain parameters
    mClassNameParam.getParam(mClassName);
    mSourceNameParam.getParam(mSourceName);
    mSinkNameParam.getParam(mSinkName);
    LOG_FN_INFO(__FILENAME__, __func__, "class:", mClassName, "source:", mSourceName, "sink:", mSinkName);

    // get related main connection
    am_Error_e result = _getTargetConnections();
    if ((E_OK != result) && (E_ALREADY_EXISTS != result))
    {
        return result;
    }

    if (mListMainConnections.empty())
    {
        LOG_FN_WARN(__FILENAME__, __func__, "NO suitable main connections found for source"
            , mSourceName, "and sink", mSinkName, "in class", mClassName);
    }

    for (auto &pMainConnectionElement : mListMainConnections)
    {
        IAmActionCommand *pAction = _createActionMainConnectionSetVolume(pMainConnectionElement);
        if (NULL != pAction)
        {
            append(pAction);
        }

        pAction = new CAmMainConnectionActionConnect(pMainConnectionElement);
        if (NULL != pAction)
        {
            pAction->setParam(ACTION_PARAM_CONNECTION_FORMAT, &mConnectionFormatParam);
            pAction->setUndoRequried(true);
            auto pClassElement = pMainConnectionElement->getClassElement();
            if (pClassElement != nullptr)
            {
                CAmActionParam<gc_SetSourceStateDirection_e > setSourceStateDir;
                setSourceStateDir.setParam(classTypeToConnectDirectionLUT[pClassElement->getClassType()]);
                pAction->setParam(ACTION_PARAM_SET_SOURCE_STATE_DIRECTION, &setSourceStateDir);
            }
            append(pAction);
        }
    }

    return E_OK;
}

int CAmActionConnect::_update(const int result)
{
    for (auto &pMainConnectionElement : mListMainConnections)
    {
        _updateConnectionElement(result, pMainConnectionElement);
    }

    return E_OK;
}

void CAmActionConnect::_updateConnectionElement(int result
        , shared_ptr<CAmMainConnectionElement> &pMainConnectionElement)
{
    /*
     * If Error occurred during the execution of one of the child action
     * and the result code is E_DATABASE_ERROR means the route connect action
     * failed on availability check , in this case don't dispose main Connection
     */
    auto pClassElement = pMainConnectionElement->getClassElement();
    if(pClassElement == nullptr)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "INVALID class  element obtained from connection"
                , pMainConnectionElement->getName());
        return;
    }

    switch (this->getStatus())
    {
    case AS_ERROR_STOPPED:
        if (result == E_DATABASE_ERROR)
        {
            mDisposeMainConnection = false;
        }

        break;
    case AS_UNDO_COMPLETE:
        if (mDisposeMainConnection && pMainConnectionElement)
        {
            pClassElement->disposeConnection(pMainConnectionElement);
        }

        break;
    case AS_COMPLETED:
    {
        if (!pMainConnectionElement)
        {
            break;
        }

        am_ConnectionState_e state = pMainConnectionElement->getState();
        if (CS_CONNECTED == state)
        {
            pClassElement->setLastVolume(pMainConnectionElement);
        }

        break;
    }
    default:
        break;
    }

    pClassElement->updateMainConnectionQueue();
}

am_Error_e CAmActionConnect::_getTargetConnections()
{
    gc_Order_e                order;
    std::vector<std::string > listExcludedClasses;
    mOrderParam.getParam(order);
    mExceptClassParam.getParam(listExcludedClasses);

    // first collect existing main connections which match the criteria
    std::vector<std::shared_ptr<CAmMainConnectionElement>> listConnections;
    CAmConnectionListFilter filter;
    filter.setClassName(mClassName);
    filter.setSinkName(mSinkName);
    filter.setListExceptClassNames(listExcludedClasses);
    CAmMainConnectionFactory::getListElements(listConnections, filter, order);

    // if exactly specified connection is not found, check if we can create the requested
    if (!mSourceName.empty() && !mSinkName.empty()
            && (CAmMainConnectionFactory::getElement(mSourceName + ":" + mSinkName) == nullptr))
    {
        auto pClass = CAmClassFactory::getElement(mSourceName, mSinkName);
        if (pClass == nullptr)
        {
            return E_NON_EXISTENT;
        }
        else if (!listExcludedClasses.empty()
                && CAmConnectionListFilter::matchesAny(listExcludedClasses, pClass->getName()))
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, "class candidate", pClass->getName(), "for source"
                    , mSourceName, "and sink", mSinkName, "explicitly excluded");
            return E_OK;
        }

        am_mainConnectionID_t connID;
        am_Error_e result = pClass->createMainConnection(mSourceName, mSinkName, connID);
        if ((result != E_OK) && (result != E_ALREADY_EXISTS))
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "failed creating main connection from"
                , mSourceName, "to", mSinkName, "in class", mClassName, "with", result);
            return result;
        }

        listConnections.push_back(CAmMainConnectionFactory::getElement(connID));
        CAmMainConnectionFactory::moveToEnd(listConnections.back()->getName(), O_NEWEST);
    }

    // group found connection elements by class
    map<string, vector<shared_ptr<CAmMainConnectionElement>>> listConnectionsPerClass;
    for (auto &pMainConnectionElement : listConnections)
    {
        listConnectionsPerClass[pMainConnectionElement->getClassName()].push_back(pMainConnectionElement);
    }

    // for each class, select a single connection by considering conflicts
    for (auto &listCandidates : listConnectionsPerClass)
    {
        shared_ptr<CAmMainConnectionElement > pSelected;
        _selectConnection(listCandidates.second, pSelected);
        if (pSelected != nullptr)
        {
            mListMainConnections.push_back(pSelected);
        }
    }

    return E_OK;
}

am_Error_e CAmActionConnect::_selectConnection(vector<shared_ptr<CAmMainConnectionElement>> &listMainConnections
        , shared_ptr<CAmMainConnectionElement> &pSelected)
{
    am_Error_e result = E_OK;

    // analyze received list
    std::shared_ptr<CAmMainConnectionElement> pConnected, pSuspended, pInitialized;
    for (auto &pMainConnection : listMainConnections)
    {
        // detect any conflicts
        switch (pMainConnection->getState())
        {
        case CS_CONNECTED:
        case CS_CONNECTING:
            pConnected = pMainConnection;
            break;

        case CS_SUSPENDED:
            pSuspended = pMainConnection;
            break;

        case CS_UNKNOWN:
            pInitialized = pMainConnection;
            break;

        case CS_DISCONNECTED:
        case CS_DISCONNECTING:
            // not relevant for conflict detection, but handled below
            break;
        }
    }

    std::shared_ptr<CAmMainConnectionElement> pPreferredCandidate;
    CAmConnectionListFilter filter;
    filter.setSourceName(mSourceName);
    filter.setListConnectionStates({CS_DISCONNECTED, CS_DISCONNECTING, CS_UNKNOWN, CS_SUSPENDED});
    for (auto &pMainConnection : listMainConnections)
    {
        // find our candidate - take first valid from already sorted list
        if (filter.matches(pMainConnection))
        {
            pPreferredCandidate = pMainConnection;
            break;
        }
    }

    // make our decision
    if (pConnected)
    {
        LOG_FN_WARN(__FILENAME__, __func__, "NOT touching"
            , pConnected->getState(), pConnected->getName());
    }
    else if (pSuspended && pPreferredCandidate && (pSuspended != pPreferredCandidate))
    {
        // If the suspended connection and the connection which we are planning to connect
        // are not the same we will end up with the situation that we have suspended
        // as well as connected connection in the same queue.

        LOG_FN_WARN(__FILENAME__, __func__, "found conflicting CS_SUSPENDED", pSuspended->getName()
            , ", ... ignoring", pPreferredCandidate->getName(), "in state", pPreferredCandidate->getState());
    }
    else if (pPreferredCandidate)
    {
        // nothing blocks us so far - go ahead with connecting
        pSelected = pPreferredCandidate;

        LOG_FN_INFO(__FILENAME__, __func__, "Connection", pSelected->getName()
            , "picked, state =", pSelected->getState());
    }

    std::shared_ptr<CAmMainConnectionElement> &pConflicting = (pConnected ? pConnected : pSuspended);
    if (pConflicting && pInitialized)
    {
        // cannot connect this newly created element - forward it to state CS_DISCONNECTED
        LOG_FN_WARN(__FILENAME__, __func__, "found conflicting", pConflicting->getState(), pConflicting->getName()
            , " disconnecting CS_UNKNOWN", pInitialized->getName());

        append(new CAmMainConnectionActionDisconnect(pInitialized));
    }

    return result;
}

IAmActionCommand *CAmActionConnect::_createActionMainConnectionSetVolume(
    std::shared_ptr<CAmMainConnectionElement > pMainConnection)
{
    IAmActionCommand *pAction = new CAmActionSetVolume();
    if (NULL != pAction)
    {
        CAmActionParam<std::string > connectionNameParam(pMainConnection->getName());
        pAction->setParam(ACTION_PARAM_CONNECTION_NAME, &connectionNameParam);

        if (mMainVolumeParam.isSet() || mMainVolumeStepParam.isSet()
                || mVolumeParam.isSet() || mVolumeStepParam.isSet())
        {
            // override default behavior
            pAction->setParam(ACTION_PARAM_MAIN_VOLUME,      &mMainVolumeParam);
            pAction->setParam(ACTION_PARAM_MAIN_VOLUME_STEP, &mMainVolumeStepParam);
            pAction->setParam(ACTION_PARAM_VOLUME,           &mVolumeParam);
            pAction->setParam(ACTION_PARAM_VOLUME_STEP,      &mVolumeStepParam);
        }
        else
        {
            // not specified otherwise - restore last/default volume
            auto pSink = pMainConnection->getMainSink();
            am_mainVolume_t mainVolume = pSink->convertVolumeToMainVolume(pMainConnection->getVolume());
            pMainConnection->getClassElement()->getLastVolume(pMainConnection, mainVolume);
            CAmActionParam<am_mainVolume_t > mainVolumeParam;
            mainVolumeParam.setParam(mainVolume);
            pAction->setParam(ACTION_PARAM_MAIN_VOLUME, &mainVolumeParam);

            LOG_FN_DEBUG(__FILENAME__, __func__, "restoring default/last volume=", mainVolume
                    , "for main connection", pMainConnection->getName());
        }

        pAction->setUndoRequried(true);
    }

    return pAction;
}

}
}
