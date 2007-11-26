#~ Copyright 2007 Wieger Wesselink.
#~ Distributed under the Boost Software License, Version 1.0.
#~ (See accompanying file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

import os
import re
from optparse import OptionParser
from mcrl2_parser import *

#--------------------------------------------------------#
#                insert_text_in_file
#--------------------------------------------------------#
# search for the lines equal to 'beginline' and 'endline'
# in filename, and replace the lines between them with 'text'
#
def insert_text_in_file(text, filename, beginline = '<!-- begin -->', endline = '<!-- end -->'):
    tmpfile = filename + '.tmp'
    try:
        infile  = open(filename)
        outfile = open(tmpfile, "w")
    except IOError, e:
        print "Unable to open file ", filename, " ", e
        return

    lines = infile.readlines()

    for i in range(len(lines)):
        outfile.write(lines[i])
        if string.rstrip(lines[i]) == beginline:
            break

    first = i+1
    for i in range(first, len(lines)):
        if string.rstrip(lines[i]) == endline:
            outfile.write(text + '\n')
            outfile.write(lines[i])
            break

    if i < len(lines)-1:
        first = i+1

    for i in range(first, len(lines)):
        outfile.write(lines[i])

    infile.close()
    outfile.close()
    os.remove(filename)
    os.rename(tmpfile, filename)

LIBSTRUCT_SYMBOL_FUNCTIONS = '''// %(name)s
inline
AFun initAFun%(name)s(AFun& f)
{
  f = ATmakeAFun("%(name)s", %(arity)d, ATfalse);
  ATprotectAFun(f);
  return f;
}

inline
AFun gsAFun%(name)s()
{
  static AFun AFun%(name)s = initAFun%(name)s(AFun%(name)s);
  return AFun%(name)s;
}

inline
bool gsIs%(name)s(ATermAppl Term)
{
  return ATgetAFun(Term) == gsAFun%(name)s();
}

'''

LIBSTRUCT_MAKE_FUNCTION = '''inline
ATermAppl gsMake%(name)s(%(parameters)s)
{
  return ATmakeAppl%(arity)d(gsAFun%(name)s()%(arguments)s);
}

'''

#---------------------------------------------------------------#
#                      generate_libstruct_functions
#---------------------------------------------------------------#
# generates C++ code for libstruct functions
#
def generate_libstruct_functions(rules, filename, ignored_phases = []):
    begin = '//--- begin generated code'
    end   = '//--- end generated code'
    names = {}
    calls = {}
    decls = {}

    functions = find_functions(rules, ignored_phases)

    for f in functions:
        name = f.name()
        names[name] = f.arity()
        calls[name] = f.default_call()
        decls[name] = f.default_declaration()

    text = ''
    mtext = '' # gsMake functions
    
    name_keys = names.keys()
    name_keys.sort()
    for name in name_keys:
        arity = names[name]
        text = text + LIBSTRUCT_SYMBOL_FUNCTIONS % {
            'name'  : name,
            'name'  : name,
            'name'  : name,
            'arity' : arity,
            'name'  : name,
            'name'  : name,
            'name'  : name,
            'name'  : name,
            'name'  : name,
            'name'  : name,
            'name'  : name
        }
        comma = ''
        if calls[name] != "":
            comma = ', '
        mtext = mtext + LIBSTRUCT_MAKE_FUNCTION % {
            'name'       : name,
            'arity'      : arity,
            'parameters' : decls[name],
            'arguments'  : comma + calls[name]
        } 
    insert_text_in_file(string.strip(text + mtext), filename, begin, end)

CHECK_RULE = '''template <typename Term>
bool check_rule_%(name)s(Term t)
{
%(body)s
}

'''

CHECK_TERM = '''// %(name)s(%(arguments)s)
template <typename Term>
bool %(check_name)s(Term t)
{
%(body)s
  return true;
}

'''
CHECK_TERM_TYPE = '''  // check the type of the term
  aterm term(aterm_traits<Term>::term(t));
  if (term.type() != AT_APPL)
    return false;
  aterm_appl a(term);
  if (!gsIs%(name)s(a))
    return false;

'''

CHECK_TERM_CHILDREN = '''  // check the children
  if (a.size() != %(arity)d)
    return false;
'''

#---------------------------------------------------------------#
#                      generate_soundness_check_functions
#---------------------------------------------------------------#
# generates C++ code for checking if terms are in the right format
#
def generate_soundness_check_functions(rules, filename, ignored_phases = []):
    begin = '//--- begin generated code'
    end   = '//--- end generated code'
    text  = '' # function definitions
    ptext = '' # function declarations (prototypes)

    functions = find_functions(rules, ignored_phases)

    for rule in rules:
        name = rule.name()
        rhs_functions = rule.functions(ignored_phases)
        body = '  return    ' + '\n         || '.join(map(lambda x: x.check_name() + '(t)', rhs_functions)) + ';'
        text = text + CHECK_RULE % {
            'name'      : name,
            'body'      : body
        }
        ptext = ptext + 'template <typename Term> bool check_rule_%s(Term t);\n' % rule.name()
    
    for f in functions:
        arguments = ', '.join(map(lambda x: x.full_name(), f.arguments))
        name = f.name()
        arity = len(f.arguments)

        body = CHECK_TERM_TYPE % {
            'name' : name
        }
        body = body + CHECK_TERM_CHILDREN % {
            'arity' : len(f.arguments)
        }
        if arity > 0:
            body = body + '#ifndef LPS_NO_RECURSIVE_SOUNDNESS_CHECKS\n'
            for i in range(arity):
                arg = f.arguments[i]
                if arg.repetitions == '':
                    body = body + '  if (!check_term_argument(a(%d), %s<aterm>))\n'    % (i, arg.check_name())
                elif arg.repetitions == '*':
                    body = body + '  if (!check_list_argument(a(%d), %s<aterm>, 0))\n' % (i, arg.check_name())
                elif arg.repetitions == '+':
                    body = body + '  if (!check_list_argument(a(%d), %s<aterm>, 1))\n' % (i, arg.check_name())
                body = body + '    {\n'
                body = body + '      std::cerr << "%s" << std::endl;\n'                % (arg.check_name())
#                body = body + '      std::cerr << a(%d).to_string() << std::endl;\n'   % (i)
                body = body + '      return false;\n'
                body = body + '    }\n'
            body = body + '#endif // LPS_NO_RECURSIVE_SOUNDNESS_CHECKS\n'

        text = text + CHECK_TERM % {
            'name'       : name,
            'arguments'  : arguments,
            'check_name' : f.check_name(),
            'body'       : body
        }
        ptext = ptext + 'template <typename Term> bool %s(Term t);\n' % f.check_name()

    insert_text_in_file(string.strip(ptext + '\n' + text), filename, begin, end)

CONSTRUCTOR_FUNCTIONS = '''// %(name)s
inline
ATermAppl initConstruct%(name)s(ATermAppl& t)
{
  t = ATmakeAppl%(arity)d(gsAFun%(name)s()%(arguments)s);
  ATprotect(reinterpret_cast<ATerm*>(&t));
  return t;
}

inline
ATermAppl construct%(name)s()
{
  static ATermAppl t = initConstruct%(name)s(t);
  return t;
}

'''

CONSTRUCTOR_RULE = '''// %(name)s
inline
ATermAppl construct%(name)s()
{
  return construct%(fname)s();
}

'''

#---------------------------------------------------------------#
#                      generate_constructor_functions
#---------------------------------------------------------------#
# generates C++ code for constructor functions
#
def generate_constructor_functions(rules, filename, ignored_phases = []):
    begin = '//--- begin generated code'
    end   = '//--- end generated code'
    text  = ''
    ptext = '' # function declarations (prototypes)

    functions = find_functions(rules, ignored_phases)

    for f in functions:
        ptext = ptext + 'ATermAppl construct%s();\n' % f.name()
        name  = f.name()
        arity = f.arity()
#        args = map(lambda x: 'reinterpret_cast<ATerm>(construct%s())' % x.name() if x.repetitions == '' else 'reinterpret_cast<ATerm>(constructList())', f.arguments)
        args = []
        for x in f.arguments:
            if x.repetitions == '':
                args.append('reinterpret_cast<ATerm>(construct%s())' % x.name())
            else:
                args.append('reinterpret_cast<ATerm>(constructList())')

#        arguments = ', ' + ', '.join(args) if len(args) > 0 else ''
        if len(args) > 0:
            arguments = ', ' + ', '.join(args) 
        else:
            arguments = ''
        text = text + CONSTRUCTOR_FUNCTIONS % {
            'name'       : name,
            'name'       : name,
            'arity'      : arity,
            'name'       : name,
            'arguments'  : arguments,
            'name'       : name,
            'name'       : name
        }

    ctext = ''
    function_names = map(lambda x: x.name(), functions)
    for rule in rules:
        if not rule.name() in function_names:
            name = rule.name()
            for f in rule.rhs:
                if f.phase == None or not f.phase.startswith('-') or not f.phase.startswith('.'):
                    fname = f.name()
                    break
            ptext = ptext + 'ATermAppl construct%s();\n' % name             
            text = text + CONSTRUCTOR_RULE % {
                'name'       : name,
                'name'       : name,
                'fname'      : fname
            }

    insert_text_in_file(string.strip(ptext + '\n' + text), filename, begin, end)

#---------------------------------------------------------------#
#                          find_functions
#---------------------------------------------------------------#
# find all functions that appear in the rhs of a rule whose phase is not in ignored_phases
def find_functions(rules, ignored_phases):
    function_map = {}
    for rule in rules:
        for f in rule.functions(ignored_phases):
            if not f.is_rule():
                function_map[f.name()] = f
    
    # do a recursion step to find additional functions (no longer necessary?)
    functions = map(lambda x: function_map[x], function_map.keys())
    for f in functions:
        for arg in f.arguments:
            for e in arg.expressions:
                if not e.is_rule():
                    function_map[e.name()] = e

    return map(lambda x: function_map[x], function_map.keys())

#---------------------------------------------------------------#
#                          parse_ebnf
#---------------------------------------------------------------#
def parse_ebnf(filename):
    rules = []
    nonterminals = []
    nonterminal_names = {}

    paragraphs = read_paragraphs(filename)
    for paragraph in paragraphs:
        #--- skip special paragraphs
        if re.match('// Date', paragraph):
            continue
        if re.match('//Specification', paragraph):
            continue
        if re.match('//Expressions', paragraph):
            continue

        #--- handle other paragraphs
        lines  = string.split(paragraph, '\n')
        clines = [] # comment lines
        glines = [] # grammar lines
        for line in lines:
            if re.match('\s*//.*', line):
                clines.append(line)
            else:
                glines.append(line)
        comment = string.join(clines, '\n')
        
        parser = EBNFParser(Mcrl2Actions())
        try:
            newrules = parser(string.join(glines, '\n'))
            for rule in newrules:
                rule.comment = comment
            rules = rules + newrules
        except tpg.SyntacticError, e:
            print "------------------------------------------------------"
            print 'grammar: ', string.join(glines, '\n')
            print e
        except tpg.LexicalError, e:
            print "------------------------------------------------------"
            print 'grammar: ', string.join(glines, '\n')
            print e
    return rules

#---------------------------------------------------------------#
#                          main
#---------------------------------------------------------------#
def main():
    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option("-s", "--soundness-checks", action="store_true", help="generate soundness check functions from internal mcrl2 format")
    parser.add_option("-l", "--libstruct", action="store_true", help="generate libstruct functions from internal mcrl2 format")
    parser.add_option("-c", "--constructors", action="store_true", help="generate constructor functions from internal mcrl2 format")
    (options, args) = parser.parse_args()

    filename = '../../../doc/specs/mcrl2.internal.txt'
    rules = parse_ebnf(filename)

    if options.soundness_checks:
        ignored_phases = ['-tc', '-lin', '-di', '-rft', '.tc']
        filename = '../include/mcrl2/core/detail/soundness_checks.h'
        generate_soundness_check_functions(rules, filename, ignored_phases)

    if options.libstruct:
        ignored_phases = []
        filename = '../include/mcrl2/core/detail/libstruct_core.h'
        generate_libstruct_functions(rules, filename, ignored_phases)

    if options.constructors:
        ignored_phases = []
        filename = '../include/mcrl2/basic/detail/constructors.h'
        generate_constructor_functions(rules, filename, ignored_phases)

if __name__ == "__main__":
    main()
