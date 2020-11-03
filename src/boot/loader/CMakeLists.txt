set(CMAKE_ASM_NASM_LINK_EXECUTABLE "/usr/bin/ld.lld-10 <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> -T ${CMAKE_CURRENT_SOURCE_DIR}/<TARGET>.ld <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
#set(CMAKE_C_LINK_EXECUTABLE "/usr/bin/ld.lld-10 <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -T ${CMAKE_CURRENT_SOURCE_DIR}/<TARGET>.ld <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

file(GLOB_RECURSE SRCS_ASM "*.asm")
#file(GLOB_RECURSE SRCS_C "*.c")
#list(APPEND SRCS_C
#    "${CMAKE_CURRENT_SOURCE_DIR}/../../freebsd/stand/libsa/printf.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/../../freebsd/lib/libc/string/strlen.c")
set_source_files_properties(${SRCS_ASM} PROPERTIES
    LANGUAGE ASM_NASM)
    #LINKER_LANGUAGE C)
#add_executable(loader ${SRCS_ASM} ${SRCS_C})
add_executable(loader ${SRCS_ASM})
target_include_directories(loader
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../common/
)
