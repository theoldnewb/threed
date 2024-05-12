#pragma once


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


#ifdef  ENABLE_CHECK
#define check(a)        check_impl(a, #a, __FILE__, __func__, __LINE__)
#define check_c(a)      check_c_impl(a, #a, __FILE__, __func__, __LINE__)
#define check_sdl(a)    check_sdl_impl(a, #a, __FILE__, __func__, __LINE__)
#else
#define check(a)        (!(a))
#define check_c(a)      (0 != (a))
#define check_sdl(a)    (!(a))
#endif


