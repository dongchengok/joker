#pragma once

#include "CoreType.h"
static_assert(sizeof(char) == 1, "Wrong type size!");
static_assert(sizeof(float) == 4, "Wrong type size!");
static_assert(sizeof(int) >= 4, "Wrong type size!");

typedef unsigned char      uchar;
typedef signed char        schar;

typedef unsigned short     ushort;
typedef signed short       sshort;

typedef unsigned int       uint;
typedef signed int         sint;

typedef unsigned long      ulong;
typedef signed long        slong;

typedef unsigned long long ulonglong;
typedef signed long long   slonglong;

static_assert(sizeof(uchar) == sizeof(schar), "Wrong type size!");
static_assert(sizeof(ushort) == sizeof(sshort), "Wrong type size!");
static_assert(sizeof(uint) == sizeof(sint), "Wrong type size!");
static_assert(sizeof(ulong) == sizeof(slong), "Wrong type size!");
static_assert(sizeof(ulonglong) == sizeof(slonglong), "Wrong type size!");

static_assert(sizeof(uchar) <= sizeof(ushort), "Wrong type size!");
static_assert(sizeof(ushort) <= sizeof(uint), "Wrong type size!");
static_assert(sizeof(uint) <= sizeof(ulong), "Wrong type size!");
static_assert(sizeof(ulong) <= sizeof(ulonglong), "Wrong type size!");

typedef schar n8;
typedef uchar u8;
static_assert(sizeof(u8) == 1, "Wrong type size!");
static_assert(sizeof(n8) == 1, "Wrong type size!");

typedef sshort n16;
typedef ushort u16;
static_assert(sizeof(n16) == 2, "Wrong type size!");
static_assert(sizeof(u16) == 2, "Wrong type size!");

typedef sint n32;
typedef uint u32;
static_assert(sizeof(n32) == 4, "Wrong type size!");
static_assert(sizeof(u32) == 4, "Wrong type size!");

typedef slonglong n64;
typedef ulonglong u64;
static_assert(sizeof(n64) == 8, "Wrong type size!");
static_assert(sizeof(u64) == 8, "Wrong type size!");

typedef float  f32;
typedef double f64;
static_assert(sizeof(f32) == 4, "Wrong type size!");
static_assert(sizeof(f64) == 8, "Wrong type size!");

#define JARRAY_SIZE(a)                    (sizeof(a) / sizeof(a[0]))

#define SPDLOG_ACTIVE_LEVEL JLOG_LEVEL
#include <spdlog/spdlog.h>
#define JLOG_LOGGER_TRACE(logger, ...)    SPDLOG_LOGGER_TRACE(logger,__VA_ARGS__)
#define JLOG_TRACE(...)                   SPDLOG_TRACE(__VA_ARGS__)

#define JLOG_LOGGER_DEBUG(logger, ...)    SPDLOG_LOGGER_DEBUG(logger,__VA_ARGS__)
#define JLOG_DEBUG(...)                   SPDLOG_DEBUG(__VA_ARGS__)

#define JLOG_LOGGER_INFO(logger, ...)     SPDLOG_LOGGER_INFO(logger,__VA_ARGS__)
#define JLOG_INFO(...)                    SPDLOG_INFO(__VA_ARGS__)

#define JLOG_LOGGER_WARN(logger, ...)     SPDLOG_LOGGER_WARN(logger,__VA_ARGS__)
#define JLOG_WARN(...)                    SPDLOG_WARN(__VA_ARGS__)

#define JLOG_LOGGER_ERROR(logger, ...)    SPDLOG_LOGGER_ERROR(logger,__VA_ARGS__)
#define JLOG_ERROR(...)                   SPDLOG_ERROR(logger,__VA_ARGS__)

#define JLOG_LOGGER_CRITICAL(logger, ...) SPDLOG_LOGGER_CRITICAL(logger,__VA_ARGS__)
#define JLOG_CRITICAL(...)                SPDLOG_CRITICAL(__VA_ARGS__)

#include <assert.h>
#define JASSERT(x)                        if(!(x)){ JLOG_CRITICAL("\nexpr:\t{}\nfile:\t{}\nline:\t{}\nfunc:\t{}",#x,__FILE__, __LINE__, __FUNCTION__); assert(false);}

#include <stdlib.h>
#include <malloc.h>
#define JMALLOC(size)                              malloc(size)
#define JMALLOC_ALIGNED(count, size, aligned_size) malloc(count* size)
#define JCALLOC(count, size)                       calloc(count, size)
#define JCALLOC_ALIGNED(count, size, aligned_size) calloc(count, size)
#define JALLOC(size)                               alloca(size)
#define JFREE(ptr)                                                                                                                                   \
    if (ptr)                                                                                                                                         \
    {                                                                                                                                                \
        free(ptr);                                                                                                                                   \
        ptr = nullptr;                                                                                                                               \
    }