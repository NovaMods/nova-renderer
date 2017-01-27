<#
.SYNOPSIS
    A simple script to handle the build lifecycle of the Nova Renderer

.DESCRIPTION
    Allows you to setup Nova on a new machine, compile Nova, and run Nova, all from a single script with a simple interface

.PARAMETER setup
    Downloads MCP, unzips it, moves it to the right place, and applies the source code transformations to Minecraft so that Nova will be run properly

.PARAMETER build
    Compiles the native code, moves the output DLL to the correct directory, then compiles the Java code

.PARAMETER nativeOnly
    Only compiles the C++ code. Does not compile the Java code

.PARAMETER clean
    Deletes all compiled code and generated build files

.PARAMETER run
    Runs Nova

.PARAMETER buildEnvironment
    Specifies the build environment to use to compile the native code. Can be one of 'mingw' or 'msvc'. Default is 'mingw'

    If set to mingw, this script assumes that you have MinGW all set up, and have mingw32-make on your path. If that's not true, you better fix it now!

    If set to msvc, this script assumes you have Visual Studio 2015 installed and have devenv.exe on your path. If that's not the case, fix it now!

.NOTES
    File Name       : nova.ps
    Author          : David "DethRaid" Dubois (dexcelstraun7@gmail.com)

.EXAMPLE
    nova -setup
    Sets up Nova, downloading MCP and using that to decompile Minecraft. Also applies the source code transformations which integrate Nova with Minecraft

.EXAMPLE
    nova -build -compiler mingw
    Builds Nova, using the MinGW build environment to compile the native code

.EXAMPLE
    nova -build -nativeOnly
    Builds Nova, compiling only the native code and skipping the Java compilation

.EXAMPLE
    nova -run
    Runs Nova, launching Minecraft and everything
#>

param (
    [switch]$setup = $false,
    [switch]$build = $false,
    [switch]$clean = $false,
    [switch]$run = $false,
    [switch]$nativeOnly = $false,
    [switch]$makePatches = $false,
    [string]$buildEnvironment = "mingw"
)

################################################################################
#  Setup                                                                       #
################################################################################

function New-NovaEnvironment {
    <#
    .SYNOPSIS
        Sets up Nova in a new environment
    .DESCRIPTION
        Sets up Nova in a new environment by downloading MCP, unpacking it, moving it to the right location, and applying the source code transformations
    #>

    Write-Host "Downloading MCP..."

    New-Item "mcp" -ItemType Directory
    Write-Information "Created directory for MCP to live in"
    Set-Location "mcp"
    Write-Information "Moved to MCP directory"

    $wc = New-Object System.Net.WebClient
    $wc.DownloadFile("http://www.modcoderpack.com/website/sites/default/files/releases/mcp931.zip", "$PSScriptRoot/mcp/mcp.zip")
    Write-Host "Downloaded MCP successfully"

    Unzip -zipfile "$PSScriptRoot/mcp/mcp.zip" -outpath "$PSScriptRoot/mcp/"
    Remove-Item "mcp.zip"
    robocopy "." "..\" "*" /s
    Set-Location ".."
    cmd.exe /C "$PSScriptRoot/decompile.bat"
    robocopy "src\minecraft" "src\main\java" "*" /s
    Write-Host "Unpacked MCP"

    # Clean up the intermediary files
    Remove-Item "src\minecraft" -Recurse
    Remove-Item "mcp" -Recurse

    # Apply our patch file
    Write-Host "Injecting Nova into Minecraft..."
    Set-Location src\main\resources\patches
    ..\..\..\..\runtime\bin\applydiff.exe -p0 -i .\nova.diff
    Set-Location ..\..\..\..\

    Write-Host "Downloading dependencies..."
    git submodule update --init --recursive

    Write-Host "Nova set up successfully"
}

function Unzip([string]$zipfile, [string]$outpath) {
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $outpath)
}

################################################################################
#  Build                                                                       #
################################################################################

function New-NovaCode([string]$buildEnvironment) {
    <#
    .SYNOPSIS
        Compiles all the Nova code you can possible imagine
    .PARAMETER buildEnvironment
        Specifies the build environment you with to use. Must be either 'mingw' or 'msvc'.
    #>

    Write-Host "Starting compilation of Nova..."

    if($buildEnvironment -eq "mingw") {
        Write-Host "Building with MinGW"
        New-MinGWNovaBuild

    } elseif($buildEnvironment -eq "msvc") {
        Write-Host "Building wth Visual Studio"
        New-VisualStudioBuild
    }

    if($buildGood -eq $false) {
        Write-Error "Compilation of native code failed, aborting"
        return $false
    }

    # I assume that everything worked properly

    # TODO: Build the patches here

    if($nativeOnly -eq $false) {
        # Compile the Java code

        gradle fatjar

        if($LASTEXITCODE -ne 0) {
            Write-Error "Gradle invocation failed, aborting"
            return $false
        }
    }

    Write-Host "Nova compiled!"
    return $true
}

function New-MinGWNovaBuild {
    <#
    .SYNOPSIS
        Builds Nova, using MinGW to compile the native code
    #>

    if(Test-Command -command "mingw32-make.exe") {
        # I don't care if it can't make the directories, I just want to make sure they're there
        New-Item target -ItemType Directory >$null 2>&1
        New-Item target\cpp -ItemType Directory >$null 2>&1
        Set-Location target\cpp

        # MinGW is probably installed, let's assume it is
        cmake -G "MinGW Makefiles" ../../src/main/cpp

        # Compile the code
        mingw32-make -f Makefile nova-renderer

        # Copy output DLL to the correct location
        robocopy "." "..\..\jars\versions\1.10\1.10-natives\" "libnova-renderer.dll"

        # $mcpLoc = Read-Host "Please enter the path to your MinGW installation. This should be the path to the folder with mingw-w64.bat in it"
        # robocopy "$($mcpLoc)\mingw64\bin" "..\..\jars\versions\1.10\1.10-natives" "libstdc++-6.dll"

        Set-Location ..\..

    } else {
        Write-Error "Could not call the MinGW Make tool, unable to build Nova. Please install MinGW AND ensure that mingw32-make is in your path"
        return $false
    }
}

function New-VisualStudioBuild {
    <#
    .SYNOPSIS
        Builds the C++ code in Nova, using Visual Studio 2015 to compile the code
    #>

    if(Test-Command -command "devenv.exe") {
        # Visual Studio is probably installed, hopefully they have the correct version
        Invoke-CMake -generator "Visual Studio 14 2015"

        # Compile the code
        # TODO: Verify that this is the actual name of the solution file
        devenv NovaRenderer.sln /Build Debug

        # Copy the DLL to the correct location
        # TODO: Verify that this is the name of the DLL
        robocopy "." "..\..\jars\versions\1.10\1.10-natives\" "nova-renderer.dll"
    } else {
        Write-Error "Could not call the Visual Studio make tool, unable to build Nova. Please enstall Visual Stusio 2015 and ensure that devenv.exe is on your path"
    }
}

function Test-Command([string]$command) {
    if($null -eq (Get-Command $command -ErrorAction SilentlyContinue)) {
        Write-Error "Could not find the $command executable. Please ensure that it's in your PATH, then run this script again" -Category ResourceUnavailable
        return $false
    } else {
        # I could return the result of the Get-Command test, but I want to output an error message upon failure
        return $true
    }
}

################################################################################
#  Clean                                                                       #
################################################################################

function Remove-BuildFiles {
    <#
    .SYNOPSIS
        Removes all the build files, including generated CMake files, compiled code, and any makefiles that happened to happen
    #>

    Set-Location target
    Remove-Item * -Recurse
    Set-Location ..

    Remove-Item target
}

################################################################################
#  Run                                                                         #
################################################################################

function Invoke-Nova {
    <#
    .SYNOPSIS
        Lanuches Nova from the correct directory
    #>

    Write-Host "Launching Nova..."

    Set-Location "jars"
    $env = "-classpath ..\build\classes\main;libraries;libraries/com/google/code/findbugs/jsr305/3.0.1/jsr305-3.0.1.jar;libraries/com/google/code/gson/gson/2.2.4/gson-2.2.4.jar;libraries/com/google/guava/guava/17.0/guava-17.0.jar;libraries/com/ibm/icu/icu4j-core-mojang/51.2/icu4j-core-mojang-51.2.jar;libraries/com/mojang/authlib/1.5.22/authlib-1.5.22.jar;libraries/com/mojang/netty/1.6/netty-1.6.jar;libraries/com/mojang/realms/1.9.1/realms-1.9.1.jar;libraries/com/paulscode/codecjorbis/20101023/codecjorbis-20101023.jar;libraries/com/paulscode/codecwav/20101023/codecwav-20101023.jar;libraries/com/paulscode/libraryjavasound/20101123/libraryjavasound-20101123.jar;libraries/com/paulscode/librarylwjglopenal/20100824/librarylwjglopenal-20100824.jar;libraries/com/paulscode/soundsystem/20120107/soundsystem-20120107.jar;libraries/commons-codec/commons-codec/1.9/commons-codec-1.9.jar;libraries/commons-io/commons-io/2.4/commons-io-2.4.jar;libraries/commons-logging/commons-logging/1.1.3/commons-logging-1.1.3.jar;libraries/io/netty/netty-all/4.0.23.Final/netty-all-4.0.23.Final.jar;libraries/it/unimi/dsi/fastutil/7.0.12_mojang/fastutil-7.0.12_mojang.jar;libraries/net/java/dev/jna/jna/3.4.0/jna-3.4.0.jar;libraries/net/java/dev/jna/platform/3.4.0/platform-3.4.0.jar;libraries/net/java/jinput/jinput/2.0.5/jinput-2.0.5.jar;libraries/net/java/jinput/jinput-platform/2.0.5/jinput-platform-2.0.5-natives-windows.jar;libraries/net/java/jutils/jutils/1.0.0/jutils-1.0.0.jar;libraries/net/sf/jopt-simple/jopt-simple/4.6/jopt-simple-4.6.jar;libraries/org/apache/commons/commons-compress/1.8.1/commons-compress-1.8.1.jar;libraries/org/apache/commons/commons-lang3/3.3.2/commons-lang3-3.3.2.jar;libraries/org/apache/httpcomponents/httpclient/4.3.3/httpclient-4.3.3.jar;libraries/org/apache/httpcomponents/httpcore/4.3.2/httpcore-4.3.2.jar;libraries/org/apache/logging/log4j/log4j-api/2.0-beta9/log4j-api-2.0-beta9.jar;libraries/org/apache/logging/log4j/log4j-core/2.0-beta9/log4j-core-2.0-beta9.jar;libraries/org/lwjgl/lwjgl/lwjgl/2.9.4-nightly-20150209/lwjgl-2.9.4-nightly-20150209.jar;libraries/org/lwjgl/lwjgl/lwjgl-platform/2.9.4-nightly-20150209/lwjgl-platform-2.9.4-nightly-20150209-natives-windows.jar;libraries/org/lwjgl/lwjgl/lwjgl_util/2.9.4-nightly-20150209/lwjgl_util-2.9.4-nightly-20150209.jar;libraries/oshi-project/oshi-core/1.1/oshi-core-1.1.jar;versions/1.10/1.10.jar -Djava.library.path=versions/1.10/1.10-natives"
    . java $env Start
    Set-Location ".."
}

################################################################################
#  Make Patches                                                                #
################################################################################

function New-Patches {
    Set-Location "../"
    New-Item "mcp-raw" -ItemType Directory >$null 2>&1
    Set-Location mcp-raw

    $wc = New-Object System.Net.WebClient
    $wc.DownloadFile("http://www.modcoderpack.com/website/sites/default/files/releases/mcp931.zip", "mcp.zip")
    Unzip -zipfile "mcp.zip" -outpath "$PSScriptRoot\..\mcp-raw"
    cmd.exe /C "decompile.bat"

    git init
    git add -A :/
    git commit -m "Commit of initial MC code"

    robocopy "..\Nova Renderer\src\main\java\net" "src\minecraft\net" "*" /S /PURGE

    git diff >> nova.patch

    Set-Location "..\Nova Renderer"
}

################################################################################
#  Main                                                                        #
################################################################################

if($setup -eq $true) {
    New-NovaEnvironment
}

if($makePatches -eq $true) {
    New-Patches
}

if($build -eq $true) {
    New-NovaCode -buildEnvironment $buildEnvironment
}

if($clean -eq $true) {
    Remove-BuildFiles
}

if($run -eq $true) {
    Invoke-Nova
}
