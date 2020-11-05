// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright (C) 2019, Sartura Ltd.
//

#include <stdio.h>
#include <stdlib.h>
#include <libuci2.h>
#include <string.h>

int main(int argc, char **argv) {
    uci2_parser_ctx_t *ctx = uci2_new_ctx();
    // err check
    if (!ctx) {
        printf("Error while creating context!\n");
        return 1;
    }

    // configuration root node
    uci2_n_t *R = UCI2_CFG_ROOT(ctx);

    // add interface type
    uci2_n_t *n_if = uci2_add_T(ctx, R, "interface");

    // add loopback
    uci2_n_t *n_lo = uci2_add_S(ctx, n_if, "loopback");
    uci2_add_O(ctx, n_lo, "opt1", "val1");

    // add eth0
    uci2_n_t *n_eth0 = uci2_add_S(ctx, n_if, "eth0");
    uci2_add_O(ctx, n_eth0, "opt2", "val2");

    // add list to eth0
    uci2_n_t *n_lst = uci2_add_L(ctx, n_eth0, "allowed_ips");
    // add list items to allowed_ips list
    uci2_add_I(ctx, n_lst, "11.8.8.2/24");
    uci2_add_I(ctx, n_lst, "12.8.8.2/24");

    // iterate the list
    uci2_iter(n_lst, li) {
        // print out just for fun
        printf("List [%s], value = [%s]\n", n_lst->name, li->name);
    }
    printf("\n");

    // add global section (unnamed type)
    uci2_n_t *n_g = uci2_add_T(ctx, R, "globals");
    // add options to globals
    uci2_n_t *n_g_o = uci2_add_O(ctx, n_g, "opt_global", "global_val");
    uci2_change_value(n_g_o, "bla");
    // add another dummy to globals
    uci2_n_t *tbd_n = uci2_add_O(ctx, n_g, "opt_dummy", "tbd");
    // delete dummy option node
    uci2_del(tbd_n);

    // export context to stdout
    uci2_export_ctx(ctx, stdout);
    // free context
    uci2_free_ctx(ctx);
    // no error
    return 0;
}
