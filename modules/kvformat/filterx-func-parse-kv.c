/*
 * Copyright (c) 2024 Axoflow
 * Copyright (c) 2024 shifter
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

#include "filterx-func-parse-kv.h"
#include "filterx/object-string.h"
#include "filterx/object-primitive.h"
#include "filterx/expr-literal.h"
#include "filterx/filterx-eval.h"
#include "filterx/filterx-globals.h"
#include "filterx/object-extractor.h"
#include "filterx/object-dict-interface.h"
#include "filterx/object-message-value.h"
#include "filterx/object-null.h"
#include "filterx/expr-generator.h"

#include "kv-parser.h"
#include "parser/parser-expr.h"
#include "scratch-buffers.h"
#include "str-utils.h"

typedef struct FilterXFunctionParseKV_
{
  FilterXGeneratorFunction super;
  FilterXExpr *msg;
  gchar value_separator;
  gchar *pair_separator;
  gchar *stray_words_key;
} FilterXFunctionParseKV;

static gboolean
_is_valid_separator_character(char c)
{
  return (c != ' '  &&
          c != '\'' &&
          c != '\"' );
}

static void
_set_value_separator(FilterXFunctionParseKV *self, gchar value_separator)
{
  self->value_separator = value_separator;
}

static void
_set_pair_separator(FilterXFunctionParseKV *self, const gchar *pair_separator)
{
  g_free(self->pair_separator);
  self->pair_separator = g_strdup(pair_separator);
}

static void
_set_stray_words_key(FilterXFunctionParseKV *self, const gchar *value_name)
{
  g_free(self->stray_words_key);
  self->stray_words_key = g_strdup(value_name);
}

static gboolean
_set_dict_value(FilterXObject *out,
                const gchar *key, gsize key_len,
                const gchar *value, gsize value_len)
{
  FILTERX_STRING_DECLARE_ON_STACK(dict_key, key, key_len);
  FILTERX_STRING_DECLARE_ON_STACK(dict_val, value, value_len);

  gboolean ok = filterx_object_set_subscript(out, dict_key, &dict_val);
  if (!ok)
    filterx_eval_push_error_static_info("Failed to evaluate parse_kv()", NULL, "set-subscript() method failed");

  filterx_object_unref(dict_key);
  filterx_object_unref(dict_val);
  return ok;
}

static gboolean
_extract_key_values(FilterXFunctionParseKV *self, const gchar *input, gsize input_len, FilterXObject *output)
{
  KVScanner scanner;
  gboolean result = FALSE;

  kv_scanner_init(&scanner, self->value_separator, self->pair_separator,
                  self->stray_words_key != NULL ? KVSSWM_COLLECT : KVSSWM_DROP);
  kv_scanner_input(&scanner, input);
  while (kv_scanner_scan_next(&scanner))
    {
      const gchar *name = kv_scanner_get_current_key(&scanner);
      gsize name_len = kv_scanner_get_current_key_len(&scanner);
      const gchar *value = kv_scanner_get_current_value(&scanner);
      gsize value_len = kv_scanner_get_current_value_len(&scanner);

      if (!_set_dict_value(output, name, name_len, value, value_len))
        goto exit;
    }

  if (self->stray_words_key &&
      !_set_dict_value(output, self->stray_words_key, -1,
                       kv_scanner_get_stray_words(&scanner), kv_scanner_get_stray_words_len(&scanner)))
    goto exit;

  result = TRUE;
exit:
  kv_scanner_deinit(&scanner);
  return result;
}

static gboolean
_generate(FilterXExprGenerator *s, FilterXObject *fillable)
{
  FilterXFunctionParseKV *self = (FilterXFunctionParseKV *) s;

  FilterXObject *obj = filterx_expr_eval(self->msg);
  if (!obj)
    {
      filterx_eval_push_error_static_info("Failed to evaluate parse_kv()", &self->super.super.super,
                                          "Failed to evaluate expression");
      return FALSE;
    }

  gsize len;
  const gchar *input;

  gboolean result = FALSE;
  if (!filterx_object_extract_string_ref(obj, &input, &len))
    {
      filterx_eval_push_error_info_printf("Failed to evaluate parse_kv()", &self->super.super.super,
                                          "Input must be string, got: %s",
                                          filterx_object_get_type_name(obj));
      goto exit;
    }

  APPEND_ZERO(input, input, len);

  result = _extract_key_values(self, input, len, fillable);

exit:
  filterx_object_unref(obj);
  return result;
}

static FilterXExpr *
_optimize(FilterXExpr *s)
{
  FilterXFunctionParseKV *self = (FilterXFunctionParseKV *) s;

  self->msg = filterx_expr_optimize(self->msg);
  return filterx_generator_function_optimize_method(&self->super);
}

static gboolean
_init(FilterXExpr *s, GlobalConfig *cfg)
{
  FilterXFunctionParseKV *self = (FilterXFunctionParseKV *) s;

  if (!filterx_expr_init(self->msg, cfg))
    return FALSE;

  return filterx_generator_function_init_method(&self->super, cfg);
}

static void
_deinit(FilterXExpr *s, GlobalConfig *cfg)
{
  FilterXFunctionParseKV *self = (FilterXFunctionParseKV *) s;
  filterx_expr_deinit(self->msg, cfg);
  filterx_generator_function_deinit_method(&self->super, cfg);
}

static void
_free(FilterXExpr *s)
{
  FilterXFunctionParseKV *self = (FilterXFunctionParseKV *) s;
  filterx_expr_unref(self->msg);
  g_free(self->pair_separator);
  filterx_generator_function_free_method(&self->super);
}

static FilterXExpr *
_extract_msg_expr_arg(FilterXFunctionArgs *args, GError **error)
{
  FilterXExpr *msg_expr = filterx_function_args_get_expr(args, 0);
  if (!msg_expr)
    {
      g_set_error(error, FILTERX_FUNCTION_ERROR, FILTERX_FUNCTION_ERROR_CTOR_FAIL,
                  "argument must be set: msg_str. " FILTERX_FUNC_PARSE_KV_USAGE);
      return NULL;
    }

  return msg_expr;
}

static gboolean
_extract_optional_args(FilterXFunctionParseKV *self, FilterXFunctionArgs *args, GError **error)
{
  const gchar *error_str = "";
  gboolean exists;
  gsize len;
  const gchar *value;

  value = filterx_function_args_get_named_literal_string(args, "value_separator", &len, &exists);
  if (exists)
    {
      if (len < 1)
        {
          error_str = "value_separator argument can not be empty";
          goto error;
        }
      if (!value)
        {
          error_str = "value_separator argument must be string literal";
          goto error;
        }
      if (!_is_valid_separator_character(value[0]))
        {
          error_str = "value_separator argument contains invalid separator character";
          goto error;
        }
      _set_value_separator(self, value[0]);
    }

  value = filterx_function_args_get_named_literal_string(args, "pair_separator", &len, &exists);
  if (exists)
    {
      if (!value)
        {
          error_str = "pair_separator argument must be string literal";
          goto error;
        }
      _set_pair_separator(self, value);
    }

  value = filterx_function_args_get_named_literal_string(args, "stray_words_key", &len, &exists);
  if (exists)
    {
      if (!value)
        {
          error_str = "stray_words_key argument must be string literal";
          goto error;
        }
      _set_stray_words_key(self, value);
    }

  return TRUE;

error:
  g_set_error(error, FILTERX_FUNCTION_ERROR, FILTERX_FUNCTION_ERROR_CTOR_FAIL,
              "%s. %s", error_str, FILTERX_FUNC_PARSE_KV_USAGE);
  return FALSE;
}

static gboolean
_extract_args(FilterXFunctionParseKV *self, FilterXFunctionArgs *args, GError **error)
{
  gsize args_len = filterx_function_args_len(args);
  if (args_len != 1)
    {
      g_set_error(error, FILTERX_FUNCTION_ERROR, FILTERX_FUNCTION_ERROR_CTOR_FAIL,
                  "invalid number of arguments. " FILTERX_FUNC_PARSE_KV_USAGE);
      return FALSE;
    }

  self->msg = _extract_msg_expr_arg(args, error);
  if (!self->msg)
    return FALSE;

  if (!_extract_optional_args(self, args, error))
    return FALSE;

  return TRUE;
}

FilterXExpr *
filterx_function_parse_kv_new(FilterXFunctionArgs *args, GError **error)
{
  FilterXFunctionParseKV *self = g_new0(FilterXFunctionParseKV, 1);
  filterx_generator_function_init_instance(&self->super, "parse_kv");
  self->super.super.generate = _generate;
  self->super.super.create_container = filterx_generator_create_dict_container;
  self->super.super.super.optimize = _optimize;
  self->super.super.super.init = _init;
  self->super.super.super.deinit = _deinit;
  self->super.super.super.free_fn = _free;
  self->value_separator = '=';
  self->pair_separator = g_strdup(", ");

  if (!_extract_args(self, args, error) ||
      !filterx_function_args_check(args, error))
    goto error;

  filterx_function_args_free(args);
  return &self->super.super.super;

error:
  filterx_function_args_free(args);
  filterx_expr_unref(&self->super.super.super);
  return NULL;
}

FILTERX_GENERATOR_FUNCTION(parse_kv, filterx_function_parse_kv_new);
