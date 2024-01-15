// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright (C) 2019, Sartura Ltd.
//

#include <uci2_ast.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

char UCI2_AST_PATH_SEP = 0x1d;


uci2_iter_t *uci2_iter_begin(uci2_iter_t *it){
    if (it->np == &it->n[0])
        return it;
    else
        return NULL;
}

uci2_iter_t *uci2_iter_end(uci2_iter_t *it){
    if (it->n[0] == NULL && it->n[1] == NULL && it->n_nr == 2) return it;
    if (it->np == &it->n[it->n_nr - 1])
        return it;
    else
        return NULL;
}

uci2_iter_t *uci2_iter_prev(uci2_iter_t *it){
    if(*it->np) it->np--;
    return it;
}

uci2_iter_t *uci2_iter_next(uci2_iter_t *it){
    if(*it->np) it->np++;
    return it;
}

uci2_iter_t *uci2_iter_first(uci2_iter_t *it){
    it->np = &it->n[1];
    return it;
}

uci2_iter_t *uci2_iter_last(uci2_iter_t *it){
    it->np = &it->n[1];
    while(*it->np != NULL) it->np++;
    --it->np;
    return it;
}

uci2_iter_t *uci2_iter_new(uci2_ast_t *n){
    if(!n) return NULL;
    uci2_iter_t *it = calloc(1,
                             sizeof(uci2_iter_t) +
                             sizeof(uci2_ast_t *) * (n->ch_nr + 2));
    it->n_nr = n->ch_nr + 2;
    if(n->ch_nr > 0) {
        memcpy(&it->n[1], n->ch, sizeof(uci2_ast_t *) * n->ch_nr);
        it->np = &it->n[1];
    }
    // last elem already a NULL pointer (calloc)
    return it;
}

uci2_ast_t *uci2_new_ast(int nt, char *name, char *value) {
    uci2_ast_t *a = malloc(sizeof(uci2_ast_t));
    a->nt = nt;
    a->ch_nr = 0;
    a->ch_un_nr = 0;
    a->index = -1;
    a->name = name;
    a->value = value;
    a->ch = NULL;
    a->parent = NULL;
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
    if (!n) return;
    if (fc) {
        for (size_t i = 0; i < n->ch_nr; i++)
            uci2_ast_free(n->ch[i], 1);
        free(n->ch);
    }
    free(n->name);
    free(n->value);
    free(n);
}

void uci2_ast_free_rc(uci2_ast_t *pool) {
    if (!pool) return;
    uci2_ast_t **children = pool->ch;
    size_t cc = pool->ch_nr;
    for (size_t i = 0; i < cc; i++) {
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

void uci2_ast_print(FILE *out, uci2_ast_t *n, int depth) {
    // skip deleted
    if (!n->parent) return;
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
    // value
    if (n->value)
        fprintf(out, "%s %s", (n->nt != UCI2_NT_PACKAGE ? " =" : ""), n->value);
    // next line
    fprintf(out, "\n");
    // process children
    for (size_t i = 0; i < n->ch_nr; i++) {
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
    // null check
    if (!n) return NULL;
    // check current node (if no parent)
    if (!n->parent && strcmp(n->name, name) == 0) return n;
    // process child nodes
    if (cc) {
        // children
        uci2_ast_t **c = n->ch;
        for (size_t i = 0; i < n->ch_nr; i++) {
            // process child and stop if match found
            uci2_ast_t *child = c[i];
            // match child's name
            if (strcmp(child->name, name) == 0) return child;
        }
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

void uci2_ast_set_indexes(uci2_ast_t *root) {
    if (root->nt == UCI2_NT_SECTION_NAME && strcmp(root->name, "@[9999]") == 0) {
        free(root->name);
        char tmp_buff[33];
        sprintf(tmp_buff, "@[%d]", root->parent->ch_un_nr);
        root->index = root->parent->ch_un_nr;
        root->name = strdup(tmp_buff);
        root->parent->ch_un_nr++;
    }
    // children
    uci2_ast_t **c = root->ch;
    for (size_t i = 0; i < root->ch_nr; i++) {
        // process child and stop if match found
        uci2_ast_t *child = c[i];
        // check if index needs to be updates
        uci2_ast_set_indexes(child);
    }
}

