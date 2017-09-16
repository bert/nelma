/**
 * @file src/malloc.h
 *
 * @brief , header.
 */
#ifndef _MALLOC_H
#define _MALLOC_H

void *mem_calloc(size_t nmemb, size_t size, char *block);
void mem_free(void *ptr);
void mem_info();

#define n_calloc(_nmemb_, _size_)mem_calloc((_nmemb_), (_size_), __FILE__)
#define n_free(_ptr_) mem_free((_ptr_))

#endif
