#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "assert.h"
#include "struct.h"
#include "data.h"
#include "error.h"
#include "space.h"
#include "malloc.h"

static struct block *sp_block_find_cache=NULL;

/** @brief Get pointer to a mesh block.
 *
 * Helper function for sp_alloc_blocks(). Should not be called from anywhere
 * else. 
 *
 * @param sp Pointer to the grid structure.
 * @param pos Index of the desired mesh block.
 * @param size Size of the three-dimensional array of mesh blocks. */
static struct block *sp_block(struct space *sp, n_v3i pos, n_v3i size)
{
	size_t off;

	assert(sp!=NULL);

	assert(pos.x >= 0);
	assert(pos.y >= 0);
	assert(pos.z >= 0);
	assert(pos.x < size.x);
	assert(pos.y < size.y);
	assert(pos.z < size.z);

	off=((pos.z * size.y) + pos.y) * size.x + pos.x;

	return(&sp->blk[off]);
}

/** @brief Allocates memory for all mesh blocks and sets some default
 * values.
 *
 * @param sp Pointer to the grid structure.
 * @return 0 on success and -1 on memory allocation error. */
static int sp_alloc_blocks(struct space *sp)
{
	n_v3i size;
	n_v3i pos;

	n_v3i blkpos;
	n_v3i blksize;

	size_t memsize;
	struct block *cur;

	assert(sp!=NULL);

	/* how many blocks we need in horizontal directions? */

	size.x=sp->size.x/ALLOC_BLOCK_SIZE;
	if(sp->size.x%ALLOC_BLOCK_SIZE>0) size.x++;

	size.y=sp->size.y/ALLOC_BLOCK_SIZE;
	if(sp->size.y%ALLOC_BLOCK_SIZE>0) size.y++;

	/* vertically we divide the grid into layers. */

	size.z=sp->laynum;

	memsize=size.x * size.y * size.z;

	sp->blk=n_calloc(memsize, sizeof(*sp->blk));
	if(sp->blk==NULL) return -1;

	for(pos.z=0;pos.z<size.z;pos.z++) {

		blksize.x=ALLOC_BLOCK_SIZE;
		blksize.y=ALLOC_BLOCK_SIZE;
		blksize.z=sp->lay[pos.z]->height;

		blkpos.z=sp->lay[pos.z]->z;

		for(pos.y=0;pos.y<size.y;pos.y++) {
			for(pos.x=0;pos.x<size.x;pos.x++) {

				blkpos.x=sp->pos.x+ALLOC_BLOCK_SIZE*pos.x;
				blkpos.y=sp->pos.y+ALLOC_BLOCK_SIZE*pos.y;

				cur=sp_block(sp, pos, size);

				blk_init(cur, sp, blkpos, blksize);

				if(pos.x>0) {
					cur->xprev=sp_block(sp, 
							v3i_sub(pos, v3i_x), 
							size);
				}
				if(pos.x<size.x-1) {
					cur->xnext=sp_block(sp, 
							v3i_add(pos, v3i_x), 
							size);
				}
				if(pos.y>0) {
					cur->yprev=sp_block(sp, 
							v3i_sub(pos, v3i_y), 
							size);
				}
				if(pos.y<size.y-1) {
					cur->ynext=sp_block(sp, 
							v3i_add(pos, v3i_y), 
							size);
				}
				if(pos.z>0) {
					cur->zprev=sp_block(sp, 
							v3i_sub(pos, v3i_z), 
							size);
				}
				if(pos.z<size.z-1) {
					cur->znext=sp_block(sp, 
							v3i_add(pos, v3i_z), 
							size);
				}
			}
		}
	}

	return 0;
}

/** @brief Free all memory allocated for mesh blocks.
 *
 * @param sp Pointer to the grid structure. */
static void sp_free_blocks(struct space *sp)
{
	struct block *cur, *ynext, *znext;

	assert(sp!=NULL);
	assert(sp->blk!=NULL);

	cur=sp->blk;

	while(cur!=NULL) {
		znext=cur->znext;
		while(cur!=NULL) {
			ynext=cur->ynext;
			while(cur!=NULL) {
				blk_free(cur);

				cur=cur->xnext;
			}
			cur=ynext;
		}
		cur=znext;
	}

	sp_block_find_cache=NULL;

	n_free(sp->blk);
	sp->blk=NULL;
}

/** @brief Checks if a position falls within the allocated part of the 
 * mesh
 *
 * @param sp Pointer to the space struct.
 * @param pos Position to be checked in absolute coordinates.
 * @return 1 if position falls inside the allocated part or 0 if not. */
int sp_pos_inside(struct space *sp, n_v3i pos)
{
	if(pos.x < sp->pos.x) return 0;
	if(pos.y < sp->pos.y) return 0;
	if(pos.z < sp->pos.z) return 0;

	if(pos.x >= sp->pos.x+sp->size.x) return 0;
	if(pos.y >= sp->pos.y+sp->size.y) return 0;
	if(pos.z >= sp->pos.z+sp->size.z) return 0;

	return 1;
}

/** @brief Finds a mesh block that holds information about certain position.
 *
 * @param sp Pointer to the space struct.
 * @param pos Position in absolute coordinates.
 * @return Pointer to the mesh block or NULL if position is outside of 
 * allocated part of the mesh. */
static struct block *sp_block_find(struct space *sp, n_v3i pos)
{
	struct block *cur, *cache;

	assert(sp!=NULL);
	assert(sp->blk!=NULL);

	cache=sp_block_find_cache;

	if(cache!=NULL) {
		if((cache->pos.z <= pos.z)
				&&(cache->pos.z + cache->size.z > pos.z)
				&&(cache->pos.y <= pos.y)
				&&(cache->pos.y + cache->size.y > pos.y)
				&&(cache->pos.x <= pos.x)
				&&(cache->pos.x + cache->size.x > pos.x)) {
			return cache;
		}
	}

	cur=sp->blk;

	while(cur!=NULL) {
		if((cur->pos.z <= pos.z)&&(cur->pos.z + cur->size.z > pos.z)) {
			break;
		}
		cur=cur->znext;
	}
	while(cur!=NULL) {
		if((cur->pos.y <= pos.y)&&(cur->pos.y + cur->size.y > pos.y)) {
			break;
		}
		cur=cur->ynext;
	}
	while(cur!=NULL) {
		if((cur->pos.x <= pos.x)&&(cur->pos.x + cur->size.x > pos.x)) {
			break;
		}
		cur=cur->xnext;
	}

	sp_block_find_cache=cur;

	return cur;
}

int sp_con_get(struct space *sp, n_v3i pos)
{
	size_t off;
	struct block *blk;
	n_v3i blkpos;

	assert(sp!=NULL);

	blk=sp_block_find(sp, pos);

	assert(blk!=NULL);

	if(blk->n==NULL) {
		return 1;
	} else {
		assert(blk->con != NULL);

		blkpos=v3i_sub(pos, blk->pos);
		off=blk_off3(blk, blkpos);
		return blk->con[off];
	}
}

/** @brief Get value of scalar field at mesh point at \a pos coordinates.
 *
 * @param sp Pointer to the grid structure.
 * @param pos Position of the desired point in absolute coordinates
 * @return Value of the field */
n_float sp_n_get(struct space *sp, n_v3i pos)
{
	size_t off;
	struct block *blk;
	n_v3i blkpos;

	assert(sp!=NULL);

	blk=sp_block_find(sp, pos);

	assert(blk!=NULL);

	if(blk->n==NULL) {
		return blk->c_n;
	} else {
		blkpos=v3i_sub(pos, blk->pos);
		off=blk_off3(blk, blkpos);
		return blk->n[off];
	}
}

/** @brief Get value of material propert at \a pos coordinates.
 *
 * @param sp Pointer to the grid structure.
 * @param pos Position of the desired point in absolute coordinates
 * @return Value of the material property */
n_float sp_a_get(struct space *sp, n_v3i pos)
{
	size_t off;
	struct block *blk;
	n_v3i blkpos;

	assert(sp!=NULL);

	blk=sp_block_find(sp, pos);

	assert(blk!=NULL);

	if(blk->a==NULL) {
		return blk->c_a;
	} else {
		blkpos=v3i_sub(pos, blk->pos);
		off=blk_off2(blk, blkpos);
		return blk->a[off];
	}
}

/** @brief Set value of material propert at \a pos coordinates.
 *
 * @param sp Pointer to the grid structure.
 * @param pos Position of the desired point in absolute coordinates
 * @param a Value of the material property */
void sp_a_set(struct space *sp, n_v3i pos, n_float a)
{
	size_t off;
	struct block *blk;
	n_v3i blkpos;

	assert(sp!=NULL);

	blk=sp_block_find(sp, pos);

	assert(blk!=NULL);

	if(blk->a==NULL) {
		if(blk->c_a==a) {
			return;
		}
		blk_convert_heterogeneous(blk);
	}

	blkpos=v3i_sub(pos, blk->pos);
	off=blk_off2(blk, blkpos);
	blk->a[off]=a;
}

/** @brief Set value of scalar field at mesh point at \a pos coordinates and
 * set that mesh point as variable.
 *
 * @param sp Pointer to the grid structure.
 * @param pos Position of the desired point in absolute coordinates
 * @param n Value of the field */
static void sp_n_set_var(struct space *sp, n_v3i pos, n_float n)
{
	size_t off;
	struct block *blk;
	n_v3i blkpos;

	assert(sp!=NULL);

	blk=sp_block_find(sp, pos);

	assert(blk!=NULL);

	if(blk->n==NULL) {
		blk_convert_variable(blk);
	}

	blkpos=v3i_sub(pos, blk->pos);

	off=blk_off3(blk, blkpos);
	blk->n[off]=n;
	blk->con[off]=0;
}

/** @brief Set value of scalar field at mesh point at \a pos coordinates and
 * set that mesh point as constant.
 *
 * @param sp Pointer to the grid structure.
 * @param pos Position of the desired point in absolute coordinates
 * @param n Value of the field */
static void sp_n_set_con(struct space *sp, n_v3i pos, n_float n)
{
	size_t off;
	struct block *blk;
	n_v3i blkpos;

	assert(sp!=NULL);

	blk=sp_block_find(sp, pos);

	if(blk==NULL) return;

	assert(blk!=NULL);

	if(blk->n==NULL) {
		if(blk->c_n==n) {
			return;
		} else {
			blk_convert_variable(blk);
		}
	}

	blkpos=v3i_sub(pos, blk->pos);

	off=blk_off3(blk, blkpos);
	blk->n[off]=n;
	blk->con[off]=1;
}

/** @brief Set value of scalar field at mesh point at \a pos coordinates and
 * set that mesh point as constant or variable.
 *
 * @param sp Pointer to the grid structure.
 * @param pos Position of the desired point in absolute coordinates
 * @param n Value of the field
 * @param con 1 if this is a constant mesh point or 0 if this is a variable
 * mesh point. */
void sp_n_set(struct space *sp, n_v3i pos, n_float n, int con)
{
	if(con) {
		sp_n_set_con(sp, pos, n);
	} else {
		sp_n_set_var(sp, pos, n);
	}
}

struct space *sp_init(n_v3f step)
{
	struct space *sp;

	sp=n_calloc(1, sizeof(*sp));
	if(sp==NULL) return NULL;

	sp->blk=NULL;

	sp->lay=NULL;
	sp->laynum=0;

	sp->size=v3i(-1, -1, -1);

	sp->pos=v3i(-1, -1, -1);

	sp->step=step;

	sp->name=NULL;
	sp->next=NULL;

	return sp;
}

void sp_done(struct space *sp)
{
	assert(sp!=NULL);

	if(sp->blk!=NULL) sp_unload(sp);

	if(sp->name!=NULL) free(sp->name);
	n_free(sp);

	return;
}

void sp_lay_load(struct space *sp, struct layer *lay)
{
	struct block *cur, *ynext;

	assert(sp!=NULL);
	assert(sp->blk!=NULL);
	assert(sp->lay!=NULL);

	cur=sp->blk;

	while(cur!=NULL) {
		if(cur->pos.z == lay->z) {
			break;
		}
		cur=cur->znext;
	}

	assert(cur!=NULL);

	while(cur!=NULL) {
		ynext=cur->ynext;
		while(cur!=NULL) {
			assert(cur->a == NULL);

			cur->c_a = lay->mat->e;

			cur=cur->xnext;
		}
		cur=ynext;
	}
}

int sp_load(struct space *sp, n_v2i pos, n_v2i size)
{
	int n;

	assert(sp!=NULL);
	assert(size.x>0);
	assert(size.y>0);

	if(sp->blk!=NULL) sp_unload(sp);

	sp->pos=v3i_ez(pos, 0);

	sp->size.x=size.x;
	sp->size.y=size.y;

	if(sp_alloc_blocks(sp)) return -1;

	for(n=0;n<sp->laynum;n++) {
		sp_lay_load(sp, sp->lay[n]);
	}

	return 0;
}

void sp_unload(struct space *sp)
{
	if(sp->blk==NULL) return;

	sp_free_blocks(sp);

	/* fixme: unload objects */
}

void sp_border(struct space *sp)
{
	n_v3i pos, abspos;

	for(pos.x=0;pos.x<sp->size.x;pos.x++) {
		for(pos.y=0;pos.y<sp->size.y;pos.y++) {
			pos.z=0;
			abspos=v3i_add(pos, sp->pos);
			sp_n_set_con(sp, abspos, 0.0);

			pos.z=sp->size.z-1;
			abspos=v3i_add(pos, sp->pos);
			sp_n_set_con(sp, abspos, 0.0);
		}
	}
	for(pos.x=0;pos.x<sp->size.x;pos.x++) {
		for(pos.z=0;pos.z<sp->size.z;pos.z++) {
			pos.y=0;
			abspos=v3i_add(pos, sp->pos);
			sp_n_set_con(sp, abspos, 0.0);

			pos.y=sp->size.y-1;
			abspos=v3i_add(pos, sp->pos);
			sp_n_set_con(sp, abspos, 0.0);
		}
	}
	for(pos.y=0;pos.y<sp->size.y;pos.y++) {
		for(pos.z=0;pos.z<sp->size.z;pos.z++) {
			pos.x=0;
			abspos=v3i_add(pos, sp->pos);
			sp_n_set_con(sp, abspos, 0.0);

			pos.x=sp->size.x-1;
			abspos=v3i_add(pos, sp->pos);
			sp_n_set_con(sp, abspos, 0.0);
		}
	}
}

/** @brief Optimize allocated mesh blocks.
 *
 * This function should be called before starting the SOR iteration.
 *
 * It checkes all allocated mesh blocks and converts as many of them to 
 * constant and/or homogeneous blocks as possible.
 *
 * @param sp Pointer to the grid structure. */
void sp_optimize(struct space *sp)
{
	unsigned long int alloc=0, alloc_changed=0;
	unsigned long int homo=0, homo_changed=0;
	unsigned long int all=0;

	struct block *cur, *ynext, *znext;

	assert(sp!=NULL);
	assert(sp->blk!=NULL);

	cur=sp->blk;

	info("Optimizing finite difference mesh...");

	while(cur!=NULL) {
		znext=cur->znext;
		while(cur!=NULL) {
			ynext=cur->ynext;
			while(cur!=NULL) {

				if(blk_convert_constant(cur)) {
					alloc_changed++;
				}

				if(blk_convert_homogeneous(cur)) {
					homo_changed++;
				}

				if(cur->n != NULL) alloc++;
				if(cur->a == NULL) homo++;

				all++;

				cur=cur->xnext;
			}
			cur=ynext;
		}
		cur=znext;
	}

	info("Allocated %d blocks of total %d (%.1f%%)", alloc, all, 
							100.0 * alloc / all);
	info("Optimization freed %d blocks", alloc_changed);

	info("Homogeneous %d blocks of total %d (%.1f%%)", homo, all, 
							100.0 * homo / all);
	info("Optimization converted %d blocks to homogeneous", homo_changed);
}
