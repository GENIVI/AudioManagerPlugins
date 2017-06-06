/*
 * CAmMainloopPulse.cpp
 *
 *  Created on: Jun 7, 2017
 *      Author: al1
 */

#include "CAmMainloopPulse.h"

#include <cstring>

#include <CAmDltWrapper.h>

DLT_IMPORT_CONTEXT(routingPulse)

using namespace am;
using namespace std;

CAmPulseLock::CAmPulseLock(pa_threaded_mainloop* mainloop) :
    mMainloop(mainloop),
    mSuccess(0)
{
    if (!pa_threaded_mainloop_in_thread(mMainloop))
    {
        pa_threaded_mainloop_lock(mMainloop);
    }
}

CAmPulseLock::~CAmPulseLock()
{
    if (!pa_threaded_mainloop_in_thread(mMainloop))
    {
        pa_threaded_mainloop_unlock(mMainloop);
    }
}

pa_context_success_cb_t CAmPulseLock::getSuccessCbk()
{
    if (!pa_threaded_mainloop_in_thread(mMainloop))
    {
        return sContextSuccess;
    }

    return nullptr;
}

int CAmPulseLock::waitResult(pa_operation* op)
{
    if (!op)
    {
        logError("PULSE - Mainloop - operation failed");

        return 0;
    }

    if (!pa_threaded_mainloop_in_thread(mMainloop))
    {
        while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
        {
            pa_threaded_mainloop_wait(mMainloop);
        }

        return mSuccess;
    }

    return 1;
}

void CAmPulseLock::sContextSuccess(pa_context *ctx, int success, void *userdata)
{
    static_cast<CAmPulseLock*>(userdata)->contextSuccess(success);
}
void CAmPulseLock::contextSuccess(int success)
{
    mSuccess = success;

    pa_threaded_mainloop_signal(mMainloop, 0);
}

CAmMainloopPulse::CAmMainloopPulse(CAmPulseClientItf& client) :
    mClient(client),
    mMainloop(nullptr),
    mContext(nullptr),
    mNullSink(-1),
    mNullSource(-1)
{
    CAmProplistPulse proplist(PA_PROP_APPLICATION_NAME, "RoutingPULSE");

    mMainloop = pa_threaded_mainloop_new();

    if (!mMainloop)
    {
        logError("PULSE - Mainloop - Can't create pulse mainloop");

        throw runtime_error("Can't create pulse mainloop");
    }

    auto api = pa_threaded_mainloop_get_api(mMainloop);

    mContext = pa_context_new_with_proplist(api, nullptr, &proplist);

    if (!mContext)
    {
        logError("PULSE - Mainloop - Can't create pulse context");

        throw runtime_error("Can't create pulse context");
    }

    pa_context_set_state_callback(mContext, sContextStateChanged, this);

    if (pa_context_connect(mContext, nullptr, pa_context_flags_t(0), nullptr))
    {
        logError("PULSE - Can't connect pulse context");

        throw runtime_error("Can't connect pulse context");
    }
}

CAmMainloopPulse::~CAmMainloopPulse()
{
    stop();

    pa_threaded_mainloop_free(mMainloop);
}

int CAmMainloopPulse::start()
{
    logInfo("PULSE - Mainloop - start");

    auto ret = pa_threaded_mainloop_start(mMainloop);

    if (ret)
    {
        logError("PULSE - Mainloop - start: ", pa_strerror(ret));
    }

    return ret;
}

void CAmMainloopPulse::stop()
{
    logInfo("PULSE - Mainloop - stop");

    pa_threaded_mainloop_stop(mMainloop);
}

bool CAmMainloopPulse::setSourceMute(uint32_t idx, int mute)
{
    CAmPulseLock lock(mMainloop);

    logInfo("PULSE - Mainloop - setSourceMuteUnlocked, idx: ", idx, ", mute: ", mute);

    auto op = pa_context_set_source_mute_by_index(mContext, idx, mute, lock.getSuccessCbk(), &lock);

    auto ret = lock.waitResult(op);

    if (op)
    {
        pa_operation_unref(op);
    }

    return ret;
}

bool CAmMainloopPulse::setSourceVolume(uint32_t idx, const pa_cvolume *volume)
{
    CAmPulseLock lock(mMainloop);

    logInfo("PULSE - Mainloop - setSourceMuteUnlocked, idx: ", idx, ", volume: ", volume->values[0]);

    auto op = pa_context_set_source_volume_by_index(mContext, idx, volume, lock.getSuccessCbk(), &lock);

    auto ret = lock.waitResult(op);

    if (op)
    {
        pa_operation_unref(op);
    }

    return ret;
}

bool CAmMainloopPulse::setSinkInputMute(uint32_t idx, int mute)
{
    CAmPulseLock lock(mMainloop);

    logInfo("PULSE - Mainloop - setSinkInputMuteUnlocked, idx: ", idx, ", mute: ", mute);

    auto op = pa_context_set_sink_input_mute(mContext, idx, mute, lock.getSuccessCbk(), &lock);

    auto ret = lock.waitResult(op);

    if (op)
    {
        pa_operation_unref(op);
    }

    return ret;
}

bool CAmMainloopPulse::setSinkInputVolume(uint32_t idx, const pa_cvolume *volume)
{
    CAmPulseLock lock(mMainloop);

    logInfo("PULSE - Mainloop - setSinkInputVolumeUnlocked, idx: ", idx, ", volume: ", volume->values[0]);

    auto op = pa_context_set_sink_input_volume(mContext, idx, volume, lock.getSuccessCbk(), &lock);

    auto ret = lock.waitResult(op);

    if (op)
    {
        pa_operation_unref(op);
    }

    return ret;
}

bool CAmMainloopPulse::setSinkMute(uint32_t idx, int mute)
{
    CAmPulseLock lock(mMainloop);

    logInfo("PULSE - Mainloop - setSinkMuteUnlocked, idx: ", idx, ", mute: ", mute);

    auto op = pa_context_set_sink_mute_by_index(mContext, idx, mute, lock.getSuccessCbk(), &lock);

    auto ret = lock.waitResult(op);

    if (op)
    {
        pa_operation_unref(op);
    }

    return ret;
}

bool CAmMainloopPulse::setSinkVolume(uint32_t idx, const pa_cvolume *volume)
{
    CAmPulseLock lock(mMainloop);

    logInfo("PULSE - Mainloop - setSinkVolumeUnlocked, idx: ", idx, ", volume: ", volume->values[0]);

    auto op = pa_context_set_sink_volume_by_index(mContext, idx, volume, lock.getSuccessCbk(), &lock);

    auto ret = lock.waitResult(op);

    if (op)
    {
        pa_operation_unref(op);
    }

    return ret;
}

bool CAmMainloopPulse::setSourceOutputMute(uint32_t idx, int mute)
{
    CAmPulseLock lock(mMainloop);

    logInfo("PULSE - Mainloop - setSourceOutputMuteUnlocked, idx: ", idx, ", mute: ", mute);

    auto op = pa_context_set_source_output_mute(mContext, idx, mute, lock.getSuccessCbk(), &lock);

    auto ret = lock.waitResult(op);

    if (op)
    {
        pa_operation_unref(op);
    }

    return ret;
}

bool CAmMainloopPulse::setSourceOutputVolume(uint32_t idx, const pa_cvolume *volume)
{
    CAmPulseLock lock(mMainloop);

    logInfo("PULSE - Mainloop - setSourceOutputVolumeUnlocked, idx: ", idx, ", volume: ", volume->values[0]);

    auto op = pa_context_set_source_output_volume(mContext, idx, volume, lock.getSuccessCbk(), &lock);

    auto ret = lock.waitResult(op);

    if (op)
    {
        pa_operation_unref(op);
    }

    return ret;
}

bool CAmMainloopPulse::moveSinkInputToSink(uint32_t sinkInputIdx, uint32_t sinkIdx)
{
    CAmPulseLock lock(mMainloop);

    logInfo("PULSE - Mainloop - moveSinkInput, sinkInputIdx: ", sinkInputIdx, ", sinkIdx: ", sinkIdx);

    auto op = pa_context_move_sink_input_by_index(mContext, sinkInputIdx, sinkIdx, lock.getSuccessCbk(), &lock);

    auto ret = lock.waitResult(op);

    if (op)
    {
        pa_operation_unref(op);
    }

    return ret;
}

bool CAmMainloopPulse::moveSinkInputToNull(uint32_t sinkInputIdx)
{
    if (mNullSink != -1)
    {
        return moveSinkInputToSink(sinkInputIdx, mNullSink);
    }

    logWarning("PULSE - Mainloop - Null Sink is not present");

    return false;
}

bool CAmMainloopPulse::moveSourceOutputToSource(uint32_t sourceOutputIdx, uint32_t sourceIdx)
{
    CAmPulseLock lock(mMainloop);

    logInfo("PULSE - Mainloop - moveSourceOutput, sourceOutputIdx: ", sourceOutputIdx, ", sourceIdx: ", sourceIdx);

    auto op = pa_context_move_source_output_by_index(mContext, sourceOutputIdx, sourceIdx, lock.getSuccessCbk(), &lock);

    auto ret = lock.waitResult(op);

    if (op)
    {
        pa_operation_unref(op);
    }

    return ret;
}

bool CAmMainloopPulse::moveSourceOutputToNull(uint32_t sourceOutputIdx)
{
    if (mNullSource != -1)
    {
        return moveSourceOutputToSource(sourceOutputIdx, mNullSource);
    }

    logWarning("PULSE - Mainloop - Null Source is not present");

    return false;
}

void CAmMainloopPulse::sContextStateChanged(pa_context *ctx, void *mainloop)
{
    static_cast<CAmMainloopPulse*>(mainloop)->contextStateChanged(ctx);
}

void CAmMainloopPulse::sEventCallback(pa_context *ctx, pa_subscription_event_type_t event, uint32_t idx, void *mainloop)
{
    static_cast<CAmMainloopPulse*>(mainloop)->eventCallback(ctx, event, idx);
}

void CAmMainloopPulse::sSubscriberCallback(pa_context *ctx, int success, void *mainloop)
{
    static_cast<CAmMainloopPulse*>(mainloop)->subscriberCallback(ctx, success);
}

void CAmMainloopPulse::sSourceInfoCallback(pa_context *ctx, const pa_source_info *info, int isLast, void *mainloop)
{
    static_cast<CAmMainloopPulse*>(mainloop)->sourceInfoCallback(ctx, info, isLast);
}

void CAmMainloopPulse::sSinkInfoCallback(pa_context *ctx, const pa_sink_info *info, int isLast, void *mainloop)
{
    static_cast<CAmMainloopPulse*>(mainloop)->sinkInfoCallback(ctx, info, isLast);
}

void CAmMainloopPulse::sSinkInputInfoCallback(pa_context *ctx, const pa_sink_input_info *info, int isLast, void *mainloop)
{
    static_cast<CAmMainloopPulse*>(mainloop)->sinkInputInfoCallback(ctx, info, isLast);
}

void CAmMainloopPulse::sSourceOutputInfoCallback(pa_context *ctx, const pa_source_output_info *info, int isLast, void *mainloop)
{
    static_cast<CAmMainloopPulse*>(mainloop)->sourceOutputInfoCallback(ctx, info, isLast);
}

void CAmMainloopPulse::contextStateChanged(pa_context *ctx)
{
    auto state = pa_context_get_state(ctx);

    logInfo("PULSE - Mainloop - contextStateChanged: ", state);

    if (state == PA_CONTEXT_FAILED)
    {
        logError("PULSE - Mianloop - context failed");
    }

    if (state == PA_CONTEXT_READY)
    {
        //when context is ready - subscriber for server events (we are mainly interested in sink inputs & source outputs)
        pa_context_set_subscribe_callback(ctx, sEventCallback, this);

        auto *op = pa_context_subscribe(ctx, PA_SUBSCRIPTION_MASK_ALL, sSubscriberCallback, this);

        if (op)
        {
            pa_operation_unref(op);
        }
        else
        {
            logError("Unable to create Pulse Audio operation: pa_context_subscribe");
        }
    }
}

void CAmMainloopPulse::eventCallback(pa_context *ctx, pa_subscription_event_type_t event, uint32_t idx)
{
    pa_subscription_event_type_t type = static_cast<pa_subscription_event_type_t>(event & PA_SUBSCRIPTION_EVENT_FACILITY_MASK);

    switch(event & PA_SUBSCRIPTION_EVENT_TYPE_MASK)
    {
        case PA_SUBSCRIPTION_EVENT_NEW:
        {
            onNewType(ctx, type, idx);

            break;
        }
        case PA_SUBSCRIPTION_EVENT_CHANGE:
        {
            break;
        }
        case PA_SUBSCRIPTION_EVENT_REMOVE:
        {
            onRemoveType(ctx, type, idx);

            break;
        }
    }
}

void CAmMainloopPulse::subscriberCallback(pa_context *ctx, int success)
{
    logInfo("PULSE - Mainloop - subscriberCallback: ", success);

    if (success)
    {
        auto op = pa_context_get_source_info_list(ctx, sSourceInfoCallback, this);

        if (op)
        {
            pa_operation_unref(op);
        }
        else
        {
            logError("Unable to create Pulse Audio operation: pa_context_get_source_info_list");
        }

        op = pa_context_get_sink_info_list(ctx, sSinkInfoCallback, this);

        if (op)
        {
            pa_operation_unref(op);
        }
        else
        {
            logError("Unable to create Pulse Audio operation: pa_context_get_sink_info_list");
        }
    }
    else
    {
        logError("routing_sender_subscriber_callback: success = false");
    }
}

void CAmMainloopPulse::sourceInfoCallback(pa_context *ctx, const pa_source_info *info, int isLast)
{
    if (!info)
    {
        return;
    }

    if (strcmp(info->name, "null") == 0)
    {
        mNullSource = info->index;
    }

    logInfo("PULSE - Mainloop - sourceInfoCallback: ", info->name, ", idx: ", info->index);

    mClient.onNewSource(info);
}

void CAmMainloopPulse::sinkInfoCallback(pa_context *ctx, const pa_sink_info *info, int isLast)
{
    if (!info)
    {
        return;
    }

    if (strcmp(info->name, "null") == 0)
    {
        mNullSink = info->index;
    }

    logInfo("PULSE - Mainloop - sinkInfoCallback: ", info->name, ", idx: ", info->index);

    mClient.onNewSink(info);
}

void CAmMainloopPulse::sinkInputInfoCallback(pa_context *ctx, const pa_sink_input_info *info, int isLast)
{
    if (!info)
    {
        return;
    }

    logInfo("PULSE - Mainloop - sinkInputInfoCallback: ", info->name, ", idx: ", info->index);

    mClient.onNewSinkInput(info);
}

void CAmMainloopPulse::sourceOutputInfoCallback(pa_context *ctx, const pa_source_output_info *info, int isLast)
{
    if (!info)
    {
        return;
    }

    logInfo("PULSE - Mainloop - sourceOutputInfoCallback: ", info->name, ", idx: ", info->index);

    mClient.onNewSourceOutput(info);
}

void CAmMainloopPulse::onNewType(pa_context *ctx, pa_subscription_event_type_t type, uint32_t idx)
{
    pa_operation *op = nullptr;

    switch(type)
    {
        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
        {
            logInfo("PULSE - Mainloop - NEW SINK INPUT idx: ", idx);

            op = pa_context_get_sink_input_info(ctx, idx, sSinkInputInfoCallback, this);

            break;
        }
        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
        {
            logInfo("PULSE - Mainloop - NEW SOURCE OUTPUT idx: ", idx);

            op = pa_context_get_source_output_info(ctx, idx, sSourceOutputInfoCallback, this);

            break;
        }
        case PA_SUBSCRIPTION_EVENT_SOURCE:
        {
            logInfo("PULSE - Mainloop - NEW SOURCE idx: ", idx);

            op = pa_context_get_source_info_by_index(ctx, idx, sSourceInfoCallback, this);

            break;
        }
        case PA_SUBSCRIPTION_EVENT_SINK:
        {
            logInfo("PULSE - Mainloop - NEW SINK idx: ", idx);

            op = pa_context_get_sink_info_by_index(ctx, idx, sSinkInfoCallback, this);

            break;
        }
        default:
        {
            break;
        }
    }

    if (op)
    {
        pa_operation_unref(op);
    }
}

void CAmMainloopPulse::onRemoveType(pa_context *ctx, pa_subscription_event_type_t type, uint32_t idx)
{
    switch(type)
    {
        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
        {
            logInfo("PULSE - Mainloop - REMOVE SINK INPUT idx: ", idx);

            mClient.onRemoveSinkInput(idx);

            break;
        }
        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
        {
            logInfo("PULSE - Mainloop - REMOVE SOURCE OUTPUT idx: ", idx);

            mClient.onRemoveSourceOutput(idx);

            break;
        }
        case PA_SUBSCRIPTION_EVENT_SOURCE:
        {
            logInfo("PULSE - Mainloop - NEW SOURCE idx: ", idx);

            mClient.onRemoveSource(idx);

            break;
        }
        case PA_SUBSCRIPTION_EVENT_SINK:
        {
            logInfo("PULSE - Mainloop - NEW SINK idx: ", idx);

            mClient.onRemoveSink(idx);

            break;
        }
        default:
        {
            break;
        }
    }
}
