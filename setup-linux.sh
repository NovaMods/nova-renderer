#!/bin/bash

vcpkg update
vcpkg upgrade --no-dry-run
vcpkg install --recurse miniz:x64-linux fmt:x64-linux glm:x64-linux glslang:x64-linux gtest:x64-linux nlohmann-json:x64-linux spirv-cross:x64-linux spirv-tools:x64-linux vulkan-memory-allocator:x64-linux vulkan:x64-linux spirv-tools:x64-linux minitrace:x64-linux
