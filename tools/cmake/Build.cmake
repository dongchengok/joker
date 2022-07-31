# 阻止多次执行
include_guard(GLOBAL)

# 添加依赖的sdk，不需要自己编译的
joker_add_subdirectories_glob("sdk/*")
# 添加三方库依赖
joker_add_subdirectories_glob("3rd/*")
# 添加正品工程
joker_add_subdirectories_glob("code/joker/*")
joker_add_subdirectories_glob("code/test/*")