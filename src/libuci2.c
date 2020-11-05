// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright (C) 2019, Sartura Ltd.
//

#include <libuci2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <uci2_parser.h>
#include <uci2_lexer.h>
#include <uci2_ast.h>

/**
 * Read contents of text file in newly allocated
 * string buffer
 *
 * @param[in]   fname   File to read
 *
 * @return      pointer to file data or 0 on error
 */
static char *fs_read_txt_file(const char *fname) {
    // check if file
    struct stat ps = {};
    stat(fname, &ps);
    if (!S_ISREG(ps.st_mode)) return NULL;
    FILE *f = fopen(fname, "r");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fsize == 0) {
        fclose(f);
        return NULL;
    }
    char *data = malloc(fsize + 1);
    if (!data) return 0;
    if (fread(data, fsize, 1, f) < 1) data = NULL;
    fclose(f);
    data[fsize] = 0;
    return data;
}

uci2_parser_ctx_t *uci2_parse_file(const char *fname) {
    // null check
    if (!fname) return NULL;
    // read file in memory
    char *d = fs_read_txt_file(fname);
    // err check
    if (!d) return NULL;
    // setup parser
    yyscan_t scanner;
    yylex_init(&scanner);
    YY_BUFFER_STATE yyb = yy_scan_string(d, scanner);
    // create AST structure
    uci2_parser_ctx_t *pres = malloc(sizeof(uci2_parser_ctx_t));
    // if parser error occurred
    if (yyparse(scanner, pres)) {
        uci2_ast_free_rc(pres->pool);
        free(pres);
        pres = NULL;
    }
    // free
    yy_delete_buffer(yyb, scanner);
    yylex_destroy(scanner);
    free(d);
    // return
    return pres;
}

void uci2_free_ctx(uci2_parser_ctx_t *p) {
    if (!p) return;
    // free ast pool
    uci2_ast_free_rc(p->pool);
    // pool freed
    p->pool = NULL;
    // free context
    free(p);
}

uci2_parser_ctx_t *uci2_new_ctx() {
    // mem
    uci2_parser_ctx_t *ctx = malloc(sizeof(uci2_parser_ctx_t));
    // init ref/pool
    ctx->pool = uci2_new_ast(UCI2_NT_ROOT, NULL, NULL);
    // create root node
    ctx->ast =
        uci2_new_ast_rc(UCI2_NT_ROOT, strdup(UCI2_AST_ROOT), NULL, ctx->pool);
    // @C node (config)
    uci2_ast_t *c = uci2_new_ast_rc(UCI2_NT_CFG_GROUP, strdup(UCI2_AST_CFG),
                                    NULL, ctx->pool);
    uci2_ast_add_ch(ctx->ast, c);
    // new context
    return ctx;
}

/**
 * Check if child node type is compatible with
 * parent node type
 *
 * @param[in]   nt      Child node type
 * @param[in]   pnt     Parent node type
 *
 * @return      0 for success or error code
 */
static int validate_parent(int nt, int pnt) {
    // ** p is checked in the calling method **
    // node type check
    switch (nt) {
    // Named type
    case UCI2_NT_SECTION_NAME:
        if (pnt != UCI2_NT_TYPE) return -1;
        break;

    // Type
    case UCI2_NT_TYPE:
        if (pnt != UCI2_NT_CFG_GROUP) return -2;
        break;

    // Options (part of type or named section)
    case UCI2_NT_OPTION:
    // List node
    case UCI2_NT_LIST:
        // Lists and Options can only be leaf nodes of
        // Type nodes (named and unnamed)
        if (!(pnt == UCI2_NT_TYPE || pnt == UCI2_NT_SECTION_NAME)) return -3;
        break;

    // List item node
    case UCI2_NT_LIST_ITEM:
        // List item node can only be a leaf node of List node
        if (pnt != UCI2_NT_LIST) return -4;
        break;

    // unsupported node
    default:
        return -5;
    }
    // no error, allowed
    return 0;
}

uci2_ast_t *uci2_add_node(uci2_parser_ctx_t *ctx, uci2_ast_t *p, int nt,
                          char *n, char *v) {
    // sanity checks
    if (!(ctx && p && n && nt > 2)) return NULL;
    // validate child - parent relationship
    if (validate_parent(nt, p->nt)) return NULL;
    // new node
    uci2_ast_t *nn = uci2_new_ast_rc(nt, 
                                     strdup(n), 
                                     (v ? strdup(v) : NULL), 
                                     ctx->pool);
    // link to parent
    uci2_ast_add_ch(p, nn);
    // new AST node
    return nn;
}

int uci2_str2bool(char *str) {
    const char *true_vals[] = {
        "true",
        "enabled",
        "1",
        "yes",
        "on"
    };
    for (int i = 0; i < 5; i++)
        if (strcmp(str, true_vals[i]) == 0) return 1;

    return 0;
}

uci2_ast_t *uci2_get_node_va(uci2_parser_ctx_t *cfg, ...) {
    // va args
    va_list ap;
    va_start(ap, cfg);
    // count how many (last has to be NULL)
    int l = 0, tl = 0, pc = 0;
    char *arg;
    char *tmp = NULL;
    // loop va args  (until last NULL arg)
    while ((arg = va_arg(ap, char *))) {
        // add to tmp str
        pc++;
        l = strlen(arg);
        tmp = realloc(tmp, tl + l + 2);
        // copy arg + 0x1d delimiter
        memcpy(&tmp[tl], arg, l);
        memcpy(&tmp[tl + l], &UCI2_AST_PATH_SEP, 1);
        // inc total l
        tl += l + 1;
    }
    // -1 will overwrite last 0x1d which is not needed
    tmp[tl - 1] = 0;
    // va end
    va_end(ap);
    // find node
    uci2_ast_t *res = uci2_ast_get(cfg->ast, tmp);
    // free tmp
    free(tmp);
    // return res
    return res;
}

/**
 * Export option (O) or List (L) node
 *
 * @param[in]   n       Pointer to options or 
 *                      list node 
 * @param[out]  out     Output stream
 */
static void export_opt_lst(uci2_n_t *n, FILE *out) {
    // Option (O)
    if (n->nt == UCI2_NT_OPTION) {
        // name = value
        fprintf(out, "%*soption %s '%s'\n", 
                UCI2_INDENT, "", n->name, n->value);

        // List (L)
    } else if (n->nt == UCI2_NT_LIST) {
        // print list items (I)
        for (int j = 0; j < n->ch_nr; j++) {
            // node pointer
            uci2_n_t *li = n->ch[j];
            // skip deleted
            if (!li->parent) continue;
            // name = value
            fprintf(out, "%*slist %s '%s'\n", 
                    UCI2_INDENT, "", n->name, li->name);
        }
    }
}

/**
 * Export all children nodes that are either
 * option (O) or List (L) node
 *
 * @param[in]   n       Pointer to parent node
 *                      containing options and/or 
 *                      lists
 * @param[out]  out     Output stream
 */
static void export_opts_lsts(uci2_n_t *s, FILE *out) {
    // loop options, and lists
    for (int i = 0; i < s->ch_nr; i++) {
        // node pointer
        uci2_n_t *n = s->ch[i];
        // skip deleted
        if (!n->parent) continue;
        // export options (O) and lists (L)
        export_opt_lst(n, out);
    }
}

/**
 * Export type (T) node
 *
 * @param[in]   n       Pointer to type (T) node
 * @param[out]  out     Output stream
 */
static void export_type(uci2_n_t *s, FILE *out) {
    // loop sections
    for (int i = 0; i < s->ch_nr; i++) {
        // node pointer
        uci2_n_t *n = s->ch[i];
        // skip deleted
        if (!n->parent) continue;
        // check is section is named
        if (n->nt == UCI2_NT_SECTION_NAME) {
            // config line and type name for consecutive
            // options
            if (i > 0) fprintf(out, "config %s", s->name);
            // section name
            if (n->index == -1)
                fprintf(out, " '%s'\n", n->name);
            else
                fprintf(out, "\n");
            // process section data
            export_opts_lsts(n, out);
            // next line
            fprintf(out, "\n");
            // unnamed
        } else {
            // options (O) and lists (L)
            export_opt_lst(n, out);
        }
    }
}

int uci2_export_ctx(uci2_parser_ctx_t *ctx, FILE *out) {
    // null check
    if (!(ctx && out)) return -1;
    // get @P (package)
    uci2_n_t *p = uci2_get_node_va(ctx, UCI2_AST_ROOT, UCI2_AST_PKG, NULL);
    if (p) fprintf(out, "package %s\n\n", p->value);
    // get @C
    uci2_n_t *r = UCI2_CFG_ROOT(ctx);
    if (!r) return -2;
    // loop types (T)
    for (int i = 0; i < r->ch_nr; i++) {
        // node pointer
        uci2_n_t *n = r->ch[i];
        // skip deleted
        if (!n->parent) continue;
        // unnamed type flag
        bool ut = false;
        // check if type is unnamed
        if (n->ch_nr > 0 && n->ch[0]->parent &&
            n->ch[0]->nt != UCI2_NT_SECTION_NAME)
            ut = true;

        // config line and type name
        fprintf(out, "config %s", n->name);
        // next line for unnamed type
        if(ut) fprintf(out, "\n");
        // process section
        export_type(n, out);
        // next line for unnamed type
        if(ut)fprintf(out, "\n");


    }
    // no error
    return 0;
}

int uci2_export_ctx_fsync(uci2_parser_ctx_t *ctx, const char* fp) {
    // sanity checks
    if(!(ctx && fp)) return -1;
    // create file
    FILE* f = fopen(fp, "w");
    // check is successful
    if(!f) return -1;
    // write context to file, return error
    // if not successful
    if(uci2_export_ctx(ctx, f)) return -2;
    // get file descriptor
    int fd = fileno(f);
    // sync data
    if(fsync(fd)) return -3;
    // close file pointer
    if(fclose(f)) return -4;
    // ok
    return 0;
}
