#!/bin/bash

set -e

CURR_DIR="$(cd "$(dirname "$0")" && pwd)"
cd ${CURR_DIR}/../src
flex uci2.l
bison -d uci2.y
echo "Lexer and Parser successfully generated."
