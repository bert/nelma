#ifndef _PNGUTIL_H
#define _PNGUTIL_H

#include <limits.h>

#include "num.h"

typedef unsigned int img_color;

#define IMG_COLOR_MAX UINT_MAX

/** @brief Structure describing a gray-scale bitmap image. */
struct image {
	/** @brief Gray-scale image with 8 bits per pixel. */
	img_color **data;

	/** @brief Width of the image in pixels. */
	unsigned int width;
	/** @brief Height of the image in pixels. */
	unsigned int height;
};

int png_read(struct image **dest, char *file_name);
int png_write(struct image *img, char *file_name);

struct image *png_alloc(int width, int height);
void png_done(struct image *img);

img_color png_get_pixel(struct image *img, n_v2i pos);
void png_set_pixel(struct image *img, n_v2i pos, img_color p);

#endif
