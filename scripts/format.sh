#!/usr/bin/env bash

################################################################################
# Equinox Logger Source Code Formatter Script
#
# Usage: ./scripts/format.sh [OPTIONS]
#
# Formats all project C/C++ source and header files (including unit tests,
# excluding ThirdParty and build directories) according to .clang-format.
################################################################################

set -euo pipefail

# Color definitions for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CLANG_FORMAT_FILE="${PROJECT_ROOT}/.clang-format"

CHECK_ONLY=false
VERBOSE=false

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

show_help() {
    cat << EOF
Equinox Logger Code Formatting Script
======================================

USAGE:
    ./scripts/format.sh [OPTIONS]

OPTIONS:
    -c, --check       Check formatting without modifying files (exit with status 1 if formatting needed)
    -v, --verbose     Display detailed output for each file being processed
    -h, --help        Show this help message

DESCRIPTION:
    Formats all C/C++ source code files (.cpp, .hpp, .h, .c, .cc, .cxx)
    in the project according to rules specified in .clang-format.
    Includes unit tests (tests/), api/, include/, src/, examples/.
    Excludes ThirdParty/ and build output directories.
EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -c|--check|--dry-run)
            CHECK_ONLY=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    print_error "clang-format is not installed or not in PATH."
    exit 1
fi

# Check if .clang-format config exists
if [ ! -f "$CLANG_FORMAT_FILE" ]; then
    print_error "Configuration file .clang-format not found at: $CLANG_FORMAT_FILE"
    exit 1
fi

print_info "Searching for C/C++ source files (excluding ThirdParty and build directories)..."

# Collect matching files, excluding ThirdParty and build/temp folders
mapfile -t FILES < <(find "$PROJECT_ROOT" \
    \( \
        -path "$PROJECT_ROOT/ThirdParty" -o \
        -path "$PROJECT_ROOT/ThirdParty/*" -o \
        -path "$PROJECT_ROOT/build" -o \
        -path "$PROJECT_ROOT/build/*" -o \
        -path "$PROJECT_ROOT/build-ci" -o \
        -path "$PROJECT_ROOT/build-ci/*" -o \
        -path "$PROJECT_ROOT/CMakeFiles" -o \
        -path "$PROJECT_ROOT/CMakeFiles/*" -o \
        -path "$PROJECT_ROOT/.git" -o \
        -path "$PROJECT_ROOT/.git/*" -o \
        -path "$PROJECT_ROOT/.vscode" -o \
        -path "$PROJECT_ROOT/.vscode/*" \
    \) -prune -o \
    -type f \( \
        -name "*.cpp" -o \
        -name "*.hpp" -o \
        -name "*.h" -o \
        -name "*.c" -o \
        -name "*.cc" -o \
        -name "*.cxx" \
    \) -print | sort)

TOTAL_FILES=${#FILES[@]}

if [ "$TOTAL_FILES" -eq 0 ]; then
    print_warning "No source files found to format."
    exit 0
fi

print_info "Found $TOTAL_FILES source file(s) to process."

if [ "$CHECK_ONLY" = true ]; then
    print_info "Checking formatting compliance..."
    NEEDS_FORMAT=0
    UNFORMATTED_FILES=()

    for file in "${FILES[@]}"; do
        rel_path="${file#"$PROJECT_ROOT/"}"
        if ! clang-format --dry-run --Werror --style=file:"$CLANG_FORMAT_FILE" "$file" &>/dev/null; then
            NEEDS_FORMAT=$((NEEDS_FORMAT + 1))
            UNFORMATTED_FILES+=("$rel_path")
            if [ "$VERBOSE" = true ]; then
                print_warning "File needs formatting: $rel_path"
            fi
        elif [ "$VERBOSE" = true ]; then
            print_info "Formatted OK: $rel_path"
        fi
    done

    if [ "$NEEDS_FORMAT" -gt 0 ]; then
        print_error "$NEEDS_FORMAT file(s) need formatting:"
        for unformatted in "${UNFORMATTED_FILES[@]}"; do
            echo -e "  ${RED}-${NC} $unformatted"
        done
        print_error "Run './scripts/format.sh' to fix formatting."
        exit 1
    else
        print_success "All $TOTAL_FILES source files comply with .clang-format."
        exit 0
    fi
else
    print_info "Formatting files using .clang-format..."
    for file in "${FILES[@]}"; do
        rel_path="${file#"$PROJECT_ROOT/"}"
        if [ "$VERBOSE" = true ]; then
            print_info "Formatting: $rel_path"
        fi
        clang-format -i --style=file:"$CLANG_FORMAT_FILE" "$file"
    done
    print_success "Successfully formatted $TOTAL_FILES source file(s)."
fi
