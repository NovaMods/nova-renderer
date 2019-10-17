git submodule update --init --recursive

Set-Location .\external\SPIRV-Cross

#######################################
# Checkout SPIRV-Cross's dependencies #
#######################################

$GlslangRev = '4b5159ea8170fa34e29f13448fddebf88e0a722a'
$SpirvToolsRev = 'bbb29870b510f83f99994358179c9ea6838c3100'
$SpirvHeadersRev = '601d738723ac381741311c6c98c36d6170be14a2'

# Ensure that we have glslang at the correct revision
if (Test-Path .\external\glslang) {
    Write-Host "Updating glslang to revision ${GlslangRev}"
    Set-Location .\external\glslang
    git fetch origin
    git checkout ${GlslangRev}

} else {
    Write-Host "Cloning glslang revision ${GlslangRev}"
    git clone git://github.com/KhronosGroup/glslang.git .\external\glslang
    Set-Location .\external\glslang
    git checkout ${GlslangRev}
}
Set-Location .\..\..

# Ensure that we have SPIRV-Tools at the correct revision
if (Test-Path .\external\spriv-tools) {
    Write-Host "Updating SPIRV-Tools to revision $SpirvToolsRev"
    Set-Location .\external\spirv-tools
    git fetch origin
    git checkout ${SpirvToolsRev}

} else {
    Write-Host "Cloning SPIRV-Tools revision $SpirvToolsRev"
    git clone git://github.com/KhronosGroup/spirv-tools.git .\external\spirv-tools
    Set-Location .\external\spirv-tools
    git checkout ${SpirvToolsRev}
}

# Ensure that SPIRV-Tools has SPIRV-Headers at the correct revision
if (Test-Path .\external\spirv-headers) {
    Write-Host "Updating SPIRV-Headers to revision ${SpirvHeadersRev}"
    Set-Location .\external\spirv-headers
    git fetch origin
    git checkout ${SpirvHeadersRev}

} else {
    Write-Host "Cloning SPIRV-Headers revision ${SpirvHeadersRev}"
    git clone git://github.com/KhronosGroup/SPIRV-Headers.git .\external\spirv-headers
    Set-Location .\external\spirv-headers
    git checkout ${SpirvHeadersRev}
}
Set-Location .\..\..\..\..\ # Holy directory length, BatMan!

####################################
# Build SPIRV-Cross's dependencies #
####################################

Write-Host "Building glslang"
New-Item .\external\glslang-build -ItemType Directory
cmake .\..\glslang -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=output
cmake --build . --config RELEASE --target install
Set-Location .\..\..

Write-Host "Building SPIRV_Tools"
New-Item .\external\spirv-tools-build -ItemType Directory
Set-Location .\external\spirv-tools-build
cmake .\..\spirv-tools -DCMAKE_BUILD_TYPE=RELEASE -DSPIRV_WERROR=OFF -DCMAKE_INSTALL_PREFIX=output
cmake --build . --config RELEASE --target install
Set-Location .\..\..

Set-Location .\..\..
