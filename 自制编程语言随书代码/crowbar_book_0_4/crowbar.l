%{
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) (result = my_yyinput(buf, max_size))
#include <stdio.h>
#include <string.h>
#include "DBG.h"
#include "crowbar.h"
#include "y.tab.h"

static int
file_input(char *buf, int max_size)
{
    int ch;
    int len;

    if (feof(yyin))
        return 0;

    for (len = 0; len < max_size; len++) {
        ch = getc(yyin);
        if (ch == EOF)
            break;
        buf[len] = ch;
    }
    return len;
}

static char **st_source_string;
static int st_current_source_line;
static int st_current_char_index;
 
void
crb_set_source_string(char **source)
{
    st_source_string = source;
    st_current_source_line = 0;
    st_current_char_index = 0;
}

static int
string_input(char *buf, int max_size)
{
    int len;

    if (st_source_string[st_current_source_line] == NULL)
        return 0;

    if (st_source_string[st_current_source_line][st_current_char_index]
        == '\0') {
        st_current_source_line++;
        st_current_char_index = 0;
    }

    if (st_source_string[st_current_source_line] == NULL)
        return 0;

    len = smaller(strlen(st_source_string[st_current_source_line])
                  - st_current_char_index,
                  max_size);
    strncpy(buf,
            &st_source_string[st_current_source_line][st_current_char_index],
            len);
    st_current_char_index += len;

    return len;
}

static int
my_yyinput(char *buf, int max_size)
{
    int result;

    switch (crb_get_current_interpreter()->input_mode) {
    case CRB_FILE_INPUT_MODE:
        result = file_input(buf, max_size);
        break;
    case CRB_STRING_INPUT_MODE:
        result = string_input(buf, max_size);
        break;
    default:
        DBG_panic(("bad default. input_mode..%d\n",
                   crb_get_current_interpreter()->input_mode));
    }

    return result;
}


int
yywrap(void)
{
    return 1;
}

static void
increment_line_number(void)
{
    crb_get_current_interpreter()->current_line_number++;
}
%}
%start COMMENT STRING_LITERAL_STATE REGEXP_LITERAL_STATE SHIFT_JIS_2ND_CHAR
%%
<INITIAL>"function"     return FUNCTION;
<INITIAL>"if"           return IF;
<INITIAL>"else"         return ELSE;
<INITIAL>"elsif"        return ELSIF;
<INITIAL>"while"        return WHILE;
<INITIAL>"for"          return FOR;
<INITIAL>"foreach"      return FOREACH;
<INITIAL>"return"       return RETURN_T;
<INITIAL>"break"        return BREAK;
<INITIAL>"continue"     return CONTINUE;
<INITIAL>"null"         return NULL_T;
<INITIAL>"true"         return TRUE_T;
<INITIAL>"false"        return FALSE_T;
<INITIAL>"closure"      return CLOSURE;
<INITIAL>"global"       return GLOBAL_T;
<INITIAL>"try"          return TRY;
<INITIAL>"catch"        return CATCH;
<INITIAL>"finally"      return FINALLY;
<INITIAL>"throw"        return THROW;
<INITIAL>"final"        return FINAL;
<INITIAL>"("            return LP;
<INITIAL>")"            return RP;
<INITIAL>"{"            return LC;
<INITIAL>"}"            return RC;
<INITIAL>"["            return LB;
<INITIAL>"]"            return RB;
<INITIAL>";"            return SEMICOLON;
<INITIAL>":"            return COLON;
<INITIAL>","            return COMMA;
<INITIAL>"&&"           return LOGICAL_AND;
<INITIAL>"||"           return LOGICAL_OR;
<INITIAL>"="            return ASSIGN_T;
<INITIAL>"=="           return EQ;
<INITIAL>"!="           return NE;
<INITIAL>">"            return GT;
<INITIAL>">="           return GE;
<INITIAL>"<"            return LT;
<INITIAL>"<="           return LE;
<INITIAL>"+"            return ADD;
<INITIAL>"-"            return SUB;
<INITIAL>"*"            return MUL;
<INITIAL>"/"            return DIV;
<INITIAL>"%"            return MOD;
<INITIAL>"+="           return ADD_ASSIGN_T;
<INITIAL>"-="           return SUB_ASSIGN_T;
<INITIAL>"*="           return MUL_ASSIGN_T;
<INITIAL>"/="           return DIV_ASSIGN_T;
<INITIAL>"%="           return MOD_ASSIGN_T;
<INITIAL>"++"           return INCREMENT;
<INITIAL>"--"           return DECREMENT;
<INITIAL>"!"            return EXCLAMATION;
<INITIAL>"."            return DOT;
<INITIAL>[A-Za-z_][A-Za-z_0-9]* {
    yylval.identifier = crb_create_identifier(yytext);
    return IDENTIFIER;
}
<INITIAL>[1-9][0-9]* {
    Expression  *expression = crb_alloc_expression(INT_EXPRESSION);
    sscanf(yytext, "%d", &expression->u.int_value);
    yylval.expression = expression;
    return INT_LITERAL;
}
<INITIAL>"0" {
    Expression  *expression = crb_alloc_expression(INT_EXPRESSION);
    expression->u.int_value = 0;
    yylval.expression = expression;
    return INT_LITERAL;
}
<INITIAL>[0-9]+\.[0-9]+ {
    Expression  *expression = crb_alloc_expression(DOUBLE_EXPRESSION);
    sscanf(yytext, "%lf", &expression->u.double_value);
    yylval.expression = expression;
    return DOUBLE_LITERAL;
}
<INITIAL>\" {
    crb_open_string_literal();
    BEGIN STRING_LITERAL_STATE;
}
<INITIAL>%%r. {
    crb_set_regexp_start_char(yytext[3]);
    crb_open_string_literal();
    BEGIN REGEXP_LITERAL_STATE;
}
<INITIAL>[ \t] ;
<INITIAL>[ \t\r\n] {increment_line_number();}
<INITIAL>#     BEGIN COMMENT;
<INITIAL>.      {
    crb_compile_error(CHARACTER_INVALID_ERR,
                      CRB_CHARACTER_MESSAGE_ARGUMENT, "bad_char", yytext[0],
                      CRB_MESSAGE_ARGUMENT_END);
}
<COMMENT>\n     {
    increment_line_number();
    BEGIN INITIAL;
}
<COMMENT>.      ;
<STRING_LITERAL_STATE>\"        {
    Expression *expression = crb_alloc_expression(STRING_EXPRESSION);
    expression->u.string_value = crb_close_string_literal();
    yylval.expression = expression;
    BEGIN INITIAL;
    return STRING_LITERAL;
}
<STRING_LITERAL_STATE>\n        {
    crb_add_string_literal('\n');
    increment_line_number();
}
<STRING_LITERAL_STATE>\\\"      crb_add_string_literal('"');
<STRING_LITERAL_STATE>\\n       crb_add_string_literal('\n');
<STRING_LITERAL_STATE>\\t       crb_add_string_literal('\t');
<STRING_LITERAL_STATE>\\\\      crb_add_string_literal('\\');
<STRING_LITERAL_STATE>.         {
    Encoding enc = crb_get_current_interpreter()->source_encoding;
    crb_add_string_literal(yytext[0]);
    if (enc == SHIFT_JIS_ENCODING
        && ((((unsigned char*)yytext)[0] >= 0x81
             && ((unsigned char*)yytext)[0] <= 0x9e)
            || (((unsigned char*)yytext)[0] >= 0xe0
                && ((unsigned char*)yytext)[0] <= 0xef))) {
        BEGIN SHIFT_JIS_2ND_CHAR;
     }
}
<SHIFT_JIS_2ND_CHAR>. {
    crb_add_string_literal(yytext[0]);
    BEGIN STRING_LITERAL_STATE;
}
<REGEXP_LITERAL_STATE>\n {
    crb_compile_error(CR_IN_REGEXP_ERR, CRB_MESSAGE_ARGUMENT_END);
}
<REGEXP_LITERAL_STATE>. {
    if (yytext[0] != crb_regexp_start_char()) {
        crb_add_string_literal(yytext[0]);
    } else {
        Expression *expression = crb_alloc_expression(REGEXP_EXPRESSION);
        CRB_Char *string_literal = crb_close_string_literal();
        expression->u.regexp_value
            = crb_create_regexp_in_compile(string_literal);
        yylval.expression = expression;
        BEGIN INITIAL;
        return REGEXP_LITERAL;
    }
}
%%
