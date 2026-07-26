#!/usr/bin/env sh
set -eu

ELF=${1:-build/01-baremetal-foundation/hairtos_baremetal.elf}
arm-none-eabi-size "$ELF"
