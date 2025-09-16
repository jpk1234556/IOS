#!/bin/bash

echo "Brandon Media OS - Setup Script (Linux)"
echo "======================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Print status
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}[OK]${NC} $2"
    else
        echo -e "${RED}[ERROR]${NC} $2"
    fi
}

echo
echo "Checking required tools..."

# Check for cross-compiler
if command_exists x86_64-elf-gcc; then
    print_status 0 "x86_64-elf-gcc found"
elif command_exists gcc; then
    print_status 1 "x86_64-elf-gcc not found, but gcc available (will use with -ffreestanding)"
    CROSS_COMPILE=""
else
    print_status 1 "No suitable compiler found"
    echo "Please install x86_64-elf cross-compiler or gcc"
    exit 1
fi

# Check for GRUB tools
if command_exists grub-mkrescue; then
    print_status 0 "grub-mkrescue found"
else
    print_status 1 "grub-mkrescue not found"
    echo "Please install GRUB tools: sudo apt install grub-pc-bin grub-common xorriso"
    exit 1
fi

# Check for QEMU
if command_exists qemu-system-x86_64; then
    print_status 0 "qemu-system-x86_64 found"
else
    print_status 1 "qemu-system-x86_64 not found"
    echo "Please install QEMU: sudo apt install qemu-system-x86"
    exit 1
fi

# Check for Make
if command_exists make; then
    print_status 0 "make found"
else
    print_status 1 "make not found"
    echo "Please install make: sudo apt install build-essential"
    exit 1
fi

echo
echo "All required tools found!"
echo

# Create necessary directories
echo "Creating build directories..."
mkdir -p build/obj build/iso build/debug config

echo "Building Brandon Media OS..."
make clean
make all

if [ $? -eq 0 ]; then
    echo
    echo -e "${GREEN}[SUCCESS]${NC} Build completed successfully!"
    echo
    echo "Available commands:"
    echo "  make run   - Test in QEMU"
    echo "  make gdb   - Debug with GDB"
    echo "  make test  - Run automated tests"
    echo "  make clean - Clean build files"
else
    echo -e "${RED}[ERROR]${NC} Build failed!"
    exit 1
fi

echo
echo "Brandon Media OS setup complete!"