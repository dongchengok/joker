# 必须使用C11版本，不许用老的
# 必须使用C++17版本，不许用老的
# 禁用扩展，好像CYGWIN要打开，谁能告诉我到底干啥了
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置默认的工程配置
set(JOKER_BUILD_SYSTEM)
set(JOKER_BUILD_OUT_DIR_NAME)
set(JOKER_BUILD_CPU_ARCHITECTURE)

message(STATUS "===========================JOKER CMAKE BUILD===========================")
message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
# message(STATUS "CMAKE_SYSTEM = ${CMAKE_SYSTEM}")
# message(STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
# message(STATUS "CMAKE_SYSTEM_VERSION = ${CMAKE_SYSTEM_VERSION}")
# message(STATUS "BUILD_PLATFORM = ${BUILD_PLATFORM}")
# message(STATUS "BUILD_CPU_ARCHITECTURE = ${BUILD_CPU_ARCHITECTURE}")
# message(STATUS "OUTPUT_DIRECTORY_NAME = ${OUTPUT_DIRECTORY_NAME}")

# CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CMAKE_BUILD_TYPE}
# CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY_<CONFIG>
# CMAKE_LIBRARY_OUTPUT_DIRECTORY_<CONFIG>
# CMAKE_RUNTIME_OUTPUT_DIRECTORY_<CONFIG>