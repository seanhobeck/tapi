/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-07-28
 */
#include <tapi/capture.h>

/*! uses calloc, close. */
#include <stdlib.h>

#ifndef _WIN32
/*! uses pipe, dup, dup2. */
#include <unistd.h>
#else
/*! uses crt _dup, _dup2, _fileno. */
#include <io.h>

/*! uses _O_BINARY. */
#include <fcntl.h>

/* i shouldn't have to do this. */
typedef ptrdiff_t ssize_t;
#endif

/*! uses errno. */
#include <errno.h>

/*! uses strncpy. */
#include <string.h>
/** \endcond */

/**
 * @brief start capturing data written to a specific stream and re-route to a specified sink.
 *
 * @param sink the sink to capture the re-routed data (see @ref<tapi/sink.h>).
 * @param stream the stream to re-route data from.
 * @return a pointer to an allocated capture structure.
 */
TAPI_HIDDEN tapi_capture_t*
tapi_make_capture(tapi_sink_t* sink, tapi_stream_t stream) {
    /* allocate the structure. */
    tapi_capture_t* capture = calloc(1u, sizeof *capture);
    capture->sink = sink;
    capture->stream = stream;

    /* open the streams. */
#ifndef _WIN32
    if (pipe(capture->fds) == -1) {
#else
    if (_pipe(capture->fds, 4096u, _O_BINARY) == -1) {
#endif
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi_make_capture; pipe failed; could not create pipe for stdout and stderr.\n");
        exit(EXIT_FAILURE);
    }

    /* flush the stream. */
    fflush(stream);
#ifndef _WIN32
    capture->dst_fd = dup(fileno(stream));
#else
    capture->dst_fd = _dup(_fileno(stream));
#endif
    if (capture->dst_fd == -1) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi_make_capture; pipe failed; could not create pipe for stdout and "
                        "stderr.\n");
        exit(EXIT_FAILURE);
    }
#ifndef _WIN32
    if (dup2(capture->fds[1], fileno(stream)) == -1) {
#else
    if (_dup2(capture->fds[1], _fileno(stream)) == -1) {
#endif
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi_make_capture; dup2 failed; could not copy over pipe_wr fd to stdout"
                        ".\n");
        exit(EXIT_FAILURE);
    }

    /* pass the pipe write end, close the copy. */
#ifndef _WIN32
    close(capture->fds[1]);
#else
    _close(capture->fds[1]);
#endif
    capture->fds[1] = -1;
    setvbuf(stream, 0x0, _IONBF, 0);
    return capture;
}

/**
 * @brief stop capturing data from a stream.
 *
 * @param capture the capture to be ended.
 */
void
tapi_stop_capture(tapi_capture_t* capture) {
    /* flush the stream which we capture from. */
    fflush(capture->stream);
#ifndef _WIN32
    if (dup2(capture->dst_fd, fileno(capture->stream)) == -1) {
#else
    if (_dup2(capture->dst_fd, _fileno(capture->stream)) == -1) {
#endif
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi_end_capture; dup2 failed; could not copy saved fd over to stdout. "
                        "errno: %d\n", errno);
        return;
    }
#ifndef _WIN32
    close(capture->dst_fd);
#else
    _close(capture->dst_fd);
#endif
    capture->dst_fd = -1;

    /* read all data from pipe, and write it to sink. */
    char buf[4096u + 1u];

    /* close writer side so pipe will hit EOF. */
    ssize_t n;
#ifndef _WIN32
    while ((n = read(capture->fds[0], buf, 4096u)) > 0l) {
#else
    while ((n = _read(capture->fds[0], buf, 4096u)) > 0l) {
#endif
        /* all we do is simply write the data to the stream, if it fails then we stop. */
        buf[n] = 0x0;
        if (capture->sink->type == E_TAPI_SINK_TYPE_BUF) {
            ssize_t remaining = (ssize_t) (capture->sink->buffer.capacity - capture->sink->buffer.length);
            if (remaining > 0) {
                memcpy(capture->sink->buffer.data + capture->sink->buffer.length, buf, remaining);
                capture->sink->buffer.length += remaining;
            }
            else break;
        }
        else fprintf(capture->sink->stream, "%s", buf);
    }

    /* close our read pipe since we are done. */
#ifndef _WIN32
    close(capture->fds[0]);
#else
    _close(capture->fds[0]);
#endif
    capture->fds[0] = -1;
    if (n == -1) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi_end_capture; read failed; could not read from stdout pipe. errno: "
                        "%d\n", errno);
    }
}

/**
 * @brief free a capture structure.
 *
 * @param capture the capture to be freed.
 */
void
tapi_cleanup_capture(tapi_capture_t* capture) {
    free(capture);
}