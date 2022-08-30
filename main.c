#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ADD 1
#define MULTIPLY 2
#define POWER 3
#define VARIABLE 4

#define MC 10
//max children

char *operation_symbol = "+*";

int total_var_id = 0;

struct EQUATION {
	int OPERATION;
	int VAR_ID;
	float CONSTANT;
	int TOTAL_CHILD;
	struct EQUATION *CHILD[MC];
};

void print_equation(struct EQUATION *eq){
	if (eq->OPERATION == 0){
		printf("%2.2f", eq->CONSTANT);
		return;
	}
	else if (eq->OPERATION == VARIABLE) {
		printf("%c", 'a'+(eq->VAR_ID));
		return;
	} else {
		printf("(");
		int i;
		for (i=0; i<(eq->TOTAL_CHILD)-1; ++i){
		  print_equation(eq->CHILD[i]);
		  printf("%c", operation_symbol[(eq->OPERATION)-1]);
		}
		print_equation(eq->CHILD[i]);
		printf(")");
	}
}

struct EQUATION *make_variable(){
	struct EQUATION *output = (struct EQUATION *)malloc(sizeof(struct EQUATION));
	memset(output, 0, sizeof(struct EQUATION));
	output->VAR_ID = total_var_id++;
	output->OPERATION = VARIABLE;
	return output;
}

struct EQUATION *make_var_id(int id){
	struct EQUATION *output = (struct EQUATION *)malloc(sizeof(struct EQUATION));
	memset(output, 0, sizeof(struct EQUATION));
	output->VAR_ID = id;
	output->OPERATION = VARIABLE;
	return output;
}

struct EQUATION *make_number(float constant){
	struct EQUATION *output = (struct EQUATION *)malloc(sizeof(struct EQUATION));
	memset(output, 0, sizeof(struct EQUATION));
	output->CONSTANT = constant;
	output->TOTAL_CHILD=2;
	return output;
}

struct EQUATION *operate_equation(int operation, struct EQUATION *eq1, struct EQUATION *eq2){
	struct EQUATION *output = (struct EQUATION *)malloc(sizeof(struct EQUATION));
	output->OPERATION = operation;
	output->CHILD[0] = eq1;
	output->CHILD[1] = eq2;
	output->TOTAL_CHILD=2;
	return output;
}

struct EQUATION *add_child(struct EQUATION *eq, struct EQUATION *child){
  eq->CHILD[(eq->TOTAL_CHILD)++] = child;
}

struct EQUATION *remove_child(struct EQUATION *eq, int n){
  int i;
  for (i=n;i<(eq->TOTAL_CHILD)-1; ++i){
    eq->CHILD[i]=eq->CHILD[i+1];
  }
  eq->CHILD[--(eq->TOTAL_CHILD)] = 0;
}

struct EQUATION *shift_up(struct EQUATION *eq, int n){
	int i;
	for (i=0; i<eq->CHILD[n]->TOTAL_CHILD; ++i){
	      add_child(eq, eq->CHILD[n]->CHILD[i]);
	    }
	remove_child(eq, n);
	return eq;
}

struct EQUATION *fix_tree(struct EQUATION *eq){
	int i;
	int j;
	if (eq->OPERATION!=ADD && eq->OPERATION!=MULTIPLY) return 0;
	for (i=0; i<(eq->TOTAL_CHILD); ++i){
	  if (eq->OPERATION==eq->CHILD[i]->OPERATION){
	    shift_up(eq, i);
		}
	}
  for (i=0; i<(eq->TOTAL_CHILD); ++i){
  	 fix_tree(eq->CHILD[i]);
  }
  return eq;
}

int approx(float a, float b){
  if (a>b){
    return (a-b) < 0.01f;
  } else {
    return (b-a) < 0.01f;
  }
}

int is_var_grp(struct EQUATION *eq){
	int ans =0;
	
	if (eq->OPERATION== MULTIPLY && eq->TOTAL_CHILD==2){
		int i;
		for (i=0; i<2; ++i){
			if (eq->CHILD[i]->OPERATION == 0 && eq->CHILD[1-i]->OPERATION == VARIABLE){
				ans = 1;
				if (i==1){
					struct EQUATION *tmp;
					tmp = eq->CHILD[0];
					eq->CHILD[0] = eq->CHILD[1];
					eq->CHILD[1] = tmp;
				}
			}
		}
	}
	return ans;
}
void p(struct EQUATION *eq){
	print_equation(eq);
	printf("\n");	
}
int dummy=0;
struct EQUATION *distribute(struct EQUATION *eq){
  int i, j;
  float mul;
  if (eq->OPERATION == MULTIPLY){
A:
    mul=1.0f;
    for (i=0; i<eq->TOTAL_CHILD; ++i){
      if (eq->CHILD[i]->OPERATION == 0){
        mul *= eq->CHILD[i]->CONSTANT;
      }
    }
    if (approx(mul, 0.0f)){
      eq = make_number(0.0f);
      return eq;
    } else {
      int del_allow = 1;
        for (i=0; i<eq->TOTAL_CHILD; ++i){
          if (eq->CHILD[i]->OPERATION == ADD){
            for (j=0; j<eq->CHILD[i]->TOTAL_CHILD; ++j){
              if (eq->CHILD[i]->CHILD[j]->OPERATION == 0){
                eq->CHILD[i]->CHILD[j]->CONSTANT *= mul;
              }
              else if (eq->CHILD[i]->CHILD[j]->OPERATION == VARIABLE){
              	eq->CHILD[i]->CHILD[j] = operate_equation(MULTIPLY, eq->CHILD[i]->CHILD[j], make_number(mul));
              }
              else if (is_var_grp(eq->CHILD[i]->CHILD[j])){
              	eq->CHILD[i]->CHILD[j]->CHILD[0]->CONSTANT *= mul;
              }
            }
            
            for (j=0; j<eq->TOTAL_CHILD; ++j)
          if (eq->CHILD[j]->OPERATION == 0)
            remove_child(eq, j);
        if (eq->TOTAL_CHILD == 1) {eq = eq->CHILD[0]; return eq;}
            eq->CHILD[i] = distribute(eq->CHILD[i]);
          }
        }
        for (i=0; i<eq->TOTAL_CHILD; ++i){
        	if (eq->CHILD[i]->OPERATION == ADD){
        		break;
        	}
        }
        if (i == eq->TOTAL_CHILD){
        	del_allow=0;
        }
        if (del_allow){} else {
      	for (i=0; i<eq->TOTAL_CHILD; ++i)
          if (eq->CHILD[i]->OPERATION == 0){
            remove_child(eq, i);
            --i;
          }
        add_child(eq, make_number(mul));
        if (eq->TOTAL_CHILD == 1) eq = eq->CHILD[0];
        }
      
    }
  } else if (eq->OPERATION == ADD){
   	float var_sum;
   	float num_sum;
B:
    var_sum = num_sum = 0.0f;
     for (i=0; i<eq->TOTAL_CHILD; ++i){
        if (is_var_grp(eq->CHILD[i])){
              var_sum += eq->CHILD[i]->CHILD[0]->CONSTANT;
              remove_child(eq, i);
              --i;
        }
        else if (eq->CHILD[i]->OPERATION == VARIABLE){
        	var_sum+=1.0f;
        	remove_child(eq, i);
        	--i;
        }
        else if (eq->CHILD[i]->OPERATION == 0){
        	num_sum+=eq->CHILD[i]->CONSTANT;
        	remove_child(eq, i);
        	--i;
        }
     }
     if (!approx(var_sum, 0.0f)){
     	add_child(eq, operate_equation(MULTIPLY, make_var_id(0), make_number(var_sum)));
     }
     if (!approx(num_sum, 0.0f)){
     	add_child(eq, make_number(num_sum));
     }
     for (i=0; i<eq->TOTAL_CHILD; ++i){
        if (eq->CHILD[i]->OPERATION == MULTIPLY && is_var_grp(eq->CHILD[i]) == 0){
          eq->CHILD[i] = distribute(eq->CHILD[i]);
          goto B;
        }
     }
     if (eq->TOTAL_CHILD == 1) {eq = eq->CHILD[0];}
  }
  return eq;
}
void solve(struct EQUATION *eq){
	int i, j;
	for (i=0; i<5; ++i){
		for (j=0; j<5; ++j){
			eq = fix_tree(eq);
		}
		eq = distribute(eq);
	}
	p(eq);
}
int main (){
	struct EQUATION *x;
	struct EQUATION *y;
	struct EQUATION *u;
	y = make_variable();
	y = operate_equation(ADD, make_number(2.0f), y);
	x = operate_equation(ADD, make_number(1.0f), make_number(2.0f));
	x = operate_equation(ADD, x, y);
	u = operate_equation(ADD, make_number(10.0f), make_number(10.0f));
	x = operate_equation(MULTIPLY, x, u);
	solve(x);
}
