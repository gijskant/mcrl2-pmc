#~ Copyright 2009, 2010 Wieger Wesselink.
#~ Distributed under the Boost Software License, Version 1.0.
#~ (See accompanying file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

import re
from path import *
from mcrl2_classes import *
from mcrl2_utility import *

EXPRESSION_VISITOR_CODE = r'''/// \\brief Visitor class for MYEXPRESSIONs.
///
/// There is a visit_<node> and a leave_<node>
/// function for each type of node. By default these functions do nothing, so they
/// must be overridden to add behavior. If the visit_<node> function returns true,
/// the recursion is continued in the children of the node.
template <typename Arg=void>
struct MYEXPRESSION_visitor
{
  /// \\brief The type of the additional argument for the recursion
  typedef Arg argument_type;

  /// \\brief These names can be used as return types of the visit functions, to make
  /// the code more readible.
  enum return_type
  {
    stop_recursion = false,
    continue_recursion = true
  };

  /// \\brief Destructor.
  virtual ~MYEXPRESSION_visitor()
  { }
%s
  /// \\brief Visits the nodes of the pbes expression, and calls the corresponding visit_<node>
  /// member functions. If the return value of a member function equals false, then the
  /// recursion in this node is stopped.
  /// \param x A term
  /// \param a An additional argument for the recursion
  void visit(const MYEXPRESSION& x, Arg& a)
  {
%s
  }
};

/// \\brief Visitor class for expressions.
///
/// If a visit_<node> function returns true, the recursion is continued
/// in the children of this node, otherwise not.
template <>
struct MYEXPRESSION_visitor<void>
{
  /// \\brief The type of the additional argument for the recursion
  typedef void argument_type;

  /// \\brief These names can be used as return types of the visit functions, to make
  /// the code more readible.
  enum return_type
  {
    stop_recursion = false,
    continue_recursion = true
  };

  /// \\brief Destructor.
  virtual ~MYEXPRESSION_visitor()
  { }
%s

  /// \\brief Visits the nodes of the expression and calls the corresponding visit_<node>
  /// member functions. If the return value of a member function equals false, then the
  /// recursion in this node is stopped.
  /// \param x A term
  void visit(const MYEXPRESSION& x)
  {
%s
  }
};
'''

EXPRESSION_BUILDER_CODE = r'''/// \\brief Modifying visitor class for expressions.
///
/// During traversal
/// of the nodes, the expression is rebuilt from scratch.
/// If a visit_<node> function returns MYEXPRESSION(), the recursion is continued
/// in the children of this node, otherwise not.
/// An arbitrary additional argument may be passed during the recursion.
template <typename Arg = void>
struct MYEXPRESSION_builder
{
  /// \\brief The type of the additional argument for the recursion
  typedef Arg argument_type;

  /// \\brief Returns true if the expression is not equal to MYEXPRESSION().
  /// This is used to determine if the recursion in a node needs to be continued.
  /// \\param x A expression
  /// \\return True if the term is not equal to MYEXPRESSION()
  bool is_finished(const MYEXPRESSION& x)
  {
    return x != MYEXPRESSION();
  }

  /// \\brief Destructor.
  virtual ~MYEXPRESSION_builder()
  { }
%s
  /// \\brief Visits the nodes of the expression, and calls the corresponding visit_<node>
  /// member functions. If the return value of a visit function equals MYEXPRESSION(),
  /// the recursion in this node is continued automatically, otherwise the returned
  /// value is used for rebuilding the expression.
  /// \\param x A expression
  /// \\param a An additional argument for the recursion
  /// \\return The visit result
  MYEXPRESSION visit(const MYEXPRESSION& x, Arg& a)
  {
#ifdef MCRL2_UPPERCASE_EXPRESSION_BUILDER_DEBUG
  std::cerr << "<visit>" << pp(x) << std::endl;
#endif
    MYEXPRESSION result;
%s
#ifdef MCRL2_UPPERCASE_EXPRESSION_BUILDER_DEBUG
  std::cerr << "<visit result>" << pp(result) << std::endl;
#endif
    return result;
  }
};

/// \\brief Modifying visitor class for expressions.
///
/// If a visit_<node> function returns true, the recursion is continued
/// in the children of this node, otherwise not.
template <>
struct MYEXPRESSION_builder<void>
{
  /// \\brief The type of the additional argument for the recursion
  typedef void argument_type;

  /// \\brief Returns true if the expression is not equal to MYEXPRESSION().
  /// This is used to determine if the recursion in a node needs to be continued.
  /// \\param x A expression
  /// \\return True if the term is not equal to MYEXPRESSION()
  bool is_finished(const MYEXPRESSION& x)
  {
    return x != MYEXPRESSION();
  }

  /// \\brief Destructor.
  virtual ~MYEXPRESSION_builder()
  { }
%s

  /// \\brief Visits the nodes of the expression and calls the corresponding visit_<node>
  /// member functions. If the return value of a member function equals false, then the
  /// recursion in this node is stopped.
  /// \param x A term
  MYEXPRESSION visit(const MYEXPRESSION& x)
  {
#ifdef MCRL2_UPPERCASE_EXPRESSION_BUILDER_DEBUG
  std::cerr << "<visit>" << pp(x) << std::endl;
#endif
    MYEXPRESSION result;
%s
#ifdef MCRL2_UPPERCASE_EXPRESSION_BUILDER_DEBUG
  std::cerr << "<visit result>" << pp(result) << std::endl;
#endif
    return result;
  }
};
'''

EXPRESSION_VISITOR_NODE_TEXT = r'''
  /// \\brief Visit NODE node
  /// \\return The result of visiting the node
  virtual bool visit_NODE(const QUALIFIED_NODE& /* x */ EXTRA_ARG)
  {
    return continue_recursion;
  }

  /// \\brief Leave NODE node
  virtual void leave_NODE()
  {}
'''

EXPRESSION_BUILDER_NODE_TEXT = r'''              
  /// \\brief Visit NODE node
  /// \\return The result of visiting the node
  virtual MYEXPRESSION visit_NODE(const QUALIFIED_NODE& /* x */ EXTRA_ARG)
  {
    return MYEXPRESSION();
  }
'''

def make_expression_visitor(filename, expression, text):
    classes = parse_classes(text)
    vtext = ''
    wtext = ''
    else_text = ''

    for c in classes:
        #(aterm, constructor, description) = c
        f = c.constructor

        node = f.name()
        qualified_node = f.qualified_name()
        print 'generating visit function for class', node
        types = [p.type() for p in f.parameters()]
        names = [p.name() for p in f.parameters()]
        arguments = f.argument_text()

        text = EXPRESSION_VISITOR_NODE_TEXT
        text = re.sub('QUALIFIED_NODE', qualified_node, text)
        text = re.sub('NODE', node, text)
        vtext = vtext + text
    
        #--- generate code fragments like this
        #
        #    if (is_imp(e))
        #    {
        #      term_type l = left(e);
        #      term_type r = right(e);
        #      bool result = visit_imp(e, l, r);
        #      if (result) {
        #        visit(l);
        #        visit(r);
        #      }
        #      leave_imp();
        #    }
        text = ''
        text = text + '%sif (is_%s(x))\n' % (else_text, node)
        if else_text == '':
            else_text = 'else '
        text = text + '{\n'
        has_children = expression in map(extract_type, types)
        args = ', '.join(names)
        if args != '':
            args = ', ' + args
        rtext = ''
        if has_children:
            rtext = 'bool result = '
        text = text + '  %svisit_%s(%s(x)EXTRA_ARG);\n' % (rtext, node, node)
        if has_children:
            text = text + '  if (result) {\n'
            for i in range(len(types)):
                if extract_type(types[i]) == expression:
                    text = text + '    visit(%s(x).%s()EXTRA_ARG);\n' % (node, names[i])
            text = text + '  }\n'
        text = text + '  leave_%s();\n' % node
        text = text + '}\n'
        wtext = wtext + text
    wtext = indent_text(wtext, '    ')
    
    vtext1 = re.sub('EXTRA_ARG', '', vtext)
    vtext2 = re.sub('EXTRA_ARG', ', Arg& /* a */', vtext)
    wtext1 = re.sub('EXTRA_ARG', '', wtext)
    wtext2 = re.sub('EXTRA_ARG', ', a', wtext)
    
    rtext = EXPRESSION_VISITOR_CODE % (vtext2, wtext2, vtext1, wtext1)
    rtext = re.sub('MYEXPRESSION', expression, rtext)
    insert_text_in_file(filename, rtext, 'generated visitor')

def make_expression_builder(filename, expression, text):
    classes = parse_classes(text)
    vtext = ''
    wtext = ''
    else_text = ''

    for c in classes:
        #(aterm, constructor, description) = c
        f = c.constructor

        node = f.name()
        qualified_node = f.qualified_name()
        types = [p.type() for p in f.parameters()]
        names = [p.name() for p in f.parameters()]
        arguments = f.argument_text()

        text = EXPRESSION_BUILDER_NODE_TEXT
        text = re.sub('MYEXPRESSION', expression, text)
        text = re.sub('QUALIFIED_NODE', qualified_node, text)
        text = re.sub('NODE', re.sub('_$', '', node), text)
        vtext = vtext + text
    
        #--- generate code fragments like this
        #
        #    if (is_and(x))
        #    {
        #      process_expression l = left(x);
        #      process_expression r = right(x);
        #      result = visit_and(x, l, r, a);
        #      if (!is_finished(result)) {
        #        result = core::optimized_and(visit(l, a), visit(r, a));
        #      }
        #    }  
        text = ''
        text = text + '%sif (is_%s(x))\n' % (else_text, re.sub('_$', '', node))
        if else_text == '':
            else_text = 'else '
        text = text + '{\n'
        text = text + '  result = visit_%s(xEXTRA_ARG);\n' % (re.sub('_$', '', node))
        text = text + '  if (!is_finished(result))\n'
        text = text + '  {\n'
        stext = ''
        for i in range(len(types)):
            if stext != '':
                stext = stext + ', '
            fcall = '%s(atermpp::aterm_appl(x)).%s()' % (node, names[i])
            if extract_type(types[i]) == expression:
                stext = stext + 'visit(%sEXTRA_ARG)' % fcall
            else:
                stext = stext + fcall
        text = text + '    result = %s(%s);\n' % (node, stext)
        text = text + '  }\n'
        text = text + '}\n'
        wtext = wtext + text
    wtext = indent_text(wtext, '    ')
    
    vtext1 = re.sub('EXTRA_ARG', '', vtext)
    vtext2 = re.sub('EXTRA_ARG', ', Arg& /* a */', vtext)
    wtext1 = re.sub('EXTRA_ARG', '', wtext)
    wtext2 = re.sub('EXTRA_ARG', ', a', wtext)
    
    rtext = EXPRESSION_BUILDER_CODE % (vtext2, wtext2, vtext1, wtext1)
    rtext = re.sub('MYEXPRESSION', expression, rtext)
    rtext = re.sub('UPPERCASE_EXPRESSION', expression.upper(), rtext)
    insert_text_in_file(filename, rtext, 'generated visitor')

def make_is_functions(filename, expression, text):
    TERM_TRAITS_TEXT = r'''
    /// \\brief Test for a %s expression
    /// \\param t A term
    /// \\return True if it is a %s expression
    inline
    bool is_%s(const %s& t)
    {
      return core::detail::gsIs%s(t);
    }
'''

    rtext = ''
    classes = parse_classes(text)
    for c in classes:
        #(aterm, constructor, description) = c
        f = c.constructor
        name = f.name()
        rtext = rtext + TERM_TRAITS_TEXT % (name, name, re.sub('_$', '', name), expression, c.aterm)
    insert_text_in_file(filename, rtext, 'generated is-functions')

if __name__ == "__main__":
    make_expression_visitor('../../process/include/mcrl2/process/process_expression_visitor.h', 'process_expression', PROCESS_EXPRESSION_CLASSES)
    make_expression_builder('../../process/include/mcrl2/process/process_expression_builder.h', 'process_expression', PROCESS_EXPRESSION_CLASSES)
    make_is_functions(      '../../process/include/mcrl2/process/process_expression.h', 'process_expression', PROCESS_EXPRESSION_CLASSES)
    make_expression_visitor('../../lps/include/mcrl2/modal_formula/state_formula_visitor.h', 'state_formula', STATE_FORMULA_CLASSES)
    make_expression_builder('../../lps/include/mcrl2/modal_formula/state_formula_builder.h', 'state_formula', STATE_FORMULA_CLASSES)
    make_is_functions(      '../../lps/include/mcrl2/modal_formula/state_formula.h', 'state_formula', STATE_FORMULA_CLASSES)
    # N.B. This doesn't work, since the pbes expression visitors need to be patched for the value true
    # make_expression_visitor('../../pbes/include/mcrl2/pbes/pbes_expression_visitor.h', 'pbes_expression', PBES_EXPRESSION_CLASSES)
    # make_expression_builder('../../pbes/include/mcrl2/pbes/pbes_expression_builder.h', 'pbes_expression', PBES_EXPRESSION_CLASSES)
    # make_is_functions('../../pbes/include/mcrl2/pbes/pbes_expression.h', PBES_EXPRESSION_CLASSES)
