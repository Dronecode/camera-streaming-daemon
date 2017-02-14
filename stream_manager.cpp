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
#include <cstring>
#include <dirent.h>
#include <system_error>

#include "log.h"
#include "stream_manager.h"

#define VIDEO_PREFIX "video"
#define DEFAULT_SERVICE_PORT 8554
#define DEFAULT_SERVICE_TYPE "_rtsp._udp"

StreamManager::StreamManager()
    : is_running(false)
    , avahi_publisher(streams, DEFAULT_SERVICE_PORT, DEFAULT_SERVICE_TYPE)
    , rtsp_server(streams, DEFAULT_SERVICE_PORT)
{
    stream_discovery();
};

StreamManager::~StreamManager()
{
    stop();
}

void StreamManager::stream_discovery()
{
    DIR *dir;
    struct dirent *f;

    streams.clear();
    errno = 0;
    if ((dir = opendir("/dev/")) == NULL)
        throw std::system_error(errno, std::generic_category(), "Invalid access to video devices");

    while ((f = readdir(dir)) != NULL) {
        if (std::strncmp(VIDEO_PREFIX, f->d_name, sizeof(VIDEO_PREFIX) - 1) == 0)
            streams.push_back(Stream{f->d_name});
    }
    closedir(dir);
}

void StreamManager::start()
{
    if (is_running)
        return;
    is_running = true;

    rtsp_server.start();
    avahi_publisher.start();
}

void StreamManager::stop()
{
    if (!is_running)
        return;
    is_running = false;

    avahi_publisher.stop();
    rtsp_server.stop();
}
