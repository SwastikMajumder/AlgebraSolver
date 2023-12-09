from nltk import CFG, ChartParser, Tree
import nltk.tree as sTree
import copy
import sys
from collections import deque
import itertools

class TreeNode:
    def __init__(self, label, children=None):
        self.label = label
        self.children = children or []

    def __str__(self):
        tree = self.to_nltk_tree()
        return sTree.TreePrettyPrinter(tree).text()
            
    @staticmethod
    def integer_digit(tree):
        output_string = ""
        for child in tree.children:
            if child.label == "-":
                output_string += "-"
                continue
            if child.label == "Digit":
                output_string += child.children[0].label
            else:
                output_string += TreeNode.integer_digit(child)
        return output_string

    @staticmethod
    def custom_print_tree(tree):
        output_string = ""
        sym = None
        start_index = 0
        if tree.label == "Multiply":
            sym = "*"
        elif tree.label == "Add":
            sym = "+"
        elif tree.label == "Subtract":
            sym = "-"
        elif tree.label == "Divide":
            sym = "/"
        elif tree.label == "Power":
            sym = "^"
        elif tree.label == "Function":
            output_string += tree.children[0].label.lower()
            sym = ","
            start_index = 1
        elif tree.label == "Letter" or tree.label == "Digit":
            return tree.children[0].label
        elif tree.label == "Integer":
            return TreeNode.integer_digit(tree)
        output_string += "("
        for child in tree.children[start_index:-1]:
            output_string += TreeNode.custom_print_tree(child)
            output_string += sym
        output_string += TreeNode.custom_print_tree(tree.children[-1])
        output_string += ")"
        return output_string

    def component_gen(self):
        component_list = [self.print_algebra()]
        for child in self.children:
            if not child.children or self.label == "Integer" or self.label == "Digit":
                continue
            component_list += child.component_gen()
        return list(set(component_list))
    
    def print_algebra(self):
        return TreeNode.custom_print_tree(self)

    def merge_add_multiply(self):
        new_children = []
        for child in self.children:
            if isinstance(child, TreeNode):
                child.merge_add_multiply()
                if child.label in {'Add', 'Multiply'} and child.label == self.label:
                    new_children.extend(child.children)
                else:
                    new_children.append(child)
            else:
                new_children.append(child)
        self.children = new_children
        return self
    def to_nltk_tree(self):
        if not self.children:
            return self.label
        else:
            return Tree(self.label, [child.to_nltk_tree() if isinstance(child, TreeNode) else child for child in self.children])

def compare_equation(eq_1, eq_2):
    return eq_1.print_algebra() == eq_2.print_algebra()

def math_parser(sentence, merge=True):
    def trim_tree(tree):
        if isinstance(tree, Tree):
            if len(tree) == 1 and isinstance(tree[0], Tree):
                return trim_tree(tree[0])
            else:
                return Tree(tree.label(), [trim_tree(child) for child in tree])
        else:
            return tree
        
    def remove_parentheses(node):
        if isinstance(node, Tree):
            children = [remove_parentheses(child) for child in node]
            return Tree(node.label(), [child for child in children if child is not None])
        else:
            return None if node in ('(', ')', ',') else node

    def attach_arguments_to_parent(tree):
        if not isinstance(tree, Tree):
            return tree
        new_children = []
        for child in tree:
            if isinstance(child, Tree) and child.label() == 'Arguments':
                new_children.extend(child)
            else:
                new_children.append(attach_arguments_to_parent(child))
        return Tree(tree.label(), new_children)

    def nltk_tree_to_custom_tree(nltk_tree):
        if isinstance(nltk_tree, Tree):
            label = nltk_tree.label()
            children = [nltk_tree_to_custom_tree(child) for child in nltk_tree]
            return TreeNode(label, children)
        else:
            return TreeNode(nltk_tree)

    def trim(tree):
        coll = TreeNode(tree.label, [])
        if tree.label == "Integer":
            return TreeNode(tree.label, copy.deepcopy(tree.children))
        for child in tree.children:
            if not child.children and (len(child.label)>1 or (not child.label.isalpha() and not child.label.isdigit())):
                continue
            coll.children += [trim(child)]
        return coll

    if len(sentence) == 1:
        if sentence.isalpha():
            return TreeNode("Letter", [TreeNode(sentence, [])])
        elif sentence.isdigit():
            return TreeNode("Digit", [TreeNode(sentence, [])])
    
    grammar = CFG.fromstring("""
    Expression   -> Add | Term
    Add          -> Expression '+' Term
    Term         -> Multiply | Factor
    Multiply     -> Factor '*' Term
    Factor       -> Power | '(' Expression ')' | Integer | Letter | Function
    Power        -> Atom '^' Factor | Atom
    Atom         -> '(' Expression ')' | Integer | Letter | Function
    Integer      -> Digit | Digit Integer | '-' Integer
    Digit        -> '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
    Letter       -> 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' | 'k' | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' | 'u' | 'v' | 'w' | 'x' | 'y' | 'z'
    Function     -> FunctionName '(' Arguments ')' | FunctionName '(' Function ')'
    Arguments    -> Expression | Expression ',' Arguments
    FunctionName -> Sin | Cos | Tan | Cosec | Sec | Cot | Integrate | Defintegrate
    Sin          -> 'sin'
    Cos          -> 'cos'
    Tan          -> 'tan'
    Cosec        -> 'cosec'
    Sec          -> 'sec'
    Cot          -> 'cot'
    Integrate    -> 'integrate'
    Defintegrate -> 'defintegrate'
    """)
    parser = ChartParser(grammar)
    sentence = sentence.replace(" ", "")
    sentence = sentence.replace("-", "+-1*")
    sentence = sentence.replace("(+", "(")
    sentence = sentence.replace("-1*1", "-1")
    if sentence[0] == "+":
        sentence = sentence[1:]
    sentence = ' '.join([i for i in sentence])
    sentence = ''.join([sentence[0]]+[sentence[i] for i in range(1, len(sentence)-1) if not(sentence[i-1].isalpha() and sentence[i]==" " and sentence[i+1].isalpha())]+[sentence[-1]])
    tokens = sentence.split()
    nltk_tree = None
    for tree in parser.parse(tokens):
        tree = remove_parentheses(tree)
        tree = trim_tree(tree)
        tree = attach_arguments_to_parent(tree)
        nltk_tree = tree
        break
    nltk_tree = nltk_tree_to_custom_tree(nltk_tree)
    nltk_tree = trim(nltk_tree)
    if merge:
        nltk_tree.merge_add_multiply()
    #print(nltk_tree.print_algebra())
    return nltk_tree

def trim_the_tree(tree):
    if tree.children:
        if len(tree.children) == 1 and tree.children[0].children:
            return trim_the_tree(tree.children[0])
        else:
            return TreeNode(tree.label, [trim_the_tree(child) for child in tree.children])
    else:
        return tree

"""
def replace_equation(equation, string_list, eq_list):
    coll = TreeNode(equation.label, [])
    tmp = equation.print_algebra()
    if tmp in eq_list:
        return TreeNode("Letter", [TreeNode(chr(ord("a")+string_list.index(tmp)), [])])
    for child in equation.children:
        coll.children.append(replace_equation(child, string_list, eq_list))
    return coll

def remove_same(eq_1, eq_2):
    if eq_1.label != eq_2.label:
        return [eq_1, eq_2]
    for i in range(len(eq_1.children)-1,-1,-1):
        for j in range(len(eq_2.children)-1,-1,-1):
            if eq_1.children[i].print_algebra() == eq_2.children[j].print_algebra():
                eq_1.children.pop(i)
                eq_2.children.pop(j)
                break
    return [eq_1, eq_2]

uset = []
def num_letter(equation):
    global uset
    if equation.label == "Letter":
        if equation.children[0].label not in uset:
            uset.append(equation.children[0].label)
    for child in equation.children:
        num_letter(child)

def dec_letter(equation):
    global uset
    coll = TreeNode(equation.label, [])
    if equation.label == "Letter":
        return TreeNode("Letter", [TreeNode(chr(ord("a")+uset.index(equation.children[0].label)), [])])
    for child in equation.children:
        coll.children.append(dec_letter(child))
    return coll
a = math_parser("((x+1)*(x+2)+4)*3")
b = math_parser("(x*x+x*2+1*x+1*2+4)*3")
aa = sorted(a.component_gen(), key=len, reverse=True)
bb = sorted(b.component_gen(), key=len, reverse=True)
a_b = list(set(aa + bb))
a_b = sorted(a_b, key=len, reverse=True)
c, d= a, b
#print(a_b)
#c = replace_equation(b,a_b,aa)
#d = replace_equation(a,a_b,bb)
while True:
    tc, td = c.print_algebra(), d.print_algebra()
    c, d = remove_same(c, d)
    c = trim_the_tree(c)
    d = trim_the_tree(d)
    if c.print_algebra() == tc and d.print_algebra() == td:
        break
num_letter(c)
num_letter(d)
print(uset)
c = dec_letter(c)
d = dec_letter(d)
print(c)
print(d)
"""
def commutative_struct(count, label):
    def generate_tree_bfs(child_counts):
        root = TreeNode(label, [])
        queue = [root]
        index = 0
        while index < len(child_counts):
            if not queue:
                return None
            current = queue.pop(0)
            total_nodes = len(current.children) + 1
            for i in range(child_counts[index]):
                child = TreeNode(label, [])
                current.children.append(child)
                queue.append(child)
            index += 1
        return root
    def count_leaf_nodes(tree):
        if not tree.children:
            return 1
        else:
            return sum(count_leaf_nodes(child) for child in tree.children)
    def serialize_tree(node):
        if not node:
            return ""
        serialized_children = ",".join(serialize_tree(child) for child in node.children)
        return f"{node.label}({serialized_children})"
    coll = {}
    for i in range(1,count+1):
        for x in itertools.product([0]+list(range(2,count+1)), repeat=i):
            x  = list(x)
            if x[0]==0:
                continue
            tree = generate_tree_bfs(x)
            if tree is None or count_leaf_nodes(tree) != count:
                continue
            coll[serialize_tree(tree)] = tree
    output = []
    for item in coll.keys():
        output.append(coll[item])
    return output
"""
tmp = commutative_struct(3, "n")
for item in tmp:
    print(item)
"""

def commutative_property(equation):
    commute_equation = None
    def commute(structure):
        nonlocal commute_equation
        if not structure.children:
            tmp = commute_equation.children.pop(0)
            return tmp
        coll = TreeNode(structure.label, [])
        for child in structure.children:
            tmp = commute(child)
            coll.children.append(tmp)
        return coll
    eq_list = []
    if equation.label == "Multiply" or equation.label == "Add":
        if len(equation.children)>2:
            structure_list = copy.deepcopy(commutative_struct(len(equation.children), equation.label))
            for structure in structure_list:
                for x in itertools.permutations(equation.children):
                    commute_equation = copy.deepcopy(TreeNode(equation.label, list(x)))
                    eq_list.append(commute(structure))
        else:
            tmp = copy.deepcopy(equation)
            tmp.children[0], tmp.children[1]  = tmp.children[1], tmp.children[0]
            eq_list.append(tmp)
    output = []
    for i in range(len(eq_list)-1):
        success = True
        for j in range(i+1, len(eq_list)):
            if eq_list[i].print_algebra() == eq_list[j].print_algebra():
                success=False
                break
        if success:
            output.append(eq_list[i])
    output.append(eq_list[-1])
    return output

#item_list = commutative_property(math_parser("x*y*z"))
#for item in item_list:
#    print(item.print_algebra())
#print()
list_output = []
def com_search(equation, depth=2):
    global list_output
    list_output.append(equation.print_algebra())
    comu = {}
    counting = 0
    if depth == 0:
        return None
    def many_com(equation):
        nonlocal counting
        nonlocal comu
        counting += 1
        if equation.label == "Multiply" or equation.label == "Add":
            comu[str(counting)] = commutative_property(equation)
        for child in equation.children:
            many_com(child)
    def change(equation, num, replace_obj):
        nonlocal counting
        coll = TreeNode(equation.label, [])
        counting += 1
        if counting == num:
            return replace_obj
        for child in equation.children:
            coll.children.append(change(child, num, replace_obj))
        return coll
    many_com(equation)
    for key in comu.keys():
        for i in range(len(comu[key])):
            counting = 0
            orig = copy.deepcopy(equation)
            equation = change(equation, int(key), comu[key][i])
            com_search(equation, depth-1)
            equation = orig
    return None
#com_search(math_parser("x*y*(w+z)"))
#list_output = list(set(list_output))
#print(list_output)
#print(len(list_output))

def apply_single_formula(equation, formula_input, formula_output):
    var_list = {}
    def formula_given(equation, formula):
        nonlocal var_list
        if formula.label == "Letter":
            if formula.children[0].label in var_list.keys():
                return var_list[formula.children[0].label].print_algebra() == equation.print_algebra()
            else:
                var_list[formula.children[0].label] = equation
                return True
        if equation.label != formula.label or len(equation.children) != len(formula.children):
            return False
        for i in range(len(equation.children)):
            if formula_given(equation.children[i], formula.children[i]) is False:
                return False
        return True

    #print(formula_given(math_parser("(x+1)*(x+1)"), math_parser("a*a")))
    #for key in var_list.keys():
    #    print(var_list[key])

    def formula_apply(formula):
        nonlocal var_list
        if formula.label == "Letter":
            return var_list[formula.children[0].label]
        coll = TreeNode(formula.label, [])
        for child in formula.children:
            coll.children.append(formula_apply(child))
        return coll
    counting = 1
    def formula_recur(equation, formula_input, formula_output):
        nonlocal var_list
        nonlocal counting
        coll = TreeNode(equation.label, [])
        var_list = {}
        if formula_given(equation, copy.deepcopy(formula_input)) is True:
            counting -= 1
            if counting == 0:
                #print(var_list['a'])
                return formula_apply(copy.deepcopy(formula_output))
        if equation.label in ["Integer", "Digit", "Letter"]:
            return equation
        for child in equation.children:
            coll.children.append(formula_recur(child, formula_input, formula_output))
        return coll
    output = []
    for i in range(1, 10):
        counting = i
        orig_len = len(output)
        tmp = formula_recur(equation, formula_input, formula_output)
        print(tmp)
        tmp = tmp.print_algebra()
        print(tmp)
        if tmp != equation.print_algebra():
            output.append(tmp)
        output = list(set(output))
    return output


def same_eq(eq_1, eq_2):
    if eq_1.print_algebra() == eq_2.print_algebra():
        return True
    if len(eq_1.children) != len(eq_2.children) or eq_1.label != eq_2.label:
        return False
    if eq_1.label in ["Multiply", "Add"]:
        for x in itertools.permutations(eq_1.children):
            if all(same_eq(x[i], eq_2.children[i]) for i in range(len(eq_1.children))):
                return True
    else:
        if all(same_eq(eq_1.children[i], eq_2.children[i]) for i in range(len(eq_1.children))):
            return True
    return False
def double_loop(equation, formula_input, formula_output):
    global list_output
    list_output = []
    com_search(math_parser(equation))
    list_output = list(set(list_output))
    parsed_equation = []
    
    for eq in list_output:
        parsed_equation.append(math_parser(eq, False))
    ans = []
    fin = []
    fot = []
    for eq in formula_input:
        fin.append(math_parser(eq))
    for eq in formula_output:
        fot.append(math_parser(eq))
        
    for x in parsed_equation:
        for i in range(len(fin)):
            tmp = apply_single_formula(x, fin[i], fot[i])
            if tmp is not None:
                ans += tmp
    
    ans = list(set(ans))
    #print(ans)
    ans = [math_parser(eq).merge_add_multiply() for eq in ans]
    output = []
    for i in range(len(ans)-1):
        if any(same_eq(ans[i], ans[j]) for j in range(i+1, len(ans))):
            continue
        output.append(ans[i])
    output.append(ans[-1])
    return [eq.print_algebra() for eq in output]
formula_total = \
"""
a a*1
(sin(a)^2)+(cos(a)^2) 1
"""
#print(math_parser("1"))
par1 = [(item.split())[0] for item in formula_total.splitlines()[1:]]
par2 = [(item.split())[1] for item in formula_total.splitlines()[1:]]
print(par1, par2)
tmp = double_loop("(sin(a)^2)+(cos(a)^2)", par1, par2)
for item in tmp:
    print(item)
#print(same_eq(math_parser("((x+y)+1)*(x+2)"), math_parser("(1+(x+y))*(2+x)")))
#print(apply_single_formula(math_parser("(x+1)*(x+1)"), math_parser("a*a"), math_parser("a^2")))
