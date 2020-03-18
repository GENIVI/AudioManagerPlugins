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

#include "CAmActionDisconnect.h"

#include "CAmLogger.h"
#include "CAmClassElement.h"
#include "CAmMainConnectionElement.h"
#include "CAmMainConnectionActionDisconnect.h"

namespace am {
namespace gc {

CAmActionDisconnect::CAmActionDisconnect()
    : CAmActionContainer(std::string("CAmActionDisconnect"))
{
    // initialize with default
    mListConnectionStatesParam.setParam({ CS_CONNECTED });

    _registerParam(ACTION_PARAM_CLASS_NAME, &mClassNameParam);
    _registerParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    _registerParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    _registerParam(ACTION_PARAM_CONNECTION_NAME, &mConnectionNameParam);
    _registerParam(ACTION_PARAM_EXCEPT_CLASS_NAME, &mListClassExceptionsParam);
    _registerParam(ACTION_PARAM_EXCEPT_SOURCE_NAME, &mlistSourceExceptionsParam);
    _registerParam(ACTION_PARAM_EXCEPT_SINK_NAME, &mListSinkExceptionsParam);
    _registerParam(ACTION_PARAM_CONNECTION_STATE, &mListConnectionStatesParam);
}

CAmActionDisconnect::~CAmActionDisconnect()
{
}

int CAmActionDisconnect::_execute(void)
{
    std::string connectionName;
    if (mConnectionNameParam.getParam(connectionName))
    {
        auto mainConnection = CAmMainConnectionFactory::getElement(connectionName);
        if (mainConnection)
        {
            mpListMainConnections.push_back(mainConnection);
        }
    }
    else
    {
        std::string                          className, sinkName, sourceName;
        std::vector < am_ConnectionState_e > listConnectionStates;
        std::vector < std::string >          listExceptClasses;
        std::vector < std::string >          listExceptSources;
        std::vector < std::string >          listExceptSinks;
        mClassNameParam.getParam(className);
        mSinkNameParam.getParam(sinkName);
        mSourceNameParam.getParam(sourceName);
        mListConnectionStatesParam.getParam(listConnectionStates);
        mListClassExceptionsParam.getParam(listExceptClasses);
        mlistSourceExceptionsParam.getParam(listExceptSources);
        mListSinkExceptionsParam.getParam(listExceptSinks);

        // Based on the parameter get the list of the connections on which action to be taken
        CAmConnectionListFilter filterObject;
        filterObject.setClassName(className);
        filterObject.setSourceName(sourceName);
        filterObject.setSinkName(sinkName);
        filterObject.setListConnectionStates(listConnectionStates);
        filterObject.setListExceptClassNames(listExceptClasses);
        filterObject.setListExceptSinkNames(listExceptSinks);
        filterObject.setListExceptSourceNames(listExceptSources);
        CAmMainConnectionFactory::getListElements(mpListMainConnections, filterObject);

        connectionName = sourceName, ":", sinkName;
    }

    // Finally from the list of the connections create the child actions
    for (auto &mainConnection : mpListMainConnections)
    {
        LOG_FN_INFO(__FILENAME__, __func__, connectionName, "selecting main connection"
            , mainConnection->getName(), "in current state =", mainConnection->getState());

        IAmActionCommand *pAction = new CAmMainConnectionActionDisconnect(mainConnection);
        if (NULL != pAction)
        {
            auto pClassElement = mainConnection->getClassElement();
            if (pClassElement)
            {
                CAmActionParam<gc_SetSourceStateDirection_e> setSourceStateDir;
                setSourceStateDir.setParam(classTypeToDisconnectDirectionLUT[pClassElement->getClassType()]);
                pAction->setParam(ACTION_PARAM_SET_SOURCE_STATE_DIRECTION, &setSourceStateDir);
            }
            append(pAction);
        }
    }

    if (mpListMainConnections.size() == 0)
    {
        LOG_FN_INFO(__FILENAME__, __func__, connectionName, "NO connections to disconnect");
    }

    return E_OK;
}

int CAmActionDisconnect::_update(const int result)
{
    std::vector<std::shared_ptr<CAmMainConnectionElement > >::iterator itListMainConnections;
    for (itListMainConnections = mpListMainConnections.begin();
         itListMainConnections != mpListMainConnections.end(); ++itListMainConnections)
    {
        if ((*itListMainConnections) != nullptr)
        {
            if ((*itListMainConnections)->permitsDispose())
            {
                auto pClassElement = (*itListMainConnections)->getClassElement();
                if (pClassElement)
                {
                    pClassElement->disposeConnection((*itListMainConnections));
                }
                *itListMainConnections = nullptr;
            }
        }
    }

    return E_OK;
}

} /* namespace gc */
} /* namespace am */
