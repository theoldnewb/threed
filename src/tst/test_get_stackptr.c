#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#define get_stack_ptr()   __builtin_return_address(0)


//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wreturn-local-addr"
// this only works in debug even with -fno-omit-frame-pointer begin() and end() will not match
// volatile uintptr_t
// get_stack_ptr()
// {
//     volatile uintptr_t sp = 0 ;
//     sp = (volatile uintptr_t)(volatile void const *) &sp ;
//     return sp ;
// }
//#pragma GCC diagnostic pop


#define begin() printf("%s %s %d %p \n", __FILE__, __func__, __LINE__, get_stack_ptr())
#define end()   printf("%s %s %d %p \n", __FILE__, __func__, __LINE__, get_stack_ptr())



void test_1()
{
    begin() ;
    end() ;
}



void test_0()
{
    begin() ;
    test_1() ;
    end() ;
}


int
main(
    int     argc
,   char *  argv[]
)
{
    begin() ;

    test_0() ;

    end() ;
    return 0 ;
}


