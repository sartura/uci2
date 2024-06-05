%code top{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #include "parser.h"
    #include "lexer.h"

    // external functions
    extern int yylex(YYSTYPE *lvalp, yyscan_t scanner);
    extern void yyerror(yyscan_t scanner, ast_t *ast, const char *string);
}

%code requires {
    #include "utils/memory.h"

    #include "ast.h"

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
    typedef void *yyscan_t;
#endif
}

// bison options
%output  "parser.c"
%defines "parser.h"
%define api.pure full
%define parse.error verbose
%lex-param   { yyscan_t *scanner }
%parse-param { yyscan_t *scanner }
%parse-param { ast_t *ast }

// initial action when parser starts
%initial-action {
    ast_init(ast);
}

// token types
%union {
    char *string;
    ast_node_t *node;
}

// terminal symbols (tokens)
%token <string>    VALUE
%token          CONFIG OPTION LIST PACKAGE

// non terminal symbol types
%type <node>    config options option lines line root package

// free string memory for discarded symbols (xcalloc in lex)
%destructor { XFREE($$); } VALUE

// set root node
%start root


%%
// root node
root : lines {
                 $$ = ast_node_new(ast, ANT_ROOT, xstrdup(AST_NODE_ROOT_NAME), 0);
                 ast->root = $$;
                 // create config node
                 ast_node_t *node = NULL;
                 node = ast_node_new(ast, ANT_CONFIG, xstrdup(AST_NODE_CONFIG_NAME), NULL);
                 ast_node_add($$, node);
                 // use children from lines
                 ast_node_move($$->children[0], $1);
                 // merge section type nodes with the same name into a single node
                 ast_node_merge($$->children[0], ANT_SECTION_TYPE);
                 // set correct names for unnamed section nodes
                 unnamed_section_name_set($$->children[0]);
             }
        | package lines {
                            $$ = ast_node_new(ast, ANT_ROOT, xstrdup(AST_NODE_ROOT_NAME), 0);
                            ast->root = $$;
                            // package
                            ast_node_add($$, $1);
                            // create config node
                            ast_node_t *node = NULL;
                            node = ast_node_new(ast, ANT_CONFIG, xstrdup(AST_NODE_CONFIG_NAME), NULL);
                            ast_node_add($$, node);
                            // use children from lines
                            ast_node_move($$->children[1], $2);
                            // merge section type nodes with the same name into a single node
                            ast_node_merge($$->children[1], ANT_SECTION_TYPE);
                            // set correct names for unnamed section nodes
                            unnamed_section_name_set($$->children[1]);
                        }
     ;

package : PACKAGE VALUE {
                            $$ = ast_node_new(ast, ANT_PACKAGE, xstrdup(AST_NODE_PACKAGE_NAME), $2);
                        }
        ;

// lines, recursive
lines : line {
                 // Use node type ANT_SENTINEL because this node is a temporary node
                 // whose children are going to be added to the node type ANT_CONFIG in the next step.
                 $$ = ast_node_new(ast, ANT_SENTINEL, NULL, NULL);
                 ast_node_add($$, $1);
             }
      | lines line {
                       ast_node_add($1, $2);
                   }
      ;

// line
line : config {
                  $$ = $1;
              }
     ;

// config line
config : CONFIG VALUE {
                          $$ = ast_node_new(ast, ANT_SECTION_TYPE, $2, NULL);
                          // ** un-named section **
                          // create new AST for unnamed section
                          ast_node_t *node = NULL;
                          node = ast_node_new(ast, ANT_SECTION_NAME, xstrdup(UNNAMED_SECTION_NAME_PLACEHOLDER), NULL);
                          ast_node_add($$, node);
                      }
        | CONFIG VALUE VALUE {
                                 $$ = ast_node_new(ast, ANT_SECTION_TYPE, $2, NULL);
                                 // ** named section **
                                 // create new AST for named section
                                 ast_node_t *node = NULL;
                                 node = ast_node_new(ast, ANT_SECTION_NAME, $3, NULL);
                                 ast_node_add($$, node);
                             }
        | CONFIG VALUE options {
                                   $$ = ast_node_new(ast, ANT_SECTION_TYPE, $2, NULL);
                                   // ** un-named section **
                                   // create new AST for unnamed section
                                   ast_node_t *node = NULL;
                                   node = ast_node_new(ast, ANT_SECTION_NAME, xstrdup(UNNAMED_SECTION_NAME_PLACEHOLDER), NULL);
                                   ast_node_add($$, node);
                                   // - use children from options
                                   // - both section and type present
                                   ast_node_move($$->children[0], $3);
                                   // merge list nodes with the same name into a single node
                                   ast_node_merge($$->children[0], ANT_LIST);
                              }
       | CONFIG VALUE VALUE options {
                                        $$ = ast_node_new(ast, ANT_SECTION_TYPE, $2, NULL);
                                        // ** named section **
                                        // create new AST for section name
                                        ast_node_t *node = NULL;
                                        node = ast_node_new(ast, ANT_SECTION_NAME, $3, NULL);
                                        ast_node_add($$, node);
                                        // - use children from options
                                        // - both section and type present
                                        ast_node_move($$->children[0], $4);
                                        // merge list nodes with the same name into a single node
                                        ast_node_merge($$->children[0], ANT_LIST);
                                    };

// options, recursive
options : option {
                     // Use node type ANT_SENTINEL because this node is a temporary node
                     // whose children are going to be added to the node type ANT_SECTION_NAME in the next step.
                     $$ = ast_node_new(ast, ANT_SENTINEL, NULL, NULL);
                     ast_node_add($$, $1);
                 }
        | options option {
                             ast_node_add($1, $2);
                         }
        ;

// option or list
option : OPTION VALUE VALUE {
                                $$ = ast_node_new(ast, ANT_OPTION, $2, $3);
                            }
        | LIST VALUE {
                              $$ = ast_node_new(ast, ANT_LIST, $2, NULL);
                          }
       | LIST VALUE VALUE {
                              $$ = ast_node_new(ast, ANT_LIST, $2, NULL);
                              // add list value as new node
                              ast_node_t *node = NULL;
                              node = ast_node_new(ast, ANT_LIST_ITEM, $3, NULL);
                              ast_node_add($$, node);
                          }
       ;

%%
