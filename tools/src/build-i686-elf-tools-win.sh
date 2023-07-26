#!/bin/sh

set -e
cd "$(dirname "$(readlink -f "$0")")"

if [ ! -d "../i686-elf-tools-linux" ]; then
    # If the linux tools do not exist, build them first because they are required to build the windows tools
    ./build-i686-elf-tools.sh
fi

BINUTILS_VERSION=2.40
GCC_VERSION=13.1.0
BUILD_OUTPUT_DIR="$(pwd)/../i686-elf-tools-windows"

git clone --depth 1 https://github.com/mxe/mxe.git
cd mxe
make -j$(nproc) gcc
cd ..

export PATH="$(pwd)/mxe/usr/bin:$(pwd)/../i686-elf-tools-linux/bin:$PATH"

wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz

tar -xf binutils-$BINUTILS_VERSION.tar.gz
tar -xf gcc-$GCC_VERSION.tar.gz

mkdir $BUILD_OUTPUT_DIR

mkdir -p binutils-$BINUTILS_VERSION/build
cd binutils-$BINUTILS_VERSION/build
rm ./config.cache || true
LDFLAGS=-s ../configure --host=i686-w64-mingw32.static --target=i686-elf --with-sysroot --disable-nls --disable-werror --prefix="${BUILD_OUTPUT_DIR}"
make -j$(nproc) MAKEINFO=true
make install MAKEINFO=true
cd ../..

cd gcc-$GCC_VERSION
contrib/download_prerequisites
mkdir -p build
cd build
LDFLAGS=-s ../configure --host=i686-w64-mingw32.static --target=i686-elf --disable-nls --enable-languages=c,c++ --without-headers --prefix="${BUILD_OUTPUT_DIR}"
make -j$(nproc) all-gcc
make install-gcc
make -j$(nproc) all-target-libgcc
make install-target-libgcc
cd ../..
