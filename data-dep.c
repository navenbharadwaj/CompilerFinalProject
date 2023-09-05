//
//  data-dep.c
//  Compiler Final Project
//
// Naveen Bharadwaj

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc.h"
#include "data-dep.h"

#define MAX_NUM_STATEMENTS    512
#define MAX_NUM_DEPENDS        128

typedef struct Statement_struct
{
    
    char written[MAX_USR_VAR_NAME_LEN + 1];
    char read1[MAX_USR_VAR_NAME_LEN + 1];
    char read2[MAX_USR_VAR_NAME_LEN + 1];

    int flow_deps[MAX_NUM_DEPENDS];
    int num_flow_deps;

    int anti_deps[MAX_NUM_DEPENDS];
    int num_anti_deps;

    int write_deps[MAX_NUM_DEPENDS];
    int num_write_deps;

    int dd_ifelse_id;
    int dd_ifelse_depth;
    int dd_inside_if;
} Statement;


Statement stmt_dep_array[MAX_NUM_STATEMENTS];
int num_stmts = 0;

int if_else_id_counter = 0;

void _dd_append_to_depend_array(int index, int to_append, int type)
{
  
    if(index > num_stmts)
    {
        printf("Index %d out of bounds for num_stmts\n", index);
        return;
    }

    int end;

    switch(type)
    {
        case FLOW:
            if(stmt_dep_array[index].num_flow_deps >= MAX_NUM_DEPENDS)
            {
                printf("Statement %d exceeded max number of flow dependencies\n", index);
                exit(1);
            }

            end = stmt_dep_array[index].num_flow_deps;
            stmt_dep_array[index].flow_deps[end] = to_append;
            stmt_dep_array[index].num_flow_deps++;

            break;
        case ANTI:
            if(stmt_dep_array[index].num_anti_deps >= MAX_NUM_DEPENDS)
            {
                printf("Statement %d exceeded max number of anti dependencies\n", index);
                exit(1);
            }

            end = stmt_dep_array[index].num_anti_deps;
            stmt_dep_array[index].anti_deps[end] = to_append;
            stmt_dep_array[index].num_anti_deps++;

            break;
        case WRITE:
            if(stmt_dep_array[index].num_write_deps >= MAX_NUM_DEPENDS)
            {
                printf("Statement %d exceeded max number of write dependencies\n", index);
                exit(1);
            }

            end = stmt_dep_array[index].num_write_deps;
            stmt_dep_array[index].write_deps[end] = to_append;
            stmt_dep_array[index].num_write_deps++;
            break;
        default:
            printf("ERROR Unknown case for _dd_append_to_depend_array\n");
    }

    return;
}


int _dd_verify_ifelse_dependence(char *var_name, int statement_num, int type)
{
    

    int is_checker_in_else = (stmt_dep_array[num_stmts].dd_ifelse_depth != 0) && !stmt_dep_array[num_stmts].dd_inside_if;
    int other_is_equal_or_deeper = stmt_dep_array[statement_num].dd_ifelse_depth >= stmt_dep_array[num_stmts].dd_ifelse_depth;
    int in_same_nest = stmt_dep_array[statement_num].dd_ifelse_id == stmt_dep_array[num_stmts].dd_ifelse_id;

    if(in_same_nest && is_checker_in_else && other_is_equal_or_deeper)
    {
        
        return 0;
    }

    int assigned_in_matching_ifelse = 0;
  
    int i;
    for(i = statement_num + 1; i < num_stmts; i++)
    {
        
        int is_assigned_later = strcmp(stmt_dep_array[i].written, var_name) == 0;
        in_same_nest = stmt_dep_array[i].dd_ifelse_id == stmt_dep_array[statement_num].dd_ifelse_id;
        int has_less_eq_depth = stmt_dep_array[i].dd_ifelse_depth <= stmt_dep_array[statement_num].dd_ifelse_depth;
        int is_in_else = (stmt_dep_array[i].dd_ifelse_depth != 0) && !stmt_dep_array[i].dd_inside_if;

      
        if(is_assigned_later && in_same_nest && has_less_eq_depth)
        {
            int on_same_level = stmt_dep_array[i].dd_ifelse_depth == stmt_dep_array[statement_num].dd_ifelse_depth;
            
            if(is_in_else && on_same_level)
            {
                assigned_in_matching_ifelse = 1;
          
            }
            
            if(is_in_else)
            {
               
                continue;
            }
            else
            {
            
                return 0;
            }
        }

        if(is_assigned_later && in_same_nest && is_in_else)
        {
            int is_one_deeper = (stmt_dep_array[i].dd_ifelse_depth - 1) == stmt_dep_array[statement_num].dd_ifelse_depth;

            if(is_one_deeper)
            {
                
                return 0;
            }
        }
    }

    _dd_append_to_depend_array(num_stmts, statement_num, type);

    in_same_nest = stmt_dep_array[statement_num].dd_ifelse_id == stmt_dep_array[num_stmts].dd_ifelse_id;
    if((type == FLOW || type == WRITE) && in_same_nest)
    {
        
        int dependency_is_le_depth = stmt_dep_array[statement_num].dd_ifelse_depth <= stmt_dep_array[num_stmts].dd_ifelse_depth;

        if(dependency_is_le_depth)
        {
            
            return 1;
        }
        else if(assigned_in_matching_ifelse)
        {
            
            return 1;
        }
    }

    return 0;
}


void _dd_check_for_dependecies(char *var_name, int written)
{
    
    int i;
    for(i = num_stmts - 1; i >= 0 ; i--)
    {
      
        if(written)
        {
            
            if(strcmp(var_name, stmt_dep_array[i].written) == 0)
            {
                
                if(stmt_dep_array[i].dd_ifelse_depth == 0)
                {
                    _dd_append_to_depend_array(num_stmts, i, WRITE);
                    break;
                }
                else if(stmt_dep_array[i].dd_ifelse_depth == 1 && !stmt_dep_array[i].dd_inside_if)
                {
                    _dd_append_to_depend_array(num_stmts, i, WRITE);
                    
                    int same_var = strcmp(var_name, stmt_dep_array[i - 1].written) == 0;
                    int same_depth = stmt_dep_array[i - 1].dd_ifelse_depth == 1;
                    int same_nest =  stmt_dep_array[i].dd_ifelse_id == stmt_dep_array[i - 1].dd_ifelse_id;
                    
                    if(same_var && same_depth && same_nest)
                    {
                        
                        _dd_append_to_depend_array(num_stmts, i - 1, WRITE);
                        break;
                    }
                }
                else
                {
                    int blocking = _dd_verify_ifelse_dependence(var_name, i, WRITE);
                    if(blocking)
                    {
                        break;
                    }
                }
            }
            else if(strcmp(var_name, stmt_dep_array[i].read1) == 0
                 || strcmp(var_name, stmt_dep_array[i].read2) == 0)
            {
                
                if(stmt_dep_array[i].dd_ifelse_depth == 0)
                {
                    _dd_append_to_depend_array(num_stmts, i, ANTI);
                }
                else
                {
                    _dd_verify_ifelse_dependence(var_name, i, ANTI);
                }
               
            }
        }
        else
        {
           
            if(strcmp(var_name, stmt_dep_array[i].written) == 0)
            {
                
                if(stmt_dep_array[i].dd_ifelse_depth == 0)
                {
                    _dd_append_to_depend_array(num_stmts, i, FLOW);
                    break;
                }
                else if(stmt_dep_array[i].dd_ifelse_depth == 1 && !stmt_dep_array[i].dd_inside_if)
                {
                    _dd_append_to_depend_array(num_stmts, i, FLOW);
                    
                    int same_var = strcmp(var_name, stmt_dep_array[i - 1].written) == 0;
                    int same_depth = stmt_dep_array[i - 1].dd_ifelse_depth == 1;
                    int same_nest =  stmt_dep_array[i].dd_ifelse_id == stmt_dep_array[i - 1].dd_ifelse_id;
                    
                    if(same_var && same_depth && same_nest)
                    {
                      
                        _dd_append_to_depend_array(num_stmts, i - 1, FLOW);
                        break;
                    }
                }
                else
                {
                    int blocking = _dd_verify_ifelse_dependence(var_name, i, FLOW);
                    if(blocking)
                    {
                        break;
                    }
                }
            }
        }
    }

    return;
}


void dd_left_ifelse_nest()
{
    if_else_id_counter++;

    return;
}


void dd_record_and_process(char *written, char *read1, char *read2, int dd_ifelse_depth, int dd_inside_if)
{
    if(num_stmts >= MAX_NUM_STATEMENTS)
    {
        printf("Max number of statements for data dependence exceeded (MAX=%d)\n", MAX_NUM_STATEMENTS);
        exit(1);
    }

    stmt_dep_array[num_stmts].num_flow_deps = 0;
    stmt_dep_array[num_stmts].num_anti_deps = 0;
    stmt_dep_array[num_stmts].num_write_deps = 0;
    stmt_dep_array[num_stmts].dd_ifelse_depth = dd_ifelse_depth;
    stmt_dep_array[num_stmts].dd_inside_if = dd_inside_if;

    if(dd_ifelse_depth > 0)
    {
        stmt_dep_array[num_stmts].dd_ifelse_id = if_else_id_counter;
    }
    else
    {
        stmt_dep_array[num_stmts].dd_ifelse_id = -1;
        stmt_dep_array[num_stmts].dd_inside_if = -1;
    }

    if(written != NULL)
    {
        strcpy(stmt_dep_array[num_stmts].written, written);
        _dd_check_for_dependecies(written, 1);
    }
    else
    {
        strcpy(stmt_dep_array[num_stmts].written, "");
    }

    
    if(read1 != NULL && read1[0] >= 'A')
    {
        strcpy(stmt_dep_array[num_stmts].read1, read1);
        _dd_check_for_dependecies(read1, 0);
    }
    else
    {
        strcpy(stmt_dep_array[num_stmts].read1, "");
    }

  
    if(read2 != NULL && read2[0] >= 'A')
    {
        strcpy(stmt_dep_array[num_stmts].read2, read2);
        _dd_check_for_dependecies(read2, 0);
    }
    else
    {
        strcpy(stmt_dep_array[num_stmts].read2, "");
    }

    num_stmts++;

    return;
}


void dd_print_out_dependencies()
{
    printf("Printing dependencies:\n");
    int i;
    for(i = 0; i < num_stmts; i++)
    {
        printf("S%d:\nFlow-dependence: ", i);
        if (stmt_dep_array[i].num_flow_deps == 0){
            printf("None");
        }
        else
        {
            int j;
            for(j = 0; j < stmt_dep_array[i].num_flow_deps; j++)
            {
                printf("S%d ", stmt_dep_array[i].flow_deps[j]);
            }
        }

        printf("\nAnti-dependence: ");
        if (stmt_dep_array[i].num_anti_deps == 0){
            printf("None");
        }
        else
        {
            int j;
            for(j = 0; j < stmt_dep_array[i].num_anti_deps; j++)
            {
                printf("S%d ", stmt_dep_array[i].anti_deps[j]);
            }
        }

        printf("\nWrite-dependence: ");
        if (stmt_dep_array[i].num_write_deps == 0){
            printf("None");
        }
        else
        {
            int j;
            for(j = 0; j < stmt_dep_array[i].num_write_deps; j++)
            {
                printf("S%d ", stmt_dep_array[i].write_deps[j]);
            }
        }

        printf("\n");
    }

    return;
}

