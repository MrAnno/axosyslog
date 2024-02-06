/*
 * Copyright (c) 2023 shifter
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

#include <unistd.h>
#include <cstdio>
#include <memory>

#include "protobuf-field.hpp"

#include "compat/cpp-start.h"

#include "filterx/object-string.h"
#include "filterx/object-datetime.h"
#include "filterx/object-primitive.h"
#include "scratch-buffers.h"
#include "generic-number.h"

#include "compat/cpp-end.h"

using namespace syslogng::grpc::otel;

void log_type_error(ProtoReflectors reflectors, const char *type)
{
  msg_error("unsupported type",
    evt_tag_str("field", reflectors.fieldDescriptor->name().c_str()),
    evt_tag_str("expected type", reflectors.fieldDescriptor->type_name()),
    evt_tag_str("type", type));
}

float double_to_float_safe(double val) {
  if (val < (double)(-FLT_MAX)) {
    return -FLT_MAX;
  } else if (val > (double)(FLT_MAX)) {
    return FLT_MAX;
  } 
  return (float)val;
};

/* C++ Implementations */

// ProtoReflectors reflectors
class BoolField : public ProtobufField {
    public:
        FilterXObject *FilterXObjectGetter(const google::protobuf::Message &message, ProtoReflectors reflectors) {
          return filterx_boolean_new(reflectors.reflection->GetBool(message, reflectors.fieldDescriptor));
        }
        bool FilterXObjectSetter(google::protobuf::Message *message, ProtoReflectors reflectors, FilterXObject *object) {
          gboolean truthy = filterx_object_truthy(object);
          reflectors.reflection->SetBool(message, reflectors.fieldDescriptor, truthy);
          return true;
        }
};

class i32Field : public ProtobufField {
    public:
        FilterXObject *FilterXObjectGetter(const google::protobuf::Message &message, ProtoReflectors reflectors) {
          return filterx_integer_new(gint32(reflectors.reflection->GetInt32(message, reflectors.fieldDescriptor)));
        }
        bool FilterXObjectSetter(google::protobuf::Message *message, ProtoReflectors reflectors, FilterXObject *object) {
          if (!filterx_object_is_type(object, &FILTERX_TYPE_NAME(integer))) {
            log_type_error(reflectors, object->type->name);
            return false;
          }
          GenericNumber *gn = filterx_primitive_generic_number(object);
          uint32_t val = gn_as_int64(gn);
          reflectors.reflection->SetInt32(message, reflectors.fieldDescriptor, val);
          return true;
        }
};

class i64Field : public ProtobufField {
    public:
        FilterXObject *FilterXObjectGetter(const google::protobuf::Message &message, ProtoReflectors reflectors) {
          return filterx_integer_new(gint64(reflectors.reflection->GetInt64(message, reflectors.fieldDescriptor)));
        }
        bool FilterXObjectSetter(google::protobuf::Message *message, ProtoReflectors reflectors, FilterXObject *object) {
          if (!filterx_object_is_type(object, &FILTERX_TYPE_NAME(integer))) {
            log_type_error(reflectors, object->type->name);
            return false;
          }
          GenericNumber *gn = filterx_primitive_generic_number(object);
          uint64_t val = gn_as_int64(gn);
          reflectors.reflection->SetInt64(message, reflectors.fieldDescriptor, val);
          return true;
        }
};

class u32Field : public ProtobufField {
    public:
        FilterXObject *FilterXObjectGetter(const google::protobuf::Message &message, ProtoReflectors reflectors) {
          return filterx_integer_new(guint32(reflectors.reflection->GetUInt32(message, reflectors.fieldDescriptor)));
        }
        bool FilterXObjectSetter(google::protobuf::Message *message, ProtoReflectors reflectors, FilterXObject *object) {
          if (!filterx_object_is_type(object, &FILTERX_TYPE_NAME(integer))) {
            log_type_error(reflectors, object->type->name);
            return false;
          }
          GenericNumber *gn = filterx_primitive_generic_number(object);
          uint32_t val = gn_as_int64(gn);
          reflectors.reflection->SetUInt32(message, reflectors.fieldDescriptor, val);
          return true;
        }
};

class u64Field : public ProtobufField {
    public:
        FilterXObject *FilterXObjectGetter(const google::protobuf::Message &message, ProtoReflectors reflectors) {
          return filterx_integer_new(guint64(reflectors.reflection->GetUInt64(message, reflectors.fieldDescriptor)));
        }
        bool FilterXObjectSetter(google::protobuf::Message *message, ProtoReflectors reflectors, FilterXObject *object) {
          if (!filterx_object_is_type(object, &FILTERX_TYPE_NAME(integer))) {
            log_type_error(reflectors, object->type->name);
            return false;
          }
          GenericNumber *gn = filterx_primitive_generic_number(object);
          uint64_t val = gn_as_int64(gn);
          reflectors.reflection->SetUInt64(message, reflectors.fieldDescriptor, val);
          return true;
        }
};

class StringField : public ProtobufField {
    public:
        FilterXObject *FilterXObjectGetter(const google::protobuf::Message &message, ProtoReflectors reflectors) {
          std::string str = reflectors.reflection->GetString(message, reflectors.fieldDescriptor);
          return filterx_string_new(str.c_str(), str.length());
        }
        bool FilterXObjectSetter(google::protobuf::Message *message, ProtoReflectors reflectors, FilterXObject *object) {
          if (!filterx_object_is_type(object, &FILTERX_TYPE_NAME(string))) {
            log_type_error(reflectors, object->type->name);
            return false;
          }
          gsize value_len = -1;
          const gchar *str = filterx_string_get_value(object, &value_len);
          std::string stdString(str);
          reflectors.reflection->SetString(message, reflectors.fieldDescriptor, stdString);
          return true;
        }
};

class DoubleField : public ProtobufField {
    public:
        FilterXObject *FilterXObjectGetter(const google::protobuf::Message &message, ProtoReflectors reflectors) {
          return filterx_double_new(gdouble(reflectors.reflection->GetDouble(message, reflectors.fieldDescriptor)));
        }
        bool FilterXObjectSetter(google::protobuf::Message *message, ProtoReflectors reflectors, FilterXObject *object) {
          if (!filterx_object_is_type(object, &FILTERX_TYPE_NAME(double))) {
            log_type_error(reflectors, object->type->name);
            return false;
          }
          GenericNumber *gn = filterx_primitive_generic_number(object);
          double val = gn_as_double(gn);
          reflectors.reflection->SetDouble(message, reflectors.fieldDescriptor, val);
          return true;
        }
};

class FloatField : public ProtobufField {
    public:
        FilterXObject *FilterXObjectGetter(const google::protobuf::Message &message, ProtoReflectors reflectors) {
            return filterx_double_new(gdouble(reflectors.reflection->GetFloat(message, reflectors.fieldDescriptor)));
        }
        bool FilterXObjectSetter(google::protobuf::Message *message, ProtoReflectors reflectors, FilterXObject *object) {
          if (!filterx_object_is_type(object, &FILTERX_TYPE_NAME(double))) {
            log_type_error(reflectors, object->type->name);
            return false;
          }
          GenericNumber *gn = filterx_primitive_generic_number(object);
          double val = gn_as_double(gn);
          float floatVal = double_to_float_safe(val);
          reflectors.reflection->SetFloat(message, reflectors.fieldDescriptor, floatVal);
          return true;
        }
};

class BytesField : public ProtobufField {
  public:
        FilterXObject *FilterXObjectGetter(const google::protobuf::Message &message, ProtoReflectors reflectors) {
            std::string bytes = reflectors.reflection->GetString(message, reflectors.fieldDescriptor);
            return filterx_bytes_new(bytes.c_str(), bytes.length());
        }
        bool FilterXObjectSetter(google::protobuf::Message *message, ProtoReflectors reflectors, FilterXObject *object) {
          if (!filterx_object_is_type(object, &FILTERX_TYPE_NAME(bytes))) {
            log_type_error(reflectors, object->type->name);
            return false;
          }
          gsize value_len = -1;
          const gchar *str = filterx_string_get_value(object, &value_len);
          std::string stdString(str);
          reflectors.reflection->SetString(message, reflectors.fieldDescriptor, stdString);
          return true;
        }    
};

std::unique_ptr<ProtobufField> *syslogng::grpc::otel::all_protobuf_converters() {
    static std::unique_ptr<ProtobufField> Converters[google::protobuf::FieldDescriptor::MAX_TYPE] = {
      std::make_unique<DoubleField>(),  //TYPE_DOUBLE = 1,    // double, exactly eight bytes on the wire.
      std::make_unique<FloatField>(),   //TYPE_FLOAT = 2,     // float, exactly four bytes on the wire.
      std::make_unique<i64Field>(),     //TYPE_INT64 = 3,     // int64, varint on the wire.  Negative numbers
                                        // take 10 bytes.  Use TYPE_SINT64 if negative
                                        // values are likely.
      std::make_unique<u64Field>(),     //TYPE_UINT64 = 4,    // uint64, varint on the wire.
      std::make_unique<i32Field>(),     //TYPE_INT32 = 5,     // int32, varint on the wire.  Negative numbers
                                        // take 10 bytes.  Use TYPE_SINT32 if negative
                                        // values are likely.
      std::make_unique<u64Field>(),     //TYPE_FIXED64 = 6,   // uint64, exactly eight bytes on the wire.
      std::make_unique<u32Field>(),     //TYPE_FIXED32 = 7,   // uint32, exactly four bytes on the wire.
      std::make_unique<BoolField>(),    //TYPE_BOOL = 8,      // bool, varint on the wire.
      std::make_unique<StringField>(),  //TYPE_STRING = 9,    // UTF-8 text.
      nullptr,                          //TYPE_GROUP = 10,    // Tag-delimited message.  Deprecated.
      nullptr,                          //TYPE_MESSAGE = 11,  // Length-delimited message.
      std::make_unique<BytesField>(),   //TYPE_BYTES = 12,     // Arbitrary byte array.
      std::make_unique<u32Field>(),     //TYPE_UINT32 = 13,    // uint32, varint on the wire
      nullptr,                          //TYPE_ENUM = 14,      // Enum, varint on the wire
      std::make_unique<i32Field>(),     //TYPE_SFIXED32 = 15,  // int32, exactly four bytes on the wire
      std::make_unique<i64Field>(),     //TYPE_SFIXED64 = 16,  // int64, exactly eight bytes on the wire
      std::make_unique<i32Field>(),     //TYPE_SINT32 = 17,    // int32, ZigZag-encoded varint on the wire
      std::make_unique<i64Field>(),     //TYPE_SINT64 = 18,    // int64, ZigZag-encoded varint on the wire
  };
  return Converters;
};

ProtobufField *syslogng::grpc::otel::protobuf_converter_by_type(google::protobuf::FieldDescriptor::Type fieldType) {
  assert(fieldType < google::protobuf::FieldDescriptor::MAX_TYPE || fieldType > 1);
  return all_protobuf_converters()[fieldType - 1].get();
} 
