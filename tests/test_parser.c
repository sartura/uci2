#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <libuci2.h>
#include <string.h>

// global parser context
uci2_ctx_t* ctx = NULL;
char* cfg_root;
size_t cfg_root_sz;

static void test_cfg_dhcp(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 6);
    strcpy(fp, cfg_root);
    strcat(fp, "dhcp");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_free_ctx(ctx);
    free(fp);
}

static void test_cfg_dropbear(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 9);
    strcpy(fp, cfg_root);
    strcat(fp, "dropbear");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_free_ctx(ctx);
    free(fp);
}

static void test_cfg_firewall(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 9);
    strcpy(fp, cfg_root);
    strcat(fp, "dropbear");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_free_ctx(ctx);
    free(fp);
}

static void test_cfg_lxc_auto(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 9);
    strcpy(fp, cfg_root);
    strcat(fp, "lxc-auto");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_null(ctx);
    uci2_free_ctx(ctx);
    free(fp);
}

static void test_cfg_network(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 8);
    strcpy(fp, cfg_root);
    strcat(fp, "network");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_free_ctx(ctx);
    free(fp);
}

static void test_cfg_openvpn(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 8);
    strcpy(fp, cfg_root);
    strcat(fp, "openvpn");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_free_ctx(ctx);
    free(fp);
}

static void test_cfg_openvswitch(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 12);
    strcpy(fp, cfg_root);
    strcat(fp, "openvswitch");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_free_ctx(ctx);
    free(fp);
}

static void test_cfg_rpcd(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 5);
    strcpy(fp, cfg_root);
    strcat(fp, "rpcd");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_free_ctx(ctx);
    free(fp);
}

static void test_cfg_system(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 7);
    strcpy(fp, cfg_root);
    strcat(fp, "system");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_free_ctx(ctx);
    free(fp);
}

static void test_cfg_wireless(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 9);
    strcpy(fp, cfg_root);
    strcat(fp, "wireless");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_free_ctx(ctx);
    free(fp);
}




static int group_setup(void **state) {
    return 0;
}

static int group_tearDown(void **state) {
    return 0;
}

int main(int argc, char *argv[]) {
    // set cfg root
    cfg_root = strdup(argv[1]);
    cfg_root_sz = strlen(cfg_root);
    // define tests
    const struct CMUnitTest test_groups[] = {
        cmocka_unit_test(test_cfg_dhcp),
        cmocka_unit_test(test_cfg_dropbear),
        cmocka_unit_test(test_cfg_firewall),
        cmocka_unit_test(test_cfg_lxc_auto),
        cmocka_unit_test(test_cfg_network),
        cmocka_unit_test(test_cfg_openvpn),
        cmocka_unit_test(test_cfg_openvswitch),
        cmocka_unit_test(test_cfg_rpcd),
        cmocka_unit_test(test_cfg_system),
        cmocka_unit_test(test_cfg_wireless)};

    // results of group testing
    return cmocka_run_group_tests(test_groups, group_setup, group_tearDown);
}
