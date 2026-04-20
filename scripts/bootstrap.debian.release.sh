#!/bin/bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
prefix="${PREFIX:-$repo_root/.deps/linux-release}"
src_root="${SRC_ROOT:-$repo_root/.bootstrap-src/linux-release}"
jobs="${JOBS:-$(nproc)}"
build_type="${CMAKE_BUILD_TYPE:-Release}"
skip_apt=0

usage() {
    cat <<EOF
Usage: $(basename "$0") [--no-apt]

Bootstraps the missing full-release build dependencies for Debian/Ubuntu into:
  $prefix

Environment overrides:
  PREFIX           install prefix for locally built dependencies
  SRC_ROOT         working directory for cloned upstream sources
  JOBS             parallel build jobs
  CMAKE_BUILD_TYPE CMake build type (default: Release)
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --no-apt)
            skip_apt=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

if [[ $EUID -eq 0 ]]; then
    sudo_cmd=()
elif command -v sudo >/dev/null 2>&1; then
    sudo_cmd=(sudo)
else
    sudo_cmd=()
fi

clone_or_update() {
    local repo_url="$1"
    local target_dir="$2"
    local ref="${3:-}"

    if [[ ! -d "$target_dir/.git" ]]; then
        git clone --depth=1 "$repo_url" "$target_dir"
    fi

    if [[ -n "$ref" ]]; then
        git -C "$target_dir" fetch --depth=1 origin "$ref"
        git -C "$target_dir" checkout --force FETCH_HEAD
    else
        git -C "$target_dir" pull --ff-only
    fi
}

if [[ $skip_apt -eq 0 ]]; then
    "${sudo_cmd[@]}" apt-get update
    "${sudo_cmd[@]}" apt-get install -y \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        git \
        libcurl4-openssl-dev \
        libpcre2-dev \
        libyaml-cpp-dev \
        pkg-config \
        python3 \
        python3-pip \
        rapidjson-dev \
        zlib1g-dev
fi

mkdir -p "$prefix" "$src_root"

toml11_dir="$src_root/toml11"
clone_or_update https://github.com/ToruNiina/toml11 "$toml11_dir" "v4.3.0"
cmake -S "$toml11_dir" -B "$toml11_dir/build" \
    -DCMAKE_BUILD_TYPE="$build_type" \
    -DCMAKE_CXX_STANDARD=11 \
    -DCMAKE_INSTALL_PREFIX="$prefix"
cmake --build "$toml11_dir/build" -j"$jobs"
cmake --install "$toml11_dir/build"

quickjspp_dir="$src_root/quickjspp"
clone_or_update https://github.com/ftk/quickjspp "$quickjspp_dir"
cmake -S "$quickjspp_dir" -B "$quickjspp_dir" -DCMAKE_BUILD_TYPE="$build_type"
cmake --build "$quickjspp_dir" --target quickjs -j"$jobs"
install -d "$prefix/lib/quickjs" "$prefix/include/quickjs"
install -m644 "$quickjspp_dir/quickjs/libquickjs.a" "$prefix/lib/quickjs/"
install -m644 "$quickjspp_dir/quickjs/quickjs.h" "$quickjspp_dir/quickjs/quickjs-libc.h" "$prefix/include/quickjs/"
install -m644 "$quickjspp_dir/quickjspp.hpp" "$prefix/include/"

libcron_dir="$src_root/libcron"
clone_or_update https://github.com/PerMalmberg/libcron "$libcron_dir"
git -C "$libcron_dir" submodule update --init
cmake -S "$libcron_dir" -B "$libcron_dir" -DCMAKE_BUILD_TYPE="$build_type" -DCMAKE_INSTALL_PREFIX="$prefix"
cmake --build "$libcron_dir" --target libcron install -j"$jobs"

cat <<EOF
Bootstrap complete.

Next steps:
  cmake --preset full-release
  cmake --build --preset full-release
EOF
