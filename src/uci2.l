%{
	#include <stdio.h>

	#include "utils/memory.h"

	#include "parser.h"
	char *uci_unquote(char *string, int string_size);
%}

%option nounput noinput noyywrap reentrant bison-bridge
%option outfile="lexer.c" header-file="lexer.h"

newline              \n
ws                   [ \t]
comment              "#".*
value                ('[^'\n\t]*')|(\"[^\"\n\t]*\")|([^ '\"\n\t]+)

config               "config"
option               "option"
package              "package"
list                 "list"

%s ST_VALUE


%%
{newline}+          { BEGIN(INITIAL); }
{comment}           ;
{ws}*               ;
{option}            { BEGIN(ST_VALUE); return OPTION; }
{list}              { BEGIN(ST_VALUE); return LIST; }
<ST_VALUE>{value}   { yylval->string = uci_unquote(yytext, yyleng); return VALUE; }
{config}            { BEGIN(ST_VALUE); return CONFIG; }
{package}           { BEGIN(ST_VALUE); return PACKAGE; }

.                   { return 1; }
%%

// how Flex handles ambiguous patterns (config and value)
// - match the longest possible string every time the scanner matches input
// - in the case of a tie, use the pattern that appears first in the program

// basic unquote method
char *uci_unquote(char *string, int string_size)
{
    char *result = NULL;

	if (string_size >= 2 && ((string[0] == '\'' && string[string_size - 1] == '\'') || (string[0] == '"' && string[string_size - 1] == '"'))) {
		result = xcalloc((size_t) (string_size - 1), sizeof(char));
		memcpy(result, string + 1, (size_t) (string_size - 2));
		result[string_size - 2] = '\0';
	} else if (string_size >= 0) {
		result = xcalloc((size_t) (string_size + 1), sizeof(char));
		memcpy(result, string, (size_t) string_size);
		result[string_size] = '\0';
	} else {
        result = NULL;
    }

	return result;
}

// yyerror
extern void yyerror(yyscan_t scanner, ast_t *ctx, const char *string)
{
	// print error condition
    fprintf(stderr, "yyerror: %s\n", string);
}
