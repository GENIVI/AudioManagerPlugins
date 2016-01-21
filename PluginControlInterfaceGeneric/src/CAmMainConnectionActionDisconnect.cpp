/******************************************************************************
 * @file: CAmMainConenctionActionDisconnect.cpp
 *
 * This file contains the definition of main connection action disconnect class
 * (member functions and data members) used to implement the logic of disconnect
 * at main connection level
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

#include "CAmMainConnectionActionDisconnect.h"
#include "CAmMainConnectionElement.h"
#include "CAmSourceActionSetState.h"
#include "CAmRouteActionDisconnect.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmMainConnectionActionDisconnect::CAmMainConnectionActionDisconnect(CAmMainConnectionElement* pMainConnection) :
                CAmActionContainer(std::string("CAmMainConnectionActionDisconnect")),
                mpMainConnection(pMainConnection),
                mActionCompleted(false)
{
}

CAmMainConnectionActionDisconnect::~CAmMainConnectionActionDisconnect()
{
}

int CAmMainConnectionActionDisconnect::_execute(void)
{
    if (NULL == mpMainConnection)
    {
        LOG_FN_ERROR("  Parameters not set.");
        return E_NOT_POSSIBLE;
    }

    IAmActionCommand* pAction = _createActionSetSourceState(mpMainConnection,
                                                            SS_OFF);
    if (NULL != pAction)
    {
        append(pAction);
    }

    //get route elements from main connection class
    std::vector<CAmRouteElement*> listRouteElements;
    std::vector<CAmRouteElement*>::iterator itListRouteElements;
    mpMainConnection->getListRouteElements(listRouteElements);
    for (itListRouteElements = listRouteElements.begin();
                    itListRouteElements != listRouteElements.end();
                    ++itListRouteElements)
    {

        // create router disconnect action for each element
        IAmActionCommand* pAction = new CAmRouteActionDisconnect(*itListRouteElements);
        if (NULL != pAction)
        {
            // add the newly created route disconnect action to dynamic action
            pAction->setUndoRequried(getUndoRequired());
            append(pAction);
        }
    }
    // update main connection state in Audio Manager daemon
    mpMainConnection->setState(CS_DISCONNECTING);
    return E_OK;
}

int CAmMainConnectionActionDisconnect::_undo(void)
{
    // update main connection state in Audio Manager daemon
    mpMainConnection->setState(CS_CONNECTING);
    return E_OK;
}

int CAmMainConnectionActionDisconnect::_update(const int result)
{
    if (AS_COMPLETED == getStatus())
    {
        if (E_OK == result)
        {
            mActionCompleted = true;
        }
        mpMainConnection->updateState();
    }
    else if (AS_UNDO_COMPLETE == getStatus())
    {
        if (mActionCompleted == false)
        {
            mpMainConnection->updateState();
        }
    }
    return E_OK;
}

IAmActionCommand* CAmMainConnectionActionDisconnect::_createActionSetSourceState(CAmMainConnectionElement *pMainConnection,
                                                                                 const am_SourceState_e sourceState)
{
    IAmActionCommand* pAction(NULL);
    CAmSourceElement* pMainSource = pMainConnection->getMainSource();
    if (pMainSource != NULL)
    {
        pAction = new CAmSourceActionSetState(pMainSource);
        if (pAction != NULL)
        {
            CAmActionParam<am_SourceState_e> sourceStateParam(sourceState);
            pAction->setParam(ACTION_PARAM_SOURCE_STATE, &sourceStateParam);
        }
    }
    return pAction;
}

} /* namespace gc */
} /* namespace am */
