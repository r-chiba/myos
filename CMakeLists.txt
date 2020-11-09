cmake_minimum_required(VERSION 3.0)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER /usr/bin/clang-10)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_FLAGS "-d -std=c11 -Wall -Wextra -Wpedantic -nostdinc -nostdlib -ffreestanding -fno-builtin -fno-stack-protector -mno-sse")

project(myos C)

enable_language(ASM_NASM)
if (NOT CMAKE_ASM_NASM_COMPILER_LOADED)
    message(FATAL_ERROR "nasm can't be used")
endif()
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

add_subdirectory(src)

add_custom_target(run ALL
    COMMAND qemu-system-x86_64
                -m 4096
                -drive id=disk,file=os.img,format=raw,if=none
                -device ahci,id=ahci
                -device ide-hd,drive=disk,bus=ahci.0
                -boot a
                -display curses
                #-d int -no-reboot -no-shutdown
                #-s -S
    DEPENDS os.img
)

#add_custom_target(osimg ALL
#    SOURCES os.img
#)

add_custom_command(OUTPUT os.img
    COMMAND cp ipl.bin os.img
    COMMAND dd if=loader.bin of=os.img bs=1 seek=512 conv=notrunc > /dev/null 2>&1
    #COMMAND echo -en "\\x00" | dd of=os.img bs=1 seek=1474559 conv=notrunc > /dev/null 2>&1
    #COMMAND printf '\\000' | dd of=os.img bs=1 seek=1474559 conv=notrunc > /dev/null 2>&1
    #COMMAND printf '\\000' | dd of=os.img bs=1 seek=16383 conv=notrunc > /dev/null 2>&1
    COMMAND dd if=kernel.bin of=os.img bs=1 seek=16896 conv=notrunc > /dev/null 2>&1
    COMMAND printf '\\000' | dd of=os.img bs=1 seek=1474559 conv=notrunc > /dev/null 2>&1
    DEPENDS ipl.bin loader.bin kernel.bin
)

add_custom_command(OUTPUT ipl.bin
    COMMAND objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/src/boot/ipl/ipl ipl.bin
    DEPENDS ipl
)

add_custom_command(OUTPUT loader.bin
    COMMAND objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/src/boot/loader/loader loader.bin
    DEPENDS loader
)

add_custom_command(OUTPUT kernel.bin
    COMMAND objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/src/kernel/kernel kernel.bin
    DEPENDS kernel
)

