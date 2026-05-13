set(FIRMWARE_NAME mik32_firmware)

add_executable(${FIRMWARE_NAME} main.c)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "Looking for ldscript: ${CMAKE_SOURCE_DIR}/hardware/mik32v2-shared/ldscripts/eeprom.ld")
message(STATUS "Building with flags: ${CMAKE_C_COMPILE_FLAGS}")

target_link_libraries(${FIRMWARE_NAME}
    PRIVATE
        mik32_hal
        mik32_ov7670_lib
)

target_link_directories(${FIRMWARE_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/hardware/mik32v2-shared/ldscripts
)
