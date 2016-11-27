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

#include "CAmLogger.h"
#include "CAmMainConnectionActionSuspend.h"
#include "CAmMainConnectionActionSetLimitState.h"
#include "CAmClassElement.h"
#include "CAmClassActionSuspend.h"

namespace am {
namespace gc {

CAmClassActionSuspend::CAmClassActionSuspend(CAmClassElement *pClassElement) :
                                CAmActionContainer(std::string("CAmClassActionSuspend")),
                                mpClassElement(pClassElement)
{
    this->_registerParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    this->_registerParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
}

CAmClassActionSuspend::~CAmClassActionSuspend(void)
{
}

int CAmClassActionSuspend::_execute(void)
{
    std::string sourceName;
    std::string sinkName;
    mSourceNameParam.getParam(sourceName);
    mSinkNameParam.getParam(sinkName);
    std::vector<CAmMainConnectionElement* > listMainConnections;
    std::vector<CAmMainConnectionElement* >::iterator itListMainConnections;
    std::vector < am_ConnectionState_e > listConnectionStates {CS_CONNECTED};
    CAmConnectionListFilter filterObject;
    filterObject.setSourceName(sourceName);
    filterObject.setSinkName(sinkName);
    filterObject.setListConnectionStates(listConnectionStates);
    mpClassElement->getListMainConnections(listMainConnections,filterObject);
    for (itListMainConnections = listMainConnections.begin();
                    itListMainConnections != listMainConnections.end(); itListMainConnections++)
    {
        IAmActionCommand* pAction = new CAmMainConnectionActionSuspend(*itListMainConnections);
        if (NULL != pAction)
        {
            pAction->setUndoRequried(true);
            append(pAction);
        }

        pAction = _createActionSetLimitState(*itListMainConnections);
        if (NULL != pAction)
        {
            append(pAction);
        }
    }
    return E_OK;
}

IAmActionCommand* CAmClassActionSuspend::_createActionSetLimitState(
                CAmMainConnectionElement* pMainConnection)
{
    IAmActionCommand* pAction = new CAmMainConnectionActionSetLimitState(pMainConnection);
    if (NULL != pAction)
    {
        CAmActionParam < gc_LimitType_e > limitTypeParam(LT_UNKNOWN);
        pAction->setParam(ACTION_PARAM_LIMIT_STATE, &limitTypeParam);
        pAction->setUndoRequried(true);
    }
    return pAction;
}

} /* namespace gc */
} /* namespace am */
