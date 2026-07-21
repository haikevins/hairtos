#!/usr/bin/env sh
set -eu

ELF=${1:-build/00-baremetal-blink/hairtos_baremetal.elf}
arm-none-eabi-size "$ELF"
