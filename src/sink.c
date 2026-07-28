/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-07-27
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

/**
 * @brief open a memory stream for a buffer.
 * 
 * @param buffer the buffer to open a memory stream to.
 * @param length the length of the memory buffer.
 * @param mode the mode in which the stream should be opened.
 * @return an open stream if successful, 0x0 o.w.
 */
tapi_stream_t 
fmemopen(void* buffer, const size_t length, const char* mode) {
    char tmp[MAX_PATH - 13u], name[MAX_PATH + 1u];
    int retval = -1, fd = 0u;
    FILE* fptr = 0x0;
    
    /* get data for a temporary path. */
    if (!GetTempPathA(MAX_PATH - 13u, tmp)) return 0x0;
    if (!GetTempFileNameA(tmp, "MemTF_", 0, name)) return 0x0;
    
    /* open the temporary path and get the FILE*. */
    retval = _sopen_s(&fd, name, _O_CREAT | _O_SHORT_LIVED | _O_TEMPORARY | _O_RDWR | \
        _O_BINARY | O_NOINHERIT, _SH_DENYRW, _S_IREAD | _S_IWRITE);
    if (retval != 0 || fd == -1) return 0x0;

    /* attempt to open the file descriptor and write to it then rewind. */
    fptr = _fdopen(fd, "wb+");
    if (!fptr) {
        _close(fd);
        return 0x0;
    }
    fwrite(buffer, length, 1u, fptr);
    rewind(fptr);
    return fptr;
};
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