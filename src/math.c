#include "math.h"
#include "defines.h"

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


uint32_t
clamp_u32(
    uint32_t const value
,   uint32_t const min_value
,   uint32_t const max_value
)
{
    require(min_value <= max_value) ;
    if(value < min_value)
    {
        return min_value ;
    }
    else if(value > max_value)
    {
        return max_value ;
    }

    return value ;
}
