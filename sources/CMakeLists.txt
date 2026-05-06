set(FIRMWARE_NAME mik32_firmware)

add_executable(${FIRMWARE_NAME} main.c)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "Looking for ldscript: ${CMAKE_SOURCE_DIR}/hardware/mik32v2-shared/ldscripts/eeprom.ld")

target_link_libraries(${FIRMWARE_NAME}
    PRIVATE
        mik32_hal
)

target_link_directories(${FIRMWARE_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/hardware/mik32v2-shared/ldscripts
)

# Convert output to hex and binary
add_custom_command(TARGET ${FIRMWARE_NAME}
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${TOOLCHAIN_PREFIX}-objcopy ARGS -O binary ${FIRMWARE_NAME}${CMAKE_EXECUTABLE_SUFFIX_C} ${FIRMWARE_NAME}.bin
    COMMAND ${TOOLCHAIN_PREFIX}-objcopy ARGS -O ihex ${FIRMWARE_NAME}${CMAKE_EXECUTABLE_SUFFIX_C} ${FIRMWARE_NAME}.hex
)

add_custom_target(flash
    COMMAND	python3 $(MIK32_UPLOADER_DIR)/mik32_upload.py ${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE_NAME}.hex --run-openocd
	--openocd-exec /usr/bin/openocd
	--openocd-target $(MIK32_UPLOADER_DIR)/openocd-scripts/target/mik32.cfg
	--openocd-interface $(MIK32_UPLOADER_DIR)/openocd-scripts/interface/ftdi/mikron-link.cfg
	--adapter-speed 500 --mcu-type MIK32V2
    COMMENT "Flashing microcontroller..."
)