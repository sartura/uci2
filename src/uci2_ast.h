// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright (C) 2019, Sartura Ltd.
//

/**
 * @file uci2_ast.h
 * @brief UCI2 AST handling
 */
#ifndef UCI2_AST_H
#define UCI2_AST_H 

#include <inttypes.h>
#include <stdio.h>

/** AST node type */
typedef struct uci2_ast uci2_ast_t;
/** Parser context type */
typedef struct uci2_parser_ctx uci2_parser_ctx_t;

/** AST node separator byte */
extern char UCI2_AST_PATH_SEP;

/**
 * Fixed node names
 */
#define UCI2_AST_ROOT       "/"
#define UCI2_AST_CFG        "@C"
#define UCI2_AST_PKG        "@P"

/**
 * Node type names
 */
#define UCI2_B_TYPE         "T"
#define UCI2_B_SECTION      "S"
#define UCI2_B_SECTION_NM   "S"
#define UCI2_B_OPTION       "O"
#define UCI2_B_LIST         "L"
#define UCI2_B_LIST_ITEM    "I"


/** UCI node type */
enum uci2_nt {
    /** Root AST node */
    UCI2_NT_ROOT            = 0,
    /** 
     * config: main configuration groups like network, 
     * system, firewall. Each configuration group has 
     * it's own file in /etc/config
     */
    UCI2_NT_CFG_GROUP       = 1,

    /**
     * Package
     */
    UCI2_NT_PACKAGE         = 2,

    /**
     * sections: config is divided into sections. A section 
     * can either be named or unnamed.
     */
    UCI2_NT_SECTION         = 3,

    /**
     * types: a section can have a type. E.g in the network 
     * config we typically have 4 sections of the type 
     * “interface”. The sections are “lan”, “wan”, “loopback” 
     * and “wan6”
     */

    /**
     * Section name when name is present
     */
    UCI2_NT_SECTION_NAME    = 4,

    /**
     * types: a section can have a type. E.g in the network 
     * config we typically have 4 sections of the type 
     * “interface”. The sections are “lan”, “wan”, “loopback” 
     * and “wan6”
     */
    UCI2_NT_TYPE            = 5,

    /**
     * options: each section have some options where you 
     * set your configuration values
     */
    UCI2_NT_OPTION          = 6,

    /**
     * values: value of option
     */
    UCI2_NT_VALUE           = 7,

    /**
     * In the lines starting with a list keyword an option with 
     * multiple values is defined. All list statements that share 
     * the same name will be combined into a single list of values 
     * with the same order as in the configuration file
     */
    UCI2_NT_LIST            = 8,

    /**
     * List item
     */
    UCI2_NT_LIST_ITEM       = 9

};

/**
 * UCI Parser context struct
 */
struct uci2_parser_ctx {
    /** root AST */
    uci2_ast_t *ast;
    /** AST node pool */
    uci2_ast_t *pool;
};

/**
 * AST node struct
 */
struct uci2_ast {
    /** Node type */
    int nt;
    /** Node name */
    char *name;
    /** Node value */
    char *value;
    /** Node index (unnamed section) */
    int index;
    /** Pointer to parent AST node */
    uci2_ast_t *parent;
    /** Array of child AST nodes */
    uci2_ast_t **ch;
    /** Number of children in ch array */
    int ch_nr;
    // Number of unnamed children in ch array */
    int ch_un_nr;
};

/**
 * Create new AST node
 * @param[in]   nt          Node type
 * @param[in]   name        Node name string
 * @param[in]   value       Node value string
 * @return      Pointer to newly created AST node
 */
uci2_ast_t *uci2_new_ast(int nt, char *name, char *value);

/**
 * Create new AST node, pooled
 * @param[in]   nt          Node type
 * @param[in]   name        Node name string
 * @param[in]   value       Node value string
 * @param[in]   ref_pool    Node reference pool
 * @return      Pointer to newly created AST node
 */
uci2_ast_t *uci2_new_ast_rc(int nt, char *name, char *value,
                            uci2_ast_t *ref_pool);

/**
 * Add child AST node to existing AST node
 * @param[in,out]   p       Parent AST node
 * @param[in]       c       Child AST node
 * @return          Pointer to Parent AST node
 */
uci2_ast_t *uci2_ast_add_ch(uci2_ast_t *p, uci2_ast_t *c);

/**
 * Free memory consumed by AST node
 * @param[in,out]   n       Pointer to AST node to be freed
 * @param[in]       fc      If true, free children recursively
 */
void uci2_ast_free(uci2_ast_t *n, int fc);

/**
 * Free memory consumed by AST nodes in ref pool
 * @param[in,out]   pool    Pointer to AST pool
 */
void uci2_ast_free_rc(uci2_ast_t *pool);

/**
 * Print AST tree
 * @param[out]  out         Output stream
 * @param[in]   n           Pointer to starting AST node
 * @param[in]   fmt         Output format type
 * @param[in]   depth       Current depth, should be 0
 */
void uci2_ast_print(FILE *out, uci2_ast_t *n, int depth);

/**
 * @brief Get AST node (/path/to/node notation)
 *
 * - nodes use similar notation like linux
 *   filesystem
 * - example 1: get interface loobpack node
 *   ======================================
 *   /@C/interface/loobpack
 *
 * - example 2: get interface wifi ipaddr
 *   ====================================================
 *   /@Cintarface/wifi/ipaddr
 *
 *
 * @param[in]       root        Pointer to root AST node
 * @param[in]       path        Destination path to search for
 * @return          AST node that matches the path
 */
uci2_ast_t *uci2_ast_get(uci2_ast_t *root, const char *path);

/**
 * Update AST auto-generated indexes
 *
 * @param[in]       root        Pointer to root AST node
 */
void uci2_ast_set_indexes(uci2_ast_t *root);

/**
 * AST node iterator
 *
 * @param[in]   n       Node to loop
 * @param[in]   c       Variable name for each child node
 */
#define uci2_ast_iter(n, c)                                   \
    for (uci2_ast_t *i = (uci2_ast_t *)n->ch,                 \
                    *c = i ? *(uci2_ast_t **)i : NULL;        \
         n->ch && i < (uci2_ast_t *)(n->ch + n->ch_nr);       \
         i = (uci2_ast_t *)((uci2_ast_t **)i + 1),            \
                    c = (i < (uci2_ast_t *)(n->ch + n->ch_nr) \
                             ? *(uci2_ast_t **)i              \
                             : NULL))

// free and set to NULL
#define UCI2_FN(p) \
    do {           \
        free(p);   \
        p = NULL;  \
    } while (0)


#endif /* ifndef UCI2_AST_H */
