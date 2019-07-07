#!/usr/bin/env python3

#################
# Version Check #
#################

import sys

tool_name = "build.py"

# We need to do this before we rely on 3.7 only things
if sys.version_info < (3, 7):
    print("Running in python version %s.%s.%s. %s requires 3.7+" % (sys.version_info[0],
                                                                    sys.version_info[1],
                                                                    sys.version_info[2],
                                                                    tool_name))
    exit(1)

import argparse
import distutils.dir_util
import enum
import functools
import itertools
import os
import queue
import re
import shutil
import glob
import subprocess
import time
import typing
import urllib.request
import zipfile

#################
# Configuration #
#################

# These are things that aren't really meant to be customized, but can be if need be

# tool_name = "build.py"
#   Definition above

version = (0, 2, 0)
version_str = f"{version[0]}.{version[1]}.{version[2]}{'-' + version[3] if len(version) == 4 else ''}"
description = f"{tool_name} version {version_str}: CMake + vcpkg one click build script."

vcpkg_default_url = "https://github.com/Microsoft/vcpkg"
vcpkg_zip_path = "archive/master.zip"
vcpkg_zip_subdir = "vcpkg-master/"

default_config_file = ".build-py"
default_custom_port_dir = "external/vcpkg/ports/"

# Evil global state
debug = False


######################
# Platform Detection #
######################


class Platform(enum.Enum):
    Windows = enum.auto()
    OSX = enum.auto()
    Linux = enum.auto()


if sys.platform == "linux" or sys.platform == "linux2":
    platform = Platform.Linux
    triplet = "x64-linux"
elif sys.platform == "darwin":
    platform = Platform.OSX
    triplet = "x64-osx"
elif sys.platform == "win32":
    platform = Platform.Windows
    triplet = "x64-windows"
else:
    print("Unknown platform", file=sys.stderr)
    exit(1)

# noinspection PyUnboundLocalVariable
if platform == Platform.Windows:
    vcpkg_exe_name = "vcpkg.exe"
else:
    vcpkg_exe_name = "vcpkg"


###########
# Utility #
###########


def log_section_title(title):
    print("\u001b[37;1m{}\u001b[0m".format(title))


def log_start(text):
    start = time.perf_counter()

    sys.stdout.write(f"\t{text}... ")
    sys.stdout.flush()

    def done(final_string, success, terminate=False):
        if final_string is None:
            if success:
                final_string = "Ok"
            else:
                final_string = "Failed"

        end = time.perf_counter()

        diff = end - start

        sys.stdout.write((f"\u001b[32;1m{final_string}\u001b[0m" if success
                          else f"\u001b[31;1m{final_string}\u001b[0m")
                         + f"  ({diff:.2f}s)\n")
        sys.stdout.flush()

        if not success and terminate:
            exit(1)

    return done


if platform == Platform.Windows:
    def decode_string(b: bytes):
        s = b.decode("utf8")
        return s.replace('\r\n', '\n')
else:
    def decode_string(b: bytes):
        return b.decode("utf8")


@functools.lru_cache(None)
def make_conditionally_absolute(base, path):
    if os.path.isabs(path):
        return path
    return os.path.normpath(os.path.join(base, path))


def unzip_file(zip_file, dest_dir, subdir):
    # The default extract methods won't let me extract a subdirectory properly, so I'm forced to read each one manually
    # It's a kludge and I hate it.
    zip_f = zipfile.ZipFile(zip_file)
    for i in zip_f.infolist():  # type: zipfile.ZipInfo
        filename = i.filename
        if not i.is_dir() and filename.startswith(subdir):
            relative = filename[len(subdir):]
            dest_path = os.path.join(dest_dir, relative)
            os.makedirs(os.path.dirname(dest_path), exist_ok=True)
            with zip_f.open(i) as zf:
                with open(dest_path, "wb") as rf:
                    rf.write(zf.read())


###############
# Arg Parsing #
###############


@functools.lru_cache(None)
def find_default_cmake():
    return shutil.which("cmake")


@functools.lru_cache(None)
def find_default_clang_format():
    possibles = [
        "clang-format",
        "clang-format-9",
        "clang-format-8",
        "clang-format-7",
        "clang-format-6.0",
    ]
    for p in possibles:
        path = shutil.which(p)
        if path:
            return path


def parse_args():
    # noinspection PyTypeChecker
    parser = argparse.ArgumentParser(prog=tool_name,
                                     description=description,
                                     formatter_class=functools.partial(argparse.HelpFormatter,
                                                                       max_help_position=40,
                                                                       width=120))
    parser.add_argument("-v", "--verbose", "--debug",
                        action="store_true",
                        help="Enable debug output.")
    directory_settings = parser.add_argument_group("Path Settings")
    directory_settings.add_argument("source_dir",
                                    metavar="SOURCE_DIR",
                                    nargs='?',
                                    help="Location of source directory",
                                    type=os.path.abspath,
                                    default=os.getcwd())
    directory_settings.add_argument("--build-dir",
                                    metavar="DIR",
                                    default="build",
                                    help="Folder to store the build tree in. Relative to source dir if not absolute. "
                                         "Default \"build\"")
    directory_settings.add_argument("--config-file",
                                    metavar="FILE",
                                    default=default_config_file,
                                    help=f"Path to config file. Relative to source dir if not absolute. Default "
                                    f"\"{default_config_file}\"")
    directory_settings.add_argument("--custom-port-dir",
                                    metavar="DIR",
                                    default=default_custom_port_dir,
                                    help=f"Folder with custom vcpkg ports to install. Relative to source dir if not "
                                    f"absolute. Default \"{default_custom_port_dir}\"")
    directory_settings.add_argument("--vcpkg-dir",
                                    metavar="DIR",
                                    default="build-vcpkg",
                                    help="Folder to install vcpkg in. Relative to source dir if not absolute. Default "
                                         "\"build-vcpkg\"")

    build_settings = parser.add_argument_group("Build Settings")
    build_settings.add_argument("--type",
                                metavar="TYPE",
                                choices=['debug', 'size', 'debug_release' 'release'],
                                help="Type of build. Choices are debug, size, debug_release, and release. Defaults to "
                                     " release",
                                default='release')
    prog_settings = build_settings.add_mutually_exclusive_group()
    prog_settings.add_argument("--vcpkg-only",
                               action="store_true",
                               help="Only install vcpkg. Do not install packages.")
    prog_settings.add_argument("--copy-deps-only",
                               action="store_true",
                               help="Only copy custom packages. Do not build or install packages.")
    prog_settings.add_argument("--deps-only",
                               action="store_true",
                               help="Only install vcpkg and packages. Do not build.")
    prog_settings.add_argument("--configure-only",
                               action="store_true",
                               help="Only configure cmake.")
    prog_settings.add_argument("--build-only",
                               action="store_true",
                               help="Only configure cmake and build.")
    build_settings.add_argument("--format",
                                action="store_true",
                                help="Automatically format code using clang-format.")

    extern_settings = parser.add_argument_group("External Tools")
    extern_settings.add_argument("--cmake-binary",
                                 metavar="PATH",
                                 help=f"Path to cmake. Default: {find_default_cmake()}",
                                 default=find_default_cmake())
    extern_settings.add_argument("--clang-format-binary",
                                 metavar="PATH",
                                 help=f"Path to clang-format. Default: {find_default_clang_format()}",
                                 default=find_default_clang_format())
    extern_settings.add_argument("--vcpkg-url",
                                 metavar="URL",
                                 help=f"URL of vcpkg's github. Default: {vcpkg_default_url}",
                                 default=vcpkg_default_url)
    return parser.parse_args()


###############
# Config File #
###############

class Settings:
    def __init__(self):
        self.packages = []
        self.project_name = None
        self.version = None


def parse_config_package_list(package_list):
    packages = []
    for match in re.finditer(r"[^\s,[]+(?:\[(?:[^,]+,\s*)*[^,\]]+])?",
                             package_list,
                             re.MULTILINE | re.UNICODE):
        packages.append(match.group(0))
    return packages


def parse_config_file(path):
    parse_func = log_start(f"Parsing config file {path}")

    if not (os.path.exists(path) and os.path.isfile(path)):
        parse_func("Not Found", False)
        return Settings()

    settings = Settings()

    with open(path, "r") as f:
        for i, line in enumerate(f, 1):
            if len(line) == 0:
                continue
            if line[0] == '#':
                continue

            split = line.split("=", maxsplit=1)
            if len(split) == 1:
                parse_func(None, False)
                print(f"No equals sign found in config file line: {i}", file=sys.stderr)
                exit(1)

            key, value = [v.strip() for v in split]  # type: str

            if key == "name" or key == "project":
                settings.project_name = value
            elif key == "packages":
                settings.packages.extend(parse_config_package_list(value))
            elif key == "version":
                settings.version = value
            else:
                parse_func(None, False)
                print(f"Unknown key on line {i}: {key}", file=sys.stderr)
                exit(1)

    parse_func(None, True)
    return settings


##################
# VCPKG Handling #
##################


def check_vcpkg_dir(vcpkg_dir):
    def file_exists(path):
        full_path = os.path.join(vcpkg_dir, path)
        return os.path.exists(full_path) and os.path.isfile(full_path)

    files = [".vcpkg-root", "bootstrap-vcpkg.sh", "bootstrap-vcpkg.bat"]

    return all(map(file_exists, files))


def download_vcpkg(vcpkg_dir, vcpkg_url):
    full_url = vcpkg_url + '/' + vcpkg_zip_path

    download_func = log_start(f"Downloading vcpkg from {full_url}")
    filename, headers = urllib.request.urlretrieve(full_url)
    download_func("Finished", True)

    extract_func = log_start(f"Extracting into {vcpkg_dir}")
    unzip_file(filename, vcpkg_dir, vcpkg_zip_subdir)
    check = check_vcpkg_dir(vcpkg_dir)

    if not check:
        extract_func(None, False)
        print("Error; vcpkg failed to download for unknown reasons", file=sys.stderr)
        exit(1)

    extract_func("Finished", True)


def bootstrap_vcpkg(vcpkg_dir):
    bootstrap_func = log_start("Bootstrapping vcpkg (this may take some time)")

    vcpkg_exe_path = os.path.join(vcpkg_dir, vcpkg_exe_name)

    if os.path.exists(vcpkg_exe_path) and os.path.isfile(vcpkg_exe_path):
        bootstrap_func("Cached", True)
        return True

    if platform == Platform.Windows:
        result = subprocess.run([os.path.join(vcpkg_dir, "bootstrap-vcpkg.bat")],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                shell=True,
                                cwd=vcpkg_dir)
    else:
        result = subprocess.run(["bash", os.path.join(vcpkg_dir, "bootstrap-vcpkg.sh")],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                cwd=vcpkg_dir)

    bootstrap_func(None, result.returncode == 0)

    if result.returncode != 0:
        print("=========== Bootstrapping stdout ===========")
        sys.stdout.write(decode_string(result.stdout))
        print("=========== Bootstrapping stderr ===========")
        sys.stdout.write(decode_string(result.stderr))
        exit(1)

    return result.returncode == 0


def prepare_vcpkg(vcpkg_dir, vcpkg_url):
    vcpkg_dir_func = log_start(f"Checking for vcpkg in {vcpkg_dir}")

    have_vcpkg = check_vcpkg_dir(vcpkg_dir)

    vcpkg_dir_func(None, have_vcpkg)

    if not have_vcpkg:
        download_vcpkg(vcpkg_dir, vcpkg_url)

    bootstrap_vcpkg(vcpkg_dir)


###############################
# Custom Package Installation #
###############################


def enumerate_ports(ports_dir):
    enumerate_func = log_start("Enumerating ports")
    if os.path.exists(ports_dir) and os.path.isdir(ports_dir):
        ports = [d for d in os.listdir(ports_dir) if os.path.isdir(os.path.join(ports_dir, d))]
        enumerate_func(f"Found {len(ports)} port{'s' if len(ports) != 1 else ''}", True)
        return ports

    enumerate_func(f"Folder not found", False)
    return []


def copy_port(port_dir, name, vcpkg_dir):
    copy_func = log_start(f"Copying {name} port")
    try:
        distutils.dir_util.copy_tree(os.path.join(port_dir, name),
                                     os.path.join(vcpkg_dir, f"ports/{name}/"),
                                     update=True)
    except Exception:
        copy_func(None, False)
        raise
    copy_func(None, True)


def copy_all_ports(port_dir, vcpkg_dir):
    ports = enumerate_ports(port_dir)

    for p in ports:
        copy_port(port_dir, p, vcpkg_dir)


########################
# Package Installation #
########################


@functools.lru_cache(None)
def vcpkg_get_exe(vcpkg_dir):
    return os.path.join(vcpkg_dir, vcpkg_exe_name)


def vcpkg_invoke(vcpkg_dir, log_func, arguments, abort=True):
    exe_path = vcpkg_get_exe(vcpkg_dir)

    arg_list = [exe_path, "--vcpkg-root", vcpkg_dir, "--triplet", triplet, *arguments]
    result = subprocess.run(arg_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    if result.returncode != 0:
        log_func(None, False)
        print("=========== vcpkg invocation ===========")
        print(arg_list)
        print("============= vcpkg stdout =============")
        sys.stdout.write(decode_string(result.stdout))
        print("============= vcpkg stderr =============")
        sys.stdout.write(decode_string(result.stderr))
        if abort:
            exit(1)

    return result


@functools.lru_cache(None)
def vcpkg_normalize_name(package):
    # Check for feature list, add if missing
    if re.match(r"^[^[]+$", package):
        return package + "[core]"
    return package


@functools.lru_cache(None)
def vcpkg_denormalize_name(package):
    # Check for feature list, add if missing
    match = re.match(r"^([^[]+).*$", package)
    if match is None:
        return package
    return match.group(1)


@functools.lru_cache(None)
def vcpkg_extract_features(package):
    package = vcpkg_normalize_name(package)

    match = re.search(r"([^\s,[]+)(?:\[((?:[^,]+,\s*)*[^,\]]*)])?", package, re.MULTILINE | re.UNICODE)

    if not match:
        print(f"Unable to parse package name {package}. This is a bug.", file=sys.stderr)
        exit(1)

    name = match.group(1)
    feature_optional_str = match.group(2)
    feature_str = feature_optional_str if feature_optional_str is not None else "core"
    features = [f.strip() for f in feature_str.split(",")]

    return name, frozenset(features)


def vcpkg_reconstruct_features(package, features):
    return vcpkg_reconstruct_features_impl(package, frozenset(features))


@functools.lru_cache(None)
def vcpkg_reconstruct_features_impl(package, features):
    return f"{package}[{','.join(features)}]"


@functools.lru_cache(None)
def vcpkg_construct_single_feature_packages(package):
    name, features = vcpkg_extract_features(package)

    if len(features) == 0:
        print(f"Package {package} returned no features. This is a bug.", file=sys.stderr)
        exit(1)

    return frozenset(f"{name}[{f}]" for f in features)


def vcpkg_parse_control_file(vcpkg_dir, package):
    # Only deals with valid CONTROL files. Invalid CONTROL files may cause crazy behavior
    package_name = vcpkg_denormalize_name(package)

    file_path = os.path.join(vcpkg_dir, f"ports/{package_name}/CONTROL")

    if not os.path.exists(file_path) or not os.path.isfile(file_path):
        print(f"Expected to find {package_name}'s control file at {file_path}", file=sys.stderr)
        exit(1)

    kvp = []
    with open(file_path, 'r') as f:
        for line in f:
            split = [s.strip() for s in line.split(":", 1)]
            split = [s for s in split if len(s) != 0]
            if len(split) != 2:
                continue
            kvp.append(split)

    package_name = ""
    current_feature = "core"

    dependency_pairs = {"core": []}

    for key, value in kvp:
        if key == "Source":
            package_name = value
        if key == "Feature":
            current_feature = value
            dependency_pairs[value] = []
        if key == "Build-Depends":
            depends = parse_config_package_list(value)
            split_deps = list(itertools.chain.from_iterable([vcpkg_construct_single_feature_packages(d) for d in depends]))
            dependency_pairs[current_feature].extend(split_deps)
        if key == "Default-Features":
            dependency_pairs[current_feature].extend([f"{package_name}[{p}]" for p in parse_config_package_list(value)])

    return {f"{package_name}[{p}]": v for p, v in dependency_pairs.items()}


def vcpkg_list_upgradeable_packages(vcpkg_dir):
    upgrade_func = log_start(f"Enumerating upgradable packages")

    result = vcpkg_invoke(vcpkg_dir, upgrade_func, ["update"])

    output_str = decode_string(result.stdout)  # type: str

    nothing_match = re.search(r"^No packages need updating\.$",
                              output_str,
                              re.UNICODE | re.MULTILINE)

    if nothing_match is not None:
        upgrade_func("0 packages", True)
        return []

    something_match = re.search(r"^The following packages differ from their port versions:$",
                                output_str,
                                re.UNICODE | re.MULTILINE)

    packages = []
    if something_match is not None:
        for package in re.finditer(r"^ {4}([^:]+):x64-(?:windows|osx|linux).+$",
                                   output_str,
                                   re.UNICODE | re.MULTILINE):
            packages.append(vcpkg_normalize_name(package.group(1)))
    else:
        upgrade_func(f"Unknown error", False)
        print("============= vcpkg stdout =============")
        sys.stdout.write(decode_string(result.stdout))
        print("============= vcpkg stderr =============")
        sys.stdout.write(decode_string(result.stderr))
        exit(1)

    upgrade_func(f"Found {len(packages)} upgradable package{'s' if len(packages) != 1 else ''}", True)

    return packages


@functools.lru_cache(None)
def vcpkg_deduplicate_features(dep_list):
    # This is only for simple deduplication. It can not be used on a install order list. Install order lists do not have
    # enough information for this to be safe. You must deduplicate at the graph level.

    dep_dict = {}
    for dep in dep_list:
        name, features = vcpkg_extract_features(dep)

        # feature lists are sets so they are unique
        if name not in dep_dict:
            dep_dict[name] = set(features)
        else:
            dep_dict[name].update(features)

    new_dep_list = set()
    for name, features in dep_dict.items():
        new_dep_list.add(vcpkg_reconstruct_features(name, features))

    if debug:
        print(new_dep_list)

    return frozenset(new_dep_list)


class VcpkgEnumerateFeatureMismatch(Exception):
    package: str

    def __init__(self, package):
        self.package = package


def vcpkg_package_dependencies(vcpkg_dir, package):
    list_func = log_start(f"Enumerating dependencies of {package}")
    result = vcpkg_invoke(vcpkg_dir, list_func, ["install", "--dry-run", "--recurse", package])

    output_str = decode_string(result.stdout)  # type: str

    already_installed_match = re.search("^The following packages are already installed:$",
                                        output_str,
                                        re.UNICODE | re.MULTILINE)

    if already_installed_match is not None:
        list_func(f"Installed", True)
        return None

    feature_mismatch_match = re.search("^The following packages will be rebuilt:$",
                                       output_str,
                                       re.UNICODE | re.MULTILINE)

    if feature_mismatch_match is not None:
        list_func(f"Feature Mismatch", False)
        # Raise an exception because there is no easy way to propagate this information to where it needs to be
        # (try/catch in vcpkg_install_packages)
        raise VcpkgEnumerateFeatureMismatch(package)

    to_install_match = re.search("^The following packages will be built and installed:$",
                                 output_str,
                                 re.UNICODE | re.MULTILINE)

    deps = []
    if to_install_match is not None:
        for dependency in re.finditer(r"^ {2}\* ([^:]+):x64-(?:windows|osx|linux)$", output_str,
                                      re.UNICODE | re.MULTILINE):
            deps.append(vcpkg_normalize_name(dependency.group(1)))
    else:
        list_func(f"Unknown error", False)
        print("============= vcpkg stdout =============")
        sys.stdout.write(decode_string(result.stdout))
        print("============= vcpkg stderr =============")
        sys.stdout.write(decode_string(result.stderr))
        exit(1)

    list_func(f"Found {len(deps)} dependenc{'ies' if len(deps) != 1 else 'y'}", True)

    return deps


def vcpkg_package_remove_dependencies(vcpkg_dir, package):
    list_func = log_start(f"Enumerating removal dependencies of {package}")
    result = vcpkg_invoke(vcpkg_dir, list_func, ["remove", "--recurse", "--dry-run", package])

    output_str = decode_string(result.stdout)  # type: str

    already_installed_match = re.search("^The following packages are not installed, so not removed:$",
                                        output_str,
                                        re.UNICODE | re.MULTILINE)

    if already_installed_match is not None:
        list_func(f"Not Installed", True)
        return None

    to_install_match = re.search("^The following packages will be removed:$",
                                 output_str,
                                 re.UNICODE | re.MULTILINE)

    deps = []
    if to_install_match is not None:
        for dependant in re.finditer(r"^ {2}\* ([^:]+):x64-(?:windows|osx|linux)$", output_str,
                                     re.UNICODE | re.MULTILINE):
            deps.append(vcpkg_normalize_name(dependant.group(1)))
    else:
        list_func(f"Unknown error", False)
        print("============= vcpkg stdout =============")
        sys.stdout.write(decode_string(result.stdout))
        print("============= vcpkg stderr =============")
        sys.stdout.write(decode_string(result.stderr))
        exit(1)

    list_func(f"Found {len(deps)} removal{'s' if len(deps) != 1 else ''}", True)

    return deps


class Package:
    name: str
    listed: bool
    queued: bool
    stated: bool
    features: set
    dependencies: typing.Set["Package"]

    def __init__(self, name: str, features: set, dependencies: set):
        self.name = name
        self.listed = False
        self.queued = False
        self.stated = False
        self.features = features
        self.dependencies = dependencies

    def __str__(self):
        return vcpkg_reconstruct_features(self.name, self.features)

    # This class is not immutable. This will change over time. This is only to allow it to be stored in a set of
    # dependencies. Having the same object twice the dependency set is okay. The second time will be ignored as it has
    # already been .listed
    def __hash__(self):
        return self.name.__hash__()


def vcpkg_dependency_resolver(vcpkg_dir, dep_func, packages):
    # This function is a mess of state and ugliness as I am trying to build it in an efficient way, only stating each
    # package the minimum amount of times needed. I will attempt to explain how it works.
    #
    # The purpose of the function is to build an order to install the packages provided that satisfies all
    # dependencies in the right order. This means I must find all dependencies of each package, then those
    # dependencies. This was simple enough when you don't take into account package features. When you account for
    # that, you end up in a situation where a needed feature can only be revealed to you in a dependency. If you've
    # already stated the package, you need to go back and re-stat it with the full set of features.
    #
    # Dependency nodes are put in a global dictionary, as well as children of their dependants. This allows the graph
    # to be walked, but also allow random access to a package of a certain name. As packages are revealed their package
    # objects are created and put into the queue. The .queued member tells you if the package has already been queued
    # but not stated yet. The .stated member tells you if the package has been stated in its current state.
    #
    # The features evaluated during the stat call always come from the object in the graph. This is to make sure we
    # are stating with all the known features. If we take from the name in the queue we get a problem where the queue
    # looks like foo[A], foo[B]. When we evaluate foo[A] we set .stated to true. We then get to foo[B] we see stated
    # is true and don't run the stat call. Normally when a new feature is set, we reset .stated when we add to the
    # queue. However, this has already been added to the queue so we can't reset it. The solution to this is to make
    # sure all stat calls evaluate with all known features by keeping the feature list full with all known features
    # at all times. When foo[A] is added the object has the features [A]. When foo[B] is added, the object has the
    # features [A,B]. So When we go to evaluate foo[A} we actually evaluate foo[A, B], preventing the problem.

    packages_to_stat = queue.Queue()
    packages_stated = {}

    # Add all packages we need to the stat queue.
    for p in packages:
        package_name, package_features = vcpkg_extract_features(p)
        packages_stated[package_name] = Package(package_name, set(package_features), set())
        packages_stated[package_name].queued = True
        packages_to_stat.put_nowait(vcpkg_normalize_name(p))

    while True:
        # Try to extract something from a queue, throwing on failure
        try:
            package_normalized = packages_to_stat.get_nowait()
        except queue.Empty:
            break

        # Break full package name into the name and features
        package_name, package_features = vcpkg_extract_features(package_normalized)

        # We've already stated this package before, no need to do this again
        if packages_stated[package_name].stated:
            continue

        # Use the full feature set known about to stat the package
        deps = dep_func(vcpkg_dir, str(packages_stated[package_name]))
        packages_stated[package_name].stated = True

        # None means the package is already installed
        if deps is None:
            continue

        for dep_fullname in deps:
            dep_name, dep_features = vcpkg_extract_features(dep_fullname)
            dep_features = set(dep_features)

            # Create package object for the dependency if it doesn't already exist
            if dep_name not in packages_stated:
                packages_stated[dep_name] = Package(dep_name, dep_features, set())

            # We need more features then currently know about.
            if len(dep_features - packages_stated[dep_name].features) != 0:
                # Rest queued and stated to allow the stat to be reevaluated
                packages_stated[dep_name].queued = False
                packages_stated[dep_name].stated = False

                # Keep the package object feature list full. This way if there is a instance of this
                # package in the queue already, it will deal with these features
                packages_stated[dep_name].features.update(dep_features)

            # Queue if not already queued.
            if not packages_stated[dep_name].queued:
                packages_stated[dep_name].queued = True
                packages_to_stat.put_nowait(dep_fullname)

            # Add the dependency objects to the current package's object dependency list
            packages_stated[package_name].dependencies.add(packages_stated[dep_name])

    sort_func = log_start("Resolving dependency tree")

    # get topological sort of tasks
    def recurse(pkg_name):
        pkg_obj = packages_stated[pkg_name]
        if pkg_obj.listed:
            return []
        pkg_obj.listed = True
        return list(itertools.chain.from_iterable([recurse(dep.name) for dep in pkg_obj.dependencies] +
                                                  [[str(pkg_obj)]]))

    ordering = list(itertools.chain.from_iterable([recurse(name) for name in packages_stated.keys()]))

    sort_func(f"{len(ordering)} package{'s' if len(ordering) != 1 else ''}", True)

    if debug:
        print(ordering)

    return ordering


def vcpkg_remove_package(vcpkg_dir, package):
    remove_func = log_start(f"Removing package {package}")

    result = vcpkg_invoke(vcpkg_dir, remove_func, ["remove", package])

    output_str = decode_string(result.stdout)

    uninstalled_match = re.search(r"^The following packages are not installed, so not removed:$",
                                  output_str,
                                  re.UNICODE | re.MULTILINE)

    if uninstalled_match is not None:
        remove_func("Not Installed", True)

    will_remove = re.search(r"^The following packages will be removed:$",
                            output_str,
                            re.UNICODE | re.MULTILINE)

    if will_remove is not None:
        remove_func(f"Removed", True)
    else:
        remove_func(f"Unknown error", False)
        print("============= vcpkg stdout =============")
        sys.stdout.write(decode_string(result.stdout))
        print("============= vcpkg stderr =============")
        sys.stdout.write(decode_string(result.stderr))
        exit(1)


def vcpkg_install_package(vcpkg_dir, package):
    install_func = log_start(f"Installing package {package}")
    # We can't abort here as we need to print out log files
    result = vcpkg_invoke(vcpkg_dir, install_func, ["install", package], abort=False)

    output_str = decode_string(result.stdout)  # type: str

    already_installed_match = re.search("^The following packages are already installed:$",
                                        output_str,
                                        re.UNICODE | re.MULTILINE)

    if already_installed_match is not None:
        install_func(f"Previously Installed", True)
        return None

    to_install_match = re.search("^The following packages will be built and installed:$",
                                 output_str,
                                 re.UNICODE | re.MULTILINE)

    deps = []
    if to_install_match is not None and result.returncode == 0:
        install_func("Installed", True)
    else:
        denormalized = vcpkg_denormalize_name(package)
        rel_path = f"buildtrees/{denormalized}/*.log"
        glob_str = os.path.join(vcpkg_dir, rel_path)
        for log_path in glob.iglob(glob_str):
            with open(log_path, 'r') as f:
                print(f"\n============= {log_path} =============")
                sys.stdout.write(f.read())

        exit(1)

    return deps


def vcpkg_list_packages(vcpkg_dir):
    list_func = log_start("Enumerating installed packages")

    result = vcpkg_invoke(vcpkg_dir, list_func, ["list"])

    output_str = decode_string(result.stdout)  # type: str

    packages = set()
    for m in re.finditer(r"(^[^:]+).*$", output_str, re.UNICODE | re.MULTILINE):
        packages.add(vcpkg_normalize_name(m.group(1)))

    list_func(f"Found {len(packages)} installed package{'s' if len(packages) != 1 else ''}", True)

    return packages


def vcpkg_list_simple_deps(vcpkg_dir, parent_packages):
    list_simple_func = log_start("Enumerating all needed packages")

    packages = set()
    known_dependencies = {}

    packages_to_stat = queue.Queue()
    for p in parent_packages:
        single_features = vcpkg_construct_single_feature_packages(p)

        for s in single_features:
            packages.add(s)
            packages_to_stat.put_nowait(s)

    while True:
        try:
            single_feature = packages_to_stat.get_nowait()
        except queue.Empty:
            break
        name = vcpkg_denormalize_name(single_feature)

        if single_feature not in known_dependencies:
            known_dependencies.update(vcpkg_parse_control_file(vcpkg_dir, name))

        if single_feature not in known_dependencies:
            print(f"Unknown feature {single_feature}.")
            exit(1)

        for d in known_dependencies[single_feature]:
            if d not in packages:
                packages.add(d)
                packages_to_stat.put_nowait(d)

    list_simple_func(f"Found {len(packages)} needed package{'s' if len(packages) != 1 else ''}", True)

    return packages


def vcpkg_remove_unneeded(vcpkg_dir, installed, needed):
    remove_func = log_start("Determining unneeded packages")

    unneeded = set(installed) - set(needed)

    remove_func(f"Found {len(unneeded)} unneeded package{'s' if len(unneeded) != 1 else ''}", True)

    log_section_title("Removing unneeded packages")

    packages_to_remove = vcpkg_dependency_resolver(vcpkg_dir, vcpkg_package_remove_dependencies, unneeded)

    for p in packages_to_remove:
        vcpkg_remove_package(vcpkg_dir, p)


def vcpkg_install_packages(vcpkg_dir, packages):
    packages = list(map(vcpkg_normalize_name, packages))

    log_section_title("Upgrading Existing Packages")
    upgradable_packages = vcpkg_list_upgradeable_packages(vcpkg_dir)
    if len(upgradable_packages) != 0:
        packages_to_remove = vcpkg_dependency_resolver(vcpkg_dir, vcpkg_package_remove_dependencies,
                                                       upgradable_packages)
        for p in packages_to_remove:
            vcpkg_remove_package(vcpkg_dir, p)
    # Installing may fail due to feature mismatches, this solves that problem in a very hacky way.
    while True:
        try:
            log_section_title("Installing Packages")
            packages_to_install = vcpkg_dependency_resolver(vcpkg_dir, vcpkg_package_dependencies, packages)
        except VcpkgEnumerateFeatureMismatch as e:
            log_section_title("Remove Packages with Feature Mismatch")
            packages_to_remove = vcpkg_dependency_resolver(vcpkg_dir, vcpkg_package_remove_dependencies, [e.package])
            for p in packages_to_remove:
                vcpkg_remove_package(vcpkg_dir, p)
            continue
        break

    # noinspection PyUnboundLocalVariable
    for p in packages_to_install:
        vcpkg_install_package(vcpkg_dir, p)

    log_section_title("Determining unneeded packages")

    installed_packages = vcpkg_list_packages(vcpkg_dir)
    needed_packages = vcpkg_list_simple_deps(vcpkg_dir, packages)

    vcpkg_remove_unneeded(vcpkg_dir, installed_packages, needed_packages)


########
# Main #
########


def main():
    arguments = parse_args()

    if arguments.vcpkg_only:
        task_vcpkg = True
        task_copy_packages = False
        task_packages = False
        task_configure = False
        task_build = False
    elif arguments.copy_deps_only:
        task_vcpkg = False
        task_copy_packages = True
        task_packages = False
        task_configure = False
        task_build = False
    elif arguments.deps_only:
        task_vcpkg = True
        task_copy_packages = True
        task_packages = True
        task_configure = False
        task_build = False
    elif arguments.configure_only:
        task_vcpkg = False
        task_copy_packages = False
        task_packages = False
        task_configure = True
        task_build = False
    elif arguments.build_only:
        task_vcpkg = False
        task_copy_packages = False
        task_packages = False
        task_configure = True
        task_build = True
    else:
        task_vcpkg = True
        task_copy_packages = True
        task_packages = True
        task_configure = True
        task_build = True

    task_format = False

    if arguments.format:
        task_format = True

    if arguments.verbose:
        global debug
        debug = True

    config_file_path = make_conditionally_absolute(arguments.source_dir, arguments.config_file)
    custom_port_dir = make_conditionally_absolute(arguments.source_dir, arguments.custom_port_dir)
    vcpkg_dir = make_conditionally_absolute(arguments.source_dir, arguments.vcpkg_dir)

    config_settings = parse_config_file(config_file_path)

    if config_settings.project_name:
        log_section_title(f"Building {'version ' + config_settings.version if config_settings.version else ''} of "
                          f"{config_settings.project_name}")

    if task_vcpkg:
        log_section_title("Installing vcpkg")
        prepare_vcpkg(vcpkg_dir, arguments.vcpkg_url)
    if task_copy_packages:
        log_section_title("Copying Custom Ports")
        copy_all_ports(custom_port_dir, vcpkg_dir)
    if task_packages:
        vcpkg_install_packages(vcpkg_dir, config_settings.packages)


if __name__ == "__main__":
    main()
