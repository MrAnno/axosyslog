/*
 * Copyright (c) 2024 Axoflow
 * Copyright (c) 2024 shifter
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
#include "expr-plus.h"
#include "object-string.h"
#include "filterx-eval.h"
#include "filterx/filterx-object-istype.h"
#include "filterx/filterx-ref.h"
#include "scratch-buffers.h"
#include "expr-generator.h"
#include "object-list-interface.h"
#include "object-dict-interface.h"

typedef struct FilterXOperatorPlusGenerator
{
  FilterXExprGenerator super;
  FilterXExpr *lhs;
  FilterXExpr *rhs;
} FilterXOperatorPlusGenerator;

static gboolean
_generate_obj(FilterXOperatorPlusGenerator *self, FilterXObject *obj, FilterXObject *fillable)
{
  fillable = filterx_ref_unwrap_rw(fillable);

  if (filterx_object_is_type(fillable, &FILTERX_TYPE_NAME(list)))
    return filterx_list_merge(fillable, obj);

  if (filterx_object_is_type(fillable, &FILTERX_TYPE_NAME(dict)))
    return filterx_dict_merge(fillable, obj);

  filterx_eval_push_error("Failed to merge objects, invalid fillable type", &self->super.super, fillable);

  return FALSE;
}

static gboolean
_handle_object_or_generator(FilterXOperatorPlusGenerator *self, FilterXExpr *expr, FilterXObject *fillable)
{
  FilterXObject *obj = NULL;
  if (filterx_expr_is_generator(expr))
    {
      filterx_generator_set_fillable(expr, filterx_expr_ref(self->super.fillable));
      obj = filterx_expr_eval(expr);
      filterx_object_unref(obj);
      return !!obj;
    }

  obj = filterx_expr_eval(expr);
  if (!obj)
    return FALSE;
  if (!_generate_obj(self, obj, fillable))
    {
      filterx_object_unref(obj);
      return FALSE;
    }

  filterx_object_unref(obj);
  return TRUE;
}

static gboolean
_expr_plus_generator_generate(FilterXExprGenerator *s, FilterXObject *fillable)
{
  FilterXOperatorPlusGenerator *self = (FilterXOperatorPlusGenerator *) s;

  if (!_handle_object_or_generator(self, self->lhs, fillable))
    return FALSE;
  if (!_handle_object_or_generator(self, self->rhs, fillable))
    return FALSE;
  return TRUE;
}

static FilterXObject *
_expr_plus_generator_create_container(FilterXExprGenerator *s, FilterXExpr *fillable_parent)
{
  FilterXOperatorPlusGenerator *self = (FilterXOperatorPlusGenerator *) s;
  FilterXExprGenerator *generator = (FilterXExprGenerator *)(filterx_expr_is_generator(
      self->rhs) ? self->rhs : self->lhs);
  return generator->create_container(generator, fillable_parent);
}

static void
_expr_plus_generator_free(FilterXExpr *s)
{
  FilterXOperatorPlusGenerator *self = (FilterXOperatorPlusGenerator *) s;
  filterx_expr_unref(self->lhs);
  filterx_expr_unref(self->rhs);
  filterx_generator_free_method(s);
}

FilterXExpr *
filterx_operator_plus_generator_new(FilterXExpr *lhs, FilterXExpr *rhs)
{
  FilterXOperatorPlusGenerator *self = g_new0(FilterXOperatorPlusGenerator, 1);
  filterx_generator_init_instance(&self->super.super);
  self->lhs = lhs;
  self->rhs = rhs;
  self->super.generate = _expr_plus_generator_generate;
  self->super.super.free_fn = _expr_plus_generator_free;
  self->super.create_container = _expr_plus_generator_create_container;

  return &self->super.super;
}
