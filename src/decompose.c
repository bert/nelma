#include <stdio.h>
#include <stdlib.h>

#include "pngutil.h"
#include "assert.h"

#include "listutil.h"

int nb=0;
int nlh=0,nlv=0;

struct line {
	int x1,x2,y1,y2;

	struct line *next;
};

struct box {
	int x1,x2,y1,y2;

	struct box *next;
};

/* horizontal lines */
LIST_NEW_H(lh, line)
LIST_NEW_C(lh, line)

/* vertical lines */
LIST_NEW_H(lv, line)
LIST_NEW_C(lv, line)

/* boxes */
LIST_NEW_H(box, box);
LIST_NEW_C(box, box);

int a_clear=15;
int a_ras=10;

#define MIN(a,b)	((a)<(b)?(a):(b))
#define MAX(a,b)	((a)>(b)?(a):(b))

/* only works for horizontal and vertical lines */
int cross(struct line *l1, struct line *l2) 
{
	struct line *t;

	assert(l1->x1<=l1->x2);
	assert(l1->y1<=l1->y2);
	assert(l2->x1<=l2->x2);
	assert(l2->y1<=l2->y2);

	if( l1->x1 != l1->x2 ) {
		/* first line isn't vertical */

		if( l2->x1 != l2->x2 ) {
			/* second line isn't vertical */
			/* two horizontal lines = bad */
			return 0;
		} 

		t=l1;
		l1=l2;
		l2=t;
	}

	if( l2->x1 == l2->x2 ) {
		/* two vertical lines = bad */
		return 0;
	}

	if( l2->y1 == l2->y2) {
		/* vertical and horiz line */
		if(( l2->y1 < l1->y1 )||( l2->y1 > l1->y2 )) {
			/* goes over the top or bottom */
			return 0;	
		}
		if((l2->x1>=l1->x1)&&(l2->x2<=l1->x1)) {
			return 1;
		} 
		if((l2->x1<=l1->x1)&&(l2->x2>=l1->x1)) {
			return 1;
		} 
	} 

	/* should never happen */
	assert(0);
	return 0;
}

void add_box(struct line *lh1, struct line *lh2, 
					struct line *lv1, struct line *lv2)
{
	struct box *b;

	if(!cross(lh1, lv1)) return;
	if(!cross(lh1, lv2)) return;
	if(!cross(lh2, lv1)) return;
	if(!cross(lh2, lv2)) return;

	b=calloc(1, sizeof(*b));

	b->x1=MIN(lv1->x1, lv2->x1);
	b->x2=MAX(lv1->x1, lv2->x1);
	b->y1=MIN(lh1->y1, lh2->y1);
	b->y2=MAX(lh1->y1, lh2->y1);

	if(abs(b->x1-b->x2)<2*a_clear) {
		free(b);
		return;
	}
	if(abs(b->y1-b->y2)<2*a_clear) {
		free(b);
		return;
	}

	box_prepend(b);
	nb++;
	if(nb%10000==0) printf("%d\n", nb);
}

void boxify()
{
	struct line *lh1, *lh2, *lv1, *lv2;

	lh1=lh_list;
	while(lh1!=NULL) {
		lv1=lv_list;
		while(lv1!=NULL) {
			lh2=lh1;
			if(cross(lh1,lv1)) while(lh2!=NULL) {
				lv2=lv1;
				if(cross(lh2, lv1)) while(lv2!=NULL) {
					add_box(lh1, lh2, lv1, lv2);
					lv2=lv2->next;
				}
				lh2=lh2->next;
			}
			lv1=lv1->next;
		}
		lh1=lh1->next;
	}
}

void scanline_v(int *num, struct image *img, int x) {
	int n;

	assert(x>0);
	assert(x<img->width);

	for(n=0;n<img->height;n++) {
		if(img->data[n][x-1]==img->data[n][x]) {
			num[n]++;
		} else {
			num[n]=0;
		}
	}
}

int scan_v(struct image *img) {
	int *num;
	int n;
	int m;
	int c;

	struct line *l;

	num=calloc(img->height, sizeof(*num));

	for(n=1;n<img->width;n++) {
		scanline_v(num, img, n);

		if(n%a_ras!=0) continue;

		c=-1;
		for(m=0;m<img->height;m++) {
			if(c<0) {
				if(num[m]>=a_clear*2) {
					c=m;
				}
			} else {
				if(num[m]<a_clear*2) {
					if(m-c>a_clear*2) {
						l=calloc(1, sizeof(*l));
						l->y1=c;
						l->y2=m-1;
						l->x1=n-a_clear;
						l->x2=n-a_clear;
						lv_prepend(l);
						nlv++;
					}
					c=-1;
				}
			}
		}
		if((c>=0)&&(m-c>a_clear*2)) {
			l=calloc(1, sizeof(*l));
			l->y1=c;
			l->y2=m-1;
			l->x1=n-a_clear;
			l->x2=n-a_clear;
			lv_prepend(l);
			nlv++;
		}
	}

	free(num);

	return 0;
}

void scanline_h(int *num, struct image *img, int y) {
	int n;

	assert(y>0);
	assert(y<img->height);

	for(n=0;n<img->width;n++) {
		if(img->data[y-1][n]==img->data[y][n]) {
			num[n]++;
		} else {
			num[n]=0;
		}
	}
}

int scan_h(struct image *img) {
	int *num;
	int n;
	int m;
	int c;

	struct line *l;

	num=calloc(img->width, sizeof(*num));

	for(n=1;n<img->height;n++) {
		scanline_h(num, img, n);

		if(n%a_ras!=0) continue;

		c=-1;
		for(m=0;m<img->width;m++) {
			if(c<0) {
				if(num[m]>=a_clear*2) {
					c=m;
				}
			} else {
				if(num[m]<a_clear*2) {
					if(m-c>a_clear*2) {
						l=calloc(1, sizeof(*l));
						l->x1=c;
						l->x2=m-1;
						l->y1=n-a_clear;
						l->y2=n-a_clear;
						lh_prepend(l);
						nlh++;
					}
					c=-1;
				}
			}
		}
		if((c>=0)&&(m-c>a_clear*2)) {
			l=calloc(1, sizeof(*l));
			l->x1=c;
			l->x2=m-1;
			l->y1=n-a_clear;
			l->y2=n-a_clear;
			lh_prepend(l);
			nlh++;
		}
	}

	free(num);

	return 0;
}

int main(int argc, char **argv) {
	struct image *img;

	struct line *l;
	struct box *b;
	int n,m,a;

	png_read(&img, "test.png");

	a=0;
	for(n=0;n<img->height;n++) {
		for(m=0;m<img->width;m++) {
			if(img->data[n][m]!=0) a++;
		}
	}

	printf("%d\n", a);

//	exit(1);

	scan_h(img);
	scan_v(img);

	printf("%d %d\n", nlh, nlv);

	boxify();

	printf("Painting...\n");

	l=lh_list;
	while(l!=NULL) {
		// printf("%d,%d - %d,%d\n", l->x1, l->y1, l->x2, l->y2);

		for(n=l->x1;n<=l->x2;n++) {
			img->data[l->y1][n]=0;
		}
		l=l->next;
	}

	l=lv_list;
	while(l!=NULL) {
		// printf("%d,%d - %d,%d\n", l->x1, l->y1, l->x2, l->y2);
		for(n=l->y1;n<=l->y2;n++) {
			img->data[n][l->x1]=0;
		}
		l=l->next;
	}

	b=box_list;
	while(b!=NULL) {
		for(n=b->y1;n<=b->y2;n++) {
			for(m=b->x1;m<=b->x2;m++) {
				img->data[n][m]=0;
			}
		}
		b=b->next;
	}

	a=0;
	for(n=0;n<img->height;n++) {
		for(m=0;m<img->width;m++) {
			if(img->data[n][m]!=0) a++;
		}
	}

	printf("%d\n", a);

	png_write(img, "test2.png");

	return 0;
}
