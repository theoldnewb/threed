#pragma once


#ifdef  __linux__
#define def_noop            ((void)0)
#define require(a)          if(!(a)){__builtin_trap();}
#define static_require(a,b) _Static_assert(a, b)
#define get_stack_ptr()     __builtin_return_address(0)
#endif


#ifdef  __APPLE__
#define def_noop    ((void)0)
#define require(a)  if(!(a)){__builtin_trap();}
#endif

#ifdef  _WIN64
#define def_noop            __noop
#define require(a)          if(!(a)){DebugBreak();}
#define static_require(a,b) static_assert(a, b)
#define get_stack_ptr()     (0)
#endif


#ifndef UNUSED
#define UNUSED(a)   ((void)a)
#endif


#define boolify(a)  ((a)?1:0)
#define array_count(a)  (sizeof(a)/sizeof(a[0]))


#define asset_ref(a, b, c)  ((a *)(((char *) b) + c))
//#define asset_off(a, b, c)  ((struct a *)(((u8 *) b) + b->offsets_[c]))
