#ifndef MY_DEBUG_H_INCLUDED
#define MY_DEBUG_H_INCLUDED

#ifdef DEBUG
#define DEBUG_PUTS(str) fprintf(stderr, "%s\n", str);
#define DEBUG_PRINTF(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__);
#else
#define DEBUG_PUTS(str)
#define DEBUG_PRINTF(fmt, ...)
#endif

#endif