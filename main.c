#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ADD 1
#define MULTIPLY 2
#define EXPONENT 3
#define VARIABLE 4

#define MC 1000
//max children

char *operation_symbol = "+*^";

#define VM 3

struct EQUATION {
	int OPERATION;
	int POWER[VM];
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
	int i;
	if (eq->OPERATION == VARIABLE) {
		for (i=0; i<VM; ++i){
			if (eq->POWER[i] != 0) break;
		}
		if (i==VM){
			printf("(%2.2f)", eq->CONSTANT);
		} else {
    	if (!approx(eq->CONSTANT, 1.0f))
	    	printf("(%2.2f*", eq->CONSTANT);
  		for (i=0; i<VM; ++i){
	    	if (eq->POWER[i] !=0){
      	  if (eq->POWER[i] > 1) printf("(");
	        printf("%c", 'a'+i);
	        if (eq->POWER[i]> 1) printf("^%d)", eq->POWER[i]);
		    }
	  	}
	    if (!approx(eq->CONSTANT, 1.0f)) printf(")");
		}
	} else {
		printf("(");
		for (i=0; i<(eq->TOTAL_CHILD)-1; ++i){
		  print_equation(eq->CHILD[i]);
		  printf("%c", operation_symbol[(eq->OPERATION)-1]);
		}
		print_equation(eq->CHILD[i]);
		printf(")");
	}
}
struct EQUATION *do_copy(struct EQUATION *eq){
	int i;
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	output->TOTAL_CHILD = eq->TOTAL_CHILD;
	output->OPERATION = eq->OPERATION;
	output->CONSTANT = eq->CONSTANT;
	memcpy(output->POWER, eq->POWER, sizeof(int)*VM);
	for (i=0; i<eq->TOTAL_CHILD; ++i){
		output->CHILD[i] = do_copy(eq->CHILD[i]);
	}
	return output;
}
void add_child(struct EQUATION *eq, struct EQUATION *child){
  eq->CHILD[(eq->TOTAL_CHILD)++] = do_copy(child);
}

void remove_child(struct EQUATION *eq, int n){
  int i;
  for (i=n;i<(eq->TOTAL_CHILD)-1; ++i){
    eq->CHILD[i]=eq->CHILD[i+1];
  }
  eq->CHILD[--(eq->TOTAL_CHILD)] = 0;
}

struct EQUATION *operate_equation(int operation, struct EQUATION *eq1, struct EQUATION *eq2){
	struct EQUATION *output = (struct EQUATION *)malloc(sizeof(struct EQUATION));
	memset(output, 0, sizeof(struct EQUATION));
	output->OPERATION = operation;
	output->CHILD[0] = do_copy(eq1);
	output->CHILD[1] = do_copy(eq2);
	output->TOTAL_CHILD=2;
	return do_copy(output);
}
void p(struct EQUATION *eq){
	print_equation(eq);
	printf("\n");	
}
int isdiv(struct EQUATION *eq){
	int i;
	if (eq->OPERATION == EXPONENT && eq->CHILD[1]->OPERATION == VARIABLE && approx(eq->CHILD[1]->CONSTANT, -1.0f)){
		for (i=0; i<VM; ++i){
			if (eq->CHILD[1]->POWER[i] != 0) break;
		}
		if (i == VM) return 1;
	}
	return 0;
}
struct EQUATION *make_variable(int id, float constant, int power){
	struct EQUATION *output = (struct EQUATION *)malloc(sizeof(struct EQUATION));
	memset(output, 0, sizeof(struct EQUATION));
	output->CONSTANT = constant;
	output->POWER[id] = power;
	output->OPERATION = VARIABLE;
	output->TOTAL_CHILD=0;
	return output;
}

struct EQUATION *make_number(float constant){
	return make_variable(0, constant, 0);
}

struct EQUATION *shift_up(struct EQUATION *eq, int n){
	int i;
	for (i=0; i<eq->CHILD[n]->TOTAL_CHILD; ++i){
	      add_child(eq, eq->CHILD[n]->CHILD[i]);
	    }
	remove_child(eq, n);
	return eq;
}

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
#define PO 9

int k3(int d, int n){
	int i;
	int circle = n%PO;
	for (i=1; i<=d; ++i){
		n = (n-circle)/PO;
		circle = n%PO;
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

struct EQUATION *fix_tree(struct EQUATION *eq){
	int i;
	for (i=0; i<eq->TOTAL_CHILD; ++i){
		if (eq->CHILD[i]->OPERATION == eq->OPERATION && (eq->OPERATION == MULTIPLY || eq->OPERATION == ADD)){
			shift_up(eq, i);
			i=-1;
		}
	}
	return eq;
}
struct EQUATION *no(int op){
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	memset(output, 0, sizeof(struct EQUATION));
	output->TOTAL_CHILD = 0;
	output->OPERATION = op;
	return output;
}
int isfrac(struct EQUATION *eq){
	if (eq->OPERATION == VARIABLE) return 0;
	if (isdiv(eq)) return 1;
	int i;
	for (i=0; i<eq->TOTAL_CHILD; ++i){
		if (isdiv(eq->CHILD[i])){return 1;}
	}
	return 0;
}
struct EQUATION *nc(struct EQUATION *eq){
	struct EQUATION *output = no(eq->OPERATION);
	if (isdiv(eq)) return make_number(1.0f);
	int i;
//	p(eq);
//	printf("%d\n", isdiv(eq->CHILD[1]));
	for (i=0; i<eq->TOTAL_CHILD; ++i){
		if (!isdiv(eq->CHILD[i])){
			add_child(output, eq->CHILD[i]);
		}
	}
	//p(output);
	if (output->TOTAL_CHILD == 0) output = eq;
	if (output->TOTAL_CHILD == 1) output = output->CHILD[0];
	return do_copy(output);
}
struct EQUATION *dc(struct EQUATION *eq){
	struct EQUATION *output = no(MULTIPLY);
	if (isdiv(eq)) return eq->CHILD[0];
	int i;
	for (i=0; i<eq->TOTAL_CHILD; ++i){
		if (isdiv(eq->CHILD[i])){
			add_child(output, eq->CHILD[i]->CHILD[0]);
		}
	}
	if (output->TOTAL_CHILD == 0) return make_number(1.0f);
	if (output->TOTAL_CHILD == 1) output = output->CHILD[0];
	return do_copy(output);
}

struct EQUATION *distribute(struct EQUATION *eq){
	struct EQUATION *output;
	struct EQUATION *output_2;
	int i, j, k;
	eq = fix_tree(eq);
	int tt = (int)pow(PO, VM);
	int iscross=0;
	if (eq->OPERATION == ADD){
		for (i=0; i<eq->TOTAL_CHILD; ++i){
		  if (isdiv(eq->CHILD[i])) iscross = 1;
			else if (eq->CHILD[i]->OPERATION == MULTIPLY)
  			for (j=0; j<eq->CHILD[i]->TOTAL_CHILD; ++j)
  				if (isdiv(eq->CHILD[i])) iscross = 1;
		}
	}
	if (iscross){
		output = no(ADD);
		int ifenter;
		output_2 = no(MULTIPLY);
		for (i=0; i<eq->TOTAL_CHILD; ++i){
			ifenter=0;
			output->CHILD[output->TOTAL_CHILD] = no(MULTIPLY);
			if (eq->CHILD[i]->OPERATION == MULTIPLY){
				for (k=0; k<eq->CHILD[i]->OPERATION; ++k){
					if (!isdiv(eq->CHILD[i]->CHILD[k])){
						add_child(output->CHILD[output->TOTAL_CHILD], eq->CHILD[i]->CHILD[k]);
						ifenter= 1;
					}
				}
			}
			if (eq->CHILD[i]->OPERATION == VARIABLE){
				add_child(output->CHILD[output->TOTAL_CHILD], eq->CHILD[i]);
				ifenter= 1;
			}
			if (eq->CHILD[i]->OPERATION == MULTIPLY){
    		for (k=0; k<eq->CHILD[i]->OPERATION; ++k){
	    		if (isdiv(eq->CHILD[i]->CHILD[k])){
						add_child(output_2, eq->CHILD[i]->CHILD[k]->CHILD[0]);
    			}
    		}
			}
			if (isdiv(eq->CHILD[i])){
				add_child(output_2, eq->CHILD[i]->CHILD[0]);
			}
			for (j=0; j<eq->TOTAL_CHILD; ++j){
				if (i!=j){
					if (eq->CHILD[j]->OPERATION == MULTIPLY){
						for (k=0; k<eq->CHILD[j]->TOTAL_CHILD; ++k){
							if (isdiv(eq->CHILD[j]->CHILD[k])){
								add_child(output->CHILD[output->TOTAL_CHILD], eq->CHILD[j]->CHILD[k]->CHILD[0]);
								ifenter=1;
							}
						}
					}
					if (isdiv(eq->CHILD[j])){
						add_child(output->CHILD[output->TOTAL_CHILD], eq->CHILD[j]->CHILD[0]);
						ifenter=1;
					}
				}
			}
			if (!ifenter) add_child(output->CHILD[output->TOTAL_CHILD], make_number(1.0f));
		  ++(output->TOTAL_CHILD);
		  //if (!ifenter){add_child(output, make_number(1.0f));}
		}
		for (i=0; i<output->TOTAL_CHILD; ++i){
	  	if (output->CHILD[i]->TOTAL_CHILD == 1) output->CHILD[i] = output->CHILD[i]->CHILD[0];
  	}
  	if (output->TOTAL_CHILD == 1) output = output->CHILD[0];
  	if (output_2->TOTAL_CHILD == 1) output_2 = output_2->CHILD[0];
  	eq = operate_equation(MULTIPLY, output, operate_equation(EXPONENT, output_2, make_number(-1.0f)));
	}
	//(eq);
	if (isfrac(eq)){
		output = distribute(nc(eq));
		output_2 = distribute(dc(eq));
		struct EQUATION *n;
		struct EQUATION *d;
		n = no(MULTIPLY);
		d = no(MULTIPLY);
		add_child(n, nc(output));
		add_child(d, nc(output_2));
		if (isfrac(output)){
			add_child(d, dc(output));
		}
		if (isfrac(output_2)){
			add_child(n, dc(output_2));
		}
		if (n->TOTAL_CHILD == 1) n = n->CHILD[0];
		if (d->TOTAL_CHILD == 1) d = d->CHILD[0];
		n = distribute(n);
		d = distribute(d);
		return operate_equation(MULTIPLY, do_copy(n), operate_equation(EXPONENT, do_copy(d), make_number(-1.0f)));
	}
	if (eq->OPERATION == MULTIPLY){
		for (i=0; i<eq->TOTAL_CHILD; ++i){
			if (eq->CHILD[i]->OPERATION == ADD) break;
		}
		if (i == eq->TOTAL_CHILD){
			output = make_variable(0, 1.0f, 0);
			for (i=0; i<eq->TOTAL_CHILD; ++i){
				for (j=0; j<VM; ++j)
		  		output->POWER[j] += eq->CHILD[i]->POWER[j];
				output->CONSTANT *= eq->CHILD[i]->CONSTANT;
			}
			return do_copy(output);
		} else {
			int t=1;
			for (i=0; i<eq->TOTAL_CHILD; ++i){
				t *= k2(eq->CHILD[i]->TOTAL_CHILD);
			}
			output = no(ADD);
			for (i=0; i<t; ++i){
				add_child(output, ok(eq, i));
			}
			output = distribute(output);
			if (output->TOTAL_CHILD == 1) return output->CHILD[0];
			return do_copy(output);
		}
	} else if (eq->OPERATION == ADD){
		output = no(ADD);
	  for (i=0; i<eq->TOTAL_CHILD; ++i){
			if (eq->CHILD[i]->OPERATION == MULTIPLY){
				eq->CHILD[i] = distribute(eq->CHILD[i]);
			}
		}
		eq = fix_tree(eq);
		for (i=0; i<tt; ++i){
			float sum=0.0f;
			for (j=0; j<eq->TOTAL_CHILD; ++j){
				for (k=0; k<VM; ++k){
					if (eq->CHILD[j]->POWER[k]!=k3(k, i)) break;
				}
				if (k==VM){
					sum += eq->CHILD[j]->CONSTANT;
				}
			}
			if (!approx(sum, 0.0f)){
				struct EQUATION *tmp = no(VARIABLE);
				for (j=0; j<VM; ++j){
					tmp->POWER[j] = k3(j, i);
				}
				tmp->CONSTANT = sum;
				add_child(output, tmp);
			}
		}
		if (output->TOTAL_CHILD == 1) return output->CHILD[0];
  	return do_copy(output);
	}
	return do_copy(eq);
}

#define m(x, y) operate_equation(MULTIPLY, x, y)
#define a(x, y) operate_equation(ADD, x, y)
#define w(x, y) operate_equation(EXPONENT, x, y)
#define n(x) make_number(x)

int main (){
	struct EQUATION *x;
	struct EQUATION *y;
	struct EQUATION *u;
	x = make_variable(0, 1.0f, 1);
	//y = make_variable(1, 1.0f, 1);
	//y = w(a(x, n(1.0f)), n(-1.0f));
	//x = a(y, w(x, n(-1.0)));
	x=a(x, w(x, n(-1.0f)));
  p(x);
	x = distribute(x);
  p(x);
}
