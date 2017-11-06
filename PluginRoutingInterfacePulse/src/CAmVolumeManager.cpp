/*
 * CAmVolumeManager.cpp
 *
 *  Created on: Jun 9, 2017
 *      Author: al1
 */

#include <algorithm>

#include <CAmDltWrapper.h>

#include "CAmVolumeManager.h"

DLT_IMPORT_CONTEXT(routingPulse)

using namespace am;
using namespace std;

CAmVolumeAlgo* CAmVolumeAlgo::createVolumeAlgo(am::am_CustomRampType_t ramp, uint32_t timeMs)
{
    if (ramp > 2 && timeMs > 0)
    {
        return new CAmVolumeLinearAlgo(timeMs);
    }

    return nullptr;
}

CAmVolumeAlgo::CAmVolumeAlgo(uint32_t timeMs) :
    mTimeMs(timeMs),
    mStartVolume(0),
    mEndVolume(0)
{
}

void CAmVolumeAlgo::start(pa_volume_t startVolume, pa_volume_t endVolume)
{
    mStartVolume = startVolume;
    mEndVolume = endVolume;

    mStartTime = chrono::steady_clock::now();
}

uint32_t CAmVolumeAlgo::getTime()
{
    return chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - mStartTime).count();
}

CAmVolumeItem::CAmVolumeItem(CAmMainloopPulse& pulse, uint32_t idx, const pa_cvolume& volume, int mute) :
    mPulse(pulse),
    mIndex(idx),
    mVolume(volume),
    mMute(mute),
    mBreakThread(false)
{
    logInfo("PULSE - VolumeManager - create volume item, idx:", mIndex, ", volume: ", mVolume.values[0], ", mute: ", mMute);
}

CAmVolumeItem::~CAmVolumeItem()
{
    stopThread();

    logInfo("PULSE - VolumeManager - delete volume item, idx:", mIndex);
}

bool CAmVolumeItem::setMute(int mute)
{
    logInfo("PULSE - VolumeManager - set mute idx:", mIndex, ", mute: ",  mute);

    if (mute != mMute)
    {
        mMute = mute;

        return setMutePulse(mMute);
    }

    return true;
}

bool CAmVolumeItem::setVolume(pa_volume_t volume, shared_ptr<CAmVolumeAlgo> algo)
{
    logInfo("PULSE - VolumeManager - set volume idx:", mIndex, ", volume: ",  volume);

    stopThread();

    lock_guard<mutex> lock(mMutex);

    bool update = false;

    for (uint8_t i = 0; i < mVolume.channels; i++)
    {
        if (mVolume.values[i] != volume)
        {
            update = true;
        }
    }

    if (update)
    {
        mAlgo = algo;

        if (mAlgo)
        {
            mAlgo->start(mVolume.values[0], volume);

            mAlgo->getNextValue(volume);
        }

        auto ret = setVolumeLocal(volume);

        if (ret && mAlgo)
        {
            startThread();
        }

        return ret;
    }

    return true;
}

bool CAmVolumeItem::setVolumeLocal(pa_volume_t volume)
{
    for (uint8_t i = 0; i < mVolume.channels; i++)
    {
        mVolume.values[i] = volume;
    }

    return setVolumePulse(&mVolume);
}

void CAmVolumeItem::startThread()
{
    mThread = thread(&CAmVolumeItem::volumeThread, this);
}

void CAmVolumeItem::stopThread()
{
    if (mThread.joinable())
    {
        mBreakThread = true;

        mThread.join();

        mBreakThread = false;
    }
}

void CAmVolumeItem::volumeThread()
{
    pa_volume_t volume;

    while(!mBreakThread && mAlgo->getNextValue(volume))
    {
        lock_guard<mutex> lock(mMutex);

        setVolumeLocal(volume);

        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

bool CAmVolumeLinearAlgo::getNextValue(pa_volume_t& volume)
{
    auto currentTime = getTime();

    if (currentTime <= mTimeMs)
    {
        volume = mStartVolume + (mEndVolume - mStartVolume) * currentTime / mTimeMs;

        return true;
    }

    return false;
}

CAmVolumeManager::CAmVolumeManager(CAmMainloopPulse& pulse) :
    mPulse(pulse)
{

}

void CAmVolumeManager::newSource(am_sourceID_t sourceID, const pa_source_info *info)
{
    lock_guard<mutex> lock(mMutex);

    if (isSourceRegistered(sourceID))
    {
        mSources[sourceID].item.reset(new CAmSourceVolume(mPulse, info->index, info->volume, info->mute));

        updateSourceState(sourceID);
        updateSourceVolume(sourceID);
    }
    else
    {
        logError("PULSE - VolumeManager - source: ", sourceID, " is not registered");
    }
}

void CAmVolumeManager::removeSource(uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    handleRemoveSource(idx, false);
}

void CAmVolumeManager::newSinkInput(am_sourceID_t sourceID, const pa_sink_input_info *info)
{
    lock_guard<mutex> lock(mMutex);

    if (isSourceRegistered(sourceID))
    {
        mSources[sourceID].item.reset(new CAmSinkInputVolume(mPulse, info->index, info->volume, info->mute));

        updateSourceState(sourceID);
        updateSourceVolume(sourceID);
    }
    else
    {
        logError("PULSE - VolumeManager - source: ", sourceID, " is not registered");
    }
}

void CAmVolumeManager::removeSinkInput(uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    handleRemoveSource(idx, true);
}

void CAmVolumeManager::newSink(am_sinkID_t sinkID, const pa_sink_info *info)
{
    lock_guard<mutex> lock(mMutex);

    if (isSinkRegistered(sinkID))
    {
        mSinks[sinkID].item.reset(new CAmSinkVolume(mPulse, info->index, info->volume, info->mute));

        updateSinkMute(sinkID);
        updateSinkVolume(sinkID);
    }
    else
    {
        logError("PULSE - VolumeManager - sink: ", sinkID, " is not registered");
    }
}

void CAmVolumeManager::removeSink(uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    handleRemoveSink(idx, false);
}

void CAmVolumeManager::newSourceOutput(am_sinkID_t sinkID, const pa_source_output_info *info)
{
    lock_guard<mutex> lock(mMutex);

    if (isSinkRegistered(sinkID))
    {
        mSinks[sinkID].item.reset(new CAmSourceOutputVolume(mPulse, info->index, info->volume, info->mute));

        updateSinkMute(sinkID);
        updateSinkVolume(sinkID);
    }
    else
    {
        logError("PULSE - VolumeManager - sink: ", sinkID, " is not registered");
    }
}

void CAmVolumeManager::removeSourceOutput(uint32_t idx)
{
    lock_guard<mutex> lock(mMutex);

    handleRemoveSink(idx, true);
}

am_Error_e CAmVolumeManager::registerSource(am_sourceID_t sourceID, am_volume_t volume, am_SourceState_e state)
{
    lock_guard<mutex> lock(mMutex);

    if (isSourceRegistered(sourceID))
    {
        return E_ALREADY_EXISTS;
    }

    mSources[sourceID] = { state, volume };

    return E_OK;
}

am_Error_e CAmVolumeManager::deregisterSource(am_sourceID_t sourceID)
{
    lock_guard<mutex> lock(mMutex);

    if (!isSourceRegistered(sourceID))
    {
        return E_NON_EXISTENT;
    }

    mSources.erase(sourceID);

    return E_OK;
}

am_Error_e CAmVolumeManager::registerSink(am_sinkID_t sinkID, am_volume_t volume, am_MuteState_e mute)
{
    lock_guard<mutex> lock(mMutex);

    if (isSinkRegistered(sinkID))
    {
        return E_ALREADY_EXISTS;
    }

    mSinks[sinkID] = { mute, volume };

    return E_OK;
}

am_Error_e CAmVolumeManager::deregisterSink(am::am_sinkID_t sinkID)
{
    lock_guard<mutex> lock(mMutex);

    if (!isSinkRegistered(sinkID))
    {
        return E_NON_EXISTENT;
    }

    mSinks.erase(sinkID);

    return E_OK;
}

am_Error_e CAmVolumeManager::setSourceState(am_sourceID_t sourceID, am_SourceState_e state)
{
    lock_guard<mutex> lock(mMutex);

    if (isSourceRegistered(sourceID))
    {
        mSources[sourceID].state = state;

        return updateSourceState(sourceID) ? E_OK : E_UNKNOWN;
    }

    return E_NON_EXISTENT;
}

am_Error_e CAmVolumeManager::setSourceVolume(am_sourceID_t sourceID, am_volume_t volume, am_CustomRampType_t ramp, am_time_t time)
{
    lock_guard<mutex> lock(mMutex);

    if (isSourceRegistered(sourceID))
    {
        mSources[sourceID].volume = volume;
        mSources[sourceID].ramp = ramp;
        mSources[sourceID].time = time;

        return updateSourceVolume(sourceID) ? E_OK : E_UNKNOWN;
    }

    return E_NON_EXISTENT;
}

am_Error_e CAmVolumeManager::setSinkMute(am_sinkID_t sinkID, am_MuteState_e mute)
{
    if (isSinkRegistered(sinkID))
    {
        mSinks[sinkID].mute = mute;

        return updateSinkMute(sinkID) ? E_OK : E_UNKNOWN;
    }

    return E_NON_EXISTENT;
}

am_Error_e CAmVolumeManager::setSinkVolume(am_sinkID_t sinkID, am_volume_t volume, am_CustomRampType_t ramp, am_time_t time)
{
    lock_guard<mutex> lock(mMutex);

    if (isSinkRegistered(sinkID))
    {
        mSinks[sinkID].volume = volume;
        mSinks[sinkID].ramp = ramp;
        mSinks[sinkID].time = time;

        return updateSinkVolume(sinkID) ? E_OK : E_UNKNOWN;
    }

    return E_NON_EXISTENT;
}

bool CAmVolumeManager::updateSourceState(am::am_sourceID_t sourceID)
{
    if (!mSources[sourceID].item)
    {
        return true;
    }

    auto state = mSources[sourceID].state;

    if (state == SS_ON)
    {
        return mSources[sourceID].item->setMute(0);
    }

    if (state == SS_OFF || state == SS_PAUSED)
    {
        return mSources[sourceID].item->setMute(1);
    }
}

bool CAmVolumeManager::updateSourceVolume(am::am_sourceID_t sourceID)
{
    if (!mSources[sourceID].item)
    {
        return true;
    }

    shared_ptr<CAmVolumeAlgo> algo(CAmVolumeAlgo::createVolumeAlgo(mSources[sourceID].ramp, mSources[sourceID].time));

    return mSources[sourceID].item->setVolume(pa_sw_volume_from_dB(mSources[sourceID].volume*0.1), algo);
}

bool CAmVolumeManager::updateSinkMute(am::am_sinkID_t sinkID)
{
    if (!mSinks[sinkID].item)
    {
        return true;
    }

    auto mute = mSinks[sinkID].mute;

    if (mute == MS_UNMUTED)
    {
        return mSinks[sinkID].item->setMute(0);
    }

    if (mute == MS_MUTED)
    {
        return mSinks[sinkID].item->setMute(1);
    }
}

bool CAmVolumeManager::updateSinkVolume(am::am_sinkID_t sinkID)
{
    if (!mSinks[sinkID].item)
    {
        return true;
    }

    shared_ptr<CAmVolumeAlgo> algo(CAmVolumeAlgo::createVolumeAlgo(mSinks[sinkID].ramp, mSinks[sinkID].time));

    return mSinks[sinkID].item->setVolume(pa_sw_volume_from_dB(mSinks[sinkID].volume*0.1), algo);
}

void CAmVolumeManager::handleRemoveSource(uint32_t idx, bool isVirtual)
{
    auto it = find_if(mSources.begin(), mSources.end(),
                      [idx, isVirtual](const pair<am_sourceID_t, rp_SourceVolume_s>& value)
                      { return value.second.item &&
                               value.second.item->getIndex() == idx &&
                               value.second.item->isVirtual() == isVirtual; });

    if (it != mSources.end())
    {
        it->second.item.reset();
    }
}

void CAmVolumeManager::handleRemoveSink(uint32_t idx, bool isVirtual)
{
    auto it = find_if(mSinks.begin(), mSinks.end(),
                      [idx, isVirtual](const pair<am_sourceID_t, rp_SinkVolume_s>& value)
                      { return value.second.item &&
                               value.second.item->getIndex() == idx &&
                               value.second.item->isVirtual() == isVirtual; });

    if (it != mSinks.end())
    {
        it->second.item.reset();
    }
}
