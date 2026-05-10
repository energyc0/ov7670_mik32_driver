add_library(camera_lib ov7670/ov7670.c)

target_link_libraries(camera_lib mik32_hal)

target_link_directories(camera_lib PUBLIC ${CMAKE_CURRENT_LIST_DIR})