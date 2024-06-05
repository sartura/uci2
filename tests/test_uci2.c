#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <errno.h>

#include <cmocka.h>

#include "utils/debug.h"

#include "parser.h"
#include "lexer.h"
#include "uci2.h"

static int setup(void **state);
static int teardown(void **state);

static int file_copy(const char *destination, const char *source);

static void test_uci2_node_get(void **state);
static void test_uci2_node_section_add(void **state);
static void test_uci2_node_option_add(void **state);
static void test_uci2_node_list_add(void **state);
static void test_uci2_node_list_element_add(void **state);

static void test_uci2_node_type_get(void **state);

static void test_uci2_node_section_type_get(void **state);
static void test_uci2_node_section_type_set(void **state);
static void test_uci2_node_section_name_get(void **state);
static void test_uci2_node_section_name_set(void **state);

static void test_uci2_node_option_name_get(void **state);
static void test_uci2_node_option_name_set(void **state);
static void test_uci2_node_option_value_get(void **state);
static void test_uci2_node_option_value_set(void **state);

static void test_uci2_node_list_name_get(void **state);
static void test_uci2_node_list_name_set(void **state);

static void test_uci2_node_list_element_value_get(void **state);
static void test_uci2_node_list_element_value_set(void **state);
static void test_uci2_string_to_boolean(void **state);

static void test_uci2_config_parse_success(void **state);
static void test_uci2_config_parse_error(void **state);
static void test_uci2_config_file_change(void **state);
static void test_uci2_node_iterator(void **state);

static const char *source_test_file_path[] = {
	"test_config_correct",
	"test_config_incorrect",
	"test_config_iterator",
	NULL,
};

static const char *destination_test_file_path[] = {
	"/etc/config/test_config_correct",
	"/etc/config/test_config_incorrect",
	"/etc/config/test_config_iterator",
	"/etc/config/test_uci2_node_section_add_config",
	"/etc/config/test_uci2_node_option_add_config",
	"/etc/config/test_uci2_node_list_add_config",
	"/etc/config/test_uci2_node_list_element_add_config",
	"/etc/config/test_sync",
	NULL,
};

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_uci2_node_get),
		cmocka_unit_test(test_uci2_node_section_add),
		cmocka_unit_test(test_uci2_node_option_add),
		cmocka_unit_test(test_uci2_node_list_add),
		cmocka_unit_test(test_uci2_node_list_element_add),
		cmocka_unit_test(test_uci2_node_type_get),
		cmocka_unit_test(test_uci2_node_section_type_get),
		cmocka_unit_test(test_uci2_node_section_type_set),
		cmocka_unit_test(test_uci2_node_section_name_get),
		cmocka_unit_test(test_uci2_node_section_name_set),
		cmocka_unit_test(test_uci2_node_option_name_get),
		cmocka_unit_test(test_uci2_node_option_name_set),
		cmocka_unit_test(test_uci2_node_option_value_get),
		cmocka_unit_test(test_uci2_node_option_value_set),
		cmocka_unit_test(test_uci2_node_list_name_get),
		cmocka_unit_test(test_uci2_node_list_name_set),
		cmocka_unit_test(test_uci2_node_list_element_value_get),
		cmocka_unit_test(test_uci2_node_list_element_value_set),
		cmocka_unit_test(test_uci2_string_to_boolean),
		cmocka_unit_test(test_uci2_config_parse_success),
		cmocka_unit_test(test_uci2_config_parse_error),
		cmocka_unit_test(test_uci2_config_file_change),
		cmocka_unit_test(test_uci2_node_iterator),
	};

	return cmocka_run_group_tests(tests, setup, teardown);
}

static int setup(void **state)
{
	int error = 0;

	for (size_t i = 0; source_test_file_path[i] != NULL && destination_test_file_path[i] != NULL; i++) {
		error = file_copy(destination_test_file_path[i], source_test_file_path[i]);
		if (error) {
			fprintf(stderr, "file_copy error: ^\n");
			goto error_out;
		}
	}

	goto out;

error_out:
out:
	return error;
}

static int teardown(void **state)
{
	int error = 0;

	for (size_t i = 0; destination_test_file_path[i] != NULL; i++) {
		error = remove(destination_test_file_path[i]);
		if (error) {
			fprintf(stderr, "remove(%s) error (%d): %s\n", destination_test_file_path[i], error, strerror(error));
			goto error_out;
		}
	}

	goto out;

error_out:
out:
	return error;
}

static int file_copy(const char *destination, const char *source)
{
	int error = 0;
	FILE *source_file = NULL;
	FILE *destination_file = NULL;
	int character;

	source_file = fopen(source, "r");
	if (source_file == NULL) {
		fprintf(stderr, "failed to open source file: %s\n", source);
		error = -1;
		goto error_out;
	}

	destination_file = fopen(destination, "w");
	if (destination_file == NULL) {
		fprintf(stderr, "failed to open destination file: %s\n", destination);
		error = -1;
		goto error_out;
	}

	while ((character = fgetc(source_file)) != EOF) {
		fputc(character, destination_file);
	}

	goto out;

error_out:
out:
	if (source_file) {
		fclose(source_file);
	}

	if (destination_file) {
		fclose(destination_file);
	}

	return error;
}

static void test_uci2_node_get(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "enabled", &node);
	assert_int_equal(error, UE_NONE);

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_section_add(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *root_node = NULL;
	uci2_node_t *new_section_node = NULL;
	const char *section_type = NULL;
	const char *section_name = NULL;
	uci2_node_t *section_node = NULL;
	uci2_node_iterator_t *iterator = NULL;
	uci2_node_t *node_next = NULL;
	size_t index = 0;
	const char *section_type_match[] = {
		"system",
		"system",
		"timeserver",
		"new_section_type",
	};
	const char *section_name_match[] = {
		"@system[0]",
		"@system[1]",
		"ntp",
		"new_section_name",
	};

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, NULL, NULL, &root_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_add(uci2_ast, root_node, "system", NULL, &new_section_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_type_get(new_section_node, &section_type);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_type, "system");

	error = uci2_node_section_name_get(new_section_node, &section_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_name, "@system[1]");

	error = uci2_node_get(uci2_ast, "@system[1]", NULL, &section_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_type_get(section_node, &section_type);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_type, "system");

	error = uci2_node_section_name_get(section_node, &section_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_name, "@system[1]");

	error = uci2_node_section_add(uci2_ast, root_node, "new_section_type", "new_section_name", &new_section_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_type_get(new_section_node, &section_type);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_type, "new_section_type");

	error = uci2_node_section_name_get(new_section_node, &section_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_name, "new_section_name");

	error = uci2_node_get(uci2_ast, "new_section_name", NULL, &section_node);
	error = uci2_node_section_type_get(section_node, &section_type);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_type, "new_section_type");

	error = uci2_node_section_name_get(section_node, &section_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_name, "new_section_name");

	error = uci2_node_section_add(uci2_ast, root_node, "timeserver", "ntp", &new_section_node);
	assert_int_not_equal(error, UE_NONE);

	error = uci2_node_section_add(uci2_ast, root_node, "new_section_type", "new_section_name", &new_section_node);
	assert_int_not_equal(error, UE_NONE);

	error = uci2_ast_sync(uci2_ast, "test_uci2_node_section_add_config");
	assert_int_equal(error, UE_NONE);

	uci2_ast_destroy(&uci2_ast);

	error = uci2_config_parse("test_uci2_node_section_add_config", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, NULL, NULL, &root_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_iterator_new(root_node, &iterator);
	assert_int_equal(error, UE_NONE);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		error = uci2_node_section_type_get(node_next, &section_type);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_type, section_type_match[index]);

		error = uci2_node_section_name_get(node_next, &section_name);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_name, section_name_match[index]);

		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_option_add(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *root_node = NULL;
	uci2_node_t *new_section_node = NULL;
	uci2_node_t *new_option_node = NULL;
	const char *option_name = NULL;
	const char *option_value = NULL;
	const char *section_type = NULL;
	const char *section_name = NULL;
	uci2_node_t *option_node = NULL;
	uci2_node_t *section_node = NULL;
	uci2_node_iterator_t *iterator = NULL;
	uci2_node_t *node_next = NULL;
	uci2_node_type_e node_type = UNT_ROOT;
	size_t index = 0;
	const char *section_type_match[] = {
		"system",
		"timeserver",
	};
	const char *section_name_match[] = {
		"@system[0]",
		"ntp",
	};

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, NULL, NULL, &root_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_add(uci2_ast, root_node, "system", NULL, &new_section_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_type_get(new_section_node, &section_type);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_type, "system");

	error = uci2_node_section_name_get(new_section_node, &section_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_name, "@system[1]");

	error = uci2_node_option_add(uci2_ast, new_section_node, "system_option", "system_value", &new_option_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_option_name_get(new_option_node, &option_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(option_name, "system_option");

	error = uci2_node_option_value_get(new_option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(option_value, "system_value");

	error = uci2_node_get(uci2_ast, "@system[1]", "system_option", &option_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_option_name_get(option_node, &option_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(option_name, "system_option");

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(option_value, "system_value");

	error = uci2_node_option_add(uci2_ast, new_section_node, "system_option", "system_value", &new_option_node);
	assert_int_not_equal(error, UE_NONE);

	uci2_node_remove(new_section_node);

	error = uci2_node_option_add(uci2_ast, new_section_node, "new_option", "new_value", &new_option_node);
	assert_int_not_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", NULL, &section_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_iterator_new(section_node, &iterator);
	assert_int_equal(error, UE_NONE);

	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		error = uci2_node_type_get(node_next, &node_type);
		assert_int_equal(error, UE_NONE);

		if (node_type == UNT_OPTION) {
			error = uci2_node_option_name_get(node_next, &option_name);
			assert_int_equal(error, UE_NONE);

			if (strcmp(option_name, "enabled") == 0) {
				uci2_node_remove(node_next);
			}
		}
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_option_add(uci2_ast, section_node, "enabled", "false", &new_option_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_option_name_get(new_option_node, &option_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(option_name, "enabled");

	error = uci2_node_option_value_get(new_option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(option_value, "false");

	error = uci2_ast_sync(uci2_ast, "test_uci2_node_option_add_config");
	assert_int_equal(error, UE_NONE);

	uci2_ast_destroy(&uci2_ast);

	error = uci2_config_parse("test_uci2_node_option_add_config", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, NULL, NULL, &root_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_iterator_new(root_node, &iterator);
	assert_int_equal(error, UE_NONE);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		error = uci2_node_section_type_get(node_next, &section_type);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_type, section_type_match[index]);

		error = uci2_node_section_name_get(node_next, &section_name);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_name, section_name_match[index]);

		if (strcmp(section_name, "ntp") == 0) {
			error = uci2_node_get(uci2_ast, section_name, "enabled", &option_node);
			assert_int_equal(error, 0);

			error = uci2_node_option_name_get(option_node, &option_name);
			assert_int_equal(error, UE_NONE);
			assert_string_equal(option_name, "enabled");

			error = uci2_node_option_value_get(option_node, &option_value);
			assert_int_equal(error, UE_NONE);
			assert_string_equal(option_value, "false");
		}

		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_list_add(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *root_node = NULL;
	uci2_node_t *new_section_node = NULL;
	uci2_node_t *new_list_node = NULL;
	const char *list_name = NULL;
	const char *section_type = NULL;
	const char *section_name = NULL;
	uci2_node_t *list_node = NULL;
	uci2_node_t *section_node = NULL;
	uci2_node_iterator_t *iterator = NULL;
	uci2_node_t *node_next = NULL;
	uci2_node_type_e node_type = UNT_ROOT;
	size_t index = 0;
	const char *section_type_match[] = {
		"system",
		"timeserver",
	};
	const char *section_name_match[] = {
		"@system[0]",
		"ntp",
	};

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, NULL, NULL, &root_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_add(uci2_ast, root_node, "system", NULL, &new_section_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_type_get(new_section_node, &section_type);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_type, "system");

	error = uci2_node_section_name_get(new_section_node, &section_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_name, "@system[1]");

	error = uci2_node_list_add(uci2_ast, new_section_node, "system_list", &new_list_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_list_name_get(new_list_node, &list_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(list_name, "system_list");

	error = uci2_node_get(uci2_ast, "@system[1]", "system_list", &list_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_list_name_get(list_node, &list_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(list_name, "system_list");

	error = uci2_node_list_add(uci2_ast, new_section_node, "system_list", &new_list_node);
	assert_int_not_equal(error, UE_NONE);

	error = uci2_node_option_add(uci2_ast, new_section_node, "system_list", "system_value", &new_list_node);
	assert_int_not_equal(error, UE_NONE);

	uci2_node_remove(new_section_node);

	error = uci2_node_list_add(uci2_ast, new_section_node, "new_list", &new_list_node);
	assert_int_not_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", NULL, &section_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_list_add(uci2_ast, section_node, "server", &new_list_node);
	assert_int_not_equal(error, UE_NONE);

	error = uci2_node_iterator_new(section_node, &iterator);
	assert_int_equal(error, UE_NONE);

	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		error = uci2_node_type_get(node_next, &node_type);
		assert_int_equal(error, UE_NONE);

		if (node_type == UNT_LIST) {
			error = uci2_node_list_name_get(node_next, &list_name);
			assert_int_equal(error, UE_NONE);

			if (strcmp(list_name, "server") == 0) {
				uci2_node_remove(node_next);
			}
		}
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_list_add(uci2_ast, section_node, "server", &new_list_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_list_name_get(new_list_node, &list_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(list_name, "server");

	error = uci2_ast_sync(uci2_ast, "test_uci2_node_list_add_config");
	assert_int_equal(error, UE_NONE);

	uci2_ast_destroy(&uci2_ast);

	error = uci2_config_parse("test_uci2_node_list_add_config", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, NULL, NULL, &root_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_iterator_new(root_node, &iterator);
	assert_int_equal(error, UE_NONE);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		error = uci2_node_section_type_get(node_next, &section_type);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_type, section_type_match[index]);

		error = uci2_node_section_name_get(node_next, &section_name);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_name, section_name_match[index]);

		if (strcmp(section_name, "ntp") == 0) {
			error = uci2_node_get(uci2_ast, section_name, "server", &list_node);
			assert_int_equal(error, 0);

			error = uci2_node_list_name_get(list_node, &list_name);
			assert_int_equal(error, UE_NONE);
			assert_string_equal(list_name, "server");
		}

		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_list_element_add(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *root_node = NULL;
	uci2_node_t *new_section_node = NULL;
	uci2_node_t *new_list_node = NULL;
	const char *list_name = NULL;
	uci2_node_t *new_list_element_node_1 = NULL;
	uci2_node_t *new_list_element_node_2 = NULL;
	const char *list_element_value = NULL;
	const char *section_type = NULL;
	const char *section_name = NULL;
	uci2_node_t *list_node = NULL;
	uci2_node_iterator_t *iterator = NULL;
	uci2_node_t *node_next = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, NULL, NULL, &root_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_add(uci2_ast, root_node, "system", NULL, &new_section_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_type_get(new_section_node, &section_type);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_type, "system");

	error = uci2_node_section_name_get(new_section_node, &section_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(section_name, "@system[1]");

	error = uci2_node_list_add(uci2_ast, new_section_node, "system_list", &new_list_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_list_name_get(new_list_node, &list_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(list_name, "system_list");

	error = uci2_node_list_element_add(uci2_ast, new_list_node, "list_element_1", &new_list_element_node_1);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_list_element_add(uci2_ast, new_list_node, "list_element_2", &new_list_element_node_2);
	assert_int_equal(error, UE_NONE);

	uci2_node_remove(new_list_element_node_1);

	error = uci2_node_get(uci2_ast, "@system[1]", "system_list", &list_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_list_name_get(list_node, &list_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(list_name, "system_list");

	error = uci2_node_iterator_new(list_node, &iterator);
	assert_int_equal(error, UE_NONE);

	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		error = uci2_node_list_element_value_get(node_next, &list_element_value);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(list_element_value, "list_element_2");
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_get(uci2_ast, "ntp", "server", &list_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_iterator_new(list_node, &iterator);
	assert_int_equal(error, UE_NONE);

	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		uci2_node_remove(node_next);
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_ast_sync(uci2_ast, "test_uci2_node_list_element_add_config");
	assert_int_equal(error, UE_NONE);

	uci2_ast_destroy(&uci2_ast);

	error = uci2_config_parse("test_uci2_node_list_element_add_config", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "@system[1]", "system_list", &list_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_iterator_new(list_node, &iterator);
	assert_int_equal(error, UE_NONE);

	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		error = uci2_node_list_element_value_get(node_next, &list_element_value);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(list_element_value, "list_element_2");
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_get(uci2_ast, "ntp", "server", &list_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_list_name_get(list_node, &list_name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(list_name, "server");

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_type_get(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	uci2_node_type_e node_type = UNT_ROOT;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "enabled", &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_type_get(node, &node_type);
	assert_int_equal(error, UE_NONE);
	assert_int_equal(node_type, UNT_OPTION);

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_section_type_get(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	const char *type = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", NULL, &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_type_get(node, &type);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(type, "timeserver");

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_section_type_set(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	const char *type = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", NULL, &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_type_get(node, &type);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(type, "timeserver");

	error = uci2_node_section_type_set(node, "test_type");
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_type_get(node, &type);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(type, "test_type");

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_section_name_get(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	const char *name = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", NULL, &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_name_get(node, &name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(name, "ntp");

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_section_name_set(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	const char *name = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", NULL, &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_name_get(node, &name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(name, "ntp");

	error = uci2_node_section_name_set(node, "test_name");
	assert_int_equal(error, UE_NONE);

	error = uci2_node_section_name_get(node, &name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(name, "test_name");

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_option_name_get(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	const char *name = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "enabled", &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_option_name_get(node, &name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(name, "enabled");

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_option_name_set(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	const char *name = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "enabled", &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_option_name_get(node, &name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(name, "enabled");

	error = uci2_node_option_name_set(node, "test_name");
	assert_int_equal(error, UE_NONE);

	error = uci2_node_option_name_get(node, &name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(name, "test_name");

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_option_value_get(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	const char *value = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "enabled", &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_option_value_get(node, &value);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(value, "true");

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_option_value_set(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	const char *value = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "enabled", &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_option_value_get(node, &value);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(value, "true");

	error = uci2_node_option_value_set(node, "test_value");
	assert_int_equal(error, UE_NONE);

	error = uci2_node_option_value_get(node, &value);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(value, "test_value");

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_list_name_get(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	const char *name = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "server", &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_list_name_get(node, &name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(name, "server");

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_list_name_set(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	const char *name = NULL;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "server", &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_list_name_get(node, &name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(name, "server");

	error = uci2_node_list_name_set(node, "test_name");
	assert_int_equal(error, UE_NONE);

	error = uci2_node_list_name_get(node, &name);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(name, "test_name");

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_list_element_value_get(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	uci2_node_iterator_t *iterator = NULL;
	size_t index = 0;
	uci2_node_t *node_next = NULL;
	const char *value = NULL;
	static const char *list_element_value_match[] = {
		"0.openwrt.pool.ntp.org",
		"1.openwrt.pool.ntp.org",
		"2.openwrt.pool.ntp.org",
		"3.openwrt.pool.ntp.org",
	};

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "server", &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_iterator_new(node, &iterator);
	assert_int_equal(error, UE_NONE);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		error = uci2_node_list_element_value_get(node_next, &value);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(value, list_element_value_match[index]);
		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_list_element_value_set(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	uci2_node_iterator_t *iterator = NULL;
	size_t index = 0;
	uci2_node_t *node_next = NULL;
	const char *value = NULL;
	static const char *list_element_value_match[] = {
		"0.openwrt.pool.ntp.org",
		"1.openwrt.pool.ntp.org",
		"2.openwrt.pool.ntp.org",
		"3.openwrt.pool.ntp.org",
	};
	static const char *list_element_value_new[] = {
		"test_0",
		"test_1",
		"test_2",
		"test_3",
	};

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "server", &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_iterator_new(node, &iterator);
	assert_int_equal(error, UE_NONE);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		error = uci2_node_list_element_value_get(node_next, &value);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(value, list_element_value_match[index]);
		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_iterator_new(node, &iterator);
	assert_int_equal(error, UE_NONE);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		error = uci2_node_list_element_value_set(node_next, list_element_value_new[index]);
		assert_int_equal(error, UE_NONE);
		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_iterator_new(node, &iterator);
	assert_int_equal(error, UE_NONE);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == UE_NONE) {
		error = uci2_node_list_element_value_get(node_next, &value);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(value, list_element_value_new[index]);
		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_string_to_boolean(void **state)
{
	uci2_error_e error = 0;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *node = NULL;
	const char *value = NULL;
	bool boolean_value = false;

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "enabled", &node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_option_value_get(node, &value);
	assert_int_equal(error, UE_NONE);
	assert_string_equal(value, "true");

	error = uci2_string_to_boolean(value, &boolean_value);
	assert_int_equal(error, UE_NONE);
	assert_true(boolean_value);

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_config_parse_success(void **state)
{
	uci2_error_e error = UE_NONE;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *root_node = NULL;
	uci2_node_iterator_t *iterator = NULL;
	uci2_node_t *node_next = NULL;
	uci2_node_type_e node_type = UNT_ROOT;
	const char *section_type_match[] = {
		"system",
		"timeserver",
	};
	const char *section_name_match[] = {
		"@system[0]",
		"ntp",
	};
	size_t index = 0;
	uci2_node_t *section_node = NULL;
	const char *section_type = NULL;
	const char *section_name = NULL;
	uci2_node_t *option_node = NULL;
	const char *option_name = NULL;
	const char *option_value = NULL;
	bool boolean_value = false;
	uci2_node_t *list_node = NULL;
	const char *list_name = NULL;
	const char *list_element_value = NULL;
	const char *system_list_element_value_match[] = {
		"10.9.0.0 255.255.255.252",
		"192.168.100.0 255.255.255.0",
	};
	const char *ntp_list_element_value_match[] = {
		"0.openwrt.pool.ntp.org",
		"1.openwrt.pool.ntp.org",
		"2.openwrt.pool.ntp.org",
		"3.openwrt.pool.ntp.org",
	};

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(uci2_ast, NULL);

	error = uci2_node_get(uci2_ast, NULL, NULL, &root_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(root_node, NULL);

	error = uci2_node_iterator_new(root_node, &iterator);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(iterator, NULL);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == 0) {
		assert_ptr_not_equal(node_next, NULL);
		error = uci2_node_type_get(node_next, &node_type);
		assert_int_equal(error, UE_NONE);
		assert_int_equal(node_type, UNT_SECTION);

		error = uci2_node_section_type_get(node_next, &section_type);
		assert_int_equal(error, UE_NONE);
		assert_ptr_not_equal(section_type, NULL);
		assert_string_equal(section_type, section_type_match[index]);

		error = uci2_node_section_name_get(node_next, &section_name);
		assert_int_equal(error, UE_NONE);
		assert_ptr_not_equal(section_name, NULL);
		assert_string_equal(section_name, section_name_match[index]);

		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_get(uci2_ast, "@system[0]", NULL, &section_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(section_node, NULL);

	error = uci2_node_section_type_get(section_node, &section_type);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(section_type, NULL);
	assert_string_equal(section_type, "system");

	error = uci2_node_section_name_get(section_node, &section_name);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(section_name, NULL);
	assert_string_equal(section_name, "@system[0]");

	error = uci2_node_get(uci2_ast, "@system[0]", "hostname", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_name_get(option_node, &option_name);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_name, NULL);
	assert_string_equal(option_name, "hostname");

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_value, NULL);
	assert_string_equal(option_value, "OpenWrt");

	error = uci2_node_get(uci2_ast, "@system[0]", "timezone", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_value, NULL);
	assert_string_equal(option_value, "UTC");

	error = uci2_node_get(uci2_ast, "@system[0]", "ttylogin", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_value, NULL);
	assert_string_equal(option_value, "1");
	error = uci2_string_to_boolean(option_value, &boolean_value);
	assert_int_equal(error, UE_NONE);
	assert_true(boolean_value);

	error = uci2_node_get(uci2_ast, "@system[0]", "log_size", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_value, NULL);
	assert_string_equal(option_value, "64");

	error = uci2_node_get(uci2_ast, "@system[0]", "log_port", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_value, NULL);
	assert_string_equal(option_value, "514");

	error = uci2_node_get(uci2_ast, "@system[0]", "urandom_seed", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_value, NULL);
	assert_string_equal(option_value, "0");
	error = uci2_string_to_boolean(option_value, &boolean_value);
	assert_int_equal(error, UE_NONE);
	assert_false(boolean_value);

	error = uci2_node_get(uci2_ast, "@system[0]", "route", &list_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(list_node, NULL);

	error = uci2_node_list_name_get(list_node, &list_name);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(list_name, NULL);
	assert_string_equal(list_name, "route");

	error = uci2_node_iterator_new(list_node, &iterator);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(iterator, NULL);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == 0) {
		assert_ptr_not_equal(node_next, NULL);
		error = uci2_node_list_element_value_get(node_next, &list_element_value);
		assert_int_equal(error, UE_NONE);
		assert_ptr_not_equal(list_element_value, NULL);
		assert_string_equal(list_element_value, system_list_element_value_match[index]);

		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_get(uci2_ast, "ntp", NULL, &section_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(section_node, NULL);

	error = uci2_node_get(uci2_ast, "ntp", "enabled", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_value, NULL);
	assert_string_equal(option_value, "true");
	error = uci2_string_to_boolean(option_value, &boolean_value);
	assert_int_equal(error, UE_NONE);
	assert_true(boolean_value);

	error = uci2_node_get(uci2_ast, "ntp", "enable_server", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_value, NULL);
	assert_string_equal(option_value, "0");
	error = uci2_string_to_boolean(option_value, &boolean_value);
	assert_int_equal(error, UE_NONE);
	assert_false(boolean_value);

	error = uci2_node_get(uci2_ast, "ntp", "server", &list_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(list_node, NULL);

	error = uci2_node_list_name_get(list_node, &list_name);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(list_name, NULL);
	assert_string_equal(list_name, "server");

	error = uci2_node_iterator_new(list_node, &iterator);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(iterator, NULL);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == 0) {
		assert_ptr_not_equal(node_next, NULL);
		error = uci2_node_list_element_value_get(node_next, &list_element_value);
		assert_int_equal(error, UE_NONE);
		assert_ptr_not_equal(list_element_value, NULL);
		assert_string_equal(list_element_value, ntp_list_element_value_match[index]);

		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_config_parse_error(void **state)
{
	uci2_error_e error = UE_NONE;
	uci2_ast_t *uci2_ast = NULL;

	error = uci2_config_parse("test_config_incorrect", &uci2_ast);
	assert_int_not_equal(error, 0);
	assert_ptr_equal(uci2_ast, NULL);
}

static void test_uci2_config_file_change(void **state)
{
	uci2_error_e error = UE_NONE;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *option_node = NULL;
	const char *option_value = NULL;
	bool boolean_value = false;
	uci2_node_t *list_node = NULL;
	uci2_node_t *list_element_node = NULL;
	uci2_node_iterator_t *iterator = NULL;
	size_t index = 0;
	uci2_node_t *node_next = NULL;
	const char *list_element_value = NULL;
	const char *list_element_value_match[] = {
		"0.openwrt.pool.ntp.org",
		"1.openwrt.pool.ntp.org",
		"2.openwrt.pool.ntp.org",
		"3.openwrt.pool.ntp.org",
		"new_value",
	};
	uci2_node_t *section_ntp_node = NULL;
	uci2_node_type_e node_type = UNT_ROOT;
	const char *option_name = NULL;
	const char *list_name = NULL;
	uci2_node_t *section_system_node = NULL;
	uci2_node_t *root_node = NULL;
	uci2_node_t *new_section_node = NULL;
	uci2_node_t *new_option_node = NULL;
	uci2_node_t *new_list_node = NULL;
	uci2_node_t *new_list_element_node = NULL;
	const char *section_type = NULL;
	const char *section_name = NULL;
	const char *section_type_match[] = {
		"timeserver",
		"new_section_type",
	};
	const char *section_name_match[] = {
		"ntp",
		"new_section_name",
	};
	uci2_node_iterator_t *list_iterator = NULL;
	const char *new_list_element_value_match[] = {
		"new_list_element_value",
	};

	error = uci2_config_parse("test_config_correct", &uci2_ast);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(uci2_ast, NULL);

	error = uci2_node_get(uci2_ast, "ntp", "enable_server", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_value_set(option_node, "1");
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "enable_server", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_value, NULL);
	assert_string_equal(option_value, "1");
	error = uci2_string_to_boolean(option_value, &boolean_value);
	assert_int_equal(error, UE_NONE);
	assert_true(boolean_value);

	error = uci2_node_get(uci2_ast, "ntp", "server", &list_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(list_node, NULL);

	error = uci2_node_list_element_add(uci2_ast, list_node, "new_value", &list_element_node);
	assert_int_equal(error, UE_NONE);

	error = uci2_node_get(uci2_ast, "ntp", "server", &list_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(list_node, NULL);

	error = uci2_node_iterator_new(list_node, &iterator);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(iterator, NULL);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == 0) {
		assert_ptr_not_equal(node_next, NULL);
		error = uci2_node_list_element_value_get(node_next, &list_element_value);
		assert_int_equal(error, UE_NONE);
		assert_ptr_not_equal(list_element_value, NULL);
		assert_string_equal(list_element_value, list_element_value_match[index]);

		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_get(uci2_ast, "ntp", "enabled", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	uci2_node_remove(option_node);

	error = uci2_node_get(uci2_ast, "ntp", "enabled", &option_node);
	assert_int_not_equal(error, 0);

	error = uci2_node_get(uci2_ast, "ntp", NULL, &section_ntp_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(section_ntp_node, NULL);
	error = uci2_node_iterator_new(section_ntp_node, &iterator);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(iterator, NULL);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == 0) {
		error = uci2_node_type_get(node_next, &node_type);
		assert_int_equal(error, UE_NONE);
		switch (node_type) {
			case UNT_OPTION:
				error = uci2_node_option_name_get(node_next, &option_name);
				assert_int_equal(error, UE_NONE);
				assert_string_equal(option_name, "enable_server");
				break;

			case UNT_LIST:
				error = uci2_node_list_name_get(node_next, &list_name);
				assert_int_equal(error, UE_NONE);
				assert_string_equal(list_name, "server");
				break;

			default:
				abort();
		}
		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_get(uci2_ast, "@system[0]", NULL, &section_system_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(section_system_node, NULL);

	uci2_node_remove(section_system_node);

	error = uci2_node_get(uci2_ast, "@system[0]", NULL, &section_system_node);
	assert_int_not_equal(error, 0);

	error = uci2_node_get(uci2_ast, NULL, NULL, &root_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(root_node, NULL);

	error = uci2_node_section_add(uci2_ast, root_node, "new_section_type", "new_section_name", &new_section_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(new_section_node, NULL);

	error = uci2_node_option_add(uci2_ast, new_section_node, "new_option_name", "new_option_value", &new_option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(new_option_node, NULL);

	error = uci2_node_list_add(uci2_ast, new_section_node, "new_list_name", &new_list_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(new_list_node, NULL);

	error = uci2_node_list_element_add(uci2_ast, new_list_node, "new_list_element_value", &new_list_element_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(new_list_element_node, NULL);

	error = uci2_node_iterator_new(root_node, &iterator);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(iterator, NULL);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == 0) {
		error = uci2_node_type_get(node_next, &node_type);
		assert_int_equal(error, UE_NONE);
		assert_int_equal(node_type, UNT_SECTION);
		error = uci2_node_section_type_get(node_next, &section_type);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_type, section_type_match[index]);
		error = uci2_node_section_name_get(node_next, &section_name);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_name, section_name_match[index]);
		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_iterator_new(new_section_node, &iterator);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(iterator, NULL);

	while (uci2_node_iterator_next(iterator, &node_next) == 0) {
		error = uci2_node_type_get(node_next, &node_type);
		assert_int_equal(error, UE_NONE);
		switch (node_type) {
			case UNT_OPTION:
				error = uci2_node_option_name_get(node_next, &option_name);
				assert_int_equal(error, UE_NONE);
				assert_string_equal(option_name, "new_option_name");

				error = uci2_node_option_value_get(node_next, &option_value);
				assert_int_equal(error, UE_NONE);
				assert_string_equal(option_value, "new_option_value");
				break;

			case UNT_LIST:
				error = uci2_node_list_name_get(node_next, &list_name);
				assert_int_equal(error, UE_NONE);
				assert_string_equal(list_name, "new_list_name");

				error = uci2_node_iterator_new(node_next, &list_iterator);
				assert_int_equal(error, UE_NONE);
				assert_ptr_not_equal(list_iterator, NULL);

				while (uci2_node_iterator_next(list_iterator, &list_element_node) == 0) {
					error = uci2_node_list_element_value_get(list_element_node, &list_element_value);
					assert_int_equal(error, UE_NONE);
					assert_string_equal(list_element_value, "new_list_element_value");
				}

				uci2_node_iterator_destroy(&list_iterator);
				break;

			default:
				abort();
		}
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_ast_sync(uci2_ast, "test_sync");
	assert_int_equal(error, UE_NONE);

	uci2_ast_destroy(&uci2_ast);

	// parse the file again after writing it to test_sync

	error = uci2_config_parse("test_sync", &uci2_ast);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(uci2_ast, NULL);

	error = uci2_node_get(uci2_ast, NULL, NULL, &root_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(root_node, NULL);

	error = uci2_node_iterator_new(root_node, &iterator);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(iterator, NULL);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == 0) {
		error = uci2_node_section_type_get(node_next, &section_type);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_type, section_type_match[index]);
		error = uci2_node_section_name_get(node_next, &section_name);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_name, section_name_match[index]);
		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_get(uci2_ast, "ntp", "enable_server", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_value, NULL);
	assert_string_equal(option_value, "1");
	error = uci2_string_to_boolean(option_value, &boolean_value);
	assert_int_equal(error, UE_NONE);
	assert_true(boolean_value);

	error = uci2_node_get(uci2_ast, "ntp", "server", &list_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(list_node, NULL);

	error = uci2_node_list_name_get(list_node, &list_name);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(list_name, NULL);
	assert_string_equal(list_name, "server");

	error = uci2_node_iterator_new(list_node, &iterator);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(iterator, NULL);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == 0) {
		assert_ptr_not_equal(node_next, NULL);
		error = uci2_node_list_element_value_get(node_next, &list_element_value);
		assert_int_equal(error, UE_NONE);
		assert_ptr_not_equal(list_element_value, NULL);
		assert_string_equal(list_element_value, list_element_value_match[index]);

		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	error = uci2_node_get(uci2_ast, "new_section_name", "new_option_name", &option_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_node, NULL);

	error = uci2_node_option_value_get(option_node, &option_value);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(option_value, NULL);
	assert_string_equal(option_value, "new_option_value");

	error = uci2_node_get(uci2_ast, "new_section_name", "new_list_name", &list_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(list_node, NULL);

	error = uci2_node_list_name_get(list_node, &list_name);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(list_name, NULL);
	assert_string_equal(list_name, "new_list_name");

	error = uci2_node_iterator_new(list_node, &iterator);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(iterator, NULL);

	index = 0;
	while (uci2_node_iterator_next(iterator, &node_next) == 0) {
		assert_ptr_not_equal(node_next, NULL);
		error = uci2_node_list_element_value_get(node_next, &list_element_value);
		assert_int_equal(error, UE_NONE);
		assert_ptr_not_equal(list_element_value, NULL);
		assert_string_equal(list_element_value, new_list_element_value_match[index]);

		index++;
	}

	uci2_node_iterator_destroy(&iterator);

	uci2_ast_destroy(&uci2_ast);
}

static void test_uci2_node_iterator(void **state)
{
	uci2_error_e error = UE_NONE;
	uci2_ast_t *uci2_ast = NULL;
	uci2_node_t *root_node = NULL;
	uci2_node_iterator_t *section_iterator = NULL;
	uci2_node_t *section_next = NULL;
	size_t section_index = 0;
	uci2_node_type_e section_node_type = UNT_ROOT;
	const char *section_type = NULL;
	const char *section_name = NULL;
	const char *section_type_match[] = {
		"rule",
		"rule",
		"rule",
		"rule",
		"rule2",
		"rule2",
	};
	const char *section_name_match[] = {
		"@rule[0]",
		"@rule[1]",
		"rule_X",
		"@rule[2]",
		"@rule2[0]",
		"@rule2[1]",
	};
	uci2_node_iterator_t *option_iterator = NULL;
	uci2_node_t *option_next = NULL;
	size_t option_index = 0;
	uci2_node_type_e option_node_type = UNT_ROOT;
	const char *option_name = NULL;
	const char *option_value = NULL;
	const char *option_0_name_match[] = {
		"name",
		"src",
		"proto",
		"dest_port",
		"target",
		"family",
	};
	const char *option_0_value_match[] = {
		"Allow-DHCP-Renew",
		"wan",
		"udp",
		"68",
		"ACCEPT",
		"ipv4",
	};
	const char *option_1_name_match[] = {
		"name",
		"src",
		"proto",
		"icmp_type",
		"family",
		"target",
	};
	const char *option_1_value_match[] = {
		"Allow-Ping",
		"wan",
		"icmp",
		"echo-request",
		"ipv4",
		"ACCEPT",
	};
	const char *option_2_name_match[] = {
		"name",
		"src",
		"proto",
		"family",
		"target",
	};
	const char *option_2_value_match[] = {
		"Allow-IGMP",
		"wan",
		"igmp",
		"ipv4",
		"ACCEPT",
	};
	const char *option_3_name_match[] = {
		"src",
		"dest_port",
		"target",
		"proto",
	};
	const char *option_3_value_match[] = {
		"wan",
		"51820",
		"ACCEPT",
		"udp",
	};
	const char *option_4_name_match[] = {
		"name",
		"src",
		"proto",
		"src_ip",
		"dest_ip",
		"dest_port",
		"family",
		"target",
	};
	const char *option_4_value_match[] = {
		"Allow-DHCPv6",
		"wan",
		"udp",
		"fc00::/6",
		"fc00::/6",
		"546",
		"ipv6",
		"ACCEPT",
	};
	const char *option_5_name_match[] = {
		"name",
		"src",
		"proto",
		"src_ip",
		"icmp_type",
		"family",
		"target",
	};
	const char *option_5_value_match[] = {
		"Allow-MLD",
		"wan",
		"icmp",
		"fe80::/10",
		"",
		"ipv6",
		"ACCEPT",
	};
	const char *list_name = NULL;
	uci2_node_iterator_t *list_element_iterator = NULL;
	uci2_node_t *list_element_next = NULL;
	size_t list_element_index = 0;
	uci2_node_type_e list_element_node_type = UNT_ROOT;
	const char *list_element_value = NULL;
	const char *list_element_value_match[] = {
		"130/0",
		"131/0",
		"132/0",
		"143/0",
	};

	error = uci2_config_parse("test_config_iterator", &uci2_ast);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(uci2_ast, NULL);

	error = uci2_node_get(uci2_ast, NULL, NULL, &root_node);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(root_node, NULL);

	error = uci2_node_iterator_new(root_node, &section_iterator);
	assert_int_equal(error, UE_NONE);
	assert_ptr_not_equal(section_iterator, NULL);

	section_index = 0;
	while (uci2_node_iterator_next(section_iterator, &section_next) == 0) {
		error = uci2_node_type_get(section_next, &section_node_type);
		assert_int_equal(error, UE_NONE);
		assert_int_equal(section_node_type, UNT_SECTION);
		error = uci2_node_section_type_get(section_next, &section_type);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_type, section_type_match[section_index]);

		error = uci2_node_section_name_get(section_next, &section_name);
		assert_int_equal(error, UE_NONE);
		assert_string_equal(section_name, section_name_match[section_index]);

		error = uci2_node_iterator_new(section_next, &option_iterator);
		assert_int_equal(error, UE_NONE);
		assert_ptr_not_equal(option_iterator, NULL);

		option_index = 0;
		while (uci2_node_iterator_next(option_iterator, &option_next) == 0) {
			switch (section_index) {
				case 0:
					error = uci2_node_type_get(option_next, &option_node_type);
					assert_int_equal(error, UE_NONE);
					assert_int_equal(option_node_type, UNT_OPTION);

					error = uci2_node_option_name_get(option_next, &option_name);
					assert_int_equal(error, UE_NONE);
					assert_string_equal(option_name, option_0_name_match[option_index]);

					error = uci2_node_option_value_get(option_next, &option_value);
					assert_int_equal(error, UE_NONE);
					assert_string_equal(option_value, option_0_value_match[option_index]);
					break;

				case 1:
					error = uci2_node_type_get(option_next, &option_node_type);
					assert_int_equal(error, UE_NONE);
					assert_int_equal(option_node_type, UNT_OPTION);

					error = uci2_node_option_name_get(option_next, &option_name);
					assert_int_equal(error, UE_NONE);
					assert_string_equal(option_name, option_1_name_match[option_index]);

					error = uci2_node_option_value_get(option_next, &option_value);
					assert_int_equal(error, UE_NONE);
					assert_string_equal(option_value, option_1_value_match[option_index]);
					break;

				case 2:
					error = uci2_node_type_get(option_next, &option_node_type);
					assert_int_equal(error, UE_NONE);
					assert_int_equal(option_node_type, UNT_OPTION);

					error = uci2_node_option_name_get(option_next, &option_name);
					assert_int_equal(error, UE_NONE);
					assert_string_equal(option_name, option_2_name_match[option_index]);

					error = uci2_node_option_value_get(option_next, &option_value);
					assert_int_equal(error, UE_NONE);
					assert_string_equal(option_value, option_2_value_match[option_index]);
					break;

				case 3:
					error = uci2_node_type_get(option_next, &option_node_type);
					assert_int_equal(error, UE_NONE);
					assert_int_equal(option_node_type, UNT_OPTION);

					error = uci2_node_option_name_get(option_next, &option_name);
					assert_int_equal(error, UE_NONE);
					assert_string_equal(option_name, option_3_name_match[option_index]);

					error = uci2_node_option_value_get(option_next, &option_value);
					assert_int_equal(error, UE_NONE);
					assert_string_equal(option_value, option_3_value_match[option_index]);
					break;

				case 4:
					error = uci2_node_type_get(option_next, &option_node_type);
					assert_int_equal(error, UE_NONE);
					assert_int_equal(option_node_type, UNT_OPTION);

					error = uci2_node_option_name_get(option_next, &option_name);
					assert_int_equal(error, UE_NONE);
					assert_string_equal(option_name, option_4_name_match[option_index]);

					error = uci2_node_option_value_get(option_next, &option_value);
					assert_int_equal(error, UE_NONE);
					assert_string_equal(option_value, option_4_value_match[option_index]);
					break;

				case 5:
					error = uci2_node_type_get(option_next, &option_node_type);
					assert_int_equal(error, UE_NONE);
					switch (option_node_type) {
						case UNT_OPTION:
							error = uci2_node_option_name_get(option_next, &option_name);
							assert_int_equal(error, UE_NONE);
							assert_string_equal(option_name, option_5_name_match[option_index]);

							error = uci2_node_option_value_get(option_next, &option_value);
							assert_int_equal(error, UE_NONE);
							assert_string_equal(option_value, option_5_value_match[option_index]);
							break;

						case UNT_LIST:
							error = uci2_node_list_name_get(option_next, &list_name);
							assert_int_equal(error, UE_NONE);
							assert_string_equal(list_name, option_5_name_match[option_index]);

							error = uci2_node_iterator_new(option_next, &list_element_iterator);
							assert_int_equal(error, UE_NONE);

							list_element_index = 0;
							while (uci2_node_iterator_next(list_element_iterator, &list_element_next) == 0) {
								error = uci2_node_type_get(list_element_next, &list_element_node_type);
								assert_int_equal(error, UE_NONE);
								assert_int_equal(list_element_node_type, UNT_LIST_ELEMENT);

								error = uci2_node_list_element_value_get(list_element_next, &list_element_value);
								assert_int_equal(error, UE_NONE);
								assert_string_equal(list_element_value, list_element_value_match[list_element_index]);
								list_element_index++;
							}

							uci2_node_iterator_destroy(&list_element_iterator);

							break;

						default:
							abort();
					}

					break;

				default:
					abort();
			}

			option_index++;
		}
		uci2_node_iterator_destroy(&option_iterator);

		section_index++;
	}

	uci2_node_iterator_destroy(&section_iterator);

	uci2_ast_destroy(&uci2_ast);
}
