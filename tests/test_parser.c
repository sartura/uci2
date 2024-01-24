#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <libuci2.h>
#include <string.h>

// global parser context
uci2_ctx_t* g_ctx = NULL;
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

static void test_cfg_unnamed(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 9);
    strcpy(fp, cfg_root);
    strcat(fp, "unnamed");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_ast_t *n = uci2_q(ctx, "rule", "@[1]");
    assert_ptr_not_equal(n, NULL);
    uci2_free_ctx(ctx);
    free(fp);
}
/*
NODE: @[0]
  |--> CHILD: name
  |--> CHILD: src
  |--> CHILD: proto
  |--> CHILD: dest_port
  |--> CHILD: target
  |--> CHILD: family
NODE: @[1]
  |--> CHILD: name
  |--> CHILD: src
  |--> CHILD: proto
  |--> CHILD: icmp_type
  |--> CHILD: family
  |--> CHILD: target
NODE: rule_X
  |--> CHILD: name
  |--> CHILD: src
  |--> CHILD: proto
  |--> CHILD: family
  |--> CHILD: target
NODE: @[2]
  |--> CHILD: src
  |--> CHILD: dest_port
  |--> CHILD: target
  |--> CHILD: proto
*/

static void test_iterators(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 9);
    strcpy(fp, cfg_root);
    strcat(fp, "unnamed");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_ast_t *n = uci2_q(ctx, "rule");
    char *lst[] = {"@[2]", "rule_X", "@[1]", "@[0]"};
    char *sub[4][6] = {
        {"name", "src", "proto", "dest_port", "target", "family"},
        {"name", "src", "proto", "icmp_type", "family", "target"},
        {"name", "src", "proto", "family", "target", NULL},
        {"src", "dest_port", "target", "proto", NULL, NULL}};

    // *************** reverse ************8
    // new iterator
    uci2_iter_t *it = UCI2_IT_NEW(n);
    it = UCI2_IT_LAST(it);
    int i = 0;
    // iterate
    for (; !UCI2_IT_BEGIN(it); it = UCI2_IT_PREV(it)) {
        // check
        if (i >= 4) fail();
        assert_string_equal(lst[i], UCI2_IT_NODE(it)->name);
        ++i;
    }
    // free iterator
    UCI2_IT_FREE(it);

    // *************** forward ************8
    // new iterator
    it = UCI2_IT_NEW(n);
    i = 0;
    // iterate
    for (; !UCI2_IT_END(it); it = UCI2_IT_NEXT(it)) {
        // check
        if (i >= 4) fail();
        assert_string_equal(lst[3 - i], UCI2_IT_NODE(it)->name);
        ++i;
    }
    // free iterator
    UCI2_IT_FREE(it);


    // *************** forward + sub-iterator ************8
    // new iterator
    it = UCI2_IT_NEW(n);
    i = 0;
    // iterate
    for (; !UCI2_IT_END(it); it = UCI2_IT_NEXT(it)) {
        // new sub-iterator
        uci2_iter_t *it2 = UCI2_IT_NEW(UCI2_IT_NODE(it));
        int j = 0;
        // iterate
        for (; !UCI2_IT_END(it2); it2 = UCI2_IT_NEXT(it2)) {
            assert_string_equal(sub[i][j], UCI2_IT_NODE(it2)->name);
            ++j;
        }
        // free sub-iterator
        UCI2_IT_FREE(it2);
        // check
        if (i >= 4) fail();
        ++i;
    }
    // free iterator
    UCI2_IT_FREE(it);

    // *************** reverse + sub-iterator ************8
    // new iterator
    it = UCI2_IT_NEW(n);
    it = UCI2_IT_LAST(it);
    i = 0;
    // iterate
    for (; !UCI2_IT_BEGIN(it); it = UCI2_IT_PREV(it)) {
        // new sub-iterator
        uci2_iter_t *it2 = UCI2_IT_NEW(UCI2_IT_NODE(it));
        int j = 0;
        // iterate
        for (; !UCI2_IT_END(it2); it2 = UCI2_IT_NEXT(it2)) {
            assert_string_equal(sub[3 - i][j], UCI2_IT_NODE(it2)->name);
            ++j;
        }
        // free sub-iterator
        UCI2_IT_FREE(it2);
        // check
        if (i >= 4) fail();
        ++i;
    }
    // free iterator
    UCI2_IT_FREE(it);
    uci2_free_ctx(ctx);
    free(fp);
}

static void test_modifiers(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 7);
    strcpy(fp, cfg_root);
    strcat(fp, "unnamed");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    uci2_ast_t *o = uci2_get_or_create_O(ctx, "name", "rule", "rule_X");
    assert_ptr_not_equal(o, NULL);
    assert_string_equal(o->value, "Allow-IGMP"); 
    o = uci2_get_or_create_O(ctx, "new_name", "rule", "rule_X");
    assert_ptr_not_equal(o, NULL);
    assert_ptr_equal(o->value, NULL); 
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
        cmocka_unit_test(test_cfg_unnamed),
        cmocka_unit_test(test_cfg_wireless),
        cmocka_unit_test(test_iterators),
        cmocka_unit_test(test_modifiers)};

    // results of group testing
    return cmocka_run_group_tests(test_groups, group_setup, group_tearDown);
}
