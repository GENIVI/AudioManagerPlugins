/******************************************************************************
 * @file CAmActionLimit.cpp
 *
 * This class handles limit / unlimit requests as defined in policy rules.
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

#include "CAmActionLimit.h"

#include "CAmClassElement.h"
#include "CAmMainConnectionElement.h"

namespace am {
namespace gc {


CAmActionLimit::CAmActionLimit(void)
    : CAmActionSetVolumeCore("CAmActionLimit")
{
    _registerParam(ACTION_PARAM_LIMIT_STATE, &mLimitStateParam);
    _registerParam(ACTION_PARAM_PATTERN,     &mPatternParam);
    _registerParam(ACTION_PARAM_VOLUME,      &mVolumeParam);
    _registerParam(ACTION_PARAM_VOLUME_STEP, &mVolumeStepParam);

}

CAmActionLimit::~CAmActionLimit()
{
    
}

int CAmActionLimit::_execute(void)
{
    _setupConnectionMap();

    _applyLimit();

    _setupVolumeMatrix({CS_CONNECTED, CS_UNKNOWN});

    std::shared_ptr<CAmRoutePointElement> pDirectElement;
    if (_isLowLevelRequest(pDirectElement))
    {
        _prepareLowLevelRequest(pDirectElement);
    }
    else
    {
        _calculateElementVolumes(SD_MAINSOURCE_TO_MAINSINK);
    }

    // finally, create the actions to modify the routing-side elements
    for (auto &settings : mMatrix)
    {
        _createActionSetElementVolume(settings.first, settings.second);
    }

    return E_OK;
}

int CAmActionLimit::_undo(void)
{
    uint32_t         pattern = 0xFFFFFFFF;
    mPatternParam.getParam(pattern);

    for (const auto &undoItem : mUndoList)
    {
        LOG_FN_WARN(__FILENAME__, __func__, undoItem.state, "for pattern", pattern
                , undoItem.limit.limitType, undoItem.limit.limitVolume
                , undoItem.pElement->getType(), undoItem.pElement->getName());

        undoItem.pElement->setLimitState(undoItem.state, undoItem.limit);
    }
    mUndoList.clear();

    return E_OK;
}

void CAmActionLimit::_applyLimit(void)
{
    // parse requested limit state from parameter set and update class element
    // as well as related main connection elements

    gc_LimitState_e  requestedLimitState;
    std::string      className, sourceName, sinkName, connectionName;
    uint32_t         pattern = 0xFFFFFFFF;
    gc_LimitVolume_s limitVolume = {LT_UNKNOWN, 0};

    mLimitStateParam.getParam(requestedLimitState);
    mClassNameParam.getParam(className);
    mSourceNameParam.getParam(sourceName);
    mSinkNameParam.getParam(sinkName);
    mPatternParam.getParam(pattern);

    // extract requested limit configuration and apply to involved class element
    if (!mLimitStateParam.isSet()
            || (requestedLimitState != LS_LIMITED && requestedLimitState != LS_UNLIMITED))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "mandatory parameter", ACTION_PARAM_LIMIT_STATE, "state NOT valid");
        return;
    }
    if (requestedLimitState == LS_LIMITED)
    {
        if (mVolumeParam.getParam((limitVolume.limitVolume)))
        {
            limitVolume.limitType = LT_ABSOLUTE;
        }
        else if (mVolumeStepParam.getParam((limitVolume.limitVolume)))
        {
            limitVolume.limitType = LT_RELATIVE;
        }
    }
    LOG_FN_DEBUG(__FILENAME__, __func__, requestedLimitState, "class name:", className, "limit:{"
            , limitVolume.limitType, limitVolume.limitVolume, "}");

    auto pClassElement = CAmClassFactory::getElement(className);
    if ((pClassElement == nullptr) && mConnectionNameParam.getParam(connectionName))
    {
        pClassElement = CAmClassFactory::getElementByConnection(connectionName);
    }

    gc_LimitVolume_s prev;
    auto pSourceElement = CAmSourceFactory::getElement(sourceName);
    auto pSinkElement = CAmSinkFactory::getElement(sinkName);
    if (pClassElement != nullptr)
    {
        mUndoList.push_front({pClassElement, pClassElement->getLimit(pattern, prev), prev});
        pClassElement->setLimitState(requestedLimitState, limitVolume, pattern);
    }
    else if (pSourceElement != nullptr)
    {
        mUndoList.push_front({pSourceElement, pSourceElement->getLimit(pattern, prev), prev});
        pSourceElement->setLimitState(requestedLimitState, limitVolume, pattern);
    }
    else if (pSinkElement != nullptr)
    {
        mUndoList.push_front({pSinkElement, pSinkElement->getLimit(pattern, prev), prev});
        pSinkElement->setLimitState(requestedLimitState, limitVolume, pattern);
    }
    else
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "class NOT found for name:", className);
        return;
    }

    // get back resulting limit map from class element and apply to involved connections
    for (auto &connectionInfo : mConnectionMap)
    {
        auto &pMainConnection = connectionInfo.first;
        if (pMainConnection != nullptr)
        {
            _getConnectionVolumes(pMainConnection, connectionInfo.second, true);
        }
    }
}

void CAmActionLimit::_prepareLowLevelRequest(const std::shared_ptr<CAmRoutePointElement> &pElement)
{
    // convert relative into absolute limits by comparing volume settings of all
    // involved connections
    _resolveRelativeLimits();

    if (pElement == nullptr)
    {
        return;
    }

    // make sure a column is present for requested element
    if (0 == mMatrix.count(pElement))
    {
        // insert new column with current settings
        _getElementVolumes(pElement, mMatrix[pElement]);
    }

    // clear and (re-)apply target limits if set for requested element
    std::list<gc_LimitVolume_s > limits;
    pElement->getLimits(limits);
    auto &column = mMatrix[pElement];
    column.target.muteState = MS_UNMUTED;
    column.target.offset = AM_VOLUME_NO_LIMIT;
    for (const auto &limit : limits)
    {
        // highest restriction wins if multiple limits are found
        if ((limit.limitType == LT_ABSOLUTE) && (limit.limitVolume == AM_MUTE))
        {
            column.target.muteState = MS_MUTED;
        }
        else if ((limit.limitType == LT_ABSOLUTE) && (limit.limitVolume < column.target.offset))
        {
            column.target.offset = limit.limitVolume;
        }
        else  // probably other limits were stronger
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, "limit", limit.limitType, limit.limitVolume
                    , "ignored for", pElement->getType(), pElement->getName());
            continue;  // skip below log entry
        }

        LOG_FN_DEBUG(__FILENAME__, __func__, "limit", limit.limitType, limit.limitVolume
                , "applied for", pElement->getType(), pElement->getName());
   }
}


} /* namespace gc */
} /* namespace am */

