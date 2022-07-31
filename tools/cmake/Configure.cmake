include(tools/cmake/ConfigurePath.cmake)
include(tools/cmake/ConfigureBuildOptions.cmake)
include(tools/cmake/ConfigureBuildType.cmake)

if(WIN32)
    include(tools/cmake/compiler/Compiler-MSVC.cmake)
else()
    message(FATAL_ERROR "unsupport toolchain!")
endif()