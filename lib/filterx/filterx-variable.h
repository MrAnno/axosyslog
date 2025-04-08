/*
 * Copyright (c) 2024 Axoflow
 * Copyright (c) 2024 László Várady
 * Copyright (c) 2023 Balazs Scheidler <balazs.scheidler@axoflow.com>
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

#ifndef FILTERX_VARIABLE_H
#define FILTERX_VARIABLE_H

#include "syslog-ng.h"
#include "filterx-object.h"

typedef enum
{
  FX_VAR_MESSAGE_TIED,
  FX_VAR_FLOATING,
  FX_VAR_DECLARED_FLOATING,
  __FX_VAR_MAX,
} FilterXVariableType;

/* currently we only have two bits in FilterXVariable to hold variable_type */
G_STATIC_ASSERT(__FX_VAR_MAX < 4);

typedef guint32 FilterXVariableHandle;
typedef guint16 FilterXGenCounter;

#define FILTERX_HANDLE_FLOATING_BIT (1UL << 31)

static inline NVHandle
filterx_variable_handle_to_nv_handle(FilterXVariableHandle handle)
{
  return handle & ~FILTERX_HANDLE_FLOATING_BIT;
}

FilterXVariableHandle filterx_map_varname_to_handle(const gchar *name, FilterXVariableType type);

typedef struct _FilterXVariable
{
  /* the MSB indicates that the variable is a floating one */
  FilterXVariableHandle handle;
  /*
   * assigned -- Indicates that the variable was assigned to a new value
   *
   * variable_type -- one of the FilterXVariableType enum
   *
   * We are storing these in bitfields to keep FilterXVariable within 16
   * bytes.
   */
  guint16 assigned:1,
          variable_type:2;
  FilterXGenCounter generation;
  FilterXObject *value;
} FilterXVariable;

/* NOTE: try to keep variable small */
G_STATIC_ASSERT(sizeof(FilterXVariable) <= 16);

void filterx_variable_init_instance(FilterXVariable *v,
                                    FilterXVariableType variable_type,
                                    FilterXVariableHandle handle);
void filterx_variable_clear(FilterXVariable *v);

static inline gboolean
filterx_variable_is_floating(FilterXVariable *v)
{
  /* matches both declared and undeclared floating values */
  return v->variable_type != FX_VAR_MESSAGE_TIED;
}

static inline gboolean
filterx_variable_is_message_tied(FilterXVariable *v)
{
  return v->variable_type == FX_VAR_MESSAGE_TIED;
}

static inline NVHandle
filterx_variable_get_nv_handle(FilterXVariable *v)
{
  return filterx_variable_handle_to_nv_handle(v->handle);
}

static inline const gchar *
filterx_variable_get_name(FilterXVariable *v, gssize *len)
{
  return log_msg_get_handle_name(filterx_variable_get_nv_handle(v), len);
}

static inline FilterXObject *
filterx_variable_get_value(FilterXVariable *v)
{
  return filterx_object_ref(v->value);
}

static inline void
filterx_variable_set_value(FilterXVariable *v, FilterXObject **new_value, gboolean assignment,
                           FilterXGenCounter generation)
{
  filterx_object_unref(v->value);
  if (new_value)
    {
      v->value = filterx_object_ref(*new_value);
    }
  else
    {
      v->value = NULL;
    }
  v->assigned = assignment;
  v->generation = generation;
}

static inline void
filterx_variable_unset_value(FilterXVariable *v, FilterXGenCounter generation)
{
  filterx_variable_set_value(v, NULL, TRUE, generation);
}

static inline gboolean
filterx_variable_is_set(FilterXVariable *v)
{
  return v->value != NULL;
}

static inline gboolean
filterx_variable_is_declared(FilterXVariable *v)
{
  return v->variable_type == FX_VAR_DECLARED_FLOATING;
}

static inline gboolean
filterx_variable_is_assigned(FilterXVariable *v)
{
  return v->assigned;
}

static inline gboolean
filterx_variable_is_same_generation(FilterXVariable *v, FilterXGenCounter generation)
{
  return v->generation == generation;
}

static inline void
filterx_variable_set_generation(FilterXVariable *v, FilterXGenCounter generation)
{
  v->generation = generation;
}

static inline void
filterx_variable_unassign(FilterXVariable *v)
{
  v->assigned = FALSE;
}

#endif
