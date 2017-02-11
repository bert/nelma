#ifndef _DUMP_H
#define _DUMP_H

#include "struct.h"

int lay_dump(struct space *sp, struct layer *lay, char *file);
int sp_dump(struct space *sp, n_axis projection, n_v3i pos, char *file);
int face_dump(struct face *list, char *file);
int map_dump(char *map, n_v2i size, char *file);

#endif
