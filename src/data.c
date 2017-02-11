#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "assert.h"
#include "struct.h"
#include "data.h"
#include "error.h"
#include "object.h"
#include "space.h"
#include "malloc.h"

/** @brief Return 0 if the coordinates of the point fall within the 
 * allocated chunk of the bitmap. 
 *
 * @param pos Coordinates of the point in map coordinates.
 * @param size Size of the allocated piece of the bitmap
 * @return 1 if \a pos is a valid map location or 0 otherwise. */
int map_check(n_v2i pos, n_v2i size)
{
	assert(size.x>0);
	assert(size.y>0);

	if((pos.x>=0)&&(pos.y>=0)&&(pos.x<size.x)&&(pos.y<size.y)) {
		return 1;
	} else {
		return 0;
	}
}

int p_overlap(n_v2i pos, n_v2i size, n_v2i p)
{
	if((p.x >= pos.x) && (p.y >= pos.y) && 
					(p.x < pos.x + size.x) && 
					(p.y < pos.y + size.y)) {
		return 1;
	} else {
		return 0;
	}
}

int rect_overlap(n_v2i pos1, n_v2i size1, n_v2i pos2, n_v2i size2)
{
	if(pos1.x >= (pos2.x + size2.x)) return 0;
	if((pos1.x + size1.x) < pos2.x) return 0;

	if(pos1.y >= (pos2.y + size2.y)) return 0;
	if((pos1.y + size1.y) < pos2.y) return 0;

	return 1;
}

char map_get(char *map, n_v2i pos, n_v2i size)
{
	size_t off;

	assert(map!=NULL);
	assert(pos.x < size.x);
	assert(pos.y < size.y);

	assert(pos.x >= 0);
	assert(pos.y >= 0);

	off=pos.y * size.x + pos.x;
	return(map[off]);
}

void map_set(char *map, n_v2i pos, n_v2i size, char c)
{
	size_t off;

	assert(map!=NULL);
	assert(pos.x < size.x);
	assert(pos.y < size.y);

	assert(pos.x >= 0);
	assert(pos.y >= 0);


	off=pos.y * size.x + pos.x;
	map[off]=c;

	return;
}

/*
void sp_fill(struct space *sp, char con, n_float n)
{
	struct node *no;
	n_v3i pos;

	for(pos.x=0;pos.x<sp->size.x;pos.x++) {
		for(pos.y=0;pos.y<sp->size.y;pos.y++) {
			for(pos.z=0;pos.z<sp->size.z;pos.z++) {
				no=sp_node(sp, pos);
				no->n=n;
				no->con=con;
			}
		}
	}

	return;
}
*/

/*
void sp_clear(struct space *sp)
{
	size_t memsize;

	memsize=((size_t) sp->size.x)*((size_t) sp->size.y)*((size_t) sp->size.z);

	memset(sp->data, 0x00, memsize*sizeof(*sp->data));

	return;
}
*/

/** @brief Allocate and initialize a new layer structure. 
 *
 * @param height Height of the layer in grid units. 
 * @param order Z-order of the layer. Layer with the lowest order number 
 * has the lowest z coordinates.
 * @param mat Default material for this layer.
 * @return Pointer to the allocated struct or NULL on error. */
struct layer *lay_init(n_int height, n_int order, struct material *mat)
{
	struct layer *lay;

	assert(height>0);
	assert(mat!=NULL);

	lay=calloc(1, sizeof(*lay));
	if(lay==NULL) return NULL;

	lay->height=height;
	lay->order=order;

	lay->z=-1;

	/* v2i_sub(v2i_cz(sp->size), v2i(1, 1)); */

	lay->mat=mat;

	lay->obj=NULL;
	lay->objnum=0;

	lay->name=NULL;
	lay->next=NULL;

	return(lay);
}

void lay_done(struct layer *lay)
{
	if(lay->name!=NULL) free(lay->name);
	if(lay->obj!=NULL) free(lay->obj);
	free(lay);

	return;
}

int lay_compare(const void *a, const void *b)
{
	struct layer *a1, *b1;

	a1=*((struct layer **) a);
	b1=*((struct layer **) b);

	return(a1->order-b1->order);
}

void sp_lay_attach(struct space *sp, struct layer *lay)
{
	n_int z;
	int n;

	assert(sp!=NULL);
	assert(lay!=NULL);

	sp->lay=realloc(sp->lay, sizeof(*sp->lay)*(sp->laynum+1));

	sp->lay[sp->laynum]=lay;

	sp->laynum++;

	qsort(sp->lay, sp->laynum, sizeof(*sp->lay), lay_compare);

	/* assign z coordinates to layers */
	z=0;

	for(n=0;n<sp->laynum;n++) {
		sp->lay[n]->z=z;
		z=z+sp->lay[n]->height;
	}

	sp->size.z=z;

	return;
}

struct layer *sp_lay_find(struct space *sp, n_int z) 
{
	int n;

	for(n=0;n<sp->laynum;n++) {
		if((sp->lay[n]->z<=z)&&(sp->lay[n]->z+sp->lay[n]->height>z)) {
			return(sp->lay[n]);
		}
	}

	return NULL;
}

struct material *mat_init(enum material_type type, n_float e, n_float g,
								n_float u)
{
	struct material *mat;

	mat=malloc(sizeof(*mat));

	mat->type=type;
	mat->e=e;
	mat->g=g;
	mat->u=u;

	mat->name=NULL;
	mat->next=NULL;

	return mat;
}

void mat_done(struct material *mat)
{
	if(mat->name!=NULL) free(mat->name);
	free(mat);

	return;
}

/** @brief Attaches an object to a layer.
 *
 * @param lay Pointer to the layer structure.
 * @param obj Pointer to the object to be attached.
 * @return Index of the attached object or -1 on error. */
int lay_obj_attach(struct layer *lay, struct object *obj)
{
	assert(lay!=NULL);
	assert(obj!=NULL);

	lay->obj=realloc(lay->obj, sizeof(*lay->obj)*(lay->objnum+1));
	if(lay->obj==NULL) return -1;

	lay->obj[lay->objnum]=obj;

	lay->objnum++;

	return lay->objnum-1;
}

/** @brief Sets a square region of a bitmap to a constant value. 
 *
 * All coordinates are relative to the bitmap lower-left corner 
 * (which has coordniate 0,0).
 *
 * Center of the square is set at the center of the square square. 
 *
 * @param center Coordinates of the center square.
 * @param r Radius.
 * @param map Pointer to the bitmap
 * @param size Size of the bitmap
 * @param c Value
 */
void map_set_square(char *map, n_v2i center, int r, n_v2i size, char c)
{
	n_v2i n;

	assert(map!=NULL);
	assert(center.x >= r);
	assert(center.y >= r);
	assert(center.x < size.x-r);
	assert(center.y < size.y-r);

	for(n.x = center.x-r ; n.x <= center.x+r ; n.x++) {
		for(n.y = center.y-r ; n.y <= center.y+r ; n.y++) {
				map_set(map, n, size, c);
		}
	}
}

/** @brief Sets a circular region of a bitmap to a constant value. 
 *
 * All coordinates are relative to the bitmap lower-left corner 
 * (which has coordniate 0,0).
 *
 * Center of the circle is set at the center of the center square. 
 *
 * @param center Coordinates of the center square.
 * @param r Radius.
 * @param map Pointer to the bitmap
 * @param size Size of the bitmap
 * @param c Value
 */
void map_set_circle(char *map, n_v2i center, int r, n_v2i size, char c)
{
	n_v2i n;
	int d;

	assert(map!=NULL);
	assert(center.x >= r);
	assert(center.y >= r);
	assert(center.x < size.x-r);
	assert(center.y < size.y-r);

	for(n.x = center.x-r ; n.x <= center.x+r ; n.x++) {
		for(n.y = center.y-r ; n.y <= center.y+r ; n.y++) {
			d=(n.x-center.x)*(n.x-center.x)+
				(n.y-center.y)*(n.y-center.y);
			if(d<=r*r) {
				map_set(map, n, size, c);
			}
		}
	}
}

void sp_add_obj(struct space *sp, struct object *obj, struct layer *lay)
{
	n_v3i pos;
	n_v3i abspos;
	
	assert(sp!=NULL);
	assert(obj!=NULL);
	assert(lay!=NULL);

	/*
	if(!obj->con) {
		debug("sp_add_obj: skiping object %s on layer %s "
				"(not constant)", obj->name, lay->name);
		return;
	}
	*/

	
	if(rect_overlap(v2i_cz(sp->pos), v2i_cz(sp->size), 
							obj->pos, obj->size)) {
		obj_load(obj);
		debug("sp_add_obj: adding object %s to layer %s at (%d, %d)", 
				obj->name, lay->name, obj->pos.x, obj->pos.y);
	} else {
		debug("sp_add_obj: skiping object %s on layer %s "
				"(not visible)", obj->name, lay->name);
		return;
	}

	for(pos.z=lay->z;pos.z<=lay->z+lay->height;pos.z++) {
		for(pos.y=0;pos.y<obj->size.y;pos.y++) {
			for(pos.x=0;pos.x<obj->size.x;pos.x++) {
				if(!map_get(obj->map, v2i(pos.x, pos.y), 
								obj->size)) {
					continue;
				}

				abspos=v3i(obj->pos.x, obj->pos.y, 0);
				abspos=v3i_add(pos, abspos);

				if(sp_pos_inside(sp, abspos)) {
					sp_n_set(sp, abspos, obj->n, obj->con);
				}

				abspos=v3i(obj->pos.x+1, obj->pos.y, 0);
				abspos=v3i_add(pos, abspos);

				if(sp_pos_inside(sp, abspos)) {
					sp_n_set(sp, abspos, obj->n, obj->con);
				}

				abspos=v3i(obj->pos.x, obj->pos.y+1, 0);
				abspos=v3i_add(pos, abspos);

				if(sp_pos_inside(sp, abspos)) {
					sp_n_set(sp, abspos, obj->n, obj->con);
				}

				abspos=v3i(obj->pos.x+1, obj->pos.y+1, 0);
				abspos=v3i_add(pos, abspos);

				if(sp_pos_inside(sp, abspos)) {
					sp_n_set(sp, abspos, obj->n, obj->con);
				}
			}
		}
	}

	if(obj->mat!=NULL) {

		pos.z=lay->z;
		for(pos.y=0;pos.y<obj->size.y;pos.y++) {
			for(pos.x=0;pos.x<obj->size.x;pos.x++) {
				if(!map_get(obj->map, v2i(pos.x, pos.y), obj->size)) {
					continue;
				}

				abspos=v3i(obj->pos.x, obj->pos.y, 0);
				abspos=v3i_add(pos, abspos);

				if(sp_pos_inside(sp, abspos)) {
					sp_a_set(sp, abspos, obj->mat->e);
				}
			}
		}
	}

	return;
}

void sp_add_obj_all(struct space *sp)
{
	int n,m;
	struct object *obj;
	struct layer *lay;

	for(n=0;n<sp->laynum;n++) {

		lay=sp->lay[n];

		for(m=0;m<lay->objnum;m++) {
			obj=lay->obj[m];

			if(obj->role!=pin) {
				sp_add_obj(sp, obj, lay);
			}
		}
	}

	for(n=0;n<sp->laynum;n++) {

		lay=sp->lay[n];

		for(m=0;m<lay->objnum;m++) {
			obj=lay->obj[m];

			if(obj->role==pin) {
				sp_add_obj(sp, obj, lay);
			}
		}
	}

	return;
}

struct face *face_new(struct face *prev, n_v3i pos, n_v3i e1, n_v3i e2) 
{
	struct face *f;

	f=calloc(1, sizeof(*f));

	if(f==NULL) return NULL;

	f->pos=pos;
	f->e1=e1;
	f->e2=e2;
	f->n=v3i_vectp(e1, e2);

	f->next=prev;

	return f;
}

void face_done(struct face *list)
{
	struct face *cur, *next;

	cur=list;
	while(cur!=NULL) {
		next=cur->next;
		free(cur);

		cur=next;
	}

	return;
}

struct face *obj_get_face_pos(struct face *list, struct object *obj, 
						struct layer *lay, n_v2i pos)
{
	n_v2i t; /* object coordinates */
	n_v2i l; /* absolute 2d coordinates */

	n_v3i s; /* absolute 3d coordinates */

	l=v2i_add(pos, obj->pos);

	if(!map_get(obj->map, pos, obj->size)) {
		return(list);
	}

	t=v2i_add(pos, v2i(-1, 0));
	if(pos.x==0||(!map_get(obj->map, t, obj->size))) {
		s=v3i_ez(l, lay->z-1);
		list=face_line(list, s, v3i_z, v3i_y, v3i_z, lay->height+2);
	}
	t=v2i_add(pos, v2i(1, 0));
	if(pos.x==obj->size.x-1||(!map_get(obj->map, t, obj->size))) {
		s=v3i_ez(l, lay->z-1);
		s=v3i_add(s, v3i(1, 0, 0));
		list=face_line(list, s, v3i_y, v3i_z, v3i_z, lay->height+2);
	}
	t=v2i_add(pos, v2i(0, -1));
	if(pos.y==0||(!map_get(obj->map, t, obj->size))) {
		s=v3i_ez(l, lay->z-1);
		list=face_line(list, s, v3i_x, v3i_z, v3i_z, lay->height+2);
	}
	t=v2i_add(pos, v2i(0, 1));
	if(pos.y==obj->size.y-1||(!map_get(obj->map, t, obj->size))) {
		s=v3i_ez(l, lay->z-1);
		s=v3i_add(s, v3i(0, 1, 0));
		list=face_line(list, s, v3i_z, v3i_x, v3i_z, lay->height+2);
	}

	s=v3i_ez(l, lay->z-1);
	list=face_new(list, s, v3i_y, v3i_x);
	s=v3i_ez(l, lay->z+lay->height+1);
	list=face_new(list, s, v3i_x, v3i_y);

	return(list);
}

struct face *obj_get_face_lay(struct object *obj, struct layer *lay, 
							struct face *list)
{
	n_v2i o; /* position in object coordinates */

	//debug("obj_get_face_lay: object %s layer %s", obj->name, lay->name);
	
	for(o.y=0;o.y<obj->size.y;o.y++) {
		for(o.x=0;o.x<obj->size.x;o.x++) {
			list=obj_get_face_pos(list, obj, lay, o);
		}
	}

	return list;
}

struct face *obj_get_face_sp(struct object *obj, struct space *sp, 
					struct face *list, n_int standoff)
{
	struct object *cp;

	int n,m;

	if(obj->map==NULL) return list;

	cp=obj_dup(obj);
	obj_grow(cp, standoff, GROW_SQUARE);

	for(n=0;n<sp->laynum;n++) {
		for(m=0;m<sp->lay[n]->objnum;m++) {
			if(sp->lay[n]->obj[m]==obj) {
				if(rect_overlap(obj->pos, obj->size, 
					v2i_cz(sp->pos), v2i_cz(sp->size))) {

					list=obj_get_face_lay(cp, sp->lay[n], list);
				}
			}
		}
	}

	obj_done(cp);
	
	return list;
}

struct face *face_line(struct face *prev, n_v3i pos, n_v3i e1, n_v3i e2, 
							n_v3i mov, int m)
{
	unsigned int n;
	struct face *list;

	assert(m>0);

	list=prev;

	for(n=0;n<m;n++) {
		list=face_new(list, pos, e1, e2);

		pos=v3i_add(pos, mov);
	}

	return list;
}

struct net *net_init()
{
	struct net *d;

	d=calloc(1, sizeof(*d));
	if(d==NULL) return NULL;

	d->next=NULL;
	d->objnum=0;
	d->obj=NULL;

	d->name=NULL;

	return d;
}

void net_done(struct net *d)
{
	if(d->obj!=NULL) free(d->obj);
	if(d->name!=NULL) free(d->name);

	free(d);
}

int net_obj_attach(struct net *net, struct object *obj)
{
	assert(net!=NULL);
	assert(obj!=NULL);

	net->obj=realloc(net->obj, sizeof(*net->obj)*(net->objnum+1));
	if(net->obj==NULL) return -1;

	net->obj[net->objnum]=obj;

	net->objnum++;

	return net->objnum-1;
}

void net_set(struct net *net, n_float n, int con)
{
	int m;
	
	for(m=0;m<net->objnum;m++) {
		debug("net_set: setting object %s to %e", net->obj[m]->name, n);
		net->obj[m]->n=n;
		net->obj[m]->con=con;
	}
}

struct face *net_get_face_sp(struct net *net, struct space *sp, n_int standoff)
{
	struct face *list;
	int n,m,o;

	struct object *obj;
	struct object *cp, *temp;
	
	list=NULL;

	for(n=0;n<sp->laynum;n++) {

		cp=NULL;
		for(m=0;m<sp->lay[n]->objnum;m++) {
			obj=sp->lay[n]->obj[m];
			
			for(o=0;o<net->objnum;o++) {
				if(net->obj[o]==obj && obj->map!=NULL) {
					if(cp==NULL) {
						cp=obj_dup(obj);
					} else {
						temp=obj_merge(cp, obj);
						obj_done(cp);
						cp=temp;
					}
				}
			}
		}

		if(cp!=NULL) {
			obj_grow(cp, standoff, GROW_SQUARE);

			if(rect_overlap(cp->pos, cp->size, v2i_cz(sp->pos), 
							v2i_cz(sp->size))) {
				list=obj_get_face_lay(cp, sp->lay[n], list);
			} else {
				assert(0);
			}

			obj_done(cp);
		}
	}

	return list;
}

struct object *net_get_composite(struct net *net)
{
	struct object *p1, *p2;
	int n;

	assert(net->objnum>0);

	obj_load(net->obj[0]);
	p1=obj_dup(net->obj[0]);

	for(n=1;n<net->objnum;n++) {
		obj_load(net->obj[n]);
		p2=obj_merge(p1, net->obj[n]);

		obj_done(p1);
		p1=p2;
	}

	p1->mat=NULL;

	if(p1->name!=NULL) {
		free(p1->name);
		p1->name=strdup("__net_get_composite__");
	}

	return p1;
}
