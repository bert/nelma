/* This code is based on "example.c", part of the libpng distribution that
 * was placed in the public domain. */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "error.h"
#include "png.h"
#include "pngutil.h"
#include "num.h"
#include "assert.h"

 /* The png_jmpbuf() macro, used in error handling, became available in
  * libpng version 1.0.6.  If you want to be able to run your code with older
  * versions of libpng, you must define the macro yourself (but only if it
  * is not already defined by libpng!).
  */

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

/* Check to see if a file is a PNG file using png_sig_cmp().  png_sig_cmp()
 * returns zero if the image is a PNG and nonzero if it isn't a PNG.
 *
 * The function check_if_png() shown here, but not used, returns nonzero (true)
 * if the file can be opened and is a PNG, 0 (false) otherwise.
 *
 * If this call is successful, and you are going to keep the file open,
 * you should call png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK); once
 * you have created the png_ptr, so that libpng knows your application
 * has read that many bytes from the start of the file.  Make sure you
 * don't call png_set_sig_bytes() with more than 8 bytes read or give it
 * an incorrect number of bytes read, or you will either have read too
 * many bytes (your fault), or you are telling libpng to read the wrong
 * number of magic bytes (also your fault).
 *
 * Many applications already read the first 2 or 4 bytes from the start
 * of the image to determine the file type, so it would be easiest just
 * to pass the bytes to png_sig_cmp() or even skip that if you know
 * you have a PNG file, and call png_set_sig_bytes().
 */
#define PNG_BYTES_TO_CHECK 4
static int png_check(char *file_name, FILE **fp)
{
	unsigned char buf[PNG_BYTES_TO_CHECK];

	/* Open the prospective PNG file. */
	if ((*fp = fopen(file_name, "rb")) == NULL) {
		error("Can't open file %s: %s", file_name, strerror(errno));
		return -1;
	}

	/* Read in some of the signature bytes */
	if (fread(buf, 1, PNG_BYTES_TO_CHECK, *fp) != PNG_BYTES_TO_CHECK) {
		error("Unexpected end of file %s", file_name);
		return -1;
	}

	/* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
	   Return zero if they match */

	if(png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK)) {
		error("Wrong signature for file %s", file_name);
		return -1;
	} else {
		return 0;
	}
}

struct image *png_alloc(int width, int height)
{
	struct image *dest;
	int n;

	dest=calloc(1, sizeof(*dest));
	if(dest==NULL) return NULL;

	dest->width=width;
	dest->height=height;

	dest->data=calloc(height, sizeof(*dest->data));
	if(dest->data==NULL) {
		free(dest);
		return NULL;
	}

	for(n=0;n<height;n++) {
		dest->data[n]=calloc(dest->width, sizeof(*dest->data[n]));
		if(dest->data[n]==NULL) {
			free(dest->data);
			free(dest);
			return NULL;
		}
	}

	return dest;
}

void png_done(struct image *img)
{
	int n;

	for(n=0;n<img->height;n++) free(img->data[n]);
	free(img->data);
	free(img);
}

/* Read a PNG file. */
int png_read(struct image **dest, char *file_name)
{
	png_structp png_ptr;
	png_infop info_ptr;
	int png_transforms;
	FILE *fp;
	struct image *img;
	int n,m,bpp;
	unsigned char **row_pointers;
	int row_bytes;

	if(png_check(file_name, &fp)) {
		error("File is not a PNG image");
		return -1;
	}

	/* Create and initialize the png_struct with the desired error handler
	 * functions. If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters. We also supply the
	 * the compiler header file version, so that we know if the application
	 * was compiled with a compatible version of the library.  REQUIRED
	 */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 
							NULL, NULL, NULL);

	if (png_ptr == NULL) {
		error("Can't allocate memory");
		fclose(fp);
		return -1;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		error("Can't allocate memory");
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return -1;
	}

	/* Set error handling if you are using the setjmp/longjmp method 
	 * (this is the normal method of doing things with libpng). 
	 * REQUIRED unless you set up your own error handlers in the 
	 * png_create_read_struct() earlier. */

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		error("libpng error");
		/* Free all of the memory associated with the png_ptr and 
		 * info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		/* If we get here, we had a problem reading the file */
		return -1;
	}

	/* Set up the input control if you are using standard C streams */
	png_init_io(png_ptr, fp);

	/* If we have already read some of the signature */
	png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

	/*
	 * If you have enough memory to read in the entire image at once,
	 * and you need to specify only transforms that can be controlled
	 * with one of the PNG_TRANSFORM_* bits (this presently excludes
	 * dithering, filling, setting background, and doing gamma
	 * adjustment), then you can read the entire image (including
	 * pixels) into the info structure with this call:
	 */

	png_transforms = 	PNG_TRANSFORM_STRIP_16|
				PNG_TRANSFORM_STRIP_ALPHA|
				PNG_TRANSFORM_PACKING | 
				PNG_TRANSFORM_EXPAND;

	png_read_png(png_ptr, info_ptr, png_transforms, NULL);

	/* At this point you have read the entire image */
	img=png_alloc(	png_get_image_width(png_ptr, info_ptr), 
			png_get_image_height(png_ptr, info_ptr));

	row_bytes=png_get_rowbytes(png_ptr, info_ptr);

	bpp=row_bytes/img->width;

	debug("png_read: %d row bytes, %d img width, %d bits per pixel", row_bytes, img->width, bpp*8);

	row_pointers = png_get_rows(png_ptr, info_ptr);

	for(n=0;n<img->height;n++) {
		for(m=0;m<img->width;m++) {
			/* FIXME: only reads the lowest byte (red channel?) */
			img->data[n][m]=row_pointers[n][m*bpp];
		}
	}

	*dest=img;

	/* Clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	/* close the file */
	fclose(fp);

	/* that's it */
	return 0;
}

/* write a png file */
int png_write(struct image *img, char *file_name)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;

	/* open the file */
	fp = fopen(file_name, "wb");
	if (fp == NULL) {
		error("Can't open file %s for writing: %s", file_name, 
							strerror(errno));
		return -1;
	}

	/* Create and initialize the png_struct with the desired error handler
	 * functions. If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters. We also check that
	 * the library version is compatible with the one used at compile time,
	 * in case we are using dynamically linked libraries.  REQUIRED.
	 */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
							NULL, NULL, NULL);

	if (png_ptr == NULL) {
		error("Can't allocate memory");
		fclose(fp);
		return -1;
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		error("Can't allocate memory");
		fclose(fp);
		png_destroy_write_struct(&png_ptr,  NULL);
		return -1;
	}

	/* Set error handling.  REQUIRED if you aren't supplying your own
	 * error handling functions in the png_create_write_struct() call.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem reading the file */
		error("libpng error");
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return -1;
	}

	/* set up the output control if you are using standard C streams */
	png_init_io(png_ptr, fp);

	/* This is the hard way */

	/* Set the image information here.  Width and height are up to 2^31,
	 * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend 
	 * on the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
	 * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE,
	 * PNG_COLOR_TYPE_RGB, or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is 
	 * either PNG_INTERLACE_NONE orPNG_INTERLACE_ADAM7, and the 
	 * compression_type and filter_type MUST currently be 
	 * PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
	 */
	png_set_IHDR(png_ptr, info_ptr, img->width, img->height, 
						sizeof(img_color)*8, 
						PNG_COLOR_TYPE_GRAY,
						PNG_INTERLACE_NONE, 
						PNG_COMPRESSION_TYPE_BASE, 
						PNG_FILTER_TYPE_BASE);

	/* Write the file header information.  REQUIRED */
	png_write_info(png_ptr, info_ptr);

	/* One of the following output methods is REQUIRED */
	png_write_image(png_ptr, (unsigned char **) img->data);

	/* It is REQUIRED to call this to finish writing the rest of the file */
	png_write_end(png_ptr, info_ptr);

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* close the file */
	fclose(fp);

	/* that's it */
	return -1;
}

img_color png_get_pixel(struct image *img, n_v2i pos)
{
	assert(pos.x >= 0);
	assert(pos.y >= 0);

	assert(pos.x < img->width);
	assert(pos.y < img->height);

	return img->data[pos.y][pos.x];
}

void png_set_pixel(struct image *img, n_v2i pos, img_color p)
{
	assert(pos.x >= 0);
	assert(pos.y >= 0);

	assert(pos.x < img->width);
	assert(pos.y < img->height);

	img->data[pos.y][pos.x]=p;
}
