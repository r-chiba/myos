cmake_minimum_required(VERSION 3.12)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER /usr/bin/clang-10)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_FLAGS
        "-std=c11 \
         -Wall \
         -Wextra \
         -Wpedantic \
         -nostdinc \
         -nostdlib \
         -fno-builtin \
         -fno-stack-protector \
         -mno-sse \
         -mno-red-zone"
)

project(myos C)

#enable_language(ASM_NASM)
#if (NOT CMAKE_ASM_NASM_COMPILER_LOADED)
#    message(FATAL_ERROR "nasm can't be used")
#endif()
#set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set(OVMF_PATH $ENV{HOME}/src/edk2/Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd)

add_subdirectory(src)

#add_custom_target(run ALL
add_custom_target(run
    COMMAND qemu-system-x86_64
                -m 4G
                -smp cores=2,threads=1,sockets=2
                --bios ${CMAKE_CURRENT_BINARY_DIR}/image/OVMF.fd
                -hda fat:rw:${CMAKE_CURRENT_BINARY_DIR}/image
                -net none
                #-d int -no-reboot -no-shutdown
                #-s
                #-s -S
    DEPENDS image
)

add_custom_target(image ALL
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/image/EFI/BOOT
    COMMAND cp ${OVMF_PATH} ${CMAKE_CURRENT_BINARY_DIR}/image
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/src/loader/loader ${CMAKE_CURRENT_BINARY_DIR}/image/EFI/BOOT/BOOTX64.efi
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/src/kernel/kernel ${CMAKE_CURRENT_BINARY_DIR}/image/kernel.elf
    COMMAND objcopy -O binary --set-section-flags .bss=alloc,load,contents
                ${CMAKE_CURRENT_BINARY_DIR}/src/kernel/kernel
                ${CMAKE_CURRENT_BINARY_DIR}/image/kernel.bin
    DEPENDS loader kernel
)

