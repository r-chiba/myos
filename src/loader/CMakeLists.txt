set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} \
         -target x86_64-pc-win32-coff"
)

set(CMAKE_C_LINK_EXECUTABLE
        "/usr/bin/lld-link-10 \
         <CMAKE_C_LINK_FLAGS> \
         <LINK_FLAGS> \
         -subsystem:efi_application \
         -nodefaultlib \
         -dll \
         -entry:UefiMain \
         <OBJECTS> \
         <LINK_LIBRARIES> \
         -out:<TARGET>"
)

#file(GLOB_RECURSE SRCS_ASM "*.asm")
file(GLOB_RECURSE SRCS_C "*.c")
list(APPEND SRCS_C
    "${CMAKE_CURRENT_SOURCE_DIR}/../freebsd/stand/libsa/printf.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../freebsd/lib/libc/string/strlen.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../freebsd/lib/libc/string/memcpy.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../freebsd/lib/libc/string/memset.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../freebsd/lib/libc/string/memcmp.c")
#set_source_files_properties(${SRCS_ASM} PROPERTIES
#    LANGUAGE ASM_NASM)
#    #LINKER_LANGUAGE C)
#add_executable(loader ${SRCS_ASM} ${SRCS_C})
#add_executable(loader ${SRCS_ASM})
add_executable(loader ${SRCS_C})
target_include_directories(loader
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include/
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include/loader
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include/loader/edk2/
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include/loader/edk2/X64/
)
