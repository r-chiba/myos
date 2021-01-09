# MyOS
## Description
This is a personal project to study how OS's work.

## Assumption
* x86-64 CPU
* Boot from UEFI

## Build and Run
```console
$ apt-get install build-essential clang-10 lld-10 cmake qemu-system
# get an OVMF image and set the OVMF_PATH variable in CMakeLists.txt to the path
$ mkdir build && cd build
$ cmake ..
$ make
```

## References
* [Advent Calendar 2018：OS自作](https://ja.tech.jar.jp/ac/2018/day00.html)
* [OSDev.org](https://wiki.osdev.org/Main_Page)
* [FreeBSD Architecture Handbook](https://www.freebsd.org/doc/en_US.ISO8859-1/books/arch-handbook/index.html)
* [linux-insides](https://0xax.gitbooks.io/linux-insides/content/)
