#!/usr/bin/env python3
#
# ===- run-clang-tidy.py - Parallel clang-tidy runner ---------*- python -*--===#
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# ===------------------------------------------------------------------------===#
# FIXME: Integrate with clang-tidy-diff.py

"""
Parallel clang-tidy runner
==========================

Runs clang-tidy over all files in a compilation database. Requires clang-tidy
and clang-apply-replacements in $PATH.

Example invocations.
- Run clang-tidy on all files in the current working directory with a default
    set of checks and show warnings in the cpp files and all project headers.
        run-clang-tidy.py $PWD

- Fix all header guards.
        run-clang-tidy.py -fix -checks=-*,llvm-header-guard

- Fix all header guards included from clang-tidy and header guards
    for clang-tidy headers.
        run-clang-tidy.py -fix -checks=-*,llvm-header-guard extra/clang-tidy \
                                            -header-filter=extra/clang-tidy

Compilation database setup:
http://clang.llvm.org/docs/HowToSetupToolingForLLVM.html
"""

from __future__ import print_function

import argparse
import collections
import functools
import glob
import itertools
import json
import multiprocessing
import os
import pprint
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import traceback
import yaml

is_py2 = sys.version[0] == '2'

if is_py2:
        import Queue as queue
else:
        import queue as queue


def find_compilation_database(path):
    """Adjusts the directory until a compilation database is found."""
    result = './'
    while not os.path.isfile(os.path.join(result, path)):
        if os.path.realpath(result) == '/':
            print('Error: could not find compilation database.')
            sys.exit(1)
        result += '../'
    return os.path.realpath(result)


def make_absolute(f, directory):
    if os.path.isabs(f):
        return f
    return os.path.normpath(os.path.join(directory, f))


def sanitize_name(tmpdir, f):
    return os.path.join(tmpdir, f.replace("/", "$|$").replace("\\", "$|$").replace(".", "$||$") + ".yaml")


def desanitize_name(f, source_tree):
    return relpath(os.path.splitext(os.path.basename(f))[0].replace("$||$", ".").replace("$|$", "/"), source_tree)


def get_tidy_invocation(f, clang_tidy_binary, checks, tmpdir, build_path,
                        header_filter, extra_arg, extra_arg_before, quiet,
                        config):
    """Gets a command line for clang-tidy."""
    start = [clang_tidy_binary]
    if header_filter is not None:
        start.append('-header-filter=' + header_filter)
    else:
        # Show warnings in all in-project headers by default.
        start.append('-header-filter=^' + build_path + '/.*')
    if checks:
        start.append('-checks=' + checks)
    if tmpdir is not None:
        start.append('-export-fixes')
        # Get a temporary file. We immediately close the handle so clang-tidy can
        # overwrite it.
        name = sanitize_name(tmpdir, f)
        start.append(name)
    for arg in extra_arg:
            start.append('-extra-arg=%s' % arg)
    for arg in extra_arg_before:
            start.append('-extra-arg-before=%s' % arg)
    start.append('-p=' + build_path)
    if quiet:
            start.append('-quiet')
    if config:
            start.append('-config=' + config)
    start.append(f)
    return start


def relpath(path, base):
    if base == '/':
        return os.path.abspath(path)
    else:
        return os.path.relpath(path, base)


@functools.lru_cache(maxsize=128)
def get_file(filename):
    try:
        with open(filename, 'r', newline='') as f:
            return f.readlines()
    except IsADirectoryError as e:
        return ['']


def get_file_line(filename, line):
    try:
        return get_file(filename)[line - 1]
    except IndexError as e:
        print(f"Error getting line {line} from {filename}. {len(get_file(filename))} lines long.")
        raise


def get_pointer_line(linetext, column):
    return "".join(c if c == '\t' else ' ' for i, c in enumerate(linetext) if i + 1 < column) + '^'


@functools.lru_cache(maxsize=None)
def get_file_offsets(filename):
    lengths = [len(l) for l in get_file(filename)]
    offsets = [0] + list(itertools.accumulate(lengths))
    return offsets


def get_line(offsets, offset):
    for i, o in enumerate(offsets):
        if o > offset:
            return (i, offset - offsets[i - 1] + 1)
    return len(offsets) - 1, 1


def canonicalize_paths(diagnostics):
    for diagnostic in diagnostics:
        if 'FilePath' in diagnostic:
            diagnostic['FilePath'] = os.path.abspath(diagnostic['FilePath'])
        if 'Replacements' in diagnostic:
            for r in diagnostic['Replacements']:
                if 'FilePath' in r:
                    r['FilePath'] = os.path.abspath(r['FilePath'])

        if 'Notes' in diagnostic:
            for n in diagnostic['Notes']:
                if 'FilePath' in n:
                    n['FilePath'] = os.path.abspath(n['FilePath'])


def deduplicate(diagnostics):
    return {(d['DiagnosticName'], d['FileOffset'], d['FilePath'], d['Message']): d for d in diagnostics}


mergekey = "Diagnostics"
def merge_replacement_files(tmpdir, source_tree, by_diagnostic):
    """Merge all replacement files in a directory into a single file"""
    # The fixes suggested by clang-tidy >= 4.0.0 are given under
    # the top level key 'Diagnostics' in the output yaml files
    merged_dict = {}
    for replacefile in sorted(glob.iglob(os.path.join(tmpdir, '*.yaml'))):
        print("Merging: " + desanitize_name(replacefile, source_tree))
        sys.stdout.flush()
        content = yaml.safe_load(open(replacefile, 'r'))
        if not content:
            continue  # Skip empty files.
        diagnostics = content.get(mergekey, [])
        canonicalize_paths(diagnostics)
        merged_dict.update(deduplicate(diagnostics))

    if by_diagnostic:
        def key(d):
            return (d['DiagnosticName'], '.h' not in os.path.basename(d['FilePath']), d['FilePath'], d['FileOffset'])
    else:
        def key(d):
            return ('.h' not in os.path.basename(d['FilePath']), d['FilePath'], d['FileOffset'], d['DiagnosticName'])


    return sorted(merged_dict.values(), key=key)


def get_colors(enabled):
    if enabled:
        return {'purple': '\033[1;35m', 'white': '\033[1;37m', 'green': '\033[32;1m', 'grey': '\033[30;1m', 'reset': '\033[0m'}
    else:
        return collections.defaultdict(lambda: '')


def print_notes(note_list, source_tree, colors):
    for note in note_list:
        path = note['FilePath']
        shortpath = relpath(path, source_tree)
        offsets = get_file_offsets(path)
        offset = note['FileOffset']
        line, col = get_line(offsets, offset)
        message = note['Message']
        linefromfile = get_file_line(path, line).rstrip('\n')
        pointerline = get_pointer_line(linefromfile, col)
        print(f"{colors['white']}{shortpath}:{line}:{col}: {colors['grey']}note: {colors['white']}{message}")
        print(colors['reset'] + linefromfile)
        print(colors['green'] + pointerline + colors['reset'])

    return len(note_list)


def print_warnings(diagnostics, source_tree, colored):
    colors = get_colors(colored)
    note_count = 0
    for diag in diagnostics:
        path = diag['FilePath']
        shortpath = relpath(path, source_tree)
        offsets = get_file_offsets(path)
        offset = diag['FileOffset']
        line, col = get_line(offsets, offset)
        message = diag['Message']
        has_fixes = 'fix: ' if 'Replacements' in diag and len(diag['Replacements']) != 0 else ''
        diagname = f"[{has_fixes}{diag['DiagnosticName']}]"
        linefromfile = get_file_line(path, line).rstrip('\n')
        pointerline = get_pointer_line(linefromfile, col)
        print(f"{colors['white']}{shortpath}:{line}:{col}: {colors['purple']}warning: {colors['white']}{message} {diagname}")
        print(colors['reset'] + linefromfile)
        print(colors['green'] + pointerline + colors['reset'])

        if 'Notes' in diag:
            note_count += print_notes(diag['Notes'], source_tree, colors)

    count = len(diagnostics)
    print(f"{count} warnings. {note_count} notes.")


def write_replacements(replacements, file):
    if replacements:
        # MainSourceFile: The key is required by the definition inside
        # include/clang/Tooling/ReplacementsYaml.h, but the value
        # is actually never used inside clang-apply-replacements,
        # so we set it to '' here.
        output = {'MainSourceFile': '', mergekey: replacements}
        with open(file, 'w') as out:
            yaml.safe_dump(output, out)
    else:
        # Empty the file:
        open(file, 'w').close()


def check_clang_apply_replacements_binary(args):
    """Checks if invoking supplied clang-apply-replacements binary works."""
    try:
        subprocess.check_call([args.clang_apply_replacements_binary, '--version'])
    except subprocess.CalledProcessError:
        print('Unable to run clang-apply-replacements. Is clang-apply-replacements '
              'binary correctly specified?', file=sys.stderr)
        traceback.print_exc()
        sys.exit(1)


def apply_fixes(args, tmpdir):
    """Calls clang-apply-fixes on a given directory."""
    invocation = [args.clang_apply_replacements_binary]
    if args.format:
        invocation.append('-format')
    if args.style:
        invocation.append('-style=' + args.style)
    invocation.append(tmpdir)
    subprocess.call(invocation)


def run_tidy(args, tmpdir, build_path, queue, lock, failed_files):
    """Takes filenames out of queue and runs clang-tidy on them."""
    while True:
        name = queue.get()
        shortname = relpath(name, args.source_tree)
        invocation = get_tidy_invocation(name, args.clang_tidy_binary, args.checks,
                                         tmpdir, build_path, args.header_filter,
                                         args.extra_arg, args.extra_arg_before,
                                         args.quiet, args.config)

        with lock:
            sys.stdout.write(args.clang_tidy_binary + ' ' + shortname + '\n')
            sys.stdout.flush()
        proc = subprocess.Popen(invocation, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output, err = proc.communicate()
        if proc.returncode != 0:
            failed_files.append(name)
            with lock:
                sys.stdout.write(' '.join(invocation) + '\n' + output.decode('utf-8') + '\n')
                if len(err) > 0:
                    sys.stderr.write(err.decode('utf-8') + '\n')
        queue.task_done()


def str2bool(v):
    v = v.lower()
    if v in ("yes", "true", "on", "t", "1"):
        return True
    elif v in ("no", "false", "off", "f", "0"):
        return False
    else:
        raise argparse.ArgumentTypeError(f"{v} is not a boolean")


def main():
    parser = argparse.ArgumentParser(description='Runs clang-tidy over all files '
                                     'in a compilation database. Requires '
                                     'clang-tidy and clang-apply-replacements in '
                                     '$PATH.')
    parser.add_argument('--clang-tidy-binary', metavar='PATH',
                        default='clang-tidy',
                        help='path to clang-tidy binary')
    parser.add_argument('--clang-apply-replacements-binary', metavar='PATH',
                        default='clang-apply-replacements',
                        help='path to clang-apply-replacements binary')
    parser.add_argument('--checks', default=None,
                        help='checks filter, when not specified, use clang-tidy '
                        'default')
    parser.add_argument('--color', metavar='BOOLEAN', type=str2bool, 
                        default=sys.stdout.isatty(), help='Output warnings in color')
    parser.add_argument('--config', default=None,
                        help='Specifies a configuration in YAML/JSON format: '
                        '  -config="{Checks: \'*\', '
                        '                       CheckOptions: [{key: x, '
                        '                                       value: y}]}" '
                        'When the value is empty, clang-tidy will '
                        'attempt to find a file named .clang-tidy for '
                        'each source file in its parent directories.')
    parser.add_argument('--header-filter', default=None,
                        help='regular expression matching the names of the '
                        'headers to output diagnostics from. Diagnostics from '
                        'the main file of each translation unit are always '
                        'displayed.')
    parser.add_argument('--error', action='store_true', help='return 1 if there are warnings')
    parser.add_argument('--export-fixes', metavar='filename', dest='export_fixes',
                        help='Create a yaml file to store suggested fixes in, '
                        'which can be applied with clang-apply-replacements.')
    parser.add_argument('-j', type=int, default=0,
                        help='number of tidy instances to be run in parallel.')
    parser.add_argument('files', nargs='*', default=['.*'],
                        help='files to be processed (regex on path)')
    parser.add_argument('--fix', action='store_true', help='apply fix-its')
    parser.add_argument('--format', action='store_true', help='Reformat code '
                        'after applying fixes')
    parser.add_argument('--style', default='file', help='The style of reformat '
                        'code after applying fixes')
    parser.add_argument('--sort', default='file', choices=['file', 'diagnostic'], help="The primary sort of the errors.")
    parser.add_argument('-p', dest='build_path',
                        help='Path used to read a compile command database.')
    parser.add_argument('-s', '--source-tree', dest='source_tree', type=os.path.abspath, default='/', help='Common path to remove from warning output')
    parser.add_argument('--extra-arg', dest='extra_arg',
                        action='append', default=[],
                        help='Additional argument to append to the compiler '
                        'command line.')
    parser.add_argument('--extra-arg-before', dest='extra_arg_before',
                        action='append', default=[],
                        help='Additional argument to prepend to the compiler '
                        'command line.')
    parser.add_argument('--quiet', action='store_true',
                        help='Run clang-tidy in quiet mode')
    args = parser.parse_args()

    db_path = 'compile_commands.json'

    if args.build_path is not None:
        build_path = args.build_path
    else:
        # Find our database
        build_path = find_compilation_database(db_path)

    try:
        invocation = [args.clang_tidy_binary, '--version']
        subprocess.check_call(invocation)
    except subprocess.CalledProcessError:
        print("Unable to run clang-tidy.", file=sys.stderr)
        sys.exit(1)

    # Load the database and extract all files.
    database = json.load(open(os.path.join(build_path, db_path)))
    files = [make_absolute(entry['file'], entry['directory'])
             for entry in database]

    max_task = args.j
    if max_task == 0:
        max_task = multiprocessing.cpu_count()

    if args.fix:
        check_clang_apply_replacements_binary(args)
    tmpdir = tempfile.mkdtemp()

    # Build up a big regexy filter from all command line arguments.
    file_name_re = re.compile('|'.join(args.files))

    return_code = 0
    try:
        # Spin up a bunch of tidy-launching threads.
        task_queue = queue.Queue(max_task)
        # List of files with a non-zero return code.
        failed_files = []
        lock = threading.Lock()
        for _ in range(max_task):
            t = threading.Thread(target=run_tidy,
                                 args=(args, tmpdir, build_path, task_queue, lock, failed_files))
            t.daemon = True
            t.start()

        # Fill the queue with files.
        all_files = [file for file in files if file_name_re.search(file)]
        for file in sorted(all_files):
            task_queue.put(file)

        # Wait for all threads to be done.
        task_queue.join()
        if len(failed_files):
            return_code = 1

    except KeyboardInterrupt:
        # This is a sad hack. Unfortunately subprocess goes
        # bonkers with ctrl-c and we start forking merrily.
        print('\nCtrl-C detected, goodbye.')
        if tmpdir:
            shutil.rmtree(tmpdir)
        os.kill(0, 9)

    diagnostics = merge_replacement_files(tmpdir, args.source_tree, args.sort == 'diagnostic')

    print_warnings(diagnostics, args.source_tree, args.color)

    if args.export_fixes:
        print('Writing fixes to ' + args.export_fixes + ' ...')
        try:
            write_replacements(diagnostics, args.export_fixes)
        except Exception as e:
            print('Error exporting fixes.\n', file=sys.stderr)
            traceback.print_exc()
            return_code = 1

    if args.fix:
        print('Applying fixes ...')
        try:
            for file in glob.glob(os.path.join(tmpdir, '*')):
                os.remove(file)
            write_replacements(diagnostics, os.path.join(tmpdir, "fixes.yaml"))
            apply_fixes(args, tmpdir)
        except Exception as e:
            print('Error applying fixes.\n', file=sys.stderr)
            traceback.print_exc()
            return_code = 1

    shutil.rmtree(tmpdir)
    sys.exit(return_code)


if __name__ == '__main__':
    main()
