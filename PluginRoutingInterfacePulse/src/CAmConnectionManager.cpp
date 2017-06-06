/*
 * CAmConnectionManager.cpp
 *
 *  Created on: Jun 15, 2017
 *      Author: al1
 */

#include <algorithm>

#include <CAmDltWrapper.h>

#include "CAmConnectionManager.h"

DLT_IMPORT_CONTEXT(routingPulse)

using namespace am;
using namespace std;

CAmConnectionElement* CAmConnectionElement::createConnectionElement(CAmMainloopPulse& pulse,
        uint32_t sourceIdx, bool isSourceVirtual,
        uint32_t sinkIdx, bool isSinkVirtual)
{
    if (isSourceVirtual && !isSinkVirtual)
    {
        return new CAmConnectionSinkInput2Sink(pulse, sourceIdx, sinkIdx);
    }
    else if (!isSourceVirtual && isSinkVirtual)
    {
        return new CAmConnectionSource2SourceOutput(pulse, sourceIdx, sinkIdx);
    }

    logError("PULSE - ConnectionManager - HW source/sink connection is not implemented");

    return nullptr;
}

CAmConnectionElement::CAmConnectionElement(CAmMainloopPulse& pulse, uint32_t sourceIdx, uint32_t sinkIdx) :
    mPulse(pulse),
    mSourceIdx(sourceIdx),
    mSinkIdx(sinkIdx)
{
}

CAmConnectionSinkInput2Sink::CAmConnectionSinkInput2Sink(CAmMainloopPulse& pulse, uint32_t sourceIdx, uint32_t sinkIdx) :
    CAmConnectionElement(pulse, sourceIdx, sinkIdx)
{
    connect();
}

CAmConnectionSinkInput2Sink::~CAmConnectionSinkInput2Sink()
{
    disconnect();
}

void CAmConnectionSinkInput2Sink::connect()
{
    logInfo("PULSE - ConnectionManager - connect SinkInput: ", mSourceIdx, " to Sink: ", mSinkIdx);

    mPulse.moveSinkInputToSink(mSourceIdx, mSinkIdx);
}

void CAmConnectionSinkInput2Sink::disconnect()
{
    logInfo("PULSE - ConnectionManager - disconnect SinkInput: ", mSourceIdx, " from Sink: ", mSinkIdx);

    mPulse.moveSinkInputToNull(mSourceIdx);
}

CAmConnectionSource2SourceOutput::CAmConnectionSource2SourceOutput(CAmMainloopPulse& pulse, uint32_t sourceIdx, uint32_t sinkIdx) :
    CAmConnectionElement(pulse, sourceIdx, sinkIdx)
{
    connect();
}

CAmConnectionSource2SourceOutput::~CAmConnectionSource2SourceOutput()
{
    disconnect();
}

void CAmConnectionSource2SourceOutput::connect()
{
    logInfo("PULSE - ConnectionManager - connect Source: ", mSourceIdx, " to SourceOutput: ", mSinkIdx);

    mPulse.moveSourceOutputToSource(mSinkIdx, mSourceIdx);
}

void CAmConnectionSource2SourceOutput::disconnect()
{
    logInfo("PULSE - ConnectionManager - disconnect Source: ", mSourceIdx, " from SourceOutput: ", mSinkIdx);

    mPulse.moveSourceOutputToNull(mSinkIdx);
}

CAmConnectionManager::CAmConnectionManager(CAmMainloopPulse& pulse) :
    mPulse(pulse)
{

}

void CAmConnectionManager::newSource(am_sourceID_t sourceID, uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    if (isSourceRegistered(sourceID))
    {
        handleNewSource(sourceID, idx, false);
    }
    else
    {
        logError("PULSE - ConnectionManager - source: ", sourceID, " is not registered");
    }
}

void CAmConnectionManager::removeSource(uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    handleRemoveSource(idx, false);
}

void CAmConnectionManager::newSinkInput(am_sourceID_t sourceID, uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    if (isSourceRegistered(sourceID))
    {
        handleNewSource(sourceID, idx, true);
    }
    else
    {
        logError("PULSE - ConnectionManager - source: ", sourceID, " is not registered");
    }
}

void CAmConnectionManager::removeSinkInput(uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    handleRemoveSource(idx, true);
}

void CAmConnectionManager::newSink(am_sinkID_t sinkID, uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    if (isSinkRegistered(sinkID))
    {
        handleNewSink(sinkID, idx, false);
    }
    else
    {
        logError("PULSE - ConnectionManager - sink: ", sinkID, " is not registered");
    }
}

void CAmConnectionManager::removeSink(uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    handleRemoveSink(idx, false);
}

void CAmConnectionManager::newSourceOutput(am_sinkID_t sinkID, uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    if (isSinkRegistered(sinkID))
    {
        handleNewSink(sinkID, idx, true);
    }
    else
    {
        logError("PULSE - ConnectionManager - sink: ", sinkID, " is not registered");
    }
}

void CAmConnectionManager::removeSourceOutput(uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    handleRemoveSink(idx, true);
}

am_Error_e CAmConnectionManager::registerSource(am_sourceID_t sourceID)
{
    lock_guard<mutex> lock(mMutex);

    if (isSourceRegistered(sourceID))
    {
        return E_ALREADY_EXISTS;
    }

    mSources[sourceID].idx = -1;
    mSources[sourceID].isVirtual = false;

    return E_OK;
}

am_Error_e CAmConnectionManager::deregisterSource(am_sourceID_t sourceID)
{
    lock_guard<mutex> lock(mMutex);

    if (!isSourceRegistered(sourceID))
    {
        return E_NON_EXISTENT;
    }

    mSources.erase(sourceID);

    return E_OK;
}

am_Error_e CAmConnectionManager::registerSink(am_sinkID_t sinkID)
{
    lock_guard<mutex> lock(mMutex);

    if (isSinkRegistered(sinkID))
    {
        return E_ALREADY_EXISTS;
    }

    mSinks[sinkID].idx = -1;
    mSinks[sinkID].isVirtual = false;

    return E_OK;
}

am_Error_e CAmConnectionManager::deregisterSink(am::am_sinkID_t sinkID)
{
    lock_guard<mutex> lock(mMutex);

    if (!isSinkRegistered(sinkID))
    {
        return E_NON_EXISTENT;
    }

    mSinks.erase(sinkID);

    return E_OK;
}

am_Error_e CAmConnectionManager::connect(am_sourceID_t sourceID, am_sinkID_t sinkID, am_connectionID_t connectionID)
{
    lock_guard<mutex> lock(mMutex);

    if (!isSourceRegistered(sourceID) && !isSinkRegistered(sinkID))
    {
        return E_NON_EXISTENT;
    }

    mConnections[connectionID].sourceID = sourceID;
    mConnections[connectionID].sinkID = sinkID;

    updateConnection(connectionID);

    return E_OK;
}

am_Error_e CAmConnectionManager::disconnect(am_connectionID_t connectionID)
{
    lock_guard<mutex> lock(mMutex);

    auto it = mConnections.find(connectionID);

    if (it == mConnections.end())
    {
        return E_NON_EXISTENT;
    }

    mConnections.erase(it);

    return E_OK;
}

void CAmConnectionManager::handleNewSource(am::am_sourceID_t sourceID, uint32_t idx, bool isVirtual)
{
    mSources[sourceID].idx = idx;
    mSources[sourceID].isVirtual = isVirtual;

    auto connectionID = findConnectionBySourceID(sourceID);

    if (connectionID)
    {
        updateConnection(connectionID);
    }
}

void CAmConnectionManager::handleNewSink(am::am_sinkID_t sinkID, uint32_t idx, bool isVirtual)
{
    mSinks[sinkID].idx = idx;
    mSinks[sinkID].isVirtual = isVirtual;

    auto connectionID = findConnectionBySinkID(sinkID);

    if (connectionID)
    {
        updateConnection(connectionID);
    }
}

void CAmConnectionManager::handleRemoveSource(uint32_t idx, bool isVirtual)
{
    auto it = find_if(mSources.begin(), mSources.end(),
                      [idx, isVirtual](const pair<am_sourceID_t, rp_SourceConnection_s>& value)
                      { return value.second.idx == idx &&
                               value.second.isVirtual == isVirtual; });

    if (it != mSources.end())
    {
        it->second.idx = -1;
    }
}

void CAmConnectionManager::handleRemoveSink(uint32_t idx, bool isVirtual)
{
    auto it = find_if(mSinks.begin(), mSinks.end(),
                      [idx, isVirtual](const pair<am_sourceID_t, rp_SinkConnection_s>& value)
                      { return value.second.idx == idx &&
                               value.second.isVirtual == isVirtual; });

    if (it != mSinks.end())
    {
        it->second.idx = -1;
    }
}

am_connectionID_t CAmConnectionManager::findConnectionBySourceID(am_sourceID_t sourceID)
{
    auto it = find_if(mConnections.begin(), mConnections.end(),
                      [sourceID](const pair<am_connectionID_t, rp_Connection_s>& value)
                      { return value.second.sourceID == sourceID; });

    if (it != mConnections.end())
    {
        return it->first;
    }

    return 0;
}

am_connectionID_t CAmConnectionManager::findConnectionBySinkID(am_sinkID_t sinkID)
{
    auto it = find_if(mConnections.begin(), mConnections.end(),
                      [sinkID](const pair<am_connectionID_t, rp_Connection_s>& value)
                      { return value.second.sinkID == sinkID; });

    if (it != mConnections.end())
    {
        return it->first;
    }

    return 0;
}

void CAmConnectionManager::updateConnection(am_connectionID_t connectionID)
{
    auto sourceID = mConnections[connectionID].sourceID;
    auto sinkID = mConnections[connectionID].sinkID;

    if (isSourceRegistered(sourceID) && isSinkRegistered(sinkID))
    {
        if (mSources[sourceID].idx != -1 && mSinks[sinkID].idx != -1)
        {
            mConnections[connectionID].connection.reset(
                    CAmConnectionElement::createConnectionElement(mPulse,
                            mSources[sourceID].idx, mSources[sourceID].isVirtual,
                            mSinks[sinkID].idx, mSinks[sinkID].isVirtual));
        }
        else if (mConnections[connectionID].connection)
        {
            mConnections[connectionID].connection.reset();
        }
    }
}
