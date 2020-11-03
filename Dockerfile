FROM ubuntu:20.04
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update

# setup build environments
RUN apt-get install -y --no-install-recommends \
        build-essential nasm clang-10 lld-10 cmake

# setup execution environments
RUN apt-get install -y --no-install-recommends qemu-system

COPY . /usr/src
WORKDIR /usr/src

# build and run the OS
RUN ls -al
RUN mkdir build && cd build && cmake .. && make

