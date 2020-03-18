/**************************************************************************//**
 * @file CAmActionSetVolumeCore.h
 *
 * @brief Common base class for CAmActionSetVolume, CAmActionLimit and CAmActionMute
 *
 * @component AudioManager Generic Controller
 *
 * @author Martin Koch   <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2019 Advanced Driver Information Technology.
 *     This code is developed by Advanced Driver Information Technology.
 *     Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 *     All rights reserved.
 *
 *****************************************************************************/


#include "CAmActionSetVolumeCore.h"
#include "CAmMainConnectionElement.h"
#include "CAmHandleStore.h"


namespace am {
namespace gc {


CAmActionSetVolumeCore::CAmActionSetVolumeCore(const std::string &name)
    : CAmActionContainer(name)
{
    _registerParam(ACTION_PARAM_CLASS_NAME,      &mClassNameParam);
    _registerParam(ACTION_PARAM_SOURCE_NAME,     &mSourceNameParam);
    _registerParam(ACTION_PARAM_SINK_NAME,       &mSinkNameParam);
    _registerParam(ACTION_PARAM_CONNECTION_NAME, &mConnectionNameParam);

    _registerParam(ACTION_PARAM_RAMP_TIME,       &mRampTimeParam);
    _registerParam(ACTION_PARAM_RAMP_TYPE,       &mRampTypeParam);
}

CAmActionSetVolumeCore::~CAmActionSetVolumeCore()
{

}

void CAmActionSetVolumeCore::_setupConnectionMap(void)
{
    std::string                                className, connectionName, sourceName, sinkName;
    mClassNameParam.getParam(className);
    mSinkNameParam.getParam(sinkName);
    mSourceNameParam.getParam(sourceName);
    mConnectionNameParam.getParam(connectionName);

    std::vector<std::shared_ptr<CAmMainConnectionElement > > listMainConnections;
    if (mConnectionNameParam.isSet())
    {
        listMainConnections.push_back(CAmMainConnectionFactory::getElement(connectionName));
    }
    else if (mSourceNameParam.isSet() && mSinkNameParam.isSet())
    {
        listMainConnections.push_back(CAmMainConnectionFactory::getElement(sourceName + ":" + sinkName));
    }
    else
    {
        CAmConnectionListFilter filter;
        filter.setClassName(className);
        filter.setSinkName(sinkName);
        filter.setListConnectionStates({ CS_CONNECTED });
        CAmMainConnectionFactory::getListElements(listMainConnections, filter);
        if (listMainConnections.empty())
        {
            // no active connection found - so lookout for inactive connections
            filter.setListConnectionStates({ CS_DISCONNECTED, CS_SUSPENDED });
            CAmMainConnectionFactory::getListElements(listMainConnections, filter);
        }
    }

    if (listMainConnections.empty())
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "listMainConnections is empty");
    }
    for (auto &pMainConnection : listMainConnections)
    {
        if (pMainConnection != nullptr)
        {
            auto &settings = mConnectionMap[pMainConnection];
            _getConnectionVolumes(pMainConnection, settings, true);

            LOG_FN_DEBUG(__FILENAME__, __func__, "found", pMainConnection->getName()
                    , "with settings.actual:", settings.actual);
        }
    }
}

bool CAmActionSetVolumeCore::_qualifiesForChanges(const std::shared_ptr<CAmRoutePointElement > &pElement)
{
    // check if given element is selectable for the intended volume changes 

    auto         &column        = mMatrix[pElement];
    ChangeInfo_t &settings      = column;
    auto         &connections   = column.connections;
    if (connections.empty() || (mConnectionMap.count(*connections.begin()) == 0))
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "skipping", pElement->getType(), pElement->getName()
                , ":", settings.actual, "due to no connections;");
        return false;
    }
    else if (connections.size() > 1)   // suppress using a shared element
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "skipping shared ", pElement->getType(), pElement->getName()
                , ":", settings.actual, "");
        return false;
    }
    for (const auto& connection : connections)
    {
        if (!(mConnectionMap[connection].adjustable))
        {
            return false;  // do not alter this element as it is part of an invariant connection
        }
    }

    return true;
}

bool CAmActionSetVolumeCore::_isLowLevelRequest(std::shared_ptr<CAmRoutePointElement> &pElement)
{
    // check conditions
    if (    mConnectionNameParam.isSet() || mClassNameParam.isSet() // high-level target
         || (mSourceNameParam.isSet() == mSinkNameParam.isSet()))   // both either set or empty
    {
        // not a direct operation
        return false;
    }

    // find target element
    std::string  elementName;
    gc_Element_e elementType = ET_UNKNOWN;
    if (mSourceNameParam.getParam(elementName))
    {
        pElement = CAmSourceFactory::getElement(elementName);
        elementType = ET_SOURCE;
    }
    else if (mSinkNameParam.getParam(elementName))
    {
        pElement = CAmSinkFactory::getElement(elementName);
        elementType = ET_SINK;
    }

    // report problems
    if (pElement == nullptr)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, elementType, elementName, "NOT found");
    }
    else if (!pElement->getVolumeSupport())
    {
        LOG_FN_ERROR(__FILENAME__, __func__, elementType, elementName, "NOT supporting volume changes!");
    }

    // understand as direct request, regardless of possible prior problems
    return true;
}

void CAmActionSetVolumeCore::_calculateElementVolumes(gc_SetSourceStateDirection_e direction)
{
    if (mConnectionMap.empty())
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "mConnectionMap is empty");
        return;
    }

    // convert relative into absolute limits by comparing volume settings of all
    // involved connections
    _resolveRelativeLimits();

    // copy matrix columns to a list sortable by ordinal
    std::list<Matrix_t::iterator> elementList;
    for (Matrix_t::iterator it = mMatrix.begin(); it != mMatrix.end(); ++it)
    {
        elementList.push_back(it);
    }

    // local sorting decision function
    auto precedes = [&] (const Matrix_t::iterator &lhs, const Matrix_t::iterator &rhs) -> bool
    {
        switch (direction)
        {
            case SD_MAINSOURCE_TO_MAINSINK:
                return lhs->second.ordinal > rhs->second.ordinal;
            default:
                return lhs->second.ordinal < rhs->second.ordinal;
        }
    };

    elementList.sort(precedes);

    // prepare main iteration
    std::map<std::shared_ptr<CAmMainConnectionElement >, VolumeInfo_t > diffMap;
    for (auto &connectionSettings : mConnectionMap)
    {
        const auto& pConnection = connectionSettings.first;
        diffMap[pConnection] = _getVolumeDiff(pConnection);
    }

    // local decision function whether all differences have vanished and calculation is completed
    static const auto nullDiff = VolumeInfo_t();
    auto done = [&] (void) -> bool
    {
        bool retVal = true;
        for (auto &item : diffMap)
        {
            retVal &= (item.second == nullDiff);
        }
        return retVal;;
    };

    // balance element volumes
    for (auto columnRef : elementList)
    {
        if (done())
        {
            return;
        }

        const auto &pElement = columnRef->first;
        auto &column = columnRef->second;
        if (_qualifiesForChanges(pElement))
        {
            VolumeInfo_t elementDiff;
            for (auto &pConnection : column.connections)
            {
                VolumeInfo_t &connectionDiff = diffMap[pConnection];
                if (connectionDiff.volume != 0)
                {
                    elementDiff.volume = connectionDiff.volume;
                }
                if (connectionDiff.offset != 0)
                {
                    elementDiff.offset = connectionDiff.offset;
                }
                if (connectionDiff.muteState != MS_UNKNOWN)
                {
                    elementDiff.muteState = connectionDiff.muteState;
                }
            }
            LOG_FN_DEBUG(__FILENAME__, __func__, column.ordinal, pElement->getType(), pElement->getName()
                    , "elementDiff", elementDiff);

            // apply calculated changes
            am_volume_t  elementMin    = pElement->getMinVolume();
            am_volume_t  elementMax    = pElement->getMaxVolume();
            column.target.volume = std::max(elementMin, std::min(elementMax, static_cast<am_volume_t >(column.actual.volume + elementDiff.volume)));
            column.target.offset = std::max(elementMin, std::min(elementMax, static_cast<am_volume_t >(column.actual.offset + elementDiff.offset)));
            column.target.muteState = (elementDiff.muteState != MS_UNKNOWN) ? elementDiff.muteState : column.target.muteState;

            // recalculate differences
            for (auto &pConnection : column.connections)
            {
                diffMap[pConnection] = _getVolumeDiff(pConnection);
            }
        }
    }

    for (auto &item : diffMap)
    {
        if (item.second != nullDiff)
        {
            LOG_FN_WARN(__FILENAME__, __func__, "could not reach requirement for connection"
                    , item.first->getName(), "- remaining difference:", item.second);
        }
    }
}

void CAmActionSetVolumeCore::_setupVolumeMatrix(const std::vector<am_ConnectionState_e > &listConnectionStates)
{
    // Populates a matrix with a dedicated column for every volume-change element.
    // Main connections used by each column are treated as (pseudo) rows.
    // The second parameter catches main connection elements which are not 
    // directly addressed by the action parameters, thus are not already
    // listed in member mConnectionMap. 

    unsigned nextOrdinal = 0;
    for (auto &addressedConnection : mConnectionMap)
    {
        auto pAddressedConnection = addressedConnection.first;
        std::list<std::shared_ptr<CAmRoutePointElement > > listElements;
        pAddressedConnection->getVolumeChangeElements(listElements, SD_MAINSINK_TO_MAINSOURCE);

        // check if this element is part of another connection
        for (auto pCandidate : listElements)
        {
            // setup column for element
            MatrixColumn_t &column = mMatrix[pCandidate];
            if (column.connections.empty())
            {
                column.ordinal = nextOrdinal;
            }
            else  // shared element
            {
                nextOrdinal = column.ordinal;
            }
            ++nextOrdinal;
            column.connections.insert(pAddressedConnection);
            _getElementVolumes(pCandidate, static_cast<ChangeInfo_t&>(column));

            // get list of all active connections using this element
            std::vector<std::shared_ptr<CAmMainConnectionElement > > listConnections;
            CAmConnectionListFilter filter;
            filter.setListConnectionStates(listConnectionStates);
            if(pCandidate->getType() == ET_SINK)
            {
                filter.setSinkName(pCandidate->getName());
            }
            else
            {
                filter.setSourceName(pCandidate->getName());
            }
            CAmMainConnectionFactory::getListElements(listConnections, filter);

            // setup volume information for main connections not directly addressed,
            // but linked through a shared source or sink
            for (auto &pAffectedConnection : listConnections)
            {
                // notice this connection as using element 
                column.connections.insert(pAffectedConnection);

                // append connection if not already listed
                if (mConnectionMap.count(pAffectedConnection) == 0)
                {
                    ConnectionInfo_t &changes = mConnectionMap[pAffectedConnection];
                    _getConnectionVolumes(pAffectedConnection, changes, false);
                }
            }
        }
    }

    // print out involved elements
    for (const auto& item : mMatrix)
    {
        const auto& pElement = item.first;
        const auto& settings = item.second;
        LOG_FN_DEBUG(__FILENAME__, __func__, "found", settings.ordinal, pElement->getType(), pElement->getName()
                , "actual:", settings.actual, "used in ", settings.connections.size(), "connections");
    }
}

void CAmActionSetVolumeCore::_getElementVolumes(const std::shared_ptr<CAmRoutePointElement > &pElement, ChangeInfo_t &changes)
{
    if (pElement != nullptr)
    {
        changes.actual.muteState = pElement->getMuteState();
        changes.actual.volume    = pElement->getVolume();
        changes.actual.offset    = pElement->getOffsetVolume();

        // initialize target with actual values
        changes.target           = changes.actual;
    }
}

CAmActionSetVolumeCore::VolumeInfo_t CAmActionSetVolumeCore::_getVolumeDiff(const std::shared_ptr<CAmMainConnectionElement > &pConnection)
{
    const ConnectionInfo_t &connectionInfo = mConnectionMap[pConnection];
    if (connectionInfo.adjustable == false)
    {
        return VolumeInfo_t();
    }

    VolumeInfo_t diff = connectionInfo.target;
    am_MuteState_e overallMute = MS_UNMUTED;

    for (const auto &column : mMatrix)
    {
        const auto &settings = column.second;
        if (settings.connections.count(pConnection) > 0) // element effective in this connection
        {
            diff.volume -= settings.target.volume;
            diff.offset -= settings.target.offset;
            if (settings.target.muteState == MS_MUTED)
            {
                overallMute = MS_MUTED;
            }
        }
    }

    // indicate a match in resultant mute state with MS_UNKNOWN
    if (overallMute == connectionInfo.target.muteState)
    {
        diff.muteState = MS_UNKNOWN;
    }

    LOG_FN_DEBUG(__FILENAME__, __func__, pConnection->getName(), diff);

    return diff;
}

void CAmActionSetVolumeCore::_getConnectionVolumes(const std::shared_ptr<CAmMainConnectionElement > &pConnection
        , ConnectionInfo_t &settings, bool adjustable)
{
    // obtain actual state
    settings.actual.muteState  = pConnection->getMuteState();
    settings.actual.volume     = pConnection->getVolume();
    settings.actual.offset     = pConnection->getOffsetVolume();

    // populate target with defaults
    settings.target.muteState  = MS_UNMUTED;
    settings.target.volume     = settings.actual.volume;
    settings.target.offset     = AM_VOLUME_NO_LIMIT;
    settings.relative          = AM_VOLUME_NO_LIMIT;
    settings.adjustable        = adjustable;

    // apply any limits if set for connection element
    std::list<gc_LimitVolume_s > limits;
    pConnection->getLimits(limits);
    for (const auto &limit : limits)
    {
        // highest restriction wins if multiple limits are found
        if ((limit.limitType == LT_ABSOLUTE) && (limit.limitVolume == AM_MUTE))
        {
            settings.target.muteState = MS_MUTED;
        }
        else if ((limit.limitType == LT_ABSOLUTE) && (limit.limitVolume < settings.target.offset))
        {
            settings.target.offset = limit.limitVolume;
        }
        else if ((limit.limitType == LT_RELATIVE) && (limit.limitVolume < settings.relative))
        {
            settings.relative = limit.limitVolume;
        }
        else  // probably other limits were stronger
        {
            LOG_FN_WARN(__FILENAME__, __func__, "limit", limit.limitType, limit.limitVolume
                    , "ignored for connection", pConnection->getName());
            continue;  // suppress below log entry
        }

        LOG_FN_DEBUG(__FILENAME__, __func__, "limit", limit.limitType, limit.limitVolume
                , "applied for connection", pConnection->getName());
    }
}

void CAmActionSetVolumeCore::_resolveRelativeLimits()
{
    // convert any specified relative limit in list1 to absolute offset by
    // comparing resulting volume to specifications found in list 2

    for (auto &item1 : mConnectionMap)
    {
        auto &target = item1.second.target;
        if (item1.second.relative != AM_VOLUME_NO_LIMIT)
        {
            for (const auto &item2 : mConnectionMap)
            {
                if (item1.first == item2.first)
                {
                    continue;
                }

                am_volume_t limit = item2.second.target.volume + item2.second.target.offset + item1.second.relative;
                if (limit < target.volume + target.offset)
                {
                    target.offset = limit - target.volume;
                }
            }

            // remove relative offset regardless whether we have found a reference connection
            item1.second.relative = AM_VOLUME_NO_LIMIT;
        }
    }
}

void CAmActionSetVolumeCore::_createActionSetElementVolume(const std::shared_ptr<CAmRoutePointElement> &pElement, const ChangeInfo_t &settings)
{
    if (pElement == nullptr)
    {
        LOG_FN_WARN(__FILENAME__, __func__, "pElement NOT valid");
    }

    if (settings.target != settings.actual)
    {
        am_time_t           rampTime = DEFAULT_RAMP_TIME;
        am_CustomRampType_t rampType = DEFAULT_RAMP_TYPE;
        mRampTimeParam.getParam(rampTime);
        mRampTypeParam.getParam(rampType);
        append(new CAmActionSetElementVolume(pElement, settings.target, rampTime, rampType));

        LOG_FN_DEBUG(__FILENAME__, __func__, pElement->getType(), pElement->getName(), settings.target);
    }
}

// *****************************************************************************
//            sub-action    S e t   E l e m e n t - V o l u m e
//

CAmActionSetVolumeCore::CAmActionSetElementVolume::CAmActionSetElementVolume(const std::shared_ptr<CAmRoutePointElement> &pElement
        , const VolumeInfo_t &settings, am_time_t rampTime, am_CustomRampType_t rampType)
    : CAmActionCommand("CAmActionSetElementVolume")
    , mHandle({H_UNKNOWN, 0})
    , mpElement(pElement)
    , mRequested(settings)
    , mRampTime(rampTime)
    , mRampType(rampType)
{
    // validate target element
    if (mpElement == nullptr)
    {
        LOG_FN_ERROR(__FILENAME__, getName(), __func__, "mpElement is NULL");
    }
    else if (!mpElement->getVolumeSupport())
    {
        LOG_FN_WARN(__FILENAME__, getName(), __func__, "NO volume support in"
                , mpElement->getType(), mpElement->getName());
    }

    // always enable the undo feature
    setUndoRequried(true);
}

CAmActionSetVolumeCore::CAmActionSetElementVolume::~CAmActionSetElementVolume()
{
    
}

int CAmActionSetVolumeCore::CAmActionSetElementVolume::_execute(void)
{
    if ((mpElement == nullptr) || (!mpElement->getVolumeSupport()))
    {
        return E_OK;
    }

    // memorize current state for potential request to roll-back
    mRecent.volume = mpElement->getVolume();
    mRecent.offset = mpElement->getOffsetVolume();
    mRecent.muteState = mpElement->getMuteState();
    if (mRecent != mRequested)
    {
        // forward request to routing side
        return _setRoutingSideVolume(mRequested);
    }

    LOG_FN_DEBUG(__FILENAME__, getName(), __func__, "nothing to do for"
            , mpElement->getType(), mpElement->getName());

    return E_OK;
}

int CAmActionSetVolumeCore::CAmActionSetElementVolume::_update(int result)
{
    CAmHandleStore::instance().clearHandle(mHandle);

    if ((mpElement == nullptr) || (!mpElement->getVolumeSupport()))
    {
        return E_OK;
    }

    if ((E_OK == result) && (AS_COMPLETED == getStatus()))
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, mpElement->getType(), mpElement->getName()
                , "volume:", mRequested.volume, "offset:", mRequested.offset, mRequested.muteState);

        mpElement->setMuteState(mRequested.muteState);
        mpElement->setVolume(mRequested.volume);
        mpElement->setOffsetVolume(mRequested.offset);
    }
    else if (AS_UNDO_COMPLETE == getStatus())
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, mpElement->getType(), mpElement->getName()
                , "volume:", mRecent.volume, "offset:", mRecent.offset, mRecent.muteState);

        mpElement->setMuteState(mRecent.muteState);
        mpElement->setVolume(mRecent.volume);
        mpElement->setOffsetVolume(mRecent.offset);
    }
    else
    {
        LOG_FN_WARN(__FILENAME__, __func__, mpElement->getType(), mpElement->getName()
                , "unhandled result", (am_Error_e)result, "return code", getStatus()
                , "for request {volume", mRequested.volume, "offset", mRequested.offset
                , mRequested.muteState, "}");
    }

    return E_OK;
}

int CAmActionSetVolumeCore::CAmActionSetElementVolume::_undo(void)
{
    LOG_FN_WARN(__FILENAME__, getName(), __func__, "reset routing-side volume for"
            , mpElement->getType(), mpElement->getName());

    return _setRoutingSideVolume(mRecent);
}

int CAmActionSetVolumeCore::CAmActionSetElementVolume::_setRoutingSideVolume(const VolumeInfo_t &request)
{
    if (mpElement == nullptr)
    {
        LOG_FN_ERROR(__FILENAME__, getName(), __func__, "mpElement is NULL");

        return E_NOT_POSSIBLE;
    }

    IAmControlReceive *pControlReceive = mpElement->getControlReceive();
    if (pControlReceive == NULL)
    {
        LOG_FN_ERROR(__FILENAME__, getName(), __func__, "pControlReceive is NULL");

        return E_NOT_POSSIBLE;
    }

    // calculate routing-side volume
    am_volume_t        volume = request.volume + request.offset;
    if (request.muteState == MS_MUTED)
    {
        volume = AM_MUTE;
    }
    else
    {
        volume = std::min( mpElement->getMaxVolume(), std::max(volume, mpElement->getMinVolume()));
    }
    am_volume_t routingVolume = mpElement->getRoutingSideVolume(volume);

    LOG_FN_INFO(__FILENAME__, getName(), __func__, mpElement->getType(), mpElement->getName(), "sending", volume
            , "as", routingVolume, "for request {volume", request.volume, "offset", request.offset, request.muteState, "}");

    am_Error_e         result = E_UNKNOWN;
    if (mpElement->getType() == ET_SINK)
    {
        result  = pControlReceive->setSinkVolume(mHandle, mpElement->getID(), routingVolume, mRampType, mRampTime);
    }
    else if (mpElement->getType() == ET_SOURCE)
    {
        result = pControlReceive->setSourceVolume(mHandle, mpElement->getID(), routingVolume, mRampType, mRampTime);
    }

    if (result == E_OK)
    {
        CAmHandleStore::instance().saveHandle(mHandle, this);
        return E_WAIT_FOR_CHILD_COMPLETION;
    }

    return result;
}


} /* namespace gc */
} /* namespace am */

