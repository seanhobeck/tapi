/*
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-16
 */
#ifndef TAPI_SINK_H
#define TAPI_SINK_H

/* we have to define this to use fileno(), as stream->_fileno is not universal across differing libc. */
#define _GNU_SOURCE 200809L

/*! @uses TAPI_EXPORT. */
#include <tapi/tapi.h>

/*! @uses FILE, struct _IO_FILE. */
#include <stdio.h>
/** \endcond */

/** a type definition for a POSIX compliant pointer to a memory/ file stream. */
typedef FILE* tapi_stream_t;

/**
 * @brief a sink for funneling data into a single stream.
 *
 * `tapi_sink_t` is a data structure responsible for transporting captured data from a specified
 *   stream into another pre-allocated user stream. sinks are used in two types, log file streams
 *   (think something like output_test_####.log), and memory streams like buffers.
 *
 * @see tapi_make_sink()
 * @see tapi_setdbf_sink()
 * @see tapi_setdfp_sink()
 * @see tapi_cleanup_sink()
 */
typedef struct {
    /** a pre-allocated buffer. */
    struct {
        /* a pointer to data and length. */
        char* data; size_t length, capacity;
    } buffer;
    /** enum for the type of sink. */
    enum {
        /** no dest. set. */
        E_TAPI_SINK_TYPE_NONE = 0x0,
        /** a pre-allocated buffer. */
        E_TAPI_SINK_TYPE_BUF = 0x1,
        /** a pre-allocated stream. */
        E_TAPI_SINK_TYPE_STR = 0x2,
    } type;
    /** a pointer to a stream. */
    tapi_stream_t stream;
} tapi_sink_t;

/**
 * @brief make a sink structure to be written to.
 *
 * @return a pointer to an allocated sink.
 */
TAPI_EXPORT tapi_sink_t*
tapi_make_sink();

/**
 * @brief set a pre-allocated buffer to the destination of the sink.
 *
 * @param sink the sink to set the destination to.
 * @param length the size of the pre-allocated buffer.
 */
TAPI_EXPORT void
tapi_setdbf_sink(tapi_sink_t* sink, size_t length);

/**
 * @brief set a pre-allocated stream to the destination of the sink.
 *
 * @param sink the sink to set the destination to.
 * @param stream the pre-allocated or opened stream.
 */
TAPI_EXPORT void
tapi_setdfp_sink(tapi_sink_t* sink, tapi_stream_t stream);

/**
 * @brief free a sink.
 *
 * @param sink the sink to be freed.
 */
TAPI_EXPORT void
tapi_cleanup_sink(tapi_sink_t* sink);
#endif /* TAPI_SINK_H */