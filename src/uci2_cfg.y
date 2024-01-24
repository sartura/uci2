%code top{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uci2_parser.h"
#include "uci2_lexer.h"

// external functions
extern int yylex(YYSTYPE * lvalp, yyscan_t scanner);
extern void yyerror(yyscan_t scanner, uci2_parser_ctx_t* ctx, const char*);
char* uci_itos(int num);

// ** macros to make BNF more readable **
// see uci2_new_ast_rc header documentation
#define AST_NRC(a, b, c) uci2_new_ast_rc(a, b, c, ctx->pool)
// see uci2_ast_add_ch header documentation
#define AST_ACH(a, b) uci2_ast_add_ch(a, b)
// move children from b to a, just set pointer, shallow copy
#define AST_MOVCH(a, b)                                          \
    do {                                                         \
        a->ch = b->ch;                                           \
        a->ch_nr = b->ch_nr;                                     \
        a->ch_un_nr = b->ch_un_nr;                               \
        b->ch = NULL;                                            \
        b->ch_nr = 0;                                            \
        for (size_t _i1 = 0; _i1 < a->ch_nr; _i1++) a->ch[_i1]->parent = a; \
    } while (0)

// add children from b to a, just set pointer, shallow copy
#define AST_ADDCH(a, b)                                                  \
    do {                                                                 \
        for (size_t _i2 = 0; _i2 < b->ch_nr; _i2++) uci2_ast_add_ch(a, b->ch[_i2]); \
    } while (0)

// crate new AST (ref counted) a with parent p, type t, name n 
// and value v
#define AST_NP(t, n, v, p)                                   \
    do {                                                     \
        uci2_ast_t* a = uci2_new_ast_rc(t, n, v, ctx->pool); \
        uci2_ast_add_ch(p, a);                               \
    } while (0)

// Merge all nodes  of the same type into a single node;
// this is used for type (T) and list (L) nodes (I)
#define AST_MERGE(lst, t)                              \
    do {                                               \
        for (size_t _i3 = 0; _i3 < lst->ch_nr; _i3++) {         \
            uci2_ast_t* n = lst->ch[_i3];                \
            if (n->nt != t) continue;                  \
            for (size_t _j1 = _i3 + 1; _j1 < lst->ch_nr; _j1++) { \
                uci2_ast_t* n2 = lst->ch[_j1];           \
                if (strcmp(n2->name, n->name) == 0) {  \
                    AST_ADDCH(n, n2);                  \
                    n2->ch_nr = 0;                     \
                    n2->ch_un_nr = 0;                  \
                    n2->parent = NULL;                 \
                }                                      \
            }                                          \
        }                                              \
    } while (0)
}

%code requires {
    #include <uci2_ast.h>
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
    typedef void* yyscan_t;
#endif
}

/* bison options */
%output  "uci2_parser.c"
%defines "uci2_parser.h"
%define api.pure full
%define parse.error verbose
%lex-param   { yyscan_t* scanner }
%parse-param { yyscan_t* scanner }
%parse-param { uci2_parser_ctx_t* ctx }

/* initial action when parser starts */
%initial-action {
    ctx->ast = 0;
    ctx->pool = uci2_new_ast(UCI2_NT_ROOT, 0, 0);
}

/* token types */
%union {
    char* str;
    uci2_ast_t* node;
}

/* terminal symbols (tokens) */
%token <str>    VALUE
%token          CONFIG OPTION LIST PACKAGE

/* non terminal symbol types */
%type <node>    config options option lines line root package

/* free string memory for discarded symbols (strdup in lex) */
%destructor { free($$); } VALUE 

/* set root node */
%start root


%%
/* root node */
root : lines {  $$ = AST_NRC(UCI2_NT_ROOT, strdup(UCI2_AST_ROOT), 0);
                ctx->ast = $$;
                // create cfg node
                AST_NP(UCI2_NT_CFG_GROUP, strdup(UCI2_AST_CFG), 0, $$);
                // use children from lines
                AST_MOVCH($$->ch[0], $1);
                // merge same type nodes into a single node
                AST_MERGE($$->ch[0], UCI2_NT_TYPE);
                // set indexes
                uci2_ast_set_indexes($$);
             }
        | package lines {  $$ = AST_NRC(UCI2_NT_ROOT, strdup(UCI2_AST_ROOT), 0);
                           ctx->ast = $$;
                           // package
                           AST_ACH($$, $1);
                           // create cfg node
                           AST_NP(UCI2_NT_CFG_GROUP, strdup(UCI2_AST_CFG), 0, $$);
                           // use children from lines
                           AST_MOVCH($$->ch[1], $2);
                           // merge same type nodes into a single node
                           AST_MERGE($$->ch[1], UCI2_NT_TYPE);

                        }
     ;

package : PACKAGE VALUE {  $$ = AST_NRC(UCI2_NT_PACKAGE, strdup(UCI2_AST_PKG), $2);  }
        ;

/* lines, recursive */
lines : line {  $$ = AST_NRC(UCI2_NT_ROOT, 0, 0); AST_ACH($$, $1);  }
      | lines line {  AST_ACH($1, $2);  }
      ;

/* line */
line : config {  $$ = $1;  }
     ;

/* config line */
config : CONFIG VALUE options {  $$ = AST_NRC(UCI2_NT_TYPE, $2, 0);
                                 // ** un-named section **
                                 // create new AST for unnamed section (index for name)
                                 char tmp_buff[33];
                                 sprintf(tmp_buff, "@[%d]", 9999);
                                 AST_NP(UCI2_NT_SECTION_NAME, strdup(tmp_buff), 0, $$);
                                 // - use children from options
                                 // - both section and type present
                                 AST_MOVCH($$->ch[0], $3);
                                 // merge scattered list items into single
                                 // list node with list items
                                 AST_MERGE($$->ch[0], UCI2_NT_LIST);
                              }
       | CONFIG VALUE VALUE options {  $$ = AST_NRC(UCI2_NT_TYPE, $2, 0);
                                       // ** named section **
                                       // create new AST for section name
                                       AST_NP(UCI2_NT_SECTION_NAME, $3, 0, $$);
                                       // - use children from options
                                       // - both section and type present
                                       AST_MOVCH($$->ch[0], $4);
                                       // merge scattered list items into single
                                       // list node with list items
                                       AST_MERGE($$->ch[0], UCI2_NT_LIST);
                                    };

/* options */
options : option {  $$ = AST_NRC(UCI2_NT_SECTION, 0, 0); AST_ACH($$, $1);  }
        | options option {  AST_ACH($1, $2);  }
        ;

/* section option or list */
option : OPTION VALUE VALUE {  $$ = AST_NRC(UCI2_NT_OPTION, $2, $3);  }
       | LIST VALUE VALUE {  $$ = AST_NRC(UCI2_NT_LIST, $2, 0); 
                             // add list value as new node
                             AST_NP(UCI2_NT_LIST_ITEM, $3, 0, $$);
                          }
       ;

%%
