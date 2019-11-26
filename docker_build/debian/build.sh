#!/bin/bash

cd /

SW_HOME="/sw"
export VCPKG_HOME="${SW_HOME}/vcpkg"
CODE_HOME="/code"

# vcpkg installation
if [ -d "${VCPKG_HOME}" ]; then
    # looks like vcpkg is installed.
    echo "VCPKG is installed."
else
    cd ${SW_HOME}
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ./vcpkg integrate install
    cd /
fi

# vcpkg packages
${VCPKG_HOME}/vcpkg install curl
${VCPKG_HOME}/vcpkg install libarchive
${VCPKG_HOME}/vcpkg install cmocka
${VCPKG_HOME}/vcpkg install json-c
${VCPKG_HOME}/vcpkg install lua

echo "VCPKG packages installed."

if [ -d "$CODE_HOME/coll" ]; then
    echo "coll is already cloned."
    cd "$CODE_HOME/coll"
    git pull
    cd -
else
    cd "$CODE_HOME"
    git clone https://github.com/abhishekmishra/coll.git
    cd -
fi

if [ -d "$CODE_HOME/coll" ]; then
    echo "Start coll build"
    cd "$CODE_HOME/coll"
    rm -fR ./build
    ./build_linux.sh
    ./build/coll_test
    cd -
fi

if [ -d "$CODE_HOME/clibdocker" ]; then
    echo "clibdocker is already cloned."
    cd "$CODE_HOME/clibdocker"
    git pull
    cd -
else
    cd "$CODE_HOME"
    git clone https://github.com/abhishekmishra/clibdocker.git
    cd -
fi

if [ -d "$CODE_HOME/clibdocker" ]; then
    echo "Start clibdocker build"
    cd "$CODE_HOME/clibdocker"
    rm -fR ./build
    ./build_linux.sh
    cd -
fi

if [ -d "$CODE_HOME/cld" ]; then
    echo "cld is already cloned."
    cd "$CODE_HOME/cld"
    git pull
    cd -
else
    cd "$CODE_HOME"
    git clone https://github.com/abhishekmishra/cld.git
    cd -
fi

if [ -d "$CODE_HOME/cld" ]; then
    echo "Start cld build"
    cd "$CODE_HOME/cld"
    rm -fR ./build
    ./build_linux.sh
    cd -
fi