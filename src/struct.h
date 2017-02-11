#ifndef _STRUCT_H
#define _STRUCT_H

#include "num.h"

/** @file 
 * @brief Data structures 
 *
 * This file defines all important data structures used throughout the code.
 *
 * Some conventions used in the code and documentation:
 *
 * - Object coordinates: these coordinates are relative to the object's 
 *   position. (0,0) means the point at the object's lower left corner. 
 *
 * - Block coordinates: coordinates relative to the current block of the
 *   finite difference mesh. (0,0,0) is equal to block->pos.
 *
 * - Absolute coordinates: absolute coordinates used in the configuration
 *   file.
 *
 * All coordinates are in mesh units. They can be converted to physical units 
 * (meters) by multiplying with step value in struct \a space.
 *
 * About coordinate systems:
 *
 * - All coordinate systems are right-handed (positive):
 *
 * <pre>
 *
 * z ^   
 *   |  /  y
 *   | /
 *   |/
 *   +------>
 *     
 *          x
 * </pre>
 *
 * A coordinate triplet (x,y,z) can be used to refer to:
 *
 * - A value of a scalar field at a particular location. 
 *   In that case the value of the field is taken from that exact location.
 *
 * - A property of the material or space. In that case the coordinate is 
 *   regarded as the coordinate of a lower left corner of a cube with 1 mesh
 *   unit sides. The material property is homogeneous inside that cube.
 */

/** @brief Material type */
enum material_type {
	/** @brief Good electrical conductor. Electrical field strength is 
	 * zero in materials of this type for electrostatic analisys. */
	metal,

	/** @brief Used for materials with low (often insignificant)
	 * conductivity. Materials of this type have non-zero field strength
	 * in electrostatic analisys. */
	dielectric
};

/** @brief Role of an object in analisys */
enum object_role {
	/** @brief Pins are considered as constant voltage sources in current
	 * field analisys. They are ignored in electrostatic analisys. */
	pin,
	/** @brief Nets are conductors between pins. Their resistances are
	 * calculated in current field analisys and capacitances between them
	 * are calculated in electrostatic analisys. */
	net,
	/** @brief This role is reserved for objects that do not play any
	 * active role in the analisys (parts of the casing, holes, ...) 
	 * They are shorted to ground (0 V). */
	none
};

/** @brief Type of the object */
enum object_type {
	/** @brief Rectangular object */
	rectangle,
	/** @brief Circular object */
	circle,
	/** @brief Object loaded from an image file */
	image
};

/** @brief Structure that describes a material used in the analisys. */
struct material {
	/** @brief Type of the material */
	enum material_type type;

	/** @brief Permittivity in As/Vm */
	n_float e; 

	/** @brief Conductivity in S/m */
	n_float g; 

	/** @brief Permeability in Vs/Am */
	n_float u; 

	/** @brief Name of the material */
	char *name;

	/** @brief Pointer to the next struct in linked list */
	struct material *next;
};

/** @brief One block of mesh points, part of the finite difference grid.
 *
 * Mesh blocks are rectangular parts of the grid. They are stored in a 
 * three-dimensional linked list. 
 *
 * The current allocation functions are pretty simple. They divide the grid
 * horizontally into equal squares (ALLOC_BLOCK_SIZE x ALLOC_BLOCK_SIZE) and 
 * vertically into layers. However the SOR routines should also work correctly 
 * with more general mesh divisions as long as the touching sides of 
 * neighboring blocks are of the same dimensions.
 *
 * The meterial property (alpha) can be either permittivity, conductivity or
 * permeability, depending on the type of calculation. Within a mesh block
 * this property is homogeneous along the z axis.
 *
 * Interpretation of the coordinates in the structure (numbers in parenthesis
 * are examples):
 *
 * <pre> 
 *
 *                 *  *  *  *
 *               XX|..|..|..|..
 *              *--+--+--+--+--*  
 *            ^  XX|..|..|..|..
 *            | *--+--+--+--+--*
 *            |  XX|..|..|..|..
 * size.z (5) | *--+--+--+--+--*
 *            |  XX|..|..|..|..
 *            | *--+--+--+--+--*
 *            V  XX|..|..|..|..
 * pos.z (0)->  *--+--+--+--+--*
 *               XX|XX|XX|XX|XX
 *                 *  *  *  *
 *
 *                  <------>                            
 * ^ Z             ^   size.x (4)     
 * |               |
 * +--> X
 *                 pos.x (5)
 *
 *
 * Key:
 *
 * + Mesh point defined within this block
 * * Mesh point defined in a neighboring block
 *
 * . Material property defined within this block
 * X Material property defined in a neighboring block
 * </pre>
 *
 */
struct block {
	/** @brief Pointer to a three-dimensional array of allocated mesh
	 * points.
	 *
	 * NULL if not allocated (constant block). In that case all mesh
	 * points have value in \a c_n.
	 *
	 * Size: size.x * size.y * size.z */
	n_float *n;

	/** @brief Pointer to a two-dimensional array of the material 
	 * property (permittivity, conductivity or permeability depending
	 * on the type of calculation). Material is always homogeneous 
	 * in the z direction. 
	 *
	 * NULL if not allocated (homogeneous block). In that case material
	 * property is taken from \a c_a
	 *
	 * Size: size.x * size.y */
	n_float *a;

	/** @brief Pointer to a three-dimensional array that tells whether
	 * point at (x,y,z) is constant or variable.
	 *
	 * NULL if not allocated (constant block). In that case all points
	 * in the block are constant.
	 *
	 * Size: size.x * size.y * size.z */
	char *con;

	/** @brief Value of all mesh points in this block if this block is 
	 * constant. Ignored otherwise */
	n_float c_n;

	/** @brief Material property if this block is homogeneous. Ignored
	 * otherwise. */
	n_float c_a;

	/** @brief Absolute position of the lower left corner of the block. */
	n_v3i pos;

	/** @brief Size of the block. */
	n_v3i size;

	/** @brief Pointer to the neighboring block.
	 *
	 * If equal to NULL, this block is on the edge of the mesh. */
	struct block *xnext;

	/** @brief Pointer to the neighboring block.
	 *
	 * If equal to NULL, this block is on the edge of the mesh. */
	struct block *xprev;

	/** @brief Pointer to the neighboring block.
	 *
	 * If equal to NULL, this block is on the edge of the mesh. */
	struct block *ynext;

	/** @brief Pointer to the neighboring block.
	 *
	 * If equal to NULL, this block is on the edge of the mesh. */
	struct block *yprev;

	/** @brief Pointer to the neighboring block.
	 *
	 * If equal to NULL, this block is on the edge of the mesh. */
	struct block *znext;

	/** @brief Pointer to the neighboring block.
	 *
	 * If equal to NULL, this block is on the edge of the mesh. */
	struct block *zprev;

	/** @brief Pointer to the space struct. */
	struct space *sp;
};

/** @brief Structure describing one layer in the computation space.
 *
 * Interpretation of the coordinates in the structure (numbers in parenthesis
 * are examples):
 *
 * <pre> 
 *                                  | next layer
 *     |  |  |  |                   v 
 *     +--+--+--+                  ---
 *     |XX|XX|XX|    ^              ^
 *     +--+--+--+    |              | this
 *     |XX|XX|XX|    |              | layer
 *     +--+--+--+    | height (4)   |
 *     |XX|XX|XX|    |              |
 *     +--+--+--+    |              |
 *     |XX|XX|XX|    v              v
 *     +--+--+--+     <- z (10)    ---
 *     |  |  |  |                   ^
 *                                  | previous layer
 *
 *  ^ Z
 *  |
 *  +--> X
 *
 *  Key:
 *
 *  X Material property defined by this layer.
 *
 * </pre>
 *
 */
struct layer {
	/** @brief Name of the layer. */
	char *name;

	/** @brief Height of the layer in mesh units */
	n_int height;

	/** @brief Number specifying the position of this layer on the z axis.
	 * Layer with the lowest order number is at the bottom of the space. */
	n_int order;

	/** @brief Z coordinate of the plane of grid points at the bottom of
	 * the layer. */
	n_int z;

	/** @brief Default material for this layer. */
	struct material *mat;

	/** @brief Array of pointers to all objects placed in this layer. */
	struct object **obj;
	/** @brief Number of pointers in the object array. */
	int objnum;

	/** @brief Pointer to the next struct in linked list */
	struct layer *next;
};

/** @brief Structure describing finite difference grid. 
 *
 * Interpretation of the coordinates in the structure (numbers in parenthesis
 * are examples):
 *
 * <pre> 
 *                +--+--+--+   
 *             ^  |  |  |  |  
 *             |  +--+--+--+ 
 *             |  |  |  |  |
 *  size.z (5) |  +--+--+--+
 *             |  |  |  |  |
 *             |  +--+--+--+
 *             V  |  |  |  |
 * pos.z (0) ->   +--+--+--+
 *
 *                 <------>                            
 *                ^   size.x (4)     
 *                |
 *
 *                pos.x (5)
 *
 *  ^ Z
 *  |
 *  +--> X
 * </pre>
 */
struct space {
	/** @brief Three-dimensional linked list of mesh blocks. */
	struct block *blk;

	/** @brief Array of pointers to all layers used in this grid */
	struct layer **lay;
	/** @brief Number of pointers in the layer array. */
	int laynum;

	/** @brief Absolute position of the allocated part of the
	 * finite difference mesh. */
	n_v3i pos;

	/** @brief Number of mesh points in the X, Y and Z direction. */
	n_v3i size;

	/** @brief Physical distance between grid points in meters in X, Y
	 * and Z direction in meters */
	n_v3f step;

	/** @brief Name of this mesh. */
	char *name;

	/** @brief Pointer to the next struct in linked list. */
	struct space *next;
};

/** @brief Structure describing a two-dimensional object.
 *
 * Objects get their third dimension from the layer on which they are placed.
 *
 */
struct object {
	/** @brief Type of the object. */
	enum object_type type;

	/** @brief Absolute X and Y position of the lower right corner
	 * of the object. */
	n_v2i pos;

	/** @brief Absolute X and Y position of the object speficied in
	 * the config file. */
	n_v2i orig_pos;
	/** @brief Size of the object in mesh units. */
	n_v2i size;

	/** @brief Radius of the object in mesh units (used only for circular 
	 * objects) */
	n_int radius;

	/** @brief Positions of seed points in image coordinates (used only
	 * for image objects) 
	 *
	 * Last entry must be (-1,-1)*/
	n_v2i *imgpos;
	/** @brief File name of the image (used only for image objects) */
	char *image;

	/** @brief Bitmap representation of the object.
	 *
	 * This is a two dimensional array in row-major order. Value 1 in array
	 * means that this grid square is occupied by this object and value 0
	 * in array means that this grid square is not occupied. */
	char *map;

	/** @brief Role of the object in analisys */
	enum object_role role;

	/** @brief Material of this object */
	struct material *mat;

	/** @brief Set to 1 if grid points occupied by this object are to be
	 * set to a constant value. Set to 0 if grid points are variable. */
	int con;

	/** @brief Constant value of object's grid points. Used only if \a con
	 * is equal to 1. */
	n_float n;

	/** @brief Name of this object. */
	char *name;

	/** @brief Pointer to the next struct in the linked list */
	struct object *next;
};

/** @brief Structure describing a two dimensional rectangular surface. 
 *
 * Surface is always an unit square. Normal vector is always parallel to one
 * of the axes. 
 *
 * Normal vector is always a cross product between e1 and e2: n = e1 x e2 */
struct face {
	/** @brief Absolute position of one corner of the surface in grid
	 * coordinates. */
	n_v3i pos;

	/** @brief Vector pointing along the first side of the surface. */
	n_v3i e1;

	/** @brief Vector pointing along the second side of the surface. */
	n_v3i e2;
	
	/** @brief Normal vector (vector perpendicular to e1 and e2. */
	n_v3i n;

	/** @brief Pointer to the next struct in the linked list */
	struct face *next;
};

/** @brief Structure describing a net.
 *
 * A net is a collection of objects.
 *
 * In electrostatic analysis capacitances are calculated between all defined
 * nets.
 */
struct net {
	/** @brief Array of pointers to all objects included in this net. */
	struct object **obj;

	/** @brief Number of pointers in the object array. */
	int objnum;

	/** @brief Name of this net. */
	char *name;

	/** @brief Pointer to the next struct in the linked list. */
	struct net *next;
};

#endif
