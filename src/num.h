/**
 * @file src/num.h
 *
 * @brief , header.
 */
#ifndef _NUM_H
#define _NUM_H

enum axis_t {
	X,
	Y,
	Z
};

typedef enum axis_t n_axis;

/**
 * @brief Floating point type used for data point values.
 */
typedef float n_float;
/**
 * @brief Integer type used for data point coordinates.
 */
typedef int n_int;

/**
 * @brief 3-dimensional floating point vector.
 */
struct v3f_t {
	n_float x,y,z;
};

/**
 * @brief 3-dimensional integer vector.
 */
struct v3i_t {
	n_int x,y,z;
};

/**
 * @brief 2-dimensional floating point vector.
 */
struct v2f_t {
	n_float x,y;
};

/**
 * @brief 2-dimensional integer vector.
 */
struct v2i_t {
	n_int x,y;
};

typedef struct v3f_t n_v3f;
typedef struct v3i_t n_v3i;
typedef struct v2f_t n_v2f;
typedef struct v2i_t n_v2i;

extern const n_v2i v2i_x;
extern const n_v2i v2i_y;

extern const n_v2i v2i_o;
extern const n_v2i v2i_i;

extern const n_v3i v3i_x;
extern const n_v3i v3i_y;
extern const n_v3i v3i_z;

extern const n_v3i v3i_o;
extern const n_v3i v3i_i;

n_v2i v2i(n_int x, n_int y);
n_v2i v2i_add(n_v2i a, n_v2i b);
n_v2i v2i_sub(n_v2i a, n_v2i b);
int v2i_positive(n_v2i a);

n_v3i v3i_ez(n_v2i a, n_int z);
n_v2i v2i_cz(n_v3i a);

n_v3i v3i(n_int x, n_int y, n_int z);
n_v3i v3i_add(n_v3i a, n_v3i b);
n_v3i v3i_sub(n_v3i a, n_v3i b);
n_v3i v3i_mul(n_v3i a, n_v3i b);
n_v3i v3i_vectp(n_v3i a, n_v3i b);

n_v3f v3f(n_float x, n_float y, n_float z);

#endif
