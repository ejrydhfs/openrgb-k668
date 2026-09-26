#!/bin/sh
# Static (offline) verification of the Redragon K668WBO-RGB protocol implementation.
# Compares every packet the OpenRGB controller builds against the vendor
# KeyboardDrv.exe V1.6.6 wire layout, and the key->LED map against Cfg.ini.
set -e

DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BIN="${TMPDIR:-/tmp}/k668_proto_test"

g++ -std=gnu++17 -Wall -Wextra -I"$DIR/.." "$DIR/k668_proto_test.cpp" -o "$BIN"
exec "$BIN" "$@"
