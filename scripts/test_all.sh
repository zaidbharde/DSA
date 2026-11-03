#!/usr/bin/env bash
# Compile and run each standalone example without leaving build products in the repository.
set -u
set -o pipefail

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd) || exit 1
BUILD=$(mktemp -d "${TMPDIR:-/tmp}/dsa-test.XXXXXX") || {
  echo "ERROR: could not create a temporary build directory" >&2
  exit 1
}
cleanup() { rm -rf -- "$BUILD"; }
trap cleanup EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

PASSED=0
FAILED=0
SKIPPED=0

record_result() {
  label=$1
  shift
  printf 'TEST: %s\n' "$label"
  if "$@"; then
    printf 'PASS: %s\n' "$label"
    PASSED=$((PASSED + 1))
  else
    status=$?
    printf 'FAIL: %s (exit %s)\n' "$label" "$status" >&2
    FAILED=$((FAILED + 1))
  fi
}

skip_toolchain() {
  printf 'SKIP: %s (missing toolchain: %s)\n' "$1" "$2"
  SKIPPED=$((SKIPPED + 1))
}

run_c() {
  source=$1
  output=$2
  if [ "$(basename "$source")" = "maximum_subarray.c" ]; then
    cc -std=c11 -Wall -Wextra -Wpedantic -Werror -DMAXIMUM_SUBARRAY_TESTING "$source" -o "$output" && "$output"
  else
    cc -std=c11 -Wall -Wextra -Wpedantic -Werror "$source" -o "$output" && "$output"
  fi
}

run_cpp() {
  source=$1
  output=$2
  if [ "$(basename "$source")" = "bellman_ford.cpp" ]; then
    c++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -DBELLMAN_FORD_SELF_TEST "$source" -o "$output" && "$output"
  else
    c++ -std=c++17 -Wall -Wextra -Wpedantic -Werror "$source" -o "$output" && "$output"
  fi
}

run_java() {
  source=$1
  output_dir=$2
  class_name=$(basename "$source" .java)
  mkdir -p "$output_dir" || return 1
  javac -Xlint:all -Werror -d "$output_dir" "$source" && java -ea -cp "$output_dir" "$class_name"
}

run_java_source() {
  source=$1
  java -ea "$source"
}

run_typescript() {
  source=$1
  output_dir=$2
  mkdir -p "$output_dir" || return 1
  tsc --strict --target ES2020 --module commonjs --outDir "$output_dir" "$source" && \
    node "$output_dir/$(basename "$source" .ts).js"
}

cd "$ROOT" || exit 1

# C
if command -v cc >/dev/null 2>&1; then
  for source in c/*.c; do
    [ -f "$source" ] || continue
    name=$(basename "$source" .c)
    record_result "$source" run_c "$ROOT/$source" "$BUILD/$name-c"
  done
else
  for source in c/*.c; do
    [ -f "$source" ] || continue
    skip_toolchain "$source" cc
  done
fi

# C++: existing root demonstrations and language-directory additions.
if command -v c++ >/dev/null 2>&1; then
  for source in ./*.cpp cpp/*.cpp; do
    [ -f "$source" ] || continue
    name=$(basename "$source" .cpp)
    record_result "$source" run_cpp "$ROOT/$source" "$BUILD/$name-cpp"
  done
else
  for source in ./*.cpp cpp/*.cpp; do
    [ -f "$source" ] || continue
    skip_toolchain "$source" c++
  done
fi

# Java examples are compiled independently to isolate generated classes. If
# javac is absent but the source-file launcher exists, use it for self-tests.
if command -v javac >/dev/null 2>&1 && command -v java >/dev/null 2>&1; then
  index=0
  for source in ./*.java java/*.java; do
    [ -f "$source" ] || continue
    index=$((index + 1))
    record_result "$source" run_java "$ROOT/$source" "$BUILD/java-$index"
  done
elif command -v java >/dev/null 2>&1; then
  for source in ./*.java java/*.java; do
    [ -f "$source" ] || continue
    record_result "$source (Java source launcher)" run_java_source "$ROOT/$source"
  done
else
  for source in ./*.java java/*.java; do
    [ -f "$source" ] || continue
    skip_toolchain "$source" java
  done
fi

# Python and JavaScript standalone demos.
if command -v python3 >/dev/null 2>&1; then
  for source in python/*.py; do
    [ -f "$source" ] || continue
    record_result "$source" python3 "$ROOT/$source"
  done
else
  for source in python/*.py; do
    [ -f "$source" ] || continue
    skip_toolchain "$source" python3
  done
fi

if command -v node >/dev/null 2>&1; then
  for source in javascript/*.js; do
    [ -f "$source" ] || continue
    record_result "$source" node "$ROOT/$source"
  done
else
  for source in javascript/*.js; do
    [ -f "$source" ] || continue
    skip_toolchain "$source" node
  done
fi

# TypeScript is type-checked strictly, then its emitted standalone demo is run.
if command -v tsc >/dev/null 2>&1 && command -v node >/dev/null 2>&1; then
  for source in typescript/*.ts; do
    [ -f "$source" ] || continue
    name=$(basename "$source" .ts)
    record_result "$source" run_typescript "$ROOT/$source" "$BUILD/ts-$name"
  done
else
  missing=tsc
  command -v tsc >/dev/null 2>&1 || missing=tsc
  command -v node >/dev/null 2>&1 || missing=node
  for source in typescript/*.ts; do
    [ -f "$source" ] || continue
    skip_toolchain "$source" "$missing"
  done
fi

# Rust
if command -v rustc >/dev/null 2>&1; then
  for source in rust/*.rs; do
    [ -f "$source" ] || continue
    name=$(basename "$source" .rs)
    record_result "$source" bash -c 'rustc --edition=2021 --test -D warnings "$1" -o "$2" && "$2"' _ "$ROOT/$source" "$BUILD/$name-rust"
  done
else
  for source in rust/*.rs; do
    [ -f "$source" ] || continue
    skip_toolchain "$source" rustc
  done
fi

printf '\nSummary: %s passed, %s failed, %s skipped for missing toolchains.\n' "$PASSED" "$FAILED" "$SKIPPED"
if [ "$FAILED" -ne 0 ]; then
  exit 1
fi
