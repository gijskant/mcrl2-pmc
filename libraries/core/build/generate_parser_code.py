#~ Copyright 2011 Wieger Wesselink.
#~ Distributed under the Boost Software License, Version 1.0.
#~ (See accompanying file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

import re
import string
from optparse import OptionParser
from mcrl2_utility import *
from parse_mcrl2_syntax import *
from path import *

PRODUCTION_MAPPING = '''
  ActDecl lps::action_label_list
  ActFrm action_formulas::action_formula
  Action lps::action
  ActionLabelList lps::action_label
  ActionList lps::action_label_list
  ActionRenameRule
  ActionRenameRuleRHS
  ActionRenameRuleSpec
  ActionRenameSpec
  ActSpec lps::action_label_list
  BagEnumElt data::detail::data_expression_pair
  BagEnumEltList data::detail::data_expression_pair_list
  BesEqnDecl bes::boolean_equation
  BesEqnDeclList bes::boolean_equation_list
  BesEqnSpec bes::boolean_equation_system
  BesExpr bes::boolean_expression
  BesInit bes::boolean_variable
  BesSpec
  BesVar bes::boolean_variable
  CommExpr process::communication_expression
  CommExprList process::communication_expression_list
  CommExprSet process::communication_expression_list
  ConsSpec data::function_symbol_list
  ConstrDecl data::structured_sort_constructor
  ConstrDeclList data::structured_sort_constructor_list
  DataExpr data::data_expression
  DataExprList data::data_expression_list
  DataExprUnit data::data_expression
  DataSpec data::data_specification
  DataValExpr data::data_expression
  Domain data::sort_expression_list
  EqnDecl data::data_equation
  EqnDeclList data::data_equation_list
  EqnSpec data::data_equation
  FixedPointOperator pbes_system::fixpoint_symbol
  GlobVarSpec data::variable_list
  IdDecl data::function_symbol
  IdsDecl data::function_symbol_list
  IdsDeclList data::function_symbol_list
  Init process::process_expression
  MapSpec data::function_symbol_list
  mCRL2Spec lps::specification
  mCRL2SpecElt
  mCRL2SpecEltList
  MultAct
  MultActId lps::action_label_list
  MultActIdList
  MultActIdSet
  PbesEqnDecl
  PbesEqnSpec
  PbesExpr pbes_system::pbes_expression
  PbesInit pbes_system::propositional_variable_instantiation
  PbesSpec pbes_system::pbes<>
  ProcDecl
  ProcExpr process::process_expression
  ProcExprThenElse process::process_expression
  ProcSpec process::process_specification
  ProjDecl data::structured_sort_constructor_argument
  ProjDeclList data::structured_sort_constructor_argument_list
  PropVarDecl pbes_system::propositional_variable
  PropVarInst pbes_system::propositional_variable_instantiation
  RegFrm regular_formulas::regular_formula
  RenExpr process::rename_expression
  RenExprList process::rename_expression_list
  RenExprSet process::rename_expression_list
  SortDecl
  SortExpr data::sort_expression
  SortExprList data::sort_expression_list
  SortSpec
  StateFrm state_formulas::state_formula
  StateVarDecl
  VarDecl data::variable
  VarsDecl data::variable_list
  VarsDeclList data::variable_list
  VarSpec data::variable_list
  WhrExpr data::where_clause
  WhrExprList data::where_clause_list
'''

PRODUCTION_FUNCTION = '''  RETURNTYPE parse_PRODUCTION(const parse_node& node)
  {
BODY
  }

'''

production_return_types = {}
for line in PRODUCTION_MAPPING.splitlines():
    words = line.split()
    if len(words) == 1:
        production_return_types[words[0]] = 'UNKNOWN'
    elif len(words) == 2:
        production_return_types[words[0]] = words[1]

def make_condition(alternative):
    result = []
    words = alternative.split()
    result.append('(node.child_count() == %d)' % len(words))
    for i, word in enumerate(words):
        if word.startswith("'"):
            result.append('(symbol_name(node.child(%d)) == "%s")' % (i, word[1:-1]))
        elif not word[-1] in '*?+':
            result.append('(symbol_name(node.child(%d)) == "%s")' % (i, word))
    if len(result) == 0:
        result.append('true')
    return ' && '.join(result)

def symbol_names(rhs):
    result = []
    for (text, comment, annotation) in rhs:
        words = text.split()
        names = []
        for i, word in enumerate(words):
            if word.startswith("'"):
                names.append(word[1:-1])
            elif not word[-1] in '*?+':
                names.append(word)
            else:
                names.append('??')
        result.append(names)
    return result

#---------------------------------------------------------------#
#                          print_alternative
#---------------------------------------------------------------#
def print_alternative(text, add_condition = False):
    args = []

    words = text.split()
    for i, word in enumerate(words):
        if word.startswith("'"):
            continue
            function = production_return_types[word]
        args.append('parse_%s(node.child(%d))' % (word, i))
    result = 'return UNKNOWN_ALTERNATIVE(' + ', '.join(args) + ');'
    if add_condition:
        result = 'if (' + make_condition(text) + ') { ' + result + ' }'
    return result

#---------------------------------------------------------------#
#                          print_production
#---------------------------------------------------------------#
def print_production(lhs, rhs):
    if not lhs in production_return_types:
        production_return_types[lhs] = 'UNKNOWN'

    text = PRODUCTION_FUNCTION
    text = re.sub('RETURNTYPE', production_return_types[lhs], text)
    text = re.sub('PRODUCTION', lhs, text)

    #print 'lhs = ', lhs
    #sn = symbol_names(rhs)
    #for s in sn:
    #    print ', '.join(s)

    if lhs.endswith('List'):
        body = '    return parse_list<%s>("%s");' % (production_return_types[lhs], lhs[:-4])
    else:
        add_condition = len(rhs) > 1
        alternatives = [print_alternative(t, add_condition) for (t, comment, annotation) in rhs]
        if len(alternatives) == 1:
            body = '    ' + alternatives[0]
        else:
            body = '    ' + '\n    else '.join(alternatives) + ('\n    report_unknown_node(node);\n    return %s();' % production_return_types[lhs])

    text = re.sub('BODY', body, text)
    print text

#---------------------------------------------------------------#
#                          print_section
#---------------------------------------------------------------#
def print_section(title, productions):
    print '  //', title, '\n'
    for (lhs, rhs) in productions:
        print_production(lhs, rhs)

#---------------------------------------------------------------#
#                          post_process_sections
#---------------------------------------------------------------#
# Generates a new production T ::= T1 | ... | Tn for the pattern (T1 | ... | Tn)
# Generates new productions TList ::= T* for patterns T* and T+ (if TList does not yet exist)
def post_process_sections(sections):

    # make a mapping of all production left hand sides
    production_names = {}
    for (title, productions) in sections:
        for (lhs, rhs) in productions:
            production_names[lhs] = True

    for i, (title, productions) in enumerate(sections):
        new_productions = {}
        for j, (lhs, rhs) in enumerate(productions):
            if lhs.endswith('List'):
                continue

            for k, (text, comment, annotation) in enumerate(rhs):

                # Generate a new production T' ::= T for the pattern (T)
                regexp = "(?<!')\((([^)]|('\)))*[^'])\)"
                m = re.search(regexp, text)
                while m != None:
                    index = 1
                    new_lhs = lhs + 'Alternative' + str(index)
                    while new_lhs in new_productions:
                        index = index + 1
                        new_lhs = lhs + 'Alternative' + str(index)
                    text = re.sub(regexp, new_lhs, text, 1)
                    new_rhs = map(string.strip, re.split(r'\s*\|\s*', m.group(1)))
                    new_rhs = [(r, '', '') for r in new_rhs]
                    new_productions[new_lhs] = (new_lhs, new_rhs)
                    m = re.search(regexp, text)

                # Generates new productions TList ::= T* for patterns T* and T+ (if TList does not yet exist)
                regexp = r'\b((\w+)\s*[*+])'
                m = re.search(regexp, text)
                while m != None:
                    new_lhs = m.group(2) + 'List'
                    text = re.sub(regexp, new_lhs, text, 1)
                    if not new_lhs in production_names:
                        production_names[new_lhs] = True
                        new_rhs = [(m.group(2) + '*', '', '')]
                        new_productions[new_lhs] = (new_lhs, new_rhs)
                    m = re.search(regexp, text)

                rhs[k] = (text, comment, annotation)

            productions[j] = (lhs, rhs)

        sections[i] = (title, productions + new_productions.values())
    return sections

#---------------------------------------------------------------#
#                          my_print
#---------------------------------------------------------------#
def my_print(sections):
    for (title, productions) in sections:
        for (lhs, rhs) in productions:
            for (text, comment, annotation) in rhs:
                print lhs, '->', text

#---------------------------------------------------------------#
#                          main
#---------------------------------------------------------------#
def main():
    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    (options, args) = parser.parse_args()

    filename = '../../../doc/specs/mcrl2-syntax.g'
    sections = parse_mcrl2_syntax(filename)
    sections = post_process_sections(sections)
    for (title, productions) in sections:
        print_section(title, productions)

if __name__ == "__main__":
    main()
