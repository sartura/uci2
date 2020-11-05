// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright (C) 2019, Sartura Ltd.
//

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <uci2_ast.h>

char UCI2_AST_PATH_SEP = 0x1d;

uci2_ast_t *uci2_new_ast(int nt, char *name, char *value) {
    uci2_ast_t *a = malloc(sizeof(uci2_ast_t));
    a->nt = nt;
    a->ch_nr = 0;
    a->name = name;
    a->value = value;
    a->ch = NULL;
    a->parent = NULL;
    a->id = 0; // not set
    return a;
}

uci2_ast_t *uci2_new_ast_rc(int nt, char *name, char *value,
                            uci2_ast_t *ref_pool) {
    // new AST
    uci2_ast_t *n = uci2_new_ast(nt, name, value);
    // add to pool for reference counting
    uci2_ast_add_ch(ref_pool, n);
    // return new node
    return n;
}

uci2_ast_t *uci2_ast_add_ch(uci2_ast_t *p, uci2_ast_t *c) {
    // set parent
    c->parent = p;
    // inc parent's child count
    p->ch_nr++;
    // add child to list of children
    p->ch = realloc(p->ch, p->ch_nr * sizeof(uci2_ast_t *));
    p->ch[p->ch_nr - 1] = c;
    // return parent node
    return p;
}

void uci2_ast_free(uci2_ast_t *n, int fc) {
    if (!n)
        return;
    if (fc) {
        for (int i = 0; i < n->ch_nr; i++)
            uci2_ast_free(n->ch[i], 1);
        free(n->ch);
    }
    free(n->name);
    free(n->value);
    free(n);
}

void uci2_ast_free_rc(uci2_ast_t *pool) {
    if (!pool)
        return;
    uci2_ast_t **children = pool->ch;
    int cc = pool->ch_nr;
    for (int i = 0; i < cc; i++) {
        uci2_ast_t *c = children[i];
        UCI2_FN(c->name);
        UCI2_FN(c->value);
        UCI2_FN(c->ch);
        UCI2_FN(c);
    }
    UCI2_FN(pool->ch);
    UCI2_FN(pool);
}

/**
 * Print to with indentation
 * @param[out]      out         Output stream
 * @param[in]       indent      Indentation count
 * @param[in]       str         String to print after indentation
 */
static void print_indent(FILE *out, int indent, const char *str) {
    fprintf(out, "%*s%s", indent, "", str);
}

const char *uci2_get_node_str(int nt) {
    // node type string
    const char *node_str = NULL;
    switch (nt) {
    case UCI2_NT_ROOT:
        node_str = UCI2_AST_ROOT;
        break;
    case UCI2_NT_PACKAGE:
        node_str = UCI2_AST_PKG;
        break;
    case UCI2_NT_CFG_GROUP:
        node_str = UCI2_AST_CFG;
        break;
    case UCI2_NT_SECTION:
        node_str = UCI2_B_SECTION;
        break;
    case UCI2_NT_TYPE:
        node_str = UCI2_B_TYPE;
        break;
    case UCI2_NT_SECTION_NAME:
        node_str = UCI2_B_SECTION_NM;
        break;
    case UCI2_NT_OPTION:
        node_str = UCI2_B_OPTION;
        break;
    case UCI2_NT_LIST:
        node_str = UCI2_B_LIST;
        break;
    case UCI2_NT_LIST_ITEM:
        node_str = UCI2_B_LIST_ITEM;
        break;
    default:
        node_str = "";
        break;
    }
    return node_str;
}

void uci2_ast_print(FILE *out, uci2_ast_t *n, int depth) {
    // skip deleted
    if (!n->parent)
        return;
    // copy node depth from args
    int nd = depth;
    // node type string
    const char *node_str = NULL;
    switch (n->nt) {
    case UCI2_NT_ROOT:
        node_str = UCI2_AST_ROOT;
        break;
    case UCI2_NT_PACKAGE:
        node_str = UCI2_AST_PKG;
        break;
    case UCI2_NT_CFG_GROUP:
        node_str = UCI2_AST_CFG;
        break;
    case UCI2_NT_SECTION:
        node_str = UCI2_B_SECTION;
        break;
    case UCI2_NT_TYPE:
        node_str = UCI2_B_TYPE;
        break;
    case UCI2_NT_SECTION_NAME:
        node_str = UCI2_B_SECTION_NM;
        break;
    case UCI2_NT_OPTION:
        node_str = UCI2_B_OPTION;
        break;
    case UCI2_NT_LIST:
        node_str = UCI2_B_LIST;
        break;
    case UCI2_NT_LIST_ITEM:
        node_str = UCI2_B_LIST_ITEM;
        break;
    default:
        node_str = "";
        break;
    }

    // inc next depth
    ++nd;

    // indent
    print_indent(out, depth * 2, "");
    // name
    fprintf(out, "[%s]", node_str);
    // do not print name for ROOT, CFG_GROUP and PACKAGE
    if (n->nt > UCI2_NT_PACKAGE)
        fprintf(out, " %s", (n->name ? n->name : "(n/a)"));
    // id
    if (n->nt == UCI2_NT_LIST || n->nt == UCI2_NT_TYPE)
        fprintf(out, " id:[%d]", n->id);
    // value
    if (n->value)
        fprintf(out, "%s %s", (n->nt != UCI2_NT_PACKAGE ? " =" : ""), n->value);
    // next line
    fprintf(out, "\n");
    // process children
    for (int i = 0; i < n->ch_nr; i++) {
        uci2_ast_print(out, n->ch[i], nd);
    }
}

/**
 * Find AST by node's name
 * @param[in]   n       Starting node
 * @param[in]   name    Node name to look for
 * @param[in]   cc      If true, try to match children
 * @return      Pointer to matched AST node or NULL
 */
static uci2_ast_t *cfg_ast_get(uci2_ast_t *n, const char *name, bool cc) {
    char *tname = NULL, *hash;
    int len = 0, id = 0;

    if (!n)
        return NULL;
    // first check for # and a number node
    hash = strchr(name, '#');
    if (!hash) {
        // check current node (if no parent)
        if (!n->parent && strcmp(n->name, name) == 0)
            return n;
        // process child nodes
        if (cc) {
            // children
            uci2_ast_t **c = n->ch;
            for (int i = 0; i < n->ch_nr; i++) {
                // process child and stop if match found
                uci2_ast_t *child = c[i];
                // match child's name
                if (strcmp(child->name, name) == 0 && child->parent != 0)
                    return child;
            }
        }
    } else {
        // hash found => get id using atoi + get config name => expensive
        // length of the config name => memcpy
        len = hash - name;
        tname = malloc(sizeof(char) * (len + 1));
        memcpy(tname, name, len);
        tname[len] = 0;
        // go beyond hash simbol
        ++hash;
        id = atoi(hash);

        // check current node (if no parent)
        if (!n->parent && strcmp(n->name, name) == 0)
            return n;
        // process child nodes
        if (cc) {
            // children
            uci2_ast_t **c = n->ch;
            for (int i = 0; i < n->ch_nr; i++) {
                // process child and stop if match found
                uci2_ast_t *child = c[i];
                // match child's name
                if (strcmp(child->name, tname) == 0 && child->id == id && child->parent != 0) {
                    free(tname);
                    return child;
                }
            }
        }
        free(tname);
    }

    return NULL;
}

uci2_ast_t *uci2_ast_get(uci2_ast_t *root, const char *path) {
    uci2_ast_t *n = NULL;
    char *tmp_p;
    char *tkn;
    char *saveptr;

    // assume first node is root
    n = root;
    // skip first token if root node (/)
    if (strlen(path) > 1 && path[0] == '/')
        tmp_p = strdup(&path[1]);
    else
        tmp_p = strdup(path);

    // tokenize
    tkn = strtok_r(tmp_p, &UCI2_AST_PATH_SEP, &saveptr);
    if (!tkn) {
        free(tmp_p);
        return NULL;
    }

    // find first
    n = cfg_ast_get(n, tkn, n);
    // if not found
    if (!n) {
        free(tmp_p);
        return NULL;
    }

    // continue with tokens
    while ((tkn = strtok_r(NULL, &UCI2_AST_PATH_SEP, &saveptr)) && n) {
        n = cfg_ast_get(n, tkn, true);
    }

    // free tmp str
    free(tmp_p);
    // return last found node
    return n;
}

void _addch(uci2_ast_t *a, uci2_ast_t *b) {
    for (int i = 0; i < b->ch_nr; i++)
        uci2_ast_add_ch(a, b->ch[i]);
}

/**
 * Removes deleted node given from the pool
 *
 * @param[in]   pool       Pool node
 * @param[in]   ptr        Pointer to remove from the pool
 */
void uci2_ast_remove_del_pooled(uci2_ast_t *pool, uci2_ast_t *ptr) {
    int i, j;

    for(i = 0; i < pool->ch_nr; i++) {
        if(pool->ch[i] == ptr) {
            // free and remove from the list
            uci2_ast_free(pool->ch[i], 1);
            for(j = i + 1; j < pool->ch_nr; j++) {
                pool->ch[j - 1] = pool->ch[j];
            }
            --pool->ch_nr;
            break;
        }
    }

    // free mem
    pool->ch = (uci2_ast_t **) realloc(pool->ch, sizeof(uci2_ast_t *) * pool->ch_nr);
}

/**
 * Removes deleted (marked with parent == NULL) nodes after parsing the file
 *
 * @param[in]   node       Root node from which to start removing deleted nodes
 * @param[in]   pool       Pool node
 */
void uci2_ast_remove_del(uci2_ast_t *node, uci2_ast_t *pool) {
    if(!node) return;
    int i, j, start_nr = node->ch_nr;
    uci2_ast_t *c = NULL;

    for(i = 0; i < node->ch_nr; i++) {
        c = node->ch[i];
        if(!c->parent) {
            // find the pointer in the pool and delete it
            // uci2_ast_remove_del_pooled(pool, c);
            // remove the element from the children list
            for(j = i + 1; j < node->ch_nr; j++) {
                node->ch[j - 1] = node->ch[j];
            }
            --node->ch_nr;
            --i;
        }
    }

    // realloc if needed -> if something was removed
    if(start_nr - node->ch_nr) {
        node->ch = (uci2_ast_t **) realloc(node->ch, sizeof(uci2_ast_t *) * node->ch_nr);
    }

    for(i = 0; i < node->ch_nr; i++) {
        uci2_ast_remove_del(node->ch[i], pool);
    }
}

/**
 * Merges AST nodes of the same type if possible
 *
 * @param[in]   node       Parent node of children which should be looked at for merging
 * @param[in]   type       Type of a new node
 */
void uci2_ast_merge(uci2_ast_t *node, enum uci2_nt type) {
    int br = 0;
    uci2_ast_t *c1, *c2;
    for (int i = 0; i < node->ch_nr; i++) {
        c1 = c2 = 0;
        br = 0;
        uci2_ast_t *n = node->ch[i];
        int nid = n->id;
        if (nid == 0) {
            n->id = ++nid;
        }
        if (n->nt != type)
            continue;
        for (int j = i + 1; j < node->ch_nr; j++) {
            uci2_ast_t *n2 = node->ch[j];
            if (strcmp(n2->name, n->name) == 0) {
                if (n->ch_nr && n2->ch_nr) {
                    // both have children -> check if the both first children are sections or list items => only sections and list items can be merged
                    c1 = n->ch[0];
                    c2 = n2->ch[0];
                    if ((c1->nt == UCI2_NT_SECTION_NAME &&
                        c2->nt == UCI2_NT_SECTION_NAME) || (c1->nt == UCI2_NT_LIST_ITEM &&
                        c2->nt == UCI2_NT_LIST_ITEM)) {
                        if (!br) {
                            // list items can be added in the same list until
                            // the list of lists is broken by another
                            // command/option/etc.
                            _addch(n, n2);
                            n2->ch_nr = 0;
                            n2->parent = NULL;
                        } else {
                            n2->id = ++nid;
                        }
                    } else {
                        n2->id = ++nid;
                        br = 1;
                    }
                }
            } else {
                br = 1;
            }
        }
    }
}
