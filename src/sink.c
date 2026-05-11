/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-03-09
 */
#include <tapi/sink.h>

/*! @uses calloc, free. */
#include <stdlib.h>

/*! @uses errno. */
#include <errno.h>
/** \endcond */

/**
 * @brief make a sink structure to be written to.
 *
 * @return a pointer to an allocated sink.
 */
tapi_sink_t*
tapi_sink_make() {
    /* allocate and return. */
    tapi_sink_t* sink = calloc(1u, sizeof *sink);
    return sink;
}

/**
 * @brief set a pre-allocated buffer to the destination of the sink.
 *
 * @param sink the sink to set the destination to.
 * @param length the size of the pre-allocated buffer.
 */
void
tapi_sink_setdbf(tapi_sink_t* sink, size_t length) {
    /* set the type and then the buffer. */
    sink->buffer.data = calloc(1u, length + 1u);
    sink->buffer.length = 0u;
    sink->buffer.capacity = length;
    sink->type = E_TAPI_SINK_TYPE_BUF;

    /* set the stream and then we are done. */
    tapi_stream_t stream = fmemopen(sink->buffer.data, sink->buffer.length, "w");
    if (stream == 0x0) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, sink_setdbf; fmemopen failed; could not open memory stream, was it "
                        "allocated? errno: %d\n", errno);
        return;
    }
    sink->stream = stream;
}

/**
 * @brief set a pre-allocated stream to the destination of the sink.
 *
 * @param sink the sink to set the destination to.
 * @param stream the pre-allocated or opened stream.
 */
void
tapi_sink_setdfp(tapi_sink_t* sink, tapi_stream_t stream) {
    /* set the type and then the stream, and we are done. */
    sink->stream = stream;
    sink->type = E_TAPI_SINK_TYPE_STR;
}

/**
 * @brief free a sink.
 *
 * @param sink the sink to be freed.
 */
void
tapi_sink_destroy(tapi_sink_t* sink) {
    if (sink->type == E_TAPI_SINK_TYPE_BUF)
        free(sink->buffer.data);
    free(sink);
};