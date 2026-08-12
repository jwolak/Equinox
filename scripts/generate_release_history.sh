#!/usr/bin/env sh
set -eu

REPO_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUTPUT_FILE="$REPO_ROOT/RELEASE_NOTES.txt"
TMP_FILE="$(mktemp)"

cat > "$TMP_FILE" <<'EOF'
# Equinox Logger release notes history
# Format: YYYY-MM-DD HH:MM:SS +ZZZZ | vMAJOR.MINOR.PATCH | commit message

EOF

git -C "$REPO_ROOT" log --reverse --pretty=format:'%H%x09%ad%x09%s' --date=format:'%Y-%m-%d %H:%M:%S %z' |
while IFS="$(printf '\t')" read -r COMMIT_HASH COMMIT_DATE COMMIT_SUBJECT; do
    VERSION="$(git -C "$REPO_ROOT" show "$COMMIT_HASH:package.json" 2>/dev/null | sed -n 's/^[[:space:]]*"version":[[:space:]]*"\([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\)".*/\1/p' | head -n 1 || true)"

    if [ -z "$VERSION" ]; then
        VERSION="unknown"
    fi

    printf '%s | v%s | %s\n' "$COMMIT_DATE" "$VERSION" "$COMMIT_SUBJECT" >> "$TMP_FILE"
done

mv "$TMP_FILE" "$OUTPUT_FILE"
echo "Generated $OUTPUT_FILE"
