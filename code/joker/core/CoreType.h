#pragma once

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

#ifndef assert
#define assert static_assert(false, "Please use JASSERT")
#endif

#define JASSERT(x)                        (void)0

#define JLOG_LOGGER_TRACE(logger, ...)    (void)0
#define JLOG_TRACE(...)                   (void)0

#define JLOG_LOGGER_DEBUG(logger, ...)    (void)0
#define JLOG_DEBUG(...)                   (void)0

#define JLOG_LOGGER_INFO(logger, ...)     (void)0
#define JLOG_INFO(...)                    (void)0

#define JLOG_LOGGER_WARN(logger, ...)     (void)0
#define JLOG_WARN(...)                    (void)0

#define JLOG_LOGGER_ERROR(logger, ...)    (void)0
#define JLOG_ERROR(...)                   (void)0

#define JLOG_LOGGER_CRITICAL(logger, ...) (void)0
#define JLOG_CRITICAL(...)                (void)0