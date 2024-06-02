#include "math.h"
#include "defines.h"
#include <math.h>

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


uint32_t
calc_mip_levels(
    uint32_t const width
,   uint32_t const height
)
{
    return (uint32_t)floor(log2(max_u32(width, height))) + 1 ;
}

