/******************************************************************************
 * @file: CAmClassActionInterrupt.cpp
 *
 * This file contains the definition of user action interrupt class (member functions
 * and data members) used to implement the logic of pushing (disconnect) the connection
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

#include "CAmActionInterrupt.h"

#include "CAmClassElement.h"
#include "CAmMainConnectionElement.h"
#include "CAmLogger.h"
#include "CAmMainConnectionActionDisconnect.h"
#include "CAmActionLimit.h"

namespace am {
namespace gc {

CAmActionInterrupt::CAmActionInterrupt()
    : CAmActionContainer(std::string("CAmActionInterrupt"))
{
    _registerParam(ACTION_PARAM_CLASS_NAME, &mClassNameParam);
    _registerParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    _registerParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    _registerParam(ACTION_PARAM_EXCEPT_CLASS_NAME, &mListClassExceptionsParam);
}

CAmActionInterrupt::~CAmActionInterrupt()
{
}

int CAmActionInterrupt::_execute(void)
{
    std::string                                              className, sourceName, sinkName;
    std::vector < std::string >                              listExceptClasses;
    mClassNameParam.getParam(className);
    mSourceNameParam.getParam(sourceName);
    mSinkNameParam.getParam(sinkName);
    mListClassExceptionsParam.getParam(listExceptClasses);
    std::vector<std::shared_ptr<CAmMainConnectionElement > > listMainConnections;
    std::vector < am_ConnectionState_e >                     listConnectionStates { CS_CONNECTED, CS_SUSPENDED };
    CAmConnectionListFilter                                  filterObject;
    filterObject.setSinkName(sinkName);
    filterObject.setSourceName(sourceName);
    filterObject.setClassName(className);
    filterObject.setListExceptClassNames(listExceptClasses);
    filterObject.setListConnectionStates(listConnectionStates);
    CAmMainConnectionFactory::getListElements(listMainConnections, filterObject);
    if ((false == sourceName.empty()) && (false == sinkName.empty())
        && (true == listMainConnections.empty()))
    {
        auto pClassElement = CAmClassFactory::getElement(sourceName, sinkName);
        if (pClassElement == nullptr)
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "NO class found for source:", sourceName
                    , "and sink:", sinkName);
            return E_NOT_POSSIBLE;
        }
        else if (!listExceptClasses.empty()
                && CAmConnectionListFilter::matchesAny(listExceptClasses, pClassElement->getName()))
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, "class candidate", pClassElement->getName(), "for source"
                    , sourceName, "and sink", sinkName, "explicitly excluded");
            return E_OK;
        }

        am_mainConnectionID_t mainConnectionID;
        am_Error_e            result;
        result = pClassElement->createMainConnection(sourceName, sinkName, mainConnectionID);
        if (result == E_OK)
        {
            // The connection state should be changed to disconnected.
            std::shared_ptr<CAmMainConnectionElement > pMainConnection = nullptr;
            pMainConnection = CAmMainConnectionFactory::getElement(mainConnectionID);
            if (pMainConnection != nullptr)
            {
                pMainConnection->setState(CS_DISCONNECTED);
            }
        }

        return result;
    }

    std::vector<std::shared_ptr<CAmMainConnectionElement > >::iterator itListMainConnections;
    for (itListMainConnections = listMainConnections.begin();
         itListMainConnections != listMainConnections.end(); ++itListMainConnections)
    {
        CAmMainConnectionFactory::moveToEnd((*itListMainConnections)->getName(), O_NEWEST);

        IAmActionCommand *pAction = new CAmMainConnectionActionDisconnect(*itListMainConnections);
        if (NULL != pAction)
        {
            auto pClassElement = (*itListMainConnections)->getClassElement();
            if (pClassElement)
            {
                pClassElement->updateMainConnectionQueue();

                if (pClassElement->getClassType() == C_PLAYBACK)  // non-default direction
                {
                    CAmActionParam<gc_SetSourceStateDirection_e > setSourceStateDir(SD_MAINSOURCE_TO_MAINSINK);
                    pAction->setParam(ACTION_PARAM_SET_SOURCE_STATE_DIRECTION, &setSourceStateDir);
                }
            }
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

IAmActionCommand *CAmActionInterrupt::_createActionSetLimitState(
    std::shared_ptr<CAmMainConnectionElement > pMainConnection)
{
    IAmActionCommand *pAction = new CAmActionLimit();
    if (NULL != pAction)
    {
        CAmActionParam<std::string> connectionNameParam(pMainConnection->getName());
        pAction->setParam(ACTION_PARAM_CONNECTION_NAME, &connectionNameParam);

        CAmActionParam<std::map<uint32_t, gc_LimitVolume_s > > mapLimitsParam;
        pAction->setParam(ACTION_PARAM_LIMIT_MAP, &mapLimitsParam);
        pAction->setUndoRequried(true);
    }

    return pAction;
}

} /* namespace gc */
} /* namespace am */
