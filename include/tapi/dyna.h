/*
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-08-20
 */
#ifndef TAPI_DYNA_H
#define TAPI_DYNA_H

#ifndef _WIN32
/*! uses pthread_rwlock. */
#define _XOPEN_SOURCE 600u /* enables POSIX.1-2001 */
#include <pthread.h>
#else
/*! uses SRWLOCK. */
#include <windows.h>
#endif

/*! uses size_t. */
#include <stddef.h>

/* for functions that are exported by tapi. */
#if (defined(__GNUC__) || defined(__IBMC__))
#define TAPI_EXPORT __attribute__((visibility("default")))
#define TAPI
#define TAPI_HIDDEN __attribute__((visibility("hidden")))
#else
/* if we are using msvc toolchain (winapi). */
#if defined(_MSC_VER)
#define TAPI_EXPORT __declspec(dllexport)
#define TAPI __cdecl
#define TAPI_HIDDEN
#else
#define TAPI_EXPORT
#define TAPI
#define TAPI_HIDDEN
#endif
#endif
/** \endcond */

/**
 * similarly to the `TAPI_MINIMAL` macro, `TAPI_THREAD_SAFE` adds locks to data structures (maps, lists)
 *  used internally by tapi. if you are testing within an environment that does not support POSIX.5+,
 *  then you can add the thread-safe=0 flag to your compilation process for tapi.
 */
#define TAPI_THREAD_SAFE

/**
 * @brief a dynamic array of pointers.
 *
 * `dyna_t` is a data structure for a dynamically allocated array, it can hold only a set item
 *   size, as well as anything less; can hold items less than the size of a pointer, but you
 *   should use the latter.
 *
 * elements are stored as pointers only, `dyna_t` does not copy or own the objects that are
 *   pointed to.
 *
 * @see tapi_dyna_create()
 * @see tapi_dyna_push()
 * @see tapi_dyna_pop()
 * @see tapi_dyna_free()
 */
typedef struct {
    /** array of data. */
    void** data;
    /** length (count) and capacity of the dynamic array. */
    size_t length, capacity;
#ifdef TAPI_THREAD_SAFE
    /** a read-write access lock to data (only one thread writes at a time). */
#ifndef _WIN32
    pthread_rwlock_t lock;
#else
    SRWLOCK lock;
#endif
#endif
} tapi_dyna_t;

/**
 * @brief create a dyna_t structure with a set item size.
 *
 * @return an allocated dynamic array.
 */
TAPI_EXPORT tapi_dyna_t* TAPI
tapi_dyna_create();

/**
 * @brief destroying / freeing a dynamically allocated array.
 *
 * @param array pointer to a dynamically allocated array.
 */
TAPI_EXPORT void TAPI
tapi_dyna_free(tapi_dyna_t* array);

/**
 * @brief push new item/data onto a dynamically allocated array.
 *
 * @param array pointer to a dynamically allocated array.
 * @param data data to be pushed onto the top of the allocated array.
 */
TAPI_EXPORT void TAPI
tapi_dyna_push(tapi_dyna_t* array, void* data);

/**
 * @brief pop data out of a dynamically allocated array,
 *  while also making sure to shift down / coalesce the memory.
 *  this function does not SHRINK the allocated memory, see
 *  @ref dyna_shrink().
 *
 * @param array pointer to a dynamically allocated array.
 * @param index index at which to pop the item.
 * @return data at the specified index, popped off the array.
 */
TAPI_EXPORT void* TAPI
tapi_dyna_pop(tapi_dyna_t* array, size_t index);

/**
 * @brief get the data at the index specified from a dynamically
 *  allocated array.
 *
 * @param array the dynamically allocated array to get information from.
 * @param index the index in the array that we are to retrieve data from.
 * @return 0x0 if the index is out of bounds or the data at a specified index in the array.
 */
TAPI_EXPORT void* TAPI
tapi_dyna_get(tapi_dyna_t* array, size_t index);

/**
 * @brief shrinks the array to the length via realloc.
 *
 * @param array the dynamically allocated array to be shrinked.
 */
TAPI_EXPORT void TAPI
tapi_dyna_shrink(tapi_dyna_t* array);

/**
 * @brief make a new dynamic array given an array of data.
 *
 * @param data the list of data to be copied into a dynamic array.
 * @param length the length of the list of data.
 * @return an allocated dyna_t structure with all data copied over.
 */
TAPI_EXPORT tapi_dyna_t* TAPI
tapi_dyna_make(void** data, size_t length);

/* a get operation. */
#define dyna_get(array, type, index) ((type) tapi_dyna_get(array, index))

/* starting an iteration. */
#define dyna_foreach_it(array, type, var, iter) \
    for (size_t iter = 0; iter < (array)->length; iter++) { \
        type var = dyna_get(array, type, iter);

/* starting an iteration. */
#define dyna_foreach(array, type, var) \
    for (size_t i = 0; i < (array)->length; i++) { \
        type var = dyna_get(array, type, i);

/* starting an iteration, backwards. */
#define dyna_inv_foreach(array, type, var) \
    for (size_t i = (array)->length; i != 0; i--) { \
        type var = dyna_get(array, type, i - 1);

/* starting an iteration, backwards. */
#define dyna_inv_foreach_it(array, type, var, iter) \
    for (size_t iter = (array)->length; iter != 0; iter--) { \
        type var = dyna_get(array, type, iter - 1);

/* ending an iteration. */
#define dyna_endforeach(array) }
#endif /* TAPI_DYNA_H */