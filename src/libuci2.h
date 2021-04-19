// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright (C) 2019, Sartura Ltd.
//

/**
 * @file libuci2.h
 * @brief UCI2 Configuration Library
 */
#ifndef UCI2_H
#define UCI2_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <uci2_ast.h>
#include <string.h>

/** shorter version of uci2_ast_t */
typedef uci2_ast_t uci2_n_t;
/** shorter version of uci2_parser_ctx_t */
typedef uci2_parser_ctx_t uci2_ctx_t;

/** indent spaces for options and lists */
#define UCI2_INDENT 4

/**
 * Parse configuration file
 *
 * @param[in]       fname   Pointer to configuration 
 *                          filename path
 *
 * @return          Pointer to parser result structure
 *                  or NULL if file does not exist or
 *                  error occurred
 */
uci2_parser_ctx_t *uci2_parse_file(const char *fname);

/**
 * Free parser context
 *
 * @param[in,out]   p       Pointer to parser context
 */
void uci2_free_ctx(uci2_parser_ctx_t *p);

/**
 * Get AST node based on query path (variadic)
 * @param[in]   cfg     Pointer to parser result data
 * @param[in]   ...     Node query string; combined arguments
 *
 * @return      AST node that matches the query path
 */
uci2_ast_t *uci2_get_node_va(uci2_parser_ctx_t *cfg, ...);

/**
 * Create new parser context
 *
 * @return      Pointer to new parser context
 */
uci2_parser_ctx_t *uci2_new_ctx();

/**
 * Add new AST node to AST tree
 *
 * @param[in]       ctx     Context pointer
 * @param[in,out]   p       Parent node
 * @param[in]       nt      Node type to add
 * @param[in]       n       Node name
 * @param[in]       v       Node value
 *
 * @return          New AST node pointer
 */
uci2_ast_t *uci2_add_node(uci2_parser_ctx_t *ctx, uci2_ast_t *p, int nt,
                          char *n, char *v);

/**
 * Converts string value to a boolean value
 *
 * @param[in]       string_value    Input string value
 * @param[out]      boolean_value   Output boolean value
 *
 * @return          0 for success or error code
 */
int uci2_str2bool(const char *string_value, bool *boolean_value);

/**
 * Export AST tree to output stream in configuration file
 * format.
 *
 * @param[in]       ctx     Context pointer
 * @param[out]      out     Output stream
 *
 * @return          0 for success or error code
 */
int uci2_export_ctx(uci2_parser_ctx_t *ctx, FILE *out);

/**
 * Export AST tree to output file path. In case of error,
 * Context pointer has to be freed manually
 *
 * @param[in]       ctx     Context pointer
 * @param[out]      fp      File path to be written to
 *
 * @return          0 for success or error code
 */
int uci2_export_ctx_fsync(uci2_parser_ctx_t *ctx, const char* fp);

/**
 * @brief Helper macro for uci2_get_node_va
 *
 * Query is prefixed with "@C" to get configuration data;
 * some other type of root nodes may be added for future
 * use
 *
 * @param[in]       cfg     Root AST node
 */
#define uci2_q(ctx, ...) \
    uci2_get_node_va(ctx, UCI2_AST_ROOT, UCI2_AST_CFG, __VA_ARGS__, NULL)

/**
 * Get list item (I) node specified by index (i)
 *
 * @param[in]       n       List node (L)
 * @param[in]       i       Index of node's item (I)
 *
 * @return          Pointer to list item or NULL if index is out 
 *                  of bounds or 'n' type is not UCI2_NT_LIST
 */
#define uci2_q_L(n, i) \
        (n && n->nt == UCI2_NT_LIST && n->ch_nr > i ? n->ch[i] : NULL)

/**
 * Get child count for node
 *
 * @param[in]       n       Node pointer
 * 
 * @return          Child count for node 'n'
 */
#define uci2_nc(n) n->ch_nr

/**
 * Print entire AST tree to stdout
 *
 * @param[in]       ctx     Parser context pointer
 */
#define uci2_ctx_print(ctx) uci2_ast_print(stdout, ctx->ast, 0)

/**
 * Print node and its children to stdout
 *
 * @param[in]       n      Node pointer
 */
#define uci2_print(n) uci2_ast_print(stdout, n, 0)

/**
 * Get value of AST node
 *
 * @param[in]       n       Node pointer
 *
 * @return          String pointer of node's value
*/
#define uci2_get_value(n) (n ? n->value : NULL)
/**
 * Get name of AST node
 *
 * @param[in]       n       Node pointer
 *
 * @return          String pointer of node's name
*/
#define uci2_get_name(n) (n ? n->name : NULL)

/**
 * Add AST node (short for uci2_add_node)
 *
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Pointer to parent node
 * @param[in]       nt      Type of new node
 * @param[in]       n       Node name string
 * @param[in]       v       Node value string
 *
 * @return          Pointer to newly created node
*/
#define uci2_add(ctx, p, nt, n, v) uci2_add_node(ctx, p, nt, n, v)

/**
 * Get Configuration root node
 *
 * @param[in]       ctx     Parser context pointer
 *
 * @return          Pointer to root configuration node (@C)
 */
#define UCI2_CFG_ROOT(ctx) \
    uci2_get_node_va(ctx, UCI2_AST_ROOT, UCI2_AST_CFG, NULL)

/**
 * Add Type (un-named) AST node (T)
 *
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Parent node pointer
 * @param[in]       n       String used for node's name
 *
 * @return          Pointer to newly created node
 */
#define uci2_add_T(ctx, p, n) uci2_add_node(ctx, p, UCI2_NT_TYPE, n, NULL)

/**
 * Add Options AST node (O)
 * 
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Parent node pointer
 * @param[in]       n       String used for node's name
 * @param[in]       v       String used for node's value
 *
 * @return          Pointer to newly created node
 */
#define uci2_add_O(ctx, p, n, v) uci2_add_node(ctx, p, UCI2_NT_OPTION, n, v)

/**
 * Add Section AST node (S)
 *
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Parent node pointer
 * @param[in]       n       String used for node's name
 *
 * @return          Pointer to newly created node
 */
#define uci2_add_S(ctx, p, n) \
    uci2_add_node(ctx, p, UCI2_NT_SECTION_NAME, n, NULL)

/**
 * Add List AST node (L)
 *
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Parent node pointer
 * @param[in]       n       String used for node's name
 *
 * @return          Pointer to newly created node
 */
#define uci2_add_L(ctx, p, n) uci2_add_node(ctx, p, UCI2_NT_LIST, n, NULL)

/**
 * Add List Item AST node (I)
 *
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Parent node pointer
 * @param[in]       n       String used for node's name
 *
 * @return          Pointer to newly created node
 */
#define uci2_add_I(ctx, p, n) uci2_add_node(ctx, p, UCI2_NT_LIST_ITEM, n, NULL)

/**
 * Iterate child nodes of 'n', access each child with 'c' pointer
 *
 * @param[in]       n       Pointer to parent node chose child nodes
 *                          should be iterated
 * @param[in]       d       Node depth
 */
#define uci2_iter(n, d) uci2_ast_iter(n, d)

/**
 * Change node 'n' value part to 'nv' string
 *
 * @param[in]       n       Pointer to node whose value member
 *                          will be changed
 * @param[in]       nv      String to replace the old value
 */
#define uci2_change_value(n, nv) \
    do {                         \
        free(n->value);          \
        n->value = strdup(nv);   \
    } while (0)

/**
 * Change node 'n' name part to 'nn' string
 *
 * @param[in]       n       Pointer to node whose value member
 *                          will be changed
 * @param[in]       nn      String to replace the old name
*/
#define uci2_change_name(n, nn) \
    do {                        \
        free(n->name);          \
        n->name = strdup(nn);   \
    } while (0)

/**
 * Mark node as deleted
 *
 * @param[in]       n       Pointer to node which will be
 *                          flaged as deleted
 */
#define uci2_del(n)              \
    do {                         \
        if (n) n->parent = NULL; \
    } while (0)


#endif /* ifndef UCI2_H */
