/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2024, Sartura d.d.
 */

#include <string.h>

#include "memory.h"

void *xmalloc(size_t size)
{
	void *res;

	res = malloc(size);

	if (res == NULL) {
		abort();
	}

	return res;
}

void *xrealloc(void *ptr, size_t size)
{
	void *res;

	res = realloc(ptr, size);

	if (res == NULL) {
		abort();
	}

	return res;
}

void *xcalloc(size_t nmemb, size_t size)
{
	void *res;

	res = calloc(nmemb, size);

	if (res == NULL) {
		abort();
	}

	return res;
}

char *xstrdup(const char *s)
{
	char *res;

	res = strdup(s);

	if (res == NULL) {
		abort();
	}

	return res;
}

uint32_t *uint32alloc(uint32_t value)
{
	uint32_t *res = NULL;

	res = xmalloc(sizeof(uint32_t));

	*res = value;

	return res;
}
