#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define CONSTANT_TERM 0
#define VARIABLE_TERM 1
#define ADD 2
#define MULTIPLY 3
#define EXPONENT 4

#define NOT_A_FUNCTION 0
#define SINE 1
#define COSINE 2
#define COSECANT 3
#define SECANT 4
#define TANGENT 5
#define COTANGENT 6
#define LAWN 7

//TYPE made by oring function and operation
#define TYPE_MACRO(op, fx) (((op)<<3)|(fx))
#define TYPE_MACRO_OP(x) (((x)->TYPE)>>3)
#define TYPE_MACRO_FX(x) (((x)->TYPE)&0x7)

#define MAX_VARIABLE_IN_FORMULA 4

char *functionNameList[] = {"sin", "cos", "cosec", "sec", "tan", "cot", "ln"};

union EQUATION_VALUE {
	char VARIABLE_NAME;
	float CONSTANT_VALUE;
};

struct EQUATION {
	unsigned char TYPE;
	union EQUATION_VALUE VALUE;
	struct EQUATION *CHILD;
	struct EQUATION_SIS *NEXT_CHILD;
};

struct EQUATION_SIS {
	struct EQUATION *CHILD;
	struct EQUATION_SIS *NEXT_CHILD;
};

#define TOTAL_FORMULA_COUNT 7

struct EQUATION *formulaInputList[TOTAL_FORMULA_COUNT];
struct EQUATION *formulaOutputList[TOTAL_FORMULA_COUNT];

//Count number of child in an equation
int equationChildCount(struct EQUATION *eq){
	int i = 0;
	struct EQUATION_SIS *sis;
	while ((i++)?(sis = sis->NEXT_CHILD):(sis = eq->NEXT_CHILD));
	return i;
}
void printEq(struct EQUATION *eq);
//New copy of an equation
struct EQUATION *doCopy(struct EQUATION *eq){
	int i;
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	output->TYPE = eq->TYPE;
	if (TYPE_MACRO_OP(output) == VARIABLE_TERM){
		output->VALUE.VARIABLE_NAME = eq->VALUE.VARIABLE_NAME;
	} else if (TYPE_MACRO_OP(output) == CONSTANT_TERM){
		output->VALUE.CONSTANT_VALUE = eq->VALUE.CONSTANT_VALUE;
	}
	struct EQUATION_SIS *sis;
	struct EQUATION_SIS *sis_2;
	i=0;
	if (eq->CHILD){
		do {
			if (i==0){
				output->CHILD = doCopy(eq->CHILD);
				sis_2 = eq->NEXT_CHILD;
				if (!sis_2) output->NEXT_CHILD = NULL;
				else output->NEXT_CHILD = malloc(sizeof(struct EQUATION_SIS));
				sis = output->NEXT_CHILD;
			} else {
				sis->CHILD = doCopy(sis_2->CHILD);
				sis_2 = sis_2->NEXT_CHILD;
				if (!sis_2) sis->NEXT_CHILD = NULL;
				else sis->NEXT_CHILD = malloc(sizeof(struct EQUATION_SIS));
				sis = sis->NEXT_CHILD;
			}
			++i;
		} while (sis_2);
	} else {
		output->NEXT_CHILD = NULL;
		output->CHILD = NULL;
	}
	return output;
}

int approx(float a, float b){
	float diff = a-b;
	if (diff < 0.0f){
		diff = -diff;
	}
	return diff < 0.001f;
}

int isSame(struct EQUATION *eq1, struct EQUATION *eq2){
	int i;
	if (equationChildCount(eq1) == equationChildCount(eq2) && eq1->TYPE == eq2->TYPE){
		if (TYPE_MACRO_OP(eq1) == VARIABLE_TERM && eq1->VALUE.VARIABLE_NAME == eq2->VALUE.VARIABLE_NAME) return 1;
		if (TYPE_MACRO_OP(eq1) == CONSTANT_TERM && approx(eq1->VALUE.CONSTANT_VALUE, eq2->VALUE.CONSTANT_VALUE)) return 1;
		if (eq1->CHILD == 0) return 0;
		i =0;
		struct EQUATION_SIS *sis;
		struct EQUATION_SIS *sis_2;
		do {
			if (i==0){
				if (!isSame(eq1->CHILD, eq2->CHILD)) return 0;
				sis_2 = eq2->NEXT_CHILD;
				sis = eq1->NEXT_CHILD;
			} else {
				if (!isSame(sis->CHILD, sis_2->CHILD)) return 0;
				sis_2 = sis_2->NEXT_CHILD;
				sis = sis->NEXT_CHILD;
			}
			++i;
		} while (sis);
		return 1;
	}
	return 0;
}

//Check if formula is applicable and find the unknown terms in formula structure, ignoring commutative property
int isFormulaApplicable(struct EQUATION *eq, struct EQUATION *formula, struct EQUATION *varList[MAX_VARIABLE_IN_FORMULA]){
	int i;
	if (TYPE_MACRO_OP(formula) == VARIABLE_TERM){
		if (TYPE_MACRO_FX(formula) == NOT_A_FUNCTION){ //A formula variable accepts anything
			if (varList[formula->VALUE.VARIABLE_NAME - 'a'] == NULL){
				varList[formula->VALUE.VARIABLE_NAME - 'a'] = doCopy(eq);
				return 1;
			} else {
				return isSame(eq, varList[formula->VALUE.VARIABLE_NAME - 'a']);
			}
		} else if (TYPE_MACRO_FX(formula) == TYPE_MACRO_FX(eq)){ //If inside formula, function should be same now
			if (varList[formula->VALUE.VARIABLE_NAME - 'a'] == NULL){
				varList[formula->VALUE.VARIABLE_NAME - 'a'] = doCopy(eq);
				varList[formula->VALUE.VARIABLE_NAME - 'a']->TYPE &= ~0x7;
				return 1;
			} else {
				return isSame(eq, varList[formula->VALUE.VARIABLE_NAME - 'a']);
			}
		}
	} else if (equationChildCount(formula) == equationChildCount(eq) && TYPE_MACRO_OP(formula) == TYPE_MACRO_OP(eq) && TYPE_MACRO_FX(formula) == TYPE_MACRO_FX(eq)){ //Function, no of children and operator should be same
		if (TYPE_MACRO_OP(formula) == CONSTANT_TERM){
			return approx(formula->VALUE.CONSTANT_VALUE, eq->VALUE.CONSTANT_VALUE); //Return the constant even if its inside a function
		} else {
			int isApplicable = 1;
			struct EQUATION_SIS *sis;
			struct EQUATION_SIS *sis_2;
			i=0;
			do {
				if (i==0){
					if (!isFormulaApplicable(eq->CHILD, formula->CHILD, varList)) isApplicable = 0;
					sis_2 = formula->NEXT_CHILD;
					sis = eq->NEXT_CHILD;
				} else {
					if (!isFormulaApplicable(sis->CHILD, sis_2->CHILD, varList)) isApplicable = 0;
					sis_2 = sis_2->NEXT_CHILD;
					sis = sis->NEXT_CHILD;
				}
				++i;
			} while(sis);
			return isApplicable;
		}
	}
	return 0;
}
struct EQUATION_SIS *selectChild_helper(struct EQUATION *eq, int n){
	struct EQUATION_SIS *sis;
	sis = eq->NEXT_CHILD;
	if (n == 1) return sis;
	--n;
	while (sis->NEXT_CHILD){
		sis = sis->NEXT_CHILD;
		if (--n == 0) return sis;
	}
}

struct EQUATION *delChild_helper(struct EQUATION *eq, int n){
	struct EQUATION_SIS *sis;
	int count = equationChildCount(eq);
	if (n == 0){
		eq->CHILD = eq->NEXT_CHILD->CHILD;
		eq->NEXT_CHILD = eq->NEXT_CHILD->NEXT_CHILD;
		return eq;
	}
	if (count == 2 && n == 1){
		eq->NEXT_CHILD = 0;
		return eq;
	}
	if (n == count-1){
		sis = selectChild_helper(eq, count-2);
		sis->NEXT_CHILD = 0;
		return eq;
	}
	sis = eq->NEXT_CHILD;
	while (sis->NEXT_CHILD){
		if (--n == 0){
			sis->CHILD = sis->NEXT_CHILD->CHILD;
			sis->NEXT_CHILD = sis->NEXT_CHILD->NEXT_CHILD;
			return eq;
		}
		sis = sis->NEXT_CHILD;
	}
}

struct EQUATION *addChild_helper(struct EQUATION *eq, struct EQUATION *added){
	int count = equationChildCount(eq);
	if (count == 1){
		eq->NEXT_CHILD = malloc(sizeof(struct EQUATION_SIS));
		eq->NEXT_CHILD->CHILD = added;
		eq->NEXT_CHILD->NEXT_CHILD = 0;
	} else {
		struct EQUATION_SIS *sis = selectChild_helper(eq, count-1);
		sis->NEXT_CHILD = malloc(sizeof(struct EQUATION_SIS));
		sis->NEXT_CHILD->CHILD = added;
		sis->NEXT_CHILD->NEXT_CHILD = 0;
	}
	return eq;
}
struct EQUATION *fixCommutative(struct EQUATION *eq){
	int i, j;
	struct EQUATION_SIS *sis;
	if (TYPE_MACRO_OP(eq) == ADD || TYPE_MACRO_OP(eq) == MULTIPLY){
		unsigned char tmp = TYPE_MACRO_OP(eq);
		while (tmp == TYPE_MACRO_OP(eq->CHILD) && TYPE_MACRO_FX(eq->CHILD) == NOT_A_FUNCTION){
			eq = addChild_helper(eq, eq->CHILD->CHILD);
			for (j=1; j<equationChildCount(eq->CHILD); ++j){
				eq = addChild_helper(eq, selectChild_helper(eq->CHILD, j)->CHILD);
			}
			eq = delChild_helper(eq, 0);
		}
		for (i=1; i<equationChildCount(eq); ++i){
			sis = selectChild_helper(eq, i);
			if (tmp == TYPE_MACRO_OP(sis->CHILD) && TYPE_MACRO_FX(sis->CHILD) == NOT_A_FUNCTION){
				eq = addChild_helper(eq, sis->CHILD);
				for (j=1; j<equationChildCount(sis->CHILD); ++j){
					eq = addChild_helper(eq, selectChild_helper(sis->CHILD, j)->CHILD);
				}
				eq = delChild_helper(eq, i);
				--i;
			}
		}
	}
	if (eq->CHILD){
		eq->CHILD = fixCommutative(eq->CHILD);
		for (i=1; i<equationChildCount(eq); ++i){
			sis = selectChild_helper(eq, i);
			sis->CHILD = fixCommutative(sis->CHILD);
		}
	}
	return eq;
}

//Replace a formula's variable given the replacement
struct EQUATION *replaceOutputFormula(struct EQUATION *formula, struct EQUATION *varList[MAX_VARIABLE_IN_FORMULA]){
	int i;
	if (TYPE_MACRO_OP(formula) == VARIABLE_TERM){
		unsigned char tmp=0;
		if (TYPE_MACRO_FX(formula) != NOT_A_FUNCTION){
			tmp = TYPE_MACRO_FX(formula);
		}
		formula = doCopy(varList[formula->VALUE.VARIABLE_NAME - 'a']); //replace
		formula->TYPE |= tmp; //Preserve function which convered the variable
		return formula;
	}
	i=0;
	struct EQUATION_SIS *sis;
	if (formula->CHILD){
		do {
			if (i==0){
				formula->CHILD = replaceOutputFormula(formula->CHILD, varList);
				sis = formula->NEXT_CHILD;
			} else {
				sis->CHILD = replaceOutputFormula(sis->CHILD, varList);
				sis = sis->NEXT_CHILD;
			}
			++i;
		} while(sis);
	}
	return formula;
}

#define MAX_DEPTH_PV 250
#define MAX_MOVE_GEN_LIST_SIZE 25

#define MAX_TT_SIZE 1000

int findInTT(struct EQUATION *eq, int *ttCount, struct EQUATION *transpositionTable[MAX_TT_SIZE]){
	int i;
	for (i=0; i<(*ttCount); ++i){
		if (isSame(eq, transpositionTable[i])) return 1;
	}	
	return 0;
}

#define A(x, y) operateEquation(ADD, NOT_A_FUNCTION, x, y)
#define M(x, y) operateEquation(MULTIPLY, NOT_A_FUNCTION, x, y)  
#define W(x, y) operateEquation(EXPONENT, NOT_A_FUNCTION, x, y)
#define N(x) makeNumber(x)
#define V(x) makeVariable(x)
#define FX(type, x) makeFunction(type, x)

//Operate on two equations
struct EQUATION *operateEquation(unsigned char operation, unsigned char funcType, struct EQUATION *eq1, struct EQUATION *eq2){
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	output->TYPE = TYPE_MACRO(operation, funcType);
	output->CHILD = eq1;
	output->NEXT_CHILD = malloc(sizeof(struct EQUATION_SIS));
	output->NEXT_CHILD->CHILD = eq2;
	output->NEXT_CHILD->NEXT_CHILD = NULL;
	return output;
}

struct EQUATION *genMoves(struct EQUATION *eq, int n, int *nodeCount){
	struct EQUATION *varList[MAX_VARIABLE_IN_FORMULA];
	int count = equationChildCount(eq);
	int i, j;
	struct EQUATION *tmp;
	struct EQUATION *tmp_2;
	if (count == 2){
		memset(varList, 0, sizeof(struct EQUATION *)*MAX_VARIABLE_IN_FORMULA);
		if (--(*nodeCount) == 0 && isFormulaApplicable(eq, formulaInputList[n], varList)){
			return replaceOutputFormula(doCopy(formulaOutputList[n]), varList);
		}
		if (*nodeCount == 0) return eq;
		tmp = genMoves(eq->CHILD, n, nodeCount);
		if (tmp){
			eq->CHILD = tmp;
			return eq;
		}
		tmp = genMoves(eq->NEXT_CHILD->CHILD, n, nodeCount);
		if (tmp){
			eq->NEXT_CHILD->CHILD = tmp;
			return eq;
		}
	}
	else if (count > 2){
		for (i=0; i<count; ++i){
			for (j=i+1; j<count; ++j){
				struct EQUATION *combine;
				if (i==0){
					combine = operateEquation(TYPE_MACRO_OP(eq), TYPE_MACRO_FX(eq), eq->CHILD, selectChild_helper(eq, j)->CHILD);
				} else {
					combine = operateEquation(TYPE_MACRO_OP(eq), TYPE_MACRO_FX(eq), selectChild_helper(eq, i)->CHILD, selectChild_helper(eq, j)->CHILD);
				}
				memset(varList, 0, sizeof(struct EQUATION *)*MAX_VARIABLE_IN_FORMULA);
				if (--(*nodeCount) == 0 && isFormulaApplicable(combine, formulaInputList[n], varList)){
					tmp = doCopy(eq);
					if (n == 0 || n == 1){
						if (i==0){
							tmp_2 = tmp->CHILD;
							tmp->CHILD = selectChild_helper(tmp, j)->CHILD;
							selectChild_helper(tmp, j)->CHILD = tmp_2;
						} else {
							tmp_2 = selectChild_helper(tmp, i)->CHILD;
							selectChild_helper(tmp, i)->CHILD = selectChild_helper(tmp, j)->CHILD;
							selectChild_helper(tmp, j)->CHILD = tmp_2;
						}
						return tmp;
					}
					tmp = delChild_helper(tmp, j);
					tmp = delChild_helper(tmp, i);
					tmp = addChild_helper(tmp, replaceOutputFormula(doCopy(formulaOutputList[n]), varList));
					return fixCommutative(tmp);
				}
				if (*nodeCount == 0) return eq;
			}
			if (i==0){
				tmp = genMoves(eq->CHILD, n, nodeCount);
				if (tmp){
					eq->CHILD = tmp;
					return eq;
				}
			} else {
				tmp = genMoves(selectChild_helper(eq, i)->CHILD, n, nodeCount);
				if (tmp){
					selectChild_helper(eq, i)->CHILD = tmp;
					return eq;
				}
			}
		}
	}
	return 0;
}

void genMoveCount(struct EQUATION *eq, int *nodeCount){
	int count = equationChildCount(eq);
	int i, j;
	if (count == 2){
		++(*nodeCount);
		genMoveCount(eq->CHILD, nodeCount);
		genMoveCount(eq->NEXT_CHILD->CHILD, nodeCount);
	}
	else if (count > 2){
		for (i=0; i<count; ++i){
			for (j=i+1; j<count; ++j){
				++(*nodeCount);
			}
			if (i==0)
			genMoveCount(eq->CHILD, nodeCount);
			else
			genMoveCount(selectChild_helper(eq, i)->CHILD, nodeCount);
		}
	}
}

void makeList(struct EQUATION *eq, int *moveListCount, struct EQUATION *moveList[MAX_MOVE_GEN_LIST_SIZE]){
	int i, j;
	int nodeCount=0;
	genMoveCount(eq, &nodeCount);
	for (i=0; i<TOTAL_FORMULA_COUNT; ++i){
		for (j=1; j<=nodeCount; ++j){
			struct EQUATION *copied = doCopy(eq);
			int tmp= j;
			copied = genMoves(copied, i, &tmp);
			if (!isSame(copied, eq)){
				moveList[(*moveListCount)++] = copied;
			}
		}
	}
}

int constantOnlyCheck_helper(struct EQUATION *eq){
	if (TYPE_MACRO_OP(eq) == CONSTANT_TERM) return 1;
	struct EQUATION_SIS *sis;
	if (eq->CHILD){
		if (!constantOnlyCheck_helper(eq->CHILD)) return 0;
		sis = eq->NEXT_CHILD;
		while (sis){
			if (!constantOnlyCheck_helper(sis->CHILD)) return 0;
			sis = sis->NEXT_CHILD;
		}
		return 1;
	} else return 0;
}

float constantFormulate_helper(struct EQUATION *eq){
	float result;
	struct EQUATION_SIS *sis;
	float sum=0;
	float mul=1;
	float a, b;
	if (eq->CHILD == 0) result = eq->VALUE.CONSTANT_VALUE; 
	else {
		switch (TYPE_MACRO_OP(eq)){
			case ADD:
				sum += constantFormulate_helper(eq->CHILD);
				sis = eq->NEXT_CHILD;
				while (sis){
					sum += constantFormulate_helper(sis->CHILD);
					sis = sis->NEXT_CHILD;
				}
				result = sum;
				break;
			case MULTIPLY:
				mul *= constantFormulate_helper(eq->CHILD);
				sis = eq->NEXT_CHILD;
				while (sis){
					mul *= constantFormulate_helper(sis->CHILD);
					sis = sis->NEXT_CHILD;
				}
				result = mul;
				break;
			case EXPONENT:
				a = constantFormulate_helper(eq->CHILD);
				b = constantFormulate_helper(eq->NEXT_CHILD->CHILD);
				result = powf(a, b);
				break;
		}
	}
	switch (TYPE_MACRO_FX(eq)){
		case NOT_A_FUNCTION:
			return result;
		case SINE:
			return sinf(result);
		case COSINE:
			return cosf(result);
		case COSECANT:
			return 1/sinf(result);
		case SECANT:
			return 1/cosf(result);
		case TANGENT:
			return tanf(result);
		case COTANGENT:
			return 1/tanf(result);
		case LAWN:
			return logf(result);
	}
}

//Make a number in form of equation
struct EQUATION *makeNumber(float x){
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	output->TYPE = TYPE_MACRO(CONSTANT_TERM, NOT_A_FUNCTION);
	output->VALUE.CONSTANT_VALUE = x;
	output->CHILD = NULL;
	output->NEXT_CHILD = NULL;
	return output;
}

struct EQUATION *applyConstantEverywhere_helper(struct EQUATION *eq){
	if (constantOnlyCheck_helper(eq)){
		return makeNumber(constantFormulate_helper(eq)); 
	}
	if (eq->CHILD == 0) return eq;
	struct EQUATION_SIS *sis;
	eq->CHILD = applyConstantEverywhere_helper(eq->CHILD);
	sis = eq->NEXT_CHILD;
	while (sis){
		sis->CHILD = applyConstantEverywhere_helper(sis->CHILD);
		sis = sis->NEXT_CHILD;		
	}
	return eq;
}

struct EQUATION *formulatePartial_helper(struct EQUATION *eq){
	if (eq->CHILD == 0) return eq;
	int isPartial = 0;
	int count = equationChildCount(eq);
	int i;
	float sum = 0;
	float mul = 1;
	float result;
	struct EQUATION_SIS *sis;
	
	if (TYPE_MACRO_OP(eq->CHILD) != CONSTANT_TERM) isPartial = 1;
	for (i=1; i<count; ++i){
		if (TYPE_MACRO_OP(selectChild_helper(eq, i)->CHILD) != CONSTANT_TERM)
			isPartial = 1;
	}

	if (isPartial == 1){
		if (TYPE_MACRO_OP(eq->CHILD) == CONSTANT_TERM) ++isPartial;
		for (i=1; i<count; ++i){
			if (TYPE_MACRO_OP(selectChild_helper(eq, i)->CHILD) == CONSTANT_TERM)
				++isPartial;
		}
	}
	
	if (isPartial > 2){
		switch (TYPE_MACRO_OP(eq)){
			case ADD:
				if (TYPE_MACRO_OP(eq->CHILD) == CONSTANT_TERM) sum += eq->CHILD->VALUE.CONSTANT_VALUE;
				for (i=1; i<count; ++i){
					sis = selectChild_helper(eq, i);
					if (TYPE_MACRO_OP(sis->CHILD) == CONSTANT_TERM) sum += sis->CHILD->VALUE.CONSTANT_VALUE;
				}
				result = sum;
				break;
			case MULTIPLY:
				if (TYPE_MACRO_OP(eq->CHILD) == CONSTANT_TERM) mul *= eq->CHILD->VALUE.CONSTANT_VALUE;
				for (i=1; i<count; ++i){
					sis = selectChild_helper(eq, i);
					if (TYPE_MACRO_OP(sis->CHILD) == CONSTANT_TERM) mul *= sis->CHILD->VALUE.CONSTANT_VALUE;
				}
				result = mul;
				break;
		}
		while (TYPE_MACRO_OP(eq->CHILD) == CONSTANT_TERM){
			eq = delChild_helper(eq, 0);
		}
		
		for (i=1; i<equationChildCount(eq); ++i){
			if (TYPE_MACRO_OP(selectChild_helper(eq, i)->CHILD) == CONSTANT_TERM){
				eq = delChild_helper(eq, i);
				--i;
			}
		}
		addChild_helper(eq, makeNumber(result));
	}
	
	return eq;
}
struct EQUATION *fixConstant(struct EQUATION *eq){
	eq = applyConstantEverywhere_helper(eq);
	eq = formulatePartial_helper(eq);
	return eq;
}

void removeMoveList(int *moveListCount, struct EQUATION *moveList[MAX_MOVE_GEN_LIST_SIZE], int n){
	int i;
	for (i=n; i<*moveListCount-1; ++i){
		moveList[i] = moveList[i+1];
	}
	--(*moveListCount);
}

void moveListProc(int *moveListCount, struct EQUATION *moveList[MAX_MOVE_GEN_LIST_SIZE]){
	int i, j, k;
	for (i=0; i<*moveListCount; ++i){
		moveList[i] = fixCommutative(fixConstant(moveList[i]));
	}
	for (i=0; i<*moveListCount; ++i){
		for (j=i+1; j<*moveListCount; ++j){
			if (isSame(moveList[i], moveList[j])){
				for (k=j; k<*moveListCount-1; ++k){
					moveList[k] = moveList[k+1];
				}
				--(*moveListCount);
				--j;
			}
		}
	}
}

int search(struct EQUATION *eq, struct EQUATION *target, int depth, int *ttCount, struct EQUATION *transpositionTable[MAX_TT_SIZE], struct EQUATION *pv[MAX_DEPTH_PV]){
	//printEq(target);
	if (isSame(eq, target)){
		return 1;	
	}
	if (findInTT(eq, ttCount, transpositionTable)) return 0;
	else {
		transpositionTable[(*ttCount)++] = doCopy(eq);
	}
	struct EQUATION *moveList[MAX_MOVE_GEN_LIST_SIZE];
	int moveListCount = 0;
	makeList(eq, &moveListCount, moveList);
	moveListProc(&moveListCount, moveList);
	int i;
	//for (i=0; i<moveListCount; ++i) printEq(moveList[i]);
	for (i=0; i<moveListCount; ++i){
		struct EQUATION *tmp = doCopy(moveList[i]);
		//printEq(tmp);
		if (search(tmp, target, depth-1, ttCount, transpositionTable, pv)){
			pv[depth] = tmp; 
			return 1;
		}
	}
	return 0;
}

//Make variable with a char letter as name
struct EQUATION *makeVariable(char variableName){
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	output->TYPE = TYPE_MACRO(VARIABLE_TERM, NOT_A_FUNCTION);
	output->VALUE.VARIABLE_NAME = variableName;
	output->CHILD = NULL;
	output->NEXT_CHILD = NULL;
	return output;
}

//Put a function outside the input equation
struct EQUATION *makeFunction(unsigned char funcType, struct EQUATION *eq){
	struct EQUATION *output = doCopy(eq);
	output->TYPE |= funcType;
	return output;
}

//Print equation
void printEquation(struct EQUATION *eq){
	int i;
	if (TYPE_MACRO_FX(eq) != NOT_A_FUNCTION){
		printf("%s", functionNameList[TYPE_MACRO_FX(eq)-1]);
		if (TYPE_MACRO_OP(eq) == CONSTANT_TERM || TYPE_MACRO_OP(eq) == VARIABLE_TERM) printf("("); //In operations already bracket is printed, no need to print twice
	}
	if (TYPE_MACRO_OP(eq) == CONSTANT_TERM){
		printf("%2.2f", eq->VALUE.CONSTANT_VALUE);
	}
	else if (TYPE_MACRO_OP(eq) == VARIABLE_TERM){
		printf("%c", eq->VALUE.VARIABLE_NAME);
	}
	else if (TYPE_MACRO_OP(eq) == ADD || TYPE_MACRO_OP(eq) == MULTIPLY || TYPE_MACRO_OP(eq) == EXPONENT){
		printf("(");
		char op;
		switch(TYPE_MACRO_OP(eq)){
			case ADD:
				op = '+';
				break;
			case MULTIPLY:
				op = '*';
				break;
			case EXPONENT:
				op = '^';
		}
		struct EQUATION_SIS *sis;
		i = 0;
		do {
			if (i==0){
				printEquation(eq->CHILD);
				sis = eq->NEXT_CHILD;
			} else {
				printf("%c", op);
				printEquation(sis->CHILD);
				sis = sis->NEXT_CHILD;
			}
			++i;
		} while (sis);
		printf(")");
	}
	if (TYPE_MACRO_FX(eq) != NOT_A_FUNCTION && (TYPE_MACRO_OP(eq) == CONSTANT_TERM || TYPE_MACRO_OP(eq) == VARIABLE_TERM)) printf(")");
}

//Print equation with a new line
void printEq(struct EQUATION *eq){
	printEquation(eq);
	printf(" ");
}

void formulaGen(){
	formulaInputList[0] = A(V('a'), V('b'));  //commutative property in addition
	formulaOutputList[0] = A(V('b'), V('a'));
	
	formulaInputList[1] = M(V('a'), V('b'));  //commutative property in multiply
	formulaOutputList[1] = M(V('b'), V('a'));
	
	formulaInputList[2] = M(V('a'), N(1)); //one can be ignored in multiplication
	formulaOutputList[2] = V('a');
	
	formulaInputList[3] = A(  M(V('a'),  V('b')), V('a')  );   //taking common (1)
	formulaOutputList[3] = M(  V('a'),  A(V('b'), N(1))  );
	
	formulaInputList[4] = A(V('a'), V('a')); // taking common (2)
	formulaOutputList[4] = M(N(2), V('a'));
	
	formulaInputList[5] = M(A(V('a'), V('b')), A(V('c'), V('d')));
	formulaOutputList[5] = A(A(M(V('a'), V('c')), M(V('a'), V('d'))), A( M(V('b'), V('c')), M(V('b'), V('d'))));
	
	formulaInputList[6] = M(V('a'), V('a'));
	formulaOutputList[6] = W(V('a'), N(2));
}

int main(){
	int i;
	formulaGen();	
	struct EQUATION *start = M(A(V('x'), N(1)), A(V('x'), N(2)));
	struct EQUATION *target = A(A(W(V('x'), N(2)), M(N(3), V('x'))), N(2));
	//struct EQUATION *target = A(A(M(V('x'), V('x')), M(V('x'), N(2))), A( M(V('x'), N(1)), N(2)));
	target = fixCommutative(target);
	//printEq(target);
	
	printf("Start: ");
	printEq(start);
	printf("Target: ");
	printEq(target);
	printf("\n");
	printf("(ENDS UP HERE)\n");
	struct EQUATION *pv[MAX_DEPTH_PV];
	for (i=0; i<MAX_DEPTH_PV; ++i) pv[i] = 0;
	struct EQUATION *transpositionTable[MAX_TT_SIZE];
	int ttCount = 0;
	search(start, target, 249, &ttCount, transpositionTable, pv);
	for (i=0; i<MAX_DEPTH_PV; ++i){
		if (pv[i]){
			printEq(pv[i]);
		}
	}
	printEq(start);
	printf("\n(STARTS HERE)");
	
	return 0;
}
