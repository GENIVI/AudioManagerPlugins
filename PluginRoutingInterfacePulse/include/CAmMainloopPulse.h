/*
 * CAmMainloopPulse.h
 *
 *  Created on: Jun 7, 2017
 *      Author: al1
 */

#ifndef CAMMAINLOOPPULSE_H_
#define CAMMAINLOOPPULSE_H_

#include <string>
#include <pulse/pulseaudio.h>

class CAmProplistPulse
{
public:
    CAmProplistPulse() { mProplist = pa_proplist_new(); }
    CAmProplistPulse(const std::string& name, const std::string& value) : CAmProplistPulse()
    {
        set(name, value);
    }
    ~CAmProplistPulse() { pa_proplist_free(mProplist); }

    int set(const std::string& name, const std::string& value)
    {
        return pa_proplist_sets(mProplist, name.c_str(), value.c_str());
    }

    pa_proplist* operator&() { return mProplist; }

private:
    pa_proplist* mProplist;
};

class CAmPulseLock
{
public:
    CAmPulseLock(pa_threaded_mainloop* mainloop);
    ~CAmPulseLock();

    pa_context_success_cb_t getSuccessCbk();
    int waitResult(pa_operation* op);

private:
    static void sContextSuccess(pa_context *ctx, int success, void *userdata);
    void contextSuccess(int success);

    int mSuccess;
    pa_threaded_mainloop*   mMainloop;
};

class CAmPulseClientItf
{
public:
    virtual ~CAmPulseClientItf() {}

    virtual void onNewSource(const pa_source_info *info) = 0;
    virtual void onRemoveSource(uint32_t idx) = 0;
    virtual void onNewSink(const pa_sink_info *info) = 0;
    virtual void onRemoveSink(uint32_t idx) = 0;
    virtual void onNewSinkInput(const pa_sink_input_info *info) = 0;
    virtual void onRemoveSinkInput(uint32_t idx) = 0;
    virtual void onNewSourceOutput(const pa_source_output_info *info) = 0;
    virtual void onRemoveSourceOutput(uint32_t idx) = 0;
};

class CAmMainloopPulse
{
public:
    CAmMainloopPulse(CAmPulseClientItf& client);
    ~CAmMainloopPulse();

    bool setSourceMute(uint32_t idx, int mute);
    bool setSourceVolume(uint32_t idx, const pa_cvolume *volume);
    bool setSinkInputMute(uint32_t idx, int mute);
    bool setSinkInputVolume(uint32_t idx, const pa_cvolume *volume);
    bool setSinkMute(uint32_t idx, int mute);
    bool setSinkVolume(uint32_t idx, const pa_cvolume *volume);
    bool setSourceOutputMute(uint32_t idx, int mute);
    bool setSourceOutputVolume(uint32_t idx, const pa_cvolume *volume);
    bool moveSinkInputToSink(uint32_t sinkInputIdx, uint32_t sinkIdx);
    bool moveSourceOutputToSource(uint32_t sourceOutputIdx, uint32_t sourceIdx);
    bool moveSinkInputToNull(uint32_t sinkInputIdx);
    bool moveSourceOutputToNull(uint32_t sourceOutputIdx);

    int start();
    void stop();

private:
    pa_threaded_mainloop*   mMainloop;
    pa_context*             mContext;
    CAmPulseClientItf&      mClient;

    uint32_t                mNullSink;
    uint32_t                mNullSource;

    static void sContextSuccessCbk(pa_context *ctx, void *mainloop);
    static void sContextStateChanged(pa_context *ctx, void *mainloop);
    static void sEventCallback(pa_context *ctx, pa_subscription_event_type_t event, uint32_t idx, void *mainloop);
    static void sSubscriberCallback(pa_context *ctx, int success, void *mainloop);
    static void sSourceInfoCallback(pa_context *ctx, const pa_source_info *info, int isLast, void *mainloop);
    static void sSinkInfoCallback(pa_context *ctx, const pa_sink_info *info, int isLast, void *mainloop);
    static void sSinkInputInfoCallback(pa_context *ctx, const pa_sink_input_info *info, int isLast, void *mainloop);
    static void sSourceOutputInfoCallback(pa_context *ctx, const pa_source_output_info *info, int isLast, void *mainloop);


    void contextStateChanged(pa_context *ctx);
    void eventCallback(pa_context *ctx, pa_subscription_event_type_t event, uint32_t idx);
    void subscriberCallback(pa_context *ctx, int success);
    void sourceInfoCallback(pa_context *ctx, const pa_source_info *info, int isLast);
    void sinkInfoCallback(pa_context *ctx, const pa_sink_info *info, int isLast);
    void sinkInputInfoCallback(pa_context *ctx, const pa_sink_input_info *info, int isLast);
    void sourceOutputInfoCallback(pa_context *ctx, const pa_source_output_info *info, int isLast);

    void onNewType(pa_context *ctx, pa_subscription_event_type_t type, uint32_t idx);
    void onRemoveType(pa_context *ctx, pa_subscription_event_type_t type, uint32_t idx);
};

#endif /* CAMMAINLOOPPULSE_H_ */
