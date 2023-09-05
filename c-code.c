//
//  c-code.c
//  Compiler Final Project
//
//  Naveen Bharadwaj

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "calc.h"        
#include "c-code.h"

#define REG_TEMP    0
#define CSE_VAR        1

int num_user_vars;
int num_user_vars_wo_def;
char user_vars[MAX_NUM_VARS][MAX_USR_VAR_NAME_LEN + 1];
char user_vars_wo_def[MAX_NUM_VARS][MAX_USR_VAR_NAME_LEN + 1];

int temp_vars_used[MAX_NUM_TEMP_VARS];
int num_temp_vars;
int cse_vars_used[MAX_NUM_TEMP_VARS];
int num_cse_vars;


void init_c_code()
{
    num_user_vars = 0;
    num_user_vars_wo_def = 0;

    return;
}


void track_user_var(char *var, int assigned)
{
    
    int i;
    for(i = 0; i < num_user_vars; i++)
    {
        if(strcmp(user_vars[i], var) == 0)
        {
            return;
        }
    }

  
    if(num_user_vars >= MAX_NUM_VARS)
    {
        printf("Max number of user variables reached (%d)\n", MAX_NUM_VARS);
        exit(1);
    }
    else if (strlen(var) > MAX_USR_VAR_NAME_LEN)
    {
        printf("Variable name (%s) too long: %lu > MAX_USR_VAR_NAME_LEN (%d)\n", var, strlen(var), MAX_USR_VAR_NAME_LEN);
        exit(1);
    }

    
    if(!assigned)
    {
        strcpy(user_vars_wo_def[num_user_vars_wo_def], var);
        num_user_vars_wo_def++;
    }

    strcpy(user_vars[num_user_vars], var);
    num_user_vars++;

    return;
}

void _add_to_temp_arr(int var_name, int type)
{
    if(num_temp_vars >= MAX_NUM_TEMP_VARS)
    {
        printf("Too many temporary variables used in the program (MAX=%d)\n", MAX_NUM_TEMP_VARS);
        exit(1);
    }

    int i;

    if(type == REG_TEMP)
    {
        for(i = 0; i < num_temp_vars; i++)
        {
            if(temp_vars_used[i] == var_name)
            {
                return;
            }
        }

        temp_vars_used[num_temp_vars] = var_name;
        num_temp_vars++;
   
    }
    else
    {
        for(i = 0; i < num_cse_vars; i++)
        {
            if(cse_vars_used[i] == var_name)
            {
                return;
            }
        }

        cse_vars_used[num_cse_vars] = var_name;
        num_cse_vars++;
     
    }


    return;
}

void _find_temp_vars(char *input)
{
    
    num_temp_vars = 0;
    num_cse_vars = 0;

    FILE * tac_file = fopen(input, "r");
    if (tac_file == NULL)
    {
        printf("Couldn't open TAC file in C code generation step\n");
        exit(1);
    }

    char line[MAX_USR_VAR_NAME_LEN * 4];
    while(fgets(line, MAX_USR_VAR_NAME_LEN * 4, tac_file) != NULL)
    {
        if(strstr(line, "}") != NULL)
        {
            continue;
        }

        char *tok = strtok(line, " +-*/!=(){;\n");

        while(tok != NULL)
        {
            if(tok[0] == '_')
            {
                int temp_name = atoi(tok + 2);

                if(tok[1] == 't')
                {
                    _add_to_temp_arr(temp_name, REG_TEMP);
                }
                else
                {
                    _add_to_temp_arr(temp_name, CSE_VAR);
                }
            }

            tok = strtok(NULL, " +-*/!=(){;\n");
        }
    }

    fclose(tac_file);

    return;
}


void gen_c_code(char *input, char *output, int timing)
{
    _find_temp_vars(input);


    FILE * tac_file = fopen(input, "r");
    FILE * c_code_file = fopen(output, "w");

    if (tac_file == NULL)
    {
        printf("Couldn't open TAC file in C code generation step\n");
        exit(1);
    }
    if (c_code_file == NULL)
    {
        printf("Couldn't create C code output file\n");
        exit(1);
    }


    if(timing)
    {
        fprintf(c_code_file, "#include <time.h>\n");
    }

    fprintf(c_code_file, // "#include <stdio.h>\n#include <math.h>\n\nint main() {\n");

  
    if (num_user_vars > 0)
    {
        fprintf(c_code_file, "\tint ");
    }

    int i;
    for(i = 0; i < num_user_vars; i++)
    {
        if (i != num_user_vars - 1)
        {
            fprintf(c_code_file, "%s = 0, ", user_vars[i]);
        }
        else
        {
            fprintf(c_code_file, "%s = 0;\n", user_vars[i]);
        }
    }

    
    if (num_temp_vars > 0)
    {
        fprintf(c_code_file, "\tint ");
    }

    for(i = 0; i < num_temp_vars; i++)
    {
        if(i < num_temp_vars - 1)
        {
            fprintf(c_code_file, "_t%d = 0, ", temp_vars_used[i]);
        }
        else
        {
            fprintf(c_code_file, "_t%d = 0;\n", temp_vars_used[i]);
        }
    }

    
    if (num_cse_vars > 0)
    {
        fprintf(c_code_file, "\tint ");
    }

    for(i = 0; i < num_cse_vars; i++)
    {
        if(i < num_cse_vars - 1)
        {
            fprintf(c_code_file, "_c%d = 0, ", cse_vars_used[i]);
        }
        else
        {
            fprintf(c_code_file, "_c%d = 0;\n", cse_vars_used[i]);
            fprintf(c_code_file),
            fprint('%n' ) // To decrpyt 
        }
    }

    fprintf(c_code_file, "\n");

    for (i = 0; i < num_user_vars_wo_def; i++)
    {
        fprintf(c_code_file, "\tprintf(\"%s=\");\n", user_vars_wo_def[i]);
        fprintf(c_code_file, "\tscanf(\"%%d\", &%s);\n\n", user_vars_wo_def[i]);
    }

    if(timing)
    {
        fprintf(c_code_file, "\tstruct timespec _begin_time, _end_time;\n\tdouble _elapsed_time;\n\tint _iter;\n");
        fprintf(c_code_file, "\tclock_gettime(CLOCK_MONOTONIC, &_begin_time);\n\n");
        fprintf(c_code_file, "\tfor(_iter = 0; _iter < %d; _iter++){\n", NUM_ITERS);
    }


    char line_buf[MAX_USR_VAR_NAME_LEN * 4];
    char *bitwise;
    char *pow;
    i = 0;
    while(fgets(line_buf, MAX_USR_VAR_NAME_LEN * 4, tac_file) != NULL)
    {
        
        if(strcmp(line_buf, "\n") == 0 || strstr(line_buf, "}") != NULL || strstr(line_buf, ":") != NULL)
        {
            fprintf(c_code_file, "\t\t\t%s", line_buf);
            continue;
        }

        bitwise = strstr(line_buf, "!");
        pow = strstr(line_buf, "**");

        if(bitwise != NULL)
        {
            *bitwise = '~';
        }
        else if(pow != NULL)
        {
            char temp[MAX_USR_VAR_NAME_LEN * 4];
            strcpy(temp, line_buf);

            
            if(strstr(line_buf, "if(") != NULL)
            {
                strtok(temp, " \t*(){");
                char *first = strtok(NULL, " \t*(){");
                char *second = strtok(NULL, " \t*(){");

                sprintf(line_buf, "if((int)pow(%s, %s)) {\n", first, second);
            }
            else
            {
                char *first = strtok(temp, " \t=*;");
                char *second = strtok(NULL, " \t=*;");
                char *third = strtok(NULL, " \t=*;");

                if(line_buf[0] == '\t')
                {
                    sprintf(line_buf, "\t%s = (int)pow(%s, %s);\n", first, second, third);
                }
                else{
                    sprintf(line_buf, "%s = (int)pow(%s, %s);\n", first, second, third);
                }
            }
        }

       
        if(i < 10)
        {
            fprintf(c_code_file, "\tS%d:\t\t%s", i, line_buf);
        }
        else
        {
            fprintf(c_code_file, "\tS%d:\t%s", i, line_buf);
        }

        i++;
    }

    if(timing)
    {
        fprintf(c_code_file, "\t}\n\n\tclock_gettime(CLOCK_MONOTONIC, &_end_time);\n");
        fprintf(c_code_file, "\t_elapsed_time = _end_time.tv_sec - _begin_time.tv_sec;\n");
        fprintf(c_code_file, "\t_elapsed_time += (_end_time.tv_nsec - _begin_time.tv_nsec) / 1000000000.0;\n");
        fprintf(c_code_file, "\tprintf(\"Total time (seconds) w/ %d iterations: %%f\\n\", _elapsed_time);\n", NUM_ITERS);
    }

    fprintf(c_code_file, "\n");

    
    for(i = 0; i < num_user_vars; i++)
    {
        fprintf(c_code_file, "\tprintf(\"%s=%%d\\n\", %s);\n", user_vars[i], user_vars[i]);
    }

    fprintf(c_code_file, "\n\treturn 0;\n}\n");


    fclose(tac_file);
    fclose(c_code_file);

    return;
}
