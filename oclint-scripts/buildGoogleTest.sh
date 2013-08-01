#! /bin/sh -e

# setup environment variables
OS=$(uname -s)
CWD=`pwd`
PROJECT_ROOT="$CWD/.."
GOOGLE_TEST_SRC="$PROJECT_ROOT/googletest"
GOOGLE_TEST_BUILD="$PROJECT_ROOT/build/googletest"

# create directory and prepare for build
mkdir -p $GOOGLE_TEST_BUILD
cd $GOOGLE_TEST_BUILD

# configure and build
if [ "$OS" = "Darwin" ]; then
    DARWIN_VERSION=`sysctl -n kern.osrelease | cut -d . -f 1`
    if [ $DARWIN_VERSION -lt 13 ]; then
        cmake -D CMAKE_CXX_FLAGS="-std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}" $GOOGLE_TEST_SRC
    else
        cmake $GOOGLE_TEST_SRC
    fi
else
    cmake $GOOGLE_TEST_SRC
fi
make

# back to the current folder
cd $CWD
