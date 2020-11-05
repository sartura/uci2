// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright (C) 2019, Sartura Ltd.
//

#include <stdio.h>
#include <stdlib.h>
#include <libuci2.h>

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    char *fp = argv[1];
    // create context for file
    uci2_parser_ctx_t *ctx = uci2_parse_file(fp);
    // err check
    if (!ctx) {
        printf("Error while loading file [%s]\n", fp);
        return 1;
    }
    // print config tree to stdout
    uci2_ast_print(stdout, ctx->ast, 0);
    // free context
    uci2_free_ctx(ctx);
    // no error
    return 0;
}
