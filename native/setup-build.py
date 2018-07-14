#!/usr/bin/env python3

import sys
import os
import os.path
import subprocess

if sys.version_info[0] < 3:
    raise Exception("You need to use python3 to run this script, you are using " + sys.version)

conanpath = os.path.abspath("buildutils/bin/conan")

if not os.path.isfile(conanpath):
    process = subprocess.run(["pip3", "install", "conan", "-t", os.path.abspath("buildutils")])
    if not process.returncode == 0:
        print("If you got a weird message that main cannot be found, remove all pip installs and use the file get-pip.py with python3 to install pip")
        raise Exception("Failed to setup conan using pip")
    print("Installed conan using pip3")
else:
    print("Using already installed conan in %s " % conanpath)

conanprocess = subprocess.run([conanpath, "install", "."], cwd=os.path.abspath("src"), env={"PYTHONPATH": os.path.abspath("buildutils")})