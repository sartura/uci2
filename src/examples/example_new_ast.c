// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright (C) 2019, Sartura Ltd.
//

#include <libuci2.h>

int main(int argc, char *argv[]) {
    int error = 0;
    uci2_ctx_t *ctx = NULL;
    uci2_n_t *node = NULL;

    ctx = uci2_parse_file(argv[1]);
    if (ctx == NULL) {
        error = -1;
        goto out;
    }

    node = uci2_q(ctx, "dhcp#3");
    if (node == NULL) {
        error = -1;
        goto out;
    }

    uci2_print(node);

out:
    uci2_free_ctx(ctx);

    return error;
}
