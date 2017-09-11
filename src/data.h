/**
 * @file src/data.h
 *
 * @brief , header.
 */

#ifndef _DATA_H
#define _DATA_H

#include "struct.h"

void lay_report_mem();

char map_get(char *map, n_v2i pos, n_v2i size);
void map_set(char *map, n_v2i pos, n_v2i size, char c);
void sp_clear(struct space *sp);
void sp_fill(struct space *sp, char con, n_float n);
struct layer *lay_init(n_int height, n_int order, struct material *mat);
void lay_done(struct layer *lay);
int lay_compare(const void *a, const void *b);
void sp_lay_attach(struct space *sp, struct layer *lay);
struct layer *sp_lay_find(struct space *sp, n_int z);
struct material *mat_init(enum material_type type, n_float e, n_float g,
								n_float u);
void mat_done(struct material *mat);
int lay_mat_attach(struct layer *lay, struct material *mat);
struct material *lay_get_mat(struct layer *lay, n_v2i pos);
int lay_obj_attach(struct layer *lay, struct object *obj);
int lay_add_obj(struct layer *lay, struct object *obj);
void sp_add_obj(struct space *sp, struct object *obj, struct layer *lay);
void sp_add_obj_all(struct space *sp);

struct face *face_new(struct face *prev, n_v3i pos, n_v3i e1, n_v3i e2);
void face_done(struct face *list);
struct face *face_line(struct face *prev, n_v3i pos, n_v3i e1, n_v3i e2, 
							n_v3i mov, int m);
struct face *net_get_face_sp(struct net *net, struct space *sp, n_int standoff);

struct net *net_init();
void net_done(struct net *d);
int net_obj_attach(struct net *net, struct object *obj);
void net_set(struct net *net, n_float n, int con);
struct face *net_get_face_sp(struct net *net, struct space *sp, n_int standoff);
struct object *net_get_composite(struct net *net);
void map_set_circle(char *map, n_v2i center, int r, n_v2i size, char c);
void map_set_square(char *map, n_v2i center, int r, n_v2i size, char c);
void sp_optimize(struct space *sp);

int lay_load(struct layer *lay, n_v2i pos, n_v2i size);
void lay_unload(struct layer *lay);
#endif
