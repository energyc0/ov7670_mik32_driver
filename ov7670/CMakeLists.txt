set(OV7670_DIR ${CMAKE_CURRENT_LIST_DIR})
add_subdirectory(arch)

add_library(mik32_ov7670_lib ov7670.c)
target_link_libraries(mik32_ov7670_lib PUBLIC mik32_arch_ov7670)

target_include_directories(mik32_ov7670_lib PUBLIC ${OV7670_DIR})