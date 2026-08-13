#!/bin/bash

################################################################################
# Equinox Logger Build Script
# 
# Usage: ./scripts/build.sh [OPTIONS] [TARGETS]
#
# This script handles building the Equinox Logger project with support for
# different build types (Debug/Release) and optional targets (examples, tests).
################################################################################

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_BASE_DIR="$PROJECT_ROOT/build"

# Default values
BUILD_TYPE="Debug"
BUILD_EXAMPLES=false
BUILD_TESTS=false
SKIP_TESTS=false
BUILD_SHARED=true
BUILD_STATIC=false
BUILD_BOTH=false
ENABLE_COVERAGE=false
SHOW_HELP=false
RUN_FORMAT=false
FORMAT_ONLY=false

################################################################################
# Helper Functions
################################################################################

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_header() {
    echo ""
    echo "================================================================================"
    echo -e "${BLUE}$1${NC}"
    echo "================================================================================"
    echo ""
}

show_help() {
    cat << 'EOF'

Equinox Logger Build Script
============================

USAGE:
    ./scripts/build.sh [OPTIONS] [TARGETS]

TARGETS:
    debug               Build library in Debug mode (default if no type specified)
    release             Build library in Release mode
    examples            Build example applications
    tests               Build and run unit tests (added to current build type)
    unit                Alias for 'tests'
    format              Format all source files using .clang-format

OPTIONS:
    --shared            Build shared library (default)
    --static            Build static library instead of shared library
    --both              Build both shared and static libraries
    --coverage          Generate LCOV coverage report (requires tests)
    --clean             Explicit clean flag (cleaning is always performed)
    --skip-tests        Build tests but do not run them
    --format            Format all source files using .clang-format
    --format-only       Format all source files and exit
    --help              Display this help message

EXAMPLES:
    # Display this help
    ./scripts/build.sh
    ./scripts/build.sh --help

    # Build debug library only (default)
    ./scripts/build.sh debug

    # Build release library with examples and tests
    ./scripts/build.sh release examples tests

    # Build debug library and examples
    ./scripts/build.sh debug examples

    # Build debug library with tests (skip running them)
    ./scripts/build.sh debug tests --skip-tests

    # Build release with static library
    ./scripts/build.sh release --static

    # Build debug with both shared and static libraries
    ./scripts/build.sh debug --both

    # Build release with static library and examples
    ./scripts/build.sh release --static examples

    # Build debug with tests and generate coverage report
    ./scripts/build.sh debug tests --coverage

    # Format all source files
    ./scripts/build.sh --format-only

BUILD DIRECTORY STRUCTURE:
    
    After building, the structure will be:
    
    build/
├── Debug/              (created with 'debug' target)
│   ├── libEquinoxLogger.so   (if --shared or default)
│   ├── libEquinoxLogger.a    (if --static or --both)
│   ├── Makefile
│   ├── CMakeCache.txt
│   ├── examples/
│   ├── tests/
│   └── ...
└── Release/            (created with 'release' target)
    ├── libEquinoxLogger.so   (if --shared or default)
    ├── libEquinoxLogger.a    (if --static or --both)
    ├── Makefile
    ├── CMakeCache.txt
        ├── tests/
        └── ...

DEFAULT BEHAVIOR:
    - If no build type is specified, 'debug' is used
    - The build directory is always cleaned before building
    - Unit tests are automatically run after building if '--skip-tests' is not set
    - Shared library is built by default
    - Use '--static' for static library only
    - Use '--both' to build both shared and static libraries simultaneously
    - Coverage requires tests to run and uses Debug build flags

EOF
}

check_cmake() {
    if ! command -v cmake &> /dev/null; then
        print_error "CMake is not installed. Please install CMake 3.22.1 or later."
        exit 1
    fi
    
    local cmake_version=$(cmake --version | head -n 1 | grep -oP '\d+\.\d+\.\d+')
    print_info "Using CMake version: $cmake_version"
}

check_compiler() {
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        print_error "No C++ compiler found. Please install g++ or clang++."
        exit 1
    fi
    
    if command -v g++ &> /dev/null; then
        local compiler_version=$(g++ --version | head -n 1)
        print_info "Using compiler: $compiler_version"
    else
        local compiler_version=$(clang++ --version | head -n 1)
        print_info "Using compiler: $compiler_version"
    fi
}

check_lcov() {
    if [ "$ENABLE_COVERAGE" = true ]; then
        if ! command -v lcov &> /dev/null || ! command -v genhtml &> /dev/null; then
            print_warning "lcov/genhtml not found. Attempting to install..."
            if command -v apt-get &> /dev/null; then
                sudo apt-get update
                sudo apt-get install -y lcov
            else
                print_error "Automatic install only supports apt-get. Please install lcov manually."
                exit 1
            fi
        fi
        if ! command -v lcov &> /dev/null || ! command -v genhtml &> /dev/null; then
            print_error "lcov/genhtml still not available after install attempt."
            exit 1
        fi
    fi
}

check_clang_format() {
    if ! command -v clang-format &> /dev/null; then
        print_error "clang-format is not installed. Please install clang-format to format code."
        exit 1
    fi
}

check_gtest() {
    if [ "$BUILD_TESTS" = true ]; then
        # Try to check with pkg-config if available
        if command -v pkg-config &> /dev/null; then
            if ! pkg-config --exists gtest gtest_main gmock 2>/dev/null; then
                print_error "Google Test (GTest) is not found on this system."
                print_error "Please install GTest first using: Setup-Environment-Scripts/gtest_install_script.sh"
                exit 1
            fi
        else
            # Fallback: check for common GTest library locations
            if [ ! -f "/usr/lib/libgtest.a" ] && [ ! -f "/usr/local/lib/libgtest.a" ] && \
               [ ! -f "/usr/lib/x86_64-linux-gnu/libgtest.a" ]; then
                print_warning "Cannot verify GTest installation (pkg-config not available)"
                print_warning "If GTest is not installed, the build will fail"
                print_warning "Install GTest using: Setup-Environment-Scripts/gtest_install_script.sh"
            fi
        fi
    fi
}

clean_build_dir() {
    local build_dir="$1"
    
    if [ -d "$build_dir" ]; then
        print_info "Cleaning build directory: $build_dir"
        rm -rf "$build_dir"
    fi
}

configure_cmake() {
    local build_dir="$1"
    local build_type="$2"
    
    print_header "Configuring CMake for $build_type build"
    
    # Create build directory
    mkdir -p "$build_dir"
    
        # Prepare CMake arguments
        local cmake_args=("-S" "$PROJECT_ROOT" "-B" "$build_dir" "-DCMAKE_BUILD_TYPE=$build_type")
    
    # Add optional flags
    if [ "$BUILD_EXAMPLES" = true ]; then
           cmake_args+=("-DEQUINOX_LOGGER_EXAMPLES=ON")
    else
           cmake_args+=("-DEQUINOX_LOGGER_EXAMPLES=OFF")
    fi
    
    if [ "$BUILD_TESTS" = true ]; then
           cmake_args+=("-DEQUINOX_LOGGER_TESTS=ON")
    else
           cmake_args+=("-DEQUINOX_LOGGER_TESTS=OFF")
    fi

    if [ "$ENABLE_COVERAGE" = true ]; then
           cmake_args+=("-DCMAKE_CXX_FLAGS=--coverage -O0 -g")
           cmake_args+=("-DCMAKE_EXE_LINKER_FLAGS=--coverage")
    fi
    
    # Handle library type configuration
    if [ "$BUILD_BOTH" = true ]; then
        # Build both shared and static libraries
           cmake_args+=("-DEQUINOX_LOGGER_BUILD_SHARED=ON" "-DEQUINOX_LOGGER_BUILD_STATIC=ON")
    elif [ "$BUILD_STATIC" = true ]; then
        # Build only static library
           cmake_args+=("-DEQUINOX_LOGGER_BUILD_STATIC=ON" "-DEQUINOX_LOGGER_BUILD_SHARED=OFF")
    else
        # Build only shared library (default)
           cmake_args+=("-DEQUINOX_LOGGER_BUILD_SHARED=ON" "-DEQUINOX_LOGGER_BUILD_STATIC=OFF")
    fi
    
        print_info "CMake args: ${cmake_args[*]}"
    
    # Run CMake with explicit source and build directories
        if cmake "${cmake_args[@]}"; then
        print_success "CMake configuration successful"
    else
        print_error "CMake configuration failed"
        exit 1
    fi
}

build_project() {
    local build_dir="$1"
    local build_type="$2"
    
    print_header "Building $build_type"
    
    cd "$build_dir"
    
    # Determine number of jobs to use
    local num_jobs=$(nproc)
    print_info "Using $num_jobs parallel jobs for build"
    
    # Run make
    if make -j "$num_jobs"; then
        print_success "Build completed successfully"
    else
        print_error "Build failed"
        exit 1
    fi
}

run_tests() {
    local build_dir="$1"
    
    if [ "$BUILD_TESTS" = false ] || [ "$SKIP_TESTS" = true ]; then
        return 0
    fi
    
    print_header "Running Unit Tests"
    
    if [ ! -f "$build_dir/tests/EquinoxLoggerTests.x86" ]; then
        print_warning "Test executable not found at $build_dir/tests/EquinoxLoggerTests.x86"
        return 1
    fi
    
    cd "$build_dir"
    
    # Performance benchmark tests are intentionally excluded from the default
    # build.sh test run so that regular CI/local validation stays focused on
    # correctness checks and avoids noisy throughput benchmarks.
    if ./tests/EquinoxLoggerTests.x86 --gtest_filter=-PerformanceBenchmark.*; then
        print_success "All non-performance tests passed"
        return 0
    else
        print_error "Some tests failed"
        return 1
    fi
}

generate_coverage() {
    local build_dir="$1"

    if [ "$ENABLE_COVERAGE" = false ]; then
        return 0
    fi

    if [ "$BUILD_TESTS" = false ] || [ "$SKIP_TESTS" = true ]; then
        print_warning "Coverage requested but tests were not executed"
        return 0
    fi

    print_header "Generating Coverage Report (LCOV)"
    cd "$build_dir"

    local coverage_dir="$build_dir/coverage"
    local coverage_info="$coverage_dir/coverage.info"

    mkdir -p "$coverage_dir"

    # Capture coverage data
    lcov --directory "$build_dir" --capture --output-file "$coverage_info" \
        --rc geninfo_unexecuted_blocks=1 \
        --ignore-errors mismatch

    # Remove system and test files from report
    lcov --remove "$coverage_info" "/usr/*" "*/tests/*" --output-file "$coverage_info"

    # Generate HTML report
    genhtml "$coverage_info" --output-directory "$coverage_dir/html"

    print_success "Coverage report generated: $coverage_dir/html/index.html"
}

format_code() {
    print_header "Formatting Source Code"

    local search_dirs=(
        "$PROJECT_ROOT/src"
        "$PROJECT_ROOT/include"
        "$PROJECT_ROOT/api"
        "$PROJECT_ROOT/tests"
        "$PROJECT_ROOT/examples"
    )

    local existing_dirs=()
    local dir
    for dir in "${search_dirs[@]}"; do
        if [ -d "$dir" ]; then
            existing_dirs+=("$dir")
        fi
    done

    if [ ${#existing_dirs[@]} -eq 0 ]; then
        print_warning "No source directories found to format"
        return 0
    fi

    find "${existing_dirs[@]}" -type f \( \
        -name "*.c" -o -name "*.cc" -o -name "*.cpp" -o -name "*.cxx" -o \
        -name "*.h" -o -name "*.hpp" \
    \) -print0 | xargs -0 -r clang-format -i -style=file

    print_success "Formatting complete"
}

display_build_summary() {
    local build_type="$1"
    local build_dir="$2"
    
    print_header "Build Summary"
    
    echo "Build Type:              $build_type"
    echo "Build Directory:         $build_dir"
    echo "Build Shared:            $BUILD_SHARED"
    echo "Build Static:            $BUILD_STATIC"
    echo "Build Both:              $BUILD_BOTH"
    echo "Build Examples:          $BUILD_EXAMPLES"
    echo "Build Tests:             $BUILD_TESTS"
    echo "Skip Running Tests:      $SKIP_TESTS"
    echo "Coverage (LCOV):         $ENABLE_COVERAGE"
    
    print_success "Build configuration set up"
}

################################################################################
# Parse Arguments
################################################################################

parse_arguments() {
    local args_provided=false
    
    for arg in "$@"; do
        args_provided=true
        
        case "$arg" in
            debug)
                BUILD_TYPE="Debug"
                ;;
            release)
                BUILD_TYPE="Release"
                ;;
            examples)
                BUILD_EXAMPLES=true
                ;;
            tests|unit)
                BUILD_TESTS=true
                ;;
            format)
                RUN_FORMAT=true
                FORMAT_ONLY=true
                ;;
            --shared)
                BUILD_SHARED=true
                BUILD_STATIC=false
                BUILD_BOTH=false
                ;;
            --static)
                BUILD_SHARED=false
                BUILD_STATIC=true
                BUILD_BOTH=false
                ;;
            --both)
                BUILD_SHARED=true
                BUILD_STATIC=true
                BUILD_BOTH=true
                ;;
            --coverage|--lcov)
                ENABLE_COVERAGE=true
                BUILD_TESTS=true
                ;;
            --clean)
                # Always clean anyway, so this is just for clarity
                ;;
            --skip-tests)
                SKIP_TESTS=true
                ;;
            --format)
                RUN_FORMAT=true
                ;;
            --format-only)
                RUN_FORMAT=true
                FORMAT_ONLY=true
                ;;
            --help|-h)
                SHOW_HELP=true
                ;;
            *)
                print_error "Unknown argument: $arg"
                SHOW_HELP=true
                ;;
        esac
    done
    
    # If no args provided, show help
    if [ "$args_provided" = false ]; then
        SHOW_HELP=true
    fi
}

################################################################################
# Main Execution
################################################################################

perform_build() {
    local build_type="$1"
    local build_shared="$2"
    local build_static="$3"
    local build_suffix="$4"
    
    # Determine build directory with optional suffix
    local BUILD_DIR="$BUILD_BASE_DIR/$build_type$build_suffix"
    
    # Display build configuration
    display_build_summary "$build_type" "$BUILD_DIR"
    
    # Clean build directory
    print_header "Cleaning"
    clean_build_dir "$BUILD_DIR"
    print_success "Clean completed"
    
    # Temporarily update global variables for this build
    local PREV_SHARED=$BUILD_SHARED
    local PREV_STATIC=$BUILD_STATIC
    local PREV_BOTH=$BUILD_BOTH
    BUILD_SHARED=$build_shared
    BUILD_STATIC=$build_static
    BUILD_BOTH=false  # Important: disable BUILD_BOTH so configure_cmake() uses correct logic
    
    # Configure with CMake
    configure_cmake "$BUILD_DIR" "$build_type"
    
    # Build the project
    build_project "$BUILD_DIR" "$build_type"
    
    # Run tests if applicable
    run_tests "$BUILD_DIR"

    # Generate coverage report if requested
    generate_coverage "$BUILD_DIR"
    
    # Restore previous values
    BUILD_SHARED=$PREV_SHARED
    BUILD_STATIC=$PREV_STATIC
    BUILD_BOTH=$PREV_BOTH
    
    # Final summary
    print_header "Build Complete"
    print_success "Build succeeded!"
    echo ""
    echo "Build artifacts located in: $BUILD_DIR"
    echo ""
}

main() {
    # Parse command line arguments
    parse_arguments "$@"
    
    # Show help if requested or no arguments
    if [ "$SHOW_HELP" = true ]; then
        show_help
        exit 0
    fi

    if [ "$FORMAT_ONLY" = true ]; then
        check_clang_format
        format_code
        exit 0
    fi
    
    # Print project info
    print_header "Equinox Logger Build System"
    print_info "Project Root: $PROJECT_ROOT"
    
    # Pre-flight checks
    print_header "Pre-flight Checks"
    check_cmake
    check_compiler
    check_lcov
    if [ "$BUILD_TESTS" = true ]; then
        check_gtest
    fi

    if [ "$RUN_FORMAT" = true ]; then
        check_clang_format
        format_code
    fi

    if [ "$ENABLE_COVERAGE" = true ] && [ "$BUILD_TYPE" = "Release" ]; then
        print_warning "Coverage enabled; switching build type to Debug for accurate results"
        BUILD_TYPE="Debug"
    fi
    
    # Handle --both option: build shared and static separately
    if [ "$BUILD_BOTH" = true ]; then
        print_header "Building Both Shared and Static Libraries"
        
        # Build shared version
        perform_build "$BUILD_TYPE" true false "-shared"
        
        # Build static version
        perform_build "$BUILD_TYPE" false true "-static"
        
        # Final combined summary
        print_header "All Builds Complete"
        print_success "Both shared and static libraries built successfully!"
        echo ""
        echo "Shared library:  $BUILD_BASE_DIR/$BUILD_TYPE-shared/"
        echo "Static library:  $BUILD_BASE_DIR/$BUILD_TYPE-static/"
        echo ""
    else
        # Normal single build
        perform_build "$BUILD_TYPE" "$BUILD_SHARED" "$BUILD_STATIC" ""
    fi
}

# Execute main function with all arguments
main "$@"
