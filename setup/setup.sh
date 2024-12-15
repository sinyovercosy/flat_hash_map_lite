#!/usr/bin/env bash
set -euxo pipefail

cd "$(dirname "$0")"

sudo ./llvm.sh 18
sudo apt-get install libc++-18-dev
sudo dpkg -i bazelisk-amd64.deb
sudo dpkg -i bazel-compile-commands_0.14.0-focal_amd64.deb
