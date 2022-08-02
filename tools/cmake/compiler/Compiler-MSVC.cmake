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

# 使用标准
# target_compile_options(${TargetProject} PRIVATE "/permissive-")
# target_compile_options(${THIS_PROJECT} PRIVATE /EHsc /W0 /Wv:18)

# /Yc"JokerCorePCH.h" /ifcOutput "JokerCore.dir\Debug\" /GS /W3 /Zc:wchar_t /I"E:\Work\joker\code\joker\core" /I"E:\Work\joker\3rd\EASTL\include" /I"E:\Work\joker\3rd\EASTL\test\packages\EABase\include\Common" /I"E:\Program Files\VulkanSDK\1.2.198.1\Include" /I"E:\Program Files\VulkanSDK\1.2.198.1\Third-Party\Include" /Zi /Gm- /Od /Ob0 /Fd"E:\Work\joker\build\code\joker\core\Debug\JokerCore.pdb" /Zc:inline /fp:precise /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "CMAKE_INTDIR=\"Debug\"" /errorReport:prompt /WX- /Zc:forScope /RTC1 /GR /Gd /MDd /std:c++17 /Fa"JokerCore.dir\Debug\" /EHsc /nologo /Fo"JokerCore.dir\Debug\" /Fp"JokerCore.dir\Debug\JokerCore.pch" /diagnostics:column 
# "command": "C:\\PROGRA~2\\MICROS~3\\2019\\COMMUN~1\\VC\\Tools\\MSVC\\1429~1.301\\bin\\Hostx64\\x64\\cl.exe  /nologo /TP -IE:\\Work\\joker\\code\\joker\\core -IE:\\Work\\joker\\3rd\\EASTL\\include -IE:\\Work\\joker\\3rd\\EASTL\\test\\packages\\EABase\\include\\Common -external:I\"E:\\Program Files\\VulkanSDK\\1.2.198.1\\Include\" -external:I\"E:\\Program Files\\VulkanSDK\\1.2.198.1\\Third-Party\\Include\" -external:W0 /DWIN32 /D_WINDOWS /W3 /GR /EHsc /MDd /Zi /Ob0 /Od /RTC1 -std:c++17  /Yc\"JokerCorePCH.h\" /Fp\"E:/Work/joker/build/code/joker/core/CMakeFiles/JokerCore.dir/JokerCorePCH.pch\"  /Focode\\joker\\core\\CMakeFiles\\JokerCore.dir\\JokerCorePCH.cpp.obj /FdTARGET_COMPILE_PDB /FS -c E:\\Work\\joker\\code\\joker\\core\\JokerCorePCH.cpp",
# E:/Work/joker/build/code/joker/core/CMakeFiles/JokerCore.dir/JokerCorePCH.pch
# E:/Work/joker/build/code/joker/core/CMakeFiles/JokerCore.dir/JokerCorePCH.pch