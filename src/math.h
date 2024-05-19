#pragma once


#include "types.h"


uint32_t
min_u32(
    uint32_t const lhs
,   uint32_t const rhs
) ;


uint32_t
max_u32(
    uint32_t const lhs
,   uint32_t const rhs
) ;


uint32_t
clamp_u32(
    uint32_t const value
,   uint32_t const min_value
,   uint32_t const max_value
) ;
