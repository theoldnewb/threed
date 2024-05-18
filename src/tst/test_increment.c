#include <stdio.h>
#include <assert.h>

typedef struct user_type
{
    int count_ ;
    int count2_ ;

} user_type ;


void
inc_0(
    int * count
)
{
    int const a = *count ;
    int *     b = count ;
    (*count)++ ;
    assert(*count == a+1) ;
    assert(count == b) ;
}


void
inc_1(
    int * count
)
{
    int const a = *count ;
    int *     b = count ;
    *count++ ;
    int *     c = count ;
    int       d = (char *)c - (char *)b ;
    assert(*count == a) ;
    printf("count=%p, b=%p c=%p, %d\n", count, b, c, d) ;
    assert((char *)count == (char *)b+sizeof(int)) ;
}


void
inc_2(
    int * count
)
{
    int const a = *count ;
    int *     b = count ;
    count[0]++ ;
    assert(*count == a+1) ;
    assert(count == b) ;
}



int
main(
    int     argc
,   char *  argv[]
)
{

    {
        user_type ut = { 0 } ;
        inc_0(&ut.count_) ;
    }

    {
        user_type ut = { 0 } ;
        inc_1(&ut.count_) ;
    }

    {
        user_type ut = { 0 } ;
        inc_2(&ut.count_) ;
    }

    return 0 ;
}


