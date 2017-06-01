/*
 * This file is part of the Camera Streaming Daemon
 *
 * Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <assert.h>
#include <dirent.h>
#include <system_error>

#include "gstreamer_pipeline_builder.h"
#include "log.h"
#include "stream_builder.h"
#include "stream_manager.h"

#define DEFAULT_SERVICE_PORT 8554
#define DEFAULT_SERVICE_TYPE "_rtsp._udp"

StreamManager::StreamManager()
    : is_running(false)
    , avahi_publisher(streams, DEFAULT_SERVICE_PORT, DEFAULT_SERVICE_TYPE)
    , rtsp_server(streams, DEFAULT_SERVICE_PORT)
    , mavlink_server(streams){};

StreamManager::~StreamManager()
{
    stop();
}

void StreamManager::start()
{
    if (is_running)
        return;
    is_running = true;

    rtsp_server.start();
    avahi_publisher.start();
    mavlink_server.start();
}

void StreamManager::init_streams(ConfFile &conf)
{
    GstreamerPipelineBuilder::get_instance().apply_configs(conf);
    for (StreamBuilder *builder : StreamBuilder::get_builders())
        for (Stream *s : builder->build_streams(conf)) {
            log_debug("Adding stream %s (%s)", s->get_path().c_str(), s->get_name().c_str());
            addStream(s);
        }
    StreamBuilder::get_builders().clear();
}

void StreamManager::stop()
{
    if (!is_running)
        return;
    is_running = false;

    avahi_publisher.stop();
    rtsp_server.stop();
    mavlink_server.stop();
}

void StreamManager::addStream(Stream *stream)
{
    static unsigned int next_id = 0;
    assert(next_id < UINT_MAX);

    stream->id = ++next_id;
    streams.push_back(std::unique_ptr<Stream>(stream));
}
