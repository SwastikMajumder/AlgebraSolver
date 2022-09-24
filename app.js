const CONSTANT = 0;
const VARIABLE = 1;
const ADD = 2;
const MULTIPLY = 3;
const EXPONENT = 4;
const SINE = 5;
const COSINE = 6;
const COSECANT = 7;
const SECANT = 8;
const TANGENT = 9;
const COTANGENT = 10;

class Equation {
	constructor(operation, variable, constant, child){
		this.operation = operation;
		this.variable = variable;
		this.constant = constant;
		this.child = child;
	}
}
function do_copy(eq) {
    var output = new Equation(eq.operation, eq.variable, eq.constant, []);
	for (var i=0; i<eq.child.length; ++i){
		output.child.push(do_copy(eq.child[i]));
	}
	return output;
}

function add_child(eq, child){
	eq.child.push(child);
	return eq;
}

function remove_child(eq, n){
	eq.child.splice(n, 1);
	return eq;
}

function shift_up(eq, n){
	for (var i=0; i<eq.child[n].child.length; ++i){
		eq = add_child(eq, eq.child[n].child[i]);
	}
	eq = remove_child(eq, n);
	return eq;
}

function fix_tree(eq){
	if (eq.child.length != 0){
		for (var i=0; i<eq.child.length; ++i){
			if (eq.child[i].operation == eq.operation && (eq.operation == MULTIPLY || eq.operation == ADD)){
				eq = shift_up(eq, i);
				i=-1;
			}
		}
	}
	return eq;
}

function fix_tree_recursive(eq){
	eq = fix_tree(eq);
	if (eq.child.length != 0){
		for (var i=0; i<eq.child.length; ++i){
			eq.child[i] = fix_tree_recursive(eq.child[i]);
		}
	}
	return eq;
}

function approx(a, b){
	if (Math.abs(a-b) > 0.001){
		return false;
	} else {
		return true;
	}
}

function is_same(eq1, eq2){
	if (eq1.child.length == eq2.child.length && eq1.operation == eq2.operation &&
		approx(eq1.constant, eq2.constant) && eq1.variable == eq2.variable){
		if (eq1.operation == EXPONENT){
			if (is_same(eq1.child[0], eq2.child[0]) && is_same(eq1.child[1], eq2.child[1]))
				return true;
			else return false;
		} else {
			var already_over=[];
			for (var i=0; i<eq1.child.length; ++i){
				for (var j=0; j<eq1.child.length; ++j){
					if (already_over.includes(j) == false && is_same(eq1.child[i], eq2.child[j])){
						already_over.push(j);
						break;
					}
					if (j==eq1.child.length-1) return false;
				}
			}
		}
	} else {
		return false;
	}
	return true;
}

function same_in_array(eq_list, eq){
	if (eq_list.length == 0) return -1;
	for (var i=0; i<eq_list.length; ++i){
		if (is_same(eq_list[i], eq)) return i;
	}
	return -1;
}

function operate_equation(op, eq1, eq2){
	return new Equation(op, 0, 0, [eq1, eq2]);
}

function operate_trig(op, eq){
	return new Equation(op, 0, 0, [eq]);
}

function make_variable(variable){
	return new Equation(VARIABLE, variable, 0, []);
}

function a(x, y){
	return operate_equation(ADD, x, y);
}

function m(x, y){
	return operate_equation(MULTIPLY, x, y);
}

function w(x, y){
	return operate_equation(EXPONENT, x, y);
}

function n(number){
	return new Equation(CONSTANT, 0, number, []);
}

function t(op, x){
	return operate_trig(op, x);
}

function print_equation(eq){
	if (eq.operation == CONSTANT){
		process.stdout.write(eq.constant.toString());
	}
	else if (eq.operation == VARIABLE){
		process.stdout.write(eq.variable);
	}
	else if (eq.operation == ADD || eq.operation == MULTIPLY || eq.operation == EXPONENT){
		process.stdout.write("(");
		for (var i=0; i<eq.child.length-1; ++i){
			print_equation(eq.child[i]);
			switch (eq.operation){
				case ADD:
					process.stdout.write("+");
					break;
				case MULTIPLY:
					process.stdout.write("*");
					break;
				case EXPONENT:
					process.stdout.write("^");
					break;
			}
		}
		print_equation(eq.child[eq.child.length-1]);
		process.stdout.write(")");
	} else {
		switch (eq.operation){
			case SINE:
				process.stdout.write("sin");
				break;
			case COSINE:
				process.stdout.write("cos");
				break;
			case COSECANT:
				process.stdout.write("cosec");
				break;
			case SECANT:
				process.stdout.write("secant");
				break;
			case TANGENT:
				process.stdout.write("tan");
				break;
			case COTANGENT:
				process.stdout.write("cot");
				break;
		}
		process.stdout.write("(");
		print_equation(eq.child[0]);
		process.stdout.write(")");
	}
}

function p(eq){
	print_equation(eq);
	process.stdout.write("\n");
}

const formula_dimension = 4;

var formula_input;
var formula_output;
var linear_tree;
var var_list_p_output;
var var_list_p;

function formula_list_gen(){
	var v_a = make_variable("a");
	var v_b = make_variable("b");
	var v_c = make_variable("c");
	var v_d = make_variable("d");
	formula_input[0].push(m(v_a, n(1)));
	formula_output[0].push(v_a);
	formula_input[0].push(a(v_a, v_a));
	formula_output[0].push(m(n(2), v_a));
	formula_input[0].push(w(v_a, n(0)));
	formula_output[0].push(n(1));
	formula_input[0].push(m(v_a, v_a));
	formula_output[0].push(w(v_a, n(2)));

	formula_input[1].push(a(v_a, m(v_a, v_b)));
	formula_output[1].push(m(v_a, a(n(1), v_b)));
	formula_input[1].push(m(v_a, w(v_a, v_b)));
	formula_output[1].push(w(v_a, a(v_b, n(1))));
	formula_input[1].push(a(w(v_a, n(-1)), v_b));
	formula_output[1].push(m(a(n(1), m(v_a, v_b)),w(v_a, n(-1))));
	
	formula_input[2].push(w(w(v_a, v_b), v_c));
	formula_output[2].push(w(v_a, m(v_b, v_c)));
	formula_input[2].push(m(w(v_a, v_b), w(v_a, v_c)));
	formula_output[2].push(w(v_a, a(v_b, v_c)));
	formula_input[2].push(a(m(v_a, v_b), m(v_a, v_c)));
	formula_output[2].push(m(v_a, a(v_b, v_c)));
	formula_input[2].push(m(v_a, a(v_b, v_c)));
	formula_output[2].push(a(m(v_a, v_b), m(v_a, v_c)));
	
	formula_input[3].push(m(a(v_a, v_b), a(v_c, v_d)));
	formula_output[3].push(a(a(m(v_a, v_c), m(v_a, v_d)), a(m(v_b, v_c), m(v_b, v_d))));
}

function tree2linear(eq){
	if (same_in_array(linear_tree, eq) == -1){
		linear_tree.push(eq);
	}
	if (eq.child.length != 0){
		for (var i=0; i<eq.child.length; ++i){
			tree2linear(eq.child[i]);
		}
	}
}

function var_list_combination(var_max, n){
	if (var_max == -1){
		var_list_p_output[n].push(JSON.parse(JSON.stringify(var_list_p[n])));
	} else {
		var letter = String.fromCharCode(var_max+"a".charCodeAt(0));
		for (var i=0; i<linear_tree.length; ++i){
			var_list_p[n][i].push(letter);
			var_list_combination(var_max-1, n);
			var_list_p[n][i].pop();
		}
	}
}

function put_value_in_formula_3(book, formula){
	for (var i=0; i<formula.child.length; ++i){
		if (formula.child[i].operation == VARIABLE){
			for (j=0; j<book.length; ++j){
				if (book[j].includes(formula.child[i].variable)){
					formula.child[i] = linear_tree[j];
				}
			}
		} else {
			formula.child[i] = put_value_in_formula(book, formula.child[i]);
		}
	}
	return formula;
}

function put_value_in_formula(book, formula){
	if (formula.child.length == 0 && formula.operation == VARIABLE){
		for (var i=0; i<book.length; ++i){
			if (book[i].includes(formula.variable)){
				formula = linear_tree[i];
			}
		}
		return formula;
	} else {
		return put_value_in_formula_3(book, formula);
	}
}
function found_in_tree(eq, inp, out){
	if (is_same(eq, inp)){
		eq = do_copy(out);
	} else {
		if (eq.child.length != 0){
			for (var i=0; i<eq.child.length; ++i){
				eq.child[i] = found_in_tree(eq.child[i], inp, out);
			}
		}
	}
	return eq;
}

function use_formula(eq){
	formula_input = [[], [], [], []];
	formula_output = [[], [], [], []];
	linear_tree = [];
	var_list_p_output = [[], [], [], []];
	var_list_p = [[], [], [], []];
	tree2linear(eq);
	for (var i=0; i<formula_dimension; ++i)
		for (var j=0; j<linear_tree.length; ++j)
			var_list_p[i].push([]);
	for (var i=0; i<formula_dimension; ++i)
		var_list_combination(i, i);
	formula_list_gen();
	for (var i=0; i<formula_dimension; ++i){
		for (var j=0; j<formula_input[i].length; ++j){
			for (var k=0; k<var_list_p_output[i].length; ++k){
				var finput = do_copy(formula_input[i][j]);
				var foutput = do_copy(formula_output[i][j]);
				finput = put_value_in_formula(var_list_p_output[i][k], finput);
				foutput = put_value_in_formula(var_list_p_output[i][k], foutput);
				var orig = do_copy(eq);
				eq = found_in_tree(eq, finput, foutput);
				if (is_same(orig, eq) == false){
					search_list.push(do_copy(eq));
					eq = orig;
				}
			}
		}
	}
	return eq;
}

function tree_fold_count(eq){
	var sc = 1;
	if (eq.child.length > 2){
		return eq.child.length;
	}
	for (var i=0; i<eq.child.length; ++i){
		sc *= tree_fold_count(eq.child[i]);
	}
	return sc;
}

function tree_fold_depth(eq){
	var sc = 0;
	if (eq.child.length > 2){
		sc = 1;
	}
	if (eq.child.length != 0){
		for (var i=0; i<eq.child.length; ++i){
			sc += tree_fold_depth(eq.child[i]);
		}
	}
	return sc;
}

function all_possible_tree_fold(eq, n){
	if (n==0){
		search_list.push(do_copy(eq));
	} else {
		var h = tree_fold_count(eq);
		for (var i=0; i<counting(h); ++i){
			var orig = do_copy(eq);
			eq = tree_fold(do_copy(eq), i);
			all_possible_tree_fold(do_copy(eq), n-1);
			eq = orig;
		}
	}
}

function counting(n){
	return treefold_helper(n).length;
}

function treefold_helper(n){
	var tmp = [];
	for (var i=0; i<n; ++i) tmp.push(String.fromCharCode(i+"a".charCodeAt(0)));
	var collection_tree_2 = create_collection_2(n);
	var pp = permutator(tmp);
	var output = [];
	for (var i=0; i<collection_tree_2.length; ++i){
		var one_tree = do_copy(collection_tree_2[i]);
		for (var j=0; j<pp.length; ++j){
			var two_tree = do_copy(one_tree);
			for (var k=0; k<n; ++k){
				nth_x_target = k+1;
				replacer = make_variable(pp[j][k]);
				two_tree = nth_x(two_tree);
			}
			output.push(two_tree);
		}
	}
	output = remove_duplicate(output);
	return output;
}
function put_value_in_formula_2(bookfrom, bookto, formula){
	for (var i=0; i<formula.child.length; ++i){
		if (formula.child[i].operation == VARIABLE){
			if (bookfrom.includes(formula.child[i].variable)){
				formula.child[i] = bookto[bookfrom.indexOf(formula.child[i].variable)];
			}
		} else {
			formula.child[i] = put_value_in_formula_2(bookfrom, bookto, formula.child[i]);
		}
	}
	return formula;
}
function tree_fold(eq, n){
	if (eq.child.length > 2){
		var formu = do_copy(treefold_helper(eq.child.length)[n]);
		var tmp = [];
		for (var i=0; i<eq.child.length; ++i) tmp.push(String.fromCharCode(i+"a".charCodeAt(0)));
		return put_value_in_formula_2(tmp, eq.child, formu);
	} else {
		for (var i=0; i<eq.child.length; ++i){
			eq.child[i] = tree_fold(eq.child[i], n);
		}
	}
	return eq;
}

function new_equation_op(op){
	return new Equation(op, 0, 0, []);
}

function count_end(eq){
	var sc = 0;
	for (var i=0; i<2; ++i){
		if (eq.child[i].operation == VARIABLE) sc++;
		else {
			sc += count_end(eq.child[i]);
		}
	}
	return sc;
}
var collection_tree;
var nth_x_target;
var replacer;
function nth_x(eq){
	for (var i=0; i<eq.child.length; ++i){
		if (eq.child[i].operation == VARIABLE){
			if (--nth_x_target == 0){
				eq.child[i] = do_copy(replacer);
				return eq;
			}
		}
		else {
			eq.child[i] = nth_x(eq.child[i]);
		}
	}
	return eq;
}

function create_collection(eq, depth){
	if (depth == 0){
		collection_tree.push(do_copy(eq));
	} else {
		for (var i=0; i<count_end(eq); ++i){
			nth_x_target=i+1;
			replacer = operate_equation(ADD, make_variable("x"), make_variable("x"));
			var orig = do_copy(eq);
			eq = nth_x(eq);
			create_collection(eq, depth-1);
			eq = orig;
		}
	}
}

var permArr = [], usedChars = [];
function permute(input) {
	var i, ch;
	for (i = 0; i < input.length; i++) {
		ch = input.splice(i, 1)[0];
		usedChars.push(ch);
		if (input.length == 0) {
			permArr.push(usedChars.slice());
		}
		permute(input);
		input.splice(i, 0, ch);
		usedChars.pop();
	}
	return permArr
};
function permutator(input){
	permArr = [];
	usedChars = [];
	return permute(input);
}
function remove_duplicate(ary){
	for (var i=0; i<ary.length; ++i){
		var is_change = false;
		for (var j=0; j<ary.length; ++j){
			if (i != j){
				if (is_same(ary[i], ary[j])){
					ary.splice(j, 1);
					is_change = true;
				}
			}
		}
		if (is_change){
			i=-1;
		}
	}
	return ary;
}
function is_constant_doer(eq){
	if (eq.operation != ADD && eq.operation != MULTIPLY && eq.operation != EXPONENT && eq.operation != CONSTANT){
		return -1;
	} else {
		for (var i=0; i<eq.child.length; ++i){
			var h = is_constant_doer(eq.child[i]);
			if (h == -1) return -1;
		}
	}
	return 0;
}
function constant_doer(eq){
	var result;
	if (eq.operation == ADD){
		var sum=0;
		for (var i=0; i<eq.child.length; ++i){
			if (eq.child[i].operation == CONSTANT){
				sum += eq.child[i].constant;
			} else {
				sum += constant_doer(eq.child[i]);
			}
		}
		result = sum;
	} else if (eq.operation == MULTIPLY){
		var product=1;
		for (var i=0; i<eq.child.length; ++i){
			if (eq.child[i].operation == CONSTANT){
				product *= eq.child[i].constant;
			} else {
				product *= constant_doer(eq.child[i]);
			}
		}
		result = product;
	} else if (eq.operation == EXPONENT){
		if (eq.child[0].operation == CONSTANT && eq.child[1].operation == CONSTANT)
			result = Math.pow(eq.child[0].constant, eq.child[1].constant);
		else if (eq.child[0].operation != CONSTANT && eq.child[1].operation == CONSTANT){
			result = Math.pow(eq.child[0].constant, constant_doer(eq.child[1]));
		} else if (eq.child[0].operation == CONSTANT && eq.child[1].operation != CONSTANT){
			result = Math.pow(constant_doer(eq.child[0]), eq.child[1].constant);
		}
	}		
	else if (eq.operation == CONSTANT){
		result = eq.constant;
	}
	return result;
}
function constant_doer_2(eq){
	if (is_constant_doer(eq) == 0){
		return n(constant_doer(eq));
	}
	for (var i=0; i<eq.child.length; ++i){
		eq.child[i] = constant_doer_2(eq.child[i]);
	}
	return eq;
}
function create_collection_2(n){
	n=n-2;
	collection_tree = [];
	var eq = a(make_variable("x"), make_variable("x"));
	create_collection(eq, n);
	var collection_tree_2 = [];
	for (var i=0; i<collection_tree.length; ++i) collection_tree_2.push(do_copy(collection_tree[i]));
	//for (var i=0; i<collection_tree_2.length; ++i) p(collection_tree_2[i]);
	collection_tree_2 = remove_duplicate(collection_tree_2);
	return collection_tree_2;
}
var search_list = [];
var target;
var principle_variation = [];
function search(eq, depth, brac){
	if (is_same(eq, target)){
		return 1;
	}
	if (depth == 0) return -1;
	search_list = [];
	use_formula(do_copy(eq));
	if (brac == 1){
		var eq_open = fix_tree_recursive(do_copy(eq));
		all_possible_tree_fold(do_copy(eq_open), tree_fold_depth(eq_open));
		brac = 0;
	} else {
		brac = 1;
	}
	var search_list_curr = [];
	for (var i=0; i<search_list.length; ++i) search_list_curr.push(constant_doer_2(do_copy(search_list[i])));
	search_list_curr = remove_duplicate(search_list_curr);
	for (var i=0; i<search_list_curr.length; ++i){
		var h = search(do_copy(search_list_curr[i]), depth-1, brac);
		if (h == 1){
			principle_variation[depth-1] = do_copy(search_list_curr[i]);
			return 1;
		}
	}
	return -1;
}

var x = make_variable("x");
target = a( w(x, n(2)),  a( m(n(3), x), n(2) ) );
x = m(a(x, n(1)), a(x, n(2)));
console.log("Start From:");
p(x);
console.log("Target:");
p(target);
console.log();
const target_depth = 4;
for (var i=0; i<target_depth; ++i) principle_variation.push(0);
search(do_copy(x), target_depth, 1);
for (var i=0; i<principle_variation.length; ++i){
	if (principle_variation[i] == 0) continue;
	p(principle_variation[i]);
}
