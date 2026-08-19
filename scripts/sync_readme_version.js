const fs = require('fs');
const path = require('path');

const repoRoot = path.resolve(__dirname, '..');
const packageJsonPath = path.join(repoRoot, 'package.json');
const readmePath = path.join(repoRoot, 'README.md');

const pkg = JSON.parse(fs.readFileSync(packageJsonPath, 'utf8'));
const version = pkg.version || '';
const shortVersion = version.split('.').slice(0, 2).join('.');

const readme = fs.readFileSync(readmePath, 'utf8');
const updated = readme
  .replace(/__VERSION_SHORT__/g, shortVersion)
  .replace(/__VERSION_FULL__/g, version);

if (updated !== readme) {
  fs.writeFileSync(readmePath, updated);
  console.log(`README version synced to ${shortVersion} (package.json: ${version})`);
} else {
  console.log(`README already uses version ${shortVersion}`);
}
