#include "app.h"
#include "vulkan.h"
#include "defines.h"
#include "debug.h"
#include "check.h"


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
    vec2 pos ;
    vec2 uv ;
} vertex ;

static uint32_t const vertex_size = sizeof(vertex) ;




static mat4 const frames[] =
{
  {
    { 21.000000, 3.000000, 0.000488, 0.000977 }
  , { 203.000000, 3.000000, 0.089355, 0.000977 }
  , { 203.000000, 157.000000, 0.089355, 0.151367 }
  , { 21.000000, 157.000000, 0.000488, 0.151367 }
  }
, {
    { 15.000000, 3.000000, 0.000488, 0.152344 }
  , { 197.000000, 3.000000, 0.089355, 0.152344 }
  , { 197.000000, 157.000000, 0.089355, 0.302734 }
  , { 15.000000, 157.000000, 0.000488, 0.302734 }
  }
, {
    { 10.000000, 3.000000, 0.000488, 0.303711 }
  , { 191.000000, 3.000000, 0.088867, 0.303711 }
  , { 191.000000, 157.000000, 0.088867, 0.454102 }
  , { 10.000000, 157.000000, 0.000488, 0.454102 }
  }
, {
    { 6.000000, 3.000000, 0.000488, 0.455078 }
  , { 188.000000, 3.000000, 0.089355, 0.455078 }
  , { 188.000000, 157.000000, 0.089355, 0.605469 }
  , { 6.000000, 157.000000, 0.000488, 0.605469 }
  }
, {
    { 3.000000, 3.000000, 0.089355, 0.303711 }
  , { 189.000000, 3.000000, 0.180176, 0.303711 }
  , { 189.000000, 157.000000, 0.180176, 0.454102 }
  , { 3.000000, 157.000000, 0.089355, 0.454102 }
  }
, {
    { 1.000000, 3.000000, 0.091797, 0.606445 }
  , { 189.000000, 3.000000, 0.183594, 0.606445 }
  , { 189.000000, 156.000000, 0.183594, 0.755859 }
  , { 1.000000, 156.000000, 0.091797, 0.755859 }
  }
, {
    { 0.000000, 2.000000, 0.089844, 0.000977 }
  , { 188.000000, 2.000000, 0.181641, 0.000977 }
  , { 188.000000, 156.000000, 0.181641, 0.151367 }
  , { 0.000000, 156.000000, 0.089844, 0.151367 }
  }
, {
    { 0.000000, 2.000000, 0.179688, 0.455078 }
  , { 187.000000, 2.000000, 0.270996, 0.455078 }
  , { 187.000000, 155.000000, 0.270996, 0.604492 }
  , { 0.000000, 155.000000, 0.179688, 0.604492 }
  }
, {
    { 2.000000, 2.000000, 0.180664, 0.303711 }
  , { 185.000000, 2.000000, 0.270020, 0.303711 }
  , { 185.000000, 155.000000, 0.270020, 0.453125 }
  , { 2.000000, 155.000000, 0.180664, 0.453125 }
  }
, {
    { 5.000000, 2.000000, 0.182129, 0.151367 }
  , { 182.000000, 2.000000, 0.268555, 0.151367 }
  , { 182.000000, 154.000000, 0.268555, 0.299805 }
  , { 5.000000, 154.000000, 0.182129, 0.299805 }
  }
, {
    { 10.000000, 1.000000, 0.177734, 0.756836 }
  , { 181.000000, 1.000000, 0.261230, 0.756836 }
  , { 181.000000, 154.000000, 0.261230, 0.906250 }
  , { 10.000000, 154.000000, 0.177734, 0.906250 }
  }
, {
    { 16.000000, 1.000000, 0.269043, 0.151367 }
  , { 180.000000, 1.000000, 0.349121, 0.151367 }
  , { 180.000000, 153.000000, 0.349121, 0.299805 }
  , { 16.000000, 153.000000, 0.269043, 0.299805 }
  }
, {
    { 24.000000, 1.000000, 0.353516, 0.000977 }
  , { 178.000000, 1.000000, 0.428711, 0.000977 }
  , { 178.000000, 152.000000, 0.428711, 0.148438 }
  , { 24.000000, 152.000000, 0.353516, 0.148438 }
  }
, {
    { 33.000000, 1.000000, 0.271484, 0.450195 }
  , { 176.000000, 1.000000, 0.341309, 0.450195 }
  , { 176.000000, 151.000000, 0.341309, 0.596680 }
  , { 33.000000, 151.000000, 0.271484, 0.596680 }
  }
, {
    { 44.000000, 0.000000, 0.272949, 0.597656 }
  , { 177.000000, 0.000000, 0.337891, 0.597656 }
  , { 177.000000, 150.000000, 0.337891, 0.744141 }
  , { 44.000000, 150.000000, 0.272949, 0.744141 }
  }
, {
    { 52.000000, 0.000000, 0.338379, 0.597656 }
  , { 179.000000, 0.000000, 0.400391, 0.597656 }
  , { 179.000000, 150.000000, 0.400391, 0.744141 }
  , { 52.000000, 150.000000, 0.338379, 0.744141 }
  }
, {
    { 52.000000, 0.000000, 0.352539, 0.745117 }
  , { 180.000000, 0.000000, 0.415039, 0.745117 }
  , { 180.000000, 149.000000, 0.415039, 0.890625 }
  , { 52.000000, 149.000000, 0.352539, 0.890625 }
  }
, {
    { 52.000000, 0.000000, 0.405762, 0.447266 }
  , { 181.000000, 0.000000, 0.468750, 0.447266 }
  , { 181.000000, 148.000000, 0.468750, 0.591797 }
  , { 52.000000, 148.000000, 0.405762, 0.591797 }
  }
, {
    { 52.000000, 0.000000, 0.435547, 0.149414 }
  , { 184.000000, 0.000000, 0.500000, 0.149414 }
  , { 184.000000, 147.000000, 0.500000, 0.292969 }
  , { 52.000000, 147.000000, 0.435547, 0.292969 }
  }
, {
    { 50.000000, 0.000000, 0.463867, 0.592773 }
  , { 186.000000, 0.000000, 0.530273, 0.592773 }
  , { 186.000000, 146.000000, 0.530273, 0.735352 }
  , { 50.000000, 146.000000, 0.463867, 0.735352 }
  }
, {
    { 42.000000, 0.000000, 0.500488, 0.149414 }
  , { 188.000000, 0.000000, 0.571777, 0.149414 }
  , { 188.000000, 145.000000, 0.571777, 0.291016 }
  , { 42.000000, 145.000000, 0.500488, 0.291016 }
  }
, {
    { 35.000000, 0.000000, 0.479492, 0.736328 }
  , { 189.000000, 0.000000, 0.554688, 0.736328 }
  , { 189.000000, 144.000000, 0.554688, 0.876953 }
  , { 35.000000, 144.000000, 0.479492, 0.876953 }
  }
, {
    { 29.000000, 0.000000, 0.538086, 0.437500 }
  , { 189.000000, 0.000000, 0.616211, 0.437500 }
  , { 189.000000, 143.000000, 0.616211, 0.577148 }
  , { 29.000000, 143.000000, 0.538086, 0.577148 }
  }
, {
    { 22.000000, 0.000000, 0.550781, 0.291992 }
  , { 188.000000, 0.000000, 0.631836, 0.291992 }
  , { 188.000000, 143.000000, 0.631836, 0.431641 }
  , { 22.000000, 143.000000, 0.550781, 0.431641 }
  }
, {
    { 17.000000, 0.000000, 0.555176, 0.730469 }
  , { 191.000000, 0.000000, 0.640137, 0.730469 }
  , { 191.000000, 142.000000, 0.640137, 0.869141 }
  , { 17.000000, 142.000000, 0.555176, 0.869141 }
  }
, {
    { 12.000000, 0.000000, 0.607422, 0.578125 }
  , { 200.000000, 0.000000, 0.699219, 0.578125 }
  , { 200.000000, 142.000000, 0.699219, 0.716797 }
  , { 12.000000, 142.000000, 0.607422, 0.716797 }
  }
, {
    { 8.000000, 0.000000, 0.632324, 0.284180 }
  , { 208.000000, 0.000000, 0.729980, 0.284180 }
  , { 208.000000, 141.000000, 0.729980, 0.421875 }
  , { 8.000000, 141.000000, 0.632324, 0.421875 }
  }
, {
    { 4.000000, 0.000000, 0.654297, 0.141602 }
  , { 214.000000, 0.000000, 0.756836, 0.141602 }
  , { 214.000000, 141.000000, 0.756836, 0.279297 }
  , { 4.000000, 141.000000, 0.654297, 0.279297 }
  }
, {
    { 2.000000, 0.000000, 0.655762, 0.000977 }
  , { 219.000000, 0.000000, 0.761719, 0.000977 }
  , { 219.000000, 141.000000, 0.761719, 0.138672 }
  , { 2.000000, 141.000000, 0.655762, 0.138672 }
  }
, {
    { 0.000000, 0.000000, 0.699707, 0.572266 }
  , { 222.000000, 0.000000, 0.808105, 0.572266 }
  , { 222.000000, 140.000000, 0.808105, 0.708984 }
  , { 0.000000, 140.000000, 0.699707, 0.708984 }
  }
, {
    { 0.000000, 0.000000, 0.745117, 0.709961 }
  , { 224.000000, 0.000000, 0.854492, 0.709961 }
  , { 224.000000, 140.000000, 0.854492, 0.846680 }
  , { 0.000000, 140.000000, 0.745117, 0.846680 }
  }
, {
    { 1.000000, 0.000000, 0.704590, 0.422852 }
  , { 224.000000, 0.000000, 0.813477, 0.422852 }
  , { 224.000000, 140.000000, 0.813477, 0.559570 }
  , { 1.000000, 140.000000, 0.704590, 0.559570 }
  }
, {
    { 4.000000, 0.000000, 0.730469, 0.280273 }
  , { 223.000000, 0.000000, 0.837402, 0.280273 }
  , { 223.000000, 140.000000, 0.837402, 0.416992 }
  , { 4.000000, 140.000000, 0.730469, 0.416992 }
  }
, {
    { 8.000000, 0.000000, 0.640625, 0.717773 }
  , { 221.000000, 0.000000, 0.744629, 0.717773 }
  , { 221.000000, 141.000000, 0.744629, 0.855469 }
  , { 8.000000, 141.000000, 0.640625, 0.855469 }
  }
, {
    { 14.000000, 0.000000, 0.640625, 0.856445 }
  , { 218.000000, 0.000000, 0.740234, 0.856445 }
  , { 218.000000, 141.000000, 0.740234, 0.994141 }
  , { 14.000000, 141.000000, 0.640625, 0.994141 }
  }
, {
    { 21.000000, 0.000000, 0.740723, 0.856445 }
  , { 214.000000, 0.000000, 0.834961, 0.856445 }
  , { 214.000000, 141.000000, 0.834961, 0.994141 }
  , { 21.000000, 141.000000, 0.740723, 0.994141 }
  }
, {
    { 30.000000, 0.000000, 0.616699, 0.432617 }
  , { 209.000000, 0.000000, 0.704102, 0.432617 }
  , { 209.000000, 142.000000, 0.704102, 0.571289 }
  , { 30.000000, 142.000000, 0.616699, 0.571289 }
  }
, {
    { 36.000000, 0.000000, 0.572266, 0.143555 }
  , { 203.000000, 0.000000, 0.653809, 0.143555 }
  , { 203.000000, 143.000000, 0.653809, 0.283203 }
  , { 36.000000, 143.000000, 0.572266, 0.283203 }
  }
, {
    { 35.000000, 0.000000, 0.576172, 0.000977 }
  , { 197.000000, 0.000000, 0.655273, 0.000977 }
  , { 197.000000, 143.000000, 0.655273, 0.140625 }
  , { 35.000000, 143.000000, 0.576172, 0.140625 }
  }
, {
    { 35.000000, 0.000000, 0.530762, 0.588867 }
  , { 191.000000, 0.000000, 0.606934, 0.588867 }
  , { 191.000000, 144.000000, 0.606934, 0.729492 }
  , { 35.000000, 144.000000, 0.530762, 0.729492 }
  }
, {
    { 36.000000, 0.000000, 0.503418, 0.000977 }
  , { 184.000000, 0.000000, 0.575684, 0.000977 }
  , { 184.000000, 145.000000, 0.575684, 0.142578 }
  , { 36.000000, 145.000000, 0.503418, 0.142578 }
  }
, {
    { 37.000000, 0.000000, 0.469238, 0.445312 }
  , { 177.000000, 0.000000, 0.537598, 0.445312 }
  , { 177.000000, 146.000000, 0.537598, 0.587891 }
  , { 37.000000, 146.000000, 0.469238, 0.587891 }
  }
, {
    { 39.000000, 0.000000, 0.485352, 0.293945 }
  , { 172.000000, 0.000000, 0.550293, 0.293945 }
  , { 172.000000, 146.000000, 0.550293, 0.436523 }
  , { 39.000000, 146.000000, 0.485352, 0.436523 }
  }
, {
    { 42.000000, 0.000000, 0.415527, 0.744141 }
  , { 172.000000, 0.000000, 0.479004, 0.744141 }
  , { 172.000000, 147.000000, 0.479004, 0.887695 }
  , { 42.000000, 147.000000, 0.415527, 0.887695 }
  }
, {
    { 43.000000, 0.000000, 0.421875, 0.299805 }
  , { 172.000000, 0.000000, 0.484863, 0.299805 }
  , { 172.000000, 148.000000, 0.484863, 0.444336 }
  , { 43.000000, 148.000000, 0.421875, 0.444336 }
  }
, {
    { 44.000000, 0.000000, 0.400879, 0.597656 }
  , { 172.000000, 0.000000, 0.463379, 0.597656 }
  , { 172.000000, 149.000000, 0.463379, 0.743164 }
  , { 44.000000, 149.000000, 0.400879, 0.743164 }
  }
, {
    { 46.000000, 0.000000, 0.341797, 0.450195 }
  , { 176.000000, 0.000000, 0.405273, 0.450195 }
  , { 176.000000, 150.000000, 0.405273, 0.596680 }
  , { 46.000000, 150.000000, 0.341797, 0.596680 }
  }
, {
    { 48.000000, 1.000000, 0.353516, 0.299805 }
  , { 187.000000, 1.000000, 0.421387, 0.299805 }
  , { 187.000000, 151.000000, 0.421387, 0.446289 }
  , { 48.000000, 151.000000, 0.353516, 0.446289 }
  }
, {
    { 46.000000, 1.000000, 0.429199, 0.000977 }
  , { 197.000000, 1.000000, 0.502930, 0.000977 }
  , { 197.000000, 152.000000, 0.502930, 0.148438 }
  , { 46.000000, 152.000000, 0.429199, 0.148438 }
  }
, {
    { 45.000000, 1.000000, 0.274414, 0.000977 }
  , { 206.000000, 1.000000, 0.353027, 0.000977 }
  , { 206.000000, 153.000000, 0.353027, 0.149414 }
  , { 45.000000, 153.000000, 0.274414, 0.149414 }
  }
, {
    { 43.000000, 1.000000, 0.270508, 0.300781 }
  , { 212.000000, 1.000000, 0.353027, 0.300781 }
  , { 212.000000, 153.000000, 0.353027, 0.449219 }
  , { 43.000000, 153.000000, 0.270508, 0.449219 }
  }
, {
    { 42.000000, 2.000000, 0.349609, 0.150391 }
  , { 217.000000, 2.000000, 0.435059, 0.150391 }
  , { 217.000000, 154.000000, 0.435059, 0.298828 }
  , { 42.000000, 154.000000, 0.349609, 0.298828 }
  }
, {
    { 40.000000, 2.000000, 0.184082, 0.605469 }
  , { 221.000000, 2.000000, 0.272461, 0.605469 }
  , { 221.000000, 155.000000, 0.272461, 0.754883 }
  , { 40.000000, 155.000000, 0.184082, 0.754883 }
  }
, {
    { 38.000000, 2.000000, 0.261719, 0.755859 }
  , { 223.000000, 2.000000, 0.352051, 0.755859 }
  , { 223.000000, 155.000000, 0.352051, 0.905273 }
  , { 38.000000, 155.000000, 0.261719, 0.905273 }
  }
, {
    { 36.000000, 2.000000, 0.089844, 0.152344 }
  , { 224.000000, 2.000000, 0.181641, 0.152344 }
  , { 224.000000, 156.000000, 0.181641, 0.302734 }
  , { 36.000000, 156.000000, 0.089844, 0.302734 }
  }
, {
    { 35.000000, 3.000000, 0.182129, 0.000977 }
  , { 223.000000, 3.000000, 0.273926, 0.000977 }
  , { 223.000000, 156.000000, 0.273926, 0.150391 }
  , { 35.000000, 156.000000, 0.182129, 0.150391 }
  }
, {
    { 35.000000, 3.000000, 0.000488, 0.606445 }
  , { 221.000000, 3.000000, 0.091309, 0.606445 }
  , { 221.000000, 157.000000, 0.091309, 0.756836 }
  , { 35.000000, 157.000000, 0.000488, 0.756836 }
  }
, {
    { 36.000000, 3.000000, 0.089844, 0.455078 }
  , { 219.000000, 3.000000, 0.179199, 0.455078 }
  , { 219.000000, 157.000000, 0.179199, 0.605469 }
  , { 36.000000, 157.000000, 0.089844, 0.605469 }
  }
, {
    { 35.000000, 3.000000, 0.000488, 0.757812 }
  , { 215.000000, 3.000000, 0.088379, 0.757812 }
  , { 215.000000, 157.000000, 0.088379, 0.908203 }
  , { 35.000000, 157.000000, 0.000488, 0.908203 }
  }
, {
    { 29.000000, 3.000000, 0.088867, 0.757812 }
  , { 210.000000, 3.000000, 0.177246, 0.757812 }
  , { 210.000000, 157.000000, 0.177246, 0.908203 }
  , { 29.000000, 157.000000, 0.088867, 0.908203 }
  }
} ;
static uint32_t const frames_count = array_count(frames) ;
//static_require(60 == frames_count, "upsi") ;


// 1433.000000, 586.000000, 0.699707, 0.572266
// 1655.000000, 586.000000, 0.808105, 0.572266
// 1655.000000, 726.000000, 0.808105, 0.708984
// 1433.000000, 726.000000, 0.699707, 0.708984


static float const spw = 256.0f ;
static float const sph = 256.0f ;
static float const half_spw = spw / 2.0f ;
static float const half_sph = sph / 2.0f ;

// static float const p0x = 1.0f ;
// static float const p0y = 1.0f ;
// static float const p1x = 183.0f ;
// static float const p1y = 1.0f ;
// static float const p2x = 183.0f ;
// static float const p2y = 155.0f ;
// static float const p3x = 1.0f ;
// static float const p3y = 155.0f ;

static float const p0x = 0.0f ;
static float const p0y = 0.0f ;
static float const p1x = 0.0f ;
static float const p1y = 0.0f ;
static float const p2x = 0.0f ;
static float const p2y = 0.0f ;
static float const p3x = 0.0f ;
static float const p3y = 0.0f ;


static float const ttw = 1.0f ;
static float const tth = 1.0f ;

// static float const t0u = 0.000488f ;
// static float const t0v = 0.000977f ;
// static float const t1u = 0.089355f ;
// static float const t1v = 0.000977f ;
// static float const t2u = 0.089355f ;
// static float const t2v = 0.151367f ;
// static float const t3u = 0.000488f ;
// static float const t3v = 0.151367f ;

static float const t0u = 0.0f ;
static float const t0v = 0.0f ;
static float const t1u = 0.0f ;
static float const t1v = 0.0f ;
static float const t2u = 0.0f ;
static float const t2v = 0.0f ;
static float const t3u = 0.0f ;
static float const t3v = 0.0f ;


static vertex const vertices[] =
{
    { {p0x, p0y}, {t0u, t0v} }
,   { {p1x, p1y}, {t1u, t1v} }
,   { {p2x, p2y}, {t2u, t2v} }
,   { {p3x, p3y}, {t3u, t3v} }
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


#define max_ubo_instance_count  16

static uint32_t animation_frame_counters[max_ubo_instance_count] = { 0 } ;

typedef struct uniform_buffer_object
{
    vec2 offset_ ;
    vec2 scale_ ;
    vec4 pos_[max_ubo_instance_count][4] ;
} uniform_buffer_object ;


static uint32_t const uniform_buffer_object_size = sizeof(uniform_buffer_object) ;


static uniform_buffer_object ubos[max_vulkan_frames_in_flight] = { 0 } ;



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
    require(60 == frames_count) ;

    static bool once = true ;
    static uint64_t previous_time = 0 ;

    uint64_t const current_time = get_app_time() ;

    if(once)
    {
        once = false ;
        previous_time = current_time ;

        uint32_t sf = 0 ;
        for(
            uint32_t i = 0
        ;   i < max_ubo_instance_count
        ;   ++i
        )
        {
            animation_frame_counters[i] = sf % frames_count ;
            sf += 3 ;
        }
    }

    uint64_t const delta_time = (current_time - previous_time) / 10 ;
    double const fractional_seconds = (double) delta_time * get_performance_frequency_inverse() ;

    float const ox = app_->half_window_width_float_ - half_spw ;
    float const oy = app_->half_window_height_float_ - half_sph ;
    float const oxr = 6.0f * half_spw * sinf(fractional_seconds * 0.5f) ;
    float const oyr = 4.0f * half_sph * sinf(fractional_seconds * 0.5f) ;

    uniform_buffer_object * ubo = &ubos[current_frame] ;

    ubo->offset_[0] = app_->half_window_width_float_ ;
    ubo->offset_[1] = app_->half_window_height_float_ ;
    ubo->scale_[0]  = app_->inverse_half_window_width_float_ ;
    ubo->scale_[1]  = app_->inverse_half_window_height_float_ ;

    float angle = fractional_seconds ;
    float angle_inc = 2.0f * M_PI / max_ubo_instance_count ;

    for(
        uint32_t i = 0
    ;   i < max_ubo_instance_count
    ;   ++i
    )
    {
        uint32_t fc = animation_frame_counters[i] % frames_count ;
        ++animation_frame_counters[i] ;

        float const px = ox + oxr * sinf(angle) ;
        float const py = oy + oyr * cosf(angle) ;

        //ubo->pos_[i][0]    = ox + oxr * sinf(angle) ;
        //ubo->pos_[i][1]    = oy + oyr * cosf(angle) ;
        SDL_memcpy(&ubo->pos_[i], &frames[fc], sizeof(mat4)) ;
        ubo->pos_[i][0][0] += px ;
        ubo->pos_[i][0][1] += py ;
        ubo->pos_[i][1][0] += px ;
        ubo->pos_[i][1][1] += py ;
        ubo->pos_[i][2][0] += px ;
        ubo->pos_[i][2][1] += py ;
        ubo->pos_[i][3][0] += px ;
        ubo->pos_[i][3][1] += py ;

        // ubo->pos_[i][0][0]    = 0 ;
        // ubo->pos_[i][0][1]    = 0 ;
        // ubo->pos_[i][0][2]    = 0 ;
        // ubo->pos_[i][0][3]    = 0 ;
        angle += angle_inc ;
    }

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
    vkCmdDrawIndexed(command_buffer, indices_count, max_ubo_instance_count, 0, 0, 0) ;

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
            ,   "ass/textures/test.png"
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
            ,   "ass/shaders/sprite_animation_shader.vert.spv"
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
            ,   "ass/shaders/sprite_animation_shader.frag.spv"
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
    ,   VK_FORMAT_R32G32_SFLOAT
    ,   offsetof(vertex, pos)
    ) ;

    add_vertex_input_attribute_description(
        vr->vertex_input_attribute_descriptions_
    ,   &vr->vertex_input_attribute_descriptions_count_
    ,   max_vulkan_vertex_input_attribute_descriptions
    ,   1
    ,   0
    ,   VK_FORMAT_R32G32_SFLOAT
    ,   offsetof(vertex, uv)
    ) ;

    // add_vertex_input_attribute_description(
    //     vr->vertex_input_attribute_descriptions_
    // ,   &vr->vertex_input_attribute_descriptions_count_
    // ,   max_vulkan_vertex_input_attribute_descriptions
    // ,   2
    // ,   0
    // ,   VK_FORMAT_R32G32B32_SFLOAT
    // ,   offsetof(vertex, color)
    // ) ;

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
    ,   VK_FALSE
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
make_rob_sprite_animation(
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

