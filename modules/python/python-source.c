/*
 * Copyright (c) 2018 Balabit
 * Copyright (c) 2018 László Várady <laszlo.varady@balabit.com>
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

#include "python-source.h"
#include "python-logmsg.h"
#include "python-helpers.h"
#include "logthrsource/logthrsourcedrv.h"
#include "thread-utils.h"
#include "str-utils.h"
#include "string-list.h"
#include "python-persist.h"
#include "python-ack-tracker.h"
#include "python-bookmark.h"
#include "python-flags.h"

#include <structmember.h>

typedef struct _PythonSourceDriver PythonSourceDriver;

struct _PythonSourceDriver
{
  LogThreadedSourceDriver super;
  PythonBinding binding;
  ThreadId thread_id;

  void (*post_message)(PythonSourceDriver *self, LogMessage *msg);

  struct
  {
    PyObject *class;
    PyObject *instance;
    PyObject *run_method;
    PyObject *request_exit_method;
    PyObject *suspend_method;
    PyObject *wakeup_method;
    PyObject *generate_persist_name;
    PyAckTrackerFactory *ack_tracker_factory;
  } py;
};

typedef struct _PyLogSource
{
  PyObject_HEAD
  PythonSourceDriver *driver;
  gchar *persist_name;
  char auto_close_batches;
} PyLogSource;

static PyTypeObject py_log_source_type;

static const gchar *python_source_format_persist_name(const LogPipe *s);

PythonBinding *
python_sd_get_binding(LogDriver *s)
{
  PythonSourceDriver *self = (PythonSourceDriver *) s;

  return &self->binding;
}

static void
python_sd_format_stats_key(LogThreadedSourceDriver *s, StatsClusterKeyBuilder *kb)
{
  PythonSourceDriver *self = (PythonSourceDriver *) s;

  PythonPersistMembers options =
  {
    .generate_persist_name_method = self->py.generate_persist_name,
    .options = self->binding.options,
    .class = self->binding.class,
    .id = self->super.super.super.id
  };

  python_format_stats_key((LogPipe *)s, kb, "python", &options);
}

static void
_ps_py_invoke_void_method_by_name(PythonSourceDriver *self, const gchar *method_name)
{
  _py_invoke_void_method_by_name(self->py.instance, method_name, self->binding.class, self->super.super.super.id);
}

static gboolean
_ps_py_invoke_bool_method_by_name_with_options(PythonSourceDriver *self, const gchar *method_name)
{
  return _py_invoke_bool_method_by_name_with_options(self->py.instance, method_name, self->binding.options,
                                                     self->binding.class, self->super.super.super.id);
}

static void
_ps_py_invoke_void_function(PythonSourceDriver *self, PyObject *func, PyObject *arg)
{
  return _py_invoke_void_function(func, arg, self->binding.class, self->super.super.super.id);
}

static gboolean
_py_invoke_init(PythonSourceDriver *self)
{
  return _ps_py_invoke_bool_method_by_name_with_options(self, "init");
}

static void
_py_invoke_deinit(PythonSourceDriver *self)
{
  _ps_py_invoke_void_method_by_name(self, "deinit");
}

static void
_py_invoke_run(PythonSourceDriver *self)
{
  _ps_py_invoke_void_function(self, self->py.run_method, NULL);
}

static void
_py_invoke_request_exit(PythonSourceDriver *self)
{
  _ps_py_invoke_void_function(self, self->py.request_exit_method, NULL);
}

static void
_py_invoke_suspend(PythonSourceDriver *self)
{
  _ps_py_invoke_void_function(self, self->py.suspend_method, NULL);
}

static void
_py_invoke_wakeup(PythonSourceDriver *self)
{
  _ps_py_invoke_void_function(self, self->py.wakeup_method, NULL);
}

static void
_py_invoke_finalize(PythonSourceDriver *self)
{
  if (self->py.instance)
    _ps_py_invoke_void_method_by_name(self, "finalize");
}

static gboolean
_py_is_log_source(PyObject *obj)
{
  return PyType_IsSubtype(Py_TYPE(obj), &py_log_source_type);
}

static void
_py_free_bindings(PythonSourceDriver *self)
{
  PyLogSource *py_instance = (PyLogSource *) self->py.instance;
  if (py_instance)
    g_free(py_instance->persist_name);

  Py_CLEAR(self->py.class);
  Py_CLEAR(self->py.instance);
  Py_CLEAR(self->py.run_method);
  Py_CLEAR(self->py.request_exit_method);
  Py_CLEAR(self->py.suspend_method);
  Py_CLEAR(self->py.wakeup_method);
  Py_CLEAR(self->py.generate_persist_name);
  Py_CLEAR(self->py.ack_tracker_factory);
}

static gboolean
_py_resolve_class(PythonSourceDriver *self)
{
  self->py.class = _py_resolve_qualified_name(self->binding.class);

  if (!self->py.class)
    {
      gchar buf[256];

      msg_error("python-source: Error looking up Python driver class",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class),
                evt_tag_str("exception", _py_format_exception_text(buf, sizeof(buf))));
      _py_finish_exception_handling();
      return FALSE;
    }

  return TRUE;
}

static gboolean
_py_init_instance(PythonSourceDriver *self)
{
  gchar buf[256];

  self->py.instance = _py_invoke_function(self->py.class, NULL, self->binding.class, self->super.super.super.id);
  if (!self->py.instance)
    {
      msg_error("python-source: Error instantiating Python driver class",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class),
                evt_tag_str("class-repr", _py_object_repr(self->py.class, buf, sizeof(buf))),
                evt_tag_str("exception", _py_format_exception_text(buf, sizeof(buf))));
      _py_finish_exception_handling();
      return FALSE;
    }

  if (!_py_is_log_source(self->py.instance))
    {
      msg_error("python-source: Error instantiating Python driver class, class is not a subclass of LogSource",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class),
                evt_tag_str("class-repr", _py_object_repr(self->py.class, buf, sizeof(buf))));
      return FALSE;
    }

  ((PyLogSource *) self->py.instance)->driver = self;
  ((PyLogSource *) self->py.instance)->auto_close_batches = TRUE;

  return TRUE;
}

static gboolean
_py_lookup_run_method(PythonSourceDriver *self)
{
  self->py.run_method = _py_get_attr_or_null(self->py.instance, "run");

  if (!self->py.run_method)
    {
      msg_error("python-source: Error initializing Python source, class does not have a run() method",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class));
      return FALSE;
    }

  return TRUE;
}

static gboolean
_py_lookup_request_exit_method(PythonSourceDriver *self)
{
  self->py.request_exit_method = _py_get_attr_or_null(self->py.instance, "request_exit");

  if (!self->py.request_exit_method)
    {
      msg_error("python-source: Error initializing Python source, class does not have a request_exit() method",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class));
      return FALSE;
    }

  return TRUE;
}

static gboolean
_py_lookup_suspend_and_wakeup_methods(PythonSourceDriver *self)
{
  self->py.suspend_method = _py_get_attr_or_null(self->py.instance, "suspend");

  if (self->py.suspend_method)
    {
      self->py.wakeup_method = _py_get_attr_or_null(self->py.instance, "wakeup");
      if (!self->py.wakeup_method)
        {
          msg_error("python-source: Error initializing Python source, class implements suspend() but wakeup() is missing",
                    evt_tag_str("driver", self->super.super.super.id),
                    evt_tag_str("class", self->binding.class));
          return FALSE;
        }
    }

  return TRUE;
}

static gboolean
_py_lookup_generate_persist_name_method(PythonSourceDriver *self)
{
  self->py.generate_persist_name = _py_get_attr_or_null(self->py.instance, "generate_persist_name");
  return TRUE;
}

static gboolean
_py_set_persist_name(PythonSourceDriver *self)
{
  const gchar *persist_name = python_source_format_persist_name(&self->super.super.super.super);
  PyLogSource *py_instance = (PyLogSource *) self->py.instance;
  py_instance->persist_name = g_strdup(persist_name);

  return TRUE;
}

static gboolean
_py_init_methods(PythonSourceDriver *self)
{
  return _py_lookup_run_method(self)
         && _py_lookup_request_exit_method(self)
         && _py_lookup_suspend_and_wakeup_methods(self)
         && _py_lookup_generate_persist_name_method(self)
         && _py_set_persist_name(self);
}

static gboolean
_py_init_bindings(PythonSourceDriver *self)
{
  gboolean initialized = _py_resolve_class(self)
                         && _py_init_instance(self)
                         && _py_init_methods(self);

  if (!initialized)
    _py_free_bindings(self);

  return initialized;
}

static gboolean
_py_set_flags(PythonSourceDriver *self)
{
  MsgFormatOptions *parse_options = log_threaded_source_driver_get_parse_options(&self->super.super.super);
  g_assert(parse_options);

  PyObject *flags = python_source_flags_new(parse_options->flags);
  if (!flags)
    {
      msg_error("python-source: Error creating flags attribute",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class));
      return FALSE;
    }

  if (PyObject_SetAttrString(self->py.instance, "flags", flags) == -1)
    {
      gchar buf[256];

      msg_error("python-source: Error setting flags attribute",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class),
                evt_tag_str("exception", _py_format_exception_text(buf, sizeof(buf))));
      _py_finish_exception_handling();

      Py_DECREF(flags);
      return FALSE;
    }

  Py_DECREF(flags);
  return TRUE;
}

static gboolean
_py_init_object(PythonSourceDriver *self)
{
  if (!_py_get_attr_or_null(self->py.instance, "init"))
    {
      msg_debug("python-source: Missing Python method, init()",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class));
      return TRUE;
    }

  if (!_py_invoke_init(self))
    {
      msg_error("python-source: Error initializing Python driver object, init() returned FALSE",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class));
      return FALSE;
    }
  return TRUE;
}

static PyObject *
_py_parse_options_new(PythonSourceDriver *self, MsgFormatOptions *parse_options)
{
  PyObject *py_parse_options = PyCapsule_New(parse_options, NULL, NULL);

  if (!py_parse_options)
    {
      gchar buf[256];

      msg_error("python-source: Error creating capsule for message parse options",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class),
                evt_tag_str("exception", _py_format_exception_text(buf, sizeof(buf))));
      _py_finish_exception_handling();
      return NULL;
    }

  return py_parse_options;
}

static gboolean
_py_init_ack_tracker_factory(PythonSourceDriver *self)
{
  PyObject *py_ack_tracker_factory = _py_get_attr_or_null(self->py.instance, "ack_tracker");

  if (!py_ack_tracker_factory)
    return TRUE;

  if (!py_is_ack_tracker_factory(py_ack_tracker_factory))
    {
      msg_error("python-source: Python source attribute ack_tracker needs to be an AckTracker subtype",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class));
      return FALSE;
    }

  self->py.ack_tracker_factory = (PyAckTrackerFactory *) py_ack_tracker_factory;

  AckTrackerFactory *ack_tracker_factory = self->py.ack_tracker_factory->ack_tracker_factory;
  self->super.worker_options.ack_tracker_factory = ack_tracker_factory_ref(ack_tracker_factory);

  return TRUE;
}

static gboolean
_py_set_parse_options(PythonSourceDriver *self)
{
  MsgFormatOptions *parse_options = log_threaded_source_driver_get_parse_options(&self->super.super.super);

  PyObject *py_parse_options = _py_parse_options_new(self, parse_options);
  if (!py_parse_options)
    return FALSE;

  if (PyObject_SetAttrString(self->py.instance, "parse_options", py_parse_options) == -1)
    {
      gchar buf[256];

      msg_error("python-source: Error setting attribute message parse options",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("class", self->binding.class),
                evt_tag_str("exception", _py_format_exception_text(buf, sizeof(buf))));
      _py_finish_exception_handling();

      Py_DECREF(py_parse_options);
      return FALSE;
    }

  Py_DECREF(py_parse_options);
  return TRUE;
}

static void
python_sd_suspend(PythonSourceDriver *self)
{
  PyGILState_STATE gstate = PyGILState_Ensure();
  _py_invoke_suspend(self);
  PyGILState_Release(gstate);
}

static void
python_sd_worker_wakeup(LogThreadedSourceWorker *w)
{
  PythonSourceDriver *control = (PythonSourceDriver *) w->control;

  PyGILState_STATE gstate = PyGILState_Ensure();
  _py_invoke_wakeup(control);
  PyGILState_Release(gstate);
}

static void
_post_message_non_blocking(PythonSourceDriver *self, LogMessage *msg)
{
  PyThreadState *state = PyEval_SaveThread();
  log_threaded_source_worker_post(self->super.workers[0], msg);
  PyEval_RestoreThread(state);

  /* GIL is used to synchronize free_to_send(), suspend() and wakeup() */
  if (!log_threaded_source_worker_free_to_send(self->super.workers[0]))
    python_sd_suspend(self);
}

static void
_post_message_blocking(PythonSourceDriver *self, LogMessage *msg)
{
  PyThreadState *state = PyEval_SaveThread();
  log_threaded_source_worker_blocking_post(self->super.workers[0], msg);
  PyEval_RestoreThread(state);
}

static gboolean
_py_sd_init(PythonSourceDriver *self)
{
  PyGILState_STATE gstate = PyGILState_Ensure();

  if (!_py_init_bindings(self))
    goto error;

  if (!_py_set_flags(self))
    goto error;

  if (!_py_init_object(self))
    goto error;

  if (!_py_init_ack_tracker_factory(self))
    goto error;

  if (!_py_set_parse_options(self))
    goto error;

  PyGILState_Release(gstate);
  return TRUE;

error:
  PyGILState_Release(gstate);
  return FALSE;
}

static inline AckTracker *
_py_sd_get_ack_tracker(PythonSourceDriver *self)
{
  return self->super.workers[0]->super.ack_tracker;
}

static gboolean
_py_sd_fill_bookmark(PythonSourceDriver *self, PyLogMessage *pymsg)
{
  if (!self->py.ack_tracker_factory)
    {
      PyErr_Format(PyExc_RuntimeError,
                   "Bookmarks can not be used without creating an AckTracker instance (self.ack_tracker)");
      return FALSE;
    }

  AckTracker *ack_tracker = _py_sd_get_ack_tracker(self);

  Bookmark *bookmark;
  Py_BEGIN_ALLOW_THREADS
  bookmark = ack_tracker_request_bookmark(ack_tracker);
  Py_END_ALLOW_THREADS;

  PyBookmark *py_bookmark = py_bookmark_new(pymsg->bookmark_data, self->py.ack_tracker_factory->ack_callback);
  py_bookmark_fill(bookmark, py_bookmark);
  Py_XDECREF(py_bookmark);

  return TRUE;
}

static PyObject *
py_log_source_post(PyObject *s, PyObject *args, PyObject *kwrds)
{
  PyLogSource *self = (PyLogSource *) s;

  if (self->driver->thread_id != get_thread_id())
    {
      /*
         Message posting must happen in a syslog-ng thread that was
         initialized by main_loop_call_thread_init(), which is not
         exposed to python. Hence posting from a python thread can
         crash syslog-ng.
      */

      PyErr_Format(PyExc_RuntimeError, "post_message must be called from main thread");
      return NULL;
    }

  PythonSourceDriver *sd = self->driver;

  PyLogMessage *pymsg;

  static const gchar *kwlist[] = {"msg", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kwrds, "O", (gchar **) kwlist, &pymsg))
    return NULL;

  if (!py_is_log_message((PyObject *) pymsg))
    {
      PyErr_Format(PyExc_TypeError, "LogMessage expected in the first parameter");
      return NULL;
    }

  if (!log_threaded_source_worker_free_to_send(sd->super.workers[0]))
    {
      msg_error("python-source: Incorrectly suspended source, dropping message",
                evt_tag_str("driver", sd->super.super.super.id));
      Py_RETURN_NONE;
    }

  if (pymsg->bookmark_data && pymsg->bookmark_data != Py_None)
    {
      if (!_py_sd_fill_bookmark(sd, pymsg))
        return NULL;
    }

  /* keep a reference until the PyLogMessage instance is freed */
  LogMessage *message = log_msg_ref(pymsg->msg);
  sd->post_message(sd, message);

  Py_RETURN_NONE;
}

static PyObject *
py_log_source_close_batch(PyObject *s)
{
  PyLogSource *self = (PyLogSource *) s;

  if (self->driver->thread_id != get_thread_id())
    {
      /*
         Message posting and batch closing must happen in a syslog-ng thread
         that was initialized by main_loop_call_thread_init(), which is not
         exposed to python.  Hence posting from a python thread can crash
         syslog-ng.
      */

      PyErr_Format(PyExc_RuntimeError, "close_batch() must be called from main thread");
      return NULL;
    }

  PythonSourceDriver *sd = self->driver;
  log_threaded_source_worker_close_batch(sd->super.workers[0]);

  Py_RETURN_NONE;
}

static PyObject *
py_log_source_set_transport_name(PyLogSource *self, PyObject *args)
{
  const gchar *transport_name;

  if (!PyArg_ParseTuple(args, "s", &transport_name))
    return NULL;

  log_threaded_source_driver_set_transport_name(&self->driver->super, transport_name);
  Py_RETURN_NONE;
}

static void
python_sd_worker_run(LogThreadedSourceWorker *w)
{
  PythonSourceDriver *control = (PythonSourceDriver *) w->control;

  control->thread_id = get_thread_id();
  PyGILState_STATE gstate = PyGILState_Ensure();
  _py_invoke_run(control);
  PyGILState_Release(gstate);
}

static void
python_sd_worker_request_exit(LogThreadedSourceWorker *w)
{
  PythonSourceDriver *control = (PythonSourceDriver *) w->control;

  PyGILState_STATE gstate = PyGILState_Ensure();
  _py_invoke_request_exit(control);
  PyGILState_Release(gstate);
}

static const gchar *
python_source_format_persist_name(const LogPipe *s)
{
  const PythonSourceDriver *self = (const PythonSourceDriver *)s;

  PythonPersistMembers options =
  {
    .generate_persist_name_method = self->py.generate_persist_name,
    .options = self->binding.options,
    .class = self->binding.class,
    .id = self->super.super.super.id
  };

  return python_format_persist_name(s, "python-source", &options);
}

static gboolean
python_sd_init(LogPipe *s)
{
  PythonSourceDriver *self = (PythonSourceDriver *) s;
  GlobalConfig *cfg = log_pipe_get_config(s);

  if (!python_binding_init(&self->binding, cfg, self->super.super.super.id))
    return FALSE;

  if(!_py_sd_init(self))
    return FALSE;

  msg_verbose("python-source: Python source initialized",
              evt_tag_str("driver", self->super.super.super.id),
              evt_tag_str("class", self->binding.class));

  gboolean retval = log_threaded_source_driver_init_method(s);
  if (!retval)
    return FALSE;

  if (self->py.suspend_method && self->py.wakeup_method)
    {
      self->post_message = _post_message_non_blocking;
    }

  self->super.auto_close_batches = ((PyLogSource *) self->py.instance)->auto_close_batches;

  return TRUE;
}

static LogThreadedSourceWorker *
_construct_worker(LogThreadedSourceDriver *s, gint worker_index)
{
  /* PythonSourceDriver uses the multi-worker API, but it is not prepared to work with more than one worker. */
  g_assert(s->num_workers == 1);

  PythonSourceDriver *self = (PythonSourceDriver *) s;

  LogThreadedSourceWorker *worker = g_new0(LogThreadedSourceWorker, 1);
  log_threaded_source_worker_init_instance(worker, s, worker_index);

  worker->request_exit = python_sd_worker_request_exit;
  worker->run = python_sd_worker_run;

  if (self->py.suspend_method && self->py.wakeup_method)
    worker->wakeup = python_sd_worker_wakeup;

  return worker;
}

static gboolean
python_sd_deinit(LogPipe *s)
{
  PythonSourceDriver *self = (PythonSourceDriver *) s;

  AckTracker *ack_tracker = _py_sd_get_ack_tracker(self);
  ack_tracker_deinit(ack_tracker);

  PyGILState_STATE gstate = PyGILState_Ensure();
  _py_invoke_deinit(self);
  PyGILState_Release(gstate);

  python_binding_deinit(&self->binding);

  return log_threaded_source_driver_deinit_method(s);
}

static void
python_sd_free(LogPipe *s)
{
  PythonSourceDriver *self = (PythonSourceDriver *) s;

  PyGILState_STATE gstate = PyGILState_Ensure();
  _py_invoke_finalize(self);
  _py_free_bindings(self);
  PyGILState_Release(gstate);

  python_binding_clear(&self->binding);
  log_threaded_source_driver_free_method(s);
}

LogDriver *
python_sd_new(GlobalConfig *cfg)
{
  PythonSourceDriver *self = g_new0(PythonSourceDriver, 1);

  log_threaded_source_driver_init_instance(&self->super, cfg);
  self->super.super.super.super.init = python_sd_init;
  self->super.super.super.super.deinit = python_sd_deinit;
  self->super.super.super.super.free_fn = python_sd_free;
  self->super.super.super.super.generate_persist_name = python_source_format_persist_name;

  self->super.format_stats_key = python_sd_format_stats_key;
  self->super.worker_options.super.stats_level = STATS_LEVEL0;
  self->super.worker_options.super.stats_source = stats_register_type("python");
  self->super.worker_construct = _construct_worker;

  self->post_message = _post_message_blocking;

  python_binding_init_instance(&self->binding);

  return &self->super.super.super;
}


static PyMethodDef py_log_source_methods[] =
{
  { "post_message", (PyCFunction) py_log_source_post, METH_VARARGS | METH_KEYWORDS, "Post message" },
  { "close_batch", (PyCFunction) py_log_source_close_batch, METH_NOARGS, "Close input batch" },
  { "set_transport_name", (PyCFunction) py_log_source_set_transport_name, METH_VARARGS, "Set transport name" },
  {NULL}
};

static PyMemberDef py_log_source_members[] =
{
  { "persist_name", T_STRING, offsetof(PyLogSource, persist_name), READONLY },
  { "auto_close_batches", T_BOOL, offsetof(PyLogSource, auto_close_batches), 0 },
  {NULL}
};

static PyTypeObject py_log_source_type =
{
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "LogSource",
  .tp_basicsize = sizeof(PyLogSource),
  .tp_dealloc = py_slng_generic_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_doc = "The LogSource class is a base class for custom Python sources.",
  .tp_new = PyType_GenericNew,
  .tp_methods = py_log_source_methods,
  .tp_members = py_log_source_members,
  0,
};

void
py_log_source_global_init(void)
{
  PyType_Ready(&py_log_source_type);
  PyModule_AddObject(PyImport_AddModule("_syslogng"), "LogSource", (PyObject *) &py_log_source_type);
}
