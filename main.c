#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ADD 1
#define MULTIPLY 2
#define EXPONENT 3
#define VARIABLE 4

#define MC 10
//max children

char *operation_symbol = "+*";

int total_var_id = 0;

struct EQUATION {
	int OPERATION;
	int VAR_ID;
	int POWER;
	float CONSTANT;
	int TOTAL_CHILD;
	struct EQUATION *CHILD[MC];
};

int approx(float a, float b){
  if (a>b){
    return (a-b) < 0.01f;
  } else {
    return (b-a) < 0.01f;
  }
}

void print_equation(struct EQUATION *eq){
	if (eq->OPERATION == VARIABLE) {
		if (eq->POWER == 0){
			printf("%2.2f", eq->CONSTANT);
		} else {
		if (!approx(eq->CONSTANT, 1.0f))
			printf("(%2.2f*", eq->CONSTANT);
  	if (eq->POWER > 1)
	    printf("(");
	  printf("%c", 'a'+eq->VAR_ID);
	  if (eq->POWER > 1)
	    printf("^%d)", eq->POWER);
	  if (!approx(eq->CONSTANT, 1.0f))
			printf(")");
		return;
		}
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

struct EQUATION *make_variable(int id, float constant, int power){
	struct EQUATION *output = (struct EQUATION *)malloc(sizeof(struct EQUATION));
	memset(output, 0, sizeof(struct EQUATION));
	output->VAR_ID = id;
	output->CONSTANT = constant;
	output->POWER = power;
	output->OPERATION = VARIABLE;
	output->TOTAL_CHILD=0;
	return output;
}

struct EQUATION *make_number(float constant){
	return make_variable(0, constant, 0);
}

struct EQUATION *operate_equation(int operation, struct EQUATION *eq1, struct EQUATION *eq2){
	struct EQUATION *output = (struct EQUATION *)malloc(sizeof(struct EQUATION));
	output->OPERATION = operation;
	output->CHILD[0] = eq1;
	output->CHILD[1] = eq2;
	output->TOTAL_CHILD=2;
	return output;
}

void add_child(struct EQUATION *eq, struct EQUATION *child){
  eq->CHILD[(eq->TOTAL_CHILD)++] = child;
}

void remove_child(struct EQUATION *eq, int n){
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

void p(struct EQUATION *eq){
	print_equation(eq);
	printf("\n");	
}
int dummy=0;

int k2(int x){
	if (x) return x;
	else return 1;
}
int k(struct EQUATION *eq, int d, int n){
	int i;
	int circle=n%k2(eq->CHILD[0]->TOTAL_CHILD);
	for (i=1;i<=d;++i){
		n = (n -circle)/k2(eq->CHILD[i-1]->TOTAL_CHILD);
		circle = n%k2(eq->CHILD[i]->TOTAL_CHILD);
	}
	return circle;
}
struct EQUATION *ok(struct EQUATION *eq, int n){
	int i;
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	output->TOTAL_CHILD = 0;
	output->OPERATION = MULTIPLY;
	for (i=0; i<eq->TOTAL_CHILD; ++i){
		if (eq->CHILD[i]->TOTAL_CHILD == 0){
	  	add_child(output, eq->CHILD[i]);
		} else {
	    add_child(output, eq->CHILD[i]->CHILD[k(eq, i, n)]);
		}
	}
	return output;
}
struct EQUATION *distribute(struct EQUATION *eq){
	int i;
	struct EQUATION *output;
	if (eq->OPERATION == MULTIPLY){
		for (i=0; i<eq->TOTAL_CHILD; ++i){
			if (eq->CHILD[i]->OPERATION == ADD) break;
		}
		if (i == eq->TOTAL_CHILD){
			output = make_variable(0, 1.0f, 0);
			for (i=0; i<eq->TOTAL_CHILD; ++i){
				output->POWER += eq->CHILD[i]->POWER;
				output->CONSTANT *= eq->CHILD[i]->CONSTANT;
			}
			return output;
		} else {
			int t=1;
			for (i=0; i<eq->TOTAL_CHILD; ++i){
				t *= k2(eq->CHILD[i]->TOTAL_CHILD);
			}
			output = malloc(sizeof(struct EQUATION));
			output->TOTAL_CHILD = 0;
	    output->OPERATION = ADD;
			for (i=0; i<t; ++i){
				add_child(output, ok(eq, i));
			}
			output = distribute(output);
			if (output->TOTAL_CHILD == 1) return output->CHILD[0];
			return output;
		}
	}
	else if (eq->OPERATION == ADD){
		float var_sum[3];
		output = malloc(sizeof(struct EQUATION));
	  output->TOTAL_CHILD = 0;
	  output->OPERATION = ADD;
	  for (i=0; i<eq->TOTAL_CHILD; ++i){
			if (eq->CHILD[i]->OPERATION == MULTIPLY){
				eq->CHILD[i] = distribute(eq->CHILD[i]);
			}
		}
		eq = fix_tree(eq);
		for (i=0; i<3; ++i){
			var_sum[i] = 0.0f;
		}
		for (i=0; i<eq->TOTAL_CHILD; ++i){
			var_sum[eq->CHILD[i]->POWER] += eq->CHILD[i]->CONSTANT;
		}
		for (i=0; i<3; ++i){
			add_child(output, make_variable(0, var_sum[i], i));
		}
		if (output->TOTAL_CHILD == 1) return output->CHILD[0];
  	return output;
	}
}

#define m(x, y) operate_equation(MULTIPLY, x, y)
#define a(x, y) operate_equation(ADD, x, y)
#define n(x) make_number(x)
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
	y = make_variable(0, 1.0f, 1);
	x = a(y, n(3.0f));
	u = a(n(2.0f), y);
	x = m(x, u);
	x = m(x, n(3.0f));
	x = a(y, x);
	x = a(n(2.0f), x);
	p(x);
	solve(x);
}
