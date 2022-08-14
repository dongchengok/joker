function(joker_project_use_pch target_project)
    # 根据工程名检查是否存在预编译头
    get_target_property(_project_name ${target_project} NAME)
    get_target_property(_project_folder ${target_project} SOURCE_DIR)
    set(_PCH_PATH "${_project_folder}/${_project_name}PCH")
    set(_PCH_HEADER "${_project_name}PCH.h")
    set(_PCH_SOURCE "${_project_name}PCH.cpp")

    # VS工程和其它工程PCH目录差异
    if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
        # Inside Visual Studio
        set(_PCH_FILE "$(IntDir)$(TargetName).pch")
    else()
        set(_PCH_FILE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${target_project}.dir/${_project_name}PCH.pch")
    endif()

    if(EXISTS "${_PCH_PATH}.h" AND EXISTS "${_PCH_PATH}.cpp")
        # 遍历所有源文件，只给cpp文件使用预编译头
        get_target_property(_TARGET_SOURCES ${target_project} SOURCES)
        # 创建预编译头
        set_property(SOURCE "${_PCH_PATH}.cpp" APPEND_STRING PROPERTY COMPILE_FLAGS " /Yc\"${_PCH_HEADER}\" /Fp\"${_PCH_FILE}\" ")
        # 设置输出文件
        set_source_files_properties("${_PCH_PATH}.cpp" PROPERTIES OBJECT_OUTPUTS "${_PCH_FILE}")
        foreach(_sourcefile ${_TARGET_SOURCES})
            if ("${_sourcefile}" MATCHES ".*\\.\\cpp$")
                if (NOT "${_sourcefile}" STREQUAL "${_PCH_PATH}.cpp")
                    # 使用预编译头
                    set_property(SOURCE "${_sourcefile}" APPEND_STRING PROPERTY COMPILE_FLAGS " /Yu\"${_PCH_HEADER}\" /Fp\"${_PCH_FILE}\" ")
                    # 设置依赖文件
                    set_source_files_properties("${_sourcefile}" PROPERTIES OBJECT_DEPENDS "${_PCH_FILE}")
                endif()
            endif()
        endforeach()
        # 不加/FI的话.h代码提示有问题
        target_compile_options(${target_project} PRIVATE "/FI${_PCH_HEADER}")
    else()
        message(STATUS "not have pch:${_PCH_PATH}.h or ${_PCH_PATH}.cpp")
    endif()
endfunction()

# add_compile_options(/EHsc /GR-)
string(REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
# _HAS_EXCEPTIONS=0

# 使用标准
# target_compile_options(${TargetProject} PRIVATE "/permissive-")
# target_compile_options(${THIS_PROJECT} PRIVATE /EHsc /W0 /Wv:18)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")