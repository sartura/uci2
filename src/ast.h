/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2024, Sartura d.d.
 */

#ifndef AST_H
#define AST_H

#include <stddef.h>

#define AST_NODE_ROOT_NAME "/"
#define AST_NODE_CONFIG_NAME "@C"
#define AST_NODE_PACKAGE_NAME "@P"

#define UNNAMED_SECTION_NAME_PLACEHOLDER "@<type>[<N>]"
#define UNNAMED_SECTION_NAME_BUFFER_SIZE_MAX (1024)

typedef struct ast_s ast_t;
typedef struct ast_node_s ast_node_t;

struct ast_s {
	ast_node_t *root;
	ast_node_t *pool;
};

struct ast_node_s {
	enum ast_node_type {
		ANT_SENTINEL,
		ANT_ROOT,
		ANT_CONFIG,
		ANT_PACKAGE,
		ANT_SECTION_TYPE,
		ANT_SECTION_NAME,
		ANT_OPTION,
		ANT_LIST,
		ANT_LIST_ITEM
	} type;
	char *name;
	char *value;
	ast_node_t *parent;
	ast_node_t **children;
	size_t children_number;
	size_t unnamed_children_number;
};

void ast_init(ast_t *ast);
ast_node_t *ast_node_new(ast_t *ast, enum ast_node_type type, char *name, char *value);
void ast_node_add(ast_node_t *parent, ast_node_t *node);
void ast_destroy(ast_t *ast);

void ast_node_move(ast_node_t *destination, ast_node_t *source);
void ast_node_merge(ast_node_t *node, enum ast_node_type type);
void unnamed_section_name_set(ast_node_t *config_node);

#endif /* ifndef AST_H */
