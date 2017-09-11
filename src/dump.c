/**
 * @file src/dump.c
 *
 * @brief , code.
 */

#include <stdio.h>
#include <math.h>

#include "dump.h"
#include "struct.h"
#include "data.h"
#include "assert.h"
#include "space.h"
#include "error.h"

n_float get_field(struct space *sp, n_v3i pos)
{
	n_v3i p1, p2;

	n_float h;
	n_float ex, ey, ez;
	n_float e;

	if(pos.x <= sp->pos.x) return 0.0;
	if(pos.y <= sp->pos.y) return 0.0;
	if(pos.z <= sp->pos.z) return 0.0;
	if(pos.x > sp->pos.x + sp->size.x - 2) return 0.0;
	if(pos.y > sp->pos.y + sp->size.y - 2) return 0.0;
	if(pos.z > sp->pos.z + sp->size.z - 2) return 0.0;

	h=sp->step.x*2;

	p1=v3i_add(pos, v3i_x);
	p2=v3i_sub(pos, v3i_x);
	ex=(sp_n_get(sp, p2) - sp_n_get(sp, p1))/h;

	h=sp->step.y*2;

	p1=v3i_add(pos, v3i_y);
	p2=v3i_sub(pos, v3i_y);
	ey=(sp_n_get(sp, p2) - sp_n_get(sp, p1))/h;

	h=sp->step.z*2;

	p1=v3i_add(pos, v3i_z);
	p2=v3i_sub(pos, v3i_z);
	ez=(sp_n_get(sp, p2) - sp_n_get(sp, p1))/h;

	e=sqrt(ex * ex + ey * ey + ez * ez);

	return e;
}

int map_dump(char *map, n_v2i size, char *file)
{
	FILE *f;

	n_v2i pos;
	int n;

	f=fopen(file, "w");
	if(f==NULL) return -1;

	for(pos.y=0;pos.y<size.y;pos.y++) {
		for(pos.x=0;pos.x<size.x;pos.x++) {
			n=map_get(map, pos, size);
			fprintf(f, "%d %d %d\n", pos.x, pos.y, n);
		}
		fprintf(f, "\n");
	}

	fclose(f);

	return 0;
}

int face_dump(struct face *list, char *file)
{
	FILE *f;

	f=fopen(file, "w");
	if(f==NULL) return -1;

	while(list!=NULL) {
		fprintf(f, "pos=(%d,%d,%d) ", list->pos.x, list->pos.y, list->pos.z);
		fprintf(f, "e1=(%d,%d,%d) ", list->e1.x, list->e1.y, list->e1.z);
		fprintf(f, "e2=(%d,%d,%d) ", list->e2.x, list->e2.y, list->e2.z);
		fprintf(f, "n=(%d,%d,%d)\n", list->n.x, list->n.y, list->n.z);
		list=list->next;
	}

	fclose(f);

	return 0;
}
/*
int lay_dump(struct space *sp, struct layer *lay, char *file)
{
	FILE *f;

	n_v2i pos;
	struct material *mat;

	f=fopen(file, "w");
	if(f==NULL) return -1;

	for(pos.y=0;pos.y<sp->size.y;pos.y++) {
		for(pos.x=0;pos.x<sp->size.x;pos.x++) {
			mat=lay_get_mat(lay, pos);
			fprintf(f, "%d %d %e %e %e\n", pos.x, pos.y, 
							mat->e,
							mat->g,
							mat->u);
		}
		fprintf(f, "\n");
	}

	fclose(f);

	return 0;
}
*/

int sp_dump(struct space *sp, n_axis projection, n_v3i pos, char *file)
{
	FILE *f;
	n_v3i abspos;

	f=fopen(file, "w");
	if(f==NULL) return -1;

	debug("sp_dump: dumping to '%s' at (%d, %d, %d)", file, 
							pos.x, pos.y, pos.z);

	fprintf(f, "# Nelma data dump\n");
	fprintf(f, "# Cross-section at (%d, %d, %d)", pos.x, pos.y, pos.z);
	fprintf(f, "# \n");

	pos=v3i_sub(pos, sp->pos);

	switch(projection) {
		case X:
			fprintf(f, "# y z field potential material constant\n");
			for(pos.z=0;pos.z<sp->size.z;pos.z++) {
				for(pos.y=0;pos.y<sp->size.y;pos.y++) {
					abspos=v3i_add(pos, sp->pos);
					fprintf(f, "%d %d %e %e %e %d\n", 
						abspos.y, abspos.z, 
						get_field(sp, abspos),
						sp_n_get(sp, abspos), 
						sp_a_get(sp, abspos),
						sp_con_get(sp, abspos));
				}
				fprintf(f, "\n");
			}
			break;
		case Y:
			fprintf(f, "# x z field potential material constant\n");
			for(pos.z=0;pos.z<sp->size.z;pos.z++) {
				for(pos.x=0;pos.x<sp->size.x;pos.x++) {
					abspos=v3i_add(pos, sp->pos);
					fprintf(f, "%d %d %e %e %e %d\n", 
						abspos.x, abspos.z, 
						get_field(sp, abspos),
						sp_n_get(sp, abspos), 
						sp_a_get(sp, abspos),
						sp_con_get(sp, abspos));
				}
				fprintf(f, "\n");
			}
			break;
		case Z:
			fprintf(f, "# x y field potential material constant\n");
			for(pos.y=0;pos.y<sp->size.y;pos.y++) {
				for(pos.x=0;pos.x<sp->size.x;pos.x++) {
					abspos=v3i_add(pos, sp->pos);
					fprintf(f, "%d %d %e %e %e %d\n", 
						abspos.x, abspos.y, 
						get_field(sp, abspos),
						sp_n_get(sp, abspos), 
						sp_a_get(sp, abspos),
						sp_con_get(sp, abspos));
				}
				fprintf(f, "\n");
			}
			break;
	}

	fclose(f);

	return 0;
}
