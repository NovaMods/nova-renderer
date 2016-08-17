<# 
.SYNOPSIS
    A simple script to handle the build lifecycle of the Nova Renderer

.DESCRIPTION
    Allows you to setup Nova on a new machine, compile Nova, and run Nova, all from a single script with a simple interface

.PARAMETER setup
    Downloads MCP, unzips it, moves it to the right place, and applies the source code transformations to Minecraft so that Nova will be run properly

.PARAMETER build
    Compiles the native code, moves the output DLL to the correct directory, then compiles the Java code

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
    [string]$buildEnvironment = "mingw"
)

if($setup -eq $true) {
    Initialize-Nova
}

if($build -eq $true) {
    New-NovaCode -buildEnvironment $buildEnvironment
}

if($clean -eq $true) {
    Clean-Nova
}

if($run -eq $true) {
    Invoke-Nova
}

function New-NovaCode([string]$buildEnvironment) {
    <#
    .SYNOPSIS
        Compiles all the Nova code you can possible imagine
    .PARAMETER buildEnvironment
        Specifies the build environment you with to use. Must be either 'mingw' or 'msvc'.
    #>

    Write-Host "Starting compilation of Nova..."

    if(Test-Environment -environmentName $buildEnvironment) {
        # We have the compiler we need! Let's compile some stuff
        Set-Location src\native
        Invoke-CMake -
    }
}

function Test-Environment([string]$environmentName) {
    <#
    .SYNOPSIS
        Checks if the toolchain for the given build environment is callable from within this script
    .PARAMETER environmentName
        The name of the compiler to check. If mingw is the provided value, checks that the MinGW toolchain exists and is set up at least marginally usefully. If the value is msvc, checks that the Visual Studio toolchain exists. Does not check the version of Visual Studio installed because that will be checked when we try to build Nova and I don't want to do it here
    #>

    if($environmentName -eq "mingw") {
        # We can simply test for mingw32-make. As long as we have that, everything else might work
        if((Get-Command "mingw32-make.exe" -ErrorAction SilentlyContinue) -eq $null) {
            Write-Error "Could not find the mingw32-make.exe executable. Please ensure that it's in your PATH, then run this script again" -Category ResourceUnavailable
            return $false
        } else {
            # I could return the result of the Get-Command test, but I want to output an error message upon failure
            return $true
        }

    } elseif($environmentName -eq "msvc") {
        # Check for the msvc build thingy
        if((Get-Command "devenv.exe" -ErrorAction SilentlyContinue) -eq $null) {
            Write-Error "Could not find the devenv.exe executable. Please ensure that it's in your PATH, then run this script again" -Category ResourceUnavailable
            return $false
        } else {
            # Again, I could just output the result of the Get-Command test, but I definitely want to output error messages whenever possible 
            return $true
        }
    } else {
        Write-Error "Compiler thing $environmentName not supported. Please specify either mingw or msvc" -Category InvalidArgument
        return $false
    }
}
