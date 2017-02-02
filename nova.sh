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

    if [ x"$1" = x"dev" ]
    then
        (cd src/main/java ; git clone git@github.com:NovaMods/Minecraft-Source.git net)
    else
        cp -r src/minecraft/net/ src/main/java/net/
    fi
    cp -r src/minecraft/mcp src/main/java/
    echo "Unpacked MCP"

    rm -rf src/minecraft
    rm -rf mcp

    echo "If patch file is UTF-16, convert it to UTF-8"
    file patches/nova.patch | grep UTF-16 && (iconv --from-code UTF-16 --to-code UTF-8 patches/nova.patch > x && mv x patches/nova.patch)

    echo "Injecting MCP into Minecraft..."
    (cd src/main/java/net ; patch -p1 ) < patches/nova.patch
    if [ $? != "0" ]; then
        echo "Failed to apply patches. Tell DethRaid about it and give him the full console log"
        exit -2
    fi

    echo "Downloading dependencies..."
    git submodule update --init --recursive

    echo "Nova set up successfully"
}

compile_java() {
    gradle fatjar
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
    compile_java
    compile_native_code
}

start_nova() {
    cd jars
    export LD_LIBRARY_PATH=../target/cpp
    java -classpath ../build/classes/main:libraries:libraries/com/google/code/findbugs/jsr305/3.0.1/jsr305-3.0.1.jar:libraries/com/google/code/gson/gson/2.2.4/gson-2.2.4.jar:libraries/com/google/guava/guava/17.0/guava-17.0.jar:libraries/com/ibm/icu/icu4j-core-mojang/51.2/icu4j-core-mojang-51.2.jar:libraries/com/mojang/authlib/1.5.22/authlib-1.5.22.jar:libraries/com/mojang/netty/1.6/netty-1.6.jar:libraries/com/mojang/realms/1.9.1/realms-1.9.1.jar:libraries/com/paulscode/codecjorbis/20101023/codecjorbis-20101023.jar:libraries/com/paulscode/codecwav/20101023/codecwav-20101023.jar:libraries/com/paulscode/libraryjavasound/20101123/libraryjavasound-20101123.jar:libraries/com/paulscode/librarylwjglopenal/20100824/librarylwjglopenal-20100824.jar:libraries/com/paulscode/soundsystem/20120107/soundsystem-20120107.jar:libraries/commons-codec/commons-codec/1.9/commons-codec-1.9.jar:libraries/commons-io/commons-io/2.4/commons-io-2.4.jar:libraries/commons-logging/commons-logging/1.1.3/commons-logging-1.1.3.jar:libraries/io/netty/netty-all/4.0.23.Final/netty-all-4.0.23.Final.jar:libraries/it/unimi/dsi/fastutil/7.0.12_mojang/fastutil-7.0.12_mojang.jar:libraries/net/java/dev/jna/jna/3.4.0/jna-3.4.0.jar:libraries/net/java/dev/jna/platform/3.4.0/platform-3.4.0.jar:libraries/net/java/jinput/jinput/2.0.5/jinput-2.0.5.jar:libraries/net/java/jinput/jinput-platform/2.0.5/jinput-platform-2.0.5-natives-windows.jar:libraries/net/java/jutils/jutils/1.0.0/jutils-1.0.0.jar:libraries/net/sf/jopt-simple/jopt-simple/4.6/jopt-simple-4.6.jar:libraries/org/apache/commons/commons-compress/1.8.1/commons-compress-1.8.1.jar:libraries/org/apache/commons/commons-lang3/3.3.2/commons-lang3-3.3.2.jar:libraries/org/apache/httpcomponents/httpclient/4.3.3/httpclient-4.3.3.jar:libraries/org/apache/httpcomponents/httpcore/4.3.2/httpcore-4.3.2.jar:libraries/org/apache/logging/log4j/log4j-api/2.0-beta9/log4j-api-2.0-beta9.jar:libraries/org/apache/logging/log4j/log4j-core/2.0-beta9/log4j-core-2.0-beta9.jar:libraries/org/lwjgl/lwjgl/lwjgl/2.9.4-nightly-20150209/lwjgl-2.9.4-nightly-20150209.jar:libraries/org/lwjgl/lwjgl/lwjgl-platform/2.9.4-nightly-20150209/lwjgl-platform-2.9.4-nightly-20150209-natives-windows.jar:libraries/org/lwjgl/lwjgl/lwjgl_util/2.9.4-nightly-20150209/lwjgl_util-2.9.4-nightly-20150209.jar:libraries/oshi-project/oshi-core/1.1/oshi-core-1.1.jar:versions/1.10/1.10.jar -Djava.library.path=versions/1.10/1.10-natives Start
}

create_patch() {
    (cd src/main/java/net/; git diff origin/minecraft-1.10-mcp ) > src/main/resources/patches/nova.patch
}

case $1 in
"setup")
    setup_nova "$@"
    ;;
"setup-dev")
    setup_nova dev
    ;;
"create-patch")
    create_patch
    ;;
"build")
    compile_all
    ;;
"start")
    start_nova
    ;;
*)
    show_help
    ;;
esac

