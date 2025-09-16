@echo off
echo Brandon Media OS - Setup Script
echo ================================

echo.
echo Checking required tools...

where x86_64-elf-gcc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] x86_64-elf-gcc not found
    echo Please install x86_64-elf cross-compiler toolchain
    goto :error
)
echo [OK] x86_64-elf-gcc found

where grub-mkrescue >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] grub-mkrescue not found
    echo Please install GRUB tools
    goto :error
)
echo [OK] grub-mkrescue found

where qemu-system-x86_64 >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] qemu-system-x86_64 not found
    echo Please install QEMU
    goto :error
)
echo [OK] qemu-system-x86_64 found

echo.
echo All required tools found!
echo.
echo Building Brandon Media OS...
make clean
make all

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Build completed successfully!
    echo.
    echo Run 'make run' to test in QEMU
    echo Run 'make gdb' for debugging
    echo Run 'make test' for automated testing
) else (
    echo [ERROR] Build failed!
    goto :error
)

goto :end

:error
echo.
echo Setup failed. Please check the requirements and try again.
exit /b 1

:end
echo.
echo Brandon Media OS setup complete!
pause