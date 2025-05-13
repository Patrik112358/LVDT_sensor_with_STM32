#ifndef DEBUGTOOLS_H
#define DEBUGTOOLS_H
#include <stdio.h> // IWYU pragma: export

#ifndef LOG_WITH_LOCKS
#  define LOG_WITHOUT_LOCKS 1
#endif

#ifdef NO_LOC_HEADER_USE_THREAD_NAME
#  ifndef __USE_GNU
#    define __USE_GNU
#    define __USE_GNU_WAS_ADDED
#  endif // __USE_GNU
#  include <pthread.h> // IWYU pragma: export
#  ifdef __USE_GNU_WAS_ADDED
#    undef __USE_GNU
#    undef __USE_GNU_WAS_ADDED
#  endif // __USE_GNU_WAS_ADDED
#endif

#define LL_DEBUG 10
#define LL_INFO  20
#define LL_WARN  30
#define LL_ERROR 40
#define LL_FATAL 50

#ifndef SILENCE_ALL_LOGS
#  define SILENCE_ALL_LOGS 0
#else
#  define SILENCE_ALL_LOGS 1
#endif

#define COL_RESET    "\033[0m"
#define FORE_RED     "\033[0;31m"
#define FORE_YELLOW  "\033[0;33m"
#define FORE_CYAN    "\033[1;36m"
#define BACK_RED     "\033[41m"

#define LL_DEBUG_MSG "Debug"
#define LL_INFO_MSG  "Info"
#define LL_WARN_MSG  FORE_YELLOW "Warning" COL_RESET
#define LL_ERROR_MSG FORE_RED "ERROR" COL_RESET
#define LL_FATAL_MSG BACK_RED "FATAL ERROR" COL_RESET

#ifdef LOG_WITHOUT_LOCKS
#  define FLOCKFILE(file)
#  define FUNLOCKFILE(file)
#  define FTRYLOCKFILE(file) 1
#else
#  define FLOCKFILE(file)    flockfile(stderr)
#  define FUNLOCKFILE(file)  funlockfile(stderr)
#  define FTRYLOCKFILE(file) ftrylockfile(stderr)
#endif

#define LL_MSG(loglevel) \
  ((loglevel == LL_DEBUG)          ? LL_DEBUG_MSG \
          : (loglevel == LL_INFO)  ? LL_INFO_MSG \
          : (loglevel == LL_WARN)  ? LL_WARN_MSG \
          : (loglevel == LL_ERROR) ? LL_ERROR_MSG \
          : (loglevel == LL_FATAL) ? LL_FATAL_MSG \
                                   : "UNKNOWN")

#ifdef NO_LOC_HEADER_USE_THREAD_NAME

#  define LOG(use_location_header, loglevel, format, ...) \
    do { \
      if(SILENCE_ALL_LOGS == 0 && loglevel >= LOGLEVEL) \
      { \
        int  stderror_was_locked = 0; \
        char _threadname[16] = { 0 }; \
        pthread_getname_np(pthread_self(), _threadname, sizeof(_threadname)); \
        if(loglevel >= LL_FATAL) { stderror_was_locked = FTRYLOCKFILE(stderr); } \
        else \
        { \
          FLOCKFILE(stderr); \
          stderror_was_locked = 1; \
        } \
        fprintf(stderr, "[%s] ", LL_MSG(loglevel)); \
        if(use_location_header) fprintf(stderr, "[%s]: ", _threadname); \
        fprintf(stderr, FORE_CYAN); \
        fprintf(stderr, format, ##__VA_ARGS__); \
        fprintf(stderr, COL_RESET); \
        if(stderror_was_locked) { FUNLOCKFILE(stderr); } \
      } \
    } while(0)

#else

#  define LOG(use_location_header, loglevel, format, ...) \
    do { \
      if(SILENCE_ALL_LOGS == 0 && loglevel >= LOGLEVEL) \
      { \
        int stderror_was_locked = 0; \
        if(loglevel >= LL_FATAL) { stderror_was_locked = FTRYLOCKFILE(stderr); } \
        else \
        { \
          FLOCKFILE(stderr); \
          stderror_was_locked = 1; \
        } \
        fprintf(stderr, "[%s] ", LL_MSG(loglevel)); \
        if(use_location_header) fprintf(stderr, "%s:%d in function %s: ", __FILE__, __LINE__, __func__); \
        fprintf(stderr, FORE_CYAN); \
        fprintf(stderr, format, ##__VA_ARGS__); \
        fprintf(stderr, COL_RESET); \
        if(stderror_was_locked) { FUNLOCKFILE(stderr); } \
      } \
    } while(0)

#endif

#define LOG_EXPRESSION(use_location_header, loglevel, format, ...) \
  LOG(use_location_header, LL_WARN, #__VA_ARGS__ " = " format, __VA_ARGS__)

#define DEBUG(format, ...)       LOG(1, LL_DEBUG, format, ##__VA_ARGS__)
#define DEBUG_NOLOC(format, ...) LOG(0, LL_DEBUG, format, ##__VA_ARGS__)
#define DEBUG_EXP(format, ...)   DEBUG_NOLOC(#__VA_ARGS__ " = " format, __VA_ARGS__)

#define INFO(format, ...)        LOG(1, LL_INFO, format, ##__VA_ARGS__)

#define WARN(format, ...)        LOG(1, LL_WARN, format, ##__VA_ARGS__)

#define ERROR(format, ...)       LOG(1, LL_ERROR, format, ##__VA_ARGS__)

#define FATAL(format, ...)       LOG(1, LL_FATAL, format, ##__VA_ARGS__)

#define DO_IF_LOGLEVEL_SUFFICIENT(loglevel, code) \
  do { \
    if(SILENCE_ALL_LOGS == 0 && loglevel >= LOGLEVEL) { code; } \
  } while(0)

#endif // DEBUGTOOLS_H
