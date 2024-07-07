#include "app.h"
#include "vulkan_rob_test.h"
#include "vulkan.h"
#include "defines.h"
#include "debug.h"
#include "check.h"
#include "log.h"

#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>
#include <cglm/cam.h>

#include <SDL3/SDL_stdinc.h>


#define max_vulkan_descriptor_set_layout_binding        4
#define max_vulkan_descriptor_pool_size                 4
#define max_vulkan_pipeline_shader_stage_create_infos   2
#define max_vulkan_vertex_input_attribute_descriptions  3
#define max_vulkan_dynamic_states                       2
#define max_vulkan_descriptor_buffer_infos              1
#define max_vulkan_descriptor_image_infos               1
#define max_vulkan_write_descriptor_sets                2


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
    VkPipeline                      graphics_pipeline_ ;

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


    VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_infos_[max_vulkan_pipeline_shader_stage_create_infos] ;
    uint32_t                        pipeline_shader_stage_create_infos_count_ ;

    VkVertexInputBindingDescription vertex_input_binding_description_ ;

    VkVertexInputAttributeDescription   vertex_input_attribute_descriptions_[max_vulkan_vertex_input_attribute_descriptions] ;
    uint32_t                            vertex_input_attribute_descriptions_count_ ;

    VkPipelineVertexInputStateCreateInfo    pipeline_vertex_input_state_create_info_ ;
    VkPipelineInputAssemblyStateCreateInfo  pipeline_input_assembly_state_create_info_ ;
    VkPipelineViewportStateCreateInfo       pipeline_viewport_state_create_info_ ;
    VkPipelineDynamicStateCreateInfo        pipeline_dynamic_state_create_info_ ;
    VkPipelineRasterizationStateCreateInfo  pipeline_rasterization_state_create_info_ ;
    VkPipelineMultisampleStateCreateInfo    pipeline_multisample_state_create_info_ ;
    VkPipelineColorBlendAttachmentState     pipeline_color_blend_attachment_state_ ;
    VkPipelineColorBlendStateCreateInfo     pipeline_color_blend_state_create_info_ ;
    VkPipelineDepthStencilStateCreateInfo   pipeline_depth_stencil_state_create_info_ ;
    VkGraphicsPipelineCreateInfo            graphics_pipeline_create_info_ ;

    VkViewport  viewport_ ;
    VkRect2D    scissor_ ;

    VkDynamicState  dynamic_states_[max_vulkan_dynamic_states] ;
    uint32_t        dynamic_states_count_ ;

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
    { {-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }
,   { { 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }
,   { { 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} }
,   { {-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }

} ;
static uint32_t const vertices_size = sizeof(vertices) ;
//static uint32_t const vertices_count = array_count(vertices) ;


static uint16_t const indices[] =
{
    0, 1, 2
,   2, 3, 0
} ;
static uint32_t const   indices_size = sizeof(indices) ;
static uint32_t const   indices_count = array_count(indices) ;


typedef struct uniform_buffer_object
{
    mat4 model ;
    mat4 view ;
    mat4 proj ;
    mat4 all ;
} uniform_buffer_object ;


static uint32_t const uniform_buffer_object_size = sizeof(uniform_buffer_object) ;


static uniform_buffer_object ubos_[max_vulkan_frames_in_flight] = { 0 } ;


static void
update_uniform_buffer(
    vulkan_context *    vc
,   vulkan_rob *        vr
,   uint32_t const      current_frame
)
{
    require(vc) ;
    require(current_frame < max_vulkan_frames_in_flight) ;
    require(current_frame < vc->frames_in_flight_count_) ;
    require(vr) ;

    static bool once = true ;
    static uint64_t previous_time = 0 ;

    uint64_t const current_time = get_app_time() ;

    if(once)
    {
        once = false ;
        previous_time = current_time ;
    }

    uint64_t const delta_time = (current_time - previous_time) ;
    double const fractional_seconds = (double) delta_time * get_performance_frequency_inverse() ;

    uniform_buffer_object * ubo = &ubos_[current_frame] ;

    float angle = fractional_seconds ;
    vec3 axis = {0.0f, 1.0f, 0.0f} ;
    glm_rotate_make(ubo->model, angle, axis) ;

    vec3 eye    = { 0.0f, 0.0f, -1.0f } ;
    vec3 center = { 0.0f, 0.0f, 0.0f } ;
    vec3 up     = { 0.0f, 1.0f, 0.0f } ;
    glm_lookat(eye, center, up, ubo->view) ;

    float fovy = glm_rad(45.0f) ;
    float aspect_ratio = (float)vc->swapchain_extent_.width / (float)vc->swapchain_extent_.height ;
    float near = 0.1f ;
    float far = 10.f ;
    //glm_perspective(fovy, aspect_ratio, near, far, ubo->proj) ;

    //glm_ortho(0.0f, vc->swapchain_extent_.width, vc->swapchain_extent_.height, 0.0f, near, far, ubo->proj) ;
    glm_ortho(-1.0f, 1.0f, 1.0f, -1.0f, near, far, ubo->proj) ;

    mat4 t ;
    glm_mat4_mul(ubo->view, ubo->model, t) ;
    glm_mat4_mul(ubo->proj, t, ubo->all) ;

    vec4 v = {-0.5f, 0.5f, 0.0f, 1.0f} ;
    vec4 q ;
    glm_mat4_mulv(ubo->all, v, q) ;

    log_debug_f32_4(q) ;

    SDL_memcpy(vr->uniform_buffers_mapped_[current_frame], ubo, uniform_buffer_object_size) ;
}



static bool
record_command_buffer(
    vulkan_context *    vc
,   vulkan_rob *        vr
,   VkCommandBuffer     command_buffer
,   VkDescriptorSet     descriptor_set
)
{
    require(vc) ;
    require(vr) ;
    require(command_buffer) ;
    require(descriptor_set) ;

    begin_timed_block() ;

    // void vkCmdBindPipeline(
    //     VkCommandBuffer                             commandBuffer,
    //     VkPipelineBindPoint                         pipelineBindPoint,
    //     VkPipeline                                  pipeline);
    vkCmdBindPipeline(
        command_buffer
    ,   VK_PIPELINE_BIND_POINT_GRAPHICS
    ,   vr->graphics_pipeline_
    ) ;


    VkBuffer vertex_buffers[] = { vr->vertex_buffer_} ;
    VkDeviceSize offsets[] = { 0 } ;

    // void vkCmdBindVertexBuffers(
    //     VkCommandBuffer                             commandBuffer,
    //     uint32_t                                    firstBinding,
    //     uint32_t                                    bindingCount,
    //     const VkBuffer*                             pBuffers,
    //     const VkDeviceSize*                         pOffsets);
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets) ;


    // void vkCmdBindIndexBuffer(
    //     VkCommandBuffer                             commandBuffer,
    //     VkBuffer                                    buffer,
    //     VkDeviceSize                                offset,
    //     VkIndexType                                 indexType);
    vkCmdBindIndexBuffer(
        command_buffer
    ,   vr->index_buffer_
    ,   0
    ,   VK_INDEX_TYPE_UINT16
    ) ;

    // void vkCmdBindDescriptorSets(
    //     VkCommandBuffer                             commandBuffer,
    //     VkPipelineBindPoint                         pipelineBindPoint,
    //     VkPipelineLayout                            layout,
    //     uint32_t                                    firstSet,
    //     uint32_t                                    descriptorSetCount,
    //     const VkDescriptorSet*                      pDescriptorSets,
    //     uint32_t                                    dynamicOffsetCount,
    //     const uint32_t*                             pDynamicOffsets);
    vkCmdBindDescriptorSets(
        command_buffer
    ,   VK_PIPELINE_BIND_POINT_GRAPHICS
    ,   vr->pipeline_layout_
    ,   0
    ,   1
    ,   &descriptor_set
    ,   0
    ,   NULL
    ) ;

    // void vkCmdDrawIndexed(
    //     VkCommandBuffer                             commandBuffer,
    //     uint32_t                                    indexCount,
    //     uint32_t                                    instanceCount,
    //     uint32_t                                    firstIndex,
    //     int32_t                                     vertexOffset,
    //     uint32_t                                    firstInstance);
    vkCmdDrawIndexed(command_buffer, indices_count, 1, 0, 0, 0) ;

    end_timed_block() ;
    return true ;
}




static bool
update_rob(
    vulkan_context *    vc
,   void *              param
,   uint32_t const      current_frame
)
{
    require(vc) ;
    begin_timed_block() ;

    vulkan_rob *    vr = &the_vulkan_rob_ ;
    require(vr == param) ;

    update_uniform_buffer(vc, vr, current_frame) ;

    end_timed_block() ;
    return true ;
}


static bool
record_rob(
    vulkan_context *    vc
,   void *              param
,   uint32_t const      current_frame
)
{
    require(vc) ;
    begin_timed_block() ;
    vulkan_rob *    vr = &the_vulkan_rob_ ;
    require(vr == param) ;

    require(current_frame < vc->frames_in_flight_count_) ;

    if(check(record_command_buffer(
                vc
            ,   vr
            ,   vc->command_buffer_[current_frame]
            ,   vr->descriptor_sets_[current_frame]
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
draw_rob(
    vulkan_context *    vc
,   void *              param
,   uint32_t const      current_frame
)
{
    require(vc) ;
    begin_timed_block() ;
    vulkan_rob *    vr = &the_vulkan_rob_ ;
    require(vr == param) ;
    require(current_frame < vc->frames_in_flight_count_) ;

    if(vc->enable_pre_record_command_buffers_)
    {
        end_timed_block() ;
        return true ;
    }

    if(check(record_command_buffer(
                vc
            ,   vr
            ,   vc->command_buffer_[current_frame]
            ,   vr->descriptor_sets_[current_frame]
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
destroy_rob(
    vulkan_context *    vc
,   void *              param
)
{
    require(vc) ;
    begin_timed_block() ;

    vulkan_rob *    vr = &the_vulkan_rob_ ;
    require(vr == param) ;

    if(vr->texture_sampler_)
    {
        // void vkDestroySampler(
        //     VkDevice                                    device,
        //     VkSampler                                   sampler,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroySampler(vc->device_, vr->texture_sampler_, NULL) ;
        vr->texture_sampler_ = NULL ;
    }

    if(vr->texture_image_view_)
    {
        vkDestroyImageView(vc->device_, vr->texture_image_view_, NULL) ;
        vr->texture_image_view_ = NULL ;
    }

    if(vr->texture_image_)
    {
        // void vkDestroyImage(
        // VkDevice                                    device,
        // VkImage                                     image,
        // const VkAllocationCallbacks*                pAllocator);
        vkDestroyImage(vc->device_, vr->texture_image_, NULL) ;
        vr->texture_image_ = NULL ;
    }

    if(vr->texture_image_memory_)
    {
        vkFreeMemory(vc->device_, vr->texture_image_memory_, NULL) ;
        vr->texture_image_memory_ = NULL ;
    }

    for(
        uint32_t i = 0
    ;   i < vc->frames_in_flight_count_
    ;   ++i
    )
    {
        vkDestroyBuffer(vc->device_, vr->uniform_buffers_[i], NULL) ;
        vr->uniform_buffers_[i] = NULL ;
        vkFreeMemory(vc->device_, vr->uniform_buffers_memory_[i], NULL) ;
        vr->uniform_buffers_memory_[i] = NULL ;
    }

    if(vr->descriptor_pool_)
    {
        // void vkDestroyDescriptorPool(
        //     VkDevice                                    device,
        //     VkDescriptorPool                            descriptorPool,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyDescriptorPool(vc->device_, vr->descriptor_pool_, NULL) ;
        vr->descriptor_pool_ = NULL ;
    }


    if(vr->descriptor_set_layout_)
    {
        // void vkDestroyDescriptorSetLayout(
        //     VkDevice                                    device,
        //     VkDescriptorSetLayout                       descriptorSetLayout,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyDescriptorSetLayout(vc->device_, vr->descriptor_set_layout_, NULL) ;
        vr->descriptor_set_layout_ = NULL ;
    }


    if(vr->index_buffer_)
    {
        vkDestroyBuffer(vc->device_, vr->index_buffer_, NULL) ;
        vr->index_buffer_ = NULL ;
    }

    if(vr->index_buffer_memory_)
    {
        vkFreeMemory(vc->device_, vr->index_buffer_memory_, NULL) ;
        vr->index_buffer_memory_ = NULL ;
    }


    if(vr->vertex_buffer_)
    {
        // void vkDestroyBuffer(
        //     VkDevice                                    device,
        //     VkBuffer                                    buffer,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyBuffer(vc->device_, vr->vertex_buffer_, NULL) ;
        vr->vertex_buffer_ = NULL ;
    }

    if(vr->vertex_buffer_memory_)
    {
        // void vkFreeMemory(
        //     VkDevice                                    device,
        //     VkDeviceMemory                              memory,
        //     const VkAllocationCallbacks*                pAllocator);
        //     }
        vkFreeMemory(vc->device_, vr->vertex_buffer_memory_, NULL) ;
        vr->vertex_buffer_memory_ = NULL ;
    }

    if(vr->graphics_pipeline_)
    {
        // void vkDestroyPipeline(
        //     VkDevice                                    device,
        //     VkPipeline                                  pipeline,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyPipeline(vc->device_, vr->graphics_pipeline_, NULL) ;
        vr->graphics_pipeline_ = NULL ;
    }

    if(vr->pipeline_layout_)
    {
        // void vkDestroyPipelineLayout(
        //     VkDevice                                    device,
        //     VkPipelineLayout                            pipelineLayout,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyPipelineLayout(vc->device_, vr->pipeline_layout_, NULL) ;
        vr->pipeline_layout_ = NULL ;
    }

    end_timed_block() ;
    return true ;
}


static bool
create_rob(
    vulkan_context *    vc
,   void *              param
)
{
    require(vc) ;
    begin_timed_block() ;

    vulkan_rob *    vr = &the_vulkan_rob_ ;
    require(vr == param) ;

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

    require(0 == vr->descriptor_pool_sizes_count_) ;
    add_descriptor_pool_size(
        vr->descriptor_pool_sizes_
    ,   &vr->descriptor_pool_sizes_count_
    ,   max_vulkan_descriptor_pool_size
    ,   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    ,   vc->frames_in_flight_count_
    ) ;
    require(1 == vr->descriptor_pool_sizes_count_) ;

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
            ,   &vr->texture_mip_levels_
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
            ,   "ass/shaders/shader_test.vert.spv"
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
            ,   "ass/shaders/shader_test.frag.spv"
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vr->frag_shader_) ;

    add_pipeline_shader_stage_create_info(
        vr->pipeline_shader_stage_create_infos_
    ,   &vr->pipeline_shader_stage_create_infos_count_
    ,   max_vulkan_pipeline_shader_stage_create_infos
    ,   vr->vert_shader_
    ,   VK_SHADER_STAGE_VERTEX_BIT
    ) ;

    add_pipeline_shader_stage_create_info(
        vr->pipeline_shader_stage_create_infos_
    ,   &vr->pipeline_shader_stage_create_infos_count_
    ,   max_vulkan_pipeline_shader_stage_create_infos
    ,   vr->frag_shader_
    ,   VK_SHADER_STAGE_FRAGMENT_BIT
    ) ;

    fill_vertex_input_binding_description(
        &vr->vertex_input_binding_description_
    ,   0
    ,   vertex_size
    ) ;

    add_vertex_input_attribute_description(
        vr->vertex_input_attribute_descriptions_
    ,   &vr->vertex_input_attribute_descriptions_count_
    ,   max_vulkan_vertex_input_attribute_descriptions
    ,   0
    ,   0
    ,   VK_FORMAT_R32G32B32_SFLOAT
    ,   offsetof(vertex, pos)
    ) ;

    add_vertex_input_attribute_description(
        vr->vertex_input_attribute_descriptions_
    ,   &vr->vertex_input_attribute_descriptions_count_
    ,   max_vulkan_vertex_input_attribute_descriptions
    ,   1
    ,   0
    ,   VK_FORMAT_R32G32B32_SFLOAT
    ,   offsetof(vertex, color)
    ) ;

    add_vertex_input_attribute_description(
        vr->vertex_input_attribute_descriptions_
    ,   &vr->vertex_input_attribute_descriptions_count_
    ,   max_vulkan_vertex_input_attribute_descriptions
    ,   2
    ,   0
    ,   VK_FORMAT_R32G32_SFLOAT
    ,   offsetof(vertex, uv)
    ) ;

    fill_pipeline_vertex_input_state_create_info(
        &vr->pipeline_vertex_input_state_create_info_
    ,   &vr->vertex_input_binding_description_
    ,   vr->vertex_input_attribute_descriptions_
    ,   vr->vertex_input_attribute_descriptions_count_
    ) ;

    fill_pipeline_input_assembly_state_create_info(
        &vr->pipeline_input_assembly_state_create_info_
    ,   VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    ,   VK_FALSE
    ) ;

    fill_viewport(
        &vr->viewport_
    ,   0
    ,   0
    ,   vc->swapchain_extent_.width
    ,   vc->swapchain_extent_.height
    ,   0
    ,   1
    ) ;

    fill_scissor(
        &vr->scissor_
    ,   0
    ,   0
    ,   vc->swapchain_extent_.width
    ,   vc->swapchain_extent_.height
    ) ;


    fill_pipeline_viewport_state_create_info(
        &vr->pipeline_viewport_state_create_info_
    ,   &vr->viewport_
    ,   &vr->scissor_
    ) ;

    add_to_dynamic_state(
        vr->dynamic_states_
    ,   &vr->dynamic_states_count_
    ,   max_vulkan_dynamic_states
    ,   VK_DYNAMIC_STATE_VIEWPORT
    ) ;

    add_to_dynamic_state(
        vr->dynamic_states_
    ,   &vr->dynamic_states_count_
    ,   max_vulkan_dynamic_states
    ,   VK_DYNAMIC_STATE_SCISSOR
    ) ;

    fill_pipeline_dynamic_state_create_info(
        &vr->pipeline_dynamic_state_create_info_
    ,   vr->dynamic_states_
    ,   vr->dynamic_states_count_
    ) ;

    fill_pipeline_rasterization_state_create_info(
        &vr->pipeline_rasterization_state_create_info_
    ,   VK_POLYGON_MODE_FILL
    ,   VK_CULL_MODE_NONE
    ,   VK_FRONT_FACE_COUNTER_CLOCKWISE
    ) ;

    fill_pipeline_multisample_state_create_info(
        &vr->pipeline_multisample_state_create_info_
    ,   vc->enable_sampling_
    ,   vc->sample_count_
    ,   vc->enable_sample_shading_
    ,   vc->min_sample_shading_
    ) ;

    fill_pipeline_color_blend_attachment_state(
        &vr->pipeline_color_blend_attachment_state_
    ,   VK_FALSE
    ) ;

    fill_pipeline_color_blend_state_create_info(
        &vr->pipeline_color_blend_state_create_info_
    ,   VK_TRUE
    ,   VK_LOGIC_OP_COPY
    ,   &vr->pipeline_color_blend_attachment_state_
    ) ;

    fill_pipeline_depth_stencil_state_create_info(
        &vr->pipeline_depth_stencil_state_create_info_
    ,   VK_TRUE
    ,   VK_TRUE
    ,   VK_COMPARE_OP_LESS
    ) ;

    fill_graphics_pipeline_create_info(
        &vr->graphics_pipeline_create_info_
    ,   vr->pipeline_layout_
    ,   vc->render_pass_
    ,   vr->pipeline_shader_stage_create_infos_
    ,   vr->pipeline_shader_stage_create_infos_count_
    ,   &vr->pipeline_vertex_input_state_create_info_
    ,   &vr->pipeline_input_assembly_state_create_info_
    ,   &vr->pipeline_viewport_state_create_info_
    ,   &vr->pipeline_rasterization_state_create_info_
    ,   &vr->pipeline_multisample_state_create_info_
    ,   &vr->pipeline_depth_stencil_state_create_info_
    ,   &vr->pipeline_color_blend_state_create_info_
    ,   &vr->pipeline_dynamic_state_create_info_
    ) ;

    // VkResult vkCreateGraphicsPipelines(
    //     VkDevice                                    device,
    //     VkPipelineCache                             pipelineCache,
    //     uint32_t                                    createInfoCount,
    //     const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkPipeline*                                 pPipelines);
    if(check_vulkan(vkCreateGraphicsPipelines(
                vc->device_
            ,   VK_NULL_HANDLE
            ,   1
            ,   &vr->graphics_pipeline_create_info_
            ,   NULL
            ,   &vr->graphics_pipeline_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vr->graphics_pipeline_) ;

    vkDestroyShaderModule(vc->device_, vr->vert_shader_, NULL) ;
    vr->vert_shader_ = NULL ;
    vkDestroyShaderModule(vc->device_, vr->frag_shader_, NULL) ;
    vr->frag_shader_ = NULL ;

    end_timed_block() ;
    return true ;
}


void
make_rob_test(
    vulkan_render_object *  out_rob
)
{
    require(out_rob) ;

    out_rob->create_func_   = create_rob ;
    out_rob->draw_func_     = draw_rob ;
    out_rob->update_func_   = update_rob ;
    out_rob->record_func_   = record_rob ;
    out_rob->destroy_func_  = destroy_rob ;
    out_rob->param_         = &the_vulkan_rob_ ;
    out_rob->vc_            = NULL ;
}

