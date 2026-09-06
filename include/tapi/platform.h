/*
* Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-09-05
 */
#ifndef TAPI_PLATFORM_H
#define TAPI_PLATFORM_H

/**
 * similarly to the `TAPI_MINIMAL` macro, `TAPI_THREAD_SAFE` adds locks to data structures (maps, lists)
 *  used internally by tapi. if you are testing within an environment that does not support POSIX.5+,
 *  then you can add the thread-safe=0 flag to your compilation process for tapi.
 */
#define TAPI_THREAD_SAFE

/* for functions that are exported by tapi. */
#if (defined(__GNUC__) || defined(__IBMC__))
#define TAPI_EXPORT __attribute__((visibility("default")))
#define TAPI
#define TAPI_HIDDEN __attribute__((visibility("hidden")))
#else
/* if we are using msvc toolchain (winapi). */
#if defined(_MSC_VER)
#define TAPI_EXPORT __declspec(dllexport)
#define TAPI __cdecl
#define TAPI_HIDDEN
#else
#define TAPI_EXPORT
#define TAPI
#define TAPI_HIDDEN
#endif
#endif

/* architecture-specific macros. */
#if defined(__amd64__) || defined(_M_AMD64)
#define TAPI_AMD64
#endif
#if defined(__i386__) || defined(_M_IX86)
#define TAPI_X86
#endif
#if defined(__aarch64__) || defined(_M_ARM64)
#define TAPI_AARCH64
#endif
#if defined(__arm__) || defined(_M_ARM)
#define TAPI_ARM32
#endif

/* platform-specific macros. */
#ifdef _WIN32
#define TAPI_WINDOWS
#endif
#ifdef __linux__
#define TAPI_LINUX
#endif
#if defined(__APPLE__) || defined(__unix__)
#define TAPI_UNIX
#endif
/** \endcond */
#endif /* TAPI_PLATFORM_H */