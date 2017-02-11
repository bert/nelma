#ifndef _OBJECT_H
#define _OBJECT_H

#include "struct.h"

#define GROW_SQUARE	1
#define GROW_ROUND	0

void obj_report_mem();

struct object *obj_init(n_v2i pos, struct material *mat);
void obj_done(struct object *obj);
int obj_load(struct object *obj);
void obj_unload(struct object *obj);
int obj_shrink_tight(struct object *obj);
void obj_invert(struct object *obj);
int obj_grow(struct object *obj, int r, int square);
struct object *obj_dup(struct object *obj);
struct object *obj_merge(struct object *obj1, struct object *obj2);

#endif
