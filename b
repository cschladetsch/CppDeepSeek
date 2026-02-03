#!/usr/bin/env bash
set -euo pipefail

WITH_DEPS=0
WITH_MODELS=0

for arg in "$@"; do
  case "$arg" in
    --deps) WITH_DEPS=1 ;;
    --models) WITH_MODELS=1 ;;
    *)
      echo "Unknown option: $arg" >&2
      echo "Usage: ./b [--deps] [--models]" >&2
      exit 1
      ;;
  esac
done

if [[ "$WITH_DEPS" -eq 1 ]]; then
  bash scripts/install_deps.sh
fi

cmake -S . -B build \
  -DCPPDEEPSEEK_BUILD_TESTS=OFF \
  -DCPPDEEPSEEK_ALLOW_FETCHCONTENT=OFF \
  -DMODELSTORE_BUILD_TESTS=OFF \
  -DMODELSTORE_ALLOW_FETCHCONTENT=OFF

cmake --build build

if [[ "$WITH_MODELS" -eq 1 ]]; then
  cmake --build build --target ensure_models
fi
