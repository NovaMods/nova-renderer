<h1 align="center">
  <img width=64px src="https://raw.githubusercontent.com/NovaMods/nova-renderer/3d87b39c7d883e47c000533d372ed8eeb0c114fd/docs/images/nova-logo-128px-noborder.png">ova Renderer
</h1>

<p align="center">
  <a href="https://github.com/NovaMods/nova-renderer/actions">
    <img alt="Github Actions" src="https://github.com/NovaMods/nova-renderer/workflows/CI/badge.svg?branch=master">
  </a>

  <a href="https://discord.gg/VGqtadw">
    <img alt="Discord Chat" src="https://img.shields.io/discord/193228267313037312.svg?color=7289DA&label=discord">
  </a>
</p>

## Purpose

We set out to make a completely new renderer for Minecraft aimed at giving more control and vastly better tooling to shaderpack developers.

### QuikFAQ

- Nova is a replacement for Minecraft's renderer built for shaderpack support and more shaderpack features. It is not:
  - Something to make Minecraft run faster
  - For older computers
  - A rewrite of the tick system
  - A rewrite of the audio system
  - Anything to do with the MC server

## Development Status

Nova Renderer is a passion project by the developers and as such does not have any set deadlines or release dates.
We are still in early development of the multi-backend rewrite and things may change at any moment. That all being said, it is still in quite active development.

## Developer Setup

[Contributing](docs/contributing.md).

### Dependencies

One dependency need to be installed system wide. The [LunarG Vulkan SDK version v1.1.126.0](https://vulkan.lunarg.com/sdk/home). Instructions for installation are on the websites.
When installed in the default location, we can detect them and use them.

Nova is built the same way most cmake projects are built. However, git submodules need to be cloned:

```
git submodule update --init --recursive
```

From there, standard cmake build:

#### Windows

```
# Generate Visual Studio Project
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 16 2019" -Wno-dev
# Build from command line, or open into Visual Studio
cmake --build build
```

#### Unix

If you have the ninja build tool, it is the recommended way of building.

```
# Generate build.ninja
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -GNinja -Wno-dev
# Build from command line, or open into Visual Studio
ninja -C build
```

If you don't have ninja, use regular make.

```
# Generate makefile
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -Wno-dev
# Build from command line
make -C build -j$(nproc)
```

Please read the following to help get a feel for the project:

- [The Project Charter](docs/project_charter.md).
- [Git Rules](docs/git.md). **These must be followed for your PR to be accepted.**

Please contact us on Discord if you want to help! We're very friendly :smile:
