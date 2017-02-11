#ifndef _SOR_H
#define _SOR_H

#include "struct.h"

/** @file 
 * @brief SOR algorithm, header */

extern n_float a_soromega;

void sor_iterate(struct space *sp);

#endif
