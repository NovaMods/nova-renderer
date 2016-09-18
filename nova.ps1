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
    [string]$buildEnvironment = "mingw"
)

################################################################################
#                                                                              #
#  Setup                                                                       #
#                                                                              #
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

    Write-Host "Downloading GLFW..."
    git submodule update --init -- src/main/cpp/3rdparty/easyloggingpp

    Write-Host "Downloading miniz..."
    git submodule update --init -- src/main/cpp/3rdparty/miniz

    Write-Host "Downloading json..."
    git submodule update --init -- src/main/cpp/3rdparty/json

    Write-Host "Downloading GLFW..."
    git submodule update --init -- src/main/cpp/3rdparty/glfw

    Write-Host "Downloading GLM..."
    git submodule update --init -- src/main/cpp/3rdparty/glm

    Write-Host "Nova set up successfully"
}

function Unzip([string]$zipfile, [string]$outpath) {
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $outpath)
}

################################################################################
#                                                                              #
#  Build                                                                       #
#                                                                              #
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
        $buildGood = New-MinGWNovaBuild

    } elseif($buildEnvironment -eq "msvc") {
        Write-Host "Building wth Visual Studio"
        $buildGood = New-VisualStudioBuild
    }

    if($buildGood -eq $false) {
        Write-Error "Compilation of native code failed, aborting"
        return $false
    }

    # I assume that everything worked properly

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
        if($LASTEXITCODE -ne 0) {
            Write-Error "Failed to invoke CMake, aborting compilation"
            return $false
        }

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

    return $true
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
#                                                                              #
#  Clean                                                                       #
#                                                                              #
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
#                                                                              #
#  Run                                                                         #
#                                                                              #
################################################################################

function Invoke-Nova {
    <#
    .SYNOPSIS
        Lanuches Nova from the correct directory
    #>

    Write-Host "Launching Nova..."

    Set-Location "jars"
    $env = "-Djava.library.path=$PSScriptRoot\jars\versions\1.10\1.10-natives"
    $jarfile = "$PSScriptRoot\build\libs\Nova Renderer-0.3.jar"
    . java $env -jar $jarfile
    Set-Location ".."
}

################################################################################
#                                                                              #
#  Main                                                                        #
#                                                                              #
################################################################################

if($setup -eq $true) {
    New-NovaEnvironment
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
