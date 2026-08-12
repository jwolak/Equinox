#!/usr/bin/env sh
set -eu

REPO_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"

cd "$REPO_ROOT"

if [ -d ".githooks" ]; then
    find .githooks -maxdepth 1 -type f -exec chmod +x {} \;
fi

git config core.hooksPath .githooks

echo "Git hooks path set to .githooks"
