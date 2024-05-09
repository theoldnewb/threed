#include <stdio.h>
#include <errno.h>

int
check_impl(
    int             result
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
)
{

    if(result)
    {
       printf("%s:%d %s: %d = %s okay.\n", file, line, func, result, expr) ;
       return 0 ;
    }

    printf("%s:%d %s: %d = %s failed.\n", file, line, func, result, expr) ;

    return 1 ;
}


int
check_c_impl(
    int             result
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
)
{
    if(0 == result)
    {
       printf("%s:%d %s: %d = %s okay.\n", file, line, func, result, expr) ;
       return 0 ;
    }

    printf("%s:%d %s: %d = %s failed. errno=%d\n", file, line, func, result, expr, errno) ;

    return 1 ;
}

#define ENABLE_CHECK

#ifdef  ENABLE_CHECK
#define check(a)        check_impl(a, #a, __FILE__, __func__, __LINE__)
#define check_c(a)      check_c_impl(a, #a, __FILE__, __func__, __LINE__)
#else
#define check(a)        (!(a))
#define check_c(a)      (0 != (a))
#endif


int
dummy(
    int i
)
{
    return i ;
}

void
do_test()
{
    if(check_c(dummy(0)))
    {
        printf("check failed.\n") ;
        return ;
    }

    printf("check okay.\n") ;
}

void
do_test2()
{
    if(check(dummy(1)))
    {
        printf("check failed.\n") ;
        return ;
    }

    printf("check okay.\n") ;
}

int
main(
    int     argc
,   char *  argv[]
)
{
    do_test() ;
    do_test2() ;
    return 0 ;
}