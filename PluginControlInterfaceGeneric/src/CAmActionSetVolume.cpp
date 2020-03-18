/******************************************************************************
 * @file: CAmActionSetVolume.cpp
 *
 * This file contains the definition of user action set volume class
 * (member functions and data members) used to implement the logic of setting
 * the volume of element at user level
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

#include "CAmActionSetVolume.h"
#include "CAmClassElement.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmMainConnectionElement.h"
#include "CAmLogger.h"


namespace am {
namespace gc {


CAmActionSetVolume::CAmActionSetVolume()
    : CAmActionSetVolumeCore("CAmActionSetVolume")
{
    _registerParam(ACTION_PARAM_MAIN_VOLUME,      &mMainVolumeParam);
    _registerParam(ACTION_PARAM_MAIN_VOLUME_STEP, &mMainVolumeStepParam);
    _registerParam(ACTION_PARAM_VOLUME,           &mVolumeParam);
    _registerParam(ACTION_PARAM_VOLUME_STEP,      &mVolumeStepParam);

}

CAmActionSetVolume::~CAmActionSetVolume()
{

}

int CAmActionSetVolume::_execute(void)
{
    // determine addressed main connections and populate member array
    _setupConnectionMap();

    bool isInternalVolumeSpecified = (mVolumeParam.isSet() || mVolumeStepParam.isSet())
            && !(mMainVolumeParam.isSet() || mMainVolumeStepParam.isSet());

    // prepare volume matrix, either direct at requested element (low level)
    // or balanced over found connections (high level)
    std::shared_ptr<CAmRoutePointElement> pDirectElement;
    if (    (isInternalVolumeSpecified || mConnectionMap.empty())
         && _isLowLevelRequest(pDirectElement))
    {
        _prepareLowLevelRequest(pDirectElement);
    }
    else
    {
        _prepareBalancedRequest();
    }

    // finally, create the actions to modify the routing-side elements
    for (auto & settings : mMatrix)
    {
        _createActionSetElementVolume(settings.first, settings.second);
    }

    return E_OK;
}

void CAmActionSetVolume::_prepareBalancedRequest(void)
{
    for (auto &settings : mConnectionMap)
    {
        am_volume_t requestedVolume = _getRequestedVolume(settings.first);

        if ((mConnectionMap.size() == 1) || (requestedVolume < settings.second.target.offset))
        {
            settings.second.target.volume = requestedVolume;
        }

        if (mMainVolumeParam.isSet() || mMainVolumeStepParam.isSet())
        {
            _compensateMainVolumeChange(settings);
        }
    }

    _setupVolumeMatrix({CS_CONNECTED});

    _calculateElementVolumes(SD_MAINSINK_TO_MAINSOURCE);
}

void CAmActionSetVolume::_prepareLowLevelRequest(std::shared_ptr<CAmRoutePointElement> &pElement)
{
    if (pElement && pElement->getVolumeSupport())
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "for", pElement->getType(), pElement->getName());

        MatrixColumn_t &settings = mMatrix[pElement];
        _getElementVolumes(pElement, settings);

        // check request against specified range
        am_volume_t requested = _getRequestedVolume(pElement);
        if ((requested > pElement->getMaxVolume()) || (requested < pElement->getMinVolume()))
        {
            am_volume_t clipped = std::max(std::min(requested, pElement->getMaxVolume()), pElement->getMinVolume());
            LOG_FN_WARN(__FILENAME__, __func__, "NOT allowed to set volume for"
                    , pElement->getType(), pElement->getName(), "to", requested, " - clipping to", clipped);
            requested = clipped;
        }

        am_volume_t diff      = requested - settings.actual.volume;
        settings.target.volume += diff;

        // re-calculate resulting volume for involved connections
        for (const auto &affectedConnection : settings.connections)
        {
            mConnectionMap[affectedConnection].target.volume += diff;
        }
    }
}

am_volume_t CAmActionSetVolume::_getRequestedVolume(const std::shared_ptr<CAmElement > &pElement)
{
    /*
     * Get the volume from given parameters (prefer main volume over volume and volume over step volume)
     * 1. In case main Volume/ step then use the main sink mapping table to convert to
     * volume.
     * 2. In case no volume parameter set then restore the last connection volume
     */

    am_volume_t     requestedVolume     = 0;
    gc_Element_e type = pElement->getType();
    switch (type)
    {
        case ET_SOURCE:
        case ET_SINK:
        case ET_CONNECTION:
            break;
        default:
            LOG_FN_ERROR(__FILENAME__, __func__, "NOT supported for", type, pElement->getName());
            return requestedVolume;
    }

    std::shared_ptr<CAmSinkElement> pSink;
    if (type == ET_SINK)
    {
        pSink = std::static_pointer_cast<CAmSinkElement>(pElement);
    }
    else if (type == ET_CONNECTION)
    {
        pSink = (std::static_pointer_cast<CAmMainConnectionElement>(pElement))->getMainSink();
    }
    else if (mMainVolumeParam.isSet() || mMainVolumeStepParam.isSet())
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "cannot handle main volume parameters for", type, pElement->getName());

        // invalid parameters - fall back to mute
        return AM_MUTE;
    }

    am_mainVolume_t requestedMainVolume = 0;
    if (mMainVolumeParam.getParam(requestedMainVolume))
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "requested main Volume =", requestedMainVolume);

        requestedVolume = pSink->convertMainVolumeToVolume(requestedMainVolume);
    }
    else if(mMainVolumeStepParam.getParam(requestedMainVolume))
    {
        // only step is given - add current volume
        switch (type)
        {
            case ET_SINK:
                {
                    am_mainVolume_t currentMainVolume = pSink->getMainVolume();
                    LOG_FN_DEBUG(__FILENAME__, __func__, "adding requested step", requestedMainVolume
                            , "to current main volume", currentMainVolume);

                    requestedVolume = pSink->convertMainVolumeToVolume(currentMainVolume + requestedMainVolume);
                }
                break;

            case ET_CONNECTION:
                {
                    const auto & pConnection = std::static_pointer_cast<CAmMainConnectionElement>(pElement);
                    am_mainVolume_t currentMainVolume = pConnection->getMainVolume();
                    LOG_FN_DEBUG(__FILENAME__, __func__, "adding requested step", requestedMainVolume
                            , "to current main volume", currentMainVolume);

                    requestedVolume = pSink->convertMainVolumeToVolume(currentMainVolume + requestedMainVolume);
                }
                break;

            default:
                break;
        }
    }
    else if (mVolumeParam.getParam(requestedVolume))
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "requested Volume =", requestedVolume);
    }
    else if(mVolumeStepParam.getParam(requestedVolume))
    {
        am_volume_t currentVolume = _getCurrentVolume(pElement);
        LOG_FN_DEBUG(__FILENAME__, __func__, " adding requested step", requestedVolume
                , "to current volume", currentVolume);
        requestedVolume += currentVolume;
    }
    else
    {
        requestedVolume = _getCurrentVolume(pElement);

        LOG_FN_DEBUG(__FILENAME__, __func__, "restoring last Volume:", requestedVolume);
    }

    return requestedVolume;
}

am_volume_t CAmActionSetVolume::_getCurrentVolume(const std::shared_ptr<CAmElement > &pElement)
{
    switch (pElement->getType())
    {
        case ET_SINK:
        case ET_SOURCE:
            return (std::static_pointer_cast<CAmRoutePointElement>(pElement))->getVolume();

        case ET_CONNECTION:
            return (std::static_pointer_cast<CAmMainConnectionElement>(pElement))->getVolume();

        default:
            LOG_FN_ERROR(__FILENAME__, __func__, "Invalid element type", pElement->getType());
            return AM_MUTE;
    }
}

void CAmActionSetVolume::_compensateMainVolumeChange(ConnectionMapItem_t &connectionInfo)
{
    ConnectionInfo_t &changeInfo = connectionInfo.second;
    am_volume_t volumeDiff = changeInfo.target.volume - changeInfo.actual.volume;
    am_volume_t offsetDiff = changeInfo.target.offset - changeInfo.actual.offset;

    if ((volumeDiff != 0) && (changeInfo.target.offset < AM_VOLUME_NO_LIMIT))
    {
        offsetDiff -= volumeDiff;
        changeInfo.target.offset = std::min(AM_VOLUME_NO_LIMIT, changeInfo.target.offset + offsetDiff);

        LOG_FN_DEBUG(__FILENAME__, __func__, connectionInfo.first->getName(), changeInfo);
    }
}

int CAmActionSetVolume::_update(int result)
{
    if (result != E_OK)
    {
        LOG_FN_WARN(__FILENAME__, __func__, "unexpected result", static_cast<am_Error_e>(result));
        return 0;
    }

    std::string     sinkName("");
    am_mainVolume_t mainVolume = 0;
    if (mConnectionMap.empty())
    {
        mSinkNameParam.getParam(sinkName);
        std::shared_ptr<CAmSinkElement > pSink = CAmSinkFactory::getElement(sinkName);
        if (pSink != nullptr)
        {
            mainVolume = pSink->convertVolumeToMainVolume(pSink->getVolume());
            pSink->setMainVolume(mainVolume);

            auto pClassElement = CAmClassFactory::getElement(pSink->getClassID());
            pClassElement->setLastVolume(NULL, sinkName, mainVolume);
        }
    }

    for (auto &settings : mConnectionMap)
    {
        const auto &changes = settings.second;
        if ((changes.target.volume != changes.actual.volume) && (changes.target.muteState == MS_UNMUTED))
        {
            auto pMainConnection = settings.first;
            if (pMainConnection != nullptr)
            {
                pMainConnection->updateMainVolume();

                auto pClassElement = CAmClassFactory::getElement(pMainConnection->getMainSource()->getClassID());
                if (pClassElement != nullptr)
                {
                    pClassElement->setLastVolume(pMainConnection, "", changes.target.volume);
                }
            }
        }
    }

    return 0;
}

} /* namespace gc */
} /* namespace am */
