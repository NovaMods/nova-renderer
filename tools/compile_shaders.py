#!/bin/python3

# Compiles shaders from HLSL to SPIR-V

# First argument is the directory of the shaders to compile, second argument is the directory to output shaders to

import os
import sys
import subprocess

from pathlib import Path

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print('Usage: compile_shaders.py <shader compiler> <HLSL directory> <SPIR-V directory>')
        exit()

    shader_compiler = str(sys.argv[1])
    hlsl_directory = str(sys.argv[2])
    spirv_directory = str(sys.argv[3])

    if not os.path.exists(spirv_directory):
        os.makedirs(spirv_directory)

    hlsl_paths = Path(hlsl_directory).glob('**/*.hlsl')

    for path in hlsl_paths:
        spirv_filename = path.stem + '.spirv'
        spirv_path = spirv_directory + '/' + spirv_filename

        output = subprocess.run([shader_compiler, str(path), spirv_path], capture_output=True, text=True)
        print(output.stdout)
        if output.returncode != 0:
            print('Could not compile shader ', str(path), ': ', output.stderr)
        else:
            print('Successfully compiled ', str(path))
