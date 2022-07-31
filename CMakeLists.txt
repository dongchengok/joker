cmake_minimum_required(VERSION 3.14)

project(joker)

include(tools/cmake/InitialSetup.cmake)
include(tools/cmake/Configure.cmake)
include(tools/cmake/Build.cmake)
# include(tools/cmake/ConfigurePath.cmake)
# include(tools/cmake/ConfigureBuildOptions.cmake)
# include(tools/cmake/ConfigurePlatform.cmake)
# include(tools/cmake/ConfigureCompiler.cmake)

# # 初始化依赖资源,该下载
# # include(tools/cmake/InitialResource.cmake)
# # 初始化通用设置
# include(tools/cmake/InitialCommon.cmake)
# # 初始化目录
# include(tools/cmake/InitialPath.cmake)
# # 初始化编译变量
# include(tools/cmake/InitialProperty.cmake)
# # 初始化设置
# include(tools/cmake/InitialOption.cmake)
# # 初始化资源
# include(tools/cmake/InitialResource.cmake)
# # 初始化编译选项
# include(tools/cmake/InitialConfigure.cmake)
# # 初始化需要编译的三方库
# include(tools/cmake/Initial3rd.cmake)
# # 初始化不需要编译的三方SDK，一般是自己安装Find个路径出来
# include(tools/cmake/InitialSdk.cmake)

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
# target_link_libraries(haha PUBLIC EABase)