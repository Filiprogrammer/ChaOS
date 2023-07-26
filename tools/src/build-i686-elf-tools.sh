#!/bin/sh

set -e
cd "$(dirname "$(readlink -f "$0")")"

BINUTILS_VERSION=2.40
GCC_VERSION=13.1.0
BUILD_OUTPUT_DIR="$(pwd)/../i686-elf-tools-linux"

wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz

tar -xf binutils-$BINUTILS_VERSION.tar.gz
tar -xf gcc-$GCC_VERSION.tar.gz

mkdir $BUILD_OUTPUT_DIR

mkdir binutils-$BINUTILS_VERSION/build
cd binutils-$BINUTILS_VERSION/build
LDFLAGS=-s ../configure --target=i686-elf --with-sysroot --disable-nls --disable-werror --prefix="${BUILD_OUTPUT_DIR}"
make -j$(nproc) MAKEINFO=true
make install MAKEINFO=true
cd ../..

cd gcc-$GCC_VERSION
contrib/download_prerequisites
mkdir build
cd build
LDFLAGS=-s ../configure --target=i686-elf --disable-nls --enable-languages=c,c++ --without-headers --prefix="${BUILD_OUTPUT_DIR}"
make -j$(nproc) all-gcc
make install-gcc
make -j$(nproc) all-target-libgcc
make install-target-libgcc
cd ../..
