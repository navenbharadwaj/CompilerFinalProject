//
//  cse.c
//  Compiler Final Project
//
//  Naveen Bharadwaj
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc.h"
#include "cse.h"

#define MAX_SUB_EXPRESSIONS 512

typedef struct Subexprssion{
    char expr_1[MAX_USR_VAR_NAME_LEN + 1];
    char op[MAX_USR_VAR_NAME_LEN + 1];
    char expr_2[MAX_USR_VAR_NAME_LEN + 1];

    int temp_var;

    int depth_created_in;
                            
    int valid;

} Subexpr;

Subexpr subexpr_table[MAX_SUB_EXPRESSIONS];
int num_sub_exprs;

int cse_next_temp_var_name;
FILE *cse_temp_tac_ptr;
FILE * cse_opt_tac_ptr;

int cse_line_num;
int cse_ifelse_depth;
int cse_changes_made;

void _cse_init_first_temp_var_name(char *input_file)
{
    FILE *input_file_ptr = fopen(input_file, "r");
    if(input_file_ptr == NULL)
    {
        printf("CSE couldn't open %s to find last assigned temp variable\n", input_file);
        exit(1);
    }

    
    cse_next_temp_var_name = 0;

    char line[MAX_USR_VAR_NAME_LEN * 4];
    while(fgets(line, MAX_USR_VAR_NAME_LEN * 4, input_file_ptr) != NULL)
    {
        if(strstr(line, "}") != NULL)
        {
            continue;
        }

        char *tok = strtok(line, " +-*/!=();\n");

        while(tok != NULL)
        {
            
            if(tok[0] == '_' && tok[1] == 'c')
            {
                int temp_name = atoi(tok + 2);

                if(temp_name >= cse_next_temp_var_name)
                {
                    cse_next_temp_var_name = temp_name + 1;
                }
            }

            tok = strtok(NULL, " +-*/!=();\n");
        }
    }

    fclose(input_file_ptr);

    

    return;
}


int _cse_record_subexpression(char *expr_1, char *op, char *expr_2, int reuse_this_temp)
{
    if(num_sub_exprs >= MAX_SUB_EXPRESSIONS)
    {
        printf("CSE module exceeded max number of recorded subexpressions (MAX=%d)\n", MAX_SUB_EXPRESSIONS);
        exit(1);
    }

    strcpy(subexpr_table[num_sub_exprs].expr_1, expr_1);
    strcpy(subexpr_table[num_sub_exprs].op, op);
    strcpy(subexpr_table[num_sub_exprs].expr_2, expr_2);
    subexpr_table[num_sub_exprs].depth_created_in = cse_ifelse_depth;
    subexpr_table[num_sub_exprs].valid = 1;

    if(reuse_this_temp == -1)
    {
        subexpr_table[num_sub_exprs].temp_var = cse_next_temp_var_name;
        cse_next_temp_var_name++;
    }
    else
    {
        
        subexpr_table[num_sub_exprs].temp_var = reuse_this_temp;
    }

    num_sub_exprs++;

    return cse_next_temp_var_name - 1;
}

int _cse_get_expression_index(char *expr_1, char *op, char *expr_2)
{
    int i;
    for(i = 0; i < num_sub_exprs; i++)
    {
        
        if(!subexpr_table[i].valid || strcmp(subexpr_table[i].op, op) != 0)
        {
            continue;
        }

        int one_one = strcmp(subexpr_table[i].expr_1, expr_1) == 0;
        int two_two = strcmp(subexpr_table[i].expr_2, expr_2) == 0;

        if(one_one && two_two)
        {
            return i;
        }
        else if(strcmp(op, "+") == 0 || strcmp(op, "*") == 0)
        {
           
            int one_two = strcmp(subexpr_table[i].expr_1, expr_2) == 0;
            int two_one = strcmp(subexpr_table[i].expr_2, expr_1) == 0;

            if(one_two && two_one)
            {
                return i;
            }
        }
    }

    return -1;
}

int _cse_used_again(char *opt_tac_name, char *expr_1, char *op, char *expr_2)
{
    FILE * temp_file_ptr = fopen(opt_tac_name, "r");
    if(temp_file_ptr == NULL)
    {
        printf("Couldn't open %s for checking number of cs usages after line %d\n", opt_tac_name, cse_line_num);
    }

    char line[MAX_USR_VAR_NAME_LEN * 4];
    int i = 0;

    
    while(fgets(line, MAX_USR_VAR_NAME_LEN * 4, temp_file_ptr) != NULL)
    {
        if(i == cse_line_num)
        {
            i++;
            break;
        }

        i++;
    }

    int temp_ifelse_context = cse_ifelse_depth;;

    if(strstr(line, "if(") != NULL)
    {
        temp_ifelse_context++;
    }
    
    while(fgets(line, MAX_USR_VAR_NAME_LEN * 4, temp_file_ptr) != NULL)
    {
        if(strstr(line, "}") != NULL)
        {
            if(strstr(line, "} else {") != NULL)
            {
                if(cse_ifelse_depth >= temp_ifelse_context)
                {
                    break;
                }

                temp_ifelse_context--;
            }

            i++;
            continue;
        }

        char *temp_expr_1;
        char *temp_op;
        char *temp_expr_2;
        int is_if_statement = 0;

        if(strstr(line, "if(") != NULL)
        {
            is_if_statement = 1;
            strtok(line, " (){\n");

            temp_expr_1 = strtok(NULL, " (){\n");
            temp_op = strtok(NULL, " (){\n");
            temp_expr_2 = strtok(NULL, " (){\n");
        }
        else
        {
            char *temp_assigned = strtok(line, " =;\n");
            int match1 = strcmp(temp_assigned, expr_1) == 0;
            int match2 = strcmp(temp_assigned, expr_2) == 0;

            if(match1 || match2)
            {
                break;
            }

            temp_expr_1 = strtok(NULL, " =;\n");
            temp_op = strtok(NULL, " =;\n");
            temp_expr_2 = strtok(NULL, " =;\n");
        }

        if(temp_expr_1 != NULL && temp_op != NULL && temp_expr_2 != NULL)
        {
            
            if(strcmp(op, temp_op) != 0)
            {
                i++;
                continue;
            }

            int one_one = strcmp(expr_1, temp_expr_1) == 0;
            int two_two = strcmp(expr_2, temp_expr_2) == 0;

            if(one_one && two_two)
            {
                fclose(temp_file_ptr);
                return 1;
            }
            else if(strcmp(op, "+") == 0 || strcmp(op, "*") == 0)
            {
                
                int one_two = strcmp(expr_1, temp_expr_2) == 0;
                int two_one = strcmp(expr_2, temp_expr_1) == 0;

                if(one_two && two_one)
                {
                    fclose(temp_file_ptr);
                    return 1;
                }
            }
        }

        if(is_if_statement)
        {
            temp_ifelse_context++;
        }

        i++;
    }


    fclose(temp_file_ptr);
    return 0;
}

void _cse_ifelse_context_invalidator(char *ifelse_line)
{
    if(strstr(ifelse_line, "if(") != NULL)
    {
        cse_ifelse_depth++;
            }
    else if(strstr(ifelse_line, "} else {") != NULL)
    {
        
        int i;
        for(i = 0; i < num_sub_exprs; i++)
        {
            
            if(!subexpr_table[i].valid)
            {
                continue;
            }

            if(subexpr_table[i].depth_created_in >= cse_ifelse_depth)
            {
                subexpr_table[i].valid = 0;
               
            }
        }

        cse_ifelse_depth--;
       
    }
    else
    {
        
    }

    return;
}

void _cse_invalidate_cs_with_assigned_var(char *assigned)
{
    int i;
    for(i = 0; i < num_sub_exprs; i++)
    {
        if(!subexpr_table[i].valid)
        {
            continue;
        }

        int match1 = strcmp(assigned, subexpr_table[i].expr_1) == 0;
        int match2 = strcmp(assigned, subexpr_table[i].expr_2) == 0;

        if(match1 || match2)
        {
            subexpr_table[i].valid = 0;
            
        }
    }

    return;
}


void _cse_process_tac_line(char *tac_line, char *opt_tac_name)
{
    
    if(strstr(tac_line, "}") != NULL)
    {
        fprintf(cse_temp_tac_ptr, "%s", tac_line);

       
        _cse_ifelse_context_invalidator(tac_line);

        return;
    }

    char temp[MAX_USR_VAR_NAME_LEN * 4];
    strcpy(temp, tac_line);

    char empty[1] = "\0";

    char *assigned;
    char *expr_1;
    char *op;
    char *expr_2;

    
    if(strstr(tac_line, "if(") != NULL)
    {
        assigned = empty;
        strtok(temp, " (){\n");

        expr_1 = strtok(NULL, " (){\n");
        op = strtok(NULL, " (){\n");
        expr_2 =strtok(NULL, " (){\n");
    }
    else
    {
        
        assigned = strtok(temp, " =;\n");
        expr_1 = strtok(NULL, " =;\n");
        op = strtok(NULL, " =;\n");
        expr_2 = strtok(NULL, " =;\n");
    }

    
    if(expr_1 != NULL && op != NULL && expr_2 != NULL)
    {
        int index = _cse_get_expression_index(expr_1, op, expr_2);

        if(index == -1)
        {
  
            int used_again = _cse_used_again(opt_tac_name, expr_1, op, expr_2);
            
            if(used_again)
            {
                int temp_var_to_use;

                
                if(strstr(assigned, "_c") != NULL)
                {
                    temp_var_to_use = atoi(assigned + 2);
                    _cse_record_subexpression(expr_1, op, expr_2, temp_var_to_use);

                
                    fprintf(cse_temp_tac_ptr, "%s", tac_line);

                  
                }
                else
                {
                    temp_var_to_use = _cse_record_subexpression(expr_1, op, expr_2, -1);

                    
                    fprintf(cse_temp_tac_ptr, "_c%d = %s %s %s;\n", temp_var_to_use, expr_1, op, expr_2);
                    
                    if(strstr(tac_line, "if(") != NULL)
                    {
                        fprintf(cse_temp_tac_ptr, "if(_c%d) {\n", temp_var_to_use);
                    }
                    else
                    {
                        
                        fprintf(cse_temp_tac_ptr, "%s = _c%d;\n", assigned, temp_var_to_use);
                    }

              
                    cse_changes_made++;
                }
            }
            else
            {
                
                fprintf(cse_temp_tac_ptr, "%s", tac_line);
            }
        }
        else
        {
            
            int temp_var_to_use = subexpr_table[index].temp_var;

            if(strstr(tac_line, "if(") != NULL)
            {
                fprintf(cse_temp_tac_ptr, "if(_c%d) {\n", temp_var_to_use);
            }
            else
            {
                fprintf(cse_temp_tac_ptr, "%s = _c%d;\n", assigned, temp_var_to_use);
            }

            
            cse_changes_made++;
        }
    }
    else
    {
       
        fprintf(cse_temp_tac_ptr, "%s", tac_line);
    }

    if(strstr(tac_line, "if(") != NULL)
    {
        
        _cse_ifelse_context_invalidator(tac_line);
    }
    else
    {

        _cse_invalidate_cs_with_assigned_var(assigned);
    }

    return;
}


int cse_do_optimization(char *opt_tac_name, char *temp_tac_name)
{
   
    num_sub_exprs = 0;
    cse_next_temp_var_name = 0;
    cse_line_num = 0;
    cse_ifelse_depth = 0;
    cse_changes_made = 0;

    cse_temp_tac_ptr = fopen(temp_tac_name, "w");
    if(cse_temp_tac_ptr == NULL)
    {
        printf("CSE couldn't open %s for writing next optimization to\n", temp_tac_name);
        exit(1);
    }

    cse_opt_tac_ptr = fopen(opt_tac_name, "r");
    if(cse_opt_tac_ptr == NULL)
    {
        printf("CSE couldn't open %s for reading in TAC\n", opt_tac_name);
        exit(1);
    }

    _cse_init_first_temp_var_name(opt_tac_name);

    char line[MAX_USR_VAR_NAME_LEN * 4];
    while(fgets(line, MAX_USR_VAR_NAME_LEN * 4, cse_opt_tac_ptr) != NULL)
    {
        _cse_process_tac_line(line, opt_tac_name);
        cse_line_num++;
    }

    fclose(cse_temp_tac_ptr);
    fclose(cse_opt_tac_ptr);

    copy_to_file(opt_tac_name, temp_tac_name);

    return cse_changes_made;
}

