/*
 * Copyright (c) 2024 Axoflow
 * Copyright (c) 2023-2024 Attila Szakacs <attila.szakacs@axoflow.com>
 * Copyright (c) 2023 László Várady
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

#ifndef GRPC_DEST_HPP
#define GRPC_DEST_HPP

#include "grpc-dest.h"

#include "compat/cpp-start.h"
#include "logthrdest/logthrdestdrv.h"
#include "compat/cpp-end.h"

#include "credentials/grpc-credentials-builder.hpp"
#include "metrics/grpc-metrics.hpp"
#include "schema/log-message-protobuf-formatter.hpp"

#include <grpcpp/server.h>

#include <list>
#include <sstream>

#define GRPC_DEST_RESPONSE_ACTIONS_ARRAY_LEN (64)

namespace syslogng {
namespace grpc {

class DestDriver
{
public:
  DestDriver(GrpcDestDriver *s);
  virtual ~DestDriver();

  virtual bool init();
  virtual bool deinit();
  virtual const char *format_stats_key(StatsClusterKeyBuilder *kb) = 0;
  virtual const char *generate_persist_name() = 0;
  virtual LogThreadedDestWorker *construct_worker(int worker_index) = 0;
  virtual bool handle_response(const ::grpc::Status &status, LogThreadedResult *ltr);

  void set_url(const char *u)
  {
    this->url.assign(u);
  }

  const std::string &get_url() const
  {
    return this->url;
  }

  void set_compression(bool c)
  {
    this->compression = c;
  }

  bool get_compression() const
  {
    return this->compression;
  }

  void set_batch_bytes(size_t b)
  {
    this->batch_bytes = b;
  }

  size_t get_batch_bytes() const
  {
    return this->batch_bytes;
  }

  void set_keepalive_time(int t)
  {
    this->keepalive_time = t;
  }

  void set_keepalive_timeout(int t)
  {
    this->keepalive_timeout = t;
  }

  void set_keepalive_max_pings(int p)
  {
    this->keepalive_max_pings_without_data = p;
  }

  void add_extra_channel_arg(std::string name, long value)
  {
    this->int_extra_channel_args.push_back(std::make_pair(name, value));
  }

  void add_extra_channel_arg(std::string name, std::string value)
  {
    this->string_extra_channel_args.push_back(std::make_pair(name, value));
  }

  bool add_header(std::string name, LogTemplate *value)
  {
    bool is_literal_string = log_template_is_literal_string(value);

    if (!this->dynamic_headers_enabled && !is_literal_string)
      return false;

    std::transform(name.begin(), name.end(), name.begin(),
                   [](auto c)
    {
      return ::tolower(c);
    });
    this->headers.push_back(NameValueTemplatePair{name, value});

    if (!is_literal_string)
      this->extend_worker_partition_key(value->template_str);

    return true;
  }

  void set_response_action(const ::grpc::StatusCode status_code, GrpcDestResponseAction action)
  {
    this->response_actions[status_code] = action;
  }

  void extend_worker_partition_key(const std::string &extension)
  {
    if (this->worker_partition_key.rdbuf()->in_avail())
      this->worker_partition_key << ",";
    this->worker_partition_key << extension;
  }

  void enable_dynamic_headers()
  {
    this->dynamic_headers_enabled = true;
    this->flush_on_key_change = true;
  }

  void set_proto_var(LogTemplate *proto_var_)
  {
    log_template_unref(this->proto_var);
    this->proto_var = log_template_ref(proto_var_);
  }

  const char *format_proto_var(LogMessage *log_msg, ssize_t *len)
  {
    if (!this->proto_var)
      return nullptr;

    LogMessageValueType lmvt;
    const gchar *serialized = log_template_get_trivial_value_and_type(this->proto_var, log_msg, len, &lmvt);
    if (lmvt != LM_VT_PROTOBUF)
      {
        msg_error("Error LogMessage type is not protobuf",
                  evt_tag_int("expected_type", LM_VT_PROTOBUF),
                  evt_tag_int("current_type", lmvt));
        return nullptr;
      }

    return serialized;
  }

  bool format_proto_var(LogMessage *log_msg, ::google::protobuf::Message *proto_msg)
  {
    ssize_t len;
    const gchar *serialized = this->format_proto_var(log_msg, &len);
    if (!serialized)
      return false;

    if (!proto_msg->ParsePartialFromArray(serialized, len))
      {
        msg_error("Unable to deserialize protobuf message",
                  evt_tag_int("proto_size", len));
        return false;
      }

    return true;
  }

  virtual LogMessageProtobufFormatter *get_log_message_protobuf_formatter()
  {
    return nullptr;
  }

  LogTemplateOptions &get_template_options()
  {
    return this->template_options;
  }

  GrpcClientCredentialsBuilderW *get_credentials_builder_wrapper()
  {
    return &this->credentials_builder_wrapper;
  }

private:
  bool set_worker_partition_key();

public:
  GrpcDestDriver *super;
  DestDriverMetrics metrics;
  syslogng::grpc::ClientCredentialsBuilder credentials_builder;

protected:
  friend class DestWorker;
  std::string url;

  bool compression;
  size_t batch_bytes;

  int keepalive_time;
  int keepalive_timeout;
  int keepalive_max_pings_without_data;

  std::stringstream worker_partition_key;
  bool flush_on_key_change;

  std::list<std::pair<std::string, long>> int_extra_channel_args;
  std::list<std::pair<std::string, std::string>> string_extra_channel_args;

  std::list<NameValueTemplatePair> headers;
  bool dynamic_headers_enabled;

  std::array<GrpcDestResponseAction, GRPC_DEST_RESPONSE_ACTIONS_ARRAY_LEN> response_actions;

  LogTemplate *proto_var = nullptr;
  LogTemplateOptions template_options;

  GrpcClientCredentialsBuilderW credentials_builder_wrapper;
};

}
}

#include "compat/cpp-start.h"

struct GrpcDestDriver_
{
  LogThreadedDestDriver super;
  syslogng::grpc::DestDriver *cpp;
};

GrpcDestDriver *grpc_dd_new(GlobalConfig *cfg, const gchar *stats_name);

#include "compat/cpp-end.h"

#endif


