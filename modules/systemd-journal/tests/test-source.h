/*
 * Copyright (c) 2010-2014 Balabit
 * Copyright (c) 2010-2014 Viktor Juhasz <viktor.juhasz@balabit.com>
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

#ifndef TEST_SOURCE_H_
#define TEST_SOURCE_H_

#include "syslog-ng.h"
#include "journal-reader.h"
#include "journald-mock.h"
#include <iv.h>

typedef struct _TestSource TestSource;
typedef struct _TestCase TestCase;

struct _TestCase
{
  void (*init)(TestCase *self, TestSource *src, JournalReader *reader, JournalReaderOptions *options);
  void (*checker)(TestCase *self, TestSource *src, LogMessage *msg);
  void (*finish)(TestCase *self);
  gpointer user_data;
};


TestSource *test_source_new(GlobalConfig *cfg);

void test_source_add_test_case(TestSource *s, TestCase *tc);
void test_source_run_tests(TestSource *s);
void test_source_finish_tc(TestSource *s);

void *journal_reader_test_prepare_with_namespace(const gchar *namespace, GlobalConfig *cfg);
gboolean journal_reader_test_allocate_namespace(void *test);
void journal_reader_test_destroy(void *test);

#endif /* TEST_SOURCE_H_ */
