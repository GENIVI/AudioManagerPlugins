/******************************************************************************
 * @file: CAmMainConnectionActionSuspend.cpp
 *
 * This file contains the definition of main connection action suspend class
 * (member functions and data members) used to implement the logic of suspend
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

#include "CAmMainConnectionActionSuspend.h"
#include "CAmSourceActionSetState.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmMainConnectionActionSuspend::CAmMainConnectionActionSuspend(
                CAmMainConnectionElement* pMainConnection) :
                                CAmActionContainer(std::string("CAmMainConnectionActionSuspend")),
                                mpMainConnection(pMainConnection)

{
}

CAmMainConnectionActionSuspend::~CAmMainConnectionActionSuspend()
{
}

int CAmMainConnectionActionSuspend::_execute(void)
{
    if (NULL == mpMainConnection)
    {
        LOG_FN_ERROR("  Parameter not set");
        return E_NOT_POSSIBLE;;
    }
    int state;
    mpMainConnection->getState(state);
    if (state == CS_SUSPENDED)
    {
        return E_OK; // already suspended
    }

    IAmActionCommand* pAction = _createActionSetSourceState(mpMainConnection,
                                                            SS_PAUSED);
    if (NULL != pAction)
    {
        append(pAction);
    }
    return E_OK;
}

int CAmMainConnectionActionSuspend::_update(const int result)
{
    if (AS_COMPLETED == getStatus())
    {
        mpMainConnection->updateState();
    }
    return E_OK;
}

IAmActionCommand* CAmMainConnectionActionSuspend::_createActionSetSourceState(CAmMainConnectionElement *pMainConnection,
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
