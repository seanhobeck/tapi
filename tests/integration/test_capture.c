/**
 * @author Sean Hobeck
 * @date 2026-06-26
 */
#include <tapi/tapi.h>

/*! uses tapi_capture_t, etc... */
#include <tapi/capture.h>

/*! uses tapi_sink_t, etc... */
#include <tapi/sink.h>

/*! uses printf. */
#include <stdio.h>

/*! uses strcmp, strlen. */
#include <string.h>

/*! uses calloc, free. */
#include <stdlib.h>

tapi_test(test_capture_empty) {
    /* arrange + act. */
    tapi_sink_t* sink = tapi_make_sink();
    tapi_setdbf_sink(sink, 16u);
    tapi_capture_t* capture = tapi_make_capture(sink, stdout);
    tapi_stop_capture(capture);

    /* assert. */
    tapi_assert(strlen(sink->buffer.data) == 0);
    tapi_cleanup_capture(capture);
    tapi_cleanup_sink(sink);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_capture_single_line) {
    /* arrange. */
    tapi_sink_t* sink = tapi_make_sink();
    tapi_setdbf_sink(sink, 16u);
    tapi_capture_t* capture = tapi_make_capture(sink, stdout);

    /* act. */
    printf("hello world");
    tapi_stop_capture(capture);

    /* assert. */
    tapi_assert(strcmp(sink->buffer.data, "hello world") == 0);
    tapi_cleanup_capture(capture);
    tapi_cleanup_sink(sink);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_capture_multiline) {
    /* arrange. */
    tapi_sink_t* sink = tapi_make_sink();
    tapi_setdbf_sink(sink, 32u);
    tapi_capture_t* capture = tapi_make_capture(sink, stdout);

    /* act. */
    printf("line 1\n");
    printf("line 2\n");
    printf("line 3\n");
    tapi_stop_capture(capture);

    /* assert. */
    tapi_assert(strcmp(sink->buffer.data, "line 1\nline 2\nline 3\n") == 0);
    tapi_cleanup_capture(capture);
    tapi_cleanup_sink(sink);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_capture_stderr) {
    /* arrange. */
    tapi_sink_t* sink = tapi_make_sink();
    tapi_setdbf_sink(sink, 32u);
    tapi_capture_t* capture = tapi_make_capture(sink, stderr);

    /* act. */
    fprintf(stderr, "error message");
    tapi_stop_capture(capture);

    /* assert. */
    tapi_assert(strcmp(sink->buffer.data, "error message") == 0);
    tapi_cleanup_capture(capture);
    tapi_cleanup_sink(sink);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_capture_both_streams) {
    /* arrange. */
    tapi_sink_t* sink1 = tapi_make_sink();
    tapi_sink_t* sink2 = tapi_make_sink();
    tapi_setdbf_sink(sink1, 256);
    tapi_setdbf_sink(sink2, 256);
    tapi_capture_t* cap1 = tapi_make_capture(sink1, stdout);
    tapi_capture_t* cap2 = tapi_make_capture(sink2, stderr);

    /* act. */
    printf("stdout output");
    fprintf(stderr, "stderr output");
    tapi_stop_capture(cap1);
    tapi_stop_capture(cap2);

    /* assert. */
    tapi_assert(strcmp(sink1->buffer.data, "stdout output") == 0);
    tapi_assert(strcmp(sink2->buffer.data, "stderr output") == 0);
    tapi_cleanup_capture(cap1);
    tapi_cleanup_capture(cap2);
    tapi_cleanup_sink(sink1);
    tapi_cleanup_sink(sink2);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_capture_large_output) {
    /* arrange. */
    tapi_sink_t* sink = tapi_make_sink();
    tapi_setdbf_sink(sink, 1024u);
    tapi_capture_t* capture = tapi_make_capture(sink, stdout);

    for (int i = 0; i < 100; i++) {
        printf("0123456789");
    }
    tapi_stop_capture(capture);

    /* assert. */
    tapi_assert(strlen(sink->buffer.data) == 1000);
    tapi_cleanup_capture(capture);
    tapi_cleanup_sink(sink);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_capture_overflow) {
    /* arrange. */
    tapi_sink_t* small_sink = tapi_make_sink();
    tapi_setdbf_sink(small_sink, 16u);

    /* act. */
    tapi_capture_t* cap = tapi_make_capture(small_sink, stdout);
    printf("this is a very long string that exceeds the buffer");
    tapi_stop_capture(cap);

    /* assert. */
    tapi_assert(strlen(small_sink->buffer.data) <= 16);
    tapi_cleanup_capture(cap);
    tapi_cleanup_sink(small_sink);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_capture_formats) {
    /* arrange. */
    tapi_sink_t* sink = tapi_make_sink();
    tapi_setdbf_sink(sink, 128u);
    tapi_capture_t* capture = tapi_make_capture(sink, stdout);

    /* act. */
    printf("int: %d, float: %.2f, str: %s, hex: 0x%x",
           42, 3.14f, "hello", 0xDEADBEEF);
    tapi_stop_capture(capture);

    /* assert. */
    tapi_assert(strstr(sink->buffer.data, "int: 42") != 0x0);
    tapi_assert(strstr(sink->buffer.data, "float: 3.14") != 0x0);
    tapi_assert(strstr(sink->buffer.data, "str: hello") != 0x0);
    tapi_assert(strstr(sink->buffer.data, "hex: 0xdeadbeef") != 0x0);
    tapi_cleanup_capture(capture);
    tapi_cleanup_sink(sink);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_capture_special_chars) {
    /* arrange. */
    tapi_sink_t* sink = tapi_make_sink();
    tapi_setdbf_sink(sink, 64u);
    tapi_capture_t* capture = tapi_make_capture(sink, stdout);

    /* act. */
    printf("tab:\t newline:\n null: quote:\"");
    tapi_stop_capture(capture);

    /* assert. */
    tapi_assert(strstr(sink->buffer.data, "tab:\t") != 0x0);
    tapi_assert(strstr(sink->buffer.data, "newline:\n") != 0x0);
    tapi_cleanup_capture(capture);
    tapi_cleanup_sink(sink);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_capture_whitespace) {
    /* arrange. */
    tapi_sink_t* sink = tapi_make_sink();
    tapi_setdbf_sink(sink, 32u);
    tapi_capture_t* capture = tapi_make_capture(sink, stdout);

    /* act. */
    printf("   \n\t\n   ");
    tapi_stop_capture(capture);

    /* assert. */
    tapi_assert(strcmp(sink->buffer.data, "   \n\t\n   ") == 0x0);
    tapi_cleanup_capture(capture);
    tapi_cleanup_sink(sink);
    return E_TAPI_TEST_RESULT_PASSED;
}

int main() {
    tapi_context_t* context = tapi_init();
    tapi_add_test(context, "test_empty", test_capture_empty);
    tapi_add_test(context, "test_single_line", test_capture_single_line);
    tapi_add_test(context, "test_multiline", test_capture_multiline);
    tapi_add_test(context, "test_stderr", test_capture_stderr);
    tapi_add_test(context, "test_both_streams", test_capture_both_streams);
    tapi_add_test(context, "test_large_output", test_capture_large_output);
    tapi_add_test(context, "test_overflow", test_capture_overflow);
    tapi_add_test(context, "test_formats", test_capture_formats);
    tapi_add_test(context, "test_special_chars", test_capture_special_chars);
    tapi_add_test(context, "test_whitespace", test_capture_whitespace);
    tapi_run_context(context);
    return 0;
}