#include "vulkan_rob.h"
#include "vulkan.h"
#include "defines.h"
#include "debug.h"
#include "check.h"


// #include <cglm/vec2.h>
// #include <cglm/vec3.h>
#include <cglm/mat4.h>
// #include <cglm/affine.h>
// #include <cglm/cam.h>



#define max_vulkan_descriptor_set_layout_binding    4
#define max_vulkan_descriptor_pool_size             4

#define max_vulkan_descriptor_buffer_infos          1
#define max_vulkan_descriptor_image_infos           1
#define max_vulkan_write_descriptor_sets            1


typedef struct vulkan_rob
{
    VkDescriptorSetLayoutBinding    descriptor_set_layout_bindings_[max_vulkan_descriptor_set_layout_binding] ;
    uint32_t                        descriptor_set_layout_bindings_count_ ;
    VkDescriptorSetLayout           descriptor_set_layout_ ;
    VkDescriptorSet                 descriptor_sets_[max_vulkan_frames_in_flight] ;

    VkDescriptorBufferInfo          descriptor_buffer_infos_[max_vulkan_frames_in_flight * max_vulkan_descriptor_buffer_infos] ;
    uint32_t                        descriptor_buffer_infos_count_ ;
    VkDescriptorImageInfo           descriptor_image_infos_[max_vulkan_frames_in_flight * max_vulkan_descriptor_image_infos] ;
    uint32_t                        descriptor_image_infos_count_ ;
    VkWriteDescriptorSet            write_descriptor_sets_[max_vulkan_frames_in_flight * max_vulkan_write_descriptor_sets] ;
    uint32_t                        write_descriptor_sets_count_ ;


    VkDescriptorPoolSize            descriptor_pool_sizes_[max_vulkan_descriptor_pool_size] ;
    uint32_t                        descriptor_pool_sizes_count_ ;
    VkDescriptorPool                descriptor_pool_ ;

    VkBuffer        uniform_buffers_[max_vulkan_frames_in_flight] ;
    VkDeviceMemory  uniform_buffers_memory_[max_vulkan_frames_in_flight] ;
    void *          uniform_buffers_mapped_[max_vulkan_frames_in_flight] ;



    VkPipelineLayoutCreateInfo      pipeline_layout_create_info_ ;
    VkPipelineLayout                pipeline_layout_ ;


    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info_ ;


    uint32_t            texture_mip_levels_ ;
    VkImage             texture_image_ ;
    VkDeviceMemory      texture_image_memory_ ;
    VkImageView         texture_image_view_ ;
    VkSampler           texture_sampler_ ;
    VkBool32            texture_enable_anisotropy_ ;
    float               texture_anisotropy_ ;


    VkBuffer        vertex_buffer_ ;
    VkDeviceMemory  vertex_buffer_memory_ ;
    VkBuffer        index_buffer_ ;
    VkDeviceMemory  index_buffer_memory_ ;

    VkShaderModule  vert_shader_ ;
    VkShaderModule  frag_shader_ ;


} vulkan_rob ;


static vulkan_rob   the_vulkan_rob_ = { 0 } ;


//////////////////////////////////////7

typedef struct vertex {
    vec3 pos ;
    vec3 color ;
    vec2 uv ;
} vertex ;

static uint32_t const vertex_size = sizeof(vertex) ;

static vertex const vertices[] =
{
    { {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }
,   { { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }
,   { { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} }
,   { {-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }
,   { {-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }
,   { { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }
,   { { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} }
,   { {-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }

} ;
static uint32_t const vertices_size = sizeof(vertices) ;
static uint32_t const vertices_count = array_count(vertices) ;


static uint16_t const indices[] =
{
    0, 1, 2
,   2, 3, 0
,   4, 5, 6
,   6, 7, 4
} ;
static uint32_t const   indices_size = sizeof(indices) ;
static uint32_t const   indices_count = array_count(indices) ;


typedef struct uniform_buffer_object
{
    mat4 model ;
    mat4 view ;
    mat4 proj ;
} uniform_buffer_object ;

static uint32_t const uniform_buffer_object_size = sizeof(uniform_buffer_object) ;




bool
create_rob(
    vulkan_context *    vc
)
{
    require(vc) ;
    begin_timed_block() ;

    vulkan_rob *    vr = &the_vulkan_rob_ ;

    // 1 == means no mip maps
    // 0 == auto mipmap generation
    vr->texture_mip_levels_         = 1 ;
    vr->texture_enable_anisotropy_  = VK_TRUE ;
    vr->texture_anisotropy_         = 1.0f ;

    // allow mipmapping when optimal tiling support is available
    if(
        vc->picked_physical_device_->swapchain_support_details_.formats_properties_->optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
    {
        vr->texture_mip_levels_ = 0 ;
    }

    require(vr->texture_anisotropy_ <= vc->picked_physical_device_->properties_.limits.maxSamplerAnisotropy) ;

    add_desriptor_set_layout_binding(
        vr->descriptor_set_layout_bindings_
    ,   &vr->descriptor_set_layout_bindings_count_
    ,   max_vulkan_descriptor_set_layout_binding
    ,   0
    ,   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    ,   VK_SHADER_STAGE_VERTEX_BIT
    ) ;

    add_desriptor_set_layout_binding(
        vr->descriptor_set_layout_bindings_
    ,   &vr->descriptor_set_layout_bindings_count_
    ,   max_vulkan_descriptor_set_layout_binding
    ,   1
    ,   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    ,   VK_SHADER_STAGE_FRAGMENT_BIT
    ) ;

    if(check(create_descriptor_set_layout(
                &vr->descriptor_set_layout_
            ,   vc->device_
            ,   vr->descriptor_set_layout_bindings_
            ,   vr->descriptor_set_layout_bindings_count_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }


    fill_pipeline_layout_create_info(
        &vr->pipeline_layout_create_info_
    ,   &vr->descriptor_set_layout_
    ,   1
    ) ;


    if(check_vulkan(vkCreatePipelineLayout(
                vc->device_
            ,   &vr->pipeline_layout_create_info_
            ,   NULL
            ,   &vr->pipeline_layout_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vr->pipeline_layout_) ;


    add_descriptor_pool_size(
        vr->descriptor_pool_sizes_
    ,   &vr->descriptor_pool_sizes_count_
    ,   max_vulkan_descriptor_pool_size
    ,   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    ,   vc->frames_in_flight_count_
    ) ;

    add_descriptor_pool_size(
        vr->descriptor_pool_sizes_
    ,   &vr->descriptor_pool_sizes_count_
    ,   max_vulkan_descriptor_pool_size
    ,   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    ,   vc->frames_in_flight_count_
    ) ;


    if(check(create_descriptor_pool(
                &vr->descriptor_pool_
            ,   vc->device_
            ,   vr->descriptor_pool_sizes_
            ,   vr->descriptor_pool_sizes_count_
            ,   vc->frames_in_flight_count_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vr->descriptor_pool_) ;


    if(check(create_descriptor_sets(
                vr->descriptor_sets_
            ,   vc->device_
            ,   vr->descriptor_set_layout_
            ,   vr->descriptor_pool_
            ,   vc->frames_in_flight_count_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vr->descriptor_sets_) ;


    if(check(create_uniform_buffers(
                vr->uniform_buffers_
            ,   vr->uniform_buffers_memory_
            ,   vr->uniform_buffers_mapped_
            ,   vc->device_
            ,   uniform_buffer_object_size
            ,   &vc->picked_physical_device_->memory_properties_
            ,   vc->frames_in_flight_count_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }


    if(check(create_texture_image(
                &vr->texture_image_
            ,   &vr->texture_image_memory_
            ,   "ass/textures/statue-1275469_1280.jpg"
            ,   vc->device_
            ,   vc->command_pool_
            ,   vc->graphics_queue_
            ,   &vc->picked_physical_device_->memory_properties_
            ,   vr->texture_mip_levels_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_texture_image_view(
                &vr->texture_image_view_
            ,   vc->device_
            ,   vr->texture_image_
            ,   vr->texture_mip_levels_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_texture_sampler(
                &vr->texture_sampler_
            ,   vc->device_
            ,   vr->texture_mip_levels_
            ,   vr->texture_enable_anisotropy_
            ,   vr->texture_anisotropy_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    add_descriptor_buffer_info(
        vr->descriptor_buffer_infos_
    ,   &vr->descriptor_buffer_infos_count_
    ,   max_vulkan_descriptor_buffer_infos
    ,   vc->frames_in_flight_count_
    ,   vr->uniform_buffers_
    ,   0
    ,   uniform_buffer_object_size
    ) ;


    add_descriptor_image_info(
        vr->descriptor_image_infos_
    ,   &vr->descriptor_image_infos_count_
    ,   max_vulkan_descriptor_image_infos
    ,   vc->frames_in_flight_count_
    ,   vr->texture_image_view_
    ,   vr->texture_sampler_
    ) ;


    add_write_descriptor_buffer_set(
        vr->write_descriptor_sets_
    ,   &vr->write_descriptor_sets_count_
    ,   max_vulkan_write_descriptor_sets
    ,   vr->descriptor_sets_
    ,   vc->frames_in_flight_count_
    ,   vr->descriptor_buffer_infos_
    ,   vr->descriptor_buffer_infos_count_
    ,   max_vulkan_descriptor_buffer_infos
    ,   vc->frames_in_flight_count_
    ,   0
    ,   0
    ,   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    ) ;

    add_write_descriptor_image_set(
        vr->write_descriptor_sets_
    ,   &vr->write_descriptor_sets_count_
    ,   max_vulkan_write_descriptor_sets
    ,   vr->descriptor_sets_
    ,   vc->frames_in_flight_count_
    ,   vr->descriptor_image_infos_
    ,   vr->descriptor_image_infos_count_
    ,   max_vulkan_descriptor_image_infos
    ,   vc->frames_in_flight_count_
    ,   0
    ,   1
    ,   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    ) ;

    update_descriptor_sets(
        vr->write_descriptor_sets_
    ,   vr->write_descriptor_sets_count_
    ,   max_vulkan_write_descriptor_sets
    ,   vc->device_
    ,   vc->frames_in_flight_count_
    ) ;


    bool
    create_vertex_buffer(
        VkBuffer *                                  out_buffer
    ,   VkDeviceMemory *                            out_buffer_memory
    ,   VkDevice const                              device
    ,   VkCommandPool const                         command_pool
    ,   VkQueue const                               graphics_queue
    ,   VkPhysicalDeviceMemoryProperties const *    pdmp
    ,   void const *                                vbo_data
    ,   VkDeviceSize const                          vbo_data_size
    ) ;


    if(check(create_vertex_buffer(
                &vr->vertex_buffer_
            ,   &vr->vertex_buffer_memory_
            ,   vc->device_
            ,   vc->command_pool_
            ,   vc->graphics_queue_
            ,   &vc->picked_physical_device_->memory_properties_
            ,   vertices
            ,   vertices_size
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vr->vertex_buffer_) ;
    require(vr->vertex_buffer_memory_) ;

    if(check(create_index_buffer(
                &vr->index_buffer_
            ,   &vr->index_buffer_memory_
            ,   vc->device_
            ,   vc->command_pool_
            ,   vc->graphics_queue_
            ,   &vc->picked_physical_device_->memory_properties_
            ,   indices
            ,   indices_size
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vr->index_buffer_) ;
    require(vr->index_buffer_memory_) ;

    if(check(load_shader_file(
                &vr->vert_shader_
            ,   vc->device_
            ,   "ass/shaders/shader.vert.spv"
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vr->vert_shader_) ;

    if(check(load_shader_file(
                &vr->frag_shader_
            ,   vc->device_
            ,   "ass/shaders/shader.frag.spv"
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vr->frag_shader_) ;



    end_timed_block() ;
    return true ;
}
