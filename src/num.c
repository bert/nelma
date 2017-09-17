/**
 * @file src/num.c
 *
 * @brief , code.
 */
#include "num.h"

/**
 * @brief 2-dimensional unit-length vector parallel to the X-axis.
 */
const n_v2i v2i_x = { x: 1, y: 0 };
/**
 * @brief 2-dimensional unit-length vector parallel to the Y-axis.
 */
const n_v2i v2i_y = { x: 0, y: 1 };

/**
 * @brief 2-dimensional zero vector.
 */
const n_v2i v2i_o = { x: 0, y: 0 };
/**
 * @brief 2-dimensional unit vector.
 */
const n_v2i v2i_i = { x: 1, y: 1 };

/**
 * @brief 3-dimensional unit-length vector parallel to the X-axis.
 */
const n_v3i v3i_x = { x: 1, y: 0, z: 0 };
/**
 * @brief 3-dimensional unit-length vector parallel to the Y-axis.
 */
const n_v3i v3i_y = { x: 0, y: 1, z: 0 };
/**
 * @brief 3-dimensional unit-length vector parallel to the Z-axis.
 */
const n_v3i v3i_z = { x: 0, y: 0, z: 1 };

/**
 * @brief 3-dimensional zero vector.
 */
const n_v3i v3i_o = { x: 0, y: 0, z: 0 };
/**
 * @brief 3-dimensional unit vector.
 */
const n_v3i v3i_i = { x: 1, y: 1, z: 1 };

/**
 * @brief Return 1 if both components are greater or equal to zero.
 */
int v2i_positive(n_v2i a)
{
	if((a.x>=0)&&(a.y>=0)) {
		return 1;
	} else {
		return 0;
	}
}

/**
 * @brief Add two 2-dimensional vectors.
 */
n_v2i v2i_add(n_v2i a, n_v2i b)
{
	n_v2i c;

	c.x=a.x+b.x;
	c.y=a.y+b.y;

	return c;
}

/**
 * @brief Subtract two 2-dimensional vectors. 
 *
 * @return a - b
 */
n_v2i v2i_sub(n_v2i a, n_v2i b)
{
	n_v2i c;

	c.x=a.x-b.x;
	c.y=a.y-b.y;

	return c;
}

/**
 * @brief Create a 2-dimensional vector from two integers.
 */
n_v2i v2i(n_int x, n_int y)
{
	n_v2i r;

	r.x=x;
	r.y=y;

	return r;
}

/**
 * @brief Create a 3-dimensional vector from three integers.
 */
n_v3i v3i(n_int x, n_int y, n_int z)
{
	n_v3i r;

	r.x=x;
	r.y=y;
	r.z=z;

	return r;
}

/**
 * @brief Add two 3-dimensional vectors.
 *
 * @return a + b
 */
n_v3i v3i_add(n_v3i a, n_v3i b)
{
	n_v3i c;

	c.x=a.x+b.x;
	c.y=a.y+b.y;
	c.z=a.z+b.z;

	return c;
}

/**
 * @brief Subtract two 3-dimensional vectors.
 *
 * @return a - b
 */
n_v3i v3i_sub(n_v3i a, n_v3i b)
{
	n_v3i c;

	c.x=a.x-b.x;
	c.y=a.y-b.y;
	c.z=a.z-b.z;

	return c;
}

/**
 * @brief Cross vector (cross product or dot product) of two
 * 3-dimensional vectors.
 *
 * @return a x b
 */
n_v3i v3i_vectp(n_v3i a, n_v3i b)
{
	n_v3i c;

	c.x=a.y * b.z - a.z * b.y;
	c.y=a.z * b.x - a.x * b.z;
	c.z=a.x * b.y - a.y * b.x;

	return c;
}

/**
 * @brief Convert a 3-dimensional integer vector into a 2-dimensional
 * integer vector (xy-plane).
 */
n_v2i v2i_cz(n_v3i a)
{
	n_v2i c;

	c.x=a.x;
	c.y=a.y;

	return c;
}

/**
 * @brief Create a 3-dimensional integer vector from a 2-dimensional
 * integer vector and an integer value (z).
 */
n_v3i v3i_ez(n_v2i a, n_int z)
{
	n_v3i c;

	c.x=a.x;
	c.y=a.y;
	c.z=z;

	return c;
}

/**
 * @brief Create a 3-dimensional float vector from three float values.
 */
n_v3f v3f(n_float x, n_float y, n_float z)
{
	n_v3f r;

	r.x=x;
	r.y=y;
	r.z=z;

	return r;
}

