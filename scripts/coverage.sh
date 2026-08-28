#!/bin/bash

################################################################################
# Equinox Logger Coverage Analysis Script
# 
# Usage: ./scripts/coverage.sh [OPTIONS]
#
# This script measures unit test coverage using lcov and generates an HTML report.
# It builds the project with coverage instrumentation, runs tests, and collects
# coverage data.
#
# Options:
#   -h, --help              Show this help message
#   -o, --open              Open the coverage report in default browser after generation
#   -c, --clean             Clean coverage data before generating new report
#   --exclude-pattern PATH  Exclude files matching pattern (can be used multiple times)
#   --min-coverage PCT      Fail if coverage is below percentage threshold
################################################################################

set -euo pipefail

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
ORANGE='\033[0;33m'
NC='\033[0m' # No Color

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build/Debug-coverage"
COVERAGE_DIR="$BUILD_DIR/coverage"
HTML_REPORT_DIR="$COVERAGE_DIR/html"

# Options
OPEN_REPORT=false
CLEAN_FIRST=false
MIN_COVERAGE=0
EXCLUDE_PATTERNS=()

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
    echo -e "${RED}[✗]${NC} $1" >&2
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_step() {
    echo -e "${ORANGE}==>${NC} $1"
}

show_help() {
    head -27 "$0" | tail -24
}

check_command() {
    if ! command -v "$1" &> /dev/null; then
        print_error "Command '$1' not found. Please install it."
        return 1
    fi
}

################################################################################
# Argument Parsing
################################################################################

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -o|--open)
            OPEN_REPORT=true
            shift
            ;;
        -c|--clean)
            CLEAN_FIRST=true
            shift
            ;;
        --exclude-pattern)
            if [[ -z "$2" ]]; then
                print_error "Missing argument for $1"
                exit 1
            fi
            EXCLUDE_PATTERNS+=("$2")
            shift 2
            ;;
        --min-coverage)
            if [[ -z "$2" ]] || ! [[ "$2" =~ ^[0-9]+$ ]]; then
                print_error "Invalid coverage threshold: $2"
                exit 1
            fi
            MIN_COVERAGE="$2"
            shift 2
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

################################################################################
# Pre-flight Checks
################################################################################

print_info "Checking for required tools..."

if ! check_command lcov; then
    print_error "lcov is required for coverage analysis"
    print_info "Install with: sudo apt-get install lcov (Ubuntu/Debian) or brew install lcov (macOS)"
    exit 1
fi

if ! check_command cmake; then
    print_error "cmake is required to build the project"
    exit 1
fi

if ! check_command make; then
    print_error "make is required to build the project"
    exit 1
fi

print_success "All required tools found"

################################################################################
# Setup
################################################################################

print_step "Setting up coverage build environment"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Clean previous coverage data if requested
if [[ "$CLEAN_FIRST" == true ]]; then
    print_info "Cleaning previous coverage data..."
    rm -rf "$COVERAGE_DIR"
    mkdir -p "$COVERAGE_DIR"
fi

################################################################################
# Build with Coverage
################################################################################

print_step "Building project with coverage instrumentation"

# Create build directory
mkdir -p "$BUILD_DIR"

# Configure CMake with coverage flags
# Using --coverage is the recommended way for GCC/Clang
cmake -S "$PROJECT_ROOT" \
    -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    "-DCMAKE_CXX_FLAGS=--coverage -fprofile-update=atomic -O0 -g" \
    "-DCMAKE_C_FLAGS=--coverage -fprofile-update=atomic -O0 -g" \
    "-DCMAKE_EXE_LINKER_FLAGS=--coverage" \
    "-DCMAKE_SHARED_LINKER_FLAGS=--coverage" \
    -DEQUINOX_LOGGER_TESTS=ON \
    -DEQUINOX_LOGGER_EXAMPLES=OFF

# Build the project
make -C "$BUILD_DIR" -j"$(nproc)"

print_success "Build completed with coverage instrumentation"

# Reset counters before tests so captured data reflects only the current run
print_info "Resetting previous coverage counters..."
lcov --directory "$BUILD_DIR" --zerocounters --quiet

################################################################################
# Run Tests
################################################################################

print_step "Running unit tests"

# Run tests and collect coverage data
if [[ -f "$BUILD_DIR/tests/EquinoxLoggerTests.x86" ]]; then
    "$BUILD_DIR/tests/EquinoxLoggerTests.x86" || {
        print_warning "Some tests failed, but continuing with coverage analysis"
    }
    print_success "Tests execution completed"
else
    print_error "Tests executable not found at $BUILD_DIR/tests/EquinoxLoggerTests.x86"
    exit 1
fi

################################################################################
# Generate Coverage Report
################################################################################

print_step "Generating coverage report with lcov"

# Create coverage directory
mkdir -p "$COVERAGE_DIR"

# Capture coverage data
print_info "Capturing coverage data..."
lcov --directory "$BUILD_DIR" \
    --capture \
    --output-file "$COVERAGE_DIR/coverage.info" \
    --rc geninfo_unexecuted_blocks=1 \
    --ignore-errors mismatch,gcov,negative \
    --quiet

# Remove coverage data for excluded patterns
if [[ ${#EXCLUDE_PATTERNS[@]} -gt 0 ]]; then
    print_info "Removing coverage for excluded patterns..."
    for pattern in "${EXCLUDE_PATTERNS[@]}"; do
        lcov --remove "$COVERAGE_DIR/coverage.info" \
            "$pattern" \
            --output-file "$COVERAGE_DIR/coverage.info" \
            --ignore-errors unused \
            --quiet
    done
fi

# Remove system headers and test files
print_info "Filtering out system files and external dependencies..."
lcov --remove "$COVERAGE_DIR/coverage.info" \
    '/usr/include/*' \
    '*/tests/*' \
    '*/test/*' \
    '*/.vscode/*' \
    '*/CMakeFiles/*' \
    '*/ThirdParty/fmt/*' \
    '*_test.cpp' \
    --output-file "$COVERAGE_DIR/coverage.info" \
    --ignore-errors unused \
    --quiet

# Generate HTML report
print_info "Generating HTML report..."
mkdir -p "$HTML_REPORT_DIR"
genhtml "$COVERAGE_DIR/coverage.info" \
    --output-directory "$HTML_REPORT_DIR" \
    --title "Equinox Logger - Coverage Report" \
    --prefix "$PROJECT_ROOT" \
    --quiet

print_success "Coverage report generated"

################################################################################
# Summary and Statistics
################################################################################

print_step "Coverage Summary"

# Extract coverage statistics
STATS=$(lcov --summary "$COVERAGE_DIR/coverage.info" 2>/dev/null || echo "")
if [[ -n "$STATS" ]]; then
    echo "$STATS" | tail -3
fi

# Check minimum coverage threshold
if [[ $MIN_COVERAGE -gt 0 ]]; then
    LINES_COVERAGE=$(lcov --summary "$COVERAGE_DIR/coverage.info" 2>/dev/null | grep "lines" | grep -oP '\d+\.\d+(?=%)' | head -1 || echo "0")
    if (( $(echo "$LINES_COVERAGE < $MIN_COVERAGE" | bc -l) )); then
        print_error "Coverage ${LINES_COVERAGE}% is below minimum threshold of ${MIN_COVERAGE}%"
        exit 1
    fi
fi

################################################################################
# Report Location
################################################################################

print_info "Coverage report saved to: $HTML_REPORT_DIR"
print_info "Main report: $HTML_REPORT_DIR/index.html"

# Open report in browser if requested
if [[ "$OPEN_REPORT" == true ]]; then
    print_info "Opening coverage report in browser..."
    if command -v xdg-open &> /dev/null; then
        xdg-open "$HTML_REPORT_DIR/index.html"
    elif command -v open &> /dev/null; then
        open "$HTML_REPORT_DIR/index.html"
    else
        print_warning "Could not automatically open browser. Please open manually: $HTML_REPORT_DIR/index.html"
    fi
fi

print_success "Coverage analysis complete!"
