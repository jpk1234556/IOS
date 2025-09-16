# x86_64 Operating System Project Structure

## Directory Organization

```
OS/
├── docs/                          # Documentation
│   ├── roadmap.md                # Development roadmap (this file's sibling)
│   ├── structure.md              # Project structure (this file)
│   ├── api/                      # API documentation
│   ├── design/                   # Design documents
│   └── tutorials/                # Development tutorials
│
├── src/                          # Source code
│   ├── boot/                     # Boot-related code
│   │   ├── multiboot_header.S    # Multiboot specification header
│   │   ├── boot.S                # Assembly entry point and CRT
│   │   └── grub.cfg              # GRUB configuration
│   │
│   ├── kernel/                   # Kernel core
│   │   ├── main.c                # Kernel main entry (kmain)
│   │   ├── console/              # Console and output
│   │   │   ├── vga.c            # VGA text mode driver
│   │   │   ├── serial.c         # Serial port driver
│   │   │   └── console.h        # Console interface
│   │   │
│   │   ├── memory/               # Memory management
│   │   │   ├── paging.c         # Page table management
│   │   │   ├── pmm.c            # Physical memory manager
│   │   │   ├── vmm.c            # Virtual memory manager
│   │   │   ├── heap.c           # Kernel heap allocator
│   │   │   └── memory.h         # Memory management interface
│   │   │
│   │   ├── interrupts/           # Interrupt handling
│   │   │   ├── idt.c            # Interrupt Descriptor Table
│   │   │   ├── isr.c            # Interrupt Service Routines
│   │   │   ├── timer.c          # PIT timer driver
│   │   │   ├── keyboard.c       # Keyboard interrupt handler
│   │   │   └── interrupts.h     # Interrupt interface
│   │   │
│   │   ├── process/              # Process management
│   │   │   ├── scheduler.c      # Process scheduler
│   │   │   ├── process.c        # Process control blocks
│   │   │   ├── context.S        # Context switching assembly
│   │   │   └── process.h        # Process management interface
│   │   │
│   │   ├── syscalls/             # System calls
│   │   │   ├── syscall.c        # Syscall dispatcher
│   │   │   ├── syscall.S        # Syscall entry assembly
│   │   │   └── syscall.h        # Syscall interface
│   │   │
│   │   └── drivers/              # Device drivers
│   │       ├── pci.c            # PCI bus driver
│   │       ├── ahci.c           # AHCI/SATA storage driver
│   │       ├── virtio.c         # VirtIO drivers (for QEMU)
│   │       └── drivers.h        # Driver interface
│   │
│   ├── fs/                       # File system
│   │   ├── vfs.c                 # Virtual File System
│   │   ├── ramfs.c               # RAM-based file system
│   │   ├── tarfs.c               # TAR file system
│   │   └── fs.h                  # File system interface
│   │
│   ├── lib/                      # Kernel libraries
│   │   ├── string.c              # String manipulation functions
│   │   ├── stdio.c               # Standard I/O functions
│   │   ├── math.c                # Basic math functions
│   │   └── lib.h                 # Library interface
│   │
│   └── include/                  # Header files
│       ├── kernel/               # Kernel headers
│       ├── arch/                 # Architecture-specific headers
│       │   └── x86_64/           # x86_64 specific
│       │       ├── cpu.h        # CPU definitions
│       │       ├── msr.h        # Model Specific Registers
│       │       └── asm.h        # Assembly macros
│       ├── types.h               # Basic type definitions
│       └── stdint.h              # Standard integer types
│
├── userland/                     # User space programs
│   ├── init/                     # Init process
│   ├── shell/                    # Basic shell
│   ├── coreutils/                # Basic utilities (ls, cat, etc.)
│   └── libc/                     # C standard library implementation
│
├── tools/                        # Build and development tools
│   ├── mkfs/                     # File system creation tools
│   ├── debugger/                 # Custom debugging tools
│   └── testing/                  # Testing utilities
│
├── build/                        # Build artifacts (generated)
│   ├── obj/                      # Object files
│   ├── iso/                      # ISO creation directory
│   └── debug/                    # Debug symbols and dumps
│
├── tests/                        # Test suite
│   ├── unit/                     # Unit tests
│   ├── integration/              # Integration tests
│   ├── benchmarks/               # Performance tests
│   └── ci/                       # Continuous Integration scripts
│
├── config/                       # Configuration files
│   ├── link.ld                   # Linker script
│   ├── qemu.cfg                  # QEMU configuration
│   └── gdb.cfg                   # GDB configuration
│
├── scripts/                      # Build and utility scripts
│   ├── build.sh                  # Build script
│   ├── run.sh                    # Run in QEMU script
│   ├── debug.sh                  # Debug setup script
│   ├── test.sh                   # Test runner script
│   └── clean.sh                  # Cleanup script
│
├── external/                     # External dependencies
│   ├── grub/                     # GRUB bootloader files
│   └── toolchain/                # Cross-compilation toolchain
│
├── Makefile                      # Main build file
├── README.md                     # Project overview
├── LICENSE                       # License file
├── .gitignore                    # Git ignore patterns
└── .github/                      # GitHub specific
    └── workflows/                # GitHub Actions CI/CD
        └── ci.yml                # Continuous Integration
```

## File Naming Conventions

### Source Files

- **C files**: `lowercase_with_underscores.c`
- **Header files**: `lowercase_with_underscores.h`
- **Assembly files**: `lowercase_with_underscores.S` (capital S for preprocessed)
- **Linker scripts**: `descriptive_name.ld`

### Directory Names

- All lowercase with underscores for separation
- Descriptive and organized by functionality
- No spaces or special characters

## Compilation Units

### Core Kernel Components

1. **Boot**: `multiboot_header.S`, `boot.S`, `grub.cfg`
2. **Main**: `kernel/main.c` - kernel entry point
3. **Console**: VGA and serial output drivers
4. **Memory**: Paging, physical/virtual memory managers
5. **Interrupts**: IDT, ISR, timer, keyboard handlers
6. **Process**: Scheduler, PCB, context switching
7. **Syscalls**: System call interface and handlers
8. **Drivers**: Hardware abstraction and device drivers
9. **FileSystem**: VFS and specific filesystem implementations

### Build Targets

- **kernel.elf**: Main kernel binary
- **os.iso**: Bootable ISO image
- **kernel.map**: Symbol map for debugging
- **kernel.debug**: Debug symbols

## Memory Layout

### Kernel Memory Map

```
0x00000000 - 0x000FFFFF : Reserved (BIOS, bootloader)
0x00100000 - 0x001FFFFF : Kernel code and data (1MB-2MB)
0x00200000 - 0x003FFFFF : Kernel stack (2MB-4MB)
0x00400000 - 0x007FFFFF : Kernel heap (4MB-8MB)
0x00800000 - 0xBFFFFFFF : Available for processes
0xC0000000 - 0xFFFFFFFF : Kernel virtual space
```

### Virtual Memory Layout

```
0x0000000000000000 - 0x00007FFFFFFFFFFF : User space (128TB)
0xFFFF800000000000 - 0xFFFFFFFFFFFFFFFF : Kernel space (128TB)
```

## Build System

### Makefile Structure

```makefile
# Main targets
all: kernel.elf
iso: os.iso
clean: clean-all
run: run-qemu
debug: debug-gdb

# Component builds
kernel-objs: boot console memory interrupts process syscalls drivers fs lib
userland-objs: init shell coreutils libc

# Testing
test: unit-tests integration-tests
benchmark: performance-tests
```

### Build Dependencies

```
kernel.elf depends on:
├── boot/ (multiboot_header.o, boot.o)
├── kernel/ (main.o, console/, memory/, interrupts/, process/, syscalls/, drivers/)
├── fs/ (vfs.o, ramfs.o, tarfs.o)
└── lib/ (string.o, stdio.o, math.o)
```

## Development Workflow

### Phase 1: Foundation

1. Set up build system (`Makefile`, `link.ld`)
2. Implement boot sequence (`multiboot_header.S`, `boot.S`)
3. Basic kernel entry (`kernel/main.c`)
4. Console output (`console/vga.c`, `console/serial.c`)

### Phase 2: Core Systems

1. Interrupt handling (`interrupts/`)
2. Memory management (`memory/`)
3. Basic device drivers (`drivers/`)

### Phase 3: Process Management

1. Process structures (`process/`)
2. Scheduler implementation
3. Context switching

### Phase 4: User Space

1. System calls (`syscalls/`)
2. User space loader
3. Basic user programs (`userland/`)

### Phase 5: File System

1. VFS implementation (`fs/vfs.c`)
2. RAM file system (`fs/ramfs.c`)
3. Persistent storage drivers

## Testing Strategy

### Unit Tests

- Located in `tests/unit/`
- Test individual components in isolation
- Mock external dependencies

### Integration Tests

- Located in `tests/integration/`
- Test component interactions
- Full system boot tests

### CI/CD Pipeline

- Automated build verification
- QEMU-based testing
- Performance regression detection

## Documentation Standards

### Code Documentation

- Doxygen-style comments for functions
- Module-level documentation in headers
- Architecture decisions documented

### API Documentation

- Complete function signatures
- Parameter descriptions
- Return value specifications
- Usage examples

## Debugging Support

### Debug Symbols

- Separate debug information in `build/debug/`
- Symbol maps for address resolution
- Stack trace capability

### Development Tools

- GDB remote debugging support
- QEMU monitor integration
- Custom debugging utilities in `tools/debugger/`

## Configuration Management

### Build Configuration

- Debug vs Release builds
- Feature toggles
- Architecture-specific options

### Runtime Configuration

- QEMU vs hardware-specific settings
- Memory layout adjustments
- Device driver selection

---

_This structure supports incremental development, testing, and maintainability while providing clear separation of concerns and scalability for future enhancements._
