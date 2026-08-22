/**
 * @author Sean Hobeck
 * @date 2026-08-03
 */
#include <tapi/tapi.h>

/*! uses tapi_stub_return. */
#include <tapi/mock.h>

/*! noinline macro. */
#ifdef _WIN32
#define TEST_NOINLINE __declspec(noinline)
#else
#define TEST_NOINLINE __attribute__((noinline))
#endif

/*! -----------------tested functions------------------ !*/

TEST_NOINLINE int 
target_function(int x) {
    return ~x + 1u;
}

TEST_NOINLINE int 
function() {
    int result1 = 0x10;
    int result2 = -0x1;
    int result3 = result1 + result2;
    int result4 = target_function(result3);
    return result4 + 1u;
}

#if defined(__x86_64__) || defined(__i386__) || defined(_M_AMD64) || defined(_M_IX86)
TEST_NOINLINE int 
asm_target_x86(int x) {
    int result = 0;
#ifndef _WIN32
    __asm__ volatile(
        "movl %1, %0\n"
        "negl %0\n"
        : "=r"(result)
        : "r"(x)
        : "cc"
    );
    return result;
#else
    return -x;
#endif
}

TEST_NOINLINE int 
asm_caller_x86() {
    int val = 0x42;
    /* direct call to test relative call detection and patching. */
    int result = asm_target_x86(val);
    return result + 1;
}
#elif defined(__aarch64__) || defined(_M_ARM64)
TEST_NOINLINE int 
asm_target_aarch64(int x) {
    int result;
#ifndef _WIN32
    __asm__ volatile(
        "neg %w0, %w1\n"
        : "=r"(result)
        : "r"(x)
    );
#else
#endif
    return result;
}

TEST_NOINLINE int 
asm_caller_aarch64() {
    int val = 0x42;
    int result = asm_target_aarch64(val);
    return result + 1;
}
#elif defined(__arm__) || defined(_M_ARM)
TEST_NOINLINE int 
asm_target_arm32(int x) {
    int result;
    __asm__ volatile(
        "rsb %0, %1, #0\n"
        : "=r"(result)
        : "r"(x)
    );
    return result;
}

TEST_NOINLINE int 
asm_caller_arm32() {
    int val = 0x42;
    int result = asm_target_arm32(val);
    return result + 1;
}


__attribute__((target("thumb")))
TEST_NOINLINE int 
asm_target_thumb(int x) {
    int result;
    __asm__ volatile(
        "rsb %0, %1, #0\n"
        : "=r"(result)
        : "r"(x)
    );
    return result;
}

__attribute__((target("thumb")))
TEST_NOINLINE int 
asm_caller_thumb() {
    int val = 0x42;
    int result = asm_target_thumb(val);
    return result + 1;
}
#endif

TEST_NOINLINE int 
nested_target(int x) {
    return x * 2;
}

TEST_NOINLINE int 
nested_middle(int x) {
    return nested_target(x) + 5;
}

TEST_NOINLINE int 
nested_caller() {
    return nested_middle(10);
}

TEST_NOINLINE int 
conditional_target(int x) {
    return x + 100;
}

TEST_NOINLINE int 
conditional_caller(int use_target) {
    if (use_target) {
        return conditional_target(50);
    }
    return 0;
}

/*! -----------------mock return values------------------ !*/

#pragma region mock return values
tapi_stub_return(tested_function_target, int, 0u);
#if defined(__x86_64__) || defined(__i386__)  || defined(_M_AMD64) || defined(_M_IX86)
tapi_stub_return(mock_asm_target_x86, int, 0x100);
#elif defined(__aarch64__) || defined(_M_ARM64)
tapi_stub_return(mock_asm_target_aarch64, int, 0x100);
#elif defined(__arm__) || defined(_M_ARM)
tapi_stub_return(mock_asm_target_arm32, int, 0x100);
tapi_stub_return(mock_asm_target_thumb, int, 0x100);
#endif
tapi_stub_return(mock_nested_target, int, 42);
tapi_stub_return(mock_conditional_target, int, 999);
#pragma endregion

/*! -----------------------tests--------------------- !*/

tapi_test(test_basic_mock){
    /* act & assert. */
    int result = function();
    tapi_assert(result == 1u);
    return E_TAPI_TEST_RESULT_PASSED;
}

#if defined(__x86_64__) || defined(__i386__) || defined(_M_AMD64) || defined(_M_IX86)
tapi_test(test_asm_x86_mock){
    /* act & assert. */
    int result = asm_caller_x86();
    tapi_assert(result == 0x101);
    return E_TAPI_TEST_RESULT_PASSED;
}
#elif defined(__aarch64__) || defined(_M_ARM64)
tapi_test(test_asm_aarch64_mock){
    /* act & assert. */
    int result = asm_caller_aarch64();
    tapi_assert(result == 0x101);
    return E_TAPI_TEST_RESULT_PASSED;
}
#elif defined(__arm__) || defined(_M_ARM)
tapi_test(test_asm_arm32_mock){
    /* act & assert. */
    int result = asm_caller_arm32();
    tapi_assert(result == 0x101);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_asm_thumb_mock){
    /* act & assert. */
    int result = asm_caller_thumb();
    tapi_assert(result == 0x101);
    return E_TAPI_TEST_RESULT_PASSED;
}
#endif

tapi_test(test_nested_mock){
    /* act & assert. */
    int result = nested_caller();
    tapi_assert(result == 47);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_conditional_mock){
    /* act & assert. */
    int result = conditional_caller(1);
    tapi_assert(result == 999);
    return E_TAPI_TEST_RESULT_PASSED;
}

/*! ----------------------==---------------------- !*/

#ifndef _WIN32
int main() {
#else
int test_mock() {
#endif
    /* basic test. */
    tapi_context_t* context = tapi_init();
    tapi_add_test_and_mock(context, "test_basic_mock", test_basic_mock, function, \
        target_function, tested_function_target);

    /* architecture-specific tests. */
#if defined(__x86_64__) || defined(__i386__) || defined(_M_AMD64) || defined(_M_IX86)
    tapi_add_test_and_mock(context, "test_asm_x86_mock", test_asm_x86_mock, asm_caller_x86, \
        asm_target_x86, mock_asm_target_x86);
#elif defined(__aarch64__) || defined(_M_ARM64)
    tapi_add_test_and_mock(context, "test_asm_aarch64_mock", test_asm_aarch64_mock, \
        asm_caller_aarch64, asm_target_aarch64, mock_asm_target_aarch64);
#elif defined(__arm__) || defined(_M_ARM)
    tapi_add_test_and_mock(context, "test_asm_arm32_mock", test_asm_arm32_mock, \
        asm_caller_arm32, asm_target_arm32, mock_asm_target_arm32);
    tapi_add_test_and_mock(context, "test_asm_thumb_mock", test_asm_thumb_mock, \
        asm_caller_thumb, asm_target_thumb, mock_asm_target_thumb);
#endif
    tapi_add_test_and_mock(context, "test_nested_mock", test_nested_mock, nested_middle, \
        nested_target, mock_nested_target);
    tapi_add_test_and_mock(context, "test_conditional", test_conditional_mock, conditional_caller, \
        conditional_target, mock_conditional_target);

    /* setup test array and run. */
    tapi_run_context(context);
    return 0;
}