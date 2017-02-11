#include "assert.h"
#include "block.h"
#include "malloc.h"

/** @file 
 * @brief Mesh blocks, header */

/** @brief Set some default values for the mesh block structure.
 *
 * Defaults are: constant block, homogeneous block, 0.0 value, 
 * 0.0 material property.
 *
 * By default mesh blocks are set to consume minimal amount of memory.
 *
 * @param blk Pointer to the block structure to be initialized.
 * @param sp Pointer to the parent grid structure.
 * @param pos Position of the block in space coordinates.
 * @param size Size of the block.
 */
void blk_init(struct block *blk, struct space *sp, n_v3i pos, n_v3i size)
{
	assert(blk!=NULL);
	assert(sp!=NULL);

	blk->pos=pos;
	blk->size=size;

	blk->n=NULL;
	blk->a=NULL;
	blk->con=NULL;

	blk->c_n=0.0;
	blk->c_a=0.0;

	blk->xnext=NULL;
	blk->xprev=NULL;
	blk->ynext=NULL;
	blk->yprev=NULL;
	blk->znext=NULL;
	blk->zprev=NULL;

	blk->sp=sp;
}

/** @brief Converts a mesh block from homogeneous to heterogeneous. 
 *
 * @param blk Pointer to the mesh block to be converted
 * @return 0 on success and -1 on error. */
int blk_convert_heterogeneous(struct block *blk)
{
	size_t memsize, n;

	assert(blk->a==NULL);

	memsize=blk->size.x * blk->size.y;
	blk->a=n_calloc(memsize, sizeof(*blk->a));
	if(blk->a==NULL) return -1;

	for(n=0;n<memsize;n++) {
		blk->a[n]=blk->c_a;
	}

	return 0;
}

/** @brief Converts a mesh block from heterogeneous to homogeneous.
 *
 * A check is performed. If the material property is homogeneous in this block
 * the block is marked as homogeneous and memory allocated for material map
 * is freed.
 *
 * If a block can't be converted this function has no effect.
 *
 * @param blk Pointer to the mesh block to be converted.
 * @return 1 if the block was converted or 0 if not. */
int blk_convert_homogeneous(struct block *blk)
{
	size_t memsize, n;
	n_float a;

	assert(blk!=NULL);

	if(blk->a==NULL) {
		/* block is already homogeneous */
		return 0;
	}

	memsize=blk->size.x * blk->size.y;

	a = blk->a[0];
	for(n=1;n<memsize;n++) {
		if(blk->a[n] != a) {
			/* block contains different materials 
			 * it can't be converted */
			return 0;
		}
	}

	blk->c_a = a;

	n_free(blk->a);
	blk->a = NULL;

	return 1;
}

/** @brief Converts a mesh block from constant to variable.
 *
 * @param blk Pointer to the mesh block to be converted
 * @return 0 on success and -1 on error. */
int blk_convert_variable(struct block *blk)
{
	size_t memsize, n;

	assert(blk!=NULL);
	assert(blk->n==NULL);
	assert(blk->con==NULL);

	memsize=blk->size.x * blk->size.y * blk->size.z;
	blk->n=n_calloc(memsize, sizeof(*blk->n));
	if(blk->n==NULL) return -1;

	for(n=0;n<memsize;n++) {
		blk->n[n]=blk->c_n;
	}

	blk->con=n_calloc(memsize, sizeof(*blk->con));
	if(blk->con==NULL) return -1;

	for(n=0;n<memsize;n++) {
		blk->con[n]=1;
	}

	return 0;
}

/** @brief Converts a mesh block from variable to constant.
 *
 * A check is performed. If all mesh points in this block are constant and 
 * have equal values then this block is marked as constant and memory 
 * allocated for point values is freed.
 *
 * If a block can't be converted this function has no effect.
 *
 * @param blk Pointer to the mesh block to be converted
 * @return 1 if the block was converted or 0 if not. */
int blk_convert_constant(struct block *blk)
{
	size_t memsize, i;
	n_float n;

	assert(blk!=NULL);

	if(blk->n == NULL) {	
		/* block already constant */
		return 0;
	}

	assert(blk->con!=NULL);

	memsize=blk->size.x * blk->size.y * blk->size.z;

	n = blk->n[0];

	for(i=0;i<memsize;i++) {
		if(blk->con[i] != 1) {
			/* not all points are constant in this block */
			return 0;
		}

		if(blk->n[i] != n) {
			/* not all points have equal values */
			return 0;
		}
	}

	blk->c_n = n;

	n_free(blk->n);
	blk->n = NULL;

	return 1;
}

/** @brief Frees any memory allocated for arrays in a mesh block 
 *
 * @param blk Pointer to the mesh block */
void blk_free(struct block *blk) 
{
	if(blk->a!=NULL) {
		n_free(blk->a);
		blk->a=NULL;
	}

	if(blk->n!=NULL) {
		n_free(blk->n);
		blk->n=NULL;
	}

	if(blk->con!=NULL) {
		n_free(blk->con);
		blk->con=NULL;
	}
}

/** @brief Get array index of a mesh point value at position \a pos.
 *
 * To get value of a mesh point, use:
 *
 * <code>
 * blk->n[blk_off3(blk, pos)];
 * </code>
 *
 * To determine whether a mesh point is constant use:
 *
 * <code>
 * blk->con[blk_off3(blk, pos)];
 * </code>
 *
 * @param blk Pointer to the mesh block.
 * @param pos Block coordinates of the desired mesh point.
 */
size_t blk_off3(struct block *blk, n_v3i pos) {
	assert(pos.x >= 0);
	assert(pos.x < blk->size.x);

	assert(pos.y >= 0);
	assert(pos.y < blk->size.y);

	assert(pos.z >= 0);
	assert(pos.z < blk->size.z);

	return ((pos.z * blk->size.y) + pos.y) * blk->size.x + pos.x;
}

/** @brief Get array index of a material property value at position \a pos.
 *
 * To get material property value, use:
 *
 * <code>
 * blk->a[blk_off2(blk, pos)];
 * </code>
 *
 * @param blk Pointer to the mesh block.
 * @param pos Block coordinates of the desired mesh point.
 */
size_t blk_off2(struct block *blk, n_v3i pos) {
	assert(pos.x >= 0);
	assert(pos.x < blk->size.x);

	assert(pos.y >= 0);
	assert(pos.y < blk->size.y);

	return pos.y * blk->size.x + pos.x;
}

/** @brief Given a pointer to a mesh block and position in block coordinates
 * find a block where these block coordinates are valid.
 *
 * Example: if pos.x<0, then this function moves the block pointer to the
 * blk->xprev and updates the pos.x accordingly so that pos.x>=0.
 *
 * @param blk Pointer to a mesh block.
 * @param pos Position in block coordinates.
 */
inline static void blk_norm(struct block **blk, n_v3i *pos)
{
	if((*pos).x < 0) {
		(*blk)=(*blk)->xprev;
		assert((*blk)!=NULL);
		(*pos).x+=(*blk)->size.x;
	} else if((*pos).x >= (*blk)->size.x) {
		(*pos).x-=(*blk)->size.x;
		(*blk)=(*blk)->xnext;
		assert((*blk)!=NULL);
	}

	if((*pos).y < 0) {
		(*blk)=(*blk)->yprev;
		assert((*blk)!=NULL);
		(*pos).y+=(*blk)->size.y;
	} else if((*pos).y >= (*blk)->size.y) {
		(*pos).y-=(*blk)->size.y;
		(*blk)=(*blk)->ynext;
		assert((*blk)!=NULL);
	}

	if((*pos).z < 0) {
		(*blk)=(*blk)->zprev;
		assert((*blk)!=NULL);
		(*pos).z+=(*blk)->size.z;
	} else if((*pos).z >= (*blk)->size.z) {
		(*pos).z-=(*blk)->size.z;
		(*blk)=(*blk)->znext;
		assert((*blk)!=NULL);
	}
}

/** @brief Slow way of getting the value of a mesh point.
 *
 * @sa BLK_N()
 *
 * @param blk Pointer to a mesh block.
 * @param pos Position of the mesh point (can fall outside of the current 
 * block). */
n_float blk_n_get(struct block *blk, n_v3i pos)
{
	assert(blk != NULL);

	blk_norm(&blk, &pos);

	if(blk->n==NULL) {
		return(blk->c_n);
	} else {
		return(BLK_N(blk, pos));
	}
}

/** @brief Slow way of getting a material property.
 *
 * @sa BLK_A()
 *
 * @param blk Pointer to a mesh block.
 * @param pos Position of the mesh point (can fall outside of the current 
 * block). */
n_float blk_a_get(struct block *blk, n_v3i pos)
{
	assert(blk != NULL);

	blk_norm(&blk, &pos);

	if(blk->a==NULL) {
		return(blk->c_a);
	} else {
		return(BLK_A(blk, pos));
	}
}
