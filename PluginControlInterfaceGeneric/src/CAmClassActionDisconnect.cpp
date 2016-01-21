/******************************************************************************
 * @file: CAmClassActionDisconnect.cpp
 *
 * This file contains the definition of user connection action disconnect class
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

#include "CAmLogger.h"
#include "CAmMainConnectionElement.h"
#include "CAmMainConnectionActionDisconnect.h"
#include "CAmMainConnectionActionSetLimitState.h"
#include "CAmClassElement.h"
#include "CAmClassActionDisconnect.h"

namespace am {
namespace gc {

CAmClassActionDisconnect::CAmClassActionDisconnect(CAmClassElement *pClassElement) :
                                CAmActionContainer(std::string("CAmClassActionDisconnect")),
                                mpClassElement(pClassElement)
{
    this->_registerParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    this->_registerParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    this->_registerParam(ACTION_PARAM_EXCEPT_SOURCE_NAME, &mlistSourceExceptions);
    this->_registerParam(ACTION_PARAM_EXCEPT_SINK_NAME, &mlistSinkExceptions);
    this->_registerParam(ACTION_PARAM_CONNECTION_STATE, &mConnectionFilter);
}

CAmClassActionDisconnect::~CAmClassActionDisconnect()
{
}

int CAmClassActionDisconnect::_execute(void)
{
    std::vector<CAmMainConnectionElement* >::iterator itListMainConnections;
    std::string sinkName;
    std::string sourceName;
    std::string connectionFilter;

    LOG_FN_ENTRY();

    if (false == mConnectionFilter.getParam(connectionFilter))
    {
        mConnectionFilter.setParam("CS_CONNECTED");
    }
    // Based on the parameter get the list of the connections on which action to be taken
    _findMainConnection();
    // Run through the exception lists and ignore the connections available in exception list
    _runExceptionList();
    LOG_FN_INFO("Number of connection to disconnect=", mpListMainConnections.size());

    // Finally from the list of the connections create the child actions

    IAmActionCommand* pAction(NULL);
    for (itListMainConnections = mpListMainConnections.begin();
                    itListMainConnections != mpListMainConnections.end(); ++itListMainConnections)
    {
        pAction = new CAmMainConnectionActionDisconnect(*itListMainConnections);
        if (NULL != pAction)
        {
            append(pAction);
        }

        if(mpClassElement->getLimitState()==LS_LIMITED)
        {
            pAction = _createActionSetLimitState(*itListMainConnections);
            if (NULL != pAction)
            {
                append(pAction);
            }
        }
    }
    return E_OK;
}

void CAmClassActionDisconnect::_findMainConnection(void)
{
    std::string sinkName;
    std::string sourceName;

    std::vector<CAmMainConnectionElement* > listTempMainConenctions;
    std::vector<CAmMainConnectionElement* >::iterator itListTempMainConnections;

    mSinkNameParam.getParam(sinkName);
    mSourceNameParam.getParam(sourceName);
    mpClassElement->getListMainConnections(listTempMainConenctions);
    LOG_FN_INFO("Number of connection in queue=", listTempMainConenctions.size());
    for (itListTempMainConnections = listTempMainConenctions.begin();
                    itListTempMainConnections != listTempMainConenctions.end();
                    ++itListTempMainConnections)
    {
        if ((sourceName == (*itListTempMainConnections)->getMainSourceName()) && (sinkName
                        == (*itListTempMainConnections)->getMainSinkName()))
        {
            /*
             * If explicit source and sink name are present then only one connection is
             * possible so we can break the loop.
             */
            mpListMainConnections.push_back(*itListTempMainConnections);
            break;
        }
        else if ((sourceName == (*itListTempMainConnections)->getMainSourceName()) && (sinkName
                        == ""))
        {
            mpListMainConnections.push_back(*itListTempMainConnections);
        }
        else if ((sinkName == (*itListTempMainConnections)->getMainSinkName()) && (sourceName == ""))
        {
            mpListMainConnections.push_back(*itListTempMainConnections);
        }
        else
        {
            if (false == _isConnectionFilter(*itListTempMainConnections))
            {
                continue;
            }
            /*
             * if no source or sink names are specified push all the connections
             */
            mpListMainConnections.push_back(*itListTempMainConnections);
        }
    }
}

bool CAmClassActionDisconnect::_isConnectionFilter(CAmMainConnectionElement* pMainConnection)
{

    bool retvalue = false;
    std::string connectionFilter;
    mConnectionFilter.getParam(connectionFilter);
    int state;
    pMainConnection->getState(state);
    LOG_FN_ENTRY(connectionFilter,state);
    if (connectionFilter == "ALL")
    {
        retvalue = true;
    }
    else if ((connectionFilter == "CS_CONNECTED") && (CS_CONNECTED == state))
    {
        retvalue = true;
    }
    else if ((connectionFilter == "CS_DISCONNECTED") && (CS_DISCONNECTED
                    == state))
    {
        return true;
    }
    else if ((connectionFilter == "OTHERS") && (CS_CONNECTED != state))
    {
        return true;
    }

    return retvalue;
}

bool CAmClassActionDisconnect::_isNameInExceptList(const std::vector<std::string >& listExceptions,
                                                   const std::string& exceptName)
{
    std::vector<std::string >::const_iterator itListExceptions;
    for (itListExceptions = listExceptions.begin(); itListExceptions != listExceptions.end();
                    ++itListExceptions)
    {
        if ((*itListExceptions) == exceptName)
        {
            return true;
        }
    }
    return false;
}

void CAmClassActionDisconnect::_runExceptionList(void)
{
    std::vector<std::string> listSinkExceptions;
    std::vector<std::string> listSourceExceptions;
    std::vector<CAmMainConnectionElement*> listTempMainConnections;
    std::vector<CAmMainConnectionElement*>::iterator itlistTempMainConnections;

    mlistSourceExceptions.getParam(listSourceExceptions);
    mlistSinkExceptions.getParam(listSinkExceptions);
    listTempMainConnections = mpListMainConnections;
    mpListMainConnections.clear();
    for (itlistTempMainConnections = listTempMainConnections.begin();
                    itlistTempMainConnections != listTempMainConnections.end();
                    ++itlistTempMainConnections)
    {
        if (!(_isNameInExceptList(listSourceExceptions,
                                  (*itlistTempMainConnections)->getMainSourceName())
              || _isNameInExceptList(listSinkExceptions,
                                     (*itlistTempMainConnections)->getMainSinkName())))
        {
            mpListMainConnections.push_back(*itlistTempMainConnections);
        }
    }
    return;
}

int CAmClassActionDisconnect::_update(const int result)
{
    std::vector<CAmMainConnectionElement*>::iterator itListMainConnections;
    int state;
    for (itListMainConnections = mpListMainConnections.begin();
                    itListMainConnections != mpListMainConnections.end();
                    ++itListMainConnections)
    {
        if ((*itListMainConnections) != NULL)
        {
            (*itListMainConnections)->getState(state);
            if(state == CS_DISCONNECTED)
            {
                mpClassElement->disposeConnection((*itListMainConnections)->getID());
                *itListMainConnections = NULL;
            }
        }
    }
    return E_OK;
}

IAmActionCommand* CAmClassActionDisconnect::_createActionSetLimitState(
                CAmMainConnectionElement* pMainConnection)
{
    IAmActionCommand* pAction = new CAmMainConnectionActionSetLimitState(pMainConnection);
    if (NULL != pAction)
    {
        CAmActionParam<gc_LimitType_e> limitTypeParam(LT_UNKNOWN);
        pAction->setParam(ACTION_PARAM_LIMIT_TYPE, &limitTypeParam);
        pAction->setUndoRequried(true);
    }
    return pAction;
}

} /* namespace gc */
} /* namespace am */

