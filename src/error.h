/**
 * @file src/error.h
 *
 * @brief TABLIX, PGA general timetable solver
 *
 * @author Copyright (C) 2002-2006 Tomaz Solc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* $Id: error.h,v 1.2 2006-09-22 15:46:27 avian Exp $ */

#ifndef _ERROR_H
#define _ERROR_H

#include <stdarg.h>

extern int a_verbosity;

void error(const char *fmt, ...);
void warning(const char *fmt, ...);
void info(const char *fmt, ...);
void debug(const char *fmt, ...);

#endif
