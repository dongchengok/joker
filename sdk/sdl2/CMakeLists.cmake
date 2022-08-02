# 如果没有找到Vulkan的话先找下Vulkan，优先用Vulkan里的sdl2
if(NOT Vulkan_FOUND)
    find_package(Vulkan REQUIRED)
endif()

if(Vulkan_FOUND)
    # 新老版本的vulkan目录不一样，真讨厌
    find_path(SDL2_INCLUDE_DIR
        NAMES SDL2/SDL.h
        HINTS
        "$ENV{VULKAN_SDK}/Third-Party/Include"
        "$ENV{VULKAN_SDK}/Include"
        )
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            # 64位系统
            find_library(SDL2_LIBRARY
                NAMES SDL2 SDL2main
                HINTS
                "$ENV{VULKAN_SDK}/Third-Party/Bin"
                "$ENV{VULKAN_SDK}/Bin"
                "$ENV{VULKAN_SDK}/Lib"
                )
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            # 32位系统
            find_library(SDL2_LIBRARY
            NAMES SDL2 SDL2main
                HINTS
                "$ENV{VULKAN_SDK}/Third-Party/Bin32"
                "$ENV{VULKAN_SDK}/Bin32"
                "$ENV{VULKAN_SDK}/Lib32"
                )
        endif()

        set(SDL2_LIBRARIES ${SDL2_LIBRARY})
        set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})

        add_library(SDL2::SDL2 UNKNOWN IMPORTED)
        set_target_properties(SDL2::SDL2  PROPERTIES
            IMPORTED_LOCATION "${SDL2_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}")
    else()
        # 其它暂时不支持
        message(FATAL_ERROR "need support!")
    endif()
else()
    # 暂时不支持其它的sdl2版本
    message(FATAL_ERROR "find sdl2 fatal, need install vulkan sdk.")
endif()