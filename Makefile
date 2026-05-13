MIK32_UPLOADER_DIR=/mik32_utils/mik32-uploader

TOOLCHAIN_PREFIX=${MIK32_TOOLCHAIN_DIR}/riscv-none-elf
BUILD_DIR=build
FIRMWARE=$(BUILD_DIR)/sources/mik32_firmware

SERIAL_BAUDRATE=115200
SERIAL_PORT=/dev/ttyUSB0

all: $(FIRMWARE).hex

$(FIRMWARE).bin: $(FIRMWARE).elf
	${TOOLCHAIN_PREFIX}-objcopy -O binary ${FIRMWARE}.elf ${FIRMWARE}.bin

$(FIRMWARE).hex: $(FIRMWARE).elf
	${TOOLCHAIN_PREFIX}-objcopy -O ihex ${FIRMWARE}.elf ${FIRMWARE}.hex

$(FIRMWARE).elf: $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

$(BUILD_DIR):
	cmake -B $(BUILD_DIR) -DSERIAL_BAUDRATE=${SERIAL_BAUDRATE}

flash: ${FIRMWARE}.hex
	python3 $(MIK32_UPLOADER_DIR)/mik32_upload.py ${FIRMWARE}.hex --run-openocd \
	--openocd-exec /usr/bin/openocd \
	--openocd-target $(MIK32_UPLOADER_DIR)/openocd-scripts/target/mik32.cfg \
	--openocd-interface $(MIK32_UPLOADER_DIR)/openocd-scripts/interface/ftdi/mikron-link.cfg \
	--adapter-speed 500 --mcu-type MIK32V2

monitor:
	picocom $(SERIAL_PORT) -b $(SERIAL_BAUDRATE) --omap crcrlf --echo

clean: $(BUILD_DIR)
	rm -rf $(BUILD_DIR)

.PHONY: flash clean