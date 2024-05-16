/*
 * Copyright (c) 2002-2017 Balabit
 * Copyright (c) 1998-2017 Balázs Scheidler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#ifndef CORRELATION_PDB_CONTEXT_H_INCLUDED
#define CORRELATION_PDB_CONTEXT_H_INCLUDED

#include "pdb-rule.h"

/**************************************************************************
 * PDBContext, represents a correlation state in the state hash table, is
 * marked with PSK_CONTEXT in the hash table key
 **************************************************************************/

/* This class encapsulates a correlation context, keyed by CorrelationKey, type == PSK_RULE. */
typedef struct _PDBContext
{
  CorrelationContext super;
  /* back reference to the last rule touching this context */
  PDBRule *rule;
} PDBContext;

PDBContext *pdb_context_new(CorrelationKey *key);

#endif
