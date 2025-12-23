#!/bin/bash
set -e

xhost +local: >/dev/null 2>&1 || true

docker compose -p cert-monitor up --build

