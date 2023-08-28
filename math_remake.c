#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define FUNCTION_COUNT 13

#define TYPE_CONSTANT -6
#define TYPE_VARIABLE -5
#define ADD -4
#define MULTIPLY -3
#define POWER -2
#define DIVISION -1
#define SINE 0
#define COSINE 1
#define TANGENT 2
#define COSECANT 3
#define SECANT 4
#define COTANGENT 5
#define SQUARE_ROOT 6
#define LAWN 7
#define EXPONENTIAL 8

#define MAX_BRACKET_LENGTH 50
#define ARBITRARY_MAX 300
#define MAX_TT_SIZE 1000

#define IS_CHAR_SMALL_LETTER(x) (((x)>='a')&&((x)<='z'))
#define IS_CHAR_CAP_LETTER(x) (((x)>='A')&&((x)<='Z'))
#define IS_CHAR_NUMBER(x) (((x)>='0')&&((x)<='9'))

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

char *FUNCTION_FULL_NAME_LIST[FUNCTION_COUNT] = {"add", "mul", "pow", "div", "sin", "cos", "tan", "csc", "sec", "cot", "sqt", "lwn", "exp"};

struct EQUATION {
	int FUNCTION_TYPE; //addition, multiplication, sine, cosine or what
	int NUMBER_OF_CHILDREN; //number of elements in a bracket
	struct EQUATION *CHILDREN[MAX_BRACKET_LENGTH]; //elements in a bracket
	int CONSTANT_VALUE; //value of the integer if its a number in the equation
	char VARIABLE_NAME; //name of the variable if its a variable in the equation
};

// for printing the tree form of equation
void convert_equation_to_string(struct EQUATION *equation, char output_string[]){
	if (equation->FUNCTION_TYPE == TYPE_VARIABLE){
		char string_from_character[2] = {'\0', '\0'};
		string_from_character[0] = equation->VARIABLE_NAME;
		strcat(output_string, string_from_character);
	} else if (equation->FUNCTION_TYPE == TYPE_CONSTANT){
		char string_from_integer_variable[ARBITRARY_MAX];
		sprintf(string_from_integer_variable, "%d", equation->CONSTANT_VALUE);
		strcat(output_string, string_from_integer_variable);
	} else {
		int is_operation_basic = 0;
		if (equation->FUNCTION_TYPE < 0){ // checking about if its a basic operation which has operator like '+'
			is_operation_basic = 1;
		}
		if (is_operation_basic == 0) strcat(output_string, FUNCTION_FULL_NAME_LIST[equation->FUNCTION_TYPE+4]); //if not print the function name
		strcat(output_string, "(");
		int i;
		for (i=0; i<(equation->NUMBER_OF_CHILDREN); ++i){ //print recursively
			convert_equation_to_string(equation->CHILDREN[i], output_string);
			if (i!=(equation->NUMBER_OF_CHILDREN)-1){
				if (is_operation_basic == 0) strcat(output_string, ",");
				else {
					if (equation->FUNCTION_TYPE == ADD) strcat(output_string, "+");
					else if (equation->FUNCTION_TYPE == MULTIPLY) strcat(output_string, "*");
					else if (equation->FUNCTION_TYPE == DIVISION) strcat(output_string, "/");
					else if (equation->FUNCTION_TYPE == POWER) strcat(output_string, "^");
				}
			}
		}
		strcat(output_string, ")");
	}
}

// append a given character inside a equation of string form
void insert_char_at_index(char input_string[], char character, int index) {
	int i;
	int length = strlen(input_string);
	memmove(input_string + index + 1, input_string + index, length - index + 1);
	input_string[index] = character;
}

// remove a character from a equation of string form
void remove_char_at_index(char input_string[], int index) {
	int i;
	int length = strlen(input_string);
	memmove(input_string + index, input_string + index + 1, length - index);
	input_string[length-1] = '\0';
}

// enforcing to add optional multiplication signs like between variables and brackets
void math_parser_explicit_multiply_sign(char given_string[]){
	int i;
	for (i=0; given_string[i+1]; ++i){
		if ((given_string[i] == ')' || IS_CHAR_SMALL_LETTER(given_string[i]) || IS_CHAR_NUMBER(given_string[i])) &&
			(given_string[i+1] == '(' || IS_CHAR_SMALL_LETTER(given_string[i+1]) || IS_CHAR_CAP_LETTER(given_string[i+1]))){
			insert_char_at_index(given_string, '*', i+1);
			++i;	
		}
	}
}

// search for a nearest addition/multiplication/division/power symbol from a given index
// ignore all sub brackets while searching, including if the bracket start from the given index itself
int search_for_basic_operation_symbol(char given_string[], int index){
	int values[4];
	values[0] = strchr(given_string+index, '+') == NULL ? 999 : strchr(given_string+index, '+')-given_string; // find the nearest addition
	values[1] = strchr(given_string+index, '*') == NULL ? 999 : strchr(given_string+index, '*')-given_string; // multiplication
	values[2] = strchr(given_string+index, '^') == NULL ? 999 : strchr(given_string+index, '^')-given_string; // so on
	values[3] = strchr(given_string+index, '/') == NULL ? 999 : strchr(given_string+index, '/')-given_string;
	int min_index = values[0];
	int i;
	for (i=1; i<4; ++i)
		if (values[i] < min_index) // determine which is the nearest one
			min_index = values[i];
	if (strchr(given_string+index, '(') == NULL){
		return min_index;
	}
	int bracket_index = strchr(given_string+index, '(')-given_string;
	if (bracket_index < min_index && min_index < find_corresponding_bracket(given_string, bracket_index))
		return search_for_basic_operation_symbol(given_string, find_corresponding_bracket(given_string, bracket_index)+1); // if the operation is inside a bracket, redo the search
	return min_index; // return the index of the operation symbol in the equation string
}

// change the equation string to have explicit brackets which are optional because of bodmas
void math_parser_explicit_bracket_bodmas(char given_string[], int start, int *end, char operation){
	int index=start;
	int list_operators[100];
	int list_operators_index = 0;
	while (1){ // make a list of operators location in the current equation without considering sub brackets
		if (given_string[index] == '+' || given_string[index] == '*' || given_string[index] == '/' || given_string[index] == '^')
			index = search_for_basic_operation_symbol(given_string, index+1);
		else
			index = search_for_basic_operation_symbol(given_string, index);
		if (index >= *end) break;
		else {
			list_operators[list_operators_index++] = index;
		}
	}
	int i;
	if (list_operators_index > 1){ //more than one term
		if (operation == '^'){ // power
			for(i=list_operators_index-1; i>=0; --i){ // bracket them from right to left
				if (given_string[list_operators[i]] == '^'){
					if (i==list_operators_index-1)
						insert_char_at_index(given_string, ')', *end); // the end term has to be identified specially because it has no operation symbol
					else
						insert_char_at_index(given_string, ')', list_operators[i+1]);
					if (i==0)
						insert_char_at_index(given_string, '(', start);
					else
						insert_char_at_index(given_string, '(', list_operators[i-1]+1);
					*end += 2;
					return;
				}
			}
			return;
		}
		if (operation == '/'){ //division
			for (i=0; i<list_operators_index; ++i){ // left to right bracketting
				if (given_string[list_operators[i]] == '/'){
					if (i == list_operators_index-1)
						insert_char_at_index(given_string, ')', *end);
					else
						insert_char_at_index(given_string, ')', list_operators[i+1]);
					if (i==0)
						insert_char_at_index(given_string, '(', start);
					else
						insert_char_at_index(given_string, '(', list_operators[i-1]+1);
					*end += 2;
					return;
				}
			}
			return;
		}		
		if (operation == '*') {
			int i;
			int plus_occur = 0;
			for (i=0; i<list_operators_index; ++i){
				if (given_string[list_operators[i]] == '+'){ plus_occur = 1; break; } //bracket for bodmas is only needed when addition and multiply are together
			}
			if (plus_occur){
				for (i=0; i<list_operators_index; ++i){
					
					if (given_string[list_operators[i]] == '*'){
						int orig = i;
						for (;i<list_operators_index && given_string[list_operators[i]] == '*'; ++i); // consider the collection of many multiplication in a single big term
						
						if (i == list_operators_index)
							insert_char_at_index(given_string, ')', *end);
						else
							insert_char_at_index(given_string, ')', list_operators[i]);
						if (orig == 0)
							insert_char_at_index(given_string, '(', start);
						else
							insert_char_at_index(given_string, '(', list_operators[orig-1]+1);
						*end += 2;
						break;
					}
				}
			}
		}		
	}
}

// the above function needs to run many times, to put all possible brackets
// this function loops it
void math_parser_explicit_bracket_bodmas_apply(char given_string[], int start, int *end){
	char prev_string[ARBITRARY_MAX]={0};
	char *bodmas_order = "^/*"; // descending order of pirority 
	int i;
	memcpy(prev_string, given_string+start, (*end)-start+1);
	for (i=0; bodmas_order[i]; ++i){
		math_parser_explicit_bracket_bodmas(given_string, start, end, bodmas_order[i]);
		while (memcmp(prev_string, given_string+start, (*end)-start+1)){ // see if equation stopped changing due to completion of bracketting everythings
			memcpy(prev_string, given_string+start, (*end)-start+1);
			math_parser_explicit_bracket_bodmas(given_string, start, end, bodmas_order[i]);
		}
	}
}

// remove extra brackets
void math_parser_remove_extra_bracket(char given_string[]){
	int i;
	for (i=0; given_string[i]; ++i){
		if (given_string[i] == '(' && (i?!IS_CHAR_CAP_LETTER(given_string[i-1]):1)){ // between two brackets if there is no operation symbol, its excess
			if (find_corresponding_bracket(given_string, i) < search_for_basic_operation_symbol(given_string, i+1)){
				remove_char_at_index(given_string, find_corresponding_bracket(given_string, i));
				remove_char_at_index(given_string, i);
				--i;
			}
		}
	}
	for (i=0; given_string[i]; ++i){
		if (given_string[i] == '(' && given_string[i+1] == '(' && // two consecutive brackets are excess
			find_corresponding_bracket(given_string, i)-find_corresponding_bracket(given_string, i+1)==1){
			remove_char_at_index(given_string, find_corresponding_bracket(given_string, i));
			remove_char_at_index(given_string, i);
			--i;
		}
	}
}

// there is no subtraction only addition with negative integers
void math_parser_no_subtraction(char given_string[]){
	int i;
	for (i=0; given_string[i+1]; ++i){ // finding a lone subtraction sign
		if (given_string[i] != '(' && given_string[i] != '*' && given_string[i] != '^' && given_string[i] != '+' && given_string[i+1] == '-'){
			insert_char_at_index(given_string, '+', i+1);
			++i;
		}
	}
	for (i=0; i<given_string[i+1]; ++i){ // multiply the term with negative one
		if (given_string[i] == '-' && (given_string[i+1] == '(' || IS_CHAR_CAP_LETTER(given_string[i+1]) || IS_CHAR_SMALL_LETTER(given_string[i+1]))){
			insert_char_at_index(given_string, '*', i+1);
			insert_char_at_index(given_string, '1', i+1);
			i += 2;
		}
	}
}

// find the corresponding bracket given the index of a starting bracket in equation string
int find_corresponding_bracket(char given_string[], int index){
	int count = 1;
	++index;
	while (1){
		char *a = strchr(given_string+index, '(');
		char *b = strchr(given_string+index, ')');
		if (a!=NULL && b!=NULL) index = MIN(a-given_string, b-given_string);
		else if (a!=NULL) index = a-given_string;
		else if (b!=NULL) index = b-given_string;
		if (given_string[index] == '('){ ++count; }
		else if (given_string[index] == ')'){ --count; }
		if (count == 0) return index;
		++index;
	};
}

// if the equation has an extra starting and corresponding ending bracket remove it
void math_parser_remove_first_bracket(char given_string[]){
	if (given_string[0] == '(' && find_corresponding_bracket(given_string, 0)==strlen(given_string)-1){
		remove_char_at_index(given_string, 0);
		remove_char_at_index(given_string, strlen(given_string)-1);
	}
}

// if there is a variable or number identify it
void math_parser_string_no_children(struct EQUATION *equation, char given_string[], int start, int end){
	if (IS_CHAR_SMALL_LETTER(given_string[start])){
		equation->FUNCTION_TYPE = TYPE_VARIABLE;
		equation->VARIABLE_NAME = given_string[start];
	} else {
		char string_number[ARBITRARY_MAX];
		memcpy(string_number, given_string+start, end-start);
		string_number[end-start+1] = '\0';
		equation->FUNCTION_TYPE = TYPE_CONSTANT; 
		equation->CONSTANT_VALUE = atoi(string_number); // string number to integers
	}
	equation->NUMBER_OF_CHILDREN = 0;
}

// do all the bodmas explicit bracketting in all parts of the equation tree
void math_parser_recursive_bodmas(char given_string[], int start, int first_time){
	int end = first_time?strlen(given_string):find_corresponding_bracket(given_string, start-1); // the case is a bit different if its for the root equation
	math_parser_explicit_bracket_bodmas_apply(given_string, start, &end);
	int i;
	for (i=start; i<(first_time?strlen(given_string):find_corresponding_bracket(given_string, start-1)); ++i){
		if (given_string[i] == '('){ //check bodmas inside sub brackets too
			math_parser_recursive_bodmas(given_string, i+1, 0);
			i = find_corresponding_bracket(given_string, i);
		}
	}
}

// represent function name as a single capital letter for easire parsing
void math_parser_captial_letter(char given_string[]){
	int i;
	for (i=0; i<FUNCTION_COUNT; ++i){
		char *tmp;
		while ((tmp=strstr(given_string, FUNCTION_FULL_NAME_LIST[i])) != NULL){
			remove_char_at_index(given_string, tmp-given_string); 
			remove_char_at_index(given_string, tmp-given_string);
			remove_char_at_index(given_string, tmp-given_string);
			insert_char_at_index(given_string, i+'A'-4, tmp-given_string); // sin is A cos is B and so on
		}
	}
}

// the main function to convert equation string into tree, taking help of the other functions we created
void math_parser_string_to_equation(struct EQUATION *equation, char given_string[], int start, int end){
	int i;
	for (i=start; i<end; ++i){ // for a specific bracket
		if (!IS_CHAR_SMALL_LETTER(given_string[i]) && !IS_CHAR_NUMBER(given_string[i]) && given_string[i] != '-') break; 
		if (i == end-1){ // if number or variable
			if (IS_CHAR_SMALL_LETTER(given_string[start])){
				equation->FUNCTION_TYPE = TYPE_VARIABLE;
				equation->VARIABLE_NAME = given_string[start];
			} else {
				char string_number[ARBITRARY_MAX];
				memcpy(string_number, given_string+start, end-start); // copy the number string to somewhere else to finally process it
				string_number[end-start+1] = '\0';
				equation->FUNCTION_TYPE = TYPE_CONSTANT;
				equation->CONSTANT_VALUE = atoi(string_number); // readnumber
			}
			equation->NUMBER_OF_CHILDREN = 0;
			return; // no children of variables and numbers
		}
	}
	equation->NUMBER_OF_CHILDREN=0;
	int prev_index = start-1;
	int function_handle = 0;
	if (search_for_basic_operation_symbol(given_string, start) >= end) function_handle = 1; // if there are no symbol that mean it is a function
	else { // trying to identify which operation this bracket about
		char operation_extract = given_string[search_for_basic_operation_symbol(given_string, start)];
		if (operation_extract == '+') equation->FUNCTION_TYPE = ADD;
		else if (operation_extract == '*') equation->FUNCTION_TYPE = MULTIPLY;
		else if (operation_extract == '^') equation->FUNCTION_TYPE = POWER;
		else if (operation_extract == '/') equation->FUNCTION_TYPE = DIVISION;
		else function_handle = 1;
	}
	if (function_handle && IS_CHAR_CAP_LETTER(given_string[start])){ // if its a function
		struct EQUATION *child = malloc(sizeof(struct EQUATION));
		equation->NUMBER_OF_CHILDREN = 1;
		equation->FUNCTION_TYPE = given_string[start]-'A';
		math_parser_string_to_equation(child, given_string, start+2, end-1); // check the only child of teh single variable function
		equation->CHILDREN[0] = child;
		return;
	}
	int curr_index;
	while (1){
		curr_index = search_for_basic_operation_symbol(given_string, prev_index+1); // seperate terms from each other in string by considering operators
		struct EQUATION *child = malloc(sizeof(struct EQUATION));
		if (curr_index >= end){ // bracket has ended
			if (given_string[prev_index+1]=='(') // find brackets so that equation tree can be made recursively from string
				math_parser_string_to_equation(child, given_string, prev_index+2, end-1);
			else
				math_parser_string_to_equation(child, given_string, prev_index+1, end);
			equation->CHILDREN[(equation->NUMBER_OF_CHILDREN)++] = child;
			return;
		} else {
			if (given_string[prev_index+1]=='(')
				math_parser_string_to_equation(child, given_string, prev_index+2, curr_index-1);
			else
				math_parser_string_to_equation(child, given_string, prev_index+1, curr_index);
		}
		equation->CHILDREN[(equation->NUMBER_OF_CHILDREN)++] = child;
		prev_index = curr_index; // store the location of the current and bracket operator to find the term which is between them
	}
}

void free_equation_memory(struct EQUATION *equation){
	int i;
	for (i=0; i<equation->NUMBER_OF_CHILDREN; ++i)
		free_equation_memory(equation->CHILDREN[i]);
	free(equation);
}


void duplicate_equation(struct EQUATION *equation_to, struct EQUATION *equation_from){
	int i;
	equation_to->NUMBER_OF_CHILDREN = equation_from->NUMBER_OF_CHILDREN;
	equation_to->FUNCTION_TYPE = equation_from->FUNCTION_TYPE;
	if (equation_to->FUNCTION_TYPE == TYPE_VARIABLE) equation_to->VARIABLE_NAME = equation_from->VARIABLE_NAME;
	else if (equation_to->FUNCTION_TYPE == TYPE_CONSTANT) equation_to->CONSTANT_VALUE= equation_from->CONSTANT_VALUE;
	for (i=0; i<equation_to->NUMBER_OF_CHILDREN; ++i){
		struct EQUATION *child = malloc(sizeof(struct EQUATION));
		duplicate_equation(child, equation_from->CHILDREN[i]);
		equation_to->CHILDREN[i] = child;
	}
}

// compare equation if they are exactly the same, considering order, even if its not necessary in case of commutatives one.
int compare_equation(struct EQUATION *equation_1, struct EQUATION *equation_2){
	if (equation_1->FUNCTION_TYPE != equation_2->FUNCTION_TYPE) return 0;
	if (equation_1->FUNCTION_TYPE <= TYPE_VARIABLE) equation_1->NUMBER_OF_CHILDREN = equation_2->NUMBER_OF_CHILDREN = 0;
	if (equation_1->FUNCTION_TYPE >= SINE) equation_1->NUMBER_OF_CHILDREN = equation_2->NUMBER_OF_CHILDREN = 1;
	if (equation_1->NUMBER_OF_CHILDREN != equation_2->NUMBER_OF_CHILDREN) return 0; // same equation should have same number of children in all brackets
	if (equation_1->FUNCTION_TYPE == TYPE_VARIABLE && equation_1->VARIABLE_NAME != equation_2->VARIABLE_NAME) return 0;
	if (equation_1->FUNCTION_TYPE == TYPE_CONSTANT && equation_1->CONSTANT_VALUE != equation_2->CONSTANT_VALUE) return 0;
	int i;
	for (i=0; i<equation_1->NUMBER_OF_CHILDREN; ++i){
		if (!compare_equation(equation_1->CHILDREN[i], equation_2->CHILDREN[i])) return 0; //  recursively compare
	}
	return 1; // not a single thing should be different
}

// apply a formula structure to an equation and find out what are the content of the variables in the formula for that equation
int apply_formula(struct EQUATION *equation, struct EQUATION *formula, struct EQUATION *variable_list[26]){
	int i;
	if (formula->FUNCTION_TYPE == TYPE_VARIABLE){ // if its a variable 
		if (variable_list[formula->VARIABLE_NAME - 'a'] == 0){ // copy if its a variable name we encountered the first time
			struct EQUATION *tmp = malloc(sizeof(struct EQUATION));
			duplicate_equation(tmp, equation);
			variable_list[formula->VARIABLE_NAME - 'a'] = tmp;
		}
		else if (compare_equation(variable_list[formula->VARIABLE_NAME - 'a'], equation) == 0) return 0; // if its not the first time compare with the already stored data, because same vairable should have same content
		return 1;
	}
	if (equation->FUNCTION_TYPE != formula->FUNCTION_TYPE) return 0;
	if (equation->NUMBER_OF_CHILDREN != formula->NUMBER_OF_CHILDREN) return 0;
	if (formula->FUNCTION_TYPE == TYPE_CONSTANT && formula->CONSTANT_VALUE != equation->CONSTANT_VALUE) return 0;
	for (i=0; i<equation->NUMBER_OF_CHILDREN; ++i){ // check if the equation matches the complete formulas heirarchical structure
		if (!apply_formula(equation->CHILDREN[i], formula->CHILDREN[i], variable_list)) return 0;
	}
	return 1;
}

//put the variable content gained by the input formula to the output formula
void replace_variables(struct EQUATION *equation, struct EQUATION *variable_list[26]){ 
	int i;
	for (i=0; i<equation->NUMBER_OF_CHILDREN; ++i){
		struct EQUATION *child = malloc(sizeof(struct EQUATION));
		if (equation->CHILDREN[i]->FUNCTION_TYPE == TYPE_VARIABLE){ // if variable, recall the content
			duplicate_equation(child, variable_list[equation->CHILDREN[i]->VARIABLE_NAME-'a']);
			equation->CHILDREN[i] = child;
			continue;
		}
		replace_variables(equation->CHILDREN[i], variable_list);
	}
}

// nothing much important with this one
void print_equation(struct EQUATION *equation){
	char output_string[ARBITRARY_MAX];
	memset(output_string, 0, ARBITRARY_MAX);
	convert_equation_to_string(equation, output_string);
	printf("%s\n", output_string);
}

// apply formulas to the equation at the root of the equation-bracket hierarchy
int generate_equation_summit(struct EQUATION *output_equation, struct EQUATION *equation, struct EQUATION *formula_input, struct EQUATION *formula_output){
	int i;
	if (equation->FUNCTION_TYPE == TYPE_CONSTANT || equation->FUNCTION_TYPE == TYPE_VARIABLE) return 0;
	struct EQUATION *variable_list[26];
	memset(variable_list, 0, sizeof(struct EQUATION *)*26);
	if (equation->NUMBER_OF_CHILDREN == formula_input->NUMBER_OF_CHILDREN){ // if same number of children with equation and formula
		if (apply_formula(equation, formula_input, variable_list)){ // check if formula structure matches 
			struct EQUATION *new_instance = malloc(sizeof(struct EQUATION));
			if (formula_output->FUNCTION_TYPE == TYPE_VARIABLE){
				duplicate_equation(new_instance, variable_list[formula_output->VARIABLE_NAME-'a']);
			} else {
				duplicate_equation(new_instance, formula_output);
				replace_variables(new_instance, variable_list);
			}
			duplicate_equation(output_equation, new_instance);
			free_equation_memory(new_instance);
			return 1;
		}
	}
	else if (equation->NUMBER_OF_CHILDREN > formula_input->NUMBER_OF_CHILDREN && formula_input->NUMBER_OF_CHILDREN >= 2){ // if formula is smaller and having lesser number of terms than what the equation has in its bracket even tho the formula can still be applied
		struct EQUATION *new_instance = malloc(sizeof(struct EQUATION));
		duplicate_equation(new_instance, equation);
		new_instance->NUMBER_OF_CHILDREN = formula_input->NUMBER_OF_CHILDREN; // consider only the equation terms starting from the start position ending after numbers of terms the formula had
		if (apply_formula(new_instance, formula_input, variable_list)){
			for (i=0; i<formula_input->NUMBER_OF_CHILDREN; ++i) free_equation_memory(new_instance->CHILDREN[i]);
			memmove(new_instance->CHILDREN, (new_instance->CHILDREN)+formula_input->NUMBER_OF_CHILDREN,
					sizeof(struct EQUATION *)*(equation->NUMBER_OF_CHILDREN-formula_input->NUMBER_OF_CHILDREN));
			new_instance->NUMBER_OF_CHILDREN = equation->NUMBER_OF_CHILDREN-formula_input->NUMBER_OF_CHILDREN;
			struct EQUATION *sub_node = malloc(sizeof(struct EQUATION)); // pack the extra terms which the formula didn't used to a separate bracket
			if (formula_output->FUNCTION_TYPE == TYPE_VARIABLE){
				duplicate_equation(sub_node, variable_list[formula_output->VARIABLE_NAME-'a']);
			} else {
				duplicate_equation(sub_node, formula_output);
				replace_variables(sub_node, variable_list);
			}
			new_instance->CHILDREN[(new_instance->NUMBER_OF_CHILDREN)++] = sub_node;
			duplicate_equation(output_equation, new_instance);
			free_equation_memory(new_instance);
			return 1;
		}
	}
	for (i=0; i<26; ++i) if (variable_list[i]) free_equation_memory(variable_list[i]);
	return 0;
}

// calculate the solutions of numerical numbers in the equation
int compute_constant(struct EQUATION *output_equation, struct EQUATION *equation){
	if (equation->FUNCTION_TYPE == TYPE_CONSTANT || equation->FUNCTION_TYPE == TYPE_VARIABLE) return 0; // we are looking for a step higher in the hierarchy because a single number can't be operated
	if ((equation->NUMBER_OF_CHILDREN) < 2 || equation->CHILDREN[0]->FUNCTION_TYPE != TYPE_CONSTANT || equation->CHILDREN[1]->FUNCTION_TYPE != TYPE_CONSTANT) return 0;
	int term_1 = equation->CHILDREN[0]->CONSTANT_VALUE;
	int term_2 = equation->CHILDREN[1]->CONSTANT_VALUE;
	int result;
	
	if (equation->FUNCTION_TYPE == ADD) result = term_1 + term_2;
	else if (equation->FUNCTION_TYPE == MULTIPLY) result = term_1 * term_2;
	else if (equation->FUNCTION_TYPE == POWER) result = (int)pow((int)term_1, (int)term_2);
	else return 0; 
	struct EQUATION *final_equation = malloc(sizeof(struct EQUATION));
	duplicate_equation(final_equation, equation);
	
	if (equation->NUMBER_OF_CHILDREN == 2){ // if there are two terms after the operating the arithmetic bracket will get destroyed
		final_equation->FUNCTION_TYPE = TYPE_CONSTANT; 
		final_equation->CONSTANT_VALUE = result;
	} else if (equation->NUMBER_OF_CHILDREN > 2){ // if there are more than two terms, then do the arithmetic only with the first two
		free_equation_memory(final_equation->CHILDREN[0]); free_equation_memory(final_equation->CHILDREN[1]);
		memmove(final_equation->CHILDREN, (final_equation->CHILDREN)+2, sizeof(struct EQUATION *)*(equation->NUMBER_OF_CHILDREN-2));
		final_equation->NUMBER_OF_CHILDREN -= 2;
		final_equation->CHILDREN[final_equation->NUMBER_OF_CHILDREN] = malloc(sizeof(struct EQUATION));
		final_equation->CHILDREN[final_equation->NUMBER_OF_CHILDREN]->FUNCTION_TYPE = TYPE_CONSTANT;
		final_equation->CHILDREN[final_equation->NUMBER_OF_CHILDREN]->CONSTANT_VALUE = result;
		(final_equation->NUMBER_OF_CHILDREN)++;
	}
	duplicate_equation(output_equation, final_equation);
	free_equation_memory(final_equation);
	return 1;
}

int equation_iterate(struct EQUATION *equation, int *index, struct EQUATION *formula_input, struct EQUATION *formula_output){
	int i;
	if (equation->FUNCTION_TYPE == TYPE_CONSTANT || equation->FUNCTION_TYPE == TYPE_VARIABLE) return 0;
	for (i=0; i<equation->NUMBER_OF_CHILDREN; ++i){
		(*index)--;
		if (*index == 0){
			struct EQUATION *curr = malloc(sizeof(struct EQUATION));
			if (formula_input == 0){
				if (compute_constant(curr, equation->CHILDREN[i])) equation->CHILDREN[i] = curr;
			} else {
				if (generate_equation_summit(curr, equation->CHILDREN[i], formula_input, formula_output) == 1) equation->CHILDREN[i]= curr;
			}
			return 1;
		}
		if (equation_iterate(equation->CHILDREN[i], index, formula_input, formula_output)) return 1;
	}
	return 0;
}

// swap terms in equation because addition and multiplication follows commutativity
// this swapping should be done in a linear way even tho the tree is the recursive
int equation_iterate_swap(struct EQUATION *equation, int *index){
	int i;
	if (equation->FUNCTION_TYPE == TYPE_CONSTANT || equation->FUNCTION_TYPE == TYPE_VARIABLE || equation->FUNCTION_TYPE == POWER) return 0; //power is not commutative
	for (i=1; i<equation->NUMBER_OF_CHILDREN; ++i){
		(*index)--; // iterating recursive
		if (*index == 0){
			struct EQUATION *tmp;
			tmp = equation->CHILDREN[0];
			equation->CHILDREN[0] = equation->CHILDREN[i];
			equation->CHILDREN[i] = tmp;
			return 1;
		}
	}
	for (i=0; i<equation->NUMBER_OF_CHILDREN; ++i){
		if (equation_iterate_swap(equation->CHILDREN[i], index)) return 1; // check for sub brackets
	}
	return 0;
}

// changing the equation string step by step considering cases to finally convert into tree format
void string_to_equation_short_helper(struct EQUATION *equation, char *equation_string){
	char given_string[ARBITRARY_MAX];
	memset(given_string, 0, ARBITRARY_MAX);
	strcat(given_string, equation_string);
	math_parser_captial_letter(given_string); // single capital letter is a function
	math_parser_explicit_multiply_sign(given_string); // some multiplication signs are optional like in multiplication of variables
	math_parser_no_subtraction(given_string); // no subtraction only addition of negative integers
	math_parser_remove_extra_bracket(given_string); // remove extra brackets which doesn't matter
	math_parser_remove_first_bracket(given_string);	 // remove the bracket for the root equation if present otherwise the parsing code can have issues
	math_parser_recursive_bodmas(given_string, 0, 1); //explicit brackets even if the bodmas exist
	math_parser_string_to_equation(equation, given_string, 0, strlen(given_string)); // convert string to tree 
}

// apply the given formula whenever possible
// we don't want division but to the power of -1 or the multiplication by one
void pre_apply_formula(struct EQUATION *equation, char *equation_input, char *equation_output){
	struct EQUATION formula_input;
	struct EQUATION formula_output;
	string_to_equation_short_helper(&formula_input, equation_input);
	string_to_equation_short_helper(&formula_output, equation_output);
	generate_equation_summit(equation, equation, &formula_input, &formula_output);
	int i;
	for (i=1;; ++i){ // check all location of the tree
		int index = i;
		if (equation_iterate(equation, &index, &formula_input, &formula_output) == 0) break;
	}
}

// oeprations can't have brackets for the same operation again. merge them.
void merge_nested_bracket(struct EQUATION *equation){
	int i;
	if (equation->FUNCTION_TYPE == ADD || equation->FUNCTION_TYPE == MULTIPLY){ // merge for addition and multiply
		for (i=0; i<equation->NUMBER_OF_CHILDREN; ++i){
			if (equation->FUNCTION_TYPE == equation->CHILDREN[i]->FUNCTION_TYPE){ // if parent and child operation is same
				struct EQUATION *orig_mem = equation->CHILDREN[i];
				memcpy((equation->CHILDREN)+equation->NUMBER_OF_CHILDREN, equation->CHILDREN[i]->CHILDREN, sizeof(struct EQUATION *)*equation->CHILDREN[i]->NUMBER_OF_CHILDREN);
				equation->NUMBER_OF_CHILDREN += equation->CHILDREN[i]->NUMBER_OF_CHILDREN;
				memmove((equation->CHILDREN)+i, (equation->CHILDREN)+i+1, sizeof(struct EQUATION *)*(equation->NUMBER_OF_CHILDREN-i-1));
				equation->NUMBER_OF_CHILDREN -= 1; // the child bracket has been killed to merge with the parent
				--i;
				free(orig_mem);
			}
		}
	}
	if (equation->FUNCTION_TYPE == TYPE_VARIABLE || equation->FUNCTION_TYPE == TYPE_CONSTANT) return;
	for (i=0; i<equation->NUMBER_OF_CHILDREN; ++i){
		merge_nested_bracket(equation->CHILDREN[i]);
	}
}

// do it many times so that no merge bracket remains
void merge_loop(struct EQUATION *equation){
	struct EQUATION *orig_equation = 0;
	do {
		if (orig_equation) free_equation_memory(orig_equation);
		orig_equation = malloc(sizeof(struct EQUATION));
		duplicate_equation(orig_equation, equation);
		merge_nested_bracket(equation);
	} while (compare_equation(orig_equation, equation)==0); // if equation is not changing anymore that means merging is done now
	free_equation_memory(orig_equation);
}

// the main function which convert equation in string format to tree format
void string_to_equation_short(struct EQUATION *equation, char *given_string){
	string_to_equation_short_helper(equation, given_string);
	pre_apply_formula(equation, "a/b", "ab^-1");
	pre_apply_formula(equation, "a*1", "a");
	pre_apply_formula(equation, "1*a", "a");
	merge_loop(equation);
}

// all possible transformations of the equation
void generate_equation(struct EQUATION *equation, char *formula_input_list[], char *formula_output_list[], int formula_count, struct EQUATION *equation_list[], int *count){
	int i;
	for (i=0; i<formula_count; ++i){ // for all formulas
		struct EQUATION curr_formula_input;
		struct EQUATION curr_formula_output;
		string_to_equation_short(&curr_formula_input, formula_input_list[i]);
		string_to_equation_short(&curr_formula_output, formula_output_list[i]);
		struct EQUATION *curr = malloc(sizeof(struct EQUATION));
		duplicate_equation(curr, equation);
		generate_equation_summit(curr, equation, &curr_formula_input, &curr_formula_output); //if formula applied to root
		if (compare_equation(curr, equation) == 0){
			equation_list[*count] = malloc(sizeof(struct EQUATION));
			duplicate_equation(equation_list[*count], curr);
			++(*count);
		}
		free_equation_memory(curr);
		int j;
		for (j=1;;++j){ // or children
			int index = j;
			struct EQUATION *curr = malloc(sizeof(struct EQUATION));
			duplicate_equation(curr, equation);
			if (equation_iterate(curr, &index, &curr_formula_input, &curr_formula_output) == 0) break;
			if (compare_equation(curr, equation) == 0){
				equation_list[*count] = malloc(sizeof(struct EQUATION));
				duplicate_equation(equation_list[*count], curr);
				++(*count);
			}
			free_equation_memory(curr);
		}
	}
	struct EQUATION *curr = malloc(sizeof(struct EQUATION));
	duplicate_equation(curr, equation);
	compute_constant(curr, equation); // generate arithemetic caluclation steps
	if (compare_equation(curr, equation) == 0){
		equation_list[*count] = malloc(sizeof(struct EQUATION));
		duplicate_equation(equation_list[*count], curr);
		++(*count);
	}
	free_equation_memory(curr);
	int j;
	for (j=1;;++j){
		int index = j;
		struct EQUATION *curr = malloc(sizeof(struct EQUATION));
		duplicate_equation(curr, equation);
		if (equation_iterate(curr, &index, 0, 0) == 0) break;
		if (compare_equation(curr, equation) == 0){
			equation_list[*count] = malloc(sizeof(struct EQUATION));
			duplicate_equation(equation_list[*count], curr);
			++(*count);
		}
		free_equation_memory(curr);
	}
	for (j=1;;++j){
		int index = j;
		struct EQUATION *curr = malloc(sizeof(struct EQUATION));
		duplicate_equation(curr, equation);
		if (equation_iterate_swap(curr, &index) == 0) break; // there are steps for using the commutative property too
		if (compare_equation(curr, equation) == 0){
			equation_list[*count] = malloc(sizeof(struct EQUATION));
			duplicate_equation(equation_list[*count], curr);
			++(*count);
		}
		free_equation_memory(curr);
	}
}

struct MATH_STEP {
	struct MATH_STEP *PARENT;
	struct EQUATION *DATA;
};

struct QUEUE_OBJ {
	struct MATH_STEP *STEP;
	struct QUEUE_OBJ *NEXT_STEP;
};

int find_in_tt(struct EQUATION *eq, int *tt_count, struct EQUATION *transposition_table[MAX_TT_SIZE]){
	int i;
	for (i=0; i<(*tt_count); ++i){
		if (compare_equation(eq, transposition_table[i])) return 1;
	}	
	return 0;
}
void enqueue(struct QUEUE_OBJ **queue_start, struct QUEUE_OBJ **queue_end, struct MATH_STEP *added){
	if (*queue_end == 0){
		*queue_end = malloc(sizeof(struct QUEUE_OBJ));
		*queue_start = *queue_end;
	} else {
		(*queue_end)->NEXT_STEP = malloc(sizeof(struct QUEUE_OBJ));
		*queue_end = (*queue_end)->NEXT_STEP;		
	}
	(*queue_end)->STEP = malloc(sizeof(struct MATH_STEP));
	memcpy((*queue_end)->STEP, added, sizeof(struct MATH_STEP));
	(*queue_end)->NEXT_STEP = 0;
}

void dequeue(struct MATH_STEP *dequeue_element, struct QUEUE_OBJ **queue_start, struct QUEUE_OBJ **queue_end){
	memcpy(dequeue_element, (*queue_start)->STEP, sizeof(struct MATH_STEP));
	*queue_start = (*queue_start)->NEXT_STEP;
	if (*queue_start == 0) *queue_end = 0;
}

int formula_function(char *formula_input_list[], char *formula_output_list[]){
	int i;
	for (i=0; i<ARBITRARY_MAX; ++i){
		formula_input_list[i] = malloc(50);
		memset(formula_input_list[i], 0, 50);
		formula_output_list[i] = malloc(50);
		memset(formula_output_list[i], 0, 50);
	}
	char *fl = "a^1=a, a^b*a^c=a^(b+c), (ab)^c=a^c*b^c, a^2=aa, aa^(-1)=1, 1-a^2=(1+a)(1-a), (a/b)=(a^c)/(b^c), sin(a)^2=1-cos(a)^2, ab+ac=a(b+c), aa=a^2, a+cba=a(bc+1), ab+ac=a(b+c), (a+b)(c+d)=ac+ad+bc+bd, a(b+c)=ab+ac, sin(a)^2+cos(a)^2=1, (a-b)^2=a^2-2ab+b^2, csc(a)=1/sin(a), cot(a)=cos(a)/sin(a)";
	char flo[500];
	int index=0;
	for (i=0; fl[i]; ++i){
		if (fl[i]==' ') continue;
		if (fl[i]==',') flo[index++]='=';
		else flo[index++]=fl[i];
	}
	const char s[2] = "=";
	char *token = strtok(flo, s);
	int count=0;
	do {
		strcpy(formula_input_list[count], token);
		token = strtok(NULL, s);
		strcpy(formula_output_list[count], token);
		token = strtok(NULL, s);
		++count;
	} while(token!=NULL);
	return count;
}

void make_list(char *f_input[], char *f_output[], int f_count, struct EQUATION *equation, int *equation_list_count, struct EQUATION *equation_list[]){
	pre_apply_formula(equation, "a/b", "ab^-1");
	pre_apply_formula(equation, "a*1", "a");
	pre_apply_formula(equation, "1*a", "a");
	merge_loop(equation);
	int i;
	generate_equation(equation, f_input, f_output, f_count, equation_list, equation_list_count);
	for (i=0; i<(*equation_list_count); ++i){
		merge_loop(equation_list[i]);
	}
	
}

// main function which does a breadth first search from the given LHS to RHS proving the given thing to prove by displaying steps
void search(char *f_input[], char *f_output[], int f_count, struct MATH_STEP *output, struct EQUATION *eq, struct EQUATION *target){
	struct EQUATION *transposition_table[MAX_TT_SIZE];
	int tt_count = 0;
	int i;
	struct EQUATION *move_list[ARBITRARY_MAX]={0};
	int move_list_count = 0;
	
	struct QUEUE_OBJ *queue_start=0;
	struct QUEUE_OBJ *queue_end=0;
	transposition_table[tt_count] = malloc(sizeof(struct EQUATION));
	duplicate_equation(transposition_table[tt_count], eq);
	tt_count++;
	
	struct MATH_STEP step;
	step.PARENT = 0;
	step.DATA = malloc(sizeof(struct EQUATION));
	
	duplicate_equation(step.DATA, eq);
	
	enqueue(&queue_start, &queue_end, &step);
	
	while (queue_end){
		struct MATH_STEP *v=malloc(sizeof(struct MATH_STEP));
		dequeue(v, &queue_start, &queue_end);
		
		if (compare_equation(v->DATA, target)){
			memcpy(output, v, sizeof(struct MATH_STEP));
			return;
		}
		
		for (i=0; i<move_list_count ; ++i) free_equation_memory(move_list[i]);
		memset(move_list, 0, sizeof(struct EQUATION *)*ARBITRARY_MAX); 
		move_list_count = 0;
		
		make_list(f_input, f_output, f_count, v->DATA, &move_list_count, move_list); // find neighbour in the graph
		
		for (i=0; i<move_list_count; ++i){
			struct EQUATION *w = malloc(sizeof(struct EQUATION));
			duplicate_equation(w, move_list[i]);
			if (find_in_tt(w, &tt_count, transposition_table) == 0){
				transposition_table[tt_count] = malloc(sizeof(struct EQUATION));
				duplicate_equation(transposition_table[tt_count], w);
				tt_count++;
				step.DATA = malloc(sizeof(struct EQUATION));
				duplicate_equation(step.DATA, w);
				step.PARENT = v;
				enqueue(&queue_start, &queue_end, &step);
			}
		}
	}
}

int main (){
	struct EQUATION start;
	string_to_equation_short(&start, "x^1");
	print_equation(&start);
	struct EQUATION target;
	string_to_equation_short(&target, "x");
	print_equation(&target);
	printf("\n");
	struct MATH_STEP output;
	
	char *formula_input_list[ARBITRARY_MAX];
	char *formula_output_list[ARBITRARY_MAX];
	int formula_total_count = formula_function(formula_input_list, formula_output_list);
	
	search(formula_input_list, formula_output_list, formula_total_count, &output, &start, &target);
	struct MATH_STEP *output_pointer = &output;
	while (output_pointer){
		print_equation(output_pointer->DATA);
		output_pointer = output_pointer->PARENT;
	}
	
	int i;
	for (i=0; i<ARBITRARY_MAX; ++i){
		free(formula_input_list[i]);
		free(formula_output_list[i]);
	}
	
	return 0;
}
