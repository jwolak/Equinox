#!/usr/bin/env python3
import json
import re
from pathlib import Path

repo_root = Path(__file__).resolve().parent.parent
package_path = repo_root / 'package.json'
readme_path = repo_root / 'README.md'

with package_path.open('r', encoding='utf-8') as f:
    pkg = json.load(f)

version = pkg.get('version', '')

readme = readme_path.read_text(encoding='utf-8')
updated = readme
updated = re.sub(
    r'^(# Equinox logging engine )\d+\.\d+\.\d+$',
    lambda match: match.group(1) + version,
    updated,
    flags=re.MULTILINE,
)
updated = re.sub(
    r'^(\*\*Thread safety C\+\+ logger version )\d+\.\d+\.\d+\*\*$',
    lambda match: match.group(1) + version,
    updated,
    flags=re.MULTILINE,
)
updated = updated.replace('__VERSION_SHORT__', version).replace('__VERSION_FULL__', version)

if updated != readme:
    readme_path.write_text(updated, encoding='utf-8')
    print(f'README version synced to {version} (package.json: {version})')
else:
    print(f'README already uses version {version}')
