/*
 * ct_check.h: Macros for testing constant-time functions
 *
 * BSD 3-Clause License
 * Copyright (c) 2021, Krzysztof Kwiatkowski
 * All rights reserved.
 *
 * For detailed descriptions:
 * see https://www.amongbytes.com/post/20210709-testing-constant-time/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CT_CHECK_H
#define CT_CHECK_H

#include <stddef.h>

// helper
#define VOID(V) ((void)V)

// Uses Clang's Memory Sanitizer
#if defined(PQC_USE_CTSANITIZER) && defined(__clang__) && defined(__has_feature)
#if __has_feature(memory_sanitizer)
#include <stddef.h>
#include <sanitizer/msan_interface.h>
#endif
#elif defined(PQC_USE_CTGRIND)
#include <stdio.h>
#include <stdlib.h>
#include <valgrind/valgrind.h>
#include <valgrind/memcheck.h>
#endif

// Set sz bytes of memory starting at address p as uninitialized. Switches on constat time checks.
static inline void ct_poison(const volatile void *p, size_t sz) {
#if defined(PQC_USE_CTSANITIZER) && defined(__clang__) && defined(__has_feature)
#if __has_feature(memory_sanitizer)
	__msan_allocated_memory(p,sz);
#endif
#elif defined(PQC_USE_CTGRIND)
	VALGRIND_MAKE_MEM_UNDEFINED(p,sz);
#else
	VOID(p), VOID(sz);
#endif
}

// Set sz bytes of memory starting at p as initialized. Switches off constat time checks.
static inline void ct_purify(const volatile void *p, size_t sz) {
#if defined(PQC_USE_CTSANITIZER) && defined(__clang__) && defined(__has_feature)
#if __has_feature(memory_sanitizer)
	__msan_unpoison(p,sz);
#endif
#elif defined(PQC_USE_CTGRIND)
	VALGRIND_MAKE_MEM_DEFINED(p,sz);
#else
	VOID(p), VOID(sz);
#endif
}

// Function instructs memory sanitizer that code expects to do operation on unintialized memory.
static inline void ct_expect_uum() {
#if defined(PQC_USE_CTSANITIZER) && defined(__clang__) && defined(__has_feature)
#if __has_feature(memory_sanitizer)
	__msan_set_expect_umr(1);
#endif
#endif
}

// Checks if action on unintialized memory has occured. If this is not a case
// then error is reported. It works in tandem with ct_expect_umr(). In current version of
// MSan, the code needs to be compiled with `-mllvm -msan-keep-going=1` flags in order to work
// correctly.
static inline void ct_require_uum() {
#if defined(PQC_USE_CTSANITIZER) && defined(__clang__) && defined(__has_feature)
#if __has_feature(memory_sanitizer)
	__msan_set_expect_umr(0);
#endif
#endif
}

static inline void ct_print_shadow(const volatile void* p, size_t sz) {
#if defined(PQC_USE_CTSANITIZER) && defined(__clang__) && defined(__has_feature)
#if __has_feature(memory_sanitizer)
	__msan_dump_shadow(p,sz);
#endif
#elif defined(PQC_USE_CTGRIND)
	size_t r = 0;
	uint8_t *b = NULL;

	b = (uint8_t*)malloc(sz);
	// crash if malloc fails
	r = VALGRIND_GET_VBITS(p,b,sz);
	if (r != 1) {
		fprintf(stderr, "Can't get shadow memory [%s]\n",
			(r==0)||(r==2)
			?"INTERNAL" /*should not happen*/
			:"NOT ADDRESABLE" /* some parts of p are not addressable.*/);
		goto end;
	}
	for (r=0; r<sz; r++) {
		fprintf(stderr, "%02X%c", ((const uint8_t*)b)[r], (r%8)?' ':' ');
	}
	fprintf(stderr, "\n");
end:
	free(b);

#else
	VOID(p),VOID(sz);
#endif
}

#endif // CT_CHECK_H
