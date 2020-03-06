/******************************************************************************
 * @file: CAmSourceActionSetState.cpp
 *
 * This file contains the definition of router action set source state class
 * (member functions and data members) used to implement the logic of setting
 * the source state at router level
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

#include "CAmSourceActionSetState.h"
#include "CAmSourceElement.h"
#include "CAmHandleStore.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmSourceActionSetState::CAmSourceActionSetState(std::shared_ptr<CAmSourceElement > pSourceElement)
    : CAmActionCommand(std::string("CAmSourceActionSetState"))
    , mpSourceElement(pSourceElement)
    , mHandle({H_UNKNOWN, 0})
    , mState(SS_UNKNNOWN)
    , mOldState(SS_UNKNNOWN)
{
    this->_registerParam(ACTION_PARAM_SOURCE_STATE, &mSourceStateParam);
}

CAmSourceActionSetState::~CAmSourceActionSetState()
{
}

int CAmSourceActionSetState::_execute(void)
{
    if ((nullptr == mpSourceElement) || (false == mSourceStateParam.getParam(mState)))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " Parameters not set");
        // set the error to invoke undo process
        return E_NOT_POSSIBLE;
    }

    mOldState = mpSourceElement->getState();
    LOG_FN_INFO(__FILENAME__, __func__, mpSourceElement->getName(), mOldState, "-->", mState);

    if (mOldState == mState)
    {
        return E_OK;
    }

    IAmControlReceive *pControlReceive = mpSourceElement->getControlReceive();
    am_Error_e result = pControlReceive->setSourceState(mHandle, mpSourceElement->getID(), mState);
    if (E_OK != result)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "  Failed to set source state:", result);
        // set the error to invoke undo process
        return result;
    }

    // register the observer
    CAmHandleStore::instance().saveHandle(mHandle, this);

    return E_WAIT_FOR_CHILD_COMPLETION;
}

int CAmSourceActionSetState::_undo(void)
{
    IAmControlReceive *pControlReceive = mpSourceElement->getControlReceive();
    am_Error_e result = pControlReceive->setSourceState(mHandle, mpSourceElement->getID(), mOldState);
    if (E_OK != result)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "  Failed to set source state:", result);
        return result;
    }

    // register the observer
    CAmHandleStore::instance().saveHandle(mHandle, this);

    return E_WAIT_FOR_CHILD_COMPLETION;
}

int CAmSourceActionSetState::_update(const int /* result */)
{
    // unregister the observer
    CAmHandleStore::instance().clearHandle(mHandle);

    LOG_FN_DEBUG(__FILENAME__, __func__, "source shared count ", mpSourceElement.use_count());
    return E_OK;
}

void CAmSourceActionSetState::_timeout(void)
{
    mpSourceElement->getControlReceive()->abortAction(mHandle);
}

} /* namespace gc */
} /* namespace am */
