#!/usr/bin/env bash
set -euo pipefail

if ! command -v apt-get >/dev/null 2>&1; then
  echo "This script supports Debian/Ubuntu via apt-get." >&2
  echo "Install libcurl dev packages for your distro, then re-run CMake." >&2
  exit 1
fi

sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build libcurl4-openssl-dev nlohmann-json3-dev
