/* Copyright (C) 2008-2009 National Institute of Information and Communications Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above
 *  copyright notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * Neither the name of the Nippon Telegraph and Telegraph Corporation
 * nor the names of its contributors may be used to endorse or
 * promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Additional notice for modifications
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * @author Sean Hobeck
 * @date 2026-08-27
 */
#include "map.h"

/*! assert. */
#include <assert.h>

/*! uint8-64_t, etc... */
#include <stdint.h>

/*! calloc, free. */
#include <stdlib.h>

/*! strcmp on hashes. */
#include <string.h>

/*! internal. */
#include "intt.h"

/* all copied from the repos in the ref below. */
#define FNV1A32_BASIS 0x811c9dc5
#define FNV1A32_PRIME 0x1000193
#define PHI32 0x9e3779b9 /*! 2^32 * phi. @ref knuth toacp... see below v */
#define FNV1A64_BASIS 0xcbf29ce484222325
#define FNV1A64_PRIME 0x100000001b3
#define PHI64 0x9e3779b97f4a7c15 /*! 2^64 * phi. @ref knuth toacp, pg 21 + sec 1.2.8. */

/**
 * @brief compute the 32-bit fnv1a hash for a string.
 *
 * @ref https://github.com/irfantopal/fnv1a/blob/main/fnv1a32.h
 * @param string the string to be computed.
 * @return the associated fnv1a hash for the string.
 */
internal uint32_t
fnv1a32(const char* string) {
    uint32_t out = FNV1A32_BASIS;
    while (*string) out = (out ^ *string++) * FNV1A32_PRIME;
    return out;
}

/**
 * @brief compute the 64-bit fnv1a hash for a string.
 *
 * @ref https://github.com/irfantopal/fnv1a/blob/main/fnv1a64.h
 * @param string the string to be computed.
 * @return the associated fnv1a hash for the string.
 */
internal uint64_t
fnv1a64(const char* string) {
    uint64_t out = FNV1A64_BASIS;
    while (*string) out = (out ^ *string++) * FNV1A64_PRIME;
    return out;
}

/**
 * @brief the first hash function used in the cuckoo hashing algorithm (just fnv1a).
 *
 * @param string the string to compute a hash for.
 * @param table_size the size of the table used in the hashmap.
 * @return the associated hash.
 */
internal uint64_t
hash1(const char* string, const size_t table_size) {
#if defined(__amd64__) || defined(__aarch64__)
    return fnv1a64(string) % table_size;
#else
    return fnv1a32(string) % table_size;
#endif
}

/**
 * @brief the second hash function used in the cuckoo hashing algorithm (golden ratio multiplier).
 *
 * @param string the string to compute a hash for.
 * @param table_size the size of the table used in the hashmap.
 * @return the associated hash.
 */
internal uint64_t
hash2(const char* string, const size_t table_size) {
#if defined(__amd64__) || defined(__aarch64__)
    return (fnv1a64(string) * PHI64) % table_size;
#else
    return (fnv1a32(string) * PHI32) % table_size;
#endif
}

/**
 * @brief free an entry from a map.
 *
 * @param entry the entry to be freed.
 */
void
free_entry(entry_t* entry) {
    if (entry->occupied) {
        free(entry->key);
        entry->key = 0x0;
        entry->value = 0x0;
        entry->occupied = false;
    }
}

/** @return a new allocated cuckoo map ready for insertion. */
map_t*
map_make(void) {
    map_t* map = calloc(1u, sizeof *map);
    map->size = 16u;
    map->t1 = calloc(16u, sizeof(entry_t));
    map->t2 = calloc(16u, sizeof(entry_t));
#ifdef TAPI_THREAD_SAFE
#ifndef _WIN32
    pthread_rwlock_init(&map->lock, 0x0);
#else
    SRWLOCK lock = { 0 };
#endif
#endif
    return map;
};

/**
 * @brief resize the map to a new size within t1 and t2, this also rehashes all
 *  the entries within the table (both t1 and t2).
 *
 * @param map the map given.
 * @param new_size the new size of the map.
 */
void
map_resize(map_t* map, size_t new_size) {
    /* make copy of the tables, then allocate new ones. */
    assert(map != 0x0);

    /* no locks need to be added here since exclusive is already done on map_push. */
    entry_t* c1 = map->t1, *c2 = map->t2;
    size_t size = map->size;
    map->t1 = calloc(new_size, sizeof(entry_t));
    map->t2 = calloc(new_size, sizeof(entry_t));
    map->size = new_size;
    map->count = 0u;

    /* reiterate... */
    for (size_t i = 0u; i < size; i++) {
        if (c1[i].occupied) {
            map_push(map, c1[i].key, c1[i].value);
            free_entry(&c1[i]);
        }
        if (c2[i].occupied) {
            map_push(map, c2[i].key, c2[i].value);
            free_entry(&c2[i]);
        }
    }
    free(c1);
    free(c2);
};

/* the max number of chaining operations that an insert will go through. */
#define MAX_CHAIN 128u

/* the max depth of recursion on push/insertion. */
#define MAX_DEPTH 24u

/**
 * @brief attempt to push an entry onto the map with a given key-value pair.
 * note, this is cuckoo hashing so any entries found on collision will simply
 *  be kicked out of their spot, possible loss of data but very unlikely due to fnv1a.
 *
 * @param map the map to push the entry onto.
 * @param key the key of the entry.
 * @param value the value of the entry.
 */
void
map_push(map_t* map, const char* key, void* value) {
    /* check recursion depth. */
    static size_t depth = 1u;
    if (depth > MAX_DEPTH)
        return;

    /* find if the already existing entry is in there, if so evict. */
    assert(map != 0x0 && key != 0x0);
    entry_t* existing = map_lookup(map, key);
#ifdef TAPI_THREAD_SAFE
#ifndef _WIN32
    pthread_rwlock_wrlock(&map->lock);
#else
    AcquireSRWLockExclusive(&map->lock);
#endif
#endif
    if (existing != 0x0) {
        free_entry(existing);
        existing->occupied = true;
#ifndef _WIN32
        existing->key = strdup(key);
#else
        existing->key = _strdup(key);
#endif
        existing->value = value;
#ifdef TAPI_THREAD_SAFE
#ifndef _WIN32
        pthread_rwlock_unlock(&map->lock);
#else
        ReleaseSRWLockExclusive(&map->lock);
#endif
#endif
        return;
    }

    /* o.w. we have to emplace based on cuckoo, first we check the load factor (opposite of ref). */
    float alpha = (float)map->count / (float)map->size;
    if (alpha > 0.5f) {
#ifdef TAPI_THREAD_SAFE
#ifndef _WIN32
        pthread_rwlock_unlock(&map->lock);
#else
        ReleaseSRWLockExclusive(&map->lock);
#endif
#endif
        map_resize(map, map->size * 2u);
        #ifdef TAPI_THREAD_SAFE
#ifndef _WIN32
    pthread_rwlock_wrlock(&map->lock);
#else
    AcquireSRWLockExclusive(&map->lock);
#endif
#endif
    }

    /* chain... */
    uint64_t iter = 0x0, table = 1u;
    void* iter_value = value;
#ifndef _WIN32
    char* iter_key = strdup(key);
#else
    char* iter_key = _strdup(key);
#endif
    for (size_t chain = 0u; chain < MAX_CHAIN; chain++) {
        /* if we are on t1. */
        if (table == 1u) {
            iter = hash1(iter_key, map->size);
            if (!map->t1[iter].occupied) {
                /* emplace and we are done. */
                map->t1[iter].key = iter_key;
                map->t1[iter].value = iter_value;
                map->t1[iter].occupied = true;
                map->count++;
#ifdef TAPI_THREAD_SAFE 
#ifndef _WIN32
                pthread_rwlock_unlock(&map->lock);
#else
                ReleaseSRWLockExclusive(&map->lock);
#endif
#endif
                return;
            }
            entry_t* entry = &map->t1[iter];
            char* temp_key = entry->key;
            void* temp_value = entry->value;
            entry->key = iter_key;
            entry->value = iter_value;

            /* set the new iteration values then swap to t2. */
            iter_key = temp_key;
            iter_value = temp_value;
            table = 2u;
        } else {
            /* if we are on t2. */
            iter = hash2(iter_key, map->size);
            if (!map->t2[iter].occupied) {
                /* emplace and we are done. */
                map->t2[iter].key = iter_key;
                map->t2[iter].value = iter_value;
                map->t2[iter].occupied = true;
                map->count++;
#ifdef TAPI_THREAD_SAFE 
#ifndef _WIN32
                pthread_rwlock_unlock(&map->lock);
#else
                ReleaseSRWLockExclusive(&map->lock);
#endif
#endif
                return;
            }
            entry_t* entry = &map->t2[iter];
            char* temp_key = entry->key;
            void* temp_value = entry->value;
            entry->key = iter_key;
            entry->value = iter_value;

            /* set the new iteration values then swap to t1. */
            iter_key = temp_key;
            iter_value = temp_value;
            table = 1u;
        }
    }

    /* if we couldn't emplace, resize to double and try again. */
    free(iter_key);
#ifdef TAPI_THREAD_SAFE 
#ifndef _WIN32
    pthread_rwlock_unlock(&map->lock);
#else
    ReleaseSRWLockExclusive(&map->lock);
#endif
#endif
    /* shouldn't recurse very far but still possible. */
    if (depth < MAX_DEPTH) {
        depth++;
        map_resize(map, map->size * 2u);
        map_push(map, key, value);
        depth--;
    }
};

/**
 * @brief attempt to pop an entry from the map with a key from the possible
 *  key-value pair. note this is no longer held by the table and must be freed
 *  manually.
 *
 * @param map the map to pop the entry from.
 * @param key the key of the entry.
 * @return if successful in popping, the entry, o.w. false.
 */
entry_t*
map_pop(map_t* map, const char* key) {
    /* attempt to look up the entry. */
    entry_t* entry = map_lookup(map, key);
#ifdef TAPI_THREAD_SAFE
#ifndef _WIN32
    pthread_rwlock_wrlock(&map->lock);
#else
    AcquireSRWLockExclusive(&map->lock);
#endif
#endif
    if (entry != 0x0) {
        entry_t* copy = calloc(1u, sizeof *copy);
        memcpy(copy, entry, sizeof *copy);
        free_entry(entry);
        map->count -= 1u;
#ifdef TAPI_THREAD_SAFE 
#ifndef _WIN32
        pthread_rwlock_unlock(&map->lock);
#else
        ReleaseSRWLockExclusive(&map->lock);
#endif
#endif
        return copy;
    }
#ifdef TAPI_THREAD_SAFE 
#ifndef _WIN32
    pthread_rwlock_unlock(&map->lock);
#else
    ReleaseSRWLockExclusive(&map->lock);
#endif
#endif
    return 0x0; /* not found :( */
};

/**
 * @brief attempt to look up an entry based on its key.
 *
 * @param map the map to look through.
 * @param key the key of the entry we are looking for.
 * @return a pointer to an entry or 0x0 if not found.
 */
entry_t*
map_lookup(map_t* map, const char* key) {
    /* check t1 then t2, very simple. */
    assert(map != 0x0 && key != 0x0);
#ifdef TAPI_THREAD_SAFE
#ifndef _WIN32
    pthread_rwlock_rdlock(&map->lock);
#else
    AcquireSRWLockShared(&map->lock);
#endif
#endif
    uint64_t idx = hash1(key, map->size);
    if (map->t1[idx].occupied && strcmp(map->t1[idx].key, key) == 0) {
#ifdef TAPI_THREAD_SAFE 
#ifndef _WIN32
        pthread_rwlock_unlock(&map->lock);
#else
        ReleaseSRWLockShared(&map->lock);
#endif
#endif
        return &map->t1[idx];
    }
    idx = hash2(key, map->size);
    if (map->t2[idx].occupied && strcmp(map->t2[idx].key, key) == 0) {
#ifdef TAPI_THREAD_SAFE 
#ifndef _WIN32
        pthread_rwlock_unlock(&map->lock);
#else
        ReleaseSRWLockShared(&map->lock);
#endif
#endif
        return &map->t2[idx];
    }
#ifdef TAPI_THREAD_SAFE 
#ifndef _WIN32
    pthread_rwlock_unlock(&map->lock);
#else
    ReleaseSRWLockShared(&map->lock);
#endif
#endif
    return 0x0; /* not found :( */
};

/**
 * @brief cleanup all items and entries within the map. this removes all
 *  given entries held by the map as well as the duplicated keys held by the
 *  entries themselves.
 *
 * @param map the map to be cleaned up.
 */
void
map_cleanup(map_t* map) {
    /* make copy of the tables, then allocate new ones. */
    assert(map != 0x0);
#ifdef TAPI_THREAD_SAFE
#ifndef _WIN32
    pthread_rwlock_wrlock(&map->lock);
#else
    AcquireSRWLockExclusive(&map->lock);
#endif
#endif
    /* iterate... */
    for (size_t i = 0u; i < map->size; i++) {
        if (map->t1[i].occupied) free_entry(&map->t1[i]);
        if (map->t2[i].occupied) free_entry(&map->t2[i]);
    }
#ifdef TAPI_THREAD_SAFE 
#ifndef _WIN32
    pthread_rwlock_unlock(&map->lock);
#else
    ReleaseSRWLockExclusive(&map->lock);
#endif
#endif
    free(map->t1);
    free(map->t2);
    free(map);
};