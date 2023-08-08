import itertools

MAX_VARIABLE_ALLOWED = 26

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
SUBSTRACT = 9
DIVISION = 10

function_name = ["add", "mul", "pow", "sin", "cos", "tan", "csc", "sec", "cot", "sub", "div"]
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
        permutation_arr = None
        if equation_1.function_type == ADD or equation_1.function_type == MULTIPLY:
            permutation_arr = list(itertools.permutations(equation_2.children))
        else:
            permutation_arr = [equation_2.children]
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

def W(term_1, term_2):
    return Equation(POWER, [term_1, term_2], None, None)

def FX(fx_type, term):
    return Equation(fx_type, term, None, None)

def N(number):
    return Equation(TYPE_CONSTANT, [], number, None)

def V(var_name):
    return Equation(TYPE_VARIABLE, [], None, var_name)

def S(term_1, term_2):
    return Equation(SUBSTRACT, [term_1, term_2], None, None)

def D(term_1, term_2):
    return Equation(DIVISION, [term_1, term_2], None, None)

formula_list = []
output_formula_list = []

def evaluate_output_formula(output_formula):
    global var_list
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
            if ord(formula.var_name)-ord("a") >= ord("m")-ord("a") and equation.function_type != TYPE_CONSTANT:
                return False
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

def iterate_formula_count(equation, formula):
    count = 1
    for i in range(len(equation.children)):
        count += iterate_formula_count(equation.children[i], formula)
    return count

iterator_n = 0
def iterate_formula(equation, formula, output_formula):
    global iterator_n
    iterator_n -= 1
    if iterator_n == 0:
        result = apply_formula_first_call(copy_equation(equation), copy_equation(formula), copy_equation(output_formula))
        if result != None:
            return result
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
    elif equation.function_type == SUBSTRACT:
        term_1 = solve_number(equation.children[0])
        term_2 = solve_number(equation.children[1])
        return term_1 - term_2
    return 0

def apply_constant(equation):
    if check_only_number(equation):
        return N(solve_number(equation))
    for i in range(len(equation.children)):
        equation.children[i] = apply_constant(equation.children[i])
    return equation

def remove_all(equation):
    for i in range(len(equation.children)-1,-1,-1):
        if equation.children[i].function_type == TYPE_CONSTANT:
            equation.children.pop(i)

def fix_constant(equation):
    if equation.function_type == TYPE_VARIABLE or equation.function_type == TYPE_CONSTANT:
        return equation
    if equation.function_type == ADD:
        summation = 0
        count = 0
        for i in range(len(equation.children)):
            if equation.children[i].function_type == TYPE_CONSTANT:
                summation += equation.children[i].const_value
                count += 1
        if count > 1:
            remove_all(equation)
            equation.children.append(N(summation))
    elif equation.function_type == MULTIPLY:
        product = 1
        count = 0
        for i in range(len(equation.children)):
            if equation.children[i].function_type == TYPE_CONSTANT:
                product *= equation.children[i].const_value
                count += 1
        if count > 1:
            remove_all(equation)
            equation.children.append(N(product))
    for i in range(len(equation.children)):
        equation.children[i] = fix_constant(equation.children[i])
    return equation

def auto_formula(equation):
    global iterator_n
    
    local_formula_list = []
    local_output_formula_list = []
    
    local_formula_list.append(M([V("a"),N(0)]))
    local_formula_list.append(M([N(0),V("a")]))
    local_output_formula_list.append(N(0))
    local_output_formula_list.append(N(0))

    local_formula_list.append(A([V("a"),N(0)]))
    local_formula_list.append(A([N(0),V("a")]))
    local_output_formula_list.append(V("a"))
    local_output_formula_list.append(V("a"))

    local_formula_list.append(M([V("a"),N(1)]))
    local_formula_list.append(M([N(1),V("a")]))
    local_output_formula_list.append(V("a"))
    local_output_formula_list.append(V("a"))

    local_formula_list.append(M([V("a"),V("a")]))
    local_output_formula_list.append(W(V("a"),N(2)))

    local_formula_list.append(W(V("a"),N(1)))
    local_output_formula_list.append(V("a"))    

    local_formula_list.append(A(   [   M([V("m"),V("a")]), V("b"), V("c"), M([V("a"),V("n")])   ]))
    local_output_formula_list.append(A([    M([ V("a"),A([V("m"),V("n")])  ])  ,   V("c"), V("b")   ]))
    
    for j in range(len(local_formula_list)):
        n = 1
        count = iterate_formula_count(equation, local_formula_list[j])
        while n <= count:
            iterator_n = n
            result = merge_nested_bracket(iterate_formula(copy_equation(equation), copy_equation(local_formula_list[j]), copy_equation(local_output_formula_list[j])))
            if result != None:
                equation = result
            n += 1
    return equation

def in_terms_of_sinxcosx_helper(equation):
    global iterator_n
    
    local_formula_list = []
    local_output_formula_list = []
    
    local_formula_list.append(FX(TANGENT,[V("a")]))
    local_output_formula_list.append(D(FX(SINE,[V("a")]), FX(COSINE,[V("a")])))

    local_formula_list.append(FX(COTANGENT,[V("a")]))
    local_output_formula_list.append(D(FX(COSINE,[V("a")]), FX(SINE,[V("a")])))

    local_formula_list.append(FX(COSECANT,[V("a")]))
    local_output_formula_list.append(D(N(1), FX(SINE,[V("a")])))

    local_formula_list.append(FX(SECANT,[V("a")]))
    local_output_formula_list.append(D(N(1), FX(COSINE,[V("a")])))
    
    for j in range(len(local_formula_list)):
        n = 1
        count = iterate_formula_count(equation, local_formula_list[j])
        while n <= count:
            iterator_n = n
            result = merge_nested_bracket(iterate_formula(copy_equation(equation), copy_equation(local_formula_list[j]), copy_equation(local_output_formula_list[j])))
            if result != None:
                equation = result
            n += 1
    return equation

def in_terms_of_sinxcosx(equation):
    while True:
        orig_equation = copy_equation(equation)
        equation = in_terms_of_sinxcosx_helper(copy_equation(equation))
        equation = merge_nested_bracket(copy_equation(equation))
        if compare_equation(orig_equation, equation) == True:
            break
    return equation

def fix(equation):
    while True:
        orig_equation = copy_equation(equation)
        equation = apply_constant(copy_equation(equation))
        equation = fix_constant(copy_equation(equation))
        equation = auto_formula(copy_equation(equation))
        equation = merge_nested_bracket(copy_equation(equation))
        if compare_equation(orig_equation, equation) == True:
            break
    return equation

def generate_move_formula(equation):
    global iterator_n
    global formula_list
    global output_formula_list
    move_list = []
    term_sinxcosx = in_terms_of_sinxcosx(copy_equation(equation))
    if compare_equation(term_sinxcosx, equation) == False:
        move_list.append(term_sinxcosx)
    for j in range(len(formula_list)):
        n = 1
        count = iterate_formula_count(equation, formula_list[j])
        while n <= count:
            iterator_n = n
            result = merge_nested_bracket(iterate_formula(copy_equation(equation), copy_equation(formula_list[j]), copy_equation(output_formula_list[j])))
            if result != None and compare_equation(result, equation) == False:
                move_list.append(fix(result))
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

def add_formula_to_list():
    global formula_list
    global output_formula_list

    formula_list.append(S(D(V("a"),V("b")),D(V("c"),V("b"))))
    output_formula_list.append(D(S(V("a"),V("c")),V("b")))

    formula_list.append(W(D(V("a"),V("b")),V("c")))
    output_formula_list.append(D(W(V("a"),V("c")),W(V("b"),V("c"))))

    formula_list.append(W(FX(SINE,[V("a")]),N(2)))
    output_formula_list.append(S(N(1),W(FX(COSINE,[V("a")]),N(2))))

    formula_list.append(S(N(1),W(V("a"),N(2))))
    output_formula_list.append(M([ A([N(1),V("a")]) , S(N(1),V("a"))  ]))

    formula_list.append(D(W(V("a"),V("b")), M([V("c"),V("a")])))
    output_formula_list.append(D(    W(V("a"),S(V("b"),N(1)))  ,    V("c")))

    formula_list.append(M([  A([V("a"),V("b")]),  A([V("c"),V("d")])    ]))
    output_formula_list.append(A([  M([V("a"),V("c")]),   M([V("a"),V("d")]),   M([V("b"),V("c")]),   M([V("b"),V("d")])    ]))

    formula_list.append(A([D(V("a"),V("b")),D(V("c"),V("d"))]))
    output_formula_list.append(D(A([  M([V("a"),V("d")])  , M([V("b"),V("c")])   ]), M([V("b"),V("d")])  ))

    formula_list.append(W(A([V("a"),V("b")]),N(2)))
    output_formula_list.append( A([W(V("a"),N(2)), W(V("b"),N(2))  ,   M([N(2),V("a"),V("b")])  ]))

    formula_list.append(  A([W(FX(COSINE,[V("a")]),N(2)),    W(FX(SINE,[V("a")]),N(2)), V("b"),V("c")    ])      )
    output_formula_list.append(A([  N(1), V("b"), V("c")  ]))

    formula_list.append( A([ M([V("b"), V("a")]) ,  V("a")])  )
    output_formula_list.append(  M([V("a"),  A([V("b"),N(1)])  ])  )

    formula_list.append(D( M([V("a"), V("b")]) , M([V("b"),V("c")]) ))
    output_formula_list.append(D(V("a"), V("c")))
    #for i in range(len(formula_list)):
    #    print(formula_list[i].eq_to_string())
    #    print(output_formula_list[i].eq_to_string())
    #    print()
    
ex = V("x")

#start = M([A([N(3),ex]),A([ex,N(2)])])
#target = A([M([ex,N(5)]), W(ex,N(2)), N(6)])

#target = D(  S(N(1),FX(COSINE,[ex])), A([N(1),FX(COSINE,[ex])])     )
#start = W(S(FX(COSECANT,[ex]),FX(COTANGENT,[ex])),N(2))

one_plus_sinx = copy_equation( A([FX(SINE,[ex]),N(1)]) )
cosx = copy_equation( FX(COSINE,[ex]) )
start = A([D(cosx, one_plus_sinx),D(one_plus_sinx, cosx)])
target = D(N(2),cosx)

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
    
    if depth == 6:
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
