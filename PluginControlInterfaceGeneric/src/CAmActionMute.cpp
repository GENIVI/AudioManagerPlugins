/******************************************************************************
 * @file CAmActionMute.cpp
 *
 * This class handles mute / unmute requests as defined in policy rules.
 *
 * @component AudioManager Generic Controller
 *
 * @author Martin Koch   <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2019 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#include "CAmActionMute.h"

#include "CAmClassElement.h"
#include "CAmMainConnectionElement.h"


namespace am {
namespace gc {


CAmActionMute::CAmActionMute(void)
    : CAmActionSetVolumeCore("CAmActionMute")
{
    _registerParam(ACTION_PARAM_MUTE_STATE, &mMuteStateParam);
    _registerParam(ACTION_PARAM_PATTERN, &mPatternParam);
}

CAmActionMute::~CAmActionMute()
{
    mUndoList.clear();
}

int CAmActionMute::_execute(void)
{
    am_MuteState_e requestedMuteState;
    if (!mMuteStateParam.getParam(requestedMuteState))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "No valid target mute state defined", mMuteStateParam.isSet());

        return E_OK;
    }

    _setupConnectionMap();

    _setupVolumeMatrix({CS_CONNECTED});

    shared_ptr<CAmRoutePointElement> pDirectElement;
    if (_isLowLevelRequest(pDirectElement))  // direct Sink- or Source-Mute Action
    {
        if (pDirectElement == nullptr)
        {
            return E_NOT_POSSIBLE;
        }

        _prepareLowLevelRequest(pDirectElement, requestedMuteState);
    }
    else
    {
        _prepareBalancedRequest(requestedMuteState);
    }

    // finally, create the actions to modify the routing-side elements
    for (auto &settings : mMatrix)
    {
        _createActionSetElementVolume(settings.first, settings.second);
    }

    return E_OK;
}

int CAmActionMute::_undo(void)
{
    uint32_t        pattern = DEFAULT_LIMIT_PATTERN;
    mPatternParam.getParam(pattern);

    for (const auto &undoItem : mUndoList)
    {
        undoItem.pElement->setLimitState(undoItem.state, undoItem.limit, pattern);
    }
    mUndoList.clear();

    return E_OK;
}

void CAmActionMute::_prepareBalancedRequest(am_MuteState_e requestedMuteState)
{
    std::string    className, connectionName;
    if (mClassNameParam.getParam(className))
    {
        _updateLimitState(CAmClassFactory::getElement(className), requestedMuteState);
    }
    else if (mConnectionNameParam.getParam(connectionName))
    {
        _updateLimitState(CAmClassFactory::getElementByConnection(connectionName), requestedMuteState);
    }
    else
    {
        for (auto &connectionInfo : mConnectionMap)
        {
            className = connectionInfo.first->getClassName();
            auto pClassElement = CAmClassFactory::getElement(className);
            _updateLimitState(pClassElement, requestedMuteState);
        }
    }

    // set target mute state
    for (auto &connectionInfo : mConnectionMap)
    {
        connectionInfo.second.target.muteState = requestedMuteState;
    }

    _calculateElementVolumes(SD_MAINSINK_TO_MAINSOURCE);
}

void CAmActionMute::_prepareLowLevelRequest(std::shared_ptr<CAmRoutePointElement> pDirectElement, am_MuteState_e requestedMuteState)
{
    if (pDirectElement)
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "setting mute state to", requestedMuteState
                , "for", pDirectElement->getType(), pDirectElement->getName());

        _updateLimitState(pDirectElement, requestedMuteState);

        if (0 == mMatrix.count(pDirectElement))
        {
            // insert new and initialize with current volume, limit and mute
            _getElementVolumes(pDirectElement, mMatrix[pDirectElement]);
        }

        // set target mute state
        mMatrix[pDirectElement].target.muteState = requestedMuteState;
    }
}

void CAmActionMute::_updateLimitState(const std::shared_ptr<CAmLimitableElement> &pElement, am_MuteState_e requestedMuteState)
{
    if (pElement == nullptr)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "INVALID element pointer!");
        return;
    }

    uint32_t        pattern = DEFAULT_LIMIT_PATTERN;
    gc_LimitState_e limitState = LS_UNLIMITED;
    am_volume_t     limitVolume = AM_VOLUME_NO_LIMIT;
    if (requestedMuteState == MS_MUTED)
    {
        limitVolume = AM_MUTE;
        limitState = LS_LIMITED;
    }
    mPatternParam.getParam(pattern);

    gc_LimitVolume_s prev;
    mUndoList.push_front({pElement, pElement->getLimit(pattern, prev), prev});

    pElement->setLimitState(limitState, {LT_ABSOLUTE, limitVolume}, pattern);
}


} /* namespace gc */
} /* namespace am */
