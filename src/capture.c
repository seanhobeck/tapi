/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-18
 */
#include <tapi/capture.h>

/*! @uses calloc, close. */
#include <stdlib.h>

/*! @uses pipe, dup, dup2. */
#include <unistd.h>

/*! @uses errno. */
#include <errno.h>

/*! @uses strncpy. */
#include <string.h>
/** \endcond */

/**
 * @brief start capturing data written to a specific stream and re-route to a specified sink.
 *
 * @param sink the sink to capture the re-routed data (see @ref<tapi/sink.h>).
 * @param stream the stream to re-route data from.
 * @return a pointer to an allocated capture structure.
 */
tapi_capture_t*
tapi_make_capture(tapi_sink_t* sink, tapi_stream_t stream) {
    /* allocate the structure. */
    tapi_capture_t* capture = calloc(1u, sizeof *capture);
    capture->sink = sink;
    capture->stream = stream;

    /* open the streams. */
    if (pipe(capture->fds) == -1) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi_make_capture; pipe failed; could not create pipe for stdout and stderr.\n");
        exit(EXIT_FAILURE);
    }

    /* flush the stream. */
    fflush(stream);
    capture->dst_fd = dup(fileno(stream));
    if (capture->dst_fd == -1) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi_make_capture; pipe failed; could not create pipe for stdout and "
                        "stderr.\n");
        exit(EXIT_FAILURE);
    }
    if (dup2(capture->fds[1], fileno(stream)) == -1) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi_make_capture; dup2 failed; could not copy over pipe_wr fd to stdout"
                        ".\n");
        exit(EXIT_FAILURE);
    }

    /* pass the pipe write end, close the copy. */
    close(capture->fds[1]);
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
    if (dup2(capture->dst_fd, fileno(capture->stream)) == -1) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi_end_capture; dup2 failed; could not copy saved fd over to stdout. "
                        "errno: %d\n", errno);
        return;
    }
    close(capture->dst_fd);
    capture->dst_fd = -1;

    /* read all data from pipe, and write it to sink. */
    char buf[4096u + 1u];

    /* close writer side so pipe will hit EOF. */
    ssize_t n;
    while ((n = read(capture->fds[0], buf, 4096u)) > 0l) {
        /* all we do is simply write the data to the stream, if it fails then we stop. */
        buf[n] = 0x0;
        if (capture->sink->type == E_TAPI_SINK_TYPE_BUF) {
            ssize_t remaining = (ssize_t) (capture->sink->buffer.capacity - capture->sink->buffer.length);
            if (remaining > 0) {
                strncpy(capture->sink->buffer.data + capture->sink->buffer.length, buf, remaining);
                capture->sink->buffer.length += n;
            }
            else break;
        }
        else fprintf(capture->sink->stream, "%s", buf);
    }

    /* close our read pipe since we are done. */
    close(capture->fds[0]);
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