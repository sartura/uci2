/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2024, Sartura d.d.
 */

#ifndef DEBUG_H_ONCE
#define DEBUG_H_ONCE

#include <stdio.h>
#include <time.h>
#include <string.h>

#if !defined(NDEBUG)
#define UCI2_DEBUG (1)
#else
#define UCI2_DEBUG (0)
#endif

#define DEBUG(...)                                                                            \
	do {                                                                                      \
		if (UCI2_DEBUG) {                                                                     \
			fprintf(stderr, "[%u] %s (%d): ", (unsigned) time(NULL), __FILENAME__, __LINE__); \
			fprintf(stderr, FIRST(__VA_ARGS__) "\n" REST(__VA_ARGS__));                       \
		}                                                                                     \
	} while (0)

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define FIRST(...) FIRST_HELPER(__VA_ARGS__, throwaway)
#define FIRST_HELPER(first, ...) first
#define REST(...) REST_HELPER(NUM(__VA_ARGS__), __VA_ARGS__)
#define REST_HELPER(qty, ...) REST_HELPER2(qty, __VA_ARGS__)
#define REST_HELPER2(qty, ...) REST_HELPER_##qty(__VA_ARGS__)
#define REST_HELPER_ONE(first)
#define REST_HELPER_TWOORMORE(first, ...) , __VA_ARGS__
#define NUM(...) SELECT_10TH(__VA_ARGS__, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, ONE, throwaway)
#define SELECT_10TH(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, ...) a10

#endif /* DEBUG_H_ONCE */
