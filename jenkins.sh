#!/usr/bin/env bash

# Fail if failed
set -e

# Coverage
curl -so codecov.bash https://codecov.io/bash
echo "End Clone"

# Clang Build
mkdir build-clang
cd build-clang
cmake ..\
    -DNOVA_TEST=ON\
    -DCMAKE_BUILD_TYPE=Debug\
    -DCMAKE_EXPORT_COMPILE_COMMANDS=On\
    -DNOVA_TREAT_WARNINGS_AS_ERRORS=On\
    -DCMAKE_C_COMPILER=clang-8 -DCMAKE_CXX_COMPILER=clang++-8\
    -DCMAKE_{C,CXX}_COMPILER_LAUNCHER=ccache\
    -DCMAKE_{C,CXX}_FLAGS="-fsanitize=address,undefined"\
    -DCMAKE_{EXE,SHARED}_LINKER_FLAGS="-fuse-ld=lld"\
    -GNinja
NINJA_STATUS="[%f/%t: %p - %r processes @ %c|%o files/s %es] " nice ninja -j8
echo "End Clang-Build"
./nova-test-unit
echo "End Clang-Tests"
cd ..

# GCC Build
mkdir build-gcc
cd build-gcc
cmake ..\
    -DNOVA_TEST=On\
    -DCMAKE_BUILD_TYPE=Debug\
    -DNOVA_TREAT_WARNINGS_AS_ERRORS=On\
    -DCMAKE_C_COMPILER=gcc-7 -DCMAKE_CXX_COMPILER=g++-7\
    -DCMAKE_{C,CXX}_COMPILER_LAUNCHER=ccache\
    -DNOVA_COVERAGE=On\
    -GNinja
NINJA_STATUS="[%f/%t: %p - %r processes @ %c|%o files/s %es] " nice ninja -j8
lcov -c -i -d . -o empty-coverage.info --gcov-tool gcov-7
echo "End GCC-Build"
./nova-test-unit
echo "End GCC-Tests"
cd ..

# Linting
cd build-clang
${WORKSPACE}/3rdparty/run-clang-tidy/run-clang-tidy.py -export-fixes fixes.yaml -j8 -header-filter "${WORKSPACE}"'/(src|tests)/.*' `find ../{src,tests}/ -iname '*.cpp'` -clang-tidy-binary clang-tidy-8
# test `cat fixes.yaml | wc -c` -eq 1
echo "End linting"
cd ..

cd build-gcc
lcov -c -d . -o live-coverage.info --gcov-tool gcov-7
lcov -a empty-coverage.info -a live-coverage.info -o coverage.info --gcov-tool gcov-7
lcov --remove coverage.info "/usr/*" "$WORKSPACE/3rdparty/*" "$WORKSPACE/tests/*" --output-file coverage.info
lcov --list coverage.info
bash ../codecov.bash -f coverage.info -t 'ae315f88-50ab-43e2-ae08-24e1b47e4f54' || echo "Codecov did not collect coverage reports"
echo "End Coverage"
cd ..
