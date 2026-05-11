/**
 * @author Sean Hobeck
 * @date 2026-01-21
 */
#ifndef INTT_H
#define INTT_H

/* internal specifier. */
#define internal static

/** enum for different internal tools results, quick eval. */
typedef enum {
    E_INTT_RESULT_FAILURE = 0x0,
    E_INTT_RESULT_SUCCESS = 0x1,
} e_intt_result_t;

/* quick macro. */
#define e_intt_passed(result) (result == E_INTT_RESULT_SUCCESS)
#endif /* INTT_H */