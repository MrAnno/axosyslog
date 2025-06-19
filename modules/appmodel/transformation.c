/*
 * Copyright (c) 2023 Balázs Scheidler <balazs.scheidler@axoflow.com>
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

#include "transformation.h"

/* TransformStep: a named filterx block */

TransformStep *
transform_step_filterx_new(const gchar *name, const gchar *expr)
{
  TransformStep *self = g_new0(TransformStep, 1);
  self->name = g_strdup(name);
  self->filterx_expr = g_strdup(expr);
  return self;
}

TransformStep *
transform_step_parser_new(const gchar *name, const gchar *expr)
{
  TransformStep *self = g_new0(TransformStep, 1);
  self->name = g_strdup(name);
  self->parser_expr = g_strdup(expr);
  return self;
}

void
transform_step_free(TransformStep *self)
{
  g_free(self->name);
  g_free(self->filterx_expr);
  g_free(self->parser_expr);
  g_free(self);
}

/* Transform: named list of TransformSteps */

void
transform_add_filterx_step(Transform *self, const gchar *name, const gchar *expr)
{
  self->steps = g_list_append(self->steps, transform_step_filterx_new(name, expr));
}

void
transform_add_parser_step(Transform *self, const gchar *name, const gchar *expr)
{
  self->steps = g_list_append(self->steps, transform_step_parser_new(name, expr));
  self->filterx_only = FALSE;
}

Transform *
transform_new(const gchar *name)
{
  Transform *self = g_new0(Transform, 1);
  self->name = g_strdup(name);
  self->filterx_only = TRUE;
  return self;
}

void
transform_free(Transform *self)
{
  g_free(self->name);
  g_list_free_full(self->steps, (GDestroyNotify) transform_step_free);
  g_free(self);
}

/* Transformation: list of Transforms corresponding to an app (with optional flavour) */

void
transformation_add_transform(Transformation *self, Transform *transform)
{
  self->transforms = g_list_append(self->transforms, transform);
}

static void
transformation_free(AppModelObject *s)
{
  Transformation *self = (Transformation *) s;

  g_list_free_full(self->transforms, (GDestroyNotify) transform_free);
}

Transformation *
transformation_new(const gchar *app, const gchar *flavour)
{
  Transformation *self = g_new0(Transformation, 1);

  appmodel_object_init_instance(&self->super, TRANSFORMATION_TYPE_NAME, app, flavour);
  self->super.free_fn = transformation_free;
  return self;
}
