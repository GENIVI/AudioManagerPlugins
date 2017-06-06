/*
 * CAmVolumeManager.h
 *
 *  Created on: Jun 9, 2017
 *      Author: al1
 */

#ifndef CAMVOLUMEMANAGER_H_
#define CAMVOLUMEMANAGER_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <audiomanagertypes.h>

#include "CAmMainloopPulse.h"


class CAmVolumeAlgo
{
public:
    static CAmVolumeAlgo* createVolumeAlgo(am::am_CustomRampType_t ramp, uint32_t timeMs);

    CAmVolumeAlgo(uint32_t timeMs);
    virtual ~CAmVolumeAlgo() {}
    void start(pa_volume_t startVolume, pa_volume_t endVolume);
    virtual bool getNextValue(pa_volume_t& volume) = 0;

protected:
    uint32_t getTime();

    float       mStartVolume;
    float       mEndVolume;
    uint32_t    mTimeMs;

private:
    std::chrono::steady_clock::time_point mStartTime;
};

class CAmVolumeLinearAlgo : public CAmVolumeAlgo
{
public:
    using CAmVolumeAlgo::CAmVolumeAlgo;

    bool getNextValue(pa_volume_t& volume) override;
};

class CAmVolumeItem
{
public:
    CAmVolumeItem(CAmMainloopPulse& pulse, uint32_t idx, const pa_cvolume& volume, int mute);
    virtual ~CAmVolumeItem();

    uint32_t getIndex() const { return mIndex; }

    bool setMute(int mute);
    bool setVolume(pa_volume_t volume, std::shared_ptr<CAmVolumeAlgo> algo = nullptr);

    virtual bool isVirtual() const = 0;

protected:
    virtual bool setMutePulse(int mute) = 0;
    virtual bool setVolumePulse(const pa_cvolume *volume) = 0;

    CAmMainloopPulse&   mPulse;
    uint32_t            mIndex;
    pa_cvolume          mVolume;
    int                 mMute;

private:
    void startThread();
    void stopThread();
    bool setVolumeLocal(pa_volume_t volume);
    void volumeThread();

    std::thread         mThread;
    std::mutex          mMutex;
    std::atomic_bool    mBreakThread;
    std::shared_ptr<CAmVolumeAlgo> mAlgo;
};

class CAmSourceVolume : public CAmVolumeItem
{
public:
    using CAmVolumeItem::CAmVolumeItem;

    bool isVirtual() const override { return false; }

private:
    bool setMutePulse(int mute) override { return mPulse.setSourceMute(mIndex, mute); }
    bool setVolumePulse(const pa_cvolume *volume) override { return mPulse.setSourceVolume(mIndex, volume); }
};

class CAmSinkInputVolume : public CAmVolumeItem
{
public:
    using CAmVolumeItem::CAmVolumeItem;

    bool isVirtual() const override { return true; }

private:
    bool setMutePulse(int mute) override { return mPulse.setSinkInputMute(mIndex, mute); }
    bool setVolumePulse(const pa_cvolume *volume) override { return mPulse.setSinkInputVolume(mIndex, volume); }
};

class CAmSinkVolume : public CAmVolumeItem
{
public:
    using CAmVolumeItem::CAmVolumeItem;

    bool isVirtual() const override { return false; }

private:
    bool setMutePulse(int mute) override { return mPulse.setSinkMute(mIndex, mute); }
    bool setVolumePulse(const pa_cvolume *volume) override { return mPulse.setSinkVolume(mIndex, volume); }
};

class CAmSourceOutputVolume : public CAmVolumeItem
{
public:
    using CAmVolumeItem::CAmVolumeItem;

    bool isVirtual() const override { return true; }

private:
    bool setMutePulse(int mute) override { return mPulse.setSourceOutputMute(mIndex, mute); }
    bool setVolumePulse(const pa_cvolume *volume) override { return mPulse.setSourceOutputVolume(mIndex, volume); }
};

class CAmVolumeManager
{
public:
    CAmVolumeManager(CAmMainloopPulse& pulse);

    void newSource(am::am_sourceID_t sourceID, const pa_source_info *info);
    void removeSource(uint32_t idx);
    void newSinkInput(am::am_sourceID_t sourceID, const pa_sink_input_info *info);
    void removeSinkInput(uint32_t idx);
    void newSink(am::am_sinkID_t sinkID, const pa_sink_info *info);
    void removeSink(uint32_t idx);
    void newSourceOutput(am::am_sinkID_t sinkID, const pa_source_output_info *info);
    void removeSourceOutput(uint32_t idx);

    am::am_Error_e registerSource(am::am_sourceID_t sourceID, am::am_volume_t volume, am::am_SourceState_e state);
    am::am_Error_e deregisterSource(am::am_sourceID_t sourceID);
    am::am_Error_e registerSink(am::am_sinkID_t sinkID, am::am_volume_t volume, am::am_MuteState_e mute);
    am::am_Error_e deregisterSink(am::am_sinkID_t sinkID);
    am::am_Error_e setSourceState(am::am_sourceID_t sourceID, am::am_SourceState_e state);
    am::am_Error_e setSourceVolume(am::am_sourceID_t sourceID, am::am_volume_t volume, am::am_CustomRampType_t ramp, am::am_time_t time);
    am::am_Error_e setSinkMute(am::am_sinkID_t sinkID, am::am_MuteState_e mute);
    am::am_Error_e setSinkVolume(am::am_sinkID_t sinkID, am::am_volume_t volume, am::am_CustomRampType_t ramp, am::am_time_t time);

private:
    bool updateSourceState(am::am_sourceID_t sourceID);
    bool updateSourceVolume(am::am_sourceID_t sourceID);
    bool updateSinkMute(am::am_sinkID_t sinkID);
    bool updateSinkVolume(am::am_sinkID_t sinkID);
    void handleRemoveSource(uint32_t idx, bool isVirtual);
    void handleRemoveSink(uint32_t idx, bool isVirtual);
    bool isSourceRegistered(am::am_sourceID_t sourceID) { return mSources.find(sourceID) != mSources.end(); }
    bool isSinkRegistered(am::am_sinkID_t sinkID) { return mSinks.find(sinkID) != mSinks.end(); }

    struct rp_SourceVolume_s
    {
        am::am_SourceState_e            state;
        am::am_volume_t                 volume;
        am::am_CustomRampType_t         ramp;
        am::am_time_t                   time;
        std::shared_ptr<CAmVolumeItem>  item;
    };

    struct rp_SinkVolume_s
    {
        am::am_MuteState_e              mute;
        am::am_volume_t                 volume;
        am::am_CustomRampType_t         ramp;
        am::am_time_t                   time;
        std::shared_ptr<CAmVolumeItem>  item;
    };

    CAmMainloopPulse&   mPulse;
    std::mutex          mMutex;

    std::unordered_map<am::am_sourceID_t, rp_SourceVolume_s> mSources;
    std::unordered_map<am::am_sinkID_t, rp_SinkVolume_s> mSinks;
};

#endif /* CAMVOLUMEMANAGER_H_ */
