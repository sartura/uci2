#!/bin/bash

set -e

CURR_DIR="$(cd "$(dirname "$0")" && pwd)"
cd ${CURR_DIR}/../src
flex uci2_cfg.l
bison -d uci2_cfg.y
echo "Lexer and Parser successfully generated."
