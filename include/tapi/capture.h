/*
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-26
 */
#ifndef TAPI_CAPTURE_H
#define TAPI_CAPTURE_H

/*! uses size_t, FILE, tapi_sink_t, ... */
#include <tapi/sink.h>

/*! uses strcmp. */
#include <string.h>
/** \endcond */

/**
 * @brief a capture structure for redirecting written output data.
 *
 * `tapi_capture_t` is a data structure for capturing outputs for various file streams, but most
 *   often used on either stdout, and or stderr. this allows for quick capturing of what your
 *   tested function may print to the console and or any expected errors it should throw along
 *   the way.
 *
 * @see tapi_make_capture()
 * @see tapi_stop_capture()
 * @see tapi_cleanup_capture()
 */
typedef struct {
    /** the destination file descriptor (buffer or file). */
    int dst_fd;
    /** pipe read(0) & write(1) end for redirection. */
    int fds[2];
    /** sink to write data to. */
    tapi_sink_t* sink;
    /** the stream to capture. */
    tapi_stream_t stream;
} tapi_capture_t;

/**
 * @brief start capturing data written to a specific stream and re-route to a specified sink.
 *
 * @param sink the sink to capture the re-routed data (see @ref<tapi/sink.h>).
 * @param stream the stream to re-route data from.
 * @return a pointer to an allocated capture structure.
 */
TAPI_EXPORT tapi_capture_t*
tapi_make_capture(tapi_sink_t* sink, tapi_stream_t stream);

/**
 * @brief stop capturing data from a stream.
 *
 * @param capture the capture to be ended.
 */
TAPI_EXPORT void
tapi_stop_capture(tapi_capture_t* capture);

/**
 * @brief free a capture structure.
 *
 * @param capture the capture to be freed.
 */
TAPI_EXPORT void
tapi_cleanup_capture(tapi_capture_t* capture);

/** assert on a string stored within a sink. */
#define tapi_assert_sink(sink, string) \
    TAPI_ASSERT(!strcmp(sink->buffer.data, string));
#endif /* TAPI_CAPTURE_H */