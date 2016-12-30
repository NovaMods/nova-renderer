#!/bin/bash

show_help() {
    echo "
nova.sh - a simple script to handle the build lifecycle of the Nova renderer

Allows you to set up Nova on a new machine, compile Nova, and run Nova (hopefully), all from a single script with a simple interface

Available parameters:

setup - downloads MCP and uses it to decompile Minecraft, downloads Nova's dependencies, and applies the source code patches to Minecraft

build - compiles the C++ code, moves the shared library to the proper directory, and compiles the Java code

nativeOnly - only compiles the C++ code and moves the shared library to the proper directory. Does not compile the Java code. This option is only considered if 'build' is also specified. 'build' must be the first argument, then this one

clean - deletes all compiled code and generated build files

run - runs Nova

Author - David \"DethRaid\" Dubois (dexcelstraun7@gmail.com)
"
}

setup_nova() {
    echo "Begin setup of Nova"

    echo "Downloading MCP"
    mkdir mcp
    cd mcp
    wget http://www.modcoderpack.com/website/sites/default/files/releases/mcp931.zip
    unzip mcp931.zip
    echo "Extracted MCP"
    cp -r * ..
    cd ..

    ./decompile.sh
    if [ $? != "0" ]; then
        echo "MCP failed to decompile Minecraft. Check the above text for errors"
        exit -1
    fi

    cp -r src/minecraft src/main/java
    echo "Unpacked MCP"

    rm -rf src/minecraft
    rm -rf mcp

    echo "Injecting MCP into Minecraft..."
    patch -p0 -i src/main/resources/patches/nova.diff
    if [ $? != "0" ]; then
        echo "Failed to apply patches. Tell DethRaid about it and give him the full console log"
        exit -2
    fi

    echo "Downloading dependencies..."
    git submodule update --init --recursive

    echo "Nova set up successfully"
}

compile_native_code() {
    echo "Compiling native code"
    mkdir -p target/cpp
    cd target/cpp

    cmake -g "Unix Makefiles" ../../src/main/cpp 
    if [ $? != "0" ]; then
        echo "Failed to execute CMake. Possibly you're missing the Vulkan library, which GLFW requires because it's stupid"
        exit -3
    fi

    make -f Makefile nova-renderer
    if [ $? != "0" ]; then
        echo "Could not compile Nova. Please submit a GitHub issue with the full console log so DethRaid can fix it"
        return -4
    fi
}

compile_all() {
    compile_native_code
}

case $1 in 
"setup") 
    setup_nova
    ;;
"build")
    compile_all
    ;;
*) 
    show_help
    ;;
esac

