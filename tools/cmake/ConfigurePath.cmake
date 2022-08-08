
# 设置一下解决方案目录
set(JOKER_SOLUTION_DIR "${CMAKE_SOURCE_DIR}")
# 设置工程目录
set(JOKER_PROJECT_DIR "${CMAKE_SOURCE_DIR}")
# 设置需要编译的第三方库目录
set(JOKER_3RD_DIR "${JOKER_SOLUTION_DIR}/3rd")
# 设置需要安装或者依赖的sdk目录
set(JOKER_SDK_DIR "${JOKER_SOLUTION_DIR}/sdk")
# 设置CMake工具目录
set(JOKER_CMAKE_DIR "${JOKER_SOLUTION_DIR}/tools/cmake")
# 设置CMake其它模块目录
set(JOKER_CMAKE_MODULE_DIR "${JOKER_CMAKE_DIR}/modules")

# 处理路径分隔符
file(TO_CMAKE_PATH "${JOKER_SOLUTION_DIR}" JOKER_SOLUTION_DIR)
file(TO_CMAKE_PATH "${JOKER_PROJECT_DIR}" JOKER_PROJECT_DIR)
file(TO_CMAKE_PATH "${JOKER_3RD_DIR}" JOKER_3RD_DIR)
file(TO_CMAKE_PATH "${JOKER_SDK_DIR}" JOKER_SDK_DIR)
file(TO_CMAKE_PATH "${JOKER_CMAKE_DIR}" JOKER_CMAKE_DIR)
file(TO_CMAKE_PATH "${JOKER_CMAKE_MODULE_DIR}" JOKER_CMAKE_MODULE_DIR)

# 打印目录
message(STATUS "===========================JOKER CMAKE PATH===========================")
message(STATUS "JOKER_SOLUTION_DIR = ${JOKER_SOLUTION_DIR}")
message(STATUS "JOKER_PROJECT_DIR = ${JOKER_PROJECT_DIR}")
message(STATUS "JOKER_3RD_DIR = ${JOKER_3RD_DIR}")
message(STATUS "JOKER_SDK_DIR = ${JOKER_SDK_DIR}")
message(STATUS "JOKER_CMAKE_DIR = ${JOKER_CMAKE_DIR}")
message(STATUS "JOKER_CMAKE_MODULE_DIR = ${JOKER_CMAKE_MODULE_DIR}")
message(STATUS "======================================================================")