#pragma once


#ifndef DEFINES_H_INC
#define DEFINES_H_INC


int
check_impl(
    int             result
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
) ;


int
check_c_impl(
    int             result
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
) ;


int
check_sdl_impl(
    int             result
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
) ;



#define ENABLE_CHECK

#ifdef  ENABLE_CHECK
#define check(a)        check_impl(a, #a, __FILE__, __func__, __LINE__)
#define check_c(a)      check_c_impl(a, #a, __FILE__, __func__, __LINE__)
#define check_sdl(a)    check_sdl_impl(a, #a, __FILE__, __func__, __LINE__)
#else
#define check(a)        (!(a))
#define check_c(a)      (0 != (a))
#define check_sdl(a)    (!(a))
#endif


#ifdef  __linux__
#define def_noop    ((void)0)
#endif

#ifdef  __APPLE__
#define def_noop    ((void)0)
#endif

#ifdef  _WIN64
#define def_noop    __noop
#endif


#ifndef UNUSED
#define UNUSED(a)   ((void)a)
#endif


#endif // DEFINES_H_INC

