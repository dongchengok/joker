macro(jokerm_apply_compile_settings)
	if (MODULE_PCH)
		#CryQt defines incompatible DLLExport in stdafx, temporarily disable PCH for CryQt now
		# if (NOT "${THIS_PROJECT}" STREQUAL "CryQt")
			USE_MSVC_PRECOMPILED_HEADER( ${THIS_PROJECT} ${MODULE_PCH_H} ${MODULE_PCH} )
		# endif()
		# set_property(TARGET ${THIS_PROJECT} APPEND PROPERTY AUTOMOC_MOC_OPTIONS -b${MODULE_PCH_H})
	endif()
	if (MODULE_OUTDIR)
		set_property(TARGET ${THIS_PROJECT} PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${MODULE_OUTDIR}")
		set_property(TARGET ${THIS_PROJECT} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${MODULE_OUTDIR}") 
	endif()
	SET_PLATFORM_TARGET_PROPERTIES( ${THIS_PROJECT} )	
	if(MODULE_SOLUTION_FOLDER)
		set_solution_folder("${MODULE_SOLUTION_FOLDER}" ${THIS_PROJECT})
	endif()

	# if (WINDOWS AND NOT OPTION_DEDICATED_SERVER)
	# 	target_compile_options(${THIS_PROJECT} PRIVATE $<$<CONFIG:Release>:-DPURE_CLIENT>)
	# endif()
	
	if (DEFINED PROJECT_BUILD_CRYENGINE AND NOT PROJECT_BUILD_CRYENGINE)
		# If option to not build engine modules is selected they are excluded from the build
		set_target_properties(${THIS_PROJECT} PROPERTIES EXCLUDE_FROM_ALL TRUE)
		set_target_properties(${THIS_PROJECT} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
	endif()

	get_target_property(target_type ${THIS_PROJECT} TYPE)
	if (target_type MATCHES "EXECUTABLE")
		target_compile_options(${THIS_PROJECT} PRIVATE -DCRY_IS_APPLICATION)
	endif()
endmacro()

macro(jokerm_sources_recursive_search)
	set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}")
	file(
		GLOB_RECURSE _SOURCES 
		LIST_DIRECTORIES false
		"${_src_root_path}/*.cpp"
		"${_src_root_path}/*.h"
	)

	foreach(_source IN ITEMS ${_SOURCES})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		source_group("${_group_path}" FILES "${_source}")
		# file(RELATIVE_PATH _source_rel "${_src_root_path}" "${_source}")
		
		# if (_group_path STREQUAL "")
		# 	set(_group_path "Root")
		# endif()
		
		# string(REPLACE "\\" "_" _group_path "${_group_path}")
		
		# set(_group_path "${_group_path}.cpp")
		
		# list(FIND UBERFILES ${_group_path} GROUP_INDEX)			
		# if(GROUP_INDEX EQUAL -1)
		# 	list(APPEND UBERFILES "${_group_path}")
		# 	set(${_group_path}_PROJECTS ${UB_PROJECTS})
		# endif()
		
		# add_to_uberfile(${_group_path} ${_source_rel})
	endforeach()
endmacro()

macro(jokderm_source_group_by_dir source_list)
	set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
	foreach(sgbd_file ${${source_list}})
		string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
		string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
		string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
		string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
		if(sgbd_nogroup)
			set(sgbd_group_name "\\")
		endif(sgbd_nogroup)
		source_group(${sgbd_group_name} FILES ${sgbd_file})
	endforeach(sgbd_file)
endmacro()

macro(jokerm_project_settings)
	set(options STATIC SHARED INTERFACE)
	set(oneValueArgs SOLUTION_FOLDER PCH OUTDIR)
	set(multiValueArgs FILE_LIST INCLUDES LIBS DEFINES)
	cmake_parse_arguments(_ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	if (_ARGS_PCH)
		string(REPLACE ".cpp" ".h" _PROJECT_PCH_HEADER_FILE ${_PROJECT_PCH})
		get_filename_component(_PROJECT_PCH_H ${_PROJECT_PCH_HEADER_FILE} NAME)
	endif()
endmacro()

macro(jokerm_project_sources_begin)
	include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )
	if(DEFINED _SOURCES)
		# message(STATUS "defined source ${_SOURCE}")
	else()
		# message(STATUS "not defined source ${_SOURCE}")
		# set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}")
		# file(
		# 	GLOB_RECURSE _SOURCES 
		# 	LIST_DIRECTORIES false
		# 	"${_src_root_path}/*.cpp"
		# 	"${_src_root_path}/*.h"
		# )
		# jokderm_source_group_by_dir(${_SOURCES})
		jokerm_sources_recursive_search()
		set(${_THIS_PROJECT}_SOURCES ${_SOURCES})
		set(_src_root_path)
	endif()
endmacro()

macro(jokerm_project_sources_end)
	set(_SOURCES)
	set(${_THIS_PROJECT}_SOURCES)
	target_include_directories(${target} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()


macro(jokerm_project_prepare)
	set(_THIS_PROJECT ${target} PARENT_SCOPE)
	set(_THIS_PROJECT ${target})
	project(${target})
	jokerm_project_settings(${ARGN})
	if(NOT ${_THIS_PROJECT}_SOURCES)
		set(${_THIS_PROJECT}_SOURCES ${_SOURCES})
	endif()	
endmacro()