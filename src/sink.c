/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-08-04
 */
#include <tapi/sink.h>

/*! uses calloc, free. */
#include <stdlib.h>

/*! uses errno. */
#include <errno.h>
/** \endcond */
#ifdef _WIN32
/*! uses _sopen_s, rewind etc.. */
#include <io.h>

/*! uses _O_CREAT, etc... */
#include <fcntl.h>

/*! uses _S_IREAD, _S_IWRITE. */
#include <sys/stat.h>
#endif

/**
 * @brief make a sink structure to be written to.
 *
 * @return a pointer to an allocated sink.
 */
tapi_sink_t*
tapi_make_sink() {
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
tapi_setdbf_sink(tapi_sink_t* sink, size_t length) {
    /* set the type and then the buffer. */
    sink->buffer.data = calloc(1u, length + 1u);
    sink->buffer.length = 0u;
    sink->buffer.capacity = length;
    sink->type = E_TAPI_SINK_TYPE_BUF;
    sink->stream = 0x0; /* we don't even use a memory stream, just straight read into the buffer. */
}

/**
 * @brief set a pre-allocated stream to the destination of the sink.
 *
 * @param sink the sink to set the destination to.
 * @param stream the pre-allocated or opened stream.
 */
void
tapi_setdfp_sink(tapi_sink_t* sink, tapi_stream_t stream) {
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
tapi_cleanup_sink(tapi_sink_t* sink) {
    if (sink->type == E_TAPI_SINK_TYPE_BUF)
        free(sink->buffer.data);
    free(sink);
};