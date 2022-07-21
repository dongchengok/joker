
add_subdirectory(EASTL)
# 接入EASTL的VS调试模板
# https://eastl.docsforge.com/#projects-and-products-using-eastl
add_custom_target(NatVis SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/EASTL/doc/EASTL.natvis)