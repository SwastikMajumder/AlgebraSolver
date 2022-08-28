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

float calc(struct EQUATION *eq){
	int i;
	float sum=0.0f;
	float mul=1.0f;
	switch (eq->OPERATION){
		case 0:
			return eq->CONSTANT;
		case ADD:
			for (i=0; i<eq->TOTAL_CHILD; ++i){
				sum+=calc(eq->CHILD[i]);
			}
			return sum;
		case MULTIPLY:
			for (i=0; i<eq->TOTAL_CHILD; ++i){
				mul*=calc(eq->CHILD[i]);
			}
			return mul;
		case POWER:
			return powf(calc(eq->CHILD[0]), calc(eq->CHILD[1]));
	}
}

int is_only_number(struct EQUATION *eq){
	if (eq->OPERATION == VARIABLE) return 0;
	else if (eq->OPERATION == 0){
		return 1;
	} else {
		return is_only_number(eq->CHILD[0]) * is_only_number(eq->CHILD[1]);
	}
}

void solve_for_var(struct EQUATION *eq, int var_id, float *answer){
	int i;
	for (i=0; i<2; ++i){
		if (eq->CHILD[i]->OPERATION != VARIABLE){
			if (is_only_number(eq->CHILD[i])){
				//printf("%2.2f", calc(eq->CHILD[i]));
				switch(eq->OPERATION){
					case ADD:
						*answer-=calc(eq->CHILD[i]);
						break;
					case MULTIPLY:
						*answer/=calc(eq->CHILD[i]);
						break;
				}
			}
		}
	}
	for (i=0; i<2; ++i){
		if (eq->CHILD[i]->OPERATION != VARIABLE){
			if (is_only_number(eq->CHILD[i]) == 0){
				solve_for_var(eq->CHILD[i], var_id, answer);
			}
		}
	}
}

struct EQUATION *open_bracket(struct EQUATION *eq, float constant){
	if (eq->OPERATION == MULTIPLY){
		int i;
		for (i=0; i<2; ++i){
			if (eq->CHILD[i]->OPERATION != VARIABLE){
				eq->CHILD[i]->CONSTANT *= constant;
				break;
			}
		}
	}
}

int operation_list[2]={ADD, MULTIPLY};

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

struct EQUATION *distribute(struct EQUATION *eq, float carry){
	int i, j;
	float mul=1.0f;
	if (eq->OPERATION == MULTIPLY){
		for (i=0; i<eq->TOTAL_CHILD; ++i){
			if (eq->CHILD[i]->OPERATION == 0){
				eq->CHILD[i]->CONSTANT *= carry;
				break;
			}
		}
		if (i == eq->TOTAL_CHILD){
			add_child(eq, make_number(carry));
		}
		for (i=0; i<eq->TOTAL_CHILD; ++i){
			if (eq->CHILD[i]->OPERATION == 0){
				mul *= eq->CHILD[i]->CONSTANT;
			}
		}
		if (approx(mul, 0.0f)){
			eq = make_number(0.0f);
		} else {
			if (approx(mul, 1.0f)==0){
				for (i=0; i<eq->TOTAL_CHILD; ++i){
					if (eq->CHILD[i]->OPERATION == ADD){
						for (j=0; j<eq->CHILD[i]->TOTAL_CHILD; ++j){
							if (eq->CHILD[i]->CHILD[j]->OPERATION == MULTIPLY){
							 distribute(eq->CHILD[i]->CHILD[j], mul);
							}
							else if (eq->CHILD[i]->CHILD[j]->OPERATION == 0){
								eq->CHILD[i]->CHILD[j]->CONSTANT *= mul;
							}
						}
					}
				}
			}
			for (i=0; i<eq->TOTAL_CHILD; ++i){
				if (eq->CHILD[i]->OPERATION == 0){
					remove_child(eq, i);
				}
			}
			//printf("%d\n",eq->TOTAL_CHILD);
			if (eq->TOTAL_CHILD == 1){
				eq = eq->CHILD[0];
			}
		}
	} else if (eq->OPERATION == ADD){
			for (i=0; i<eq->TOTAL_CHILD; ++i){
				if (eq->CHILD[i]->OPERATION == MULTIPLY){
					distribute(eq->CHILD[i], carry);
				}
		 }
	}
	return eq;
}	

int main (){
	struct EQUATION *x;
	/*
	x = make_variable();
	x = operate_equation(ADD, x, make_number(2.0f));
	x = operate_equation(MULTIPLY, x, make_number(5.0f));
	x = operate_equation(ADD, x, make_variable());
	
	*/
	/*
	x = operate_equation(ADD, make_number(1.0f), make_number(5.0f));
	x = operate_equation(MULTIPLY, make_number(10.0f), x);
	print_equation(x);
	printf("\n");
	printf("%2.2f", calc(x));
	*/
	/*
	struct EQUATION *y;
	y = operate_equation(MULTIPLY, make_number(6.0f), make_number(8.0f));
	x = operate_equation(ADD, y, make_variable());
	x = operate_equation(MULTIPLY, x, make_number(4.0f));
	x = operate_equation(ADD, x, make_number(3.0f));
	print_equation(x);
	float answer = 0.0f;
	solve_for_var(x, total_var_id-1, &answer);
	printf("\n%2.2f", answer);
	*/
	x = operate_equation(ADD, make_number(1.0f), make_number(2.0f));
	x = operate_equation(ADD, x, make_number(3.0f));
	x = operate_equation(MULTIPLY, x, make_number(5.0f));
	print_equation(x);
	printf("\n");
	x=fix_tree(x);
	print_equation(x);
	printf("\n");
	x = distribute(x, 1.0f);
	print_equation(x);
	printf("\n");
	printf("%2.2f\n", calc(x));
}
