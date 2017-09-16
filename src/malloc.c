/**
 * @file src/malloc.c
 *
 * @brief , code.
 */
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "error.h"
#include "malloc.h"

#ifdef MALLOC_DEBUG
struct code_module {
	char *name;
	size_t bytes;

	struct code_module *next;
};

struct malloc_block_info {
	struct code_module *owner;
	size_t bytes;
};

static struct code_module *code_modules=NULL;

static struct code_module *mem_block_find(char *name)
{
	struct code_module *dest;

	assert(name!=NULL);

	dest=code_modules;
	while(dest!=NULL) {
		if(!strcmp(dest->name, name)) return dest;
		dest=dest->next;
	}

	/* block was not found. allocate a new one */

	dest=calloc(1, sizeof(*dest));

	dest->name=strdup(name);
	dest->bytes=0;
	dest->next=code_modules;

	code_modules=dest;

	return(dest);
}

void *mem_calloc(size_t nmemb, size_t size, char *block)
{
	struct code_module *dest;
	struct malloc_block_info *info;
	void *r;

	r=calloc(1, nmemb*size+sizeof(*info));
	if(r!=NULL) {
		dest=mem_block_find(block);
		dest->bytes+=nmemb*size;

		info=r;

		info->owner=dest;
		info->bytes=nmemb*size;

		return r+sizeof(*info);
	} else {
		return NULL;
	}
}

void mem_free(void *ptr)
{
	struct malloc_block_info *info;

	info=ptr-sizeof(*info);

	assert(info->owner!=NULL);

	info->owner->bytes-=info->bytes;

	free(info);
}

void mem_info()
{
	struct code_module *cur;

	info("Memory allocation info:");

	cur=code_modules;
	while(cur!=NULL) {
		info("   %16d bytes - %s", cur->bytes, cur->name);
		cur=cur->next;
	}
}

#else

void *mem_calloc(size_t nmemb, size_t size, char *block)
{
	return calloc(nmemb, size);
}

void mem_free(void *ptr)
{
	free(ptr);
}

void mem_info()
{
}

#endif
