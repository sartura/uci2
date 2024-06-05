/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2024, Sartura d.d.
 */

#ifndef UCI2_H_ONCE
#define UCI2_H_ONCE

#include <stdbool.h>
#include <stdint.h>

#define UCI2_VERSION_MAJOR 2
#define UCI2_VERSION_MINOR 0
#define UCI2_VERSION_PATCH 0

typedef struct ast_s uci2_ast_t;
typedef struct ast_node_s uci2_node_t;
typedef struct uci2_node_iterator_s uci2_node_iterator_t;

typedef enum {
#define UCI2_ERROR_TABLE                                        \
	XM(UE_NONE, 0, "Success")                                   \
	XM(UE_INVALID_ARGUMENT, -1, "Invalid argument")             \
	XM(UE_FILE_NOT_FOUND, -2, "File not found")                 \
	XM(UE_FILE_IO, -3, "File I/O error")                        \
	XM(UE_PARSER, -4, "Parser error")                           \
	XM(UE_NODE_NOT_FOUND, -5, "Node not found")                 \
	XM(UE_NODE_TYPE_MISMATCH, -6, "Node type mismatch")         \
	XM(UE_NODE_ATTRIBUTE_MISSING, -7, "Node attribute missing") \
	XM(UE_NODE_DUPLICATE, -8, "Node name already exists")       \
	XM(UE_ITERATOR_END, -9, "Iterator reached the end")

#define XM(ENUM, CODE, DESCRIPTION) ENUM = CODE,
	UCI2_ERROR_TABLE
#undef XM
} uci2_error_e;

typedef enum {
	UNT_ROOT,
	UNT_SECTION,
	UNT_OPTION,
	UNT_LIST,
	UNT_LIST_ELEMENT,
} uci2_node_type_e;

uint32_t uci2_version_numeric(void);
const char *uci2_version_string(void);

uci2_error_e uci2_config_parse(const char *config, uci2_ast_t **out);
uci2_error_e uci2_config_remove(const char *config);

uci2_error_e uci2_ast_create(uci2_ast_t **out);
uci2_error_e uci2_ast_sync(uci2_ast_t *uci2_ast, const char *config);
void uci2_ast_destroy(uci2_ast_t **uci2_ast);

uci2_error_e uci2_node_get(uci2_ast_t *uci2_ast, const char *section, const char *option, uci2_node_t **out);
uci2_error_e uci2_node_section_add(uci2_ast_t *uci2_ast, uci2_node_t *parent, const char *type, const char *name, uci2_node_t **out);
uci2_error_e uci2_node_option_add(uci2_ast_t *uci2_ast, uci2_node_t *parent, const char *name, const char *value, uci2_node_t **out);
uci2_error_e uci2_node_list_add(uci2_ast_t *uci2_ast, uci2_node_t *parent, const char *name, uci2_node_t **out);
uci2_error_e uci2_node_list_element_add(uci2_ast_t *uci2_ast, uci2_node_t *parent, const char *value, uci2_node_t **out);
void uci2_node_remove(uci2_node_t *node);

uci2_error_e uci2_node_iterator_new(uci2_node_t *node, uci2_node_iterator_t **out);
void uci2_node_iterator_destroy(uci2_node_iterator_t **node_iterator);
uci2_error_e uci2_node_iterator_next(uci2_node_iterator_t *node_iterator, uci2_node_t **out);
uci2_error_e uci2_node_type_get(uci2_node_t *node, uci2_node_type_e *out);

uci2_error_e uci2_node_section_type_get(uci2_node_t *node, const char **type);
uci2_error_e uci2_node_section_type_set(uci2_node_t *node, const char *type);
uci2_error_e uci2_node_section_name_get(uci2_node_t *node, const char **name);
uci2_error_e uci2_node_section_name_set(uci2_node_t *node, const char *name);

uci2_error_e uci2_node_option_name_get(uci2_node_t *node, const char **name);
uci2_error_e uci2_node_option_name_set(uci2_node_t *node, const char *name);
uci2_error_e uci2_node_option_value_get(uci2_node_t *node, const char **value);
uci2_error_e uci2_node_option_value_set(uci2_node_t *node, const char *value);

uci2_error_e uci2_node_list_name_get(uci2_node_t *node, const char **name);
uci2_error_e uci2_node_list_name_set(uci2_node_t *node, const char *name);

uci2_error_e uci2_node_list_element_value_get(uci2_node_t *node, const char **value);
uci2_error_e uci2_node_list_element_value_set(uci2_node_t *node, const char *value);

uci2_error_e uci2_string_to_boolean(const char *string_value, bool *out);

const char *uci2_error_description_get(uci2_error_e error);

#endif /* UCI2_H_ONCE */
