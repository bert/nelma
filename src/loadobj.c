#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "data.h"
#include "error.h"
#include "assert.h"
#include "pngutil.h"
#include "object.h"

static int pos_check(n_v2i pos, n_v2i size)
{
	return((pos.x<size.x)&&(pos.y<size.y)&&(pos.x>=0)&&(pos.y>=0));
}

/** @brief Recursive line flood-fill algorithm.
 *
 * @param obj Destination object
 * @param img Source image
 * @param pos Current drawing position
 * @param opos Seed point position */
static void obj_color_line(struct object *obj, struct image *img, 
							n_v2i pos, n_v2i opos) 
{
	img_color op, p;
	n_v2i n;
	n_int w, e;

	op=png_get_pixel(img, opos);

	p=png_get_pixel(img, pos);

	if(op!=p) return;

	if(map_get(obj->map, pos, obj->size)) return;

	map_set(obj->map, pos, obj->size, 1);

	n.y=pos.y;
	for(n.x=pos.x+1;n.x<obj->size.x;n.x++) {
		p=png_get_pixel(img, n);
		if(op!=p) break;
		if(map_get(obj->map, n, obj->size)) break;

		map_set(obj->map, n, obj->size, 1);
	}

	e=n.x;

	for(n.x=pos.x-1;n.x>=0;n.x--) {
		p=png_get_pixel(img, n);
		if(op!=p) break;
		if(map_get(obj->map, n, obj->size)) break;

		map_set(obj->map, n, obj->size, 1);
	}

	w=n.x;

	for(n.x=w+1;n.x<e;n.x++) {
		n.y=pos.y+1;
		if(n.y<obj->size.y) {
			obj_color_line(obj, img, n, opos);
		}
		n.y=pos.y-1;
		if(n.y>=0) {
			obj_color_line(obj, img, n, opos);
		}
	}
}

/*
static void obj_color(struct object *obj, uch *image, int bpp, 
							n_v2i pos, n_v2i opos) 
{
	uch *op, *p;
	n_v2i npos;

	op=get_pixel(image, bpp, obj->size, opos);
	p=get_pixel(image, bpp, obj->size, pos);

	if(!memcmp(op, p, bpp)) {
		map_set(obj->map, pos, obj->size, 1);

		npos=v2i_add(pos, v2i(1,0));
		if(pos_check(npos, obj->size)&&!map_get(obj->map, npos, obj->size)) {
			obj_color(obj, image, bpp, npos, opos);
		}
		npos=v2i_add(pos, v2i(-1,0));
		if(pos_check(npos, obj->size)&&!map_get(obj->map, npos, obj->size)) {
			obj_color(obj, image, bpp, npos, opos);
		}
		npos=v2i_add(pos, v2i(0,1));
		if(pos_check(npos, obj->size)&&!map_get(obj->map, npos, obj->size)) {
			obj_color(obj, image, bpp, npos, opos);
		}
		npos=v2i_add(pos, v2i(0,-1));
		if(pos_check(npos, obj->size)&&!map_get(obj->map, npos, obj->size)) {
			obj_color(obj, image, bpp, npos, opos);
		}
	}

	return;
}
*/

/** @brief Loads a circular object. */
int obj_load_circle(struct object *obj)
{
	n_v2i n;
	n_v2i center;
	double d;
	size_t memsize;

	assert(obj->type==circle);

	obj->size.x=obj->radius*2;
	obj->size.y=obj->radius*2;

	obj->pos.x=obj->orig_pos.x+obj->radius;
	obj->pos.y=obj->orig_pos.y+obj->radius;

	center.x=obj->radius;
	center.y=obj->radius;

	memsize=obj->size.x * obj->size.y;
	obj->map=calloc(memsize, sizeof(*obj->map));
	if(obj->map==NULL) {
		return -1;
	}

	for(n.x=0;n.x<obj->size.x;n.x++) {
		for(n.y=0;n.y<obj->size.y;n.y++) {
			d=((n.x+0.5)-center.x)*((n.x+0.5)-center.x)+
				((n.y+0.5)-center.y)*((n.y+0.5)-center.y);
			if(d<obj->radius*obj->radius) {
				map_set(obj->map, n, obj->size, 1);
			}
		}
	}

	return 0;
}

/** @brief Loads a rectangular object. */
int obj_load_rectangle(struct object *obj)
{
	n_v2i n;
	size_t memsize;

	assert(obj->type==rectangle);

	memsize=obj->size.x * obj->size.y;
	obj->map=calloc(memsize, sizeof(*obj->map));
	if(obj->map==NULL) {
		return -1;
	}

	for(n.x=0;n.x<obj->size.x;n.x++) {
		for(n.y=0;n.y<obj->size.y;n.y++) {
			map_set(obj->map, n, obj->size, 1);
		}
	}

	obj->pos=obj->orig_pos;

	return 0;
}

/** @brief Loads image object.
 *
 * Object's geometry is loaded from a PNG file with a algorithm that is
 * similar to flood-fill. This means that the object will be composed of all
 * points in the image that are connected to the seed point and are of the 
 * same color.
 *
 * The position @a pos always refers to the lower left corner of the image, 
 * even if the actual object is smaller.
 *
 * <pre>
 *             v--- image coordinates
 *         (0, 0) ____________
 *               |            |  . = seed point
 *               |   ____     |
 *               |  |  . |    |
 *               |  |    |    |
 *               |  |____|<------ actual object
 *               |  ^         |
 *               |  '------------ world coordinates of the actual object
 *               |____________|
 * (pos.x, pos.y) 
 *             ^- world coordinates in the argument
 * </pre>
 */
int obj_load_image(struct object *obj)
{
	int r,n;

	struct image *img;
	size_t memsize;

	n_v2i imgpos;

	assert(obj->type==image);
	assert(obj->imgpos!=NULL);

	r=png_read(&img, obj->image);
	if(r!=0) {
		error("Can't read %s", obj->image);
		return -1;
	}
		
	obj->size.x=img->width;
	obj->size.y=img->height;

	memsize=obj->size.x * obj->size.y;

	obj->map=calloc(memsize, sizeof(*obj->map));
	if(obj->map==NULL) {
		png_done(img);
		return -1;
	}

	n=0;
	while(1) {
		if(!v2i_positive(obj->imgpos[n])) break;

		if((img->width<=obj->imgpos[n].x)||
					(img->height<=obj->imgpos[n].y)||
					(obj->imgpos[n].x<0)||
					(obj->imgpos[n].y<0)) {
			error("Invalid position (%d, %d) in "
					"PNG file with dimensions (%d, %d)", 
					obj->imgpos[n].x, obj->imgpos[n].y, 
					img->width, img->height);
			png_done(img);
			free(obj->map);
			return -1;
		}

		imgpos.x = obj->imgpos[n].x;
		imgpos.y = obj->imgpos[n].y;

		obj_color_line(obj, img, imgpos, imgpos);

		n++;
	}

	png_done(img);

	obj->pos=obj->orig_pos;

	obj_shrink_tight(obj);

	return 0;
}
