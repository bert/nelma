/**
 * @file src/block.h
 * 
 * @brief Mesh blocks, header
 */

#ifndef _BLOCK_H
#define _BLOCK_H

#include "space.h"

/**
 * @brief Fast way of getting or setting the value of a mesh point.
 *
 * @sa blk_n_get()
 *
 * @param _blk_ Pointer to a mesh block.
 * @param _pos_ Position of the mesh point in valid block coordinates.
 */
#define BLK_N(_blk_, _pos_)	((_blk_)->n[blk_off3((_blk_), (_pos_))])

/**
 * @brief Fast way to check if a mesh point is constant.
 *
 * @param _blk_ Pointer to a mesh block.
 * @param _pos_ Position of the mesh point in valid block coordinates.
 */
#define BLK_CON(_blk_, _pos_)	((_blk_)->con[blk_off3((_blk_), (_pos_))])

/**
 * @brief Fast way to get material property.
 *
 * @sa blk_a_get()
 *
 * @param _blk_ Pointer to a mesh block.
 * @param _pos_ Position in valid block coordinates.
 */
#define BLK_A(_blk_, _pos_)	((_blk_)->a[blk_off2((_blk_), (_pos_))])

n_float blk_n_get(struct block *blk, n_v3i pos);
n_float blk_a_get(struct block *blk, n_v3i pos);

size_t blk_off3(struct block *blk, n_v3i pos);
size_t blk_off2(struct block *blk, n_v3i pos);

void blk_init(struct block *blk, struct space *sp, n_v3i pos, n_v3i size);

int blk_convert_heterogeneous(struct block *blk);
int blk_convert_homogeneous(struct block *blk);

int blk_convert_variable(struct block *blk);
int blk_convert_constant(struct block *blk);

void blk_free(struct block *blk);
#endif
