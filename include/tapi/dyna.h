/*
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-04
 */
#ifndef TAPI_DYNA_H
#define TAPI_DYNA_H

/*! @uses size_t. */
#include <stddef.h>

/*! @uses pthread_rwlock. */
#define _XOPEN_SOURCE 600u /* enables POSIX.1-2001 */
#include <pthread.h>

/* for functions that are exported by tapi. */
#if (defined(__GNUC__) || defined(__IBMC__))
#define TAPI_EXPORT __attribute__((visibility("default")))
#else
/* if we are using msvc toolchain (winapi). */
#if (defined(_MSC_VER))
#define TAPI_EXPORT __declspec(dllexport)
#else
#define TAPI_EXPORT
#endif
#endif
/** \endcond */

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
    pthread_rwlock_t lock;
#endif
} tapi_dyna_t;

/**
 * @brief create a dyna_t structure with a set item size.
 *
 * @return an allocated dynamic array.
 */
TAPI_EXPORT tapi_dyna_t*
tapi_dyna_create();

/**
 * @brief destroying / freeing a dynamically allocated array.
 *
 * @param array pointer to a dynamically allocated array.
 */
TAPI_EXPORT void
tapi_dyna_free(tapi_dyna_t* array);

/**
 * @brief push new item/data onto a dynamically allocated array.
 *
 * @param array pointer to a dynamically allocated array.
 * @param data data to be pushed onto the top of the allocated array.
 */
TAPI_EXPORT void
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
TAPI_EXPORT void*
tapi_dyna_pop(tapi_dyna_t* array, size_t index);

/**
 * @brief get the data at the index specified from a dynamically
 *  allocated array.
 *
 * @param array the dynamically allocated array to get information from.
 * @param index the index in the array that we are to retrieve data from.
 * @return 0x0 if the index is out of bounds or the data at a specified index in the array.
 */
TAPI_EXPORT void*
tapi_dyna_get(tapi_dyna_t* array, size_t index);

/**
 * @brief shrinks the array to the length via realloc.
 *
 * @param array the dynamically allocated array to be shrinked.
 */
TAPI_EXPORT void
tapi_dyna_shrink(tapi_dyna_t* array);

/**
 * @brief make a new dynamic array given an array of data.
 *
 * @param data the list of data to be copied into a dynamic array.
 * @param length the length of the list of data.
 * @return an allocated dyna_t structure with all data copied over.
 */
TAPI_EXPORT tapi_dyna_t*
tapi_dyna_make(void** data, size_t length);

/* a get operation. */
#define DYNA_GET(array, type, index) ((type) tapi_dyna_get(array, index))

/*! if we are compiling with the thread-safe flag. */
#ifdef TAPI_THREAD_SAFE
/* starting an iteration. */
#define DYNA_FOREACH_IT(array, type, var, iter) \
    pthread_rwlock_rdlock(&(array)->lock); \
    for (size_t iter = 0; iter < (array)->length; iter++) { \
        type var = DYNA_GET(array, type, iter);

/* starting an iteration. */
#define DYNA_FOREACH(array, type, var) \
    pthread_rwlock_rdlock(&(array)->lock); \
    for (size_t i = 0; i < (array)->length; i++) { \
        type var = DYNA_GET(array, type, i);

/* starting an iteration, backwards. */
#define DYNA_INV_FOREACH(array, type, var) \
    pthread_rwlock_rdlock(&(array)->lock); \
    for (size_t i = (array)->length; i != 0; i--) { \
        type var = DYNA_GET(array, type, i - 1);

/* starting an iteration, backwards. */
#define DYNA_INV_FOREACH_IT(array, type, var, iter) \
    pthread_rwlock_rdlock(&(array)->lock); \
    for (size_t iter = (array)->length; iter != 0; iter--) { \
        type var = DYNA_GET(array, type, iter - 1);

/* ending an iteration. */
#define DYNA_ENDFOREACH(array) \
        pthread_rwlock_unlock(&(array)->lock); \
    }
#else
/* starting an iteration. */
#define DYNA_FOREACH_IT(array, type, var, iter) \
    for (size_t iter = 0; iter < (array)->length; iter++) { \
        type var = DYNA_GET(array, type, iter);

/* starting an iteration. */
#define DYNA_FOREACH(array, type, var) \
    for (size_t i = 0; i < (array)->length; i++) { \
        type var = DYNA_GET(array, type, i);

/* starting an iteration, backwards. */
#define DYNA_INV_FOREACH(array, type, var) \
    for (size_t i = (array)->length; i != 0; i--) { \
        type var = DYNA_GET(array, type, i - 1);

/* starting an iteration, backwards. */
#define DYNA_INV_FOREACH_IT(array, type, var, iter) \
    for (size_t iter = (array)->length; iter != 0; iter--) { \
        type var = DYNA_GET(array, type, iter - 1);

/* ending an iteration. */
#define DYNA_ENDFOREACH(array) }
#endif /* TAPI_THREAD_SAFE */
#endif /* TAPI_DYNA_H */
