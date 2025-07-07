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

#ifndef FILTERX_PARSE_XML_H_INCLUDED
#define FILTERX_PARSE_XML_H_INCLUDED

#include "filterx/expr-function.h"

FILTERX_GENERATOR_FUNCTION_DECLARE(parse_xml);

FilterXExpr *filterx_generator_function_parse_xml_new(FilterXFunctionArgs *args, GError **error);


typedef struct FilterXParseXmlState_ FilterXParseXmlState;
struct FilterXParseXmlState_
{
  GArray *xml_elem_context_stack;

  void (*free_fn)(FilterXParseXmlState *self);
};

void filterx_parse_xml_state_init_instance(FilterXParseXmlState *self);
void filterx_parse_xml_state_free_method(FilterXParseXmlState *self);

static inline void
filterx_parse_xml_state_free(FilterXParseXmlState *self)
{
  self->free_fn(self);
  g_free(self);
}


typedef struct XmlElemContext_
{
  FilterXObject *current_obj;
  FilterXObject *parent_obj;
} XmlElemContext;

void xml_elem_context_init(XmlElemContext *self, FilterXObject *parent_obj, FilterXObject *current_obj);
void xml_elem_context_destroy(XmlElemContext *self);
void xml_elem_context_set_current_obj(XmlElemContext *self, FilterXObject *current_obj);
void xml_elem_context_set_parent_obj(XmlElemContext *self, FilterXObject *parent_obj);


static inline void
xml_elem_context_stack_push(GArray *xml_elem_context_stack, XmlElemContext *elem_context)
{
  g_array_append_val(xml_elem_context_stack, *elem_context);
}

static inline XmlElemContext *
xml_elem_context_stack_peek_last(GArray *xml_elem_context_stack)
{
  return &g_array_index(xml_elem_context_stack, XmlElemContext, xml_elem_context_stack->len - 1);
}

static inline void
xml_elem_context_stack_remove_last(GArray *xml_elem_context_stack)
{
  xml_elem_context_destroy(xml_elem_context_stack_peek_last(xml_elem_context_stack));
  g_array_remove_index(xml_elem_context_stack, xml_elem_context_stack->len - 1);
}


typedef struct FilterXGeneratorFunctionParseXml_ FilterXGeneratorFunctionParseXml;
struct FilterXGeneratorFunctionParseXml_
{
  FilterXGeneratorFunction super;
  FilterXExpr *xml_expr;

  FilterXParseXmlState *(*create_state)(void);

  void (*start_elem)(FilterXGeneratorFunctionParseXml *self,
                     GMarkupParseContext *context, const gchar *element_name,
                     const gchar **attribute_names, const gchar **attribute_values,
                     FilterXParseXmlState *state, GError **error);
  void (*end_elem)(FilterXGeneratorFunctionParseXml *self,
                   GMarkupParseContext *context, const gchar *element_name,
                   FilterXParseXmlState *state, GError **error);
  void (*text)(FilterXGeneratorFunctionParseXml *self,
               GMarkupParseContext *context, const gchar *text, gsize text_len,
               FilterXParseXmlState *state, GError **error);
};

void filterx_parse_xml_start_elem_method(FilterXGeneratorFunctionParseXml *self,
                                         GMarkupParseContext *context, const gchar *element_name,
                                         const gchar **attribute_names, const gchar **attribute_values,
                                         FilterXParseXmlState *state, GError **error);
void filterx_parse_xml_end_elem_method(FilterXGeneratorFunctionParseXml *self,
                                       GMarkupParseContext *context, const gchar *element_name,
                                       FilterXParseXmlState *state, GError **error);
void filterx_parse_xml_text_method(FilterXGeneratorFunctionParseXml *self,
                                   GMarkupParseContext *context, const gchar *text, gsize text_len,
                                   FilterXParseXmlState *state, GError **error);
gboolean filterx_parse_xml_prepare_elem(const gchar *new_elem_name, XmlElemContext *last_elem_context,
                                        gboolean has_attrs, XmlElemContext *new_elem_context, GError **error);

void filterx_parse_xml_replace_string_text(XmlElemContext *elem_context, const gchar *element_name, const gchar *text,
                                           gsize text_len, GError **error);

#endif
