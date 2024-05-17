#include <stdio.h>
#include <assert.h>

typedef struct user_type
{
    int count_ ;

} user_type ;


void inc(
    int * count
)
{
    void * p = count ;
    //*count++ ;
    (*count)++ ;
    assert(p == count) ;
}


int
main(
    int     argc
,   char *  argv[]
)
{

    user_type ut = { 0 } ;

    inc(&ut.count_) ;

    return 0 ;
}


