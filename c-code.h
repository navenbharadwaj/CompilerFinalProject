//
//  c-code.h
//  Compiler Final Project
// Naveen Bharadwaj

#define NUM_ITERS 500000
#define MAX_NUM_TEMP_VARS 1024

void init_c_code();
void track_user_var(char *var, int assigned);
void gen_c_code(char * input, char * output, int timing);
