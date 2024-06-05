uci2
===========

uci2 is an Open Source library for parsing OpenWrt [UCI (Unified Configuration Interface)](https://openwrt.org/docs/guide-user/base-system/uci) files.
The parser is implemented using [Flex](https://github.com/westes/flex) and [Bison](https://www.gnu.org/software/bison/) and the result is an [Abstract Syntax Tree (AST)](https://en.wikipedia.org/wiki/Abstract_syntax_tree) representing the UCI file.

## Building and Installing

To build the library use the following commands:

```
mkdir build
cd build
cmake ..
make
```

To build the library with tests enabled use the following commands:

```
mkdir build
cd build
cmake -DENABLE_TESTS=ON ..
make
```

To install the library use the following command:

```
sudo make install
```

If you made changes to the `uci2_cfg.l` and/or `uci2_cfg.y` files you need to regenerate the lexer and parser code by running the script `scripts/gen_grammar.sh` which runs the relevant `flex` and `bison` commands and then you can rebuild and reinstall the library using the steps above.
