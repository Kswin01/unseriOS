BUILD_DIR ?= build

CURR_DIR := $(abspath .)
BOOT_DIR := $(CURR_DIR)/boot
KERNEL_DIR := $(CURR_DIR)/kernel

KERNEL_FILES := $(addprefix $(BUILD_DIR)/, boot.o kernel.o)
LINKER_FILE := $(BOOT_DIR)/linker.ld
IMAGE_FILE := $(BUILD_DIR)/kernel.elf

# Move this to clang eventually
TOOLCHAIN := aarch64-none-elf
QEMU := qemu-system-aarch64
CPU := cortex-a53

CC := $(TOOLCHAIN)-gcc
LD := $(TOOLCHAIN)-ld
AS := $(TOOLCHAIN)-as
AR := $(TOOLCHAIN)-ar
RANLIB := $(TOOLCHAIN)-ranlib
OBJCOPY := $(TOOLCHAIN)-objcopy

vpath %.c $(BOOT_DIR) $(KERNEL_DIR)
vpath %.s $(BOOT_DIR) $(KERNEL_DIR)

# $(BUILD_DIR)/%.elf: $(BUILD_DIR)/%.o
# 	$(LD) $(LDFLAGS) $< $(LIBS) -o $@

CFLAGS := -ffreestanding
LDFLAGS := -nostdlib

$(BUILD_DIR)/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | build
	$(AS) -g -mcpu=$(CPU) $< -o $@

# This is just the last linker step
${IMAGE_FILE}: ${KERNEL_FILES} ${LINKER_FILE}
	$(LD) $(LDFLAGS) -T$(LINKER_FILE) $(KERNEL_FILES) -o $(IMAGE_FILE)

build:
	mkdir -p $(BUILD_DIR)

all: $(IMAGE_FILE)

qemu: $(IMAGE_FILE)
	$(QEMU) -machine virt \
			-cpu cortex-a53 \
			-device loader,file=$(IMAGE_FILE),addr=0x40100000,cpu-num=0 \
			-m size=2G \
			-nographic \

# Clean build
clean:
	rm -rf $(BUILD_DIR)
