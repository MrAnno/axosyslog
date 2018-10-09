/*
 * Copyright (c) 2017 Balabit
 * Copyright (c) 2017 László Várady
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "logproto-http-server.h"
#include "buffer.h"
#include "http-parser.h"
#include "http-message.h"
#include "http-errors.h"
#include "syslog-ng.h"
#include "messages.h"

#include <errno.h>

//TODO: http-message-size megák lehetnek, induljunk egy kisebb értékkel és növeljük meg max-ig
//TODO: követni az üzenet elejét, ezzel lehetne ritkábban splitelni (buffer_reset használni lehet?)
//TODO: a kimenő buffert mi állítjuk össze mindig? akkor nem kell buffer limit, jól kell méretezni

typedef enum _State
{
  STATE_RECEIVE_HTTP_REQUEST,
  STATE_SEND_HTTP_RESPONSE,
  STATE_HTTP_ERROR,
  STATE_PROCESS_LOG_MESSAGES
} State;

typedef struct _ExtractMessagesCallback
{
  LPHTTPExtractMessagesFunc func;
  gpointer user_data;
} ExtractMessagesCallback;

typedef struct _CreateResponseCallback
{
  LPHTTPCreateResponse func;
  gpointer user_data;
} CreateResponseCallback;

struct _LogProtoHTTPServer
{
  LogProtoServer super;

  State state;
  Buffer in_buffer;
  Buffer out_buffer;
  HTTPParser *http_parser;

  GQueue *pending_messages;

  ExtractMessagesCallback extract_messages;
  CreateResponseCallback create_response;
};

static gboolean
log_proto_http_server_prepare(LogProtoServer *s, GIOCondition *cond)
{
  LogProtoHTTPServer *self = (LogProtoHTTPServer *) s;

  if (self->state == STATE_PROCESS_LOG_MESSAGES)
    return TRUE;

  GIOCondition proto_io_direction;
  gboolean unprocessed_data_in_buffer;
  if (self->state == STATE_SEND_HTTP_RESPONSE || self->state == STATE_HTTP_ERROR)
    {
      unprocessed_data_in_buffer = buffer_size(&self->out_buffer) != 0;
      proto_io_direction = G_IO_OUT;
    }
  else if (self->state == STATE_RECEIVE_HTTP_REQUEST)
    {
      unprocessed_data_in_buffer = buffer_size(&self->in_buffer) != 0;
      proto_io_direction = G_IO_IN;
    }
  else
    g_assert_not_reached();

  *cond = self->super.transport->cond;

  /* if there's no pending I/O in the transport layer */
  if (*cond == 0)
    *cond = proto_io_direction;

  return unprocessed_data_in_buffer;
}

static LogProtoStatus
_convert_io_status_to_log_proto_status(GIOStatus io_status)
{
  switch (io_status)
    {
    case G_IO_STATUS_NORMAL:
    case G_IO_STATUS_AGAIN:
      return LPS_SUCCESS;
    case G_IO_STATUS_ERROR:
      return LPS_ERROR;
    case G_IO_STATUS_EOF:
      return LPS_EOF;
    default:
      g_assert_not_reached();
    }
}

static GIOStatus
log_proto_http_server_fetch_data(LogProtoHTTPServer *self, LogTransportAuxData *aux)
{
  if (G_UNLIKELY(!buffer_allocated(&self->in_buffer)))
    buffer_allocate(&self->in_buffer, self->super.options->init_buffer_size);

  gssize rc = log_transport_read(self->super.transport, buffer_end(&self->in_buffer),
                                 buffer_unused_capacity(&self->in_buffer), aux);

  if (rc < 0)
    {
      if (errno != EAGAIN)
        {
          msg_error("I/O error occurred while reading HTTP request",
                    evt_tag_int(EVT_TAG_FD, self->super.transport->fd),
                    evt_tag_errno(EVT_TAG_OSERROR, errno));
          return G_IO_STATUS_ERROR;
        }
      else
        return G_IO_STATUS_AGAIN;
    }
  else if (rc == 0)
    {
      msg_verbose("EOF occurred while reading", evt_tag_int(EVT_TAG_FD, self->super.transport->fd));
      return G_IO_STATUS_EOF;
    }

  buffer_increase_size(&self->in_buffer, rc);
  return G_IO_STATUS_NORMAL;
}

static void
log_proto_http_server_set_error_response(LogProtoHTTPServer *self, HTTPStatusCode error)
{
  if (G_UNLIKELY(!buffer_allocated(&self->out_buffer)))
    buffer_allocate(&self->out_buffer, MAX(self->super.options->init_buffer_size, 512));

  //TODO: HTTPStatusCode is not used
  buffer_reset(&self->out_buffer);
  gsize response_length = sizeof(HTTP_ERROR_500_PAGE) - 1;
  memcpy(self->out_buffer.buffer, HTTP_ERROR_500_PAGE, response_length);
  buffer_increase_size(&self->out_buffer, response_length);

  self->state = STATE_HTTP_ERROR;
}

static HTTPRequest *
log_proto_http_server_parse_request(LogProtoHTTPServer *self, LogProtoStatus status)
{
  if (status == LPS_EOF)
    {
      if (!http_parser_signal_end_of_stream(self->http_parser))
        goto http_error;
    }
  else
    {
      const gchar *buf_start = (const gchar *) buffer_start(&self->in_buffer);
      gsize buf_bytes = buffer_size(&self->in_buffer);

      gsize consumed_bytes;
      if (!http_parser_feed(self->http_parser, buf_start, buf_bytes, &consumed_bytes))
        goto http_error;

      buffer_consume(&self->in_buffer, consumed_bytes);
    }

  if (http_parser_is_message_complete(self->http_parser))
    {
      buffer_split(&self->in_buffer);

      msg_debug("Message is complete");
      return (HTTPRequest *) http_parser_steal_message(self->http_parser);
    }

  if (self->in_buffer.size >= self->in_buffer.capacity) //TODO: self->super.super.options->max_msg_size
    {
      log_proto_http_server_set_error_response(self, HTTP_PAYLOAD_TOO_LARGE);
      msg_error("HTTP request is too long");
      return NULL;
    }

  return NULL;

http_error:
  log_proto_http_server_set_error_response(self, HTTP_BAD_REQUEST);

  GError *http_error = http_parser_get_last_error(self->http_parser);
  msg_error("Invalid HTTP request", evt_tag_str("http_error", http_error->message));
  g_error_free(http_error);

  return NULL;
}

static HTTPRequest *
log_proto_http_server_receive_request(LogProtoHTTPServer *self, LogTransportAuxData *aux, LogProtoStatus *status)
{
  /* This function returns when an HTTP request is complete or when error or EAGAIN occurs,
     so it can not be used as an "edge-triggered" reader */
  while (self->state == STATE_RECEIVE_HTTP_REQUEST)
    {
      if (buffer_is_empty(&self->in_buffer))
        {
          GIOStatus io_status = log_proto_http_server_fetch_data(self, aux);
          *status = _convert_io_status_to_log_proto_status(io_status);

          if (io_status == G_IO_STATUS_AGAIN || *status == LPS_ERROR)
            return NULL;
        }

      HTTPRequest *http_request = log_proto_http_server_parse_request(self, *status);
      if (http_request)
        return http_request;

      if (*status != LPS_SUCCESS)
        return NULL;
    }

  return NULL;
}

static GIOStatus
log_proto_http_server_flush_response(LogProtoHTTPServer *self)
{
  gssize rc = log_transport_write(self->super.transport, buffer_start(&self->out_buffer),
                                  buffer_size(&self->out_buffer));

  if (rc < 0)
    {
      if (errno != EAGAIN)
        {
          msg_error("I/O error occurred while sending HTTP response",
                    evt_tag_int(EVT_TAG_FD, self->super.transport->fd),
                    evt_tag_errno(EVT_TAG_OSERROR, errno));
          return G_IO_STATUS_ERROR;
        }
      else
        return G_IO_STATUS_AGAIN;
    }

  buffer_consume(&self->out_buffer, rc);
  return G_IO_STATUS_NORMAL;
}

static LogProtoStatus
log_proto_http_server_send_response(LogProtoHTTPServer *self)
{
  GIOStatus io_status = log_proto_http_server_flush_response(self);
  LogProtoStatus status = _convert_io_status_to_log_proto_status(io_status);

  if (!buffer_is_empty(&self->out_buffer))
    return status;

  if (self->state == STATE_HTTP_ERROR)
    return LPS_ERROR;

  self->state = STATE_RECEIVE_HTTP_REQUEST;
  return status;
}

static void
log_proto_http_server_extract_messages(LogProtoHTTPServer *self, const HTTPRequest *http_request)
{
  if (!self->extract_messages.func)
    goto skip_log_message_processing;

  self->pending_messages = self->extract_messages.func(http_request, self->extract_messages.user_data);

  if (!self->pending_messages)
    goto skip_log_message_processing;

  if (g_queue_get_length(self->pending_messages) == 0)
    {
      g_queue_free(self->pending_messages);
      self->pending_messages = NULL;
      goto skip_log_message_processing;
    }

  self->state = STATE_PROCESS_LOG_MESSAGES;

skip_log_message_processing:
  self->state = STATE_SEND_HTTP_RESPONSE;
}

static void
log_proto_http_server_create_response(LogProtoHTTPServer *self, const HTTPRequest *http_request)
{
  if (G_UNLIKELY(!buffer_allocated(&self->out_buffer)))
    buffer_allocate(&self->out_buffer, MAX(self->super.options->init_buffer_size, 512));

  if (!self->create_response.func)
    {
      buffer_reset(&self->out_buffer);
      memcpy(self->out_buffer.buffer, "HTTP/1.1 200 OK\r\nContent-Length:5\r\n\r\nhello", 42);
      buffer_increase_size(&self->out_buffer, 42);
      return;
    }

  HTTPResponse *http_response = self->create_response.func(http_request, self->create_response.user_data);

  //TODO: bufi összeállít
  //TODO: 500 ha nincs response vagy nincs callback beállítva

  http_response_free(http_response);
}

static void
log_proto_http_server_extract_log_messages_and_create_response(LogProtoHTTPServer *self, HTTPRequest *http_request)
{
  log_proto_http_server_extract_messages(self, http_request);
  log_proto_http_server_create_response(self, http_request);
}

static LogMessage *
log_proto_http_server_process_log_messages(LogProtoHTTPServer *self)
{
  LogMessage *log_message = g_queue_pop_head(self->pending_messages);
  if (!log_message)
    {
      g_queue_free(self->pending_messages);
      self->pending_messages = NULL;
      self->state = STATE_SEND_HTTP_RESPONSE;
    }

  return log_message;
}

static LogProtoStatus
log_proto_http_server_process(LogProtoServer *s, const guchar **msg, gsize *msg_len, gboolean *may_read,
                              LogTransportAuxData *aux, Bookmark *bookmark)
{
  LogProtoHTTPServer *self = (LogProtoHTTPServer *) s;
  LogProtoStatus status = LPS_SUCCESS;

  // the loop decreases the load of main loop
  while(1)
    {
      switch (self->state)
        {
        case STATE_RECEIVE_HTTP_REQUEST:
          ;
          HTTPRequest *http_request = log_proto_http_server_receive_request(self, aux, &status);
          if (http_request)
            {
              log_proto_http_server_extract_log_messages_and_create_response(self, http_request);
              http_request_free(http_request);
            }
          else
            {
              return status;
            }
          break;

        case STATE_PROCESS_LOG_MESSAGES:
          ;
          LogMessage *log_message = log_proto_http_server_process_log_messages(self);
          (void) log_message;
          if (log_message)
            return LPS_SUCCESS;
          break;

        case STATE_SEND_HTTP_RESPONSE:
        case STATE_HTTP_ERROR:
          status = log_proto_http_server_send_response(self);
          if (self->state != STATE_RECEIVE_HTTP_REQUEST)
            return status;
          break;

        default:
          g_assert_not_reached();
        }
    }

  if (status != LPS_SUCCESS)
    log_transport_aux_data_reinit(aux);

  return status;
}

static void
log_proto_http_server_free(LogProtoServer *s)
{
  LogProtoHTTPServer *self = (LogProtoHTTPServer *) s;

  buffer_deallocate(&self->in_buffer);
  buffer_deallocate(&self->out_buffer);
  http_parser_free(self->http_parser);

  log_proto_server_free_method(s);
}

void
log_proto_http_server_set_extract_messages(LogProtoHTTPServer *self, LPHTTPExtractMessagesFunc extract_messages,
                                           gpointer user_data)
{
  self->extract_messages.func = extract_messages;
  self->extract_messages.user_data = user_data;
}

void
log_proto_http_server_set_create_response(LogProtoHTTPServer *self, LPHTTPCreateResponse create_response,
                                          gpointer user_data)
{
  self->create_response.func = create_response;
  self->create_response.user_data = user_data;
}

LogProtoServer *
log_proto_http_server_new(LogTransport *transport, const LogProtoServerOptions *options)
{
  LogProtoHTTPServer *self = g_new0(LogProtoHTTPServer, 1);

  log_proto_server_init(&self->super, transport, options);
  self->super.prepare = log_proto_http_server_prepare;
  self->super.fetch = log_proto_http_server_process;
  self->super.free_fn = log_proto_http_server_free;

  self->http_parser = http_request_parser_new();

  return &self->super;
}
