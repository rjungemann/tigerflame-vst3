# TigerFlame VST3/CLAP Plugin - Justfile
# Usage: just <recipe>

# Default recipe
default:
    {{ just --justfile '{{ justfile() }}' --list }}

# ============ Configuration ============

# Configure for Release build
configure:
    #!/usr/bin/env bash
    set -euxo pipefail
    rm -rf build
    cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_IMGUI_UI=ON \
        -DBUILD_STANDALONE=OFF \
        -DBUILD_STANDALONE_MIDI=OFF \
        -DBUILD_CLAP=OFF \
        -DBUILD_TESTS=ON

# Configure for Debug build
configure-debug:
    #!/usr/bin/env bash
    set -euxo pipefail
    rm -rf build
    cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_IMGUI_UI=ON \
        -DBUILD_STANDALONE=OFF \
        -DBUILD_STANDALONE_MIDI=OFF \
        -DBUILD_CLAP=OFF \
        -DBUILD_TESTS=ON

# Configure with sanitizers
configure-asan:
    #!/usr/bin/env bash
    set -euxo pipefail
    rm -rf build
    cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Debug \
        -DENABLE_SANITIZER_ADDRESS=ON \
        -DENABLE_SANITIZER_LEAK=ON \
        -DENABLE_SANITIZER_UNDEFINED=ON \
        -DBUILD_IMGUI_UI=ON \
        -DBUILD_STANDALONE=OFF \
        -DBUILD_TESTS=ON

# Configure with coverage
configure-coverage:
    #!/usr/bin/env bash
    set -euxo pipefail
    rm -rf build
    cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_WITH_COVERAGE=ON \
        -DBUILD_TESTS=ON

# ============ Building ============

# Build all targets
build:
    #!/usr/bin/env bash
    set -euxo pipefail
    cmake --build build --config Release -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

build-debug:
    #!/usr/bin/env bash
    set -euxo pipefail
    cmake --build build --config Debug -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# Build VST3 plugin only
build-vst3:
    #!/usr/bin/env bash
    set -euxo pipefail
    cmake --build build --target tigerflame-vst3 -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# Build tests only
build-tests:
    #!/usr/bin/env bash
    set -euxo pipefail
    cmake --build build --target tigerflame-tests -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# Build standalone application
build-standalone:
    #!/usr/bin/env bash
    set -euxo pipefail
    cmake --build build --target tigerflame-standalone -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# Build CLAP plugin
build-clap:
    #!/usr/bin/env bash
    set -euxo pipefail
    cmake --build build --target tigerflame-clap -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# ============ Testing ============

# Run all tests
test: build-tests
    #!/usr/bin/env bash
    set -euxo pipefail
    cd build && ctest --output-on-failure -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# Run specific test
test-one test_name:
    #!/usr/bin/env bash
    set -euxo pipefail
    cd build && ctest -R "^{test_name}$" --output-on-failure

# Run tests with verbose output
test-verbose: build-tests
    #!/usr/bin/env bash
    set -euxo pipefail
    cd build && ./tigerflame-tests -v

# Run tests with specific tags
test-tags tag:
    #!/usr/bin/env bash
    set -euxo pipefail
    cd build && ./tigerflame-tests -t "[{tag}]"

# ============ Running ============

# Run standalone application
run: build-standalone
    #!/usr/bin/env bash
    set -euxo pipefail
    cd build && ./tigerflame-standalone

# Run with debug output
run-debug: build-standalone
    #!/usr/bin/env bash
    set -euxo pipefail
    cd build && ./tigerflame-standalone --debug

# ============ Cleanup ============

# Clean build directory
clean:
    #!/usr/bin/env bash
    rm -rf build

# Deep clean (includes fetched dependencies)
clean-all:
    #!/usr/bin/env bash
    rm -rf build extern

# ============ Install ============

# Install VST3 plugin (macOS)
install-mac:
    #!/usr/bin/env bash
    set -euxo pipefail
    cmake --build build --target install
    # Copy to standard VST3 locations
    mkdir -p ~/Library/Audio/Plug-Ins/VST3
    cp -R build/VST3/TigerFlame.vst3 ~/Library/Audio/Plug-Ins/VST3/
    
install-win:
    #!/usr/bin/env bash
    set -euxo pipefail
    cmake --build build --target install
    # Copy to standard VST3 locations
    mkdir -p "${ProgramFiles}/Common Files/VST3"
    cp -R build/VST3/TigerFlame.vst3 "${ProgramFiles}/Common Files/VST3/"
    mkdir -p "${LOCALAPPDATA}/Programs/Common Files/VST3"
    cp -R build/VST3/TigerFlame.vst3 "${LOCALAPPDATA}/Programs/Common Files/VST3/"

# ============ Development ============

# Full build and test cycle
dev:
    #!/usr/bin/env bash
    set -euxo pipefail
    just configure && just build && just test

# Debug build and test
dev-debug:
    #!/usr/bin/env bash
    set -euxo pipefail
    just configure-debug && just build-debug && just test

# Open build folder in VS Code
dev-vscode:
    code build

# Show build info
dev-info:
    #!/usr/bin/env bash
    cmake --build build --target help

# ============ CI/CD ============

# CI build (all configurations)
ci:
    #!/usr/bin/env bash
    set -euxo pipefail
    just configure && just build
    just configure-debug && just build-debug
    just test

# CI build with coverage
ci-coverage:
    #!/usr/bin/env bash
    set -euxo pipefail
    just configure-coverage && just build-debug
    just test
    # Generate coverage report
    lcov --capture --directory build --output-file coverage.info
    genhtml coverage.info --output-directory coverage-html

# ============ Documentation ============

# Generate Doxygen documentation
docs:
    #!/usr/bin/env bash
    set -euxo pipefail
    doxygen Doxyfile 2>/dev/null || echo "Doxygen not installed"

# ============ Utility ============

# List all recipes
list:
    #!/usr/bin/env bash
    just --list

# Show CMake cache
show-cache:
    #!/usr/bin/env bash
    cmake -LA build/CMakeCache.txt 2>/dev/null || echo "No build directory"

# Show disk usage
disk-usage:
    #!/usr/bin/env bash
    du -sh build extern 2>/dev/null || echo "No build directory"
