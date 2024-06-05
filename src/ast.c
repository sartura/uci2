/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2024, Sartura d.d.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "utils/memory.h"

#include "ast.h"

void ast_init(ast_t *ast)
{
	assert(ast);

	ast->root = NULL;
	ast->pool = xcalloc(1, sizeof(ast_node_t));
}

ast_node_t *ast_node_new(ast_t *ast, enum ast_node_type type, char *name, char *value)
{
	ast_node_t *node = NULL;

	assert(ast);

	node = xcalloc(1, sizeof(ast_node_t));
	node->type = type;
	node->name = name;
	node->value = value;

	ast_node_add(ast->pool, node);

	return node;
}

void ast_node_add(ast_node_t *parent, ast_node_t *node)
{
	assert(parent);
	assert(node);

	node->parent = parent;
	parent->children_number++;
	parent->children = xrealloc(parent->children, parent->children_number * sizeof(ast_node_t *));
	parent->children[parent->children_number - 1] = node;
}

void ast_destroy(ast_t *ast)
{
	if (ast) {
		if (ast->pool) {
			for (size_t i = 0; i < ast->pool->children_number; i++) {
				XFREE(ast->pool->children[i]->name);
				XFREE(ast->pool->children[i]->value);
				XFREE(ast->pool->children[i]->children);
				XFREE(ast->pool->children[i]);
			}

			XFREE(ast->pool->children);
			XFREE(ast->pool);
		}

		XFREE(ast);
	}
}

void ast_node_move(ast_node_t *destination, ast_node_t *source)
{
	assert(destination);
	assert(destination->parent);
	assert(source);
	assert(source->parent);

	destination->children = source->children;
	destination->children_number = source->children_number;
	destination->unnamed_children_number = source->unnamed_children_number;
	source->children = NULL;
	source->children_number = 0;
	source->unnamed_children_number = 0;
	for (size_t i = 0; i < destination->children_number; i++) {
		destination->children[i]->parent = destination;
	}
}

void ast_node_merge(ast_node_t *node, enum ast_node_type type)
{
	assert(node);
	assert(node->parent);

	for (size_t i = 0; i < node->children_number; i++) {
		if (node->children[i] == NULL &&
			node->children[i]->parent == NULL &&
			node->children[i]->type != type &&
			node->children[i]->name == NULL) {
			continue;
		}

		for (size_t j = i + 1; j < node->children_number; j++) {
			if (node->children[j]->name &&
				node->children[i]->name &&
				strcmp(node->children[j]->name, node->children[i]->name) == 0) {
				for (size_t k = 0; k < node->children[j]->children_number; k++) {
					ast_node_add(node->children[i], node->children[j]->children[k]);
				}

				node->children[j]->children_number = 0;
				node->children[j]->unnamed_children_number = 0;
				node->children[j]->parent = NULL;
			}
		}
	}
}

void unnamed_section_name_set(ast_node_t *config_node)
{
	ast_node_t *section_type_node = NULL;
	ast_node_t *section_name_node = NULL;
	char unnamed_section_name[UNNAMED_SECTION_NAME_BUFFER_SIZE_MAX + 1] = {0};

	assert(config_node);
	assert(config_node->parent);

	for (size_t i = 0; i < config_node->children_number; i++) {
		section_type_node = config_node->children[i];
		if (section_type_node &&
			section_type_node->parent &&
			section_type_node->name) {
			for (size_t j = 0; j < config_node->children[i]->children_number; j++) {
				section_name_node = section_type_node->children[j];
				if (section_name_node &&
					section_name_node->parent &&
					section_name_node->type == ANT_SECTION_NAME &&
					section_name_node->name &&
					strcmp(section_name_node->name, UNNAMED_SECTION_NAME_PLACEHOLDER) == 0) {
					XFREE(section_name_node->name);
					snprintf(unnamed_section_name, sizeof(unnamed_section_name), "@%s[%zu]", section_type_node->name, section_type_node->unnamed_children_number);
					section_name_node->name = xstrdup(unnamed_section_name);
					section_type_node->unnamed_children_number++;
				}
			}
		}
	}
}
