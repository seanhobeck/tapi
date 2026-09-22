/**
 * @author Sean Hobeck
 * @date 2026-09-21
 */
#include "test_posix.h"
#include "test_windows.h"

int main(void) {
    test_posix();
    test_windows();
    return 0;
}