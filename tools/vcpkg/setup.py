#!/usr/bin/env python3

import os
import subprocess
import argparse

import distutils
from distutils import dir_util

import glob

from sys import platform, stdout

# force overrides
packages = [
    'foundational',
    'miniz',
    'fmt',
    'glm',
    'glslang',
    'gtest',
    'nlohmann-json',
    'spirv-cross',
    'spirv-tools',
    'vulkan-memory-allocator',
    'vulkan',
    'minitrace',
    'eastl'
]

if platform == "linux" or platform == "linux2":
    target = "x64-linux"
elif platform == "darwin":
    target = "x64-osx"
elif platform == "win32":
    target = "x64-windows"


def available_ports(source_dir, vcpkg_dir):
    portdir = os.path.join(source_dir, "external/vcpkg/ports")
    return [d for d in os.listdir(portdir) if os.path.isdir(os.path.join(portdir, d))]


def copy_tree(src, dest):
    print(f"Copying {src} to {dest}")
    stdout.flush()
    distutils.dir_util.copy_tree(src, dest)


def copy_port(name, source_dir, vcpkg_dir):
    copy_tree(os.path.join(source_dir, f"external/vcpkg/ports/{name}/"), os.path.join(vcpkg_dir, f"ports/{name}/"))


def copy_ports(source_dir, vcpkg_dir):
    for d in available_ports(source_dir, vcpkg_dir):
        print(f"Copying {d}")
        copy_port(d, source_dir, vcpkg_dir)


def install_package(name, vcpkg_dir):
    if subprocess.run(["vcpkg", "install", f"{name}:{target}", "--recurse"]).returncode != 0:
        for file in glob.glob(os.path.join(vcpkg_dir, f"buildtrees/{name}/*.log")):
            print(f"Contents of file {file}")
            with open(file) as f:
                stdout.write(f.read())
                stdout.flush()
        exit(1)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-dir", type=str, default=os.getcwd())
    parser.add_argument("--vcpkg-dir", type=str, required=True)
    parser.add_argument("--no-install", action='store_true', default=False)
    parsed = parser.parse_args()

    copy_ports(parsed.source_dir, parsed.vcpkg_dir)

    if(parsed.no_install == True):
        return

    subprocess.run(['vcpkg', 'update'])
    subprocess.run(['vcpkg', 'upgrade', '--no-dry-run'])

    for package in packages:
        install_package(package, parsed.vcpkg_dir)


if __name__ == '__main__':
    main()
