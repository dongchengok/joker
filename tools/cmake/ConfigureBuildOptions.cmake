
option(JOPTION_RHI_MULTI "支持多渲染接口切换" ON)
option(JOPTION_RHI_NULL "支持Null" ON)
option(JOPTION_RHI_VULKAN "支持Vulkan" ON)
# option(JOPTION_RHI_API)

set(JOPTION_RHI_MULTI OFF)
# set(JOPTION_RHI_NULL ON)

message(STATUS "JOPTION_RHI_MULTI=${JOPTION_RHI_MULTI}")
message(STATUS "JOPTION_RHI_NULL=${JOPTION_RHI_NULL}")
message(STATUS "JOPTION_RHI_VULKAN=${JOPTION_RHI_VULKAN}")

if(JOPTION_RHI_MULTI)
    add_definitions(-DJOPTION_RHI_MULTI)
    if(JOPTION_RHI_NULL)
        add_definitions(-DJOPTION_RHI_NULL=1)
    endif()

    if(JOPTION_RHI_VULKAN)
        add_definitions(-DJOPTION_RHI_VULKAN=2)
    endif()
else()
    add_definitions(-DJOPTION_RHI_NULL=1)
    add_definitions(-DJOPTION_RHI_VULKAN=2)
    add_definitions(-DJOPTION_RHI_API=JOPTION_RHI_VULKAN)
endif()

# 全局关闭stl异常,不同编译器需要配合不同编译选项
add_definitions(-D_HAS_EXCEPTIONS=0)