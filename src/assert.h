#ifndef _ASSERT_H
#define _ASSERT_H

#ifdef DEBUG

#include <stdio.h>
#include <stdlib.h>

#define assert(expr) if(!(expr)) {					\
			fprintf(stderr, "assertion failed: "		\
					"%s, file %s, line %d\n",	\
			__STRING(expr), __FILE__, __LINE__);           	\
			abort();					\
		     }

#else

#define assert(expr)

#endif

#endif
