#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${BUILD_DIR:-"$script_dir/build"}"
build_type="${BUILD_TYPE:-Release}"
jobs="${JOBS:-}"

detect_qt_prefix() {
  if [[ -n "${CMAKE_PREFIX_PATH:-}" ]]; then
    printf '%s\n' "$CMAKE_PREFIX_PATH"
    return 0
  fi

  if [[ -n "${QT_PREFIX_PATH:-}" ]]; then
    printf '%s\n' "$QT_PREFIX_PATH"
    return 0
  fi

  if command -v qtpaths6 >/dev/null 2>&1; then
    local qt_version qt_prefix
    qt_version="$(qtpaths6 --qt-version 2>/dev/null || true)"
    qt_prefix="$(qtpaths6 --query QT_INSTALL_PREFIX 2>/dev/null || true)"
    if [[ "$qt_version" == 6.11* && -n "$qt_prefix" ]]; then
      printf '%s\n' "$qt_prefix"
      return 0
    fi
  fi

  local root version_file candidate_prefix
  for root in "$HOME/Qt" "$HOME/.local/Qt" "/opt/Qt" "/opt/qt" "/usr/local/Qt"; do
    [[ -d "$root" ]] || continue
    while IFS= read -r version_file; do
      [[ -f "$version_file" ]] || continue
      if grep -q 'PACKAGE_VERSION "6\.11' "$version_file"; then
        candidate_prefix="$(dirname "$(dirname "$(dirname "$(dirname "$version_file")")")")"
        if [[ -d "$candidate_prefix" ]]; then
          printf '%s\n' "$candidate_prefix"
          return 0
        fi
      fi
    done < <(find "$root" -type f -path '*/lib/cmake/Qt6/Qt6ConfigVersionImpl.cmake' 2>/dev/null | sort -V)
  done

  return 1
}

cmake_prefix_path="${CMAKE_PREFIX_PATH:-${QT_PREFIX_PATH:-}}"
if [[ -z "$cmake_prefix_path" ]]; then
  if ! cmake_prefix_path="$(detect_qt_prefix)"; then
    cat >&2 <<'EOF'
Could not auto-detect a Qt 6.11 install.
Set QT_PREFIX_PATH or CMAKE_PREFIX_PATH to your Qt 6.11 kit prefix and try again.
EOF
    exit 1
  fi
fi

cmake_args=(
  -S "$script_dir"
  -B "$build_dir"
  -DCMAKE_BUILD_TYPE="$build_type"
)

if [[ -n "$cmake_prefix_path" ]]; then
  cmake_args+=(-DCMAKE_PREFIX_PATH="$cmake_prefix_path")
fi

cmake "${cmake_args[@]}"

build_args=(
  --build "$build_dir"
  --parallel
)

if [[ -n "$jobs" ]]; then
  build_args+=("$jobs")
fi

cmake "${build_args[@]}"
