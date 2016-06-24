%{

#include <stdio.h>
#include <string.h>
#include "DBG.h"
#include "hobby.h"
#include "y.tab.h"

int yywrap(void){
	return 1;
}

static void increment_line_number(){
	hbb_get_current_interpreter() -> current_line_number++;
}

%}

%start COMMENT STRING_LITERAL_STATE

%%

<INITIAL>"function" return FUNCTION;
<INITIAL>"if"		return IF;
<INITIAL>"else"		return ELSE;
<INITIAL>"elseif"	return ELSEIF;
<INITIAL>"while"	return WHILE;
<INITIAL>"for"		return FOR;
<INITIAL>"return"	return RETURN_T;
<INITIAL>"break"	return BREAK;
<INITIAL>"continue"	return CONTINUE;
<INITIAL>"null"		return NULL_T;
<INITIAL>"true"		return TRUE_T;
<INITIAL>"false" 	return FALSE_T;
<INITIAL>"global"	return GLOBAL_T;
<INITIAL>"("		return LP;
<INITIAL>")"		return RP;
<INITIAL>"{"		return LC;
<INITIAL>"}"		return RC;
<INITIAL>";"		return SEMICOLON;
<INITIAL>","		return COMMA;
<INITIAL>"&&"		return LOGICAL_AND;
<INITIAL>"||"		return LOGICAL_OR;
<INITIAL>"="		return ASSIGN;
<INITIAL>"=="		return EQ;
<INITIAL>"!="		return NE;
<INITIAL>">"		return GT;
<INITIAL>">="		return GE;
<INITIAL>"<"		return LT;
<INITIAL>"<="		return LE;
<INITIAL>"+"		return ADD;
<INITIAL>"-"		return SUB;
<INITIAL>"*"		return MUL;
<INITIAL>"/"		return DIV;
<INITIAL>"%"		return MOD;

	/* ID定义 */
<INITIAL>[A-Za-z_][A-Za-z_0-9]* {
	yylval.identifier = hbb_create_identifier(yytext);
	return IDENTIFIER;
}		

<INITIAL>([1-9][0-9]*)|"0" {
	Expression *expression = hbb_alloc_expression(INT_EXPRESSION);
	sscanf(yytext,"%d",&expression->u.int_value);
	yylval.expression = expression;
	return INT_LITERAL;
}

<INITIAL>[0-9]+\.[0-9]+ {
	Expression *expression = hbb_alloc_expression(DOUBLE_EXPRESSION);
	sscanf(yytext,"%lf", &expression->u.double_value);
	yylval.expression = expression;
	return DOUBLE_LITERAL;
}

<INITIAL>\" {
	hbb_open_string_literal();
	BEGIN STRING_LITERAL_STATE;
}

<INITIAL>[ \t] ;

<INITIAL>\n {
	increment_line_number();
}

<INITIAL># BEGIN COMMENT;

<INITIAL>. {
	char buf[LINE_BUF_SIZE];

	if (isprint(yytext[0])){
		buf[0] = yytext[0];
		buf[1] = '\0';
	}else{
		sprintf(buf,"0x%02x",(unsigned char)yytext[0]);
	}

	hbb_compile_error(CHARACTER_INVALID_ERR,
		STRING_MESSAGE_ARGUMENT,"bad_char",buf,
		MESSAGE_ARGUMENT_END);
}


<COMMENT>\n {
	increment_line_number();
	BEGIN INITIAL;
}


<COMMENT>.  ;


	/* 处理字符状态的 结束 状态返回 */
<STRING_LITERAL_STATE>\"	{
	Expression *expression = hbb_alloc_expression(STRING_EXPRESSION);
	expression->u.string_value = hbb_close_string_literal();
	yylval.expression = expression;
	BEGIN INITIAL;
	return STRING_LITERAL;	
}

	/* 字符串处理\n */
<STRING_LITERAL_STATE>\n  {
	hbb_add_string_literal('\n');
	increment_line_number();
}

	/* 处理转移字符的方法 */
<STRING_LITERAL_STATE>\\\"   hbb_add_string_literal('"');
<STRING_LITERAL_STATE>\\n 	 hbb_add_string_literal('\n');
<STRING_LITERAL_STATE>\\t  	 hbb_add_string_literal('\t');
<STRING_LITERAL_STATE>\\\\	 hbb_add_string_literal('\\');
<STRING_LITERAL_STATE>.		 hbb_add_string_literal(yytext[0]);

%%
