/*
 * Copyright (c) 2024 Axoflow
 * Copyright (c) 2024 Attila Szakacs <attila.szakacs@axoflow.com>
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

#include <criterion/criterion.h>
#include "libtest/filterx-lib.h"

#include "filterx/func-flatten.h"
#include "filterx/object-string.h"
#include "filterx/object-primitive.h"
#include "filterx/json-repr.h"
#include "filterx/expr-literal.h"

#include "apphook.h"
#include "scratch-buffers.h"

static void
_assert_flatten_init_fail(GList *args)
{
  GError *err = NULL;
  GError *args_err = NULL;
  FilterXExpr *func = filterx_function_flatten_new(filterx_function_args_new(args, &args_err), &err);
  cr_assert(!func);
  cr_assert(err);
  g_error_free(err);
}

static void
_assert_flatten(GList *args, const gchar *expected_repr)
{
  FilterXExpr *modifiable_object_expr = filterx_expr_ref(((FilterXFunctionArg *) args->data)->value);

  GError *err = NULL;
  GError *args_err = NULL;
  FilterXExpr *func = filterx_function_flatten_new(filterx_function_args_new(args, &args_err), &err);
  cr_assert(!err);

  FilterXObject *obj = init_and_eval_expr(func);
  cr_assert(obj);
  gboolean success;
  cr_assert(filterx_boolean_unwrap(obj, &success));
  cr_assert(success);

  FilterXObject *modifiable_object = init_and_eval_expr(modifiable_object_expr);
  cr_assert(modifiable_object);

  assert_object_repr_equals(modifiable_object, expected_repr);

  filterx_object_unref(obj);
  filterx_expr_unref(func);
  filterx_object_unref(modifiable_object);
  filterx_expr_unref(modifiable_object_expr);
}

Test(filterx_func_flatten, invalid_args)
{
  GList *args = NULL;

  /* no args */
  _assert_flatten_init_fail(NULL);

  /* non-literal separator */
  args = g_list_append(args, filterx_function_arg_new(NULL, filterx_literal_new(filterx_test_dict_new())));
  args = g_list_append(args, filterx_function_arg_new("separator",
                                                      filterx_non_literal_new(filterx_boolean_new(TRUE))));
  _assert_flatten_init_fail(args);
  args = NULL;

  /* literal non-string separator */
  args = g_list_append(args, filterx_function_arg_new(NULL, filterx_literal_new(filterx_test_dict_new())));
  args = g_list_append(args, filterx_function_arg_new("separator",
                                                      filterx_literal_new(filterx_integer_new(42))));
  _assert_flatten_init_fail(args);
  args = NULL;

  /* too many args */
  args = g_list_append(args, filterx_function_arg_new(NULL, filterx_literal_new(filterx_test_dict_new())));
  args = g_list_append(args, filterx_function_arg_new(NULL, filterx_literal_new(filterx_string_new("", -1))));
  _assert_flatten_init_fail(args);
  args = NULL;
}

Test(filterx_func_flatten, default_separator)
{
  const gchar *input =
    "{\"top_level_field\":42,\"top_level_dict\":{\"inner_field\":1337,\"inner_dict\":{\"inner_inner_field\":1}}}";
  GList *args = g_list_append(NULL, filterx_function_arg_new(NULL,
                                                             filterx_literal_new(filterx_object_from_json(input, -1, NULL))));
  _assert_flatten(args,
                  "{\"top_level_field\":42,\"top_level_dict.inner_field\":1337,\"top_level_dict.inner_dict.inner_inner_field\":1}");
}

Test(filterx_func_flatten, custom_separator)
{
  const gchar *input =
    "{\"top_level_field\":42,\"top_level_dict\":{\"inner_field\":1337,\"inner_dict\":{\"inner_inner_field\":1}}}";
  GList *args = g_list_append(NULL, filterx_function_arg_new(NULL,
                                                             filterx_literal_new(filterx_object_from_json(input, -1, NULL))));
  args = g_list_append(args, filterx_function_arg_new("separator", filterx_literal_new(filterx_string_new("->", -1))));
  _assert_flatten(args,
                  "{\"top_level_field\":42,\"top_level_dict->inner_field\":1337,\"top_level_dict->inner_dict->inner_inner_field\":1}");
}

static void
setup(void)
{
  app_startup();
  init_libtest_filterx();
}

static void
teardown(void)
{
  scratch_buffers_explicit_gc();
  deinit_libtest_filterx();
  app_shutdown();
}

TestSuite(filterx_func_flatten, .init = setup, .fini = teardown);
