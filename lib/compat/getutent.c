/*
 * Copyright (c) 2002-2012 Balabit
 * Copyright (c) 1998-2012 Balázs Scheidler
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

#include "compat/getutent.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#if !defined(SYSLOG_NG_HAVE_GETUTENT) && !defined(SYSLOG_NG_HAVE_GETUTXENT) && defined(SYSLOG_NG_HAVE_UTMP_H)

static int utent_fd = -1;

#ifndef _PATH_UTMP
#define _PATH_UTMP "/var/log/utmp"
#endif

struct utmp *getutent(void)
{
  static struct utmp ut;
  int rc;

  if (utent_fd == -1)
    {
      utent_fd = open(_PATH_UTMP, O_RDONLY | O_NOCTTY);
    }
  if (utent_fd == -1)
    return NULL;
  rc = read(utent_fd, &ut, sizeof(ut));
  if (rc <= 0)
    {
      close(utent_fd);
      utent_fd = -1;
      return NULL;
    }
  else
    {
      return &ut;
    }
}

void endutent(void)
{
  if (utent_fd != -1)
    {
      close(utent_fd);
      utent_fd = -1;
    }
}

#endif
