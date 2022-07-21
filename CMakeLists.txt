cmake_minimum_required(VERSION 3.14)

if(NOT DEFINED JOKERENGINE_DIR)
	set(JOKERENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# 初始化环境
# include(Tools/CMake/InitialSetup.cmake)

# # Setting the project version
# if (NOT VERSION)
# 	if (METADATA_VERSION)
# 		set(VERSION ${METADATA_VERSION})
# 	else()
# 		set(VERSION "1.0.0.0")
# 	endif()
# endif()
# set(METADATA_VERSION ${VERSION} CACHE STRING "Version number for executable metadata" FORCE)

# # Declare the project.
# set(METADATA_PROJECT_NAME "CRYENGINE" CACHE STRING "Name of the solution project")
# project("${METADATA_PROJECT_NAME}_${BUILD_PLATFORM}"
# 		VERSION "${METADATA_VERSION}"
# 		LANGUAGES C CXX)

# include(Tools/CMake/Configure.cmake)

# 需要使用Ninja或者Makefile配合clangd来做代码补全
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(3rd)

project(joker)
add_executable(haha main.cpp)

target_link_libraries(haha PUBLIC EASTL)
# target_link_libraries(haha PUBLIC EABase)