# 把各种全局状态和全局变量都设置一下，明确一下每个东西到底是干啥的

# 目录名
set(JOKER_NAME_3RD "3rd")
set(JOKER_NAME_ENGINE "engine")
set(JOKER_NAME_EXAMPLE "example")
set(JOKER_NAME_EDITOR "editor")
set(JOKER_NAME_MODULE "module")
set(JOKER_NAME_CMAKE "cmake")

# 模块列表
set(JOKER_TARGETS_3RD)
set(JOKER_TARGETS_ENGINE)
set(JOKER_TARGETS_EXAMPLE)
set(JOKER_TARGETS_EDITOR)
set(JOKER_TARGETS_MODULE)
set(JOKER_TARGETS_CMAKE)

# 需要使用Ninja或者Makefile配合clangd来做代码补全
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 给CMamke的文件目录重命名一下
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER ${JOKER_NAME_CMAKE})

# 开启工程目录设置
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include("tools/cmake/CommonVar.cmake")
include("tools/cmake/CommonMacros.cmake")
include("tools/cmake/CommonFunctions.cmake")