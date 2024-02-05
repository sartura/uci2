#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <libuci2.h>
#include <string.h>

#define UNIT_TEST_DEBUG_LOG 0

// global parser context
uci2_ctx_t* g_ctx = NULL;
char* cfg_root;
size_t cfg_root_sz;
char *dropbear_banner_name = "BannerFile";
char *dropbear_banner_value = "/etc/banner";
char *dropbear_pwdauth_new_name = "PwdAuth";
char *dropbear_off_value = "off";
char *sys_server_list[] = {
	"0.openwrt.pool.ntp.org",
        "1.openwrt.pool.ntp.org",
        "2.openwrt.pool.ntp.org",
        "3.openwrt.pool.ntp.org"
};

bool validate_name_value_indx(uci2_n_t *parent_n, size_t *indx, char *p_nname, char *p_val);
bool test_validate_dhcp(uci2_ctx_t *ctx);
bool test_validate_dropbear(uci2_ctx_t *ctx);
bool test_validate_dropbear_with_save(uci2_ctx_t *ectx);
bool test_validate_system(uci2_ctx_t *ctx);
bool replicate_to_new_ctx(uci2_n_t *o_node, uci2_ctx_t *n_ctx, uci2_n_t *n_node);
bool test_validate_system_replicate(uci2_ctx_t *ctx);

static void test_cfg_dhcp(void **state) {
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + 6);
    strcpy(fp, cfg_root);
    strcat(fp, "dhcp");
    uci2_ctx_t *ctx = uci2_parse_file(fp);
    assert_ptr_not_equal(ctx, NULL);
    bool valid_flag = test_validate_dhcp(ctx);
    if (!valid_flag)
	    printf("\ndhcp validation failed!");
    assert_true(valid_flag);
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
    bool valid_flag = test_validate_dropbear(ctx);
    if (!valid_flag)
	    printf("\ndropbear validation failed!");
    assert_true(valid_flag);
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
    bool valid_flag = test_validate_system(ctx);
    if (!valid_flag)
	    printf("\nsystem validation failed!");
    assert_true(valid_flag);
    valid_flag = test_validate_system_replicate(ctx);
    if (!valid_flag)
	    printf("\nsystem replication validation failed!");
    assert_true(valid_flag);
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
    const char *unm_str = "unnamed";
    cfg_root[cfg_root_sz] = 0;
    char *fp = malloc(cfg_root_sz + strlen(unm_str) + 1);
    strcpy(fp, cfg_root);
    strcat(fp, unm_str);
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
    int ret = 0;
    // set cfg root
    cfg_root = strdup("./");
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
    ret = cmocka_run_group_tests(test_groups, group_setup, group_tearDown);

    if (cfg_root)
	    free(cfg_root);

    return ret;
}

bool validate_name_value_indx(uci2_ast_t *parent_n, size_t *indx, char *p_nname, char *p_val) {
	char *val, *nname;
	size_t i;

	i = *indx;*indx = i + 1;
	if (parent_n == NULL) {
	    	printf("\n%s:parent is NULL for index %ld and name %s !", __func__, i, p_nname);
		return false;
	}
#if UNIT_TEST_DEBUG_LOG
	printf("\n%s:processing for parent node %s, index %ld", __func__, uci2_get_name(parent_n), i);
	if(parent_n->parent != NULL)
		printf("\n%s:And it's parent node is %s", __func__, uci2_get_name(parent_n->parent));
#endif
	uci2_n_t *n =  parent_n->ch[i];
	if (n == NULL) {
	    	printf("\n%s:uci2_q_L(%ld) returns NULL for %s !", __func__, i, p_nname);
		return false;
	}
	nname = uci2_get_name(n);
	if (nname == NULL) {
	    	printf("\n%s:uci2_get_name() returns NULL for %s !", __func__, p_nname);
		return false;
	}
	if (strcmp(nname, p_nname) != 0) {
	    	printf("\n%s:name doesn't match for %s!", __func__, p_nname);
		return false;
	}
	val = uci2_get_value(n);
	if (val == NULL) {
	    	printf("\n%s:uci2_get_value() returns NULL doesn't match for %s!", __func__, p_nname);
		return false;
	}
	if (strcmp(val, p_val) != 0) {
	    	printf("\n%s:value %s doesn't match for %s!", __func__, p_val, p_nname);
		return false;
	}

	return true;
}

bool test_validate_dhcp(uci2_ctx_t *ctx) {
	uci2_ast_t* parent_n;
	uci2_n_t *n;
	char *val;
	size_t i = 0, ch_cnt;

	/* Validating dnsmasq section */
       	parent_n = uci2_q(ctx, "dnsmasq", "@[0]");
	if (parent_n == NULL) {
		return false;
	}
	ch_cnt = uci2_nc(parent_n);
#if UNIT_TEST_DEBUG_LOG
	printf("\n%s:Number of child nodes %ld", __func__, ch_cnt);
#endif
		
	i = 0;
	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "domainneeded", "1"))
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "boguspriv", "1"))
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "filterwin2k", "0"))
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "localise_queries", "1"))
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "rebind_protection", "1"))
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "rebind_localhost", "1"))
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "local", "/lan/"))
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "domain", "lan"))
		return false;

	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "expandhosts", "1"))
		return false;

	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "nonegcache", "0"))
		return false;

	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "authoritative", "1"))
		return false;

	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "readethers", "1"))
		return false;

	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "leasefile", "/tmp/dhcp.leases"))
		return false;

	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "resolvfile", "/tmp/resolv.conf.auto"))
		return false;

	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "nonwildcard", "1"))
		return false;

	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "localservice", "1"))
		return false;

	/* Validating dhcp.lan section */
       	n = uci2_q(ctx, "dhcp", "lan", "interface");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "lan") != 0) {
		printf("\n%s:failed for lan.interface", __func__);
		return false;
	}

       	n = uci2_q(ctx, "dhcp", "lan", "start");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "100") != 0) {
		printf("\n%s:failed for lan.start", __func__);
		return false;
	}

       	n = uci2_q(ctx, "dhcp", "lan", "limit");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "150") != 0) {
		printf("\n%s:failed for lan.limit", __func__);
		return false;
	}

       	n = uci2_q(ctx, "dhcp", "lan", "leasetime");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "12h") != 0) {
		printf("\n%s:failed for lan.leasetime", __func__);
		return false;
	}

       	n = uci2_q(ctx, "dhcp", "lan", "dhcpv6");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "server") != 0) {
		printf("\n%s:failed for lan.dhcpv6", __func__);
		return false;
	}

       	n = uci2_q(ctx, "dhcp", "lan", "ra");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "server") != 0) {
		printf("\n%s:failed for lan.ra", __func__);
		return false;
	}

	/* Validating dhcp.wan section */
       	n = uci2_q(ctx, "dhcp", "wan", "interface");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "wan") != 0) {
		printf("\n%s:failed for wan.ra", __func__);
		return false;
	}

       	n = uci2_q(ctx, "dhcp", "wan", "ignore");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "1") != 0) {
		printf("\n%s:failed for wan.ignore", __func__);
		return false;
	}

	/* Validating odhcpd.odhcpd section */
       	n = uci2_q(ctx, "odhcpd", "odhcpd", "maindhcp");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "0") != 0) {
		printf("\n%s:failed for odhcpd.maindhcp", __func__);
		return false;
	}

       	n = uci2_q(ctx, "odhcpd", "odhcpd", "leasefile");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "/tmp/hosts/odhcpd") != 0) {
		printf("\n%s:failed for odhcpd.leasefile", __func__);
		return false;
	}

       	n = uci2_q(ctx, "odhcpd", "odhcpd", "leasetrigger");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "/usr/sbin/odhcpd-update") != 0) {
		printf("\n%s:failed for odhcpd.leasetrigger", __func__);
		return false;
	}

       	n = uci2_q(ctx, "odhcpd", "odhcpd", "loglevel");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "4") != 0) {
		printf("\n%s:failed for odhcpd.loglevel", __func__);
		return false;
	}

	return true;
}

bool test_validate_dropbear(uci2_ctx_t *ctx) {
	uci2_ast_t* parent_n;
	size_t i = 0, ch_cnt;
	bool bool_val;

	/* Validating dropbear section */
       	parent_n = uci2_q(ctx, "dropbear", "@[0]");
	if (parent_n == NULL) {
		return false;
	}
	ch_cnt = uci2_nc(parent_n);
#if UNIT_TEST_DEBUG_LOG
	printf("\n%s:Number of child nodes %ld", __func__, ch_cnt);
#endif
		
	i = 0;
	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "PasswordAuth", "on"))
		return false;

	/* Change the name */
	uci2_change_name(parent_n->ch[i-1], dropbear_pwdauth_new_name);

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "RootPasswordAuth", "on"))
		return false;

	uci2_str2bool(uci2_get_value(parent_n->ch[i-1]), &bool_val);
	if (!bool_val)
		return false;

	/* Change the value */
	uci2_change_value(parent_n->ch[i-1], dropbear_off_value);

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "Port", "22"))
		return false;

	uci2_n_t *nn = uci2_add_O(ctx, parent_n, dropbear_banner_name, dropbear_banner_value);

	if (nn == NULL) {
		printf("\n%s:uci2_add_O() failed for %s", __func__, dropbear_banner_name);
		return false;
	}

       	if (!validate_name_value_indx(parent_n, &i, dropbear_banner_name, dropbear_banner_value))
		return false;

	if (!test_validate_dropbear_with_save(ctx))
		return false;

	uci2_del(nn);

	i--;
       	if (nn->parent != NULL && validate_name_value_indx(parent_n, &i, dropbear_banner_name, dropbear_banner_value))
		return false;

	return true;

}

bool test_validate_dropbear_with_save(uci2_ctx_t *ectx) {
	uci2_ast_t* parent_n;
	size_t i = 0, ch_cnt;
	bool ret_flag = false;
	char *cfg_file = "_tmp_dropbear";
	bool bool_val;

	cfg_root[cfg_root_sz] = 0;
	char *fp = malloc(cfg_root_sz + strlen(cfg_file) + 2);
	if (fp == NULL) 
		goto Exit_Val_DB_Save;

	strcpy(fp, cfg_root);
	strcat(fp, cfg_file);
	FILE *out = fopen(fp, "w");
	uci2_export_ctx(ectx, out);
	fclose(out);

	uci2_ctx_t *ctx = uci2_parse_file(fp);
	assert_ptr_not_equal(ctx, NULL);

	/* Validating dropbear section */
       	parent_n = uci2_q(ctx, "dropbear", "@[0]");
	if (parent_n == NULL) 
		goto Exit_Val_DB_Save;

	ch_cnt = uci2_nc(parent_n);
#if UNIT_TEST_DEBUG_LOG
	printf("\n%s:Number of child nodes %ld", __func__, ch_cnt);
#endif
		
	i = 0;
	/* if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "PasswordAuth", "on")) */
	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, dropbear_pwdauth_new_name, "on"))
		goto Exit_Val_DB_Save;

       	/* if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "RootPasswordAuth", "on")) */
       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "RootPasswordAuth", dropbear_off_value))
		goto Exit_Val_DB_Save;

	uci2_str2bool(uci2_get_value(parent_n->ch[i-1]), &bool_val);
	if (bool_val)
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "Port", "22"))
		goto Exit_Val_DB_Save;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, dropbear_banner_name, dropbear_banner_value))
		goto Exit_Val_DB_Save;


	ret_flag = true;

Exit_Val_DB_Save:
	if (ctx)
		uci2_free_ctx(ctx);
	if (fp)
		free(fp);

	return ret_flag;
}

bool test_validate_system(uci2_ctx_t *ctx) {
	uci2_ast_t* parent_n, *nl;
	uci2_n_t *n;
	size_t i = 0, ch_cnt;
	size_t sys_srv_list_sz;
	char *val;

	/* Validating system section */
       	parent_n = uci2_q(ctx, "system", "@[0]");
	if (parent_n == NULL) {
		return false;
	}
	ch_cnt = uci2_nc(parent_n);
#if UNIT_TEST_DEBUG_LOG
	printf("\n%s:Number of child nodes %ld", __func__, ch_cnt);
#endif
		
	i = 0;
	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "hostname", "OpenWrt"))
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "timezone", "UTC"))
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "ttylogin", "0"))
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "log_size", "64"))
		return false;

       	if (ch_cnt <= i || !validate_name_value_indx(parent_n, &i, "urandom_seed", "0"))
		return false;

	/* Validating timeserver section */
       	n = uci2_q(ctx, "timeserver", "ntp", "enabled");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "1") != 0) {
		printf("\n%s:failed for timeserver.ntp.enabled", __func__);
		return false;
	}

       	n = uci2_q(ctx, "timeserver", "ntp", "enable_server");
	val = uci2_get_value(n);
	if (val == NULL || strcmp(val, "0") != 0) {
		printf("\n%s:failed for timeserver.ntp.enable_server", __func__);
		return false;
	}

       	n = uci2_q(ctx, "timeserver", "ntp", "server");
	if (n == NULL) {
		printf("\n%s:uci2_q() returns NULL for timeserver.ntp.server", __func__);
		return false;
	}

	sys_srv_list_sz = sizeof(sys_server_list)/sizeof(sys_server_list[0]);
	for (size_t j =0;j < sys_srv_list_sz;j++) {
		nl = uci2_q_L(n, j);
		if (nl == NULL) {
			printf("\n%s:uci2_q_L() return NULL for timeserver.ntp.server[%ld]", __func__, j);
			break;
		}
		printf("\n%s:%ld.Name %s", __func__, j, uci2_get_name(nl));
		val = uci2_get_name(nl);
		if (val == NULL || strcmp(val, sys_server_list[j]) != 0) {
			printf("\n%s:failed for timeserver.ntp.server[%ld]", __func__, j);
			return false;
		}
	}

	return true;
}

bool replicate_to_new_ctx(uci2_n_t *o_node, uci2_ctx_t *n_ctx, uci2_n_t *n_node) {

	uci2_n_t *o_parent, *n_parent, *n, *nn;
	size_t i = 0, ch_cnt;

	o_parent = o_node;
	n_parent = n_node;
	ch_cnt = uci2_nc(o_parent);
#if UNIT_TEST_DEBUG_LOG
	printf("\n%s:Replicating the number of child nodes %ld", __func__, ch_cnt);
#endif
	for (i = 0; i<ch_cnt;i++) {
		n =  o_parent->ch[i];
		switch(n->nt) {
			case UCI2_NT_SECTION:
			case UCI2_NT_TYPE:
				nn = uci2_add_T(n_ctx, n_parent, uci2_get_name(n)); 
				if (nn == NULL || !replicate_to_new_ctx(n, n_ctx, nn))
					return false;
				break;
			case UCI2_NT_SECTION_NAME:
				nn = uci2_add_S(n_ctx, n_parent, uci2_get_name(n)); 
				if (nn == NULL || !replicate_to_new_ctx(n, n_ctx, nn))
					return false;
				break;

			case UCI2_NT_OPTION:
				nn = uci2_add_O(n_ctx, n_parent, uci2_get_name(n), uci2_get_value(n));
				if (nn == NULL)
					return false;
				break;

			case UCI2_NT_LIST:
				nn = uci2_add_L(n_ctx, n_parent, uci2_get_name(n));
				if (nn == NULL || !replicate_to_new_ctx(n, n_ctx, nn))
					return false;
				break;

			case UCI2_NT_LIST_ITEM:
				nn = uci2_add_I(n_ctx, n_parent, uci2_get_name(n));
				if (nn == NULL)
					return false;
				break;

			default:
				printf("\n%s:Error - type %d is not processed", __func__, n->nt);
				return false;

		}
	}

	return true;

}

bool test_validate_system_replicate(uci2_ctx_t *ctx) {
	uci2_ctx_t *nctx = NULL, *rctx = NULL;
	uci2_n_t *o_root, *n_root;
	bool ret_flag = false;
	char *fp = NULL;
	char *cfg_file = "_tmp_system";

	o_root = UCI2_CFG_ROOT(ctx);
	nctx = uci2_new_ctx();
	n_root = UCI2_CFG_ROOT(nctx);
	if (o_root == NULL || nctx == NULL || n_root == NULL)
		goto Exit_Door_Sys_Repl;
		
	if (!replicate_to_new_ctx(o_root, nctx, n_root))
		goto Exit_Door_Sys_Repl;

	cfg_root[cfg_root_sz] = 0;
	fp = malloc(cfg_root_sz + strlen(cfg_file) + 2);
	strcpy(fp, cfg_root);
	strcat(fp, cfg_file);
	uci2_export_ctx_fsync(nctx, fp);
	rctx = uci2_parse_file(fp);
	if (rctx == NULL)
		goto Exit_Door_Sys_Repl;

	ret_flag = test_validate_system(rctx);

Exit_Door_Sys_Repl:
	if (nctx)
    		uci2_free_ctx(nctx);
	if (rctx)
    		uci2_free_ctx(rctx);
	if (fp)
		free(fp);

	return ret_flag;
}
