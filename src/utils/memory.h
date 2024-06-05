/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2024, Sartura d.d.
 */

#ifndef MEMORY_H_ONCE
#define MEMORY_H_ONCE

#include <stdlib.h>
#include <stdint.h>

#define XFREE(x)    \
	do {            \
		free(x);    \
		(x) = NULL; \
	} while (0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
void *xcalloc(size_t nmemb, size_t size);
char *xstrdup(const char *s);
uint32_t *uint32alloc(uint32_t value);

#endif /* MEMORY_H_ONCE */
