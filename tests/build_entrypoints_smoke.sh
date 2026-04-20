#!/bin/bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cmake --list-presets -S "$repo_root" >/dev/null
test -x "$repo_root/scripts/bootstrap.debian.release.sh"
bash -n "$repo_root/scripts/bootstrap.debian.release.sh"
