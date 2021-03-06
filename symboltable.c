
/*
   This is a very simple c compiler written by Prof. Jenq Kuen Lee,
   Department of Computer Science, National Tsing-Hua Univ., Taiwan,
   Fall 1995.

   This is used in compiler class.
   This file contains Symbol Table Handling.

*/

#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include "symboltable.h"

extern FILE *f_asm;
int cur_counter = 0;
int cur_fcounter = 0;
int cur_scope   = 1;
int stack_cur_offset = 0;
int l_stack[1000] = {0};
int s_top = 0;
char *copys(char* s);





void push_lstack(int e){
	l_stack[s_top] = e;
	s_top++;
}

int pop_lstack(){
	s_top--;
	return l_stack[s_top];
}


void init_ftable(){
	int i,j;
	for(i=0;i<300;i++){
		ftable[i].declared = 0;
		ftable[i].defined = 0;
		for(j=0;j<10;j++)
			ftable[i].argType[j] = -1;
			
	}
	
}

/*

  init_symbol_table();

*/
void init_symbol_table()
{
  //table = malloc( sizeof(struct symbol_entry)*MAX_TABLE_SIZE);
  bzero(&table[0], sizeof(struct symbol_entry)*MAX_TABLE_SIZE);

}

/*
   To install a symbol in the symbol table 

*/
char  *
install_symbol(s)
char *s;
{
//   printf("cur_counter%d\n",cur_counter);
   if (cur_counter >= MAX_TABLE_SIZE)
     err("Symbol Table Full");
   else {
    printf("1\n");
    table[cur_counter].scope = cur_scope;
    //printf("11\n");
    table[cur_counter].name = copys(s);
    //printf("2\n");
    table[cur_counter].mtype = -1;
    table[cur_counter].stack_offset = stack_cur_offset;
    //stack_cur_offset ++;
    cur_counter++;
  }
   return(s);
}


void install_function(char* s){
  ftable[cur_fcounter].name = copys(s);
  /*
  ftable[cur_fcounter].returnType = rt;
  ftable[cur_fcounter].argSize = size;
  int i;
  for(i=0;i<size;i++)
    ftable[cur_fcounter].argType = para_type[i];
  */
  cur_fcounter++;
  return ;
}

int look_up_function(char* s){
   int i;
	
   if (cur_fcounter==0) return(-1);
   for (i=cur_fcounter-1;i>=0; i--)
     {
       if (!strcmp(s,ftable[i].name))
       return(i);
     }
       return(-1);
}

/*
   To return an integer as an index of the symbol table

*/
int
look_up_symbol(s)
char *s;
{
   int i;

   if (cur_counter==0) return(-1);
   for (i=cur_counter-1;i>=0; i--)
     {
       if (!strcmp(s,table[i].name)){
	    //fprintf(stderr,"scope%d  cur_scopr:%d \n",table[i].scope,cur_scope); 
	    return(i);
	}
     }
       return(-1);
 }


/*
   Pop up symbols of the given scope from the symbol table upon the
   exit of a given scope.

*/
void
pop_up_symbol(int scope)
{
   int i;
   if (cur_counter==0) return;
   for (i=cur_counter-1;i>=0; i--)
     {
       if (table[i].scope !=scope) break;
     }
   if (i<0) cur_counter = 0;
   cur_counter = i+1;
  
}



/*
   Set up parameter scope and offset

*/
set_scope_and_offset_of_param(s)
char *s;
{

  int i,j,index;
  int total_args;

   index = look_up_symbol(s);
   if (index<0) err("Error in function header");
   else {
      table[index].type = T_FUNCTION;
      total_args = cur_counter -index -1;
      table[index].total_args=total_args;
      for (j=total_args, i=cur_counter-1;i>index; i--,j--)
        {
          table[i].scope= cur_scope;
          table[i].offset= j;
          table[i].mode  = ARGUMENT_MODE;
        }
   }

}



/*
   Set up local var offset

*/
set_local_vars(functor)
char *functor;
{

  int i,j,index,index1;
  int total_locals;

  index = look_up_symbol(functor);
  index1 = index + table[index].total_args;
  total_locals= cur_counter -index1 -1;
  if (total_locals <0)
     err("Error in number of local variables");
  table[index].total_locals=total_locals;
  for (j=total_locals, i=cur_counter-1;j>0; i--,j--)
        {
          table[i].scope= cur_scope;
          table[i].offset= j;
          table[i].mode  = LOCAL_MODE;
        }

}



/*
  Set GLOBAL_MODE to global variables

*/

set_global_vars(s)
char *s;
{
  int index;
  index =look_up_symbol(s);
  table[index].mode = GLOBAL_MODE;
  table[index].scope =1;
}


/*

To generate house-keeping work at the beginning of the function

*/

code_gen_func_header(functor)
char *functor;
{

fprintf(f_asm,"   ;    %s\n",functor);
fprintf(f_asm,"        assume cs:_TEXT\n");
fprintf(f_asm,"_%s      proc  near\n",functor);
fprintf(f_asm,"        push bp\n");
fprintf(f_asm,"        mov  bp,sp\n");
fprintf(f_asm,"   ;    \n");

}

/*

  To generate global symbol vars

*/
code_gen_global_vars()
{
  int i;

  fprintf(f_asm,"_BSS     segment word public 'BSS'\n");
  for (i=0; i<cur_counter; i++)
     {
       if (table[i].mode == GLOBAL_MODE)
	 {
            fprintf(f_asm,"_%s	label	word\n",table[i].name);
            fprintf(f_asm,"	db	2 dup (?)\n");
         }
     }
 fprintf(f_asm,"_BSS    ends\n");

}


/*

 To geenrate house-keeping work at the end of a function

*/

code_gen_at_end_of_function_body(functor)
char *functor;
{
  int i;

  fprintf(f_asm,"   ;    \n");
  fprintf(f_asm,"        mov sp, bp\n");
  fprintf(f_asm,"        pop bp\n");
  fprintf(f_asm,"        ret\n");
  fprintf(f_asm,"_%s     endp\n",functor);

}





/*******************Utility Functions ********************/
/*
 * copyn -- makes a copy of a string with known length
 *
 * input:
 *	  n - lenght of the string "s"
 *	  s - the string to be copied
 *
 * output:
 *	  pointer to the new string
 */
/*
char   *
copyn(n, s)
	register int n;
	register char *s;
{
	register char *p, *q;
	//char   *calloc();

	p = q = (register char*)calloc(1,n);
	while (--n >= 0)
		*q++ = *s++;
	return (p);
}


/*
 * copys -- makes a copy of a string
 *
 * input:
 *	  s - string to be copied
 *
 * output:
 *	  pointer to the new string
 */


char* copys(char * s)
{
	char * ss;
	printf("a2 %s\n",s);
	ss = (char*) malloc(sizeof(char)*(strlen(s)+10));
	memset(ss,0,sizeof(char)*(strlen(s)+10));
 	printf("a1\n");
	strcpy(ss,s);
	return ss;
	
}






