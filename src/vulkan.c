#include "app.h"
#include "vulkan.h"
#include "types.h"
#include "check.h"
#include "log.h"
#include "debug.h"


#include <SDL3/SDL_vulkan.h>


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static vulkan_context   the_vulkan_context_ = { 0 } ;
static vulkan_context * vc_ = &the_vulkan_context_ ;


static char const vk_layer_khronos_validation_name[]            = "VK_LAYER_KHRONOS_validation" ;
static char const vk_ext_debug_utils_extension_name[]           = VK_EXT_DEBUG_UTILS_EXTENSION_NAME ;
static char const vk_create_debug_utils_messenger_ext_name[]    = "vkCreateDebugUtilsMessengerEXT" ;
static char const vk_destroy_debug_utils_messenger_ext_name[]   = "vkDestroyDebugUtilsMessengerEXT" ;


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT          messageSeverity
,   VkDebugUtilsMessageTypeFlagsEXT                 messageType
,   VkDebugUtilsMessengerCallbackDataEXT const *    pCallbackData
,   void *                                          pUserData
)
{
    UNUSED(messageSeverity) ;
    UNUSED(messageType) ;
    UNUSED(pUserData) ;

    //std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    if(pCallbackData)
    {
        if(pCallbackData->pMessage)
        {
            log_debug_str(pCallbackData->pMessage) ;
        }
    }
    return VK_FALSE;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static void
dump_char_star_array(
    char const * const * const  s
,   uint32_t const              n
)
{
    for(uint32_t i = 0 ; i < n ; ++i)
    {
        log_debug_u32(i) ;
        require(s) ;
        log_debug_str(s[i]) ;
    }
}


static void
dump_extension_property(
    VkExtensionProperties const *   extprop
)
{
    require(extprop) ;

    // typedef struct VkExtensionProperties {
    //     char        extensionName[VK_MAX_EXTENSION_NAME_SIZE];
    //     uint32_t    specVersion;
    // } VkExtensionProperties;
    log_debug_str(extprop->extensionName) ;
    log_debug_u32(extprop->specVersion) ;
}


static void
dump_extension_properties(
    VkExtensionProperties const *   extprop
,   uint32_t const                  extprop_count
)
{
    log_debug_u32(extprop_count) ;
    for(
        uint32_t i = 0
    ;   i < extprop_count
    ;   ++i
    )
    {

        log_debug_u32(i) ;
        require(extprop) ;
        dump_extension_property(&extprop[i]) ;
    }
}


static bool
has_extension(
    VkExtensionProperties const *   haystack
,   uint32_t const                  haystack_count
,   char const *                    needle
)
{
    for(
        uint32_t i = 0
    ;   i < haystack_count
    ;   ++i
    )
    {
        require(haystack) ;
        require(needle) ;
        if(0 == SDL_strcmp(haystack[i].extensionName, needle))
        {
            return true ;
        }
    }
    return false ;
}


static bool
has_all_extensions(
    VkExtensionProperties const *   haystack
,   uint32_t const                  haystack_count
,   char const * const *            needles
,   uint32_t const                  needles_count
)
{
    uint32_t found_count = 0 ;
    for(
        uint32_t i = 0
    ;   i < needles_count
    ;   ++i
    )
    {
        require(needles) ;
        found_count += has_extension(haystack, haystack_count, needles[i]) ;
    }

    return needles_count == found_count ;
}


static void
add_to_extensions(
    char const **   extensions
,   uint32_t *      extensions_count
,   char const *    desired_extension
)
{
    require(*extensions_count < max_vulkan_desired_extensions) ;
    extensions[*extensions_count] = desired_extension ;
    ++*extensions_count ;
}


static void
dump_layer_property(
    VkLayerProperties const *   lp
)
{
    require(lp) ;
    // typedef struct VkLayerProperties {
    //     char        layerName[VK_MAX_EXTENSION_NAME_SIZE];
    //     uint32_t    specVersion;
    //     uint32_t    implementationVersion;
    //     char        description[VK_MAX_DESCRIPTION_SIZE];
    // } VkLayerProperties;
    log_debug_str(lp->layerName) ;
    log_debug_u32(lp->specVersion) ;
    log_debug_u32(lp->implementationVersion) ;
    log_debug_str(lp->description) ;
}


static void
dump_layer_properties(
    VkLayerProperties const *   lp
,   uint32_t const              lp_count
)
{
    log_debug_u32(lp_count) ;
    for(
        uint32_t i = 0
    ;   i < lp_count
    ;   ++i
    )
    {
        log_debug_u32(i) ;
        require(lp) ;
        dump_layer_property(&lp[i]) ;
    }
}


static bool
has_layer_property(
    VkLayerProperties const *   haystack
,   uint32_t const              haystack_count
,   char const *                needle
)
{
    for(
        uint32_t i = 0
    ;   i < haystack_count
    ;   ++i
    )
    {
        require(haystack) ;
        require(needle) ;
        if(0 == SDL_strcmp(haystack[i].layerName, needle))
        {
            return true ;
        }
    }
    return false ;
}


static bool
has_all_layer_properties(
    VkLayerProperties const *   haystack
,   uint32_t const              haystack_count
,   char const * const *        needles
,   uint32_t const              needles_count
)
{
    uint32_t found_count = 0 ;
    for(
        uint32_t i = 0
    ;   i < needles_count
    ;   ++i
    )
    {
        require(needles) ;
        found_count += has_layer_property(haystack, haystack_count, needles[i]) ;
    }

    return needles_count == found_count ;
}


static void
add_to_layers(
    char const **   layers
,   uint32_t *      layers_count
,   char const *    desired_layer
)
{
    require(*layers_count < max_vulkan_desired_layers) ;
    layers[*layers_count] = desired_layer ;
    ++*layers_count ;
}


static bool
create_instance_extensions_properties(
    VkExtensionProperties **    out_instance_extensions_properties
,   uint32_t *                  out_instance_extensions_properties_count
)
{
    require(out_instance_extensions_properties) ;
    require(out_instance_extensions_properties_count) ;
    begin_timed_block() ;

    // VkResult vkEnumerateInstanceExtensionProperties(
    //     const char*                                 pLayerName,
    //     uint32_t*                                   pPropertyCount,
    //     VkExtensionProperties*                      pProperties);
    if(check_vulkan(vkEnumerateInstanceExtensionProperties(
                NULL
            ,   out_instance_extensions_properties_count
            ,   NULL
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(check(*out_instance_extensions_properties = alloc_array(
                VkExtensionProperties
            ,   *out_instance_extensions_properties_count
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    //require(*out_instance_extensions_properties) ;

    if(check_vulkan(vkEnumerateInstanceExtensionProperties(
                NULL
            ,   out_instance_extensions_properties_count
            ,   *out_instance_extensions_properties
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
create_layer_properties(
    VkLayerProperties **    out_layer_properties
,   uint32_t *              out_layer_properties_count
)
{
    require(out_layer_properties) ;
    require(out_layer_properties_count) ;
    begin_timed_block() ;

    // VkResult vkEnumerateInstanceLayerProperties(
    //     uint32_t*                                   pPropertyCount,
    //     VkLayerProperties*                          pProperties);
    if(check_vulkan(
            vkEnumerateInstanceLayerProperties(
                out_layer_properties_count
            ,   NULL
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_layer_properties_count) ;

    if(check(*out_layer_properties = alloc_array(
                VkLayerProperties
            ,   *out_layer_properties_count
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    //require(*out_layer_properties) ;

    if(check_vulkan(
            vkEnumerateInstanceLayerProperties(
                out_layer_properties_count
            ,   *out_layer_properties
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
create_function_pointers(
    VkInstance          instance
,   vulkan_context *    vc
)
{
    require(instance) ;
    require(vc) ;

    vc->create_debug_messenger_func_ = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance
    ,   vk_create_debug_utils_messenger_ext_name
    ) ;

    require(vc->create_debug_messenger_func_) ;

    vc->destroy_debug_messenger_func_ = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance
    ,   vk_destroy_debug_utils_messenger_ext_name
    ) ;

    require(vc->destroy_debug_messenger_func_) ;

    return true ;
}


static bool
create_debug_messenger(
    VkDebugUtilsMessengerEXT *          out_debug_messenger
,   VkInstance                          instance
,   PFN_vkCreateDebugUtilsMessengerEXT  create_func
)
{
    require(out_debug_messenger) ;
    require(instance) ;
    require(create_func) ;

    // typedef struct VkDebugUtilsMessengerCreateInfoEXT {
    //     VkStructureType                         sType;
    //     const void*                             pNext;
    //     VkDebugUtilsMessengerCreateFlagsEXT     flags;
    //     VkDebugUtilsMessageSeverityFlagsEXT     messageSeverity;
    //     VkDebugUtilsMessageTypeFlagsEXT         messageType;
    //     PFN_vkDebugUtilsMessengerCallbackEXT    pfnUserCallback;
    //     void*                                   pUserData;
    // } VkDebugUtilsMessengerCreateInfoEXT;
    static VkDebugUtilsMessengerCreateInfoEXT dumcie = { 0 } ;
    dumcie.sType            = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT ;
    dumcie.messageSeverity  = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ;
    dumcie.messageType      = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ;
    dumcie.pfnUserCallback  = vulkan_debug_callback ;
    dumcie.pUserData        = NULL ;

    // VkResult vkCreateDebugUtilsMessengerEXT(
    //     VkInstance                                  instance,
    //     const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkDebugUtilsMessengerEXT*                   pMessenger);
    if(check_vulkan(create_func(
                instance
            ,   &dumcie
            ,   NULL
            ,   out_debug_messenger
            )
        )
    )
    {
        return false ;
    }

    return true ;
}




static bool
create_vulkan_instance(
    VkInstance *            out_instance
,   char const * const *    desired_instance_extensions
,   uint32_t const          desired_instance_extensions_count
,   char const * const *    desired_layers
,   uint32_t const          desired_layers_count
,   VkBool32 const          enable_validation
)
{
    require(out_instance) ;
    begin_timed_block() ;


    // typedef struct VkApplicationInfo {
    //     VkStructureType    sType;
    //     const void*        pNext;
    //     const char*        pApplicationName;
    //     uint32_t           applicationVersion;
    //     const char*        pEngineName;
    //     uint32_t           engineVersion;
    //     uint32_t           apiVersion;
    // } VkApplicationInfo;
    static VkApplicationInfo   application_info = { 0 } ;
    application_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO ;
    application_info.pNext              = NULL ;
    application_info.pApplicationName   = "threed" ;
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0) ;
    application_info.pEngineName        = "No Engine" ;
    application_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0) ;
    application_info.apiVersion         = VK_API_VERSION_1_0 ;


    // typedef struct VkInstanceCreateInfo {
    //     VkStructureType             sType;
    //     const void*                 pNext;
    //     VkInstanceCreateFlags       flags;
    //     const VkApplicationInfo*    pApplicationInfo;
    //     uint32_t                    enabledLayerCount;
    //     const char* const*          ppEnabledLayerNames;
    //     uint32_t                    enabledExtensionCount;
    //     const char* const*          ppEnabledExtensionNames;
    // } VkInstanceCreateInfo;
    static VkInstanceCreateInfo instance_create_info = { 0 } ;
    instance_create_info.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO ;
    instance_create_info.pNext                      = NULL ;
    instance_create_info.flags                      = 0 ;
    instance_create_info.pApplicationInfo           = &application_info ;
    instance_create_info.enabledLayerCount          = 0 ;
    instance_create_info.ppEnabledLayerNames        = NULL ;
    if(enable_validation)
    {
    instance_create_info.enabledLayerCount          = desired_layers_count ;
    instance_create_info.ppEnabledLayerNames        = desired_layers ;
    }
    instance_create_info.enabledExtensionCount      = desired_instance_extensions_count ;
    instance_create_info.ppEnabledExtensionNames    = desired_instance_extensions ;


    // VkResult vkCreateInstance(
    //     const VkInstanceCreateInfo*                 pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkInstance*                                 pInstance);
    if(check_vulkan(vkCreateInstance(
                &instance_create_info
            ,   NULL
            ,   out_instance
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_instance) ;

    end_timed_block() ;
    return true ;
}


static bool
destroy_vulkan_instance(
    vulkan_context *    vc
)
{
    require(vc) ;
    begin_timed_block() ;

    if(vc->instance_)
    {
        vkDestroyInstance(vc->instance_, NULL) ;
        vc->instance_ = NULL ;
    }

    end_timed_block() ;
    return true ;
}


int
create_vulkan()
{
    begin_timed_block() ;

    vc_->enable_validation_ = VK_TRUE ;


    vc_->platform_instance_extensions_ = SDL_Vulkan_GetInstanceExtensions(
        &vc_->platform_instance_extensions_count_
    ) ;

    dump_char_star_array(
        vc_->platform_instance_extensions_
    ,   vc_->platform_instance_extensions_count_
    ) ;

    if(check(create_instance_extensions_properties(
                &vc_->instance_extensions_properties_
            ,   &vc_->instance_extensions_properties_count_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    dump_extension_properties(
        vc_->instance_extensions_properties_
    ,   vc_->instance_extensions_properties_count_
    ) ;

    for(
        uint32_t i = 0
    ;   i < vc_->platform_instance_extensions_count_
    ;   ++i
    )
    {
        add_to_extensions(
            vc_->desired_extensions_
        ,   &vc_->desired_extensions_count_
        ,   vc_->platform_instance_extensions_[i]
        ) ;
    }

    if(vc_->enable_validation_)
    {
        add_to_extensions(
            vc_->desired_extensions_
        ,   &vc_->desired_extensions_count_
        ,   vk_ext_debug_utils_extension_name
        ) ;
    }

    dump_char_star_array(
        vc_->desired_extensions_
    ,   vc_->desired_extensions_count_
    ) ;

    check(has_all_extensions(
            vc_->instance_extensions_properties_
        ,   vc_->instance_extensions_properties_count_
        ,   vc_->desired_extensions_
        ,   vc_->desired_extensions_count_
        )
    ) ;

    ///// layers

    if(check(create_layer_properties(
                &vc_->layer_properties_
            ,   &vc_->layer_properties_count_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    dump_layer_properties(
        vc_->layer_properties_
    ,   vc_->layer_properties_count_
    ) ;

    add_to_layers(
        vc_->desired_layers_
    ,   &vc_->desired_layers_count_
    ,   vk_layer_khronos_validation_name
    ) ;

    dump_char_star_array(
        vc_->desired_layers_
    ,   vc_->desired_layers_count_
    ) ;

    check(has_all_layer_properties(
            vc_->layer_properties_
        ,   vc_->layer_properties_count_
        ,   vc_->desired_layers_
        ,   vc_->desired_layers_count_
        )
    ) ;

    if(check(create_vulkan_instance(
                &vc_->instance_
            ,   vc_->desired_extensions_
            ,   vc_->desired_extensions_count_
            ,   vc_->desired_layers_
            ,   vc_->desired_layers_count_
            ,   vc_->enable_validation_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    check(create_function_pointers(vc_->instance_, vc_)) ;

    if(check(create_debug_messenger(
                &vc_->debug_messenger_
            ,   vc_->instance_
            ,   vc_->create_debug_messenger_func_
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


void
destroy_vulkan()
{
    begin_timed_block() ;

    destroy_vulkan_instance(vc_) ;

    end_timed_block() ;
}
