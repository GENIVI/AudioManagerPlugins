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

#ifndef ROUTINGSENDERMAINLOPPPULSE_H_
#define ROUTINGSENDERMAINLOPPPULSE_H_

bool routing_sender_create_mainloop(void *thiz);

void * routing_sender_start_mainloop(void *thiz);

void routing_sender_get_sink_info_callback(
        pa_context *c,
        const pa_sink_info *i,
        int is_last, void *thiz);

void routing_sender_context_state_callback(pa_context *c, void *thiz);

bool routing_sender_get_source_info(pa_context *c, void *thiz);

void routing_sender_pa_event_callback(
        pa_context *c,
        pa_subscription_event_type_t t,
        uint32_t idx, void *thiz);

void routing_sender_get_sink_input_info_callback(
        pa_context *c,
        const pa_sink_input_info *i,
        int eol,
        void *thiz);

void routing_sender_get_source_output_info_callback(
        pa_context *c,
        const pa_source_output_info *i,
        int eol,
        void *userdata);

bool routing_sender_move_sink_input(
        pa_context *c,
        uint32_t sink_input_index,
        uint32_t sink_index,
        void *thiz);

bool routing_sender_move_source_output(
        pa_context *c,
        uint32_t source_output_index,
        uint32_t source_index,
        void *thiz);

bool routing_sender_sink_input_volume_ramp(
        pa_context *c,
        uint32_t sink_input_index,
        pa_volume_t crt_volume,
        pa_volume_t volume,
        uint16_t ramp_time,
        void *thiz);

bool routing_sender_sink_input_volume(
        pa_context *c,
        uint32_t sink_input_index,
        pa_volume_t volume,
        void *thiz);

bool routing_sender_sink_input_mute(
        pa_context *c,
        uint32_t sink_input_index,
        bool mute,
        void *thiz);

bool routing_sender_sink_volume(
        pa_context *c,
        uint32_t sink_index,
        pa_volume_t volume,
        void *thiz);

#endif
