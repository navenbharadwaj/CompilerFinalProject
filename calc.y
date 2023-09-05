%{
// Naveen Bharadwaj

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calc.h"
#include "c-code.h"
#include "cse.h"
#include "data-dep.h"
#include "copy-stmt-elim.h"

int yylex(void);					

void my_free(char *ptr);
char* lc(char *str);
void gen_tac_assign(char *var, char *expr);
char* gen_tac_expr(char *one, char *op, char *three);
void gen_tac_if(char *cond_expr);
void gen_tac_else(char *expr);
void yyerror(const char *);

int do_gen_else = 0;				
int ifelse_depth = 0;					
int inside_if = 0;					
int temp_var_ctr = 0;				

int flex_line_num = 1;		
FILE * yyin;				
FILE * tac_file;		
%}

%define parse.error verbose		
%token INTEGER POWER VARIABLE	
								
%union
{
	int dval;
	char * str;
}

%type <str> INTEGER POWER VARIABLE


%type <str> expr

%right '='
%right '?'
%left '+' '-'
%left '*' '/'
%precedence '!'		
%right POWER		

%start calc

%%

calc :
	calc expr '\n'		{ my_free($2); gen_tac_else(NULL); }
	|
	;

expr :
	INTEGER				{ $$ = $1; }
	| VARIABLE        	{ $$ = lc($1); track_user_var(lc($1), 0); }
	| VARIABLE '=' expr	{ $$ = lc($1); gen_tac_assign(lc($1), $3); my_free($3); }
	| expr '+' expr		{ $$ = gen_tac_expr($1, "+", $3); my_free($1); my_free($3); }
	| expr '-' expr		{ $$ = gen_tac_expr($1, "-", $3); my_free($1); my_free($3); }
	| expr '*' expr		{ $$ = gen_tac_expr($1, "*", $3); my_free($1); my_free($3); }
	| expr '/' expr		{ $$ = gen_tac_expr($1, "/", $3); my_free($1); my_free($3); }
	| '!' expr			{ $$ = gen_tac_expr(NULL, "!", $2); my_free($2); }		
	| expr POWER expr	{ $$ = gen_tac_expr($1, "**", $3); my_free($1); my_free($3);}
	| '(' expr ')'		{ $$ = $2; }					
	| '(' expr ')' '?' { gen_tac_if($2); } '(' expr ')'
						{
							$$ = $7;
							do_gen_else++;	
							// printf("do_gen_else incremented by \"%s\": %d\n", $7, do_gen_else);
							my_free($2);
						}
	;

%%

void my_free(char * ptr)
{
	if(ptr == NULL)
	{
		yyerror("Tried to free null pointer!");
	}
	else
	{
		// printf("Freed token: %s\n", ptr);
		free(ptr);
	}

	return;
}


char* lc(char *str)
{
	int i;
	for (i = 0; i < strlen(str); i++)
	{
		str[i] = tolower(str[i]);
	}

	return str;
}

void gen_tac_assign(char *var, char *expr)
{
	track_user_var(var, 1);

	fprintf(tac_file, "%s = %s;\n", var, expr);
	// printf("WROTE OUT: %s", tac_buf);

	dd_record_and_process(var, expr, NULL, ifelse_depth, inside_if);

	gen_tac_else(var);

	return;
}

char* gen_tac_expr(char *one, char *op, char *three)
{
	char tmp_var_name[16]; 	// temp var names: _t0123456789

	
	sprintf(tmp_var_name, "_t%d", temp_var_ctr);
	temp_var_ctr++;

	if (one != NULL)
	{
	
		fprintf(tac_file, "%s = %s %s %s;\n", tmp_var_name, one, op, three);
		dd_record_and_process(tmp_var_name, one, three, ifelse_depth, inside_if);
	}
	else	
	{
		fprintf(tac_file, "%s = %s%s;\n", tmp_var_name, op, three);
		dd_record_and_process(tmp_var_name, NULL, three, ifelse_depth, inside_if);
	}

	return strdup(tmp_var_name);
}

void gen_tac_if(char *cond_expr)
{
	char buf[MAX_USR_VAR_NAME_LEN * 2];
	sprintf(buf, "if(%s) {\n", cond_expr);
	fprintf(tac_file, buf);

	dd_record_and_process(NULL, cond_expr, NULL, ifelse_depth, inside_if);


	ifelse_depth++;
	inside_if = 1;
	

	if(ifelse_depth > MAX_IFELSE_DEPTH)
	{
		char err_buf[128];
		sprintf(err_buf, "Max depth of if-statements exceeded (MAX=%d)", MAX_IFELSE_DEPTH);
		yyerror(err_buf);

		exit(1);
	}

	return;
}


void gen_tac_else(char *expr)
{
	

	for (; do_gen_else > 0; do_gen_else--)
	{
		inside_if = 0;
		
		if(expr != NULL)
		{
			fprintf(tac_file, "} else {\n%s = 0;\n}\n", expr);
			dd_record_and_process(expr, NULL, NULL, ifelse_depth, inside_if);
		}
		else
		{
			fprintf(tac_file, "} else {\n}\n");
		}

		
		ifelse_depth--;

		if(ifelse_depth > 0)
		{
			inside_if = 1;
			
		}
		else
		{
			
			inside_if = 0;
			
			dd_left_ifelse_nest();
		}
	}

	return;
}

void yyerror(const char *s)
{
	printf("%s\n", s);
}

void copy_to_file(char *dest_name, char *source_name)
{
	FILE *source_ptr = fopen(source_name, "r");
	FILE *dest_ptr = fopen(dest_name, "w");

	if(source_ptr == NULL)
	{
		printf("Couldn't open %s for copying to %s\n", source_name, dest_name);
		exit(1);
	}

	if(dest_ptr == NULL)
	{
		printf("Couldn't create %s\n", dest_name);
		exit(1);
	}

	char line[MAX_USR_VAR_NAME_LEN * 4];
	while(fgets(line, MAX_USR_VAR_NAME_LEN * 4, source_ptr) != NULL)
	{
		fprintf(dest_ptr, "%s", line);
	}

	fclose(source_ptr);
	fclose(dest_ptr);

	return;
}

int main(int argc, char *argv[])
{
	// Open the input program file
	if (argc != 2)
	{
		yyerror("Need to provide input file");
		exit(1);
	}
	else
	{
		yyin = fopen(argv[1], "r");
		if(yyin == NULL)
		{
			yyerror("Couldn't open input file");
			exit(1);
		}
	}

	
	char *frontend_tac_name = "Output/tac-frontend.txt";
	tac_file = fopen(frontend_tac_name, "w");

	if (tac_file == NULL)
	{
		yyerror("Couldn't create TAC file");
		exit(1);
	}

	init_c_code();	

	yyparse();	

	
	fclose(yyin);
	fclose(tac_file);

	dd_print_out_dependencies();	


	char *opt_tac_name = "Output/tac-opt.txt";
	copy_to_file(opt_tac_name, frontend_tac_name);

	char *temp_tac_name = "Output/tac-opt-temp.txt";
	int cse_changes = 0;
	int cpt_st_changes = 0;
	int num_opt_loops = 0;

	do
	{
		cse_changes = cse_do_optimization(opt_tac_name, temp_tac_name);
		cpt_st_changes = cp_st_do_optimization(opt_tac_name, temp_tac_name);
		num_opt_loops++;

	} while(cse_changes > 0 || cpt_st_changes > 0);

	printf("\nPerformed %d optimization loops\n", num_opt_loops);

	gen_c_code(frontend_tac_name, "Output/backend.c", 0);
	gen_c_code(frontend_tac_name, "Output/backend-timing.c", 1);
	gen_c_code(opt_tac_name, "Output/backend-opt.c", 0);
	gen_c_code(opt_tac_name, "Output/backend-opt-timing.c", 1);

	return 0;
}
