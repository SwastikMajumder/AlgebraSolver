#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ADD 1
#define MULTIPLY 2
#define POWER 3
#define VARIABLE 4

char *operation_symbol = "+*";

int total_var_id = 0;

struct EQUATION {
	int OPERATION;
	int VAR_ID;
	float CONSTANT;
	struct EQUATION *CHILD[2];
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
		print_equation(eq->CHILD[0]);
		printf("%c", operation_symbol[eq->OPERATION-1]);
		print_equation(eq->CHILD[1]);
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
	return output;
}

struct EQUATION *operate_equation(int operation, struct EQUATION *eq1, struct EQUATION *eq2){
	struct EQUATION *output = (struct EQUATION *)malloc(sizeof(struct EQUATION));
	output->OPERATION = operation;
	output->CHILD[0] = eq1;
	output->CHILD[1] = eq2;
	return output;
}

float calc(struct EQUATION *eq){
	switch (eq->OPERATION){
		case 0:
			return eq->CONSTANT;
		case ADD:
			return calc(eq->CHILD[0]) + calc(eq->CHILD[1]);
		case MULTIPLY:
			return calc(eq->CHILD[0]) * calc(eq->CHILD[1]);
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
	struct EQUATION *y;
	y = operate_equation(MULTIPLY, make_number(6.0f), make_number(8.0f));
	x = operate_equation(ADD, y, make_variable());
	x = operate_equation(MULTIPLY, x, make_number(4.0f));
	x = operate_equation(ADD, x, make_number(3.0f));
	print_equation(x);
	float answer = 0.0f;
	solve_for_var(x, total_var_id-1, &answer);
	printf("\n%2.2f", answer);
}
