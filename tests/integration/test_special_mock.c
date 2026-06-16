/**
 * @author Sean Hobeck
 * @date 2026-06-16
 */
#include <tapi/tapi.h>

/*! @uses TAPI_ADD_TEST_AND_MOCK, etc... */
#include <tapi/mock.h>

/*! @uses free, calloc, malloc. */
#include <stdlib.h>

/*! @uses snprintf. */
#include <stdio.h>

/* region for all tested functions. */
#pragma region
void
target_function(char* string) {
    char* buffer = malloc(100u);
    snprintf(buffer, 100u, "%s", string);
    printf("%s\n", buffer);
    free(buffer);
}
#pragma endregion

int main() {
    tapi_context_t* context = tapi_init();
    tapi_test_t* test1 = tapi_make_test("test_malloc", target_function);
    tapi_test_add_mock(context, test1, )
    TAPI_RUN_TESTS();
    return 0;
}