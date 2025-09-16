/* multiboot_header.S - minimal Multiboot header (multiboot v1) */
.section .multiboot
.align 4
.long 0x1BADB002       /* magic */
.long 0x00010003       /* flags: memory info + mods + header addr align (example) */
.long -(0x1BADB002 + 0x00010003) /* checksum */