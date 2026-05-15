add_library(mik32_arch_ov7670 mik32.c)
add_library(mik32_i2c_bitbang i2c_bitbang.c)
add_library(mik32_sccb sccb.c)
add_library(mik32_uart uart.c)

target_link_libraries(mik32_uart PUBLIC mik32_hal)
target_link_libraries(mik32_i2c_bitbang PUBLIC mik32_hal)
target_link_libraries(mik32_sccb PUBLIC mik32_uart mik32_i2c_bitbang)
target_link_libraries(mik32_arch_ov7670 PUBLIC mik32_sccb mik32_uart)

target_include_directories(mik32_arch_ov7670 PUBLIC ${OV7670_DIR})
target_include_directories(mik32_i2c_bitbang PUBLIC ${OV7670_DIR})