/*
 * Copyright (c) 2021 One Identity
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#ifndef MQTT_DESTINATION_WORKER_H_INCLUDED
#define MQTT_DESTINATION_WORKER_H_INCLUDED

#include "logthrdest/logthrdestdrv.h"
#include "thread-utils.h"

#include <iv.h>

#include <MQTTClient.h>

typedef struct _MQTTDestinationWorker
{
  LogThreadedDestWorker super;
  MQTTClient client;
  GString *topic;

  GString *string_to_write;
  GString *topic_name_buffer;

  struct iv_timer yield_timer;
} MQTTDestinationWorker;

LogThreadedDestWorker *mqtt_dw_new(LogThreadedDestDriver *o, gint worker_index);


#endif /* MQTT_DESTINATION_WORKER_H_INCLUDED */
