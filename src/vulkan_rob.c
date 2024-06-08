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



typedef struct vulkan_rob
{
    VkDescriptorSetLayoutBinding    descriptor_set_layout_bindings_[max_vulkan_descriptor_set_layout_binding] ;
    uint32_t                        descriptor_set_layout_bindings_count_ ;
    VkDescriptorSetLayout           descriptor_set_layout_ ;
    VkDescriptorSet                 descriptor_sets_[max_vulkan_frames_in_flight] ;


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


} vulkan_rob ;



static vulkan_rob   the_vulkan_rob_ = { 0 } ;



typedef struct uniform_buffer_object
{
    mat4 model ;
    mat4 view ;
    mat4 proj ;
} uniform_buffer_object ;

static uint32_t const uniform_buffer_object_size = sizeof(uniform_buffer_object) ;


#if 0
    for(
        uint32_t i = 0
    ;   i < frames_in_flight_count
    ;   ++i
    )
    {
        require(vc->descriptor_sets_[i]) ;

        // typedef struct VkDescriptorBufferInfo {
        //     VkBuffer        buffer;
        //     VkDeviceSize    offset;
        //     VkDeviceSize    range;
        // } VkDescriptorBufferInfo;
        VkDescriptorBufferInfo dbi = { 0 } ;
        dbi.buffer  = vc->uniform_buffers_[i] ;
        dbi.offset  = 0 ;
        dbi.range   = uniform_buffer_object_size ;


        // typedef struct VkDescriptorImageInfo {
        //     VkSampler        sampler;
        //     VkImageView      imageView;
        //     VkImageLayout    imageLayout;
        // } VkDescriptorImageInfo;
        VkDescriptorImageInfo dii = { 0 } ;
        dii.sampler     = vc->texture_sampler_ ;
        dii.imageView   = vc->texture_image_view_ ;
        dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ;


        // typedef struct VkWriteDescriptorSet {
        //     VkStructureType                  sType;
        //     const void*                      pNext;
        //     VkDescriptorSet                  dstSet;
        //     uint32_t                         dstBinding;
        //     uint32_t                         dstArrayElement;
        //     uint32_t                         descriptorCount;
        //     VkDescriptorType                 descriptorType;
        //     const VkDescriptorImageInfo*     pImageInfo;
        //     const VkDescriptorBufferInfo*    pBufferInfo;
        //     const VkBufferView*              pTexelBufferView;
        // } VkWriteDescriptorSet;
        VkWriteDescriptorSet wds[2] = { 0 } ;
        wds[0].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET ;
        wds[0].pNext               = NULL ;
        wds[0].dstSet              = vc->descriptor_sets_[i] ;
        wds[0].dstBinding          = 0 ;
        wds[0].dstArrayElement     = 0 ;
        wds[0].descriptorCount     = 1 ;
        wds[0].descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ;
        wds[0].pImageInfo          = NULL ;
        wds[0].pBufferInfo         = &dbi ;
        wds[0].pTexelBufferView    = NULL ;

        wds[1].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET ;
        wds[1].pNext               = NULL ;
        wds[1].dstSet              = vc->descriptor_sets_[i] ;
        wds[1].dstBinding          = 1 ;
        wds[1].dstArrayElement     = 0 ;
        wds[1].descriptorCount     = 1 ;
        wds[1].descriptorType      = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ;
        wds[1].pImageInfo          = &dii ;
        wds[1].pBufferInfo         = NULL ;
        wds[1].pTexelBufferView    = NULL ;

        // void vkUpdateDescriptorSets(
        //     VkDevice                                    device,
        //     uint32_t                                    descriptorWriteCount,
        //     const VkWriteDescriptorSet*                 pDescriptorWrites,
        //     uint32_t                                    descriptorCopyCount,
        //     const VkCopyDescriptorSet*                  pDescriptorCopies);
        vkUpdateDescriptorSets(
            vc->device_
        ,   array_count(wds)
        ,   wds
        ,   0
        ,   NULL
        ) ;


    }

#endif


bool
create_rob(
    vulkan_context *    vc
)
{
    require(vc) ;
    begin_timed_block() ;

    vulkan_rob *    vr = &the_vulkan_rob_ ;

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

    // 1 == means no mip maps
    // 0 == auto mipmap generation

    uint32_t mip_levels = 1 ;
    if(
        vc->picked_physical_device_->swapchain_support_details_.formats_properties_->optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
    {
        mip_levels = 0 ;
    }

    if(check(create_texture_image(
                &vr->texture_image_
            ,   &vr->texture_image_memory_
            ,   "ass/textures/statue-1275469_1280.jpg"
            ,   vc->device_
            ,   vc->command_pool_
            ,   vc->graphics_queue_
            ,   &vc->picked_physical_device_->memory_properties_
            ,   mip_levels
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
