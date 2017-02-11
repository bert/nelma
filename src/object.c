#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "error.h"
#include "assert.h"
#include "loadobj.h"
#include "data.h"

/** @brief Amount of memory used by object maps (bytes) */
static long int obj_memsize=0;

/** @brief Prints the amount of memory used by object maps */
void obj_report_mem()
{
	info("%ld bytes allocated for object maps", obj_memsize);
}

/** @brief Allocate and initialize a new object structure. 
 *
 * @param pos Position of the bottom left corner of the object in grid
 * coordinates.
 * @param mat Material for the object.
 * @return Pointer to the allocated struct or NULL on error. */
struct object *obj_init(n_v2i pos, struct material *mat)
{
	struct object *obj;

	obj=calloc(1, sizeof(*obj));
	if(obj==NULL) return NULL;

	obj->map=NULL;

	obj->orig_pos=pos;

	obj->pos=v2i(-1,-1);
	obj->size=v2i(-1, -1);

	obj->radius=-1;
	obj->imgpos=NULL;
	obj->image=NULL;

	obj->mat=mat;

	obj->con=1;
	obj->n=0.0;

	obj->name=NULL;
	obj->next=NULL;

	return obj;
}

/** @brief Frees all memory used by an object structure.
 *
 * @param obj Pointer to the structure to be freed. */
void obj_done(struct object *obj)
{
	assert(obj!=NULL);

	if(obj->map!=NULL) {
		obj_unload(obj);
	}
	if(obj->name!=NULL) free(obj->name);
	if(obj->image!=NULL) free(obj->image);
	if(obj->imgpos!=NULL) free(obj->imgpos);
	free(obj);

	return;
}

/** @brief Loads object's bitmap into memory and sets position and size 
 * fields.
 *
 * @param obj Pointer to the structure to be loaded.
 * @return 0 on success and -1 on error. */
int obj_load(struct object *obj)
{
	int r;

	if(obj->map!=NULL) return 0;

	switch(obj->type) {
		case rectangle: r=obj_load_rectangle(obj);
				break;

		case circle:	r=obj_load_circle(obj);
				break;

		case image:	r=obj_load_image(obj);
				break;

		default:	return -1;
	}

	obj_memsize+=obj->size.x * obj->size.y;

	return r;
}

/** @brief Frees object's bitmap. Position and size fields remain set.
 *
 * @param obj Pointer to the structure to be unloaded. */
void obj_unload(struct object *obj)
{
	if(obj->map==NULL) return;
	
	free(obj->map);
	obj->map=NULL;

	obj_memsize-=obj->size.x * obj->size.y;

	return;
}

/** @brief Trims unused space on the edges of the bitmap.
 *
 * Changes bitmap, size and position fields.
 *
 * @param obj Pointer to the structure to be trimmed.
 * @return 0 on success and -1 on error. */
int obj_shrink_tight(struct object *obj)
{
	char *new_map;
	size_t new_memsize;

	n_v2i new_size, new_pos;
	n_v2i n,min,max;

	min.x=obj->size.x-1;
	min.y=obj->size.y-1;
	max.x=0;
	max.y=0;

	for(n.y=0;n.y<obj->size.y;n.y++) {
		for(n.x=0;n.x<obj->size.x;n.x++) {
			if(map_get(obj->map, n, obj->size)) {
				if(n.x<min.x) min.x=n.x;
				if(n.y<min.y) min.y=n.y;

				if(n.x>max.x) max.x=n.x;
				if(n.y>max.y) max.y=n.y;
			}
		}
	}

	assert(min.x<max.x);
	assert(min.y<max.y);

	new_size.x=max.x-min.x+1;
	new_size.y=max.y-min.y+1;

	new_pos.x=obj->pos.x+min.x;
	new_pos.y=obj->pos.y+min.y;

	new_memsize=new_size.x * new_size.y;
	new_map=calloc(new_memsize, sizeof(*new_map));
	obj_memsize+=new_size.x * new_size.y;

	if(new_map==NULL) return -1;

	for(n.y=0;n.y<obj->size.y;n.y++) {
		for(n.x=0;n.x<obj->size.x;n.x++) {
			if(map_get(obj->map, n, obj->size)) {
				assert(n.x-min.x>=0);
				assert(n.y-min.y>=0);
				map_set(new_map, v2i_sub(n, min), new_size, 1);
			}
		}
	}

	free(obj->map);
	obj_memsize-=obj->size.x * obj->size.y;

	//debug("obj_shrink_tight: before (%d, %d) after (%d, %d)", obj->pos.x,
	//	obj->pos.y, new_pos.x, new_pos.y);

	obj->pos=new_pos;
	obj->size=new_size;
	obj->map=new_map;

	return 0;
}

/** @brief Inverts object's bitmap.
 *
 * @param obj Pointer to the object to be inverted. */
void obj_invert(struct object *obj)
{
	n_v2i n;

	assert(obj->map!=NULL);

	for(n.y=0;n.y<obj->size.y;n.y++) {
		for(n.x=0;n.x<obj->size.x;n.x++) {
			if(map_get(obj->map, n, obj->size)) {
				map_set(obj->map, n, obj->size, 0);
			} else {
				map_set(obj->map, n, obj->size, 1);
			}
		}
	}
}

/** @brief Grows the object's bitmap by the specified radius.
 *
 * @param obj Pointer to the object.
 * @param r Radius of the growth (0 means no change).
 * @param square If set to 1 the object will retain sharp corners.
 * @return 0 on success and -1 on error. */
int obj_grow(struct object *obj, int r, int square) 
{
	char *new_map;
	size_t new_memsize;

	n_v2i new_size;
	n_v2i n,m;

	assert(r>0);

	new_size=v2i_add(obj->size, v2i(r*2,r*2));
	new_memsize=new_size.x * new_size.y;

	new_map=calloc(new_memsize, sizeof(*new_map));
	if(new_map==NULL) return -1;
	obj_memsize+=new_size.x * new_size.y;

	for(n.y=0;n.y<obj->size.y;n.y++) {
		for(n.x=0;n.x<obj->size.x;n.x++) {
			if(!map_get(obj->map, n, obj->size)) continue;

			m=v2i_add(n, v2i(r,r));

			if(square==GROW_SQUARE) {
				map_set_square(new_map, m, r, new_size, 1);
			} else {
				map_set_circle(new_map, m, r, new_size, 1);
			}
		}
	}

	free(obj->map);
	obj_memsize-=obj->size.x * obj->size.y;

	obj->pos=v2i_sub(obj->pos, v2i(r, r));
	obj->size=new_size;

	obj->map=new_map;

	return 0;
}

/** @brief Duplicates an object structure.
 *
 * Bitmap and all fields except the \a next pointer are duplicated.
 *
 * @param obj Pointer to the object to be duplicated.
 * @return Pointer to the duplicate. */
struct object *obj_dup(struct object *obj)
{
	struct object *copy;
	size_t size;
	int n,m;

	copy=calloc(1, sizeof(*copy));
	if(copy==NULL) return NULL;
	memcpy(copy, obj, sizeof(*copy));

	if(obj->map!=NULL) {
		size=copy->size.x*copy->size.y;
		copy->map=calloc(size, sizeof(*copy->map));

		if(copy->map==NULL) {
			free(copy);
			return NULL;
		}

		obj_memsize+=copy->size.x * copy->size.y;

		memcpy(copy->map, obj->map, size*sizeof(*copy->map));
	}

	if(obj->imgpos!=NULL) {
		n=0;
		while(v2i_positive(obj->imgpos[n])) n++;

		n++;

		copy->imgpos=calloc(n, sizeof(*obj->imgpos));

		for(m=0;m<n;m++) {
			copy->imgpos[m]=obj->imgpos[m];
		}
	}

	if(obj->name!=NULL) {
		copy->name=strdup(obj->name);
	}

	if(obj->image!=NULL) {
		copy->image=strdup(obj->image);
	}

	copy->next=NULL;

	return copy;
}

#define MIN(a,b)	((a)>(b)?(b):(a))
#define MAX(a,b)	((a)>(b)?(a):(b))

/** @brief Returns a new object that is the composite of two objects.
 *
 * Bitmaps of the objects are merged. Size and position are adjusted.
 *
 * @param obj1 Pointer to the first object.
 * @param obj2 Pointer to the second object.
 * @return Pointer to the new composite object. */
struct object *obj_merge(struct object *obj1, struct object *obj2)
{
	struct object *dest;
	size_t memsize;

	n_v2i pos, off, size, n;

	pos.x=MIN(obj1->pos.x, obj2->pos.x);
	pos.y=MIN(obj1->pos.y, obj2->pos.y);

	n.x=MAX(obj1->pos.x+obj1->size.x, obj2->pos.x+obj2->size.x);
	n.y=MAX(obj1->pos.y+obj1->size.y, obj2->pos.y+obj2->size.y);

	size=v2i_sub(n, pos);

	dest=obj_init(pos, obj1->mat);

	dest->pos=dest->orig_pos;

	if(dest==NULL) return NULL;

	dest->size=size;

	dest->name=strdup("__obj_merge__");

	memsize=size.x * size.y;
	dest->map=calloc(memsize, sizeof(*dest->map));
	if(dest->map==NULL) {
		obj_done(dest);
		return NULL;
	}

	obj_memsize+=size.x * size.y;

	off=v2i_sub(obj1->pos, dest->pos);
	for(n.y=0;n.y<obj1->size.y;n.y++) {
		for(n.x=0;n.x<obj1->size.x;n.x++) {
			if(map_get(obj1->map, n, obj1->size)) {
				map_set(dest->map, v2i_add(off, n), dest->size, 1);
			}
		}
	}
	off=v2i_sub(obj2->pos, dest->pos);
	for(n.y=0;n.y<obj2->size.y;n.y++) {
		for(n.x=0;n.x<obj2->size.x;n.x++) {
			if(map_get(obj2->map, n, obj2->size)) {
				map_set(dest->map, v2i_add(off, n), dest->size, 1);
			}
		}
	}

	return dest;
}
