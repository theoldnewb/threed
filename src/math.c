#include "math.h"


uint32_t
max_u32(
    uint32_t const lhs
,   uint32_t const rhs
)
{
    if(lhs > rhs)
    {
        return lhs ;
    }
    else
    {
        return rhs ;
    }
}


uint32_t
min_u32(
    uint32_t const lhs
,   uint32_t const rhs
)
{
    if(lhs < rhs)
    {
        return lhs ;
    }
    else
    {
        return rhs ;
    }
}