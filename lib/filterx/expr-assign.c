/*
 * Copyright (c) 2023 Balazs Scheidler <balazs.scheidler@axoflow.com>
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
#include "filterx/expr-assign.h"
#include "filterx/object-primitive.h"
#include "filterx/filterx-eval.h"
#include "filterx/object-null.h"
#include "filterx/object-message-value.h"
#include "scratch-buffers.h"

static inline FilterXObject *
_assign(FilterXBinaryOp *self, FilterXObject *value)
{
  FilterXObject *cloned = filterx_object_cow_fork2(filterx_object_ref(value), NULL);
  if (!filterx_expr_assign(self->lhs, &cloned))
    {
      filterx_object_unref(cloned);
      return NULL;
    }

  return cloned;
}

static inline FilterXObject *
_suppress_error(void)
{
  if (debug_flag)
    {
      gint error_count = filterx_eval_get_error_count();
      gchar buf[FILTERX_EVAL_ERROR_IDX_FMT_SIZE];

      for (gint err_idx = 0; err_idx < error_count; err_idx++)
        {
          msg_debug("FilterX: null coalesce assignment suppressing error",
                    filterx_eval_format_error_index_tag(err_idx, buf),
                    filterx_eval_format_error_location_tag(err_idx),
                    filterx_eval_format_error_tag(err_idx));
        }
    }

  filterx_eval_clear_errors();

  return filterx_null_new();
}

static FilterXObject *
_nullv_assign_eval(FilterXExpr *s)
{
  FilterXBinaryOp *self = (FilterXBinaryOp *) s;

  FilterXObject *value = filterx_expr_eval(self->rhs);

  if (!value || filterx_object_is_type(value, &FILTERX_TYPE_NAME(null))
      || (filterx_object_is_type(value, &FILTERX_TYPE_NAME(message_value))
          && filterx_message_value_get_type(value) == LM_VT_NULL))
    {
      if (!value)
        return _suppress_error();

      return value;
    }

  FilterXObject *result = _assign(self, value);
  filterx_object_unref(value);
  return result;
}

static FilterXObject *
_assign_eval(FilterXExpr *s)
{
  FilterXBinaryOp *self = (FilterXBinaryOp *) s;

  FilterXObject *value = filterx_expr_eval(self->rhs);

  if (!value)
    return NULL;

  FilterXObject *result = _assign(self, value);
  filterx_object_unref(value);
  return result;
}


/* NOTE: takes the object reference */
FilterXExpr *
filterx_assign_new(FilterXExpr *lhs, FilterXExpr *rhs)
{
  FilterXBinaryOp *self = g_new0(FilterXBinaryOp, 1);

  filterx_binary_op_init_instance(self, "assign", lhs, rhs);
  self->super.eval = _assign_eval;
  self->super.ignore_falsy_result = TRUE;
  return &self->super;
}

FilterXExpr *
filterx_nullv_assign_new(FilterXExpr *lhs, FilterXExpr *rhs)
{
  FilterXExpr *self = filterx_assign_new(lhs, rhs);
  self->type = "nullv_assign";
  self->eval = _nullv_assign_eval;
  return self;
}
