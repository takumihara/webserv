#ifndef MY_DEBUG_H_INCLUDED
#define MY_DEBUG_H_INCLUDED


#ifdef DEBUG
# define DEBUG_PUTS(str) puts(str)
# define DEBUG_PRINTF(fmt, ...)  printf(fmt, __VA_ARGS__);      
# define DEBUG_PRINT_ITER(begin, end) \
	do { \
		puts(*begin) \
		begin++;  \
	} while (++begin != end)
#else
# define DEBUG_PUTS(str)
# define DEBUG_PRINTF(fmt, ...)
# define DEBUG_PRINT_ITER(begin, end)
#endif

#endif