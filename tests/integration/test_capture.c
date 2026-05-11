/**
 * @author Sean Hobeck
 * @date 2026-03-24
 */
#include <tapi/tapi.h>

/*! @uses tapi_capture_t, etc... */
#include <tapi/capture.h>

/*! @uses tapi_sink_t, etc... */
#include <tapi/sink.h>

/*! @uses printf. */
#include <stdio.h>

/*! @uses strcmp, strlen. */
#include <string.h>

/*! @uses calloc, free. */
#include <stdlib.h>

TAPI_MAKE_TEST(test_capture_empty) {
    /* arrange + act. */
    TAPI_CAPTURE(stdout, 16u);
    TAPI_END_CAPTURE();

    /* assert. */
    TAPI_ASSERT(strlen(sink->buffer.data) == 0);
    TAPI_DESTROY_CAPTURE();
    return E_TAPI_TEST_RESULT_PASSED;
}

TAPI_MAKE_TEST(test_capture_single_line) {
    /* arrange. */
    TAPI_CAPTURE(stdout, 16u);

    /* act. */
    printf("hello world");
    TAPI_END_CAPTURE();

    /* assert. */
    TAPI_ASSERT(strcmp(sink->buffer.data, "hello world") == 0);
    TAPI_DESTROY_CAPTURE();
    return E_TAPI_TEST_RESULT_PASSED;
}

TAPI_MAKE_TEST(test_capture_multiline) {
    /* arrange. */
    TAPI_CAPTURE(stdout, 32u);

    /* act. */
    printf("line 1\n");
    printf("line 2\n");
    printf("line 3\n");
    TAPI_END_CAPTURE();

    /* assert. */
    TAPI_ASSERT(strcmp(sink->buffer.data, "line 1\nline 2\nline 3\n") == 0);
    TAPI_DESTROY_CAPTURE();
    return E_TAPI_TEST_RESULT_PASSED;
}

TAPI_MAKE_TEST(test_capture_stderr) {
    /* arrange. */
    TAPI_CAPTURE(stderr, 32u);

    /* act. */
    fprintf(stderr, "error message");
    TAPI_END_CAPTURE();

    /* assert. */
    TAPI_ASSERT(strcmp(sink->buffer.data, "error message") == 0);
    TAPI_DESTROY_CAPTURE();
    return E_TAPI_TEST_RESULT_PASSED;
}

TAPI_MAKE_TEST(test_capture_both_streams) {
    /* arrange. */
    tapi_sink_t* sink1 = tapi_sink_make();
    tapi_sink_t* sink2 = tapi_sink_make();
    tapi_sink_setdbf(sink1, 256);
    tapi_sink_setdbf(sink2, 256);
    tapi_capture_t* cap1 = tapi_capture_make(sink1, stdout);
    tapi_capture_t* cap2 = tapi_capture_make(sink2, stderr);

    /* act. */
    printf("stdout output");
    fprintf(stderr, "stderr output");
    tapi_capture_end(cap1);
    tapi_capture_end(cap2);

    /* assert. */
    TAPI_ASSERT(strcmp(sink1->buffer.data, "stdout output") == 0);
    TAPI_ASSERT(strcmp(sink2->buffer.data, "stderr output") == 0);
    tapi_capture_destroy(cap1);
    tapi_capture_destroy(cap2);
    tapi_sink_destroy(sink1);
    tapi_sink_destroy(sink2);
    return E_TAPI_TEST_RESULT_PASSED;
}

TAPI_MAKE_TEST(test_capture_large_output) {
    /* arrange. */
    TAPI_CAPTURE(stdout, 1024u);

    for (int i = 0; i < 100; i++) {
        printf("0123456789");
    }
    TAPI_END_CAPTURE();

    /* assert. */
    TAPI_ASSERT(strlen(sink->buffer.data) == 1000);
    TAPI_DESTROY_CAPTURE();
    return E_TAPI_TEST_RESULT_PASSED;
}

TAPI_MAKE_TEST(test_capture_overflow) {
    /* arrange. */
    tapi_sink_t* small_sink = tapi_sink_make();
    tapi_sink_setdbf(small_sink, 16u);

    /* act. */
    tapi_capture_t* cap = tapi_capture_make(small_sink, stdout);
    printf("this is a very long string that exceeds the buffer");
    tapi_capture_end(cap);

    /* assert. */
    TAPI_ASSERT(strlen(small_sink->buffer.data) <= 16);
    tapi_capture_destroy(cap);
    tapi_sink_destroy(small_sink);
    return E_TAPI_TEST_RESULT_PASSED;
}

TAPI_MAKE_TEST(test_capture_formats) {
    /* arrange. */
    TAPI_CAPTURE(stdout, 128u);

    /* act. */
    printf("int: %d, float: %.2f, str: %s, hex: 0x%x",
           42, 3.14f, "hello", 0xDEADBEEF);
    TAPI_END_CAPTURE();

    /* assert. */
    TAPI_ASSERT(strstr(sink->buffer.data, "int: 42") != 0x0);
    TAPI_ASSERT(strstr(sink->buffer.data, "float: 3.14") != 0x0);
    TAPI_ASSERT(strstr(sink->buffer.data, "str: hello") != 0x0);
    TAPI_ASSERT(strstr(sink->buffer.data, "hex: 0xdeadbeef") != 0x0);
    TAPI_DESTROY_CAPTURE();
    return E_TAPI_TEST_RESULT_PASSED;
}

TAPI_MAKE_TEST(test_capture_special_chars) {
    /* arrange. */
    TAPI_CAPTURE(stdout, 64u);

    /* act. */
    printf("tab:\t newline:\n null: quote:\"");
    TAPI_END_CAPTURE();

    /* assert. */
    TAPI_ASSERT(strstr(sink->buffer.data, "tab:\t") != 0x0);
    TAPI_ASSERT(strstr(sink->buffer.data, "newline:\n") != 0x0);
    TAPI_DESTROY_CAPTURE();
    return E_TAPI_TEST_RESULT_PASSED;
}

TAPI_MAKE_TEST(test_capture_whitespace) {
    /* arrange. */
    TAPI_CAPTURE(stdout, 32u);

    /* act. */
    printf("   \n\t\n   ");
    TAPI_END_CAPTURE();

    /* assert. */
    TAPI_ASSERT(strcmp(sink->buffer.data, "   \n\t\n   ") == 0x0);
    TAPI_DESTROY_CAPTURE();
    return E_TAPI_TEST_RESULT_PASSED;
}

int main() {
    TAPI_ADD_TEST("test_empty", test_capture_empty);
    TAPI_ADD_TEST("test_single_line", test_capture_single_line);
    TAPI_ADD_TEST("test_multiline", test_capture_multiline);
    TAPI_ADD_TEST("test_stderr", test_capture_stderr);
    TAPI_ADD_TEST("test_both_streams", test_capture_both_streams);
    TAPI_ADD_TEST("test_large_output", test_capture_large_output);
    TAPI_ADD_TEST("test_overflow", test_capture_overflow);
    TAPI_ADD_TEST("test_formats", test_capture_formats);
    TAPI_ADD_TEST("test_special_chars", test_capture_special_chars);
    TAPI_ADD_TEST("test_whitespace", test_capture_whitespace);
    TAPI_RUN_TESTS();
    return 0;
}