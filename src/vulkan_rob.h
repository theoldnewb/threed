#pragma once


#include "types.h"




typedef struct vulkan_context vulkan_context ;


bool
create_rob(
    vulkan_context *    vc
) ;


bool
update_rob(
    vulkan_context *    vc
) ;


bool
draw_rob(
    vulkan_context *    vc
) ;


