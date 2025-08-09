BUILD_DIR ?= build

CURR_DIR := $(abspath .)
BOOT_DIR := $(CURR_DIR)/boot
KERNEL_DIR := $(CURR_DIR)/kernel
KERNEL_INC := $(KERNEL_DIR)/include

LINKER_FILE := $(BOOT_DIR)/linker.ld
IMAGE_FILE := $(BUILD_DIR)/kernel.elf

LIBC := $(BUILD_DIR)/libc.a
LIBC_SRC := $(abspath libc/src)
LIBC_INC := $(abspath libc/include)
LIBC_OBJS := $(patsubst $(LIBC_SRC)/%.c, $(BUILD_DIR)/%.o, $(wildcard $(LIBC_SRC)/*.c))

KERNEL_OBJS := $(addprefix $(BUILD_DIR)/, boot.o kernel.o interrupt.o uart.o cpu.o mem.o c_traps.o traps.o mmu.o)

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

vpath %.c $(BOOT_DIR) $(KERNEL_DIR)/src
vpath %.s $(BOOT_DIR) $(KERNEL_DIR)/src
vpath %.S $(BOOT_DIR) $(KERNEL_DIR)/src

CFLAGS := -ffreestanding -nostdlib -g3 -I$(KERNEL_INC) -I$(LIBC_INC)
ASM_FLAGS := -I$(KERNEL_INC)
LDFLAGS := -nostdlib -T$(LINKER_FILE)
LIBS := $(LIBC)

$(BUILD_DIR)/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | build
	$(AS) -g -mcpu=$(CPU) $< -o $@

$(BUILD_DIR)/%.o: %.S | build
	$(CC) $(ASM_FLAGS) -x assembler-with-cpp -c -mcpu=$(CPU) $< -o $@

# This is just the last linker step
${IMAGE_FILE}: $(LIBC) ${KERNEL_OBJS} ${LINKER_FILE}
	$(LD) $(LDFLAGS) $(KERNEL_OBJS) $(LIBS) -o $(IMAGE_FILE)
	@echo "----- BUILT SYSTEM -----"

# Build libc.a from all libc/src/*.c
$(LIBC): $(LIBC_OBJS) | build
	$(AR) rcs $@ $^
	$(RANLIB) $@

# Compile libc .c files
$(BUILD_DIR)/%.o: $(LIBC_SRC)/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p $(BUILD_DIR)

all: $(LIBC) $(IMAGE_FILE)

qemu: $(IMAGE_FILE)
	$(QEMU) -machine virt,gic-version=3 \
			-cpu cortex-a53 \
			-device loader,file=$(IMAGE_FILE),addr=0x40100000,cpu-num=0 \
			-m size=2G \
			-nographic

-include util/util.mk

# Clean build
clean:
	rm -rf $(BUILD_DIR)


