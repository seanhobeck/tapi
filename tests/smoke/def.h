/**
 * @author Sean Hobeck
 * @date 2026-10-23
 */
#ifndef DEF_H
#define DEF_H
#include <tapi/tapi.h>
#include <tapi/mock.h>
#include <tapi/capture.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*! noinline macro. */
#ifdef _WIN32
#define test_noinline __declspec(noinline)
#else
#define test_noinline __attribute__((noinline))
#endif
#endif /* DEF_H. */
