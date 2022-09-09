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

int total_var_id = 0;

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
	return output;
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
int ranger=0;

struct EQUATION *unite(struct EQUATION *eq){
		struct EQUATION *st1;
		int i;
	st1=	malloc(sizeof(struct EQUATION));
	st1->TOTAL_CHILD = 0;
	st1->OPERATION = MULTIPLY;
		int count=0;
		for (i=0; i<eq->TOTAL_CHILD; ++i){
	    if (isdiv(eq->CHILD[i])){
	    	add_child(st1, eq->CHILD[i]->CHILD[0]);
	    	count = 1;
	    }
		}
		for (i=0; i<eq->TOTAL_CHILD; ++i){
	    if (isdiv(eq->CHILD[i])){
	    	remove_child(eq, i);
	    	--i;
	    }
		}
		if (count){
		 add_child(eq, operate_equation(EXPONENT, st1, make_number(-1.0f)));
		}
		for (i=0; i<eq->TOTAL_CHILD; ++i)
  		if (eq->CHILD[i]->TOTAL_CHILD == 1) eq->CHILD[i]=eq->CHILD[i]->CHILD[0];
  	if (eq->TOTAL_CHILD == 1) eq = eq->CHILD[0];
		return eq;
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
	if (eq->OPERATION == EXPONENT){
		fix_tree(eq->CHILD[0]); return eq;
	}
	else if (eq->OPERATION!=ADD && eq->OPERATION!=MULTIPLY){
    return eq;
	}
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
struct EQUATION *cross(struct EQUATION *eq){
	int i, j, k;
	struct EQUATION *output;
	output = malloc(sizeof(struct EQUATION));
	output->TOTAL_CHILD = 0;
	output->OPERATION = MULTIPLY;
	if (eq->OPERATION == ADD){
	  for(i=0; i<eq->TOTAL_CHILD; ++i){
	  	if (eq->CHILD[i]->OPERATION != MULTIPLY){
	  		add_child(eq, operate_equation(MULTIPLY, make_number(1.0f), eq->CHILD[i]));
	  		remove_child(eq, i);
	  		--i;
	  	}
	  }
	  for (i=0; i<eq->TOTAL_CHILD; ++i)
	  	for (j=0; j<eq->CHILD[i]->TOTAL_CHILD; ++j)
	  	  if (isdiv(eq->CHILD[i]->CHILD[j]))
	  	      add_child(output, eq->CHILD[i]->CHILD[j]->CHILD[0]);
	  for (k=0; k<eq->TOTAL_CHILD; ++k)
	    for (i=0; i<eq->TOTAL_CHILD; ++i)
  	  	for (j=0; j<eq->CHILD[i]->TOTAL_CHILD; ++j)
  	  	  if (isdiv(eq->CHILD[i]->CHILD[j]))
	  	      if (i!=k) add_child(eq->CHILD[k], eq->CHILD[i]->CHILD[j]->CHILD[0]);
    for (i=0; i<eq->TOTAL_CHILD; ++i)
	  	for (j=0; j<eq->CHILD[i]->TOTAL_CHILD; ++j)
	  	  if (isdiv(eq->CHILD[i]->CHILD[j]))
	  	      remove_child(eq->CHILD[i], j);
	}
  return operate_equation(MULTIPLY, eq, operate_equation(EXPONENT, output, make_number(-1.0f)));
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

struct EQUATION *distribute(struct EQUATION *eq){
	eq=fix_tree(eq);
	eq=fix_tree(eq);
	eq=fix_tree(eq);
	int i;
	int j;
	int k;
	struct EQUATION *output;
	struct EQUATION *output_2 = malloc(sizeof(struct EQUATION));
	int tt = (int)pow(PO, VM);
	if (eq->OPERATION == MULTIPLY){
		eq = unite(eq);
		for (i=0; i<eq->TOTAL_CHILD; ++i){
			if (isdiv(eq->CHILD[i])){
			   struct EQUATION *div = do_copy(eq->CHILD[i]);
		    remove_child(eq, i);
		    if (eq->TOTAL_CHILD == 1) eq=eq->CHILD[0];
		    eq=fix_tree(eq);
		    eq=fix_tree(eq);
		    eq = distribute(eq);
		    div->CHILD[0] = distribute(div->CHILD[0]);
		    return operate_equation(MULTIPLY, eq, div);
			}
		}
		for (i=0; i<eq->TOTAL_CHILD; ++i){
			if (eq->CHILD[i]->OPERATION == ADD) break;
		}
		if (i == eq->TOTAL_CHILD){
			output = make_variable(0, 1.0f, 0);
			output_2->TOTAL_CHILD = 0;
			output_2->OPERATION = MULTIPLY;
			for (i=0; i<eq->TOTAL_CHILD; ++i){
				if (eq->CHILD[i]->OPERATION == EXPONENT){
					eq->CHILD[i] = distribute(eq->CHILD[i]);
					add_child(output_2, eq->CHILD[i]);
					continue;
				}
				for (j=0; j<VM; ++j)
		  		output->POWER[j] += eq->CHILD[i]->POWER[j];
				output->CONSTANT *= eq->CHILD[i]->CONSTANT;
			}
			if (output_2->TOTAL_CHILD > 0){
				if (approx(output->CONSTANT, 1.0f)){
					if (output_2->TOTAL_CHILD == 1) return output_2->CHILD[0];
				}	else {
					add_child(output_2, output);
				}
				return output_2;
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
		for (i=0; i<eq->TOTAL_CHILD; ++i){
			eq->CHILD[i] = unite(eq->CHILD[i]);
		}
		int iscross=0;
		for (i=0; i<eq->TOTAL_CHILD; ++i){
			if (isdiv(eq->CHILD[i])) iscross = 1;
			if (eq->CHILD[i]->OPERATION == MULTIPLY)
			  for (j=0; j<eq->CHILD[i]->TOTAL_CHILD; ++j)
			  	if (isdiv(eq->CHILD[i]->CHILD[j])){
			  	  iscross = 1;
			  	}
		}
		if (iscross){
			eq = cross(eq);
			eq = distribute(eq);
			return eq;
		}
		output = malloc(sizeof(struct EQUATION));
	  output->TOTAL_CHILD = 0;
	  output->OPERATION = ADD;
	  for (i=0; i<eq->TOTAL_CHILD; ++i){
			if (eq->CHILD[i]->OPERATION == MULTIPLY){
				eq->CHILD[i] = distribute(eq->CHILD[i]);
			}
		}
		eq = fix_tree(eq);
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
				struct EQUATION *tmp = malloc(sizeof(struct EQUATION));
				tmp->TOTAL_CHILD = 0;
	      tmp->OPERATION = VARIABLE;
				for (j=0; j<VM; ++j){
					tmp->POWER[j] = k3(j, i);
				}
				tmp->CONSTANT = sum;
				add_child(output, tmp);
			}
		}
		if (output->TOTAL_CHILD == 1) return output->CHILD[0];
  	return output;
	}
	return eq;
}

int powersame(struct EQUATION *eq1, struct EQUATION *eq2){
	int i;
	for(i=0; i<VM; ++i){
		if (eq1->POWER[i] != eq2->POWER[i]) return 0;
	}
	return 1;
}

int same(struct EQUATION *eq1, struct EQUATION *eq2){
	int i;
	int j;
	//if (dummy)
	//p(eq1);
	if (eq1->TOTAL_CHILD == eq2->TOTAL_CHILD && eq1->OPERATION == eq2->OPERATION && (eq1->OPERATION == VARIABLE ? approx(eq1->CONSTANT, eq2->CONSTANT) && powersame(eq1, eq2):1)){
	  if (eq1->TOTAL_CHILD != 0){
  	int check[eq1->TOTAL_CHILD];
  	memset(check, 0, sizeof(int)*eq1->TOTAL_CHILD);
  	for (i=0; i<eq1->TOTAL_CHILD; ++i){
  		for (j=0; j<eq2->TOTAL_CHILD; ++j){
  		 	if (check[j] == 0 && same(eq1->CHILD[i], eq2->CHILD[j])){
  	     	check[j] = 1;
  	     	break;
  		 	}
  		}
  		if (j==eq2->TOTAL_CHILD){
  		  return 0;
  		}
  	}
	  } else {
	  	return 1;
	  }
	} else {
		return 0;
	}
	return 1;
}
/*
struct EQUATION *common(struct EQUATION *eq){
	int i, j;
	if (eq->OPERATION == MULTIPLY){
  	for (i=0; i<eq->TOTAL_CHILD; ++i){
  	  for (j=0; j<eq->TOTAL_CHILD; ++j){
	      if (i != j && eq->CHILD[i]->OPERATION == EXPONENT && approx(eq->CHILD[i]->CHILD[1]->CONSTANT, -1.0f) && eq->CHILD[j]->OPERATION == EXPONENT && approx(eq->CHILD[j]->CHILD[1]->CONSTANT, -1.0f) && same(eq->CHILD[i]->CHILD[0], eq->CHILD[j]->CHILD[0])){
	            eq->CHILD[i]->CHILD[0] = operate_equation(MULTIPLY, eq->CHILD[i]->CHILD[0], eq->CHILD[j]->CHILD[0]);
	        remove_child(eq, j);
	        if (eq->TOTAL_CHILD == 1) return eq->CHILD[0];
	        return eq;
        }
		  }
	  }
  }
	return eq;
}
struct EQUATION *common2(struct EQUATION *eq){
	int i;
	int j;
	int k;
	struct EQUATION *rec;
	if (eq->OPERATION == ADD){
		for (i=0; i<eq->TOTAL_CHILD; ++i){
			if (eq->CHILD[i]->OPERATION == MULTIPLY){
				for (j=0; j<eq->CHILD[i]->TOTAL_CHILD; ++j){
					rec = eq->CHILD[i]->CHILD[j];
					for (k=0; k<eq->CHILD[i]->TOTAL_CHILD; ++k){
					}
			  		if (j != k && eq->CHILD[i]->CHILD[j]->OPERATION == EXPONENT && approx(eq->CHILD[i]->CHILD[j]->CHILD[1], -1.0f) && eq->CHILD[i]->CHILD[k]->OPERATION == EXPONENT && approx(eq->CHILD[i]->CHILD[k]->CHILD[1], -1.0f)){
		  			}
					for (k=0; k<eq->CHILD[i]->TOTAL_CHILD; ++k){
			  		if (j != k && eq->CHILD[i]->CHILD[j]->OPERATION == EXPONENT && approx(eq->CHILD[i]->CHILD[j]->CHILD[1], -1.0f)){
			  			if (same(eq->CHILD[i]->CHILD[j]->CHILD[0], eq->CHILD[i]->CHILD[k]->CHILD[0])){
			  				remove_child(eq->CHILD[])
			  			}
		  			}
					}
				}
			}
	}
}
*/



#define m(x, y) operate_equation(MULTIPLY, x, y)
#define a(x, y) operate_equation(ADD, x, y)
#define w(x, y) operate_equation(EXPONENT, x, y)
#define n(x) make_number(x)
void solve(struct EQUATION *eq){
	int i, j;
	for (i=0; i<20; ++i){
		for (j=0; j<20; ++j){
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
	x = make_variable(0, 1.0f, 1);
	//x = a(w(m(w(x, n(-1.0f)),a(x, n(1.0f))), n(-1.0f)), w(a(x, n(2.0f)), n(-1.0f)));
	//x = m(x, x);
	x = w(x, n(-1.0f));
	x = a(m(x, x), x);
	p(x);
//	x = cross(x);
//	p(x);
	x = distribute(x);
	p(x);
}
