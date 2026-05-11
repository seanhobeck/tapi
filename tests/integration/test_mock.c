/**
 * @author Sean Hobeck
 * @date 2026-03-24
 */
#include <tapi/tapi.h>

/*! @uses TAPI_MOCK_RETURN. */
#include <tapi/mock.h>

/* region for all of the test call targets and assembly specific fuctions. */
#pragma region test call targets
int target_function(int x) {
    return ~x + 1u;
}

int function() {
    int result1 = 0x10;
    int result2 = -0x1;
    int result3 = result1 + result2;
    int result4 = target_function(result3);
    return result4 + 1u;
}

#if defined(__x86_64__) || defined(__i386__)
int asm_target_x86(int x) {
    int result;
    __asm__ volatile(
        "movl %1, %0\n"
        "negl %0\n"
        : "=r"(result)
        : "r"(x)
        : "cc"
    );
    return result;
}

int asm_caller_x86() {
    int val = 0x42;
    /* direct call to test relative call detection and patching. */
    int result = asm_target_x86(val);
    return result + 1;
}
#elif defined(__aarch64__)
int asm_target_aarch64(int x) {
    int result;
    __asm__ volatile(
        "neg %w0, %w1\n"
        : "=r"(result)
        : "r"(x)
    );
    return result;
}

int asm_caller_aarch64() {
    int val = 0x42;
    int result = asm_target_aarch64(val);
    return result + 1;
}
#elif defined(__arm__)
int asm_target_arm32(int x) {
    int result;
    __asm__ volatile(
        "rsb %0, %1, #0\n"
        : "=r"(result)
        : "r"(x)
    );
    return result;
}

int asm_caller_arm32() {
    int val = 0x42;
    int result = asm_target_arm32(val);
    return result + 1;
}

__attribute__((target("thumb")))
int asm_target_thumb(int x) {
    int result;
    __asm__ volatile(
        "rsb %0, %1, #0\n"
        : "=r"(result)
        : "r"(x)
    );
    return result;
}

__attribute__((target("thumb")))
int asm_caller_thumb() {
    int val = 0x42;
    int result = asm_target_thumb(val);
    return result + 1;
}
#endif

int nested_target(int x) {
    return x * 2;
}

int nested_middle(int x) {
    return nested_target(x) + 5;
}

int nested_caller() {
    return nested_middle(10);
}

int conditional_target(int x) {
    return x + 100;
}

int conditional_caller(int use_target) {
    if (use_target) {
        return conditional_target(50);
    }
    return 0;
}
#pragma endregion

/* region for all of the setups and mock return values. */
#pragma region mock return values
TAPI_MOCK_RETURN(tested_function_target, int, 0u);
#if defined(__x86_64__) || defined(__i386__)
TAPI_MOCK_RETURN(mock_asm_target_x86, int, 0x100);
#elif defined(__aarch64__)
TAPI_MOCK_RETURN(mock_asm_target_aarch64, int, 0x100);
#elif defined(__arm__)
TAPI_MOCK_RETURN(mock_asm_target_arm32, int, 0x100);
TAPI_MOCK_RETURN(mock_asm_target_thumb, int, 0x100);
#endif
TAPI_MOCK_RETURN(mock_nested_target, int, 42);
TAPI_MOCK_RETURN(mock_conditional_target, int, 999);
#pragma endregion

/* region for all of the tests. */
#pragma region tests
TAPI_MAKE_TEST(test_basic_mock){
    /* act & assert. */
    int result = function();
    TAPI_ASSERT(result == 1u);
    return E_TAPI_TEST_RESULT_PASSED;
}

#if defined(__x86_64__) || defined(__i386__)
TAPI_MAKE_TEST(test_asm_x86_mock){
    /* act & assert. */
    int result = asm_caller_x86();
    TAPI_ASSERT(result == 0x101);
    return E_TAPI_TEST_RESULT_PASSED;
}
#elif defined(__aarch64__)
TAPI_MAKE_TEST(test_asm_aarch64_mock){
    /* act & assert. */
    int result = asm_caller_aarch64();
    TAPI_ASSERT(result == 0x101);
    return E_TAPI_TEST_RESULT_PASSED;
}
#elif defined(__arm__)
TAPI_MAKE_TEST(test_asm_arm32_mock){
    /* act & assert. */
    int result = asm_caller_arm32();
    TAPI_ASSERT(result == 0x101);
    return E_TAPI_TEST_RESULT_PASSED;
}

TAPI_MAKE_TEST(test_asm_thumb_mock){
    /* act & assert. */
    int result = asm_caller_thumb();
    TAPI_ASSERT(result == 0x101);
    return E_TAPI_TEST_RESULT_PASSED;
}
#endif

TAPI_MAKE_TEST(test_nested_mock){
    /* act & assert. */
    int result = nested_caller();
    TAPI_ASSERT(result == 47);
    return E_TAPI_TEST_RESULT_PASSED;
}

TAPI_MAKE_TEST(test_conditional_mock){
    /* act & assert. */
    int result = conditional_caller(1);
    TAPI_ASSERT(result == 999);
    return E_TAPI_TEST_RESULT_PASSED;
}
#pragma endregion

int main(){
    /* basic test. */
    TAPI_ADD_TEST_AND_MOCK("test_basic_mock", test_basic_mock, function, \
        target_function, tested_function_target);

    /* architecture-specific tests. */
#if defined(__x86_64__) || defined(__i386__)
    TAPI_ADD_TEST_AND_MOCK("test_asm_x86_mock", test_asm_x86_mock, asm_caller_x86, \
        asm_target_x86, mock_asm_target_x86);
#elif defined(__aarch64__)
    TAPI_ADD_TEST_AND_MOCK("test_asm_aarch64_mock", test_asm_aarch64_mock, \
        asm_caller_aarch64, asm_target_aarch64, mock_asm_target_aarch64);
#elif defined(__arm__)
    TAPI_ADD_TEST_AND_MOCK("test_asm_arm32_mock", test_asm_arm32_mock, \
        asm_caller_arm32, asm_target_arm32, mock_asm_target_arm32);
    TAPI_ADD_TEST_AND_MOCK("test_asm_thumb_mock", test_asm_thumb_mock, \
        asm_caller_thumb, asm_target_thumb, mock_asm_target_thumb);
#endif
    TAPI_ADD_TEST_AND_MOCK("test_nested_mock", test_nested_mock, nested_middle, \
        nested_target, mock_nested_target);
    TAPI_ADD_TEST_AND_MOCK("test_conditional", test_conditional_mock, conditional_caller, \
        conditional_target, mock_conditional_target);

    /* setup test array and run. */
    TAPI_RUN_TESTS();
    return 0;
}