/**
 * @author Sean Hobeck
 * @date 2026-08-28
 */
#include "int/map.h"

/*! uses assert. */
#include <assert.h>

/*! uses printf. */
#include <stdio.h>

/*! uses calloc. */
#include <stdlib.h>

/**
 * @brief test pushing a singular item into a map.
 */
void
test_map_push_single_item(void) {
    /* arrange. */
    map_t* map = map_make();

    /* act. */
    map_push(map, "1", 0x1234);

    /* assert. */
    assert(map != 0x0);
    assert(map->size != 0);
    assert(map->count == 1u);
    assert(((float)map->count / (float)map->size) <= 0.5f);
    printf("correctly pushed a single item into a map!\n");
};

/**
 * @brief test pushing multiple (small) items into a map.
 */
void
test_map_push_multiple_small_items(void) {
    /* arrange. */
    map_t* map = map_make();

    /* act. */
    map_push(map, "1", 0x1234);
    map_push(map, "2", 0x5678);
    map_push(map, "3", 0x91011);

    /* assert. */
    assert(map != 0x0);
    assert(map->size != 0);
    assert(map->count == 3u);
    assert(((float)map->count / (float)map->size) <= 0.5f);
    printf("correctly pushed a multiple (small count) items into a map!\n");
};

/**
 * @brief test pushing multiple (medium) items into a map.
 */
void
test_map_push_multiple_medium_items(void) {
    /* arrange. */
    map_t* map = map_make();

    /* act. */
    map_push(map, "1", 0x1234);
    map_push(map, "2", 0x5678);
    map_push(map, "3", 0x91011);
    map_push(map, "4", 0x12113);
    map_push(map, "5", 0x960451);
    map_push(map, "6", 0x91011);
    map_push(map, "7", 0x11012);
    map_push(map, "8", 0x11);

    /* assert. */
    assert(map != 0x0);
    assert(map->size != 0);
    assert(map->count == 8u);
    assert(((float)map->count / (float)map->size) <= 0.5f);
    printf("correctly pushed a multiple (medium count) items into a map!\n");
};

/**
 * @brief test pushing multiple (large) items into a map.
 */
void
test_map_push_multiple_large_items(void) {
    /* arrange. */
    map_t* map = map_make();

    /* act. */
    for (int i = 0; i < 100; i++) {
        char* buffer = calloc(1u, 10u);
        snprintf(buffer, 10u, "%u", i);
        map_push(map, buffer, i);
    }

    /* assert. */
    assert(map != 0x0);
    assert(map->size != 0);
    assert(map->count == 100u);
    assert(((float)map->count / (float)map->size) <= 0.5f);
    printf("correctly pushed a multiple (large count) items into a map!\n");
};

/**
 * @brief test pushing multiple (massive) items into a map.
 */
void
test_map_push_multiple_massive_items(void) {
    /* arrange. */
    map_t* map = map_make();

    /* act. */
    for (int i = 0; i < 0x100; i++) {
        char* buffer = calloc(1u, 10u);
        snprintf(buffer, 10u, "%u", i);
        map_push(map, buffer, i);
    }

    /* assert. */
    assert(map != 0x0);
    assert(map->size != 0);
    assert(map->count == 0x100);
    assert(((float)map->count / (float)map->size) <= 0.5f);
    printf("correctly pushed a multiple (massive count) items into a map!\n");
};

/**
 * @brief test popping a single item from a map.
 */
void
test_map_pop_single_item(void) {
    /* arrange. */
    map_t* map = map_make();
    map_push(map, "1", 0x1234);

    /* act. */
    entry_t* entry = map_pop(map, "1");

    /* assert. */
    assert(entry != 0x0);
    assert(entry->value == 0x1234);
    assert(map->count == 0u);
    printf("correctly popped a single item from a map!\n");
};

/**
 * @brief test popping multiple (small) items from a map.
 */
void
test_map_pop_multiple_small_items(void) {
    /* arrange. */
    map_t* map = map_make();
    map_push(map, "1", 0x1234);
    map_push(map, "2", 0x5678);
    map_push(map, "3", 0x91011);

    /* act. */
    entry_t* entry1 = map_pop(map, "1");
    entry_t* entry2 = map_pop(map, "2");
    entry_t* entry3 = map_pop(map, "3");

    /* assert. */
    assert(entry1 != 0x0);
    assert(entry1->value == 0x1234);
    assert(entry2 != 0x0);
    assert(entry2->value == 0x5678);
    assert(entry3 != 0x0);
    assert(entry3->value == 0x91011);
    assert(map->count == 0u);
    printf("correctly popped a multiple (small count) items from a map!\n");
};

/**
 * @brief test popping multiple (medium) items from a map.
 */
void
test_map_pop_multiple_medium_items(void) {
    /* arrange. */
    map_t* map = map_make();
    map_push(map, "1", 0x1234);
    map_push(map, "2", 0x5678);
    map_push(map, "3", 0x91011);
    map_push(map, "4", 0x12113);
    map_push(map, "5", 0x960451);
    map_push(map, "6", 0x91011);
    map_push(map, "7", 0x11012);
    map_push(map, "8", 0x11);

    /* act. */
    entry_t* entry1 = map_pop(map, "1");
    entry_t* entry2 = map_pop(map, "2");
    entry_t* entry3 = map_pop(map, "3");
    entry_t* entry4 = map_pop(map, "4");
    entry_t* entry5 = map_pop(map, "5");
    entry_t* entry6 = map_pop(map, "6");
    entry_t* entry7 = map_pop(map, "7");
    entry_t* entry8 = map_pop(map, "8");

    /* assert. */
    assert(entry1 != 0x0);
    assert(entry2 != 0x0);
    assert(entry3 != 0x0);
    assert(entry4 != 0x0);
    assert(entry5 != 0x0);
    assert(entry6 != 0x0);
    assert(entry7 != 0x0);
    assert(entry8 != 0x0);
    assert(entry1->value == 0x1234);
    assert(entry2->value == 0x5678);
    assert(entry3->value == 0x91011);
    assert(entry4->value == 0x12113);
    assert(entry5->value == 0x960451);
    assert(entry6->value == 0x91011);
    assert(entry7->value == 0x11012);
    assert(entry8->value == 0x11);
    assert(map->count == 0u);
    printf("correctly popped a multiple (medium count) items from a map!\n");
};

/**
 * @brief test popping multiple (large) items from a map.
 */
void
test_map_pop_multiple_large_items(void) {
    /* arrange. */
    map_t* map = map_make();
    for (int i = 0; i < 100; i++) {
        char* buffer = calloc(1u, 10u);
        snprintf(buffer, 10u, "%u", i);
        map_push(map, buffer, i);
    }

    /* act & assert. */
    for (int i = 0; i < 100; i++) {
        char* buffer = calloc(1u, 10u);
        snprintf(buffer, 10u, "%u", i);
        entry_t* entry = map_pop(map, buffer);
        assert(entry != 0x0);
        assert(entry->value == i);
    }
    assert(map->count == 0u);
    printf("correctly popped a multiple (large count) items from a map!\n");
};

/**
 * @brief test popping multiple (massive) items from a map.
 */
void
test_map_pop_multiple_massive_items(void) {
    /* arrange. */
    map_t* map = map_make();
    for (int i = 0; i < 0x100; i++) {
        char* buffer = calloc(1u, 10u);
        snprintf(buffer, 10u, "%u", i);
        map_push(map, buffer, i);
    }

    /* act & assert. */
    for (int i = 0; i < 0x100; i++) {
        char* buffer = calloc(1u, 10u);
        snprintf(buffer, 10u, "%u", i);
        entry_t* entry = map_pop(map, buffer);
        assert(entry != 0x0);
        assert(entry->value == i);
    }
    assert(map->count == 0u);
    printf("correctly popped a multiple (massive count) items from a map!\n");
};

/**
 * @brief test looking up a single item from a map.
 */
void
test_map_lookup_single_item(void) {
    /* arrange. */
    map_t* map = map_make();
    map_push(map, "1", 0x1234);

    /* act. */
    entry_t* entry = map_lookup(map, "1");

    /* assert. */
    assert(entry != 0x0);
    assert(entry->value == 0x1234);
    assert(map->count == 1u);
    printf("correctly looked up a single item from a map!\n");
};

/**
 * @brief test looking up multiple items from a map.
 */
void
test_map_lookup_multiple_items(void) {
    /* arrange. */
    map_t* map = map_make();
    map_push(map, "1", 0x1234);
    map_push(map, "2", 0x5678);
    map_push(map, "3", 0x91011);
    map_push(map, "4", 0x12113);

    /* act. */
    entry_t* entry1 = map_lookup(map, "1");
    entry_t* entry2 = map_lookup(map, "2");
    entry_t* entry3 = map_lookup(map, "3");
    entry_t* entry4 = map_lookup(map, "4");

    /* assert. */
    assert(entry1 != 0x0);
    assert(entry2 != 0x0);
    assert(entry3 != 0x0);
    assert(entry4 != 0x0);
    assert(entry1->value == 0x1234);
    assert(entry2->value == 0x5678);
    assert(entry3->value == 0x91011);
    assert(entry4->value == 0x12113);
    assert(map->count == 4u);
    printf("correctly looked up multiple items from a map!\n");
};

/**
 * @brief test looking up a replaced item from a map.
 */
void
test_map_lookup_replaced_item(void) {
    /* arrange. */
    map_t* map = map_make();
    map_push(map, "1", 0x1234);
    map_push(map, "2", 0x5678);

    /* act & assert. */
    entry_t* entry1 = map_lookup(map, "1");
    entry_t* entry2 = map_lookup(map, "2");
    assert(entry1 != 0x0);
    assert(entry2 != 0x0);
    assert(entry1->value == 0x1234);
    assert(entry2->value == 0x5678);
    map_push(map, "1", 0x91011);
    map_push(map, "2", 0x1112301);
    entry_t* entry3 = map_lookup(map, "1");
    entry_t* entry4 = map_lookup(map, "2");
    assert(entry3 != 0x0);
    assert(entry4 != 0x0);
    assert(entry3->value == 0x91011);
    assert(entry4->value == 0x1112301);
    assert(map->count == 2u);
    printf("correctly looked up replaced items from a map!\n");
};

/**
 * @brief test looking up a non-existent item from a map.
 */
void
test_map_lookup_nonexistent_item(void) {
    /* arrange. */
    map_t* map = map_make();
    map_push(map, "1", 0x1234);
    map_push(map, "2", 0x5678);

    /* act & assert. */
    entry_t* entry = map_lookup(map, "3");
    assert(entry == 0x0);
    assert(map->count == 2u);
    printf("correctly looked up a non-existent item from a map!\n");
};

/* we use a singular main function per suite of testing done. */
#ifndef _WIN32
int main() {
#else
/*! for test_map. */
#include "test_map.h"

int test_map() {
#endif
    /* map_push tests. */
    printf("----src/int/map.c: 'push' unit tests----\n");
    test_map_push_single_item();
    test_map_push_multiple_small_items();
    test_map_push_multiple_medium_items();
    test_map_push_multiple_large_items();
    test_map_push_multiple_massive_items();

    /* map_pop tests. */
    printf("\n----src/int/map.c:  'pop'  unit tests----\n");
    test_map_pop_single_item();
    test_map_pop_multiple_small_items();
    test_map_pop_multiple_medium_items();
    test_map_pop_multiple_large_items();
    test_map_pop_multiple_massive_items();

    /* map_lookup tests. */
    printf("\n----src/int/map.c:  'lookup'  unit tests----\n");
    test_map_lookup_single_item();
    test_map_lookup_multiple_items();
    test_map_lookup_replaced_item();
    test_map_lookup_nonexistent_item();
    return 0;
}