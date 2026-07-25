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
 * @date 2026-07-16
 */
#ifndef MAP_H
#define MAP_H

/*! uses bool, true, false. */
#include <stdbool.h>

/*! size_t. */
#include <stddef.h>

/*! uses pthread_rwlock. */
#include <tapi/dyna.h>

/**
 * a data structure for representing the key-value pair/entry within a hashmap (often
 *  referred to as a dictionary). this struct only holds the memory of the key, nothing
 *  else.
 */
typedef struct {
    char* key; /* the key of the entry (lookup value). */
    void* value; /* the value of the entry, a pointer to something. */
    bool occupied; /* if this entry is currently occupied, true, if filled. */
} entry_t;

/**
 * @brief free an entry from a map.
 *
 * @param entry the entry to be freed.
 */
void
free_entry(entry_t* entry);

/**
 * a data structure for representing a cuckoo-based hashmap with a maximum load factor
 *  of 50%. a large amount of this is modified from the reference below and put into my
 *  style, please read either the gpl, lgpl or bsd license if you plan on copying this
 *  header.
 *
 * @ref https://github.com/canasai/ckhash/blob/master/cuckoo_hash/cuckoo_hash.h
 */
typedef struct {
    entry_t* t1, *t2; /* both tables used within cuckoo hashing. */
    size_t count, size; /* the size and count of entries found within t1/t2. */
#ifdef TAPI_THREAD_SAFE
    /** a read-write access lock to t1 and t2 (only one thread writes at a time). */
    pthread_rwlock_t lock;
#endif
} map_t;

/** @return a new allocated cuckoo map ready for insertion. */
map_t*
map_make(void);

/**
 * @brief resize the map to a new size within t1 and t2, this also rehashes all
 *  the entries within the table (both t1 and t2).
 *
 * @param map the map given.
 * @param new_size the new size of the map.
 */
void
map_resize(map_t* map, size_t new_size);

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
map_push(map_t* map, const char* key, void* value);

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
map_pop(map_t* map, const char* key);

/**
 * @brief attempt to look up an entry based on its key.
 *
 * @param map the map to look through.
 * @param key the key of the entry we are looking for.
 * @return a pointer to an entry or 0x0 if not found.
 */
entry_t*
map_lookup(map_t* map, const char* key);

/**
 * @brief cleanup all items and entries within the map. this removes all
 *  given entries held by the map as well as the duplicated keys held by the
 *  entries themselves.
 *
 * @param map the map to be cleaned up.
 */
void
map_cleanup(map_t* map);

/* a get operation simplified for a map. */
#define map_get(map, key, value_type) \
    (value_type)(map_lookup(map, key)->value);
#endif /* MAP_H */
