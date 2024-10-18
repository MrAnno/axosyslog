/*
 * Copyright (c) 2024 Attila Szakacs
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "object-otel-resource.hpp"
#include "otel-field.hpp"

#include "compat/cpp-start.h"
#include "filterx/object-extractor.h"
#include "filterx/object-string.h"
#include "compat/cpp-end.h"

#include <stdexcept>

using namespace syslogng::grpc::otel::filterx;

/* C++ Implementations */

Resource::Resource(FilterXOtelResource *s) : super(s)
{
}

Resource::Resource(FilterXOtelResource *s, FilterXObject *protobuf_object) : super(s)
{
  const gchar *value;
  gsize length;
  if (!filterx_object_extract_protobuf_ref(protobuf_object, &value, &length))
    throw std::runtime_error("Argument is not a protobuf object");

  if (!resource.ParsePartialFromArray(value, length))
    throw std::runtime_error("Failed to parse from protobuf object");
}

Resource::Resource(const Resource &o, FilterXOtelResource *s) : super(s), resource(o.resource)
{
}

std::string
Resource::marshal(void)
{
  return resource.SerializePartialAsString();
}

bool
Resource::set_subscript(FilterXObject *key, FilterXObject **value)
{
  try
    {
      std::string key_str = extract_string_from_object(key);
      ProtoReflectors reflectors(resource, key_str);
      ProtobufField *converter = otel_converter_by_field_descriptor(reflectors.fieldDescriptor);

      FilterXObject *assoc_object = NULL;
      if (!converter->Set(&resource, key_str, *value, &assoc_object))
        return false;

      filterx_object_unref(*value);
      *value = assoc_object;
      return true;
    }
  catch(const std::exception &ex)
    {
      return false;
    }
}

FilterXObject *
Resource::get_subscript(FilterXObject *key)
{
  try
    {
      std::string key_str = extract_string_from_object(key);
      ProtoReflectors reflectors(resource, key_str);
      ProtobufField *converter = otel_converter_by_field_descriptor(reflectors.fieldDescriptor);

      return converter->Get(&resource, key_str);
    }
  catch(const std::exception &ex)
    {
      return nullptr;
    }
}

bool
Resource::unset_key(FilterXObject *key)
{
  try
    {
      std::string key_str = extract_string_from_object(key);
      ProtoReflectors reflectors(resource, key_str);
      ProtobufField *converter = otel_converter_by_field_descriptor(reflectors.fieldDescriptor);

      return converter->Unset(&resource, key_str);
    }
  catch(const std::exception &ex)
    {
      return false;
    }
}

bool
Resource::is_key_set(FilterXObject *key)
{
  try
    {
      std::string key_str = extract_string_from_object(key);
      ProtoReflectors reflectors(resource, key_str);
      ProtobufField *converter = otel_converter_by_field_descriptor(reflectors.fieldDescriptor);

      return converter->IsSet(&resource, key_str);
    }
  catch(const std::exception &ex)
    {
      return false;
    }
}

uint64_t
Resource::len() const
{
  return get_protobuf_message_set_field_count(resource);
}

bool
Resource::iter(FilterXDictIterFunc func, void *user_data)
{
  return iter_on_otel_protobuf_message_fields(resource, func, user_data);
}

const opentelemetry::proto::resource::v1::Resource &
Resource::get_value() const
{
  return resource;
}

/* C Wrappers */

static void
_free(FilterXObject *s)
{
  FilterXOtelResource *self = (FilterXOtelResource *) s;

  delete self->cpp;
  self->cpp = NULL;

  filterx_object_free_method(s);
}

static gboolean
_set_subscript(FilterXDict *s, FilterXObject *key, FilterXObject **new_value)
{
  FilterXOtelResource *self = (FilterXOtelResource *) s;

  return self->cpp->set_subscript(key, new_value);
}

static FilterXObject *
_get_subscript(FilterXDict *s, FilterXObject *key)
{
  FilterXOtelResource *self = (FilterXOtelResource *) s;

  return self->cpp->get_subscript(key);
}

static gboolean
_unset_key(FilterXDict *s, FilterXObject *key)
{
  FilterXOtelResource *self = (FilterXOtelResource *) s;

  return self->cpp->unset_key(key);
}

static gboolean
_is_key_set(FilterXDict *s, FilterXObject *key)
{
  FilterXOtelResource *self = (FilterXOtelResource *) s;

  return self->cpp->is_key_set(key);
}

static guint64
_len(FilterXDict *s)
{
  FilterXOtelResource *self = (FilterXOtelResource *) s;

  return self->cpp->len();
}

static gboolean
_iter(FilterXDict *s, FilterXDictIterFunc func, gpointer user_data)
{
  FilterXOtelResource *self = (FilterXOtelResource *) s;

  return self->cpp->iter(func, user_data);
}

static gboolean
_truthy(FilterXObject *s)
{
  return TRUE;
}

static gboolean
_marshal(FilterXObject *s, GString *repr, LogMessageValueType *t)
{
  FilterXOtelResource *self = (FilterXOtelResource *) s;

  std::string serialized = self->cpp->marshal();

  g_string_truncate(repr, 0);
  g_string_append_len(repr, serialized.c_str(), serialized.length());
  *t = LM_VT_PROTOBUF;
  return TRUE;
}

static void
_init_instance(FilterXOtelResource *self)
{
  filterx_dict_init_instance(&self->super, &FILTERX_TYPE_NAME(otel_resource));

  self->super.get_subscript = _get_subscript;
  self->super.set_subscript = _set_subscript;
  self->super.unset_key = _unset_key;
  self->super.is_key_set = _is_key_set;
  self->super.len = _len;
  self->super.iter = _iter;
}

FilterXObject *
_filterx_otel_resource_clone(FilterXObject *s)
{
  FilterXOtelResource *self = (FilterXOtelResource *) s;

  FilterXOtelResource *clone = g_new0(FilterXOtelResource, 1);
  _init_instance(clone);

  try
    {
      clone->cpp = new Resource(*self->cpp, self);
    }
  catch (const std::runtime_error &)
    {
      g_assert_not_reached();
    }

  return &clone->super.super;
}

FilterXObject *
filterx_otel_resource_new_from_args(FilterXExpr *s, GPtrArray *args)
{
  FilterXOtelResource *self = g_new0(FilterXOtelResource, 1);
  _init_instance(self);

  try
    {
      if (!args || args->len == 0)
        {
          self->cpp = new Resource(self);
        }
      else if (args->len == 1)
        {
          FilterXObject *arg = (FilterXObject *) g_ptr_array_index(args, 0);
          if (filterx_object_is_type(arg, &FILTERX_TYPE_NAME(dict)))
            {
              self->cpp = new Resource(self);
              if (!filterx_dict_merge(&self->super.super, arg))
                throw std::runtime_error("Failed to merge dict");
            }
          else
            {
              self->cpp = new Resource(self, arg);
            }
        }
      else
        {
          throw std::runtime_error("Invalid number of arguments");
        }
    }
  catch (const std::runtime_error &e)
    {
      msg_error("FilterX: Failed to create OTel Resource object", evt_tag_str("error", e.what()));
      filterx_object_unref(&self->super.super);
      return NULL;
    }

  return &self->super.super;
}

static FilterXObject *
_list_factory(FilterXObject *self)
{
  return filterx_otel_array_new();
}

static FilterXObject *
_dict_factory(FilterXObject *self)
{
  return filterx_otel_kvlist_new();
}

FILTERX_SIMPLE_FUNCTION(otel_resource, filterx_otel_resource_new_from_args);


FILTERX_DEFINE_TYPE(otel_resource, FILTERX_TYPE_NAME(dict),
                    .is_mutable = TRUE,
                    .marshal = _marshal,
                    .clone = _filterx_otel_resource_clone,
                    .truthy = _truthy,
                    .list_factory = _list_factory,
                    .dict_factory = _dict_factory,
                    .free_fn = _free,
                   );
