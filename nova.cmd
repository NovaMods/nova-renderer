@ECHO off

REM Name:        nova.cmd
REM Purpose:     Handle the buid lifecycle of the Nova Renderer
REM Author:      dexcelstraun7@gmail.com
REM Revision:    August 2016 - initial version

REM When downloading everything, we should download the pre-build GLFW binaries because building from source code is hard

SETLOCAL ENABLEEXTENSIONS
SET me=%~n0
SET parent=%~dp0

SET command=%1

REM error codes
REM These error codes are OR'd together. That's possibly important.
SET /A errno=0
SET /A ERROR_CMAKE_NOT_FOUND=1
SET /A ERROR_COMPILER_NOT_PRESENT=2
SET /A ERROR_MAKE_NOT_PRESENT=4

IF /I "%1" == "help" (
    CALL :showhelp
    EXIT /B 0
) ELSE IF /I "%1" == "build" (
    SET PREFIX_PATH=%~3
    SET CXX=%2
    CALL :build
    EXIT /B %errno%
) ELSE IF /I "%1" == "clean" (
    CALL :clean
    EXIT /B %errno%
) ELSE IF /I "%1" == "setup" (
    CALL :setup
    EXIT /B %errno%
)

:showhelp
    ECHO This is a utility to build, run, and package the Minecraft Nova Renderer
    ECHO Usage: 
    ECHO     nova help
    ECHO     nova setup
    ECHO     nova build ^<compiler name^> ^<CMAKE_PREFIX_PATH^>
    ECHO     nova clean
    ECHO     nova run
    ECHO:
    ECHO Command detail:
    ECHO     help        Print this message
    ECHO     setup       Downloads MCP, sets up MCP, downloads GLM and GLFW to the correct directories
    ECHO     build       Compile the Nova Renderer
    ECHO     clean       Deletes all generated files, including generated build scripts and compiled code
    ECHO     run         Launch Minecraft with the Nova Renderer
    ECHO:
    ECHO Arguments for build:
    ECHO     ^<compiler name^>      Optional but encouraged. Specify the C++ compiler to use. Supported options are g++ and msvc. If this parameter is not present, this script will try to detect if you have g++ or msvc installed, and will use whichever it finds
    ECHO:
    ECHO     ^<CMAKE_PREFIX_PATH^>  Optional. Tells CMake the prefix path where all your dependencies are installed. Eventually this script will manage downloading all dependencies and put them in a good location, but this will work for now
    ECHO:
    ECHO     Note that, because I'm bad at scripting, if you want to specify the prefix path, you also have to specify the compiler name. However, you can put the compiler name all by itself and there will be no problems
    ECHO:
    PAUSE

REM Downloads all the super spicy dependencies and spices them up
REM Now with extra spice!
:setup
    CALL :setupmcp
    CALL :applypatches
    CALL :setupcxxdependencies
    EXIT /B

REM Basic idea: download MCP, unzip it, run the decompile.bat, party
REM Advanced idea: Create a new folder for MCP, download it there
:setupmcp
    MKDIR mcp
    CD mcp

    EXIT /B

REM applies the soruce code patches which integrate Nova into Minecraft
REM in the future, this may go away and I might figure out how to do this at runtime. That would be cool because then I could do it in the real world
:applypatches
    EXIT /B

REM Downloads the C++ code's dependencies (GLM and GLFW for now), unzipping and copying where appropriate
:setupcxxdependencies
    EXIT /B

REM Cleans up all the compiled files. ALL OF THEM
REM This includes Java .class files, all files generated from the C++ code, the things that CMake generates, etc
:clean
    CD src\native
    mingw32-make -f Makefile clean
    DEL CMakeCache.txt
    DEL cmake_install.cmake

    ECHO Generated files deleted

    EXIT /B

:build
    ECHO Starting compilation of Nova...
    ECHO:
    IF NOT DEFINED CXX (
        CALL :findcompiler
    )

    REM make sure the compiler we have selected exists on the system path
    call %%CXX%% 2> nul
    IF /I "%ERRORLEVEL%" EQU "9009" (
        ECHO It seems like your specified compiler doesn't exist on your system PATH. That's going to be a problem. This script will still generate build files for you, but it won't be able to actually compile the native code. You'll have to do that manually
        SET /A error^|=%ERROR_COMPILER_NOT_PRESENT%
    )

    CD src\native

    CALL :generatebuildfiles
    CALL :compilenative
    CALL :movedlls

    cd ..\..

    CALL :compilejava

    EXIT /B

REM Tries to find a compiler on the system path. First searches for G++, then MSVC, then asks the user what they want
:findcompiler
    ECHO Compiler not specified, trying to auto-detect...
    REM try to use G++, then try to use msvc
    g++ --version
    IF /I "%ERRORLEVEL%" EQU "9009" (
        REM g++ not found. Try MSVC
        REM Google tells me that trying to use MSVC from the command line is madness itself, so I'll just ask the user what
        REM compiler they want

        SET /P "Could not auto-detect C++ compiler. What C++ compiler would you like to use? We support g++ and msvc " %cxx%

    ) ELSE (
        ECHO G++ found. Assuming MinGW.
        SET CXX=g++
    )
    EXIT /B

REM Generates the build files needed to, you know, build Nova
:generatebuildfiles
    REM convert from the value in cxx to a value that CMake understands
    IF /I "%CXX%" EQU "g++" (
        REM this is easy. We're on Windows, and we have G++, so we're going to generate MinGW makefiles
        cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="%PREFIX_PATH%" src
    )
    
    IF /I "%ERRORLEVEL%" EQU "9009" (
        ECHO CMake not found. Please ensure that CMake is in your path, then run this script again
        SET /A error^|=%ERROR_CMAKE_NOT_FOUND%
    )
    
    EXIT /B

REM executes compilation of the C++ part of Nova
:compilenative
    REM first, we need to check if we're using G++ or MSVC
    IF /I "%CXX%" EQU "g++" (
        ECHO Compiling with G++ under MinGW

        mingw32-make -f Makefile nova-renderer

        IF /I "%ERRORLEVEL%" EQU "9009" (
            ECHO Could not find mingw32-make, which I wanted to use to build this code. Please install MinGW and ensure that mingw32-make is in your PATH, or tell me to compile with MSVC
            SET /A error^|=%ERROR_MAKE_NOT_PRESENT%
            EXIT /B
        ) ELSE IF /I "%ERRORLEVEL%" EQU "0" (
            ECHO Build successful
        )

        REM Now to copy all the DLLs we need to their proper directories...
        REM We need to copy both nova-renderer.dll and libstdc+++.dll

    )

    EXIT /B

REM This one is simple. All we need to do is copy some files to where they need to be
REM Copying nova is easy. nova-renderer.dll hangs out in the root directory, so we'll just copy it to where we need it
REM If we're using MinGW, we probably need to copy MinGW's libc++ thing. Where that lives depends on a lot.
:movedlls
    XCOPY /Y libnova-renderer.dll ..\..\jars\versions\1.10\1.10-natives\
    EXIT /B

REM compiles the Java code. This is simple
:compilejava
    CALL gradle build
    EXIT /B
