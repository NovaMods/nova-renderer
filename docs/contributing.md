# Contributing to Nova Renderer

We are always welcome to new contributers. Please read the following before contributing.

### Contact Us

If you want to contribute, we recommend that you drop us a line over on Discord so we can help onboard you properly. 
However, we welcome pull requests from anyone, talking to us beforehand is not required.

## Dependencies

One dependency needs to be installed system wide. The [LunarG Vulkan SDK version v1.1.126.0](https://vulkan.lunarg.com/sdk/home). Installation instructions can be found on the website.
When installed in the default location, we can detect it and use it.

## Compiling

Nova is built the same way most cmake projects are built. However, git submodules need to be cloned:

```
git submodule update --init --recursive
```

From there, standard cmake build:

### Windows

```
# Generate Visual Studio Project
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 16 2019" -Wno-dev
# Build from command line, or open into Visual Studio
cmake --build build
```

### Unix

If you have the ninja build tool, it is the recommended way of building.

```
# Generate Visual Studio Project
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -GNinja -Wno-dev
# Build from command line, or open into Visual Studio
ninja -C build
```

If you don't have ninja, use regular make.

```
# Generate Ninja
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -Wno-dev
# Build from command line
make -C build -j$(nproc)
```

#### Principles

- [The Project Charter](project_charter.md).
- [Project Pipeline](project_pipeline.md). Don't know what to do? Look here!

#### Rules

**These must be followed for your PR to be accepted.**

- [Git Rules/Cheatsheet](git.md).
