import itertools

MAX_VARIABLE_ALLOWED = 5

TYPE_VARIABLE = -1
TYPE_CONSTANT = -2
ADD = 0
MULTIPLY = 1
POWER = 2
SINE = 3
COSINE = 4
TANGENT = 5
COSECANT = 6
SECANT = 7
COTANGENT = 8

function_name = ["add", "mul", "pow", "sin", "cos", "tan", "csc", "sec", "cot"]
var_list = [None]*MAX_VARIABLE_ALLOWED

def string_equation(equation):
    string=""
    if equation.function_type == TYPE_VARIABLE:
        string += equation.var_name
    elif equation.function_type == TYPE_CONSTANT:
        string += str(equation.const_value)
    else:
        string += function_name[equation.function_type]+"("
        for i in range(len(equation.children)):
            string += string_equation(equation.children[i])
            if i != len(equation.children)-1:
                string += ","
        string += ")"
    return string

class Equation:
    def __init__(self, function_type, children, const_value, var_name):
        self.function_type = function_type
        self.children = children
        self.const_value = const_value
        self.var_name = var_name
    def eq_to_string(self):
        return string_equation(self)
        
                
def compare_number(number_1, number_2):
    if abs(number_1 - number_2) < 0.001:
        return True
    return False

def compare_equation(equation_1, equation_2):
    if equation_1.function_type != equation_2.function_type:
        return False
    if len(equation_1.children) != len(equation_2.children):
        return False
    if equation_1.function_type == TYPE_VARIABLE and equation_1.var_name != equation_2.var_name:
        return False
    if equation_1.function_type == TYPE_CONSTANT and compare_number(equation_1.const_value, equation_2.const_value) == False:
        return False
    if len(equation_1.children) != 0:
        permutation_arr = list(itertools.permutations(equation_2.children))
        for i in permutation_arr:
            for j in range(len(equation_1.children)):
                if compare_equation(equation_1.children[j], i[j]) == False:
                    break
                elif j == len(equation_1.children)-1:
                    return True
        return False
    return True

def copy_equation(equation):
    output = Equation(equation.function_type, [], equation.const_value, equation.var_name)
    for i in range(len(equation.children)):
        output.children.append(copy_equation(equation.children[i]))
    return output

def M(term):
    return Equation(MULTIPLY, term, None, None)

def A(term):
    return Equation(ADD, term, None, None)

def W(term):
    return Equation(POWER, term, None, None)

def FX(fx_type, term):
    return Equation(fx_type, term, None, None)

def N(number):
    return Equation(TYPE_CONSTANT, [], number, None)

def V(var_name):
    return Equation(TYPE_VARIABLE, [], None, var_name)

formula_list = []
output_formula_list = []

def evaluate_output_formula(output_formula):
    if output_formula.function_type == TYPE_VARIABLE:
        return copy_equation(var_list[ord(output_formula.var_name)-ord("a")])
    for i in range(len(output_formula.children)):
        output_formula.children[i] = evaluate_output_formula(output_formula.children[i])
    return output_formula

def apply_formula_first_call(equation, formula, output_formula):
    global var_list
    for i in range(MAX_VARIABLE_ALLOWED):
        var_list[i] = None
    result = apply_formula_recursive(equation, formula)
    if result == True:
        return evaluate_output_formula(output_formula)
    return None

def apply_formula_recursive(equation, formula):
    global var_list
    if formula.function_type == TYPE_VARIABLE:
        if var_list[ord(formula.var_name)-ord("a")] == None:
           var_list[ord(formula.var_name)-ord("a")] = copy_equation(equation)
        else:
            if compare_equation(var_list[ord(formula.var_name)-ord("a")], equation) == False:
                return False
        return True
    if equation.function_type != formula.function_type:
        return False
    if len(equation.children) != len(formula.children):
        return False
    if formula.function_type == TYPE_CONSTANT and compare_number(equation.const_value, formula.const_value) == False:
        return False
    for i in range(len(equation.children)):
        if apply_formula_recursive(equation.children[i], formula.children[i]) == False:
            return False
    return True

def merge_nested_bracket(equation):
    index=0
    while index<len(equation.children):
        if equation.function_type == ADD or equation.function_type == MULTIPLY:
            if equation.function_type == equation.children[index].function_type:
                j = 0
                while j<len(equation.children[index].children):
                    equation.children.append(copy_equation(equation.children[index].children[j]))
                    j += 1
                equation.children.pop(index)
                index -= 1    
        index += 1
    for i in range(len(equation.children)):
        equation.children[i] = merge_nested_bracket(equation.children[i])
    return equation

def fact(n):
    product = 1
    for i in range(1, n+1):
        product *= i
    return product

def nCr(n,r):
    return int(fact(n)/(fact(r)*fact(n-r)))

def iterate_formula_count(equation, formula):
    count = 0
    if len(equation.children) >= len(formula.children):
        count = nCr(len(equation.children),len(formula.children))
    for i in range(len(equation.children)):
        count += iterate_formula_count(equation.children[i], formula)
    return count

iterator_n = 0
def iterate_formula(equation, formula, output_formula):
    global iterator_n
    if len(equation.children) >= len(formula.children):
        for j in range(nCr(len(equation.children), len(formula.children))):
            iterator_n -= 1
            if iterator_n == 0:
                eq = copy_equation(equation)
                eq.children = list(itertools.combinations(equation.children, len(formula.children)))[j]
                result = apply_formula_first_call(copy_equation(eq), copy_equation(formula), copy_equation(output_formula))
                if result != None:
                    eq_2 = copy_equation(equation)
                    eq_2.children = []
                    for i in range(len(equation.children)):
                        for j in range(len(eq.children)):
                            if compare_equation(equation.children[i], eq.children[j]) == True:
                                break
                            elif j == len(eq.children)-1:
                                eq_2.children.append(equation.children[i])
                    eq_2.children.append(result)
                    if len(eq_2.children) == 1:
                        eq_2 = eq_2.children[0]
                    return merge_nested_bracket(eq_2)
    for i in range(len(equation.children)):
        if iterator_n != 0:
            equation.children[i] = iterate_formula(equation.children[i], formula, output_formula)
    return equation

def check_only_number(equation):
    if equation.function_type == TYPE_CONSTANT:
        return True
    if equation.function_type == TYPE_VARIABLE:
        return False
    for i in range(len(equation.children)):
        if check_only_number(equation.children[i]) == False:
            return False
    return True

def solve_number(equation):
    if equation.function_type == TYPE_CONSTANT:
        return equation.const_value
    elif equation.function_type == ADD:
        summation = 0
        for i in range(len(equation.children)):
            summation += solve_number(equation.children[i])
        return summation
    elif equation.function_type == MULTIPLY:
        product = 1
        for i in range(len(equation.children)):
            product *= solve_number(equation.children[i])
        return product
    elif equation.function_type == POWER:
        term_1 = solve_number(equation.children[0])
        term_2 = solve_number(equation.children[1])
        return pow(term_1, term_2)
    return 0

def apply_constant(equation):
    if check_only_number(equation):
        return N(solve_number(equation))
    for i in range(len(equation.children)):
        equation.children[i] = apply_constant(equation.children[i])
    return equation

def fix_constant(equation):
    while True:
        if equation.function_type == TYPE_VARIABLE or equation.function_type == TYPE_CONSTANT:
            return equation
        if equation.function_type == ADD:
            summation = 0
            count = 0
            for i in range(len(equation.children)):
                if equation.children[i].function_type == TYPE_CONSTANT and compare_number(equation.children[i].const_value, 0) == False:
                    count += 1
            if count == 1:
                break
            for i in range(len(equation.children)-1,-1,-1):
                if equation.children[i].function_type == TYPE_CONSTANT:
                    summation += equation.children[i].const_value
                    equation.children.pop(i)
            if compare_number(summation, 0) == False:
                equation.children.append(N(summation))
        elif equation.function_type == MULTIPLY:
            product = 1
            for i in range(len(equation.children)):
                if equation.children[i].function_type == TYPE_CONSTANT and compare_number(equation.children[i].const_value, 0) == True:
                    return N(0)
            count = 0
            for i in range(len(equation.children)):
                if equation.children[i].function_type == TYPE_CONSTANT and compare_number(equation.children[i].const_value, 1) == False:
                    count += 1
            if count == 1:
                break
            for i in range(len(equation.children)-1,-1,-1):
                if equation.children[i].function_type == TYPE_CONSTANT:
                    product *= equation.children[i].const_value
                    equation.pop(i)
            if compare_number(product, 1) == False:
                equation.children.append(N(product))
        elif equation.function_type == POWER and equation.children[1].function_type == TYPE_CONSTANT:
            if compare_number(equation.children[1].const_value, 0) == True:
                return N(1)
            if compare_number(equation.children[1].const_value, 1) == True:
                equation = equation.children[0]
                continue
        if len(equation.children) == 1 and (equation.function_type == ADD or equation.function_type == MULTIPLY):
            equation = equation.children[0]
        else:
            break
    for i in range(len(equation.children)):
        equation.children[i] = fix_constant(equation.children[i])
    return equation

def fix(equation):
    return merge_nested_bracket(fix_constant(apply_constant(equation)))

def generate_move_formula(equation):
    global iterator_n
    global formula_list
    global output_formula_list
    move_list = []
    constant_done = fix(copy_equation(equation))
    if compare_equation(equation, constant_done) == False:
        move_list.append(constant_done)
    for j in range(len(formula_list)):
        n = 1
        count = iterate_formula_count(equation, formula_list[j])
        while n <= count:
            iterator_n = n
            result = merge_nested_bracket(iterate_formula(copy_equation(equation), copy_equation(formula_list[j]), copy_equation(output_formula_list[j])))
            if result != None and compare_equation(result, equation) == False:
                move_list.append(result)
            n += 1
    return move_list

def blank_equation(function_type):
    return  Equation(function_type, [], None, None)

def distributive_formula(term_1, term_2):
    global formula_list
    global output_formula_list
    arr = list(itertools.product(term_1, term_2))
    output = blank_equation(ADD)
    for i in range(len(arr)):
        output.children.append(M([arr[i][0],arr[i][1]]))
    output_formula_list.append(output)
    if len(term_1) == 1:
        term_1 = term_1[0]
    else:
        term_1 = A(term_1)
    if len(term_2) == 1:
        term_2 = term_2[0]
    else:
        term_2 = A(term_2)
    formula_list.append(M([term_1, term_2]))

def distributive_gen():
    global formula_list
    global output_formula_list
    arr = list(itertools.product([1, 2, 3],[1, 2, 3]))
    arr.remove((1,1))
    for i in range(len(arr)):
        u=0
        j=0
        term_1 = []
        term_2 = []
        while j<arr[i][0]:
            term_1.append(V(chr(ord("a")+u)))
            j += 1
            u += 1
        j=0
        while j<arr[i][1]:
            term_2.append(V(chr(ord("a")+u)))
            j += 1
            u += 1
        distributive_formula(term_1, term_2)

def common_gen(arr, length):
    global formula_list
    global output_formula_list
    
    for i in range(2, length):
        arr_2 = list(itertools.permutations(arr, i))
        for j in range(len(arr_2)):
            output = blank_equation(ADD)
            summation = 0
            for k in range(len(arr_2[j])):
                if len(arr_2[j][k]) == 1:
                    output.children.append(V(arr_2[j][k][0]))
                    continue
                output_2 = blank_equation(MULTIPLY)
                for l in range(len(arr_2[j][k])):
                    if arr_2[j][k][l] == "a":
                        output_2.children.append(V(arr_2[j][k][l]))
                    else:
                        output_2.children.append(V(chr(ord(arr_2[j][k][l])+summation)))
                output.children.append(output_2)
                summation += len(arr_2[j][k]) - 1
            formula_list.append(copy_equation(output))
            for k in range(len(output.children)):
                if compare_equation(output.children[k], V("a")):
                    output.children[k] = N(1)
                else:
                    for l in range(len(output.children[k].children)):
                        if compare_equation(output.children[k].children[l], V("a")):
                            output.children[k].children.pop(l)
                            if len(output.children[k].children) == 1:
                                output.children[k] = output.children[k].children[0]
                            break
            output_formula_list.append(M([V("a"),output]))

def add_formula_to_list():
    global formula_list
    global output_formula_list

    #common_gen([ ["a"], ["a","b"], ["b","a"], ["a","b","c"], ["b","a","c"], ["b","c","a"] ], 4)
    common_gen([ ["a"], ["a","b"], ["b","a"] ], 3)
    distributive_gen()
    
    formula_list.append(M([V("a"),V("a")]))
    output_formula_list.append(W([V("a"),N(2)]))
    '''
    for i in range(len(formula_list)):
        print(formula_list[i].eq_to_string())
        print(output_formula_list[i].eq_to_string())
        print()
    '''
#ex = FX(SINE, V("x"))
ex = V("x")
target = A([M([ex,N(5)]), W([ex,N(2)]), N(6)])
start = M([A([N(3),ex]),A([ex,N(2)])])
#start = A([V("x"),N(1),M([V("x"),N(2)])])

transposition_table = []
tt_depth = []

def findInTT(equation):
    for i in range(len(transposition_table)):
        if compare_equation(transposition_table[i], equation) == True:
            return i
    return -1

def search(equation, depth):
    global target
    global transposition_table
    global tt_depth
    if compare_equation(target, equation):
        print(target.eq_to_string())
        return True
    s_dep = findInTT(equation)
    if s_dep != -1:
        if tt_depth[s_dep] > depth:
            tt_depth[s_dep] = depth
        else:
            return 0
    transposition_table.append(copy_equation(equation))
    tt_depth.append(100)
    
    if depth == 4:
        return False
    move_list = generate_move_formula(equation)
    #for i in move_list:
    #    print(i.eq_to_string())
    for i in move_list:
        if search(copy_equation(i), depth+1):
            print(equation.eq_to_string())
            return True
    return False

add_formula_to_list()
print("Start: " + start.eq_to_string())
print("Target: " + target.eq_to_string())
print()
search(start, 0)
