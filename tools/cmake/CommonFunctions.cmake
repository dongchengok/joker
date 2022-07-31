
# 列出所有目录
function(joker_list_subdirectories_glob globPattern result)
	# LIST_DIRECTORIES 默认是false木有目录
	file(GLOB _pathList LIST_DIRECTORIES true "${globPattern}")
	set(_dirList)
	foreach(_child ${_pathList})
		if (IS_DIRECTORY "${_child}")
			# message(STATUS "joker_list_subdirectories_glob:${_child}")
			list(APPEND _dirList "${_child}")
		endif()
	endforeach()
	# 需要对父级作用域生效
	set(${result} ${_dirList} PARENT_SCOPE)
endfunction()

# 添加所有含有CMakeLists.txt和CMakeLists.cmake文件的子目录
# CMakeLists.cmake是为了处理find_package的作用域问题
function(joker_add_subdirectories_glob globPattern)
	set(_dirs)
	joker_list_subdirectories_glob("${globPattern}" _dirs)
	foreach(dir ${_dirs})
		if (EXISTS "${dir}/CMakeLists.txt")
			# message(STATUS "joker_add_subdirectory ${dir}")
			add_subdirectory("${dir}")
		endif()
		if (EXISTS "${dir}/CMakeLists.cmake")
			# message(STATUS "${dir}/CMakeLists.cmake")
			include("${dir}/CMakeLists.cmake")
		endif()
	endforeach()
endfunction()

function(joker_add_subdirectories)
	joker_add_subdirectories_glob("*")
endfunction()

function(joker_project_use_pch)
	# 这个自定义的话不太方便，先不用这个处理预编译头了，自己搞一下
	# target_precompile_headers
    message(FATAL_ERROR "undefined!")
endfunction()

function(joker_project_3rd target)
	jokerm_project_prepare(${ARGN})
	jokerm_project_sources_begin()
	if(_ARGS_INTERFACE)
		add_library(${_THIS_PROJECT} INTERFACE ${${_THIS_PROJECT}_SOURCES})
	elseif()
		add_library(${_THIS_PROJECT} STATIC ${${_THIS_PROJECT}_SOURCES})
	endif()
	set_target_properties(${_THIS_PROJECT}  PROPERTIES FOLDER ${JOKER_NAME_3RD})
	# jokerm_project_compile_options(${THIS_PROJECT})
	joker_project_use_pch(${_THIS_PROJECT})
	jokerm_project_sources_end()
endfunction()

function(joker_project_engine target)
	jokerm_project_prepare(${ARGN})
	jokerm_project_sources_begin()
	# 引擎库都用static编译
	add_library(${_THIS_PROJECT} STATIC ${${_THIS_PROJECT}_SOURCES})
	set_target_properties(${_THIS_PROJECT}  PROPERTIES FOLDER ${JOKER_NAME_ENGINE})
	# jokerm_project_compile_options(${THIS_PROJECT})
	joker_project_use_pch(${_THIS_PROJECT})
	jokerm_project_sources_end()
endfunction()

function(joker_project_app target)
	jokerm_project_prepare(${ARGN})
	jokerm_project_sources_begin()
	# add_library(${_THIS_PROJECT} STATIC ${${_THIS_PROJECT}_SOURCES})
	add_executable(${_THIS_PROJECT} ${${_THIS_PROJECT}_SOURCES})
	set_target_properties(${_THIS_PROJECT}  PROPERTIES FOLDER "exe")
	# jokerm_project_compile_options(${THIS_PROJECT})
	jokerm_project_sources_end()
endfunction()


function(joker_project_lib target)
	jokerm_project_prepare(${ARGN})
	if(_ARGS_FORCE_SHARED)
		add_library(${THIS_PROJECT} SHARED ${${THIS_PROJECT}_SOURCES})
	elseif (_ARGS_FORCE_STATIC)
		add_library(${THIS_PROJECT} STATIC ${${THIS_PROJECT}_SOURCES})
	else()
		add_library(${THIS_PROJECT} ${${THIS_PROJECT}_SOURCES})
	endif()
	jokerm_project_compile_options(${THIS_PROJECT})
endfunction()

function(joker_project_exe target)
	jokerm_project_prepare(${ARGN})
	# set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}")
	# file(
	# 	GLOB_RECURSE _source_list 
	# 	LIST_DIRECTORIES false
	# 	"${_src_root_path}/*.cpp"
	# 	"${_src_root_path}/*.h"
	# )
	# set(${THIS_PROJECT}_SOURCES ${_source_list})
	jokerm_project_sources_begin()
	message(STATUS ${_source_list})
	add_executable(${_THIS_PROJECT} ${${_THIS_PROJECT}_SOURCES})
	# jokerm_project_compile_options(${THIS_PROJECT})
	jokerm_project_sources_end()
endfunction()