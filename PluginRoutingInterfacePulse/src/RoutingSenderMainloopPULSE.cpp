/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2014, Wind River Systems
 * Copyright (C) 2014, GENIVI Alliance
 *
 * This file is part of Pulse Audio Interface Routing Plugin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License (MPL), v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 *
 * List of changes:
 *
 * 21.08.2014, Adrian Scarlat, First version of the code;
 *                             Porting code from AM ver1.x to AM ver3.0;
 *                             Added Copyright and License information;
 */

/* Includes */

#include <pthread.h>
#include "CAmDltWrapper.h"
#include "RoutingSenderPULSE.h"
#include "RoutingSenderMainloopPULSE.h"

static pthread_t        *p_thread;
static pa_mainloop      *main_loop;

/* struct used for ramp_volume changing */
typedef struct ramp_volume
{
    uint32_t sink_input_index;
    uint32_t volume_ini;
    uint32_t volume_end;
    /* max ramp time in ms */
    uint16_t ramp_max_time;
    /* current delay between calls in ms */
    uint16_t ramp_crt_elapsed;
    /* aux used to calculate delay between calls */
    timespec start_time;
};
/* map used for storing ramp_volume information for sources */
std::map<uint32_t, ramp_volume> g_sinkInputId2rampVolume;

/* Defines */
#define QUIT_REASON_PA_DOWN     1
#define QUIT_REASON_AMGR_DOWN   2

DLT_IMPORT_CONTEXT(routingPulse)

bool routing_sender_create_mainloop(void *thiz)
{
    if (!thiz) {
        logError("Can not create an working thread for Pulse Audio without a Routing plugin\n");
        return false;
    }
    p_thread = (pthread_t *) malloc(sizeof(pthread_t));
    pthread_create(p_thread, NULL, routing_sender_start_mainloop, thiz);
    return true;
}


void* routing_sender_start_mainloop(void *thiz)
{
    pa_mainloop_api  *mainloop_api;
    pa_proplist      *proplist;
    pa_context       *context;
    int              ret = 0;

    if (!thiz) {
        logError("Can not create an working thread for Pulse Audio without a Routing plugin\n");
        return NULL;
    }

    proplist = pa_proplist_new();
    if (pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "RoutingPULSE") < 0) {
        logError("Can not prepare Pulse Audio main loop: pa_proplist_sets\n");
        goto end;
    }

    if (!(main_loop = pa_mainloop_new()))
    {
        logError("Can not prepare Pulse Audio main loop: pa_mainloop_new\n");
        goto end;
    }

    if (!(mainloop_api = pa_mainloop_get_api(main_loop)))
    {
        logError("Can not prepare Pulse Audio main loop: pa_mainloop_get_api\n");
        goto end;
    }

    if (!(context = pa_context_new_with_proplist(mainloop_api, NULL, proplist)))
    {
        logError("Can not prepare Pulse Audio main loop: pa_context_new_with_proplist\n");
        goto end;
    }

    pa_context_set_state_callback(context, routing_sender_context_state_callback, thiz);

    if (pa_context_connect(context, NULL, pa_context_flags_t(0), NULL) < 0)
    {
        logError("Can not prepare Pulse Audio main loop: pa_context_new_with_proplist\n");
        goto end;
    }
    ((RoutingSenderPULSE *) thiz)->setPAContext(context);

    pa_mainloop_run(main_loop, &ret);

end:
    if (ret == QUIT_REASON_PA_DOWN)
    {
        ret = 0;
        //mail loop ended
        if (context)
        {
            pa_context_disconnect(context);
            pa_context_unref(context);
            context = NULL;
        }

        if (proplist)
        {
            pa_proplist_free(proplist);
            proplist = NULL;
        }

        if (main_loop)
        {
            //pa_signal_done();
            pa_signal_done();
            pa_mainloop_free(main_loop);
            main_loop = NULL;
        }

        //... pulse audio is down ... retry to connect
        usleep(100000);
        routing_sender_start_mainloop(thiz);
    }
    //TDOO: else if -check other value for "ret"
    else
    {
        return thiz;
    }
}


void routing_sender_pa_event_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *thiz)
{
    switch(t)
    {
        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
        {
            pa_context_get_sink_input_info(
                    c, idx, routing_sender_get_sink_input_info_callback, thiz);
            break;
        }
        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
        {
            pa_context_get_sink_input_info(
                    c, idx, routing_sender_get_sink_input_info_callback, thiz);
            break;
        }
        default:
        {
            logInfo("Pulse Audio event", t, "was ignored");
        }
    }
}


void routing_sender_get_sink_input_info_callback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata)
{
    (void) eol;

    RoutingSenderPULSE* thiz = (RoutingSenderPULSE*) userdata;
    if (!thiz)
    {
        logError("pa_context_get_sink_input_info was called with wrong params\n");
        return;
    }

    /* init map of ramp_volumes, usefull later */
    if (i != NULL && strcmp(i->name,"null") != 0)
    {
        ramp_volume source_ramp_volume;
        source_ramp_volume.sink_input_index = i->index;
        source_ramp_volume.ramp_crt_elapsed = 0;
        g_sinkInputId2rampVolume.insert(std::make_pair(i->index, source_ramp_volume));
    }

    thiz->getSinkInputInfoCallback(c, i, userdata);
}


void routing_sender_get_source_output_info_callback(pa_context *c, const pa_source_output_info *i, int eol, void *userdata)
{
    (void) eol;

    RoutingSenderPULSE* thiz = (RoutingSenderPULSE*) userdata;
    if (!thiz)
    {
        logError("pa_context_get_surce_output_info was called with wrong params\n");
        return;
    }
    thiz->getSourceOutputInfoCallback(c, i, userdata);
}



void routing_sender_get_sink_info_callback(pa_context *c, const pa_sink_info *i, int is_last, void *userdata)
{
    RoutingSenderPULSE* thiz = (RoutingSenderPULSE*) userdata;
    if (!thiz)
    {
        logError("pa_context_get_sink_info was called with wrong params\n");
        return;
    }
    thiz->getSinkInfoCallback(c, i, is_last, userdata);
}


void routing_sender_get_source_info_callback(pa_context *c, const pa_source_info *i, int is_last, void *userdata)
{
    RoutingSenderPULSE* thiz = (RoutingSenderPULSE*) userdata;
    if (!thiz)
    {
        logError("pa_context_get_source_info was called with wrong params\n");
        return;
    }
    thiz->getSourceInfoCallback(c, i, is_last, userdata);
}


void routing_sender_subscriber_callback(pa_context *c, int success, void *thiz) {
    if (success)
    {
        pa_operation *o = pa_context_get_sink_info_list(c, routing_sender_get_sink_info_callback, thiz);
        if (o)
        {
            pa_operation_unref(o);
        }
        else
        {
            logError("Unable to create Pulse Audio operation:",
                    "pa_context_get_sink_info_list");
        }
    }
    else
    {
        logError("routing_sender_subscriber_callback: success = false");
    }
}


void routing_sender_context_state_callback(pa_context *c, void *thiz)
 {

    if (pa_context_get_state(c) == PA_CONTEXT_FAILED)
    {
        pa_mainloop_quit(main_loop, QUIT_REASON_PA_DOWN);
    }
    if (pa_context_get_state(c) == PA_CONTEXT_READY)
    {
        //when context is ready - subscriber for server events (we are mainly interested in sink inputs & source outputs)
        pa_context_set_subscribe_callback(c, routing_sender_pa_event_callback, thiz);
        pa_operation *o = pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_ALL, routing_sender_subscriber_callback, thiz);
        if (o)
        {
            pa_operation_unref(o);
        }
        else
        {
            logError("Unable to create Pulse Audio operation:",
                            "pa_context_subscribe");
        }
    }
    //other states are not relevant
}


bool routing_sender_get_source_info(pa_context *c, void *thiz) {
    if (pa_context_get_state(c) == PA_CONTEXT_READY)
    {
        pa_operation *o = pa_context_get_source_info_list(c, routing_sender_get_source_info_callback, thiz);
        if (o)
        {
            pa_operation_unref(o);
        }
        else
        {
            logError("Unable to create Pulse Audio operation:",
                    "pa_context_get_sink_info_list");
            return false;
        }
    }
    else
    {
        logError("Can not get Pulse Audio sources info - context not ready\n");
        return false;
    }

    return true;
}


bool routing_sender_move_sink_input(pa_context *c, uint32_t sink_input_index, uint32_t sink_index, void *thiz)
{
    if (pa_context_get_state(c) == PA_CONTEXT_READY)
    {
        pa_operation *o = pa_context_move_sink_input_by_index(c, sink_input_index, sink_index, NULL, thiz);
        if (o)
        {
            pa_operation_unref(o);
        }
        else
        {
            logError("Unable to create Pulse Audio operation:",
                    "pa_context_move_sink_input_by_index");
            return false;
        }
    }
    else
    {
        logError("Can not move sink input - context not ready\n");
        return false;
    }
    return true;
}


bool routing_sender_move_source_output(pa_context *c, uint32_t source_output_index, uint32_t source_index, void *thiz)
{
    if (pa_context_get_state(c) == PA_CONTEXT_READY)
    {
        pa_operation *o = pa_context_move_source_output_by_index(c, source_output_index, source_index, NULL, thiz);
        if (o)
        {
            pa_operation_unref(o);
        }
        else
        {
            logError("Unable to create Pulse Audio operation:",
                    "pa_context_set_sink_input_volume");
            return false;
        }
    }
    else
    {
        logError("Can not move source output - context not ready\n");
        return false;
    }
    return true;
}

static inline uint16_t timespec2mili(const timespec & time)
{
    return (uint16_t)((time.tv_nsec == -1 && time.tv_sec == -1) ?
                      -1 :
                      time.tv_sec * 1000 + time.tv_nsec / 1000000);
}

/* Considers that time2 > time1 */
static inline uint16_t timespec2DeltaMili(const timespec & time1, const timespec & time2)
{
    timespec l_deltaTime;
    l_deltaTime.tv_sec = time2.tv_sec - time1.tv_sec;
    l_deltaTime.tv_nsec = time2.tv_nsec - time1.tv_nsec;
    return timespec2mili(l_deltaTime);
}

static void routing_sender_sink_input_volume_cb(pa_context *c, int success, void *data)
{
    logInfo("routing_sender_sink_input_volume_cb: success=", success, " data=", data);
    if (success)
    {
        ramp_volume * l_ramp_volume = (ramp_volume *)data;
        timespec l_endTime;
        clock_gettime(0, &l_endTime);
        l_ramp_volume->ramp_crt_elapsed = timespec2DeltaMili(l_ramp_volume->start_time, l_endTime);
        if (l_ramp_volume->ramp_crt_elapsed >= l_ramp_volume->ramp_max_time)
        {
            return;
        }

        logInfo("routing_sender_sink_input_volume_cb: ms elapsed=", l_ramp_volume->ramp_crt_elapsed, " of ", l_ramp_volume->ramp_max_time);

        /* ######## Calculate new volume with formula: ##########
                                crt_time x ( vol_end - vol_ini )
           new_vol = vol_ini + ----------------------------------
                                            max_time
           ###################################################### */
        uint32_t new_volume =
                l_ramp_volume->volume_ini +
                ( ( l_ramp_volume->ramp_crt_elapsed * ( l_ramp_volume->volume_end - l_ramp_volume->volume_ini ) ) / l_ramp_volume->ramp_max_time );
        logInfo("routing_sender_sink_input_volume_cb: vol_ini=",l_ramp_volume->volume_ini,"vol_crt=",new_volume,"vol_end=",l_ramp_volume->volume_end);


        /* ***** Set volume again ***** */
        pa_cvolume *volumeCh = (pa_cvolume *) malloc(sizeof(pa_cvolume));
        volumeCh->channels = 1;//TODO: check is stream is mono / stereo
        volumeCh->values[0] = new_volume;

        pa_operation *o = pa_context_set_sink_input_volume(c, l_ramp_volume->sink_input_index, volumeCh, NULL, NULL);
        if (o)
        {
            pa_operation_unref(o);
        }
        else
        {
            logError("Unable to create Pulse Audio operation:",
                    "pa_context_set_sink_input_volume");
            return ;
        }
        usleep(10000);

        volumeCh->channels = 2;//TODO: check is stream is mono / stereo
        volumeCh->values[0] = new_volume;
        volumeCh->values[1] = new_volume;
        logInfo("routing_sender_sink_input_volume_cb: will set vol=", new_volume);
        o = pa_context_set_sink_input_volume(c, l_ramp_volume->sink_input_index, volumeCh, routing_sender_sink_input_volume_cb, l_ramp_volume);
        if (o)
        {
            pa_operation_unref(o);
        }
        else
        {
            logError("Unable to create Pulse Audio operation:",
                    "pa_context_set_sink_input_volume");
            return ;
        }
    }

}

bool routing_sender_sink_input_volume_ramp(pa_context *c, uint32_t sink_input_index, uint32_t crt_volume, uint32_t volume, uint16_t ramp_time, void *thiz)
{
    if (pa_context_get_state(c) == PA_CONTEXT_READY)
    {
        /* before everything, check to see if ramp_volume struct exists */
        std::map<uint32_t, ramp_volume>::iterator iter = g_sinkInputId2rampVolume.find(sink_input_index);
        std::map<uint32_t, ramp_volume>::iterator iterEnd = g_sinkInputId2rampVolume.end();
        if (iter != iterEnd)
        {
            /* set test volume with only 1 unit more or less, just to see how callback responds */
            pa_volume_t test_volume = ((crt_volume * MAX_PULSE_VOLUME) / 100);
            test_volume += volume > crt_volume ? 1 : -1;

            pa_cvolume *volumeCh = (pa_cvolume *) malloc(sizeof(pa_cvolume));
            volumeCh->channels = 1;//TODO: check is stream is mono / stereo
            volumeCh->values[0] = test_volume;

            pa_operation *o = pa_context_set_sink_input_volume(c, sink_input_index, volumeCh, NULL, NULL);
            if (o)
            {
                pa_operation_unref(o);
            }
            else
            {
                logError("Unable to create Pulse Audio operation:",
                        "pa_context_set_sink_input_volume");
                return false;
            }

            volumeCh->channels = 2;//TODO: check is stream is mono / stereo
            volumeCh->values[0] = test_volume;
            volumeCh->values[1] = test_volume;

            ramp_volume * l_ramp_volume = (ramp_volume *) &iter->second;
            logInfo("routing_sender_sink_input_volume_ramp: searching ",sink_input_index,"found ramp_vlume struct with sinkInputId ",l_ramp_volume->sink_input_index," (should be equal) ");
            l_ramp_volume->volume_ini = (crt_volume * MAX_PULSE_VOLUME) / 100;
            l_ramp_volume->volume_end = (volume * MAX_PULSE_VOLUME) / 100;
            l_ramp_volume->ramp_max_time = ramp_time;
            clock_gettime(0, &l_ramp_volume->start_time);
            l_ramp_volume->ramp_crt_elapsed = 0;
            logInfo("routing_sender_sink_input_volume_ramp: will set vol=", test_volume);
            o = pa_context_set_sink_input_volume(c, sink_input_index, volumeCh, routing_sender_sink_input_volume_cb, l_ramp_volume);
            if (o)
            {
                pa_operation_unref(o);
            }
            else
            {
                logError("Unable to create Pulse Audio operation:",
                        "pa_context_set_sink_input_volume");
                return false;
            }
        }
        else
        {
            logInfo("routing_sender_sink_input_volume_ramp: didn't find struct with sinkInputId ", sink_input_index);
            /* make-it the old traditional way */
            return routing_sender_sink_input_volume(c, sink_input_index, volume, thiz);
        }


    }
    else
    {
        logError("Can not set sink input volume - context not ready\n");
        return false;
    }
    return true;
}

bool routing_sender_sink_input_volume(pa_context *c, uint32_t sink_input_index, uint32_t volume, void *thiz)
{
    if (pa_context_get_state(c) == PA_CONTEXT_READY)
    {
        pa_cvolume *volumeCh = (pa_cvolume *) malloc(sizeof(pa_cvolume));
        volumeCh->channels = 1;//TODO: check is stream is mono / stereo
        volumeCh->values[0] = (volume * MAX_PULSE_VOLUME) / 100;

        pa_operation *o = pa_context_set_sink_input_volume(c, sink_input_index, volumeCh, NULL, thiz);
        if (o)
        {
            pa_operation_unref(o);
        }
        else
        {
            logError("Unable to create Pulse Audio operation:",
                    "pa_context_set_sink_input_volume");
            return false;
        }

        volumeCh->channels = 2;//TODO: check is stream is mono / stereo
        volumeCh->values[0] = (volume * MAX_PULSE_VOLUME) / 100;
        volumeCh->values[1] = (volume * MAX_PULSE_VOLUME) / 100;

        o = pa_context_set_sink_input_volume(c, sink_input_index, volumeCh, NULL, NULL);
        if (o)
        {
            pa_operation_unref(o);
        }
        else
        {
            logError("Unable to create Pulse Audio operation:",
                    "pa_context_set_sink_input_volume");
            return false;
        }
    }
    else
    {
        logError("Can not set sink input volume - context not ready\n");
        return false;
    }
    return true;
}


bool routing_sender_sink_input_mute(pa_context *c, uint32_t sink_input_index, bool mute, void *thiz)
{
    if (pa_context_get_state(c) == PA_CONTEXT_READY)
    {
        pa_operation *o = pa_context_set_sink_input_mute(
            c, sink_input_index, mute ? 1 : 0, NULL, thiz);
        if (o)
        {
            pa_operation_unref(o);
        }
        else
        {
            logError("Unable to create Pulse Audio operation:",
                    "pa_context_set_sink_input_mute");
            return false;
        }
    }
    else
    {
        logError("Can not set sink input volume - context not ready\n");
        return false;
    }
    return true;
}


bool routing_sender_sink_volume(pa_context *c, uint32_t sink_index, uint32_t volume, void *thiz)
{
    if (pa_context_get_state(c) == PA_CONTEXT_READY)
    {
        pa_cvolume *volumeCh = (pa_cvolume *) malloc(sizeof(pa_cvolume));
        volumeCh->channels = 2;//TODO: check is stream is mono / stereo
        volumeCh->values[0] = (volume * MAX_PULSE_VOLUME) / 100;
        volumeCh->values[1] = (volume * MAX_PULSE_VOLUME) / 100;

        pa_operation *o = pa_context_set_sink_volume_by_index(c, sink_index, volumeCh, NULL, thiz);
        if (o)
        {
            pa_operation_unref(o);
        }
        else
        {
            logError("Unable to create Pulse Audio operation:",
                    "pa_context_set_sink_input_volume");
            return false;
        }
    }
    else
    {
        logError("Can not set sink input volume - context not ready\n");
        return false;
    }
    return true;
}

//TODO - implements mute/un-mute and sink suspend(pause)
//TODO - IMPORTANT !! implement volume change for sink input even multiple sink inputs are created during one connection
