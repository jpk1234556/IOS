# Brandon Media OS - Neural SCADA GUI Setup Instructions

## Quick Setup (Recommended - WSL)

### Step 1: Install WSL with Ubuntu

```powershell
# Run as Administrator
wsl --install -d Ubuntu
```

### Step 2: Setup Build Environment in WSL

```bash
# Update packages
sudo apt update && sudo apt upgrade -y

# Install cross-compiler toolchain
sudo apt install -y build-essential nasm

# Install x86_64-elf cross-compiler
sudo apt install -y gcc-multilib binutils

# Install GRUB tools
sudo apt install -y grub-pc-bin grub-common xorriso

# Install QEMU for testing
sudo apt install -y qemu-system-x86

# Install additional tools
sudo apt install -y git make
```

### Step 3: Navigate to Project and Build

```bash
# Navigate to your project (from WSL)
cd /mnt/c/Users/JPK/OS

# Run setup script
chmod +x scripts/setup.sh
./scripts/setup.sh

# Build the Neural SCADA OS
make clean
make iso
make run
```

## Alternative Setup (Native Windows)

### Prerequisites

1. **Download and Install MinGW-w64**

   - Download from: https://www.mingw-w64.org/downloads/
   - Or use MSYS2: https://www.msys2.org/

2. **Install QEMU for Windows**

   - Download from: https://www.qemu.org/download/#windows
   - Add to PATH

3. **Install Cross-Compiler Toolchain**
   ```bash
   # In MSYS2 terminal
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-binutils
   pacman -S make
   ```

### Building with Native Tools

```bash
# Set up environment
export PATH="/mingw64/bin:$PATH"

# Build project
make clean
make iso
make run
```

## Docker Alternative (Cross-Platform)

### Create Docker Build Environment

```dockerfile
# Create Dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    nasm \
    gcc-multilib \
    binutils \
    grub-pc-bin \
    grub-common \
    xorriso \
    qemu-system-x86 \
    git \
    make

WORKDIR /workspace
```

```bash
# Build Docker image
docker build -t brandonos-build .

# Run build container
docker run -it -v /path/to/OS:/workspace brandonos-build

# Inside container
make clean && make iso && make run
```

## What You'll See When Running

🎮 **Neural SCADA 3D Parallax Interface**

- Animated neural matrix background with flowing patterns
- Professional SCADA control panel with real-time gauges
- Interactive buttons (Start, Stop, Reset, Emergency)
- Live simulation of Temperature, Pressure, Flow, Power
- System status indicators and progress bars
- Cyberpunk visual effects with dark blue/white theme

🎯 **Interactive Features**

- Mouse hover effects on all controls
- Keyboard navigation (Tab, Enter, Space, Escape)
- Real-time gauge animations with critical alarms
- Parallax layer movement creating 3D depth
- Accessibility features with high contrast mode

## Quick Start Commands

```bash
# Clean build
make clean

# Build kernel and create ISO
make iso

# Run in QEMU with GUI
make run

# Debug mode
make gdb

# Automated test
make test
```

## Expected Output

When successfully running, you'll see:

1. **Boot Messages**: "Brandon Media OS v0.1 - Neural interface online"
2. **System Initialization**: All neural systems coming online
3. **GUI Startup**: "Neural GUI System initialized"
4. **SCADA Interface**: Live control panel with animated gauges
5. **Real-time Simulation**: Gauges moving with realistic data

The system runs at 60fps with smooth animations and provides a professional industrial control interface with stunning cyberpunk aesthetics!

## Troubleshooting

### Common Issues:

- **"make not found"**: Install build tools (see setup above)
- **"x86_64-elf-gcc not found"**: Install cross-compiler toolchain
- **"grub-mkrescue failed"**: Install GRUB tools
- **"QEMU not found"**: Install QEMU emulator

### Performance Tips:

- Use WSL for best performance on Windows
- Allocate at least 512MB RAM to QEMU
- Enable hardware acceleration if available

## System Requirements

- Windows 10/11 with WSL support OR Linux/macOS
- 4GB RAM minimum (for build tools and QEMU)
- x86_64 processor
- Internet connection for downloading tools
