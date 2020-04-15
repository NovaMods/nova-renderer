#!/bin/python3

# Compiles shaders from HLSL to SPIR-V

# First argument is the directory of the shaders to compile, second argument is the directory to output shaders to

import os
import sys
import subprocess

from pathlib import Path

if __name__ == '__main__':
    if len(sys.argv) is not 3:
        print('Usage: compile_shaders.py <HLSL directory> <SPIR-V directory>')
        exit()

    hlsl_directory = str(sys.argv[1])
    spirv_directory = str(sys.argv[3])

    hlsl_paths = Path(hlsl_directory).glob('**/*.hlsl')

    for path in hlsl_paths:
        # Determine which shader stage we're handling. this will have to get updated as Nova supports more shaders
        suffixes = path.suffixes
        if 'vertex' in suffixes:
            target = 'vs_6_0'
        elif 'pixel' in suffixes:
            target = 'ps_6_0'
        else:
            print('Unknown shader type for shader %s, skipping', str(path))
            continue

        spirv_filename = path.stem + '.spirv'
        spirv_path = spirv_directory + '/' + spirv_filename

        output = subprocess.run(['dxc', '-spirv', '-fspv-target-env=vulkan1.1', '-fspv-reflect',
                                 '-E', 'main', '-Fo', spirv_path, '-T', target, str(path)], capture_output=True)
        if output.returncode is not 0:
            print('Could not compile shader %s: %s', output.stderr)
