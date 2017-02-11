#ifndef _SPACE_H
#define _SPACE_H

#include <stdlib.h>

#include "struct.h"
#include "block.h"

n_float sp_n_get(struct space *sp, n_v3i pos);
n_float sp_a_get(struct space *sp, n_v3i pos);
void sp_a_set(struct space *sp, n_v3i pos, n_float a);
void sp_n_set(struct space *sp, n_v3i pos, n_float n, int con);
int sp_con_get(struct space *sp, n_v3i pos);

int sp_pos_inside(struct space *sp, n_v3i pos);

struct space *sp_init(n_v3f step);
void sp_done(struct space *sp);

int sp_load(struct space *sp, n_v2i pos, n_v2i size);
void sp_unload(struct space *sp);

void sp_border(struct space *sp);

void sp_optimize(struct space *sp);

#endif
