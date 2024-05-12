#pragma once


#ifdef  __linux__
#define def_noop    ((void)0)
#define require(a)  if(!(a)){__builtin_trap();}
#endif

#ifdef  __APPLE__
#define def_noop    ((void)0)
#define require(a)  if(!(a)){__builtin_trap();}
#endif

#ifdef  _WIN64
#define def_noop    __noop
#define require(a)  if(!(a)){DebugBreak();}
#endif

#ifndef UNUSED
#define UNUSED(a)   ((void)a)
#endif


#define boolify(a)  ((a)?1:0)
