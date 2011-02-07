#~ Copyright 2011 Wieger Wesselink.
#~ Distributed under the Boost Software License, Version 1.0.
#~ (See accompanying file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

import os
import re
from path import *
from mcrl2_utility import *

SUBSTITUTE_FUNCTION_TEXT = '''template <typename T, typename Substitution>
  void substitute_variables(T& x,
                            Substitution sigma,
                            typename boost::disable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                           )
  {
    core::make_update_apply_builder<NAMESPACE::data_expression_builder>(sigma)(x);
  }

  template <typename T, typename Substitution>
  T substitute_variables(const T& x,
                         Substitution sigma,
                         typename boost::enable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                        )
  {   
    return core::make_update_apply_builder<NAMESPACE::data_expression_builder>(sigma)(x);
  }

  template <typename T, typename Substitution>
  void substitute_free_variables(T& x,
                                 Substitution sigma,
                                 typename boost::disable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                                )
  {
    data::detail::make_substitute_free_variables_builder<NAMESPACE::data_expression_builder, NAMESPACE::add_data_variable_binding>(sigma)(x);
  }

  template <typename T, typename Substitution>
  T substitute_free_variables(const T& x,
                              Substitution sigma,
                              typename boost::enable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                             )
  {
    return data::detail::make_substitute_free_variables_builder<NAMESPACE::data_expression_builder, NAMESPACE::add_data_variable_binding>(sigma)(x);
  }

  template <typename T, typename Substitution, typename VariableContainer>
  void substitute_free_variables(T& x,
                                 Substitution sigma,
                                 const VariableContainer& bound_variables,
                                 typename boost::disable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                                )
  {
    data::detail::make_substitute_free_variables_builder<NAMESPACE::data_expression_builder, NAMESPACE::add_data_variable_binding>(sigma)(x, bound_variables);
  }

  template <typename T, typename Substitution, typename VariableContainer>
  T substitute_free_variables(const T& x,
                              Substitution sigma,
                              const VariableContainer& bound_variables,
                              typename boost::enable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                             )
  {
    return data::detail::make_substitute_free_variables_builder<NAMESPACE::data_expression_builder, NAMESPACE::add_data_variable_binding>(sigma)(x, bound_variables);
  }
'''

FIND_VARIABLES_FUNCTION_TEXT = '''  /// \\\\brief Returns all variables that occur in an object
  /// \param[in] x an object containing variables
  /// \param[in,out] o an output iterator to which all variables occurring in x are written.
  /// \\\\return All variables that occur in the term x
  template <typename T, typename OutputIterator>
  void find_variables(const T& x, OutputIterator o)
  {
    data::detail::make_find_variables_traverser<NAMESPACE::traverser>(o)(x);
  }

  /// \\\\brief Returns all variables that occur in an object
  /// \param[in] x an object containing variables
  /// \\\\return All variables that occur in the object x
  template <typename T>
  std::set<data::variable> find_variables(const T& x)
  {
    std::set<data::variable> result;
    NAMESPACE::find_variables(x, std::inserter(result, result.end()));
    return result;
  }

  /// \\\\brief Returns all variables that occur in an object
  /// \param[in] x an object containing variables
  /// \param[in,out] o an output iterator to which all variables occurring in x are added.
  /// \\\\return All free variables that occur in the object x
  template <typename T, typename OutputIterator>
  void find_free_variables(const T& x, OutputIterator o, typename atermpp::detail::disable_if_container<OutputIterator>::type* = 0)
  {
    data::detail::make_find_free_variables_traverser<NAMESPACE::variable_traverser, NAMESPACE::add_data_variable_binding>(o)(x);
  }

  /// \\\\brief Returns all variables that occur in an object
  /// \param[in] x an object containing variables
  /// \param[in,out] o an output iterator to which all variables occurring in x are written.
  /// \param[in] bound a container of variables
  /// \\\\return All free variables that occur in the object x
  template <typename T, typename OutputIterator, typename VariableContainer>
  void find_free_variables(const T& x, OutputIterator o, const VariableContainer& bound)
  {
    data::detail::make_find_free_variables_traverser<NAMESPACE::variable_traverser, NAMESPACE::add_data_variable_binding>(o, bound)(x);
  }

  /// \\\\brief Returns all variables that occur in an object
  /// \param[in] x an object containing variables
  /// \\\\return All free variables that occur in the object x
  template <typename T>
  std::set<data::variable> find_free_variables(const T& x)
  {
    std::set<data::variable> result;
    NAMESPACE::find_free_variables(x, std::inserter(result, result.end()));
    return result;
  }

  /// \\\\brief Returns all variables that occur in an object
  /// \param[in] x an object containing variables
  /// \param[in] bound a bound a container of variables
  /// \\\\return All free variables that occur in the object x
  template <typename T, typename VariableContainer>
  std::set<data::variable> find_free_variables(const T& x, VariableContainer const& bound, typename atermpp::detail::enable_if_container<VariableContainer, data::variable>::type* = 0)
  {
    std::set<data::variable> result;
    NAMESPACE::find_free_variables(x, std::inserter(result, result.end()), bound);
    return result;
  }
'''

def generate_code(filename, namespace, label, text):
    text = re.sub('NAMESPACE', namespace, text)
    insert_text_in_file(filename, text, 'generated %s %s code' % (namespace, label))
    print_labels(namespace, label)   

def print_labels(namespace, label):
    print '//--- start generated %s %s code ---//' % (namespace, label)
    print '//--- end generated %s %s code ---//' % (namespace, label)

def generate_substitute_functions():
    generate_code('../../lps/include/mcrl2/lps/substitute.h'          , 'lps'             , 'replace', SUBSTITUTE_FUNCTION_TEXT)
    generate_code('../../lps/include/mcrl2/modal_formula/substitute.h', 'action_formulas' , 'replace', SUBSTITUTE_FUNCTION_TEXT)
    generate_code('../../lps/include/mcrl2/modal_formula/substitute.h', 'regular_formulas', 'replace', SUBSTITUTE_FUNCTION_TEXT)
    generate_code('../../lps/include/mcrl2/modal_formula/substitute.h', 'state_formulas'  , 'replace', SUBSTITUTE_FUNCTION_TEXT)
    generate_code('../../pbes/include/mcrl2/pbes/substitute.h'        , 'pbes_system'     , 'replace', SUBSTITUTE_FUNCTION_TEXT)
    generate_code('../../process/include/mcrl2/process/substitute.h'  , 'process'         , 'replace', SUBSTITUTE_FUNCTION_TEXT)

def generate_find_variable_functions():
    generate_code('../../lps/include/mcrl2/lps/find.h'          , 'lps'             , 'find', FIND_VARIABLES_FUNCTION_TEXT)
    generate_code('../../lps/include/mcrl2/modal_formula/find.h', 'action_formulas' , 'find', FIND_VARIABLES_FUNCTION_TEXT)
    generate_code('../../lps/include/mcrl2/modal_formula/find.h', 'regular_formulas', 'find', FIND_VARIABLES_FUNCTION_TEXT)
    generate_code('../../lps/include/mcrl2/modal_formula/find.h', 'state_formulas'  , 'find', FIND_VARIABLES_FUNCTION_TEXT)
    generate_code('../../pbes/include/mcrl2/pbes/find.h'        , 'pbes_system'     , 'find', FIND_VARIABLES_FUNCTION_TEXT)
    generate_code('../../process/include/mcrl2/process/find.h'  , 'process'         , 'find', FIND_VARIABLES_FUNCTION_TEXT)

if __name__ == "__main__":
    #generate_substitute_functions()
    generate_find_variable_functions()
