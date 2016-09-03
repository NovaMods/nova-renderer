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

    Write-Information "We're at $PSScriptRoot"

    New-Item "mcp" -ItemType Directory
    Write-Information "Created directory for MCP to live in"
    Set-Location "mcp"
    Write-Information "Moved to MCP directory"

    $wc = New-Object System.Net.WebClient
    $wc.DownloadFile("http://www.modcoderpack.com/website/sites/default/files/releases/mcp931.zip", "$PSScriptRoot/mcp/mcp.zip")
    Write-Information "Downloaded MCP successfully"

    Unzip -zipfile "$PSScriptRoot/mcp/mcp.zip" -outpath "$PSScriptRoot/mcp/"
    Write-Information "Unzipped MCP" 
    Remove-Item "mcp.zip"
    Write-Information "Deleted MCP zip (but not really)"
    robocopy "." "..\" "*" /s
    Write-Information "Copied MCP files to the root directory"
    Set-Location ".."
    Write-Information "Followed the files I've copied"
    cmd.exe /C "$PSScriptRoot/decompile.bat"
    Write-Information "Decompiled MCP"
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

    Write-Information "Starting compilation of Nova..."

    if($buildEnvironment -eq "mingw") {
        Write-Information "Building with MinGW"
        New-MinGWNovaBuild

    } elseif($buildEnvironment -eq "msvc") {
        Write-Information "Building wth Visual Studio"
        New-VisualStudioBuild
    }

    # I assume that everything worked properly

    if($nativeOnly -eq $false) {
        # Compile the Java code

        gradle build
    }

    Write-Information "Nova compiled!"
}

function New-MinGWNovaBuild {
    <#
    .SYNOPSIS
        Builds Nova, using MinGW to compile the native code
    #>

    if(Test-Command -command "mingw32-make.exe") {
        New-Item target -ItemType Directory
        New-Item target\cpp -ItemType Directory
        Set-Location target\cpp

        # MinGW is probably installed, let's assume it is
        Invoke-CMake -generator "MinGW Makefiles"

        # Compile the code
        mingw32-make -f Makefile nova-renderer

        # Copy output DLL to the correct location
        robocopy "." "..\..\jars\versions\1.10\1.10-natives\" "libnova-renderer.dll" 

        Write-Information "Copied Nova, but you're going to have to put libstc++.dll on your path somewhere, or else you won't be able to run Nova."

        Set-Location ..\..

    } else {
        Write-Error "Could not call the MinGW Make tool, unable to build Nova. Please install MinGW AND ensure that mingw32-make is in your path"
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

function Invoke-CMake([string]$generator) {
    <#
    .SYNOPSIS
        Invokes CMake using the provided generator
    .PARAMETER geneartor 
        The generator for CMake to use
    #>

    cmake -G $generator ../../src/main/cpp
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

    Set-Location src\main\cpp

    # Remove CMake files
    Remove-Item "CMakeFiles","CMakeCache.txt" -Recurse

    # Check for MinGW files
    if(Test-Path "Makefile") {
        # We're dealing with a MinGW build
        Remove-Item "Makefile","libnova-renderer.dll","libnova-renderer.dll.a"
    }

    if(Test-Path "nova-renderer.sln") {
        # TODO: Verify that this is the name of the Visual Studio solution, and remove all the files involved
        Remove-Item "nova-renderer.dll","nova-renderer.dll.lib"
    }

    Set-Location ..\..\..
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

    Write-Information "Launching Nova..."
    
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
