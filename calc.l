{\rtf1\ansi\ansicpg1252\cocoartf2638
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fnil\fcharset0 AndaleMono;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\paperw11900\paperh16840\margl1440\margr1440\vieww11520\viewh8400\viewkind0
\pard\tx566\tx1133\tx1700\tx2267\tx2834\tx3401\tx3968\tx4535\tx5102\tx5669\tx6236\tx6803\pardirnatural\partightenfactor0

\f0\fs24 \cf0 \
// Naveen Bharadwaj\
\
#include <stdlib.h> \
#include <string.h>\
#include "calc.tab.h"\
\
// #define DEBUG 			\
\
extern int flex_line_num; 	\
extern FILE * yyin;		\
void yyerror(char *);	\
\
// Options below exclude unused input and yyunput functions\
%\}\
\
%option noinput	\
%option nounput 	\
\
%%\
\
[A-Za-z][A-Za-z0-9]* \{	\
	#ifdef DEBUG\
	printf("token %s at line %d\\n", yytext, flex_line_num);\
	#endif\
\
	yylval.str = strdup(yytext); \
	return VARIABLE;\
	\}\
\
[0-9]+ \{\
	#ifdef DEBUG\
	printf("token %s at line %d\\n", yytext, flex_line_num);\
	#endif\
\
	yylval.str = strdup(yytext); \
	return INTEGER;\
	\}\
\
"**" \{\
	#ifdef DEBUG\
	printf("token %s at line %d\\n", yytext, flex_line_num);\
	#endif\
\
	return POWER;\
	\}\
\
[-+()=*/!?]	\{\
		#ifdef DEBUG\
		printf("token %s at line %d\\n", yytext, flex_line_num);\
		#endif\
\
		return *yytext;		\
	\}\
\
"\\n" \{\
	#ifdef DEBUG\
	printf("token \\\\n at line %d\\n", flex_line_num);\
	#endif\
\
	flex_line_num++;\
	return *yytext;\
	\}\
\
[ \\t]+ \{\
	#ifdef DEBUG\
	printf("token is whitespace(s) at line %d\\n", flex_line_num);\
	#endif\
	\
	\}\
\
.	\{ yyerror("invalid character"); \}\
\
%%\
\
int yywrap() \{ /* need this to avoid link problem */\
	return 1;\
\}\
}