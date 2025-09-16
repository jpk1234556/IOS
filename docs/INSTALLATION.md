# Brandon Media OS - Installation Guide

## Development Environment Setup

### Windows Development Setup

#### 1. Install MSYS2 (Recommended)

1. Download and install MSYS2 from https://www.msys2.org/
2. Open MSYS2 terminal and update the package database:

```bash
pacman -Syu
```

#### 2. Install Required Tools in MSYS2

```bash
# Install basic development tools
pacman -S base-devel git

# Install cross-compiler toolchain
pacman -S mingw-w64-x86_64-gcc

# For x86_64-elf toolchain (you may need to build from source or use prebuilt)
# Alternative: Download prebuilt from https://github.com/lordmilko/i686-elf-tools
```

#### 3. Install QEMU

```bash
# In MSYS2
pacman -S mingw-w64-x86_64-qemu

# Or download from https://www.qemu.org/download/#windows
```

#### 4. Install GRUB Tools

```bash
# In MSYS2
pacman -S grub

# Or use WSL for Linux-compatible grub-mkrescue
```

### Alternative: Use WSL (Windows Subsystem for Linux)

#### 1. Install WSL2

```powershell
# In PowerShell as Administrator
wsl --install
```

#### 2. Setup Ubuntu and Install Tools

```bash
# Update package list
sudo apt update

# Install build essentials
sudo apt install build-essential nasm

# Install cross-compiler (build from source or use package)
sudo apt install gcc-multilib

# Install QEMU
sudo apt install qemu-system-x86

# Install GRUB tools
sudo apt install grub-pc-bin grub-common xorriso
```

#### 3. Build x86_64-elf Cross-Compiler from Source

```bash
# Create working directory
mkdir ~/cross-compiler && cd ~/cross-compiler

# Download binutils and gcc
wget https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-13.1.0/gcc-13.1.0.tar.xz

# Extract and build binutils
tar -xf binutils-2.40.tar.xz
mkdir build-binutils && cd build-binutils
../binutils-2.40/configure --target=x86_64-elf --prefix=/opt/cross --disable-nls
make -j$(nproc)
sudo make install

# Add to PATH
export PATH=/opt/cross/bin:$PATH

# Extract and build gcc
cd .. && tar -xf gcc-13.1.0.tar.xz
mkdir build-gcc && cd build-gcc
../gcc-13.1.0/configure --target=x86_64-elf --prefix=/opt/cross --disable-nls --enable-languages=c --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
sudo make install-gcc
sudo make install-target-libgcc
```

### Docker Alternative (Cross-Platform)

#### 1. Create Dockerfile

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    nasm \
    xorriso \
    grub-pc-bin \
    grub-common \
    qemu-system-x86 \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Install x86_64-elf cross-compiler
RUN wget https://github.com/lordmilko/i686-elf-tools/releases/download/7.1.0/x86_64-elf-tools-linux.zip \
    && unzip x86_64-elf-tools-linux.zip \
    && mv x86_64-elf-tools /opt/ \
    && echo 'export PATH=/opt/x86_64-elf-tools/bin:$PATH' >> ~/.bashrc

WORKDIR /workspace
```

#### 2. Build and Run Development Environment

```bash
# Build Docker image
docker build -t brandon-media-os-dev .

# Run development container
docker run -it -v $(pwd):/workspace brandon-media-os-dev
```

## Verification

### Test Installation

Run the setup script to verify all tools are installed:

```bash
# Windows (MSYS2/PowerShell)
scripts\setup.bat

# Linux/WSL/Docker
chmod +x scripts/setup.sh && ./scripts/setup.sh
```

### Manual Verification

```bash
# Check cross-compiler
x86_64-elf-gcc --version

# Check QEMU
qemu-system-x86_64 --version

# Check GRUB tools
grub-mkrescue --version

# Check Make
make --version
```

## Quick Start After Installation

```bash
# Build kernel
make

# Create ISO and run in QEMU
make run

# Debug with GDB
make gdb

# Run automated tests
make test
```

## Troubleshooting

### Common Issues

#### 1. x86_64-elf-gcc not found

- Ensure cross-compiler is in PATH
- Try using system gcc with `-ffreestanding` flag
- Build from source or download prebuilt toolchain

#### 2. grub-mkrescue not working on Windows

- Use WSL or MSYS2
- Install xorriso and grub-pc-bin packages
- Ensure all GRUB dependencies are met

#### 3. QEMU not starting

- Check if virtualization is enabled in BIOS
- Verify QEMU installation
- Try different QEMU parameters

#### 4. Build failures

- Check make version (GNU Make required)
- Verify all source files are present
- Clean and rebuild: `make clean && make`

### Performance Tips

- Use `-j$(nproc)` for parallel compilation
- Enable hardware acceleration in QEMU when available
- Use SSD for faster build times
- Consider using ccache for incremental builds

## Next Steps

After successful installation:

1. Read `README.md` for project overview
2. Study `roadmap.md` for development phases
3. Examine `structure.md` for project organization
4. Start with Phase 1 implementation
5. Follow the development workflow in the documentation

---

_For support, refer to the project documentation or create an issue on the repository._
