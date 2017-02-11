#include "sor.h" 
#include "data.h"
#include "assert.h"
#include "space.h"

/** @file 
 * @brief SOR algorithm, code */

/** @brief SOR exstrapolation (omega) parameter. Must be 0 <= omega <= 2.0. */
n_float a_soromega=1.0;

/** @brief Helper macro for sor_iterate_block_corners() */
#define ITERATE_ONE \
			ex1y1z1=blk_a_get(blk, v3i_sub(pos, v3i(1,1,1)));  \
			ex1y2z1=blk_a_get(blk, v3i_sub(pos, v3i(1,0,1)));  \
			ex2y1z1=blk_a_get(blk, v3i_sub(pos, v3i(0,1,1)));  \
			ex2y2z1=blk_a_get(blk, v3i_sub(pos, v3i(0,0,1)));  \
 \
			ex1y1z2=blk_a_get(blk, v3i_sub(pos, v3i(1,1,0))); \
			ex1y2z2=blk_a_get(blk, v3i_sub(pos, v3i(1,0,0))); \
			ex2y1z2=blk_a_get(blk, v3i_sub(pos, v3i(0,1,0))); \
			ex2y2z2=blk_a_get(blk, v3i_sub(pos, v3i(0,0,0))); \
 \
			kx1=(ex1y1z1+ex1y1z2+ex1y2z1+ex1y2z2)*ax; \
			kx2=(ex2y1z1+ex2y1z2+ex2y2z1+ex2y2z2)*ax; \
 \
			ky1=(ex1y1z1+ex1y1z2+ex2y1z1+ex2y1z2)*ay; \
			ky2=(ex1y2z1+ex1y2z2+ex2y2z1+ex2y2z2)*ay; \
 \
			kz1=(ex1y1z1+ex1y2z1+ex2y1z1+ex2y2z1)*az; \
			kz2=(ex1y1z2+ex1y2z2+ex2y1z2+ex2y2z2)*az; \
 \
			n1= kx1*blk_n_get(blk, v3i_sub(pos, v3i_x)); \
			n1+=kx2*blk_n_get(blk, v3i_add(pos, v3i_x)); \
			n1+=ky1*blk_n_get(blk, v3i_sub(pos, v3i_y)); \
			n1+=ky2*blk_n_get(blk, v3i_add(pos, v3i_y)); \
			n1+=kz1*blk_n_get(blk, v3i_sub(pos, v3i_z)); \
			n1+=kz2*blk_n_get(blk, v3i_add(pos, v3i_z)); \
 \
			n1=n1/(kx1+kx2+ky1+ky2+kz1+kz2); \
 \
			n2=(1.0 - a_soromega) * BLK_N(blk, pos) +  \
				a_soromega * n1; \
 \
			BLK_N(blk, pos) = n2;

/** @brief Performs a single SOR iteration on the edges of a mesh block 
 *
 * @param blk Pointer to the mesh block. */
void static sor_iterate_block_corners(struct block *blk)
{
	n_float n1,n2;
	n_float kx1,kx2,ky1,ky2,kz1,kz2;
	n_float ex1y1z1, ex1y2z1, ex2y1z1, ex2y2z1;
	n_float ex1y1z2, ex1y2z2, ex2y1z2, ex2y2z2;
	n_float ax,ay,az;

	struct space *sp;

	n_v3i pos;

	sp=blk->sp;

	ax=sp->step.z * sp->step.y / 4 / sp->step.x;
	ay=sp->step.z * sp->step.x / 4 / sp->step.y;
	az=sp->step.x * sp->step.y / 4 / sp->step.z;

	/* bottom */

	pos.z = 0;
	for(pos.y = 0; pos.y < blk->size.y; pos.y++) {
		for(pos.x = 0; pos.x < blk->size.x; pos.x++) {

			if(BLK_CON(blk, pos)) continue;

			ITERATE_ONE
		}
	}

	/* top */

	if(blk->size.z > 1) {
		pos.z = blk->size.z-1;
		for(pos.y = 0; pos.y < blk->size.y; pos.y++) {
			for(pos.x = 0; pos.x < blk->size.x; pos.x++) {

				if(BLK_CON(blk, pos)) continue;

				ITERATE_ONE
			}
		}
	}

	/* xprev */

	pos.x = 0;
	for(pos.z = 1; pos.z < blk->size.z - 1; pos.z++) {
		for(pos.y = 0; pos.y < blk->size.y; pos.y++) {

			if(BLK_CON(blk, pos)) continue;
			
			ITERATE_ONE
		}
	}

	/* xnext */

	if(blk->size.x > 1) {
		pos.x = blk->size.x-1;
		for(pos.z = 1; pos.z < blk->size.z - 1; pos.z++) {
			for(pos.y = 0; pos.y < blk->size.y; pos.y++) {

				if(BLK_CON(blk, pos)) continue;

				ITERATE_ONE
			}
		}
	}


	/* yprev */

	pos.y = 0;
	for(pos.z = 1; pos.z < blk->size.z - 1; pos.z++) {
		for(pos.x = 1; pos.x < blk->size.x - 1; pos.x++) {

			if(BLK_CON(blk, pos)) continue;
			
			ITERATE_ONE
		}
	}

	/* ynext */

	if(blk->size.y > 1) {
		pos.y = blk->size.y-1;
		for(pos.z = 1; pos.z < blk->size.z - 1; pos.z++) {
			for(pos.x = 1; pos.x < blk->size.x - 1; pos.x++) {

				if(BLK_CON(blk, pos)) continue;

				ITERATE_ONE
			}
		}
	}
}

/** @brief Performs a single SOR iteration on the center of a heterogeneous 
 * mesh block 
 *
 * @param blk Pointer to the mesh block. */
int static sor_iterate_block_n(struct block *blk)
{
	n_float n1,n2;
	n_float kx1,kx2,ky1,ky2,kz;
	n_float ex1y1, ex1y2, ex2y1, ex2y2;
	n_float ax,ay,az;

	struct space *sp;

	n_v3i pos;

	assert(blk->a!=NULL);

	sp=blk->sp;

	ax=sp->step.z * sp->step.y / 2 / sp->step.x;
	ay=sp->step.z * sp->step.x / 2 / sp->step.y;
	az=sp->step.x * sp->step.y / 4 / sp->step.z;

	for(pos.z = 1; pos.z < blk->size.z - 1; pos.z++) {
		for(pos.y = 1; pos.y < blk->size.y - 1; pos.y++) {
			for(pos.x = 1; pos.x < blk->size.x - 1; pos.x++) {
				if(BLK_CON(blk, pos)) continue;

				ex1y1=BLK_A(blk, v3i_sub(pos, v3i(1,1,0)));
				ex1y2=BLK_A(blk, v3i_sub(pos, v3i(1,0,0)));
				ex2y1=BLK_A(blk, v3i_sub(pos, v3i(0,1,0)));
				ex2y2=BLK_A(blk, v3i_sub(pos, v3i(0,0,0)));

				kz=(ex1y1+ex1y2+ex2y1+ex2y2)*az;

				kx1=(ex1y1+ex1y2)*ax;
				kx2=(ex2y1+ex2y2)*ax;

				ky1=(ex1y1+ex2y1)*ay;
				ky2=(ex1y2+ex2y2)*ay;

				n1=0.0;
				n1+=kx1*BLK_N(blk, v3i_sub(pos, v3i_x));
				n1+=kx2*BLK_N(blk, v3i_add(pos, v3i_x));
				n1+=ky1*BLK_N(blk, v3i_sub(pos, v3i_y));
				n1+=ky2*BLK_N(blk, v3i_add(pos, v3i_y));
				n1+=kz*BLK_N(blk, v3i_sub(pos, v3i_z));
				n1+=kz*BLK_N(blk, v3i_add(pos, v3i_z));

				n1=n1/(kx1+kx2+ky1+ky2+2*kz);

				n2=(1.0 - a_soromega) * BLK_N(blk, pos) + 
							a_soromega * n1;

				BLK_N(blk, pos) = n2;
			}
		}
	}

	return 0;
}

/** @brief Performs a single SOR iteration on the center of a homogeneous 
 * mesh block 
 *
 * @param blk Pointer to the mesh block. */
void static sor_iterate_block_h(struct block *blk)
{
	n_float n1,n2;
	n_float kx,ky,kz;
	n_float bx,by,bz;
	n_float ks;

	struct space *sp;

	n_v3i pos;

	assert(blk->a == NULL);

	sp=blk->sp;

	bx = sp->step.z * sp->step.y / sp->step.x;
	by = sp->step.z * sp->step.x / sp->step.y;
	bz = sp->step.x * sp->step.y / sp->step.z;

	kx = blk->c_a * bx;
	ky = blk->c_a * by;
	kz = blk->c_a * bz;

	ks=1/(2*kx+2*ky+2*kz);

	for(pos.z = 1; pos.z < blk->size.z - 1; pos.z++) {
		for(pos.y = 1; pos.y < blk->size.y - 1; pos.y++) {
			for(pos.x = 1; pos.x < blk->size.x - 1; pos.x++) {

				if(BLK_CON(blk, pos)) continue;

				n1=0;
				n1+=kx*(BLK_N(blk, v3i_sub(pos, v3i_x)) +
					BLK_N(blk, v3i_add(pos, v3i_x)));

				n1+=ky*(BLK_N(blk, v3i_sub(pos, v3i_y)) +
					BLK_N(blk, v3i_add(pos, v3i_y)));

				n1+=kz*(BLK_N(blk, v3i_sub(pos, v3i_z)) +
					BLK_N(blk, v3i_add(pos, v3i_z)));

				n1=n1*ks;

				n2=(1.0 - a_soromega) * BLK_N(blk, pos) + 
							a_soromega * n1;

				
				BLK_N(blk, pos) = n2;
			}
		}
	}
}

/** @brief Performs a single SOR iteration on the center of a homogeneous 
 * mesh block 
 *
 * @param blk Pointer to the mesh block. */
void static sor_iterate_block_h_fast(struct block *blk)
{
	n_float n1,n2;
	n_float kx,ky,kz;
	n_float bx,by,bz;
	n_float ks;

	struct space *sp;

	int stopx;

	n_float *o, *ox1, *ox2, *oy1, *oy2, *oz1, *oz2;
	char *con;

	n_v3i pos;

	assert(blk->a == NULL);

	sp=blk->sp;

	bx = sp->step.z * sp->step.y / sp->step.x;
	by = sp->step.z * sp->step.x / sp->step.y;
	bz = sp->step.x * sp->step.y / sp->step.z;

	kx = blk->c_a * bx;
	ky = blk->c_a * by;
	kz = blk->c_a * bz;

	ks = 1/(2*kx+2*ky+2*kz);

	kx = kx * ks;
	ky = ky * ks;
	kz = kz * ks;

	stopx = blk->size.x - 1;

	for(pos.z = 1; pos.z < blk->size.z - 1; pos.z++) {
		for(pos.y = 1; pos.y < blk->size.y - 1; pos.y++) {
			pos.x = 1;

			o = &BLK_N(blk, pos);

			ox1 = &BLK_N(blk, v3i_sub(pos, v3i_x));
			ox2 = &BLK_N(blk, v3i_add(pos, v3i_x));

			oy1 = &BLK_N(blk, v3i_sub(pos, v3i_y));
			oy2 = &BLK_N(blk, v3i_add(pos, v3i_y));

			oz1 = &BLK_N(blk, v3i_sub(pos, v3i_z));
			oz2 = &BLK_N(blk, v3i_add(pos, v3i_z));

			con = &BLK_CON(blk, pos);

			for(; pos.x < stopx; pos.x++) {

				if(!(*con)) {

					n1=kx * ((*ox1) + (*ox2));

					n1+=ky * ((*oy1) + (*oy2));

					n1+=kz * ((*oz1) + (*oz2));

					n2=(1.0 - a_soromega) * (*o) + 
							a_soromega * n1;

				
					(*o) = n2;
				}

				ox1=o;
				o=ox2;
				ox2++;

				oy1++;
				oy2++;

				oz1++;
				oz2++;

				con++;
			}
		}
	}
}

/** @brief Peforms a single iteration of the SOR algorithm on one mesh block.
 *
 * @param blk Pointer to the mesh block. */
void sor_iterate_block(struct block *blk) 
{
	assert(blk->size.x > 0);
	assert(blk->size.y > 0);
	assert(blk->size.z > 0);

	if(blk->n==NULL) {
		return;
	}

	if(blk->a==NULL) {
		sor_iterate_block_h_fast(blk);
	} else {
		sor_iterate_block_n(blk);
	}

	sor_iterate_block_corners(blk);
}

/** @brief Performs a single iteration of the SOR algorithm on the whole mesh.
 *
 * @param sp Pointer to the space struct. */
void sor_iterate(struct space *sp)
{
	struct block *cur, *ynext, *znext;

	cur=sp->blk;

	while(cur!=NULL) {
		znext=cur->znext;
		while(cur!=NULL) {
			ynext=cur->ynext;
			while(cur!=NULL) {
				sor_iterate_block(cur);

				cur=cur->xnext;
			}
			cur=ynext;
		}
		cur=znext;
	}
}
