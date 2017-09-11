/**
 * @file src/error.c
 *
 * @brief TABLIX, PGA general timetable solver.
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

/* $Id: error.c,v 1.2 2006-09-22 15:46:27 avian Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "error.h"

int a_verbosity=2;

#define LINESIZE	1024

static void msg_print(int prio, const char *fmt, va_list ap)
{
	char msg[LINESIZE];

	if(prio>a_verbosity) {
		return;
	}

	vsnprintf(msg, LINESIZE, fmt, ap);
	msg[LINESIZE-1]=0;

        fprintf(stderr, "%s\n", msg);
}

void error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	msg_print(0, fmt, ap);
	
	va_end(ap);
}

void warning(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	msg_print(1, fmt, ap);

	va_end(ap);
}

void info(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	msg_print(2, fmt, ap);

	va_end(ap);
}

void debug(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	msg_print(3, fmt, ap);

	va_end(ap);
}
