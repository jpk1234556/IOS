# Makefile - build x86_64 multiboot kernel and GRUB ISO
CROSS_COMPILE ?= x86_64-elf-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

CFLAGS := -O2 -mno-red-zone -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -I./src/include
LDFLAGS := -T config/link.ld

# Source files
BOOT_SRCS := src/boot/multiboot_header.S src/boot/boot.S src/boot/uefi_boot.c src/boot/uefi_manager.c
KERNEL_SRCS := src/kernel/main.c
INTERRUPT_SRCS := src/kernel/interrupts/idt.c src/kernel/interrupts/isr.S src/kernel/interrupts/exceptions.c src/kernel/interrupts/irq.c src/kernel/interrupts/timer.c src/kernel/interrupts/interrupt_control.S
MEMORY_SRCS := src/kernel/memory/paging.c src/kernel/memory/paging_asm.S src/kernel/memory/pmm.c src/kernel/memory/vmm.c src/kernel/memory/heap.c
PROCESS_SRCS := src/kernel/process/process.c src/kernel/process/context.S src/kernel/process/scheduler.c src/kernel/process/threads.c src/kernel/process/ipc.c
SYSCALL_SRCS := src/kernel/syscalls/syscall.c src/kernel/syscalls/syscall_entry.S src/kernel/syscalls/user_mode.c
DRIVER_SRCS := src/kernel/drivers/pci.c src/kernel/drivers/hal.c src/kernel/drivers/virtio_net.c src/kernel/drivers/framebuffer.c src/kernel/drivers/device_test.c src/kernel/drivers/gui.c src/kernel/drivers/gui_widgets.c src/kernel/drivers/gui_animations.c src/kernel/drivers/gui_accessibility.c src/kernel/drivers/graphics_3d.c src/kernel/drivers/input.c src/kernel/drivers/scada_demo.c
SMP_SRCS := src/kernel/smp/smp.c src/kernel/smp/advanced_scheduler.c
SECURITY_SRCS := src/kernel/security/security.c
USERLAND_SRCS := userland/lib/neural_app.c userland/neural_demo/neural_demo.c userland/shell/neural_shell.c
FS_SRCS := src/fs/vfs.c src/fs/ramfs.c src/fs/file_ops.c src/fs/dir_ops.c src/fs/storage.c
LIB_SRCS := src/lib/utils.c
SRCS := $(BOOT_SRCS) $(KERNEL_SRCS) $(INTERRUPT_SRCS) $(MEMORY_SRCS) $(PROCESS_SRCS) $(SYSCALL_SRCS) $(DRIVER_SRCS) $(SMP_SRCS) $(SECURITY_SRCS) $(FS_SRCS) $(USERLAND_SRCS) $(LIB_SRCS)

# Object files
OBJS := $(SRCS:.S=.o)
OBJS := $(OBJS:.c=.o)

TARGET := build/kernel.elf
ISODIR := build/iso
ISO := build/os.iso

.PHONY: all clean iso run gdb test setup-dirs

all: setup-dirs $(TARGET)

setup-dirs:
	@if not exist "build\obj" mkdir build\obj
	@if not exist "build\iso" mkdir build\iso
	@if not exist "build\debug" mkdir build\debug

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

iso: $(TARGET)
	@if exist "$(ISODIR)" rmdir /s /q "$(ISODIR)"
	@mkdir "$(ISODIR)\boot\grub"
	@copy "$(TARGET)" "$(ISODIR)\boot\kernel.elf"
	@copy "src\boot\grub.cfg" "$(ISODIR)\boot\grub\grub.cfg"
	grub-mkrescue -o $(ISO) $(ISODIR) || (echo "grub-mkrescue failed - ensure grub tools are installed" && exit 1)

run: iso
	qemu-system-x86_64 -cdrom $(ISO) -serial stdio -display none -boot d

gdb: iso
	start /B qemu-system-x86_64 -cdrom $(ISO) -serial stdio -display none -S -gdb tcp::1234
	@echo "QEMU started with GDB server on port 1234"
	@echo "In another terminal run: x86_64-elf-gdb build/kernel.elf"
	@echo "Then: (gdb) target remote :1234"

test: iso
	@echo "Running CI test..."
	@timeout 5 qemu-system-x86_64 -cdrom $(ISO) -serial stdio -display none > build\output.txt 2>&1 || echo "Timeout reached"
	@findstr /C:"Brandon Media OS" build\output.txt >nul && echo "BOOT TEST: PASSED" || (echo "BOOT TEST: FAILED" && type build\output.txt)

clean:
	@if exist "src\boot\*.o" del /q src\boot\*.o
	@if exist "src\kernel\*.o" del /q src\kernel\*.o
	@if exist "src\kernel\interrupts\*.o" del /q src\kernel\interrupts\*.o
	@if exist "src\kernel\memory\*.o" del /q src\kernel\memory\*.o
	@if exist "src\kernel\process\*.o" del /q src\kernel\process\*.o
	@if exist "src\kernel\syscalls\*.o" del /q src\kernel\syscalls\*.o
	@if exist "src\kernel\drivers\*.o" del /q src\kernel\drivers\*.o
	@if exist "src\kernel\smp\*.o" del /q src\kernel\smp\*.o
	@if exist "src\kernel\security\*.o" del /q src\kernel\security\*.o
	@if exist "userland\lib\*.o" del /q userland\lib\*.o
	@if exist "userland\neural_demo\*.o" del /q userland\neural_demo\*.o
	@if exist "userland\shell\*.o" del /q userland\shell\*.o
	@if exist "src\fs\*.o" del /q src\fs\*.o
	@if exist "build\kernel.elf" del /q build\kernel.elf
	@if exist "build\iso" rmdir /s /q build\iso
	@if exist "build\os.iso" del /q build\os.iso
	@if exist "build\output.txt" del /q build\output.txt

help:
	@echo "Available targets:"
	@echo "  all      - Build kernel"
	@echo "  iso      - Create bootable ISO"
	@echo "  run      - Run in QEMU"
	@echo "  gdb      - Run with GDB debugging"
	@echo "  test     - Run automated test"
	@echo "  clean    - Clean build files"
	@echo "  help     - Show this help"