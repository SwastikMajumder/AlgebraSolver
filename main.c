#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define CONSTANT_TERM 0
#define VARIABLE_TERM 1
#define ADD 2
#define MULTIPLY 3
#define EXPONENT 4
#define SINE 5
#define COSINE 6
#define COSECANT 7
#define SECANT 8
#define TANGENT 9
#define COTANGENT 10
#define LAWN 11

#define MAX_BRACKET_GROUP 50
#define MAX_BRACKET_GROUP_LIST_SIZE 200
#define MAX_LONG_BRACKET 50
#define MAX_LONG_BRACKET_GEN_LIST 250
#define TOTAL_FORMULA_COUNT 15
#define MAX_VARIABLE_IN_FORMULA 4
#define MAX_MOVE_GEN_LIST_SIZE 200
#define MAX_TT_SIZE 1000

#define FX(type, x) operateEquation(type, x)
#define A(x, y) operateEquation2(ADD, x, y)
#define M(x, y) operateEquation2(MULTIPLY, x, y)
#define W(x, y) operateEquation2(EXPONENT, x, y)
#define N(x) makeNumber(x)
#define V(x) makeVariable(x)

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

void printEq(struct EQUATION *eq);

struct EQUATION *formulaInputList[TOTAL_FORMULA_COUNT];
struct EQUATION *formulaOutputList[TOTAL_FORMULA_COUNT];

int equationChildCount(struct EQUATION *eq){
	int i = 0;
	struct EQUATION_SIS *sis;
	while ((i++)?(sis = sis->NEXT_CHILD):(sis = eq->NEXT_CHILD));
	return i;
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

struct EQUATION *doCopy(struct EQUATION *eq){
	int i;
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	output->TYPE = eq->TYPE;
	if (output->TYPE == VARIABLE_TERM){
		output->VALUE.VARIABLE_NAME = eq->VALUE.VARIABLE_NAME;
	} else if (output->TYPE == CONSTANT_TERM){
		output->VALUE.CONSTANT_VALUE = eq->VALUE.CONSTANT_VALUE;
	}
	struct EQUATION_SIS *sis;
	int count = equationChildCount(eq);
	if (count == 1 && eq->CHILD == NULL){
		output->NEXT_CHILD = NULL;
		output->CHILD = NULL;
		return output;
	}
	if (count == 1){
		output->NEXT_CHILD = NULL;
		output->CHILD = doCopy(eq->CHILD);
		return output;
	}
	if (count > 1){
		sis = output->NEXT_CHILD = malloc(sizeof(struct EQUATION_SIS));	
		output->CHILD = doCopy(eq->CHILD);
		for (i=1; i<count; ++i){
			sis->CHILD = doCopy(selectChild_helper(eq, i)->CHILD);
			if (count - 1 == i)
				sis->NEXT_CHILD = NULL;
			else 
				sis = sis->NEXT_CHILD = malloc(sizeof(struct EQUATION_SIS));
		}
	}
	return output;
}


struct EQUATION *delChild_helper(struct EQUATION *eq, int n){
	struct EQUATION_SIS *sis;
	int count = equationChildCount(eq);
	if (count == 1 && n == 0){
		eq->CHILD = 0;
		eq->NEXT_CHILD = 0;
		return eq;
	}
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
	if (count == 1 && eq->CHILD == 0){
		eq->CHILD = added;
	}
	else if (count == 1){
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

struct EQUATION *operateEquation(unsigned char type, struct EQUATION *eq){
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	output->TYPE = type;
	output->CHILD = doCopy(eq);
	output->NEXT_CHILD = NULL;
	return output;
}

struct EQUATION *operateEquation2(unsigned char type, struct EQUATION *eq1, struct EQUATION *eq2){
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	output->TYPE = type;
	output->CHILD = doCopy(eq1);
	output->NEXT_CHILD = malloc(sizeof(struct EQUATION_SIS));
	output->NEXT_CHILD->CHILD = doCopy(eq2);
	output->NEXT_CHILD->NEXT_CHILD = NULL;
	return output;
}

struct EQUATION *makeVariable(char variableName){
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	output->TYPE = VARIABLE_TERM;
	output->VALUE.VARIABLE_NAME = variableName;
	output->CHILD = NULL;
	output->NEXT_CHILD = NULL;
	return output;
}

struct EQUATION *makeNumber(float x){
	struct EQUATION *output = malloc(sizeof(struct EQUATION));
	output->TYPE = CONSTANT_TERM;
	output->VALUE.CONSTANT_VALUE = x;
	output->CHILD = NULL;
	output->NEXT_CHILD = NULL;
	return output;
}

struct EQUATION *fixCommutative(struct EQUATION *eq){
	int i, j;
	struct EQUATION_SIS *sis;
	if (eq->TYPE == ADD || eq->TYPE == MULTIPLY){
		unsigned char tmp = eq->TYPE;
		while (tmp == eq->CHILD->TYPE){
			eq = addChild_helper(eq, eq->CHILD->CHILD);
			for (j=1; j<equationChildCount(eq->CHILD); ++j){
				eq = addChild_helper(eq, selectChild_helper(eq->CHILD, j)->CHILD);
			}
			eq = delChild_helper(eq, 0);
		}
		for (i=1; i<equationChildCount(eq); ++i){
			sis = selectChild_helper(eq, i);
			if (tmp == sis->CHILD->TYPE){
				eq = addChild_helper(eq, sis->CHILD->CHILD);
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
		if (eq->NEXT_CHILD){
			for (i=1; i<equationChildCount(eq); ++i){
				sis = selectChild_helper(eq, i);
				sis->CHILD = fixCommutative(sis->CHILD);
			}
		}
	}
	return eq;
}

int constantOnlyCheck_helper(struct EQUATION *eq){
	if (eq->TYPE == CONSTANT_TERM) return 1;
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
	struct EQUATION_SIS *sis;
	float sum=0;
	float mul=1;
	float a, b;
	if (eq->CHILD == 0) return eq->VALUE.CONSTANT_VALUE; 
	else {
		switch (eq->TYPE){
			case ADD:
				sum += constantFormulate_helper(eq->CHILD);
				sis = eq->NEXT_CHILD;
				while (sis){
					sum += constantFormulate_helper(sis->CHILD);
					sis = sis->NEXT_CHILD;
				}
				return sum;
			case MULTIPLY:
				mul *= constantFormulate_helper(eq->CHILD);
				sis = eq->NEXT_CHILD;
				while (sis){
					mul *= constantFormulate_helper(sis->CHILD);
					sis = sis->NEXT_CHILD;
				}
				return mul;
			case EXPONENT:
				a = constantFormulate_helper(eq->CHILD);
				b = constantFormulate_helper(eq->NEXT_CHILD->CHILD);
				return powf(a, b);
			case SINE:
				return sinf(constantFormulate_helper(eq->CHILD));
			case COSINE:
				return cosf(constantFormulate_helper(eq->CHILD));
			case COSECANT:
				return 1/sinf(constantFormulate_helper(eq->CHILD));
			case SECANT:
				return 1/cosf(constantFormulate_helper(eq->CHILD));
			case TANGENT:
				return tanf(constantFormulate_helper(eq->CHILD));
			case COTANGENT:
				return 1/tanf(constantFormulate_helper(eq->CHILD));
			case LAWN:
				return logf(constantFormulate_helper(eq->CHILD));
		}
	}
	return 0;
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
	
	if (eq->CHILD->TYPE != CONSTANT_TERM) isPartial = 1;
	for (i=1; i<count; ++i){
		if (selectChild_helper(eq, i)->CHILD->TYPE != CONSTANT_TERM)
			isPartial = 1;
	}

	if (isPartial == 1){
		if (eq->CHILD->TYPE == CONSTANT_TERM) ++isPartial;
		for (i=1; i<count; ++i){
			if (selectChild_helper(eq, i)->CHILD->TYPE == CONSTANT_TERM)
				++isPartial;
		}
	}
	
	if (isPartial > 2){
		switch (eq->TYPE){
			case ADD:
				if (eq->CHILD->TYPE == CONSTANT_TERM) sum += eq->CHILD->VALUE.CONSTANT_VALUE;
				for (i=1; i<count; ++i){
					sis = selectChild_helper(eq, i);
					if (sis->CHILD->TYPE == CONSTANT_TERM) sum += sis->CHILD->VALUE.CONSTANT_VALUE;
				}
				result = sum;
				break;
			case MULTIPLY:
				if (eq->CHILD->TYPE == CONSTANT_TERM) mul *= eq->CHILD->VALUE.CONSTANT_VALUE;
				for (i=1; i<count; ++i){
					sis = selectChild_helper(eq, i);
					if (sis->CHILD->TYPE == CONSTANT_TERM) mul *= sis->CHILD->VALUE.CONSTANT_VALUE;
				}
				result = mul;
				break;
		}
		while (eq->CHILD->TYPE == CONSTANT_TERM){
			eq = delChild_helper(eq, 0);
		}
		
		for (i=1; i<equationChildCount(eq); ++i){
			if (selectChild_helper(eq, i)->CHILD->TYPE == CONSTANT_TERM){
				eq = delChild_helper(eq, i);
				--i;
			}
		}
		addChild_helper(eq, makeNumber(result));
	}
	count = equationChildCount(eq);
	eq->CHILD = formulatePartial_helper(eq->CHILD);
	for (i=1; i<count; ++i){
		sis = selectChild_helper(eq, i);
		sis->CHILD = formulatePartial_helper(sis->CHILD);
	}
	return eq;
}

struct EQUATION *fixConstant(struct EQUATION *eq){
	eq = applyConstantEverywhere_helper(eq);
	eq = formulatePartial_helper(eq);
	return eq;
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
		if (eq1->TYPE == VARIABLE_TERM && eq1->VALUE.VARIABLE_NAME == eq2->VALUE.VARIABLE_NAME) return 1;
		if (eq1->TYPE == CONSTANT_TERM && approx(eq1->VALUE.CONSTANT_VALUE, eq2->VALUE.CONSTANT_VALUE)) return 1;
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

struct EQUATION *replaceOutputFormula(struct EQUATION *formula, struct EQUATION *varList[MAX_VARIABLE_IN_FORMULA]){
	int i;
	if (formula->TYPE == VARIABLE_TERM){
		formula = doCopy(varList[formula->VALUE.VARIABLE_NAME - 'a']);
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

void bracketGroupList2(int bg[MAX_BRACKET_GROUP], int bgList[MAX_BRACKET_GROUP_LIST_SIZE][MAX_BRACKET_GROUP], int *bgListCount,
					  int totalSum, int prevNum, int sum, int location){
	int i;
	int j;
	for (i=prevNum;(i+sum)<=totalSum; ++i){
		bg[location] = i;
		if ((i+sum) == totalSum){
			for (j=0; j<=location; ++j){
				bgList[*bgListCount][j] = bg[j];
			}
			(*bgListCount)++;
			break;
		}
		bracketGroupList2(bg, bgList, bgListCount, totalSum, i, sum+i, location+1);
	}
}

struct EQUATION *genMovesCommutative(struct EQUATION *eq, int *nodeCount){
	int count = equationChildCount(eq);
	int i, j;
	struct EQUATION *tmp;
	struct EQUATION *tmp_2;
	if (eq->TYPE < EXPONENT && count > 1){
		for (i=0; i<count; ++i){
			for (j=i+1; j<count; ++j){
				if (--(*nodeCount) == 0){
					tmp = doCopy(eq);
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
				if (*nodeCount == 0) return eq;
			}
		}
	}
	if (eq->CHILD){
		for (i=0; i<count; ++i){
			if (i==0){
				tmp = genMovesCommutative(eq->CHILD, nodeCount);
				if (tmp){
					eq->CHILD = tmp;
					return eq;
				}
			} else {
				tmp = genMovesCommutative(selectChild_helper(eq, i)->CHILD, nodeCount);
				if (tmp){
					selectChild_helper(eq, i)->CHILD = tmp;
					return eq;
				}
			}
		}
	}
	return 0;
}

void genMoveCountCommutative(struct EQUATION *eq, int *nodeCount){
	int i, j;
	int count = equationChildCount(eq);
	if (eq->TYPE < EXPONENT && count > 1){
		for (i=0; i<count; ++i){
			for (j=i+1; j<count; ++j){
				++(*nodeCount);
			}
		}
	}
	if (eq->CHILD){
		for (i=0; i<count; ++i){
			if (i==0)
			genMoveCountCommutative(eq->CHILD, nodeCount);
			else
			genMoveCountCommutative(selectChild_helper(eq, i)->CHILD, nodeCount);
		}
	}
}
struct EQUATION *bracketGrouper_helper(struct EQUATION *eq, int bg[MAX_BRACKET_GROUP]){
	int i, j;
	struct EQUATION *tmp;
	struct EQUATION *output;
	int count = equationChildCount(eq);
	output = doCopy(eq);
	int not_one = 0;
	for (i=0; bg[i]; ++i) if (bg[i] != 1) not_one = 1;
	if (!not_one) return output;
	for (i=0; i<count; ++i) delChild_helper(output, 0);
	for (i=0; bg[i]; ++i){
		int sum_left=0;
		int sum_right=0;
		if (bg[i] == 1){
			for (j=0; j<i; ++j) sum_left+=bg[j];
			if (sum_left == 0) output = addChild_helper(output, doCopy(eq->CHILD));
			else output = addChild_helper(output, selectChild_helper(eq, sum_left)->CHILD);
		} else {
			tmp = doCopy(eq);
			for (j=0; j<i; ++j) sum_left+=bg[j];
			for (j=0; j<sum_left; ++j) tmp = delChild_helper(tmp, 0);
			for (j=i+1; bg[j]; ++j) sum_right+=bg[j];
			for (j=0; j<sum_right; ++j) tmp = delChild_helper(tmp, bg[i]);
			output = addChild_helper(output, tmp);
		}
	}
	return output;
}
int isFormulaApplicable(struct EQUATION *eq, struct EQUATION *formula, struct EQUATION *varList[MAX_VARIABLE_IN_FORMULA]){
	int i, j;
	struct EQUATION *tmp;
	struct EQUATION_SIS *sis;
	struct EQUATION_SIS *sis_2;
	int isApplicable;
	if (formula->TYPE == VARIABLE_TERM){
		if (varList[formula->VALUE.VARIABLE_NAME - 'a'] == NULL){
			varList[formula->VALUE.VARIABLE_NAME - 'a'] = doCopy(eq);
			return 1;
		} else {
			return isSame(eq, varList[formula->VALUE.VARIABLE_NAME - 'a']);
		}
	} else if (equationChildCount(formula) == equationChildCount(eq) && eq->TYPE == formula->TYPE){
		if (formula->TYPE == CONSTANT_TERM){
			return approx(formula->VALUE.CONSTANT_VALUE, eq->VALUE.CONSTANT_VALUE);
		} else {
			isApplicable = 1;
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
struct EQUATION *genMoves(struct EQUATION *eq, int n, int *nodeCount){
	int i;
	struct EQUATION *varList[MAX_VARIABLE_IN_FORMULA];
	int count = equationChildCount(eq);
	struct EQUATION *tmp;
	if (eq->CHILD){
		memset(varList, 0, sizeof(struct EQUATION *)*MAX_VARIABLE_IN_FORMULA);
		if (--(*nodeCount) == 0 && isFormulaApplicable(eq, formulaInputList[n], varList)){
			tmp = replaceOutputFormula(doCopy(formulaOutputList[n]), varList);
			return tmp;
		}
		if (*nodeCount == 0) return eq;
		for (i=0; i<count; ++i){
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
	int i;
	int count = equationChildCount(eq);
	if (eq->CHILD){
		++(*nodeCount);
		for (i=0; i<count; ++i){
			if (i==0)
			genMoveCount(eq->CHILD, nodeCount);
			else
			genMoveCount(selectChild_helper(eq, i)->CHILD, nodeCount);
		}
	}
}
void makeList(struct EQUATION *eq, int *moveListCount, struct EQUATION *moveList[MAX_MOVE_GEN_LIST_SIZE]){
	int i, j;
	int k;
	int nodeCount=0;
	genMoveCountCommutative(eq, &nodeCount);
	for (j=1; j<=nodeCount; ++j){
		struct EQUATION *copied = doCopy(eq);
		int tmp= j;
		copied = genMovesCommutative(copied, &tmp);
		if (!isSame(copied, eq)){
			moveList[(*moveListCount)++] = copied;
		}
	}
	struct EQUATION *genList[MAX_LONG_BRACKET_GEN_LIST];
	int genListCount = longBracketGen(eq, genList);
	for (k=0; k<genListCount; ++k){
		nodeCount = 0;
		genMoveCount(genList[k], &nodeCount);
		for (i=0; i<TOTAL_FORMULA_COUNT; ++i){
			for (j=1; j<=nodeCount; ++j){
				struct EQUATION *copied = doCopy(genList[k]);
				int tmp= j;
				copied = genMoves(copied, i, &tmp);
				if (!isSame(copied, genList[k])){
					moveList[(*moveListCount)++] = copied;
				}
			}
		}
	}
}

void moveListProc(int *moveListCount, struct EQUATION *moveList[MAX_MOVE_GEN_LIST_SIZE]){
	int i, j, k;
	for (i=0; i<*moveListCount; ++i){
		moveList[i] = fixCommutative(moveList[i]);
		moveList[i] = fixConstant(moveList[i]);
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

int findInTT(struct EQUATION *eq, int *ttCount, struct EQUATION *transpositionTable[MAX_TT_SIZE]){
	int i;
	for (i=0; i<(*ttCount); ++i){
		if (isSame(eq, transpositionTable[i])) return 1;
	}	
	return 0;
}

struct MATH_STEP {
	struct MATH_STEP *PARENT;
	struct EQUATION *DATA;
};

struct QUEUE_OBJ {
	struct MATH_STEP *STEP;
	struct QUEUE_OBJ *NEXT_STEP;
};

void enqueue(struct QUEUE_OBJ **queueStart, struct QUEUE_OBJ **queueEnd, struct MATH_STEP *added){
	if (*queueEnd == 0){
		*queueEnd = malloc(sizeof(struct QUEUE_OBJ));
		*queueStart = *queueEnd;
	} else {
		(*queueEnd)->NEXT_STEP = malloc(sizeof(struct QUEUE_OBJ));
		*queueEnd = (*queueEnd)->NEXT_STEP;		
	}
	(*queueEnd)->STEP = malloc(sizeof(struct MATH_STEP));
	memcpy((*queueEnd)->STEP, added, sizeof(struct MATH_STEP));
	(*queueEnd)->NEXT_STEP = 0;
}

struct MATH_STEP *dequeue(struct QUEUE_OBJ **queueStart, struct QUEUE_OBJ **queueEnd){
	struct MATH_STEP *output = malloc(sizeof(struct MATH_STEP));
	memcpy(output, (*queueStart)->STEP, sizeof(struct MATH_STEP));
	*queueStart = (*queueStart)->NEXT_STEP;
	if (*queueStart == 0) *queueEnd = 0;
	return output;
}

struct MATH_STEP *search(struct EQUATION *eq, struct EQUATION *target){
	struct EQUATION *transpositionTable[MAX_TT_SIZE];
	int ttCount = 0;
	int i;
	struct EQUATION *moveList[MAX_MOVE_GEN_LIST_SIZE];
	int moveListCount = 0;
	
	struct QUEUE_OBJ *queueStart=0;
	struct QUEUE_OBJ *queueEnd=0;
	
	transpositionTable[ttCount++] = doCopy(eq);
	
	struct MATH_STEP step;
	step.PARENT = 0;
	step.DATA = doCopy(eq);
	
	enqueue(&queueStart, &queueEnd, &step);
	
	while (queueEnd){
		struct MATH_STEP *v = dequeue(&queueStart, &queueEnd);
		
		if (isSame(v->DATA, target)){
			return v;
		}
		
		memset(moveList, 0, sizeof(struct EQUATION *)*MAX_MOVE_GEN_LIST_SIZE);
		moveListCount = 0;
		
		makeList(v->DATA, &moveListCount, moveList);
		moveListProc(&moveListCount, moveList);
		
		for (i=0; i<moveListCount; ++i){
			struct EQUATION *w = doCopy(moveList[i]);
			if (findInTT(w, &ttCount, transpositionTable) == 0){
				transpositionTable[ttCount++] = doCopy(w);	
				step.DATA = doCopy(w);
				step.PARENT = v;
				enqueue(&queueStart, &queueEnd, &step);
			}
		}
	}
	return 0;
}

void printEquation(struct EQUATION *eq){
	int i;
	if (eq->TYPE > EXPONENT){
		printf("%s", functionNameList[eq->TYPE - EXPONENT -1]);
		if (eq->CHILD->TYPE > EXPONENT || eq->CHILD->TYPE == CONSTANT_TERM || eq->CHILD->TYPE == VARIABLE_TERM) printf("(");
		printEquation(eq->CHILD);
		if (eq->CHILD->TYPE > EXPONENT || eq->CHILD->TYPE == CONSTANT_TERM || eq->CHILD->TYPE == VARIABLE_TERM) printf(")");
		return;
	}
	if (eq->TYPE == CONSTANT_TERM){
		printf("%2.2f", eq->VALUE.CONSTANT_VALUE);
	}
	else if (eq->TYPE == VARIABLE_TERM){
		printf("%c", eq->VALUE.VARIABLE_NAME);
	}
	else if (eq->TYPE == ADD || eq->TYPE == MULTIPLY || eq->TYPE == EXPONENT){
		printf("(");
		char op;
		switch(eq->TYPE){
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
}

void printEq(struct EQUATION *eq){
	printEquation(eq);
	printf("\n");
}

void formulaGen(){	
	formulaInputList[0] = M(V('a'), N(1));
	formulaOutputList[0] = V('a');
	
	formulaInputList[1] = A(  M(V('a'),  V('b')), V('a')  );
	formulaOutputList[1] = M(  V('a'),  A(V('b'), N(1))  );
	
	formulaInputList[2] = A(V('a'), V('a'));
	formulaOutputList[2] = M(N(2), V('a'));
	
	formulaInputList[3] = M(A(V('a'), V('b')), A(V('c'), V('d')));
	formulaOutputList[3] = A(A(M(V('a'), V('c')), M(V('a'), V('d'))), A( M(V('b'), V('c')), M(V('b'), V('d'))));
	
	formulaInputList[4] = M(V('a'), V('a'));
	formulaOutputList[4] = W(V('a'), N(2));
	
	formulaInputList[5] = A(W(FX(SINE, V('a')), N(2)), W(FX(COSINE, V('a')), N(2)));
	formulaOutputList[5] = N(1);
	
	//formulaInputList[6] = W(A(V('a'), V('b')), N(2));
	//formulaOutputList[6] = A(A(W(V('a'), N(2)), W(V('b'), N(2))), M(N(2), M(V('a'), V('b'))));
	
	formulaInputList[6] = FX(COTANGENT, V('a'));
	formulaOutputList[6] = M(FX(COSINE, V('a')), W(FX(SINE, V('a')), N(-1)));
	
	formulaInputList[7] = FX(COSECANT, V('a'));
	formulaOutputList[7] = W(FX(SINE, V('a')), N(-1));
	
	formulaInputList[8] = A(V('a'), M(V('a'), V('b')));
	formulaOutputList[8] = M(V('a'), A(V('b'), N(1)));

	formulaInputList[9] = A(M(V('b'), V('a')), V('a'));
	formulaOutputList[9] = M(V('a'), A(N(1), V('b')));
	
	formulaInputList[10] = W(FX(SINE, V('a')), N(2));
	formulaOutputList[10] = A(N(1),M(W(FX(COSINE,V('a')),N(2)),N(-1)));
	
	formulaInputList[11] = W(M(V('a'), V('b')),N(2));
	formulaOutputList[11] = M(W(V('a'),N(2)),W(V('b'),N(2)));
	
	formulaInputList[12] = W(W(V('a'),V('b')),V('c'));
	formulaOutputList[12] = W(W(V('a'),V('c')),V('b'));
	
	formulaInputList[13] = A(N(1),M(W(V('a'),N(2)),N(-1)));
	formulaOutputList[13] = M(A(N(1),V('a')),A(N(1),M(V('a'),N(-1))));
	
	formulaInputList[14] = M(W(V('a'),N(2)),W(M(V('a'),V('b')),N(-1)));
	formulaOutputList[14] = M(V('a'),W(V('b'),N(-1)));
}

void addOne(int *limitAry, int limitCount, int *output){
	output[0] += 1;
	int i;
	for (i=0; i<limitCount; ++i){
		if (output[i] == limitAry[i]){
			output[i+1]++;
			output[i] = 0;
		}
	}
}

void iterateLimit(int *limitAry, int limitCount, int n, int *output){
	int i, j;
	memset(output, 0, sizeof(int)*limitCount);
	for (i=0; i<n; ++i){
		addOne(limitAry, limitCount, output);
	}
}

int multiplyLimit(int *limitAry, int limitCount){
	int mul = 1;
	int i;
	for (i=0; i<limitCount; ++i){
		mul *= limitAry[i];
	}
	return mul;
}

void longBracketSetLimit(struct EQUATION *eq, int *limitAry, int *limitCount){
	int i, j;
	int count = equationChildCount(eq);
	if (eq->CHILD) longBracketSetLimit(eq->CHILD, limitAry, limitCount);
	for (i=1; i<count; ++i){
		longBracketSetLimit(selectChild_helper(eq, i)->CHILD, limitAry, limitCount);
	}
	if (count > 2){
		int bg[MAX_BRACKET_GROUP];
		int bgList[MAX_BRACKET_GROUP_LIST_SIZE][MAX_BRACKET_GROUP];
		for (i=0; i<MAX_BRACKET_GROUP_LIST_SIZE; ++i)
			for (j=0; j<MAX_BRACKET_GROUP; ++j)
				bgList[i][j] = 0;
		int bgListCount = 0;
		bracketGroupList2(bg, bgList, &bgListCount, count, 1, 0, 0);
		limitAry[(*limitCount)++] = bgListCount-1;
	}
}

struct EQUATION *longBracketReturn(struct EQUATION *eq, int *limitAry, int *nodeCount){
	int i, j;
	int count = equationChildCount(eq);
	if (eq->CHILD) eq->CHILD = longBracketReturn(eq->CHILD, limitAry, nodeCount);
	for (i=1; i<count; ++i){
		struct EQUATION_SIS *sis = selectChild_helper(eq, i);
		sis->CHILD = longBracketReturn(sis->CHILD, limitAry, nodeCount);
	}
	if (count > 2){
		int bg[MAX_BRACKET_GROUP];
		int bgList[MAX_BRACKET_GROUP_LIST_SIZE][MAX_BRACKET_GROUP];
		for (i=0; i<MAX_BRACKET_GROUP_LIST_SIZE; ++i)
			for (j=0; j<MAX_BRACKET_GROUP; ++j)
				bgList[i][j] = 0;
		int bgListCount = 0;
		bracketGroupList2(bg, bgList, &bgListCount, count, 1, 0, 0);
		int index = limitAry[(*nodeCount)++];
		for (i=0; bgList[index][i]; ++i){
			bg[i] = bgList[index][i];
		}
		bg[i] = 0;
		return bracketGrouper_helper(eq, bg);
	}
	return eq;
}

int longBracketGen(struct EQUATION *eq, struct EQUATION *genList[MAX_LONG_BRACKET_GEN_LIST]){
	int i;
	int j;
	int count=0;
	int limitAry[MAX_LONG_BRACKET];
	int limitCount = 0;
	longBracketSetLimit(eq, limitAry, &limitCount);
	int output[MAX_LONG_BRACKET];
	int result = multiplyLimit(limitAry, limitCount);
	for (i=0; i<result; ++i){
		iterateLimit(limitAry, limitCount, i, output);
		int nodeCount = 0;
		genList[count++] = longBracketReturn(doCopy(eq), output, &nodeCount);
	}
	return count;
}

int main(){
	int i, j;
	formulaGen();
	
	//struct EQUATION *start = M(A(V('x'), N(1)), A(V('x'), N(2)));
	//struct EQUATION *target = A(A(W(V('x'), N(2)), M(N(3), V('x'))), N(2));
	
	struct EQUATION *g3 = A(FX(COSECANT,V('x')),M(FX(COTANGENT,V('x')),N(-1)));
	struct EQUATION *start = W(g3, N(2));
	struct EQUATION *g1 = A(N(1),M(FX(COSINE,(V('x'))),N(-1)));
	struct EQUATION *g4 = FX(COSINE, V('x'));
	struct EQUATION *g2 = M(A(N(1),g4),A(N(1),M(g4,N(-1))));
	struct EQUATION *target = M(A(N(1),M(g4,N(-1))),W(A(N(1),g4),N(-1)));
	
	target = fixCommutative(target);
	printf("Start: ");
	printEq(start);
	printf("Target: ");
	printEq(target);
	printf("\n");
	clock_t t;
    t = clock();
	struct MATH_STEP *output = search(start, target);
	t = clock() - t;
	double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
	printf("The search took %f seconds\n\n", time_taken);
	while (output){
		printEq(output->DATA);
		output = output->PARENT;
	}
	printf("\n");
	return 0;
}
