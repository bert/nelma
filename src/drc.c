#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>

#include "assert.h"
#include "error.h"
#include "pngutil.h"
#include "num.h"
#include "config.h"

char *a_maskfile=NULL;

int a_minclear=2;
img_color a_bgcolor=0;
int a_minstandoff=INT_MAX;

/** @brief Recursive line flood-fill algorithm. */
static void drc_img_color_line(struct image *src_img, struct image *dst_img, 
						n_v2i pos, n_v2i opos, 
						img_color color)
{
	img_color op, p;
	n_v2i n;
	n_int w, e;

	op=png_get_pixel(src_img, opos);
	p=png_get_pixel(src_img, pos);

	if(op!=p) return;
	if(png_get_pixel(dst_img, pos)==color) return;

	png_set_pixel(dst_img, pos, color);

	n.y=pos.y;
	for(n.x=pos.x+1;n.x<src_img->width;n.x++) {
		p=png_get_pixel(src_img, n);
		if(op!=p) break;
		if(png_get_pixel(dst_img, n)==color) break;

		png_set_pixel(dst_img, n, color);
	}

	e=n.x;

	for(n.x=pos.x-1;n.x>=0;n.x--) {
		p=png_get_pixel(src_img, n);
		if(op!=p) break;
		if(png_get_pixel(dst_img, n)==color) break;

		png_set_pixel(dst_img, n, color);
	}

	w=n.x;

	for(n.x=w+1;n.x<e;n.x++) {
		n.y=pos.y+1;
		if(n.y<src_img->height) {
			drc_img_color_line(src_img, dst_img, n, opos, color);
		}
		n.y=pos.y-1;
		if(n.y>=0) {
			drc_img_color_line(src_img, dst_img, n, opos, color);
		}
	}
}

static struct image *drc_color_nets(struct image *mask)
{
	struct image *dest;
	img_color curcolor;
	n_v2i n;

	dest=png_alloc(mask->width, mask->height);

	curcolor=1;

	for(n.y=0;n.y<dest->height;n.y++) {
		for(n.x=0;n.x<dest->width;n.x++) {
			if(png_get_pixel(mask, n)==0 && png_get_pixel(dest, n)==a_bgcolor) {
				drc_img_color_line(mask, dest, n, n, curcolor);

				if(curcolor==IMG_COLOR_MAX) {
					error("Max nets reached");
					exit(1);
				} 
				curcolor++;
			}
		}
	}

	return(dest);
}

static int drc_check_line(struct image *mask, n_v2i start, n_v2i step) {
	n_v2i pos;

	img_color color2, color1, color3;

	int length = -1;
	int errors = 0;

	pos=start;

	color2=-1;
	color3=-1;

	while( pos.x >= 0 && pos.y >= 0 && 
				pos.x < mask->width && pos.y < mask->height ) {

		color1=png_get_pixel(mask, pos);

		if(color2!=color1) {
			if(length > -1 && color2 == a_bgcolor 
					&& color3 != color1) {

				if(length < a_minclear) {
					printf("Image position (%d,%d): "
						"insufficient clearance "
						"between copper areas\n", 
								pos.x,
								pos.y);

					debug("%d - %d", color3, color1);
	
					errors++;
				}

				if(length < a_minstandoff) {
					a_minstandoff = length;
				}
			}
			length=0;
			color3=color2;
			color2=color1;
		}

		length++;

		pos.x+=step.x;
		pos.y+=step.y;
	}

	return errors;
}

static int drc_check_all_lines(struct image *mask)
{
	n_v2i pos;
	int errors=0;

	for(pos.x = 0; pos.x < mask->width; pos.x++) {
		pos.y = 0;
		errors+=drc_check_line(mask, pos, v2i(0, 1));
		errors+=drc_check_line(mask, pos, v2i(1, 1));
		errors+=drc_check_line(mask, pos, v2i(-1, 1));
	}
	for(pos.y = 0; pos.y < mask->height; pos.y++) {
		pos.x = 0;
		errors+=drc_check_line(mask, pos, v2i(1, 0));
		errors+=drc_check_line(mask, pos, v2i(1, 1));
		pos.x = mask->width - 1;
		errors+=drc_check_line(mask, pos, v2i(-1, 1));
	}

	return errors;
}

int drc_main(char *maskfile)
{
	struct image *mask, *copy;

	if(png_read(&mask, maskfile)) {
		return -1;
	}

	copy=drc_color_nets(mask);

	printf("\nFound %d DRC errors\n", drc_check_all_lines(copy));

	printf("Minimum stand-off (-s option) value for nelma-cap: %d\n", 
							a_minstandoff);

	png_done(copy);
	png_done(mask);
	
	return 0;
}

void main_syntax()
{
	printf("NELMA, Numerical ElectroMagnetics. DRC checker version %s\n", 
								VERSION);
	printf("Copyright (c) 2006 Tomaz Solc\n\n");

	printf("SYNTAX: nelma-drc layer.png\n\n");

	printf("Bug reports to <tomaz.solc@tablix.org>\n");

	return;
}

void main_header(int argc, char **argv)
{
	int n;
	time_t t;

	t=time(NULL);

	printf("* Created by NELMA DRC checker calculator, version %s\n", 
								VERSION);
	printf("* %s", ctime(&t));
	printf("*\n");
	printf("* Command line:");

	for(n=0;n<argc;n++) {
		printf(" %s", argv[n]);
	}
	printf("\n\n");
}

void main_args(int argc, char **argv)
{
	int c;

        while ((c=getopt(argc, argv, "h"))!=-1) {
                switch (c) {
			case 'h': main_syntax();
				  return;
			default:
				  error("Wrong command line arguments (try -h)\n");
				  return;
                }
        }

	if(!(optind<argc)) {
		error("Missing mask file (try -h)");
		return;
	}

	a_maskfile=argv[optind];
}

int main(int argc, char **argv) 
{
	int r;

	main_args(argc, argv);

	if(a_maskfile==NULL) return 1;

	main_header(argc, argv);

	r=drc_main(a_maskfile);
	
	if(r) {
		error("Can't load mask file %s", a_maskfile);
		return 1;
	}

	return 0;
}
