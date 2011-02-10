#~ Copyright 2009, 2010 Wieger Wesselink.
#~ Distributed under the Boost Software License, Version 1.0.
#~ (See accompanying file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

import os
import re
from path import *
from mcrl2_classes import *
from mcrl2_utility import *

def compare_classes(x, y):
    if 'X' in x.modifiers() and 'X' in y.modifiers():
        return cmp(x.index, y.index)
    return cmp('X' in x.modifiers(), 'X' in y.modifiers())

def make_traverser(filename, traverser, add_traverser, parent_traverser, class_map, all_classes, namespace, expression, dependencies):
    TRAVERSER = '''template <template <class> class Traverser, class Derived>
struct <ADD_TRAVERSER>: public Traverser<Derived>
{
  typedef Traverser<Derived> super;
  using super::enter;
  using super::leave;
  using super::operator();

<VISIT_TEXT>
};

/// \\\\brief Traverser class
template <typename Derived>
struct <TRAVERSER>: public <ADD_TRAVERSER><<PARENT_TRAVERSER>, Derived>
{
  typedef <ADD_TRAVERSER><<PARENT_TRAVERSER>, Derived> super;
  using super::enter;
  using super::leave;
  using super::operator();
};
'''
    classnames = parse_classnames(class_map[namespace], namespace)

    result = []
    classes = [all_classes[name] for name in classnames]

    # preserve the same order as old generation
    classes.sort(compare_classes)

    for c in classes:
        if is_dependent_type(dependencies, c.classname(True)) or ('E' in c.modifiers() and is_dependent_type(dependencies, c.superclass(True))):
            result.append(c.traverser_function(all_classes, dependencies))
    visit_text = indent_text('\n'.join(result), '  ')
    text = TRAVERSER
    text = re.sub('<PARENT_TRAVERSER>', parent_traverser, text)
    text = re.sub('<TRAVERSER>', traverser, text)
    text = re.sub('<ADD_TRAVERSER>', add_traverser, text)
    text = re.sub('<VISIT_TEXT>', visit_text, text)

    #----------------------------------------------------------------------------------------#
    # N.B. THIS IS AN UGLY HACK to deal with the optional time function in some LPS classes
    # TODO: investigate if the time interface can be improved
    text = re.sub(r'static_cast<Derived&>\(\*this\)\(x.time\(\)\);', '''if (x.has_time())
    {
      static_cast<Derived&>(*this)(x.time());
    }''', text)
    #----------------------------------------------------------------------------------------#

    label = add_traverser
    if namespace in ['action_formulas', 'regular_formulas', 'state_formulas']:
        label = re.sub('^add_', '%s::add_' % namespace, label)
    insert_text_in_file(filename, text, 'generated %s code' % label)

def make_builder(filename, builder, add_builder, parent_builder, class_map, all_classes, namespace, expression, dependencies, modifiability_map):
    BUILDER = '''template <template <class> class Builder, class Derived>
struct <ADD_BUILDER>: public Builder<Derived>
{
  typedef Builder<Derived> super;
  using super::enter;
  using super::leave;
  using super::operator();

<VISIT_TEXT>
};

/// \\\\brief Builder class
template <typename Derived>
struct <BUILDER>: public <ADD_BUILDER><<PARENT_BUILDER>, Derived>
{
  typedef <ADD_BUILDER><<PARENT_BUILDER>, Derived> super;
  using super::enter;
  using super::leave;
  using super::operator();
};
'''

    classnames = parse_classnames(class_map[namespace], namespace)

    result = []
    classes = [all_classes[name] for name in classnames]

    # preserve the same order as old generation
    classes.sort(compare_classes)

    for c in classes:
        if is_dependent_type(dependencies, c.classname(True)) or ('E' in c.modifiers() and is_dependent_type(dependencies, c.superclass(True))):
            result.append(c.builder_function(all_classes, dependencies, modifiability_map))
    visit_text = indent_text('\n'.join(result), '  ')
    text = BUILDER
    text = re.sub('<PARENT_BUILDER>', parent_builder, text)
    text = re.sub('<BUILDER>', builder, text)
    text = re.sub('<ADD_BUILDER>', add_builder, text)
    text = re.sub('<VISIT_TEXT>', visit_text, text)

    #----------------------------------------------------------------------------------------#
    # N.B. THIS IS AN UGLY HACK to deal with the optional time function in some LPS classes
    # TODO: investigate if the time interface can be improved
    text = re.sub(r'x.time\(\) = static_cast<Derived&>\(\*this\)\(x.time\(\)\);', '''if (x.has_time())
    {
      x.time() = static_cast<Derived&>(*this)(x.time());
    }''', text)
    #----------------------------------------------------------------------------------------#

    label = add_builder
    if namespace in ['action_formulas', 'regular_formulas', 'state_formulas']:
        label = re.sub('^add_', '%s::add_' % namespace, label)
    insert_text_in_file(filename, text, 'generated %s code' % label)

if __name__ == "__main__":
    class_map = {
          'core'             : CORE_CLASSES,
          'data'             : DATA_EXPRESSION_CLASSES + ASSIGNMENT_EXPRESSION_CLASSES + SORT_EXPRESSION_CLASSES + CONTAINER_TYPES + BINDER_TYPES + ABSTRACTION_EXPRESSION_CLASSES + STRUCTURED_SORT_ELEMENTS + DATA_CLASSES,
          'state_formulas'   : STATE_FORMULA_CLASSES,
          'regular_formulas' : REGULAR_FORMULA_CLASSES,
          'action_formulas'  : ACTION_FORMULA_CLASSES,
          'lps'              : LPS_CLASSES,
          'process'          : PROCESS_CLASSES + PROCESS_EXPRESSION_CLASSES,
          'pbes_system'      : PBES_CLASSES + PBES_EXPRESSION_CLASSES,
          'bes'              : BOOLEAN_CLASSES + BOOLEAN_EXPRESSION_CLASSES
        }

    all_classes = parse_class_map(class_map)

    boolean_expression_dependencies = find_dependencies(all_classes, 'bes::boolean_expression')
    data_expression_dependencies    = find_dependencies(all_classes, 'data::data_expression')
    pbes_expression_dependencies    = find_dependencies(all_classes, 'pbes_system::pbes_expression')
    process_expression_dependencies = find_dependencies(all_classes, 'process::process_expression')
    sort_expression_dependencies    = find_dependencies(all_classes, 'data::sort_expression')
    action_formula_dependencies     = find_dependencies(all_classes, 'action_formulas::action_formula')
    regular_formula_dependencies    = find_dependencies(all_classes, 'regular_formulas::regular_formula')
    state_formula_dependencies      = find_dependencies(all_classes, 'state_formulas::state_formula')

    variable_dependencies           = find_dependencies(all_classes, 'data::variable')
    boolean_variable_dependencies   = find_dependencies(all_classes, 'bes::boolean_variable')
    identifier_string_dependencies  = find_dependencies(all_classes, 'core::identifier_string')

    #print_dependencies(data_expression_dependencies, "data_expression_dependencies")

    modifiability_map = make_modifiability_map(all_classes)

    # sort_expression_builder
    make_builder('../../data/include/mcrl2/data/builder.h'        , 'sort_expression_builder', 'add_sort_expressions', 'core::builder'                            , class_map, all_classes, 'data'            , 'data::sort_expression', sort_expression_dependencies, modifiability_map)
    make_builder('../../lps/include/mcrl2/lps/builder.h'          , 'sort_expression_builder', 'add_sort_expressions', 'data::sort_expression_builder'            , class_map, all_classes, 'lps'             , 'data::sort_expression', sort_expression_dependencies, modifiability_map)
    make_builder('../../process/include/mcrl2/process/builder.h'  , 'sort_expression_builder', 'add_sort_expressions', 'lps::sort_expression_builder'             , class_map, all_classes, 'process'         , 'data::sort_expression', sort_expression_dependencies, modifiability_map)
    make_builder('../../pbes/include/mcrl2/pbes/builder.h'        , 'sort_expression_builder', 'add_sort_expressions', 'data::sort_expression_builder'            , class_map, all_classes, 'pbes_system'     , 'data::sort_expression', sort_expression_dependencies, modifiability_map)
    make_builder('../../lps/include/mcrl2/modal_formula/builder.h', 'sort_expression_builder', 'add_sort_expressions', 'lps::sort_expression_builder'             , class_map, all_classes, 'action_formulas' , 'data::sort_expression', sort_expression_dependencies, modifiability_map)
    make_builder('../../lps/include/mcrl2/modal_formula/builder.h', 'sort_expression_builder', 'add_sort_expressions', 'action_formulas::sort_expression_builder' , class_map, all_classes, 'regular_formulas', 'data::sort_expression', sort_expression_dependencies, modifiability_map)
    make_builder('../../lps/include/mcrl2/modal_formula/builder.h', 'sort_expression_builder', 'add_sort_expressions', 'regular_formulas::sort_expression_builder', class_map, all_classes, 'state_formulas'  , 'data::sort_expression', sort_expression_dependencies, modifiability_map)

    # data_expression_builder
    make_builder('../../data/include/mcrl2/data/builder.h'        , 'data_expression_builder', 'add_data_expressions', 'core::builder'                            , class_map, all_classes, 'data'            , 'data::data_expression', data_expression_dependencies, modifiability_map)
    make_builder('../../lps/include/mcrl2/lps/builder.h'          , 'data_expression_builder', 'add_data_expressions', 'data::data_expression_builder'            , class_map, all_classes, 'lps'             , 'data::data_expression', data_expression_dependencies, modifiability_map)
    make_builder('../../process/include/mcrl2/process/builder.h'  , 'data_expression_builder', 'add_data_expressions', 'lps::data_expression_builder'             , class_map, all_classes, 'process'         , 'data::data_expression', data_expression_dependencies, modifiability_map)
    make_builder('../../pbes/include/mcrl2/pbes/builder.h'        , 'data_expression_builder', 'add_data_expressions', 'data::data_expression_builder'            , class_map, all_classes, 'pbes_system'     , 'data::data_expression', data_expression_dependencies, modifiability_map)
    make_builder('../../lps/include/mcrl2/modal_formula/builder.h', 'data_expression_builder', 'add_data_expressions', 'lps::data_expression_builder'             , class_map, all_classes, 'action_formulas' , 'data::data_expression', data_expression_dependencies, modifiability_map)
    make_builder('../../lps/include/mcrl2/modal_formula/builder.h', 'data_expression_builder', 'add_data_expressions', 'action_formulas::data_expression_builder' , class_map, all_classes, 'regular_formulas', 'data::data_expression', data_expression_dependencies, modifiability_map)
    make_builder('../../lps/include/mcrl2/modal_formula/builder.h', 'data_expression_builder', 'add_data_expressions', 'regular_formulas::data_expression_builder', class_map, all_classes, 'state_formulas'  , 'data::data_expression', data_expression_dependencies, modifiability_map)

    # pbes_expression_builder
    make_builder('../../pbes/include/mcrl2/pbes/builder.h', 'pbes_expression_builder', 'add_pbes_expressions', 'core::builder', class_map, all_classes, 'pbes_system', 'pbes_system::pbes_expression', pbes_expression_dependencies, modifiability_map)

    # boolean_expression_builder
    make_builder('../../bes/include/mcrl2/bes/builder.h', 'boolean_expression_builder', 'add_boolean_expressions', 'core::builder', class_map, all_classes, 'bes', 'bes::boolean_expression', boolean_expression_dependencies, modifiability_map)

    # process_expression_builder
    make_builder('../../process/include/mcrl2/process/builder.h', 'process_expression_builder', 'add_process_expressions', 'core::builder', class_map, all_classes, 'process', 'process::process_expression', process_expression_dependencies, modifiability_map)

    # state_formula_builder
    make_builder('../../lps/include/mcrl2/modal_formula/builder.h', 'state_formula_builder', 'add_state_formula_expressions', 'core::builder', class_map, all_classes, 'state_formulas', 'state_formulas::state_formula', state_formula_dependencies, modifiability_map)

    # action_formula_builder
    make_builder('../../lps/include/mcrl2/modal_formula/builder.h', 'action_formula_builder', 'add_action_formula_expressions', 'core::builder', class_map, all_classes, 'action_formulas', 'action_formulas::action_formula', action_formula_dependencies, modifiability_map)

    # regular_formula_builder
    make_builder('../../lps/include/mcrl2/modal_formula/builder.h', 'regular_formula_builder', 'add_regular_formula_expressions', 'core::builder', class_map, all_classes, 'regular_formulas', 'regular_formulas::regular_formula', regular_formula_dependencies, modifiability_map)

    # sort_expression_traverser
    make_traverser('../../data/include/mcrl2/data/traverser.h'        , 'sort_expression_traverser', 'add_traverser_sort_expressions', 'core::traverser'                            , class_map, all_classes, 'data'            , 'data::sort_expression', sort_expression_dependencies)
    make_traverser('../../lps/include/mcrl2/lps/traverser.h'          , 'sort_expression_traverser', 'add_traverser_sort_expressions', 'data::sort_expression_traverser'            , class_map, all_classes, 'lps'             , 'data::sort_expression', sort_expression_dependencies)
    make_traverser('../../process/include/mcrl2/process/traverser.h'  , 'sort_expression_traverser', 'add_traverser_sort_expressions', 'lps::sort_expression_traverser'             , class_map, all_classes, 'process'         , 'data::sort_expression', sort_expression_dependencies)
    make_traverser('../../pbes/include/mcrl2/pbes/traverser.h'        , 'sort_expression_traverser', 'add_traverser_sort_expressions', 'data::sort_expression_traverser'            , class_map, all_classes, 'pbes_system'     , 'data::sort_expression', sort_expression_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'sort_expression_traverser', 'add_traverser_sort_expressions', 'lps::sort_expression_traverser'             , class_map, all_classes, 'action_formulas' , 'data::sort_expression', sort_expression_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'sort_expression_traverser', 'add_traverser_sort_expressions', 'action_formulas::sort_expression_traverser' , class_map, all_classes, 'regular_formulas', 'data::sort_expression', sort_expression_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'sort_expression_traverser', 'add_traverser_sort_expressions', 'regular_formulas::sort_expression_traverser', class_map, all_classes, 'state_formulas'  , 'data::sort_expression', sort_expression_dependencies)

    # data_expression_traverser
    make_traverser('../../data/include/mcrl2/data/traverser.h'        , 'data_expression_traverser', 'add_traverser_data_expressions', 'core::traverser'                            , class_map, all_classes, 'data'            , 'data::data_expression', data_expression_dependencies)
    make_traverser('../../lps/include/mcrl2/lps/traverser.h'          , 'data_expression_traverser', 'add_traverser_data_expressions', 'data::data_expression_traverser'            , class_map, all_classes, 'lps'             , 'data::data_expression', data_expression_dependencies)
    make_traverser('../../process/include/mcrl2/process/traverser.h'  , 'data_expression_traverser', 'add_traverser_data_expressions', 'lps::data_expression_traverser'             , class_map, all_classes, 'process'         , 'data::data_expression', data_expression_dependencies)
    make_traverser('../../pbes/include/mcrl2/pbes/traverser.h'        , 'data_expression_traverser', 'add_traverser_data_expressions', 'data::data_expression_traverser'            , class_map, all_classes, 'pbes_system'     , 'data::data_expression', data_expression_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'data_expression_traverser', 'add_traverser_data_expressions', 'lps::data_expression_traverser'             , class_map, all_classes, 'action_formulas' , 'data::data_expression', data_expression_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'data_expression_traverser', 'add_traverser_data_expressions', 'action_formulas::data_expression_traverser' , class_map, all_classes, 'regular_formulas', 'data::data_expression', data_expression_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'data_expression_traverser', 'add_traverser_data_expressions', 'regular_formulas::data_expression_traverser', class_map, all_classes, 'state_formulas'  , 'data::data_expression', data_expression_dependencies)

    # variable_traverser
    make_traverser('../../data/include/mcrl2/data/traverser.h'        , 'variable_traverser', 'add_traverser_variables', 'core::traverser'                     , class_map, all_classes, 'data'            , 'data::variable', variable_dependencies)
    make_traverser('../../lps/include/mcrl2/lps/traverser.h'          , 'variable_traverser', 'add_traverser_variables', 'data::variable_traverser'            , class_map, all_classes, 'lps'             , 'data::variable', variable_dependencies)
    make_traverser('../../process/include/mcrl2/process/traverser.h'  , 'variable_traverser', 'add_traverser_variables', 'lps::variable_traverser'             , class_map, all_classes, 'process'         , 'data::variable', variable_dependencies)
    make_traverser('../../pbes/include/mcrl2/pbes/traverser.h'        , 'variable_traverser', 'add_traverser_variables', 'data::variable_traverser'            , class_map, all_classes, 'pbes_system'     , 'data::variable', variable_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'variable_traverser', 'add_traverser_variables', 'lps::variable_traverser'             , class_map, all_classes, 'action_formulas' , 'data::variable', variable_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'variable_traverser', 'add_traverser_variables', 'action_formulas::variable_traverser' , class_map, all_classes, 'regular_formulas', 'data::variable', variable_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'variable_traverser', 'add_traverser_variables', 'regular_formulas::variable_traverser', class_map, all_classes, 'state_formulas'  , 'data::variable', variable_dependencies)

    # boolean_variable_traverser
    make_traverser('../../bes/include/mcrl2/bes/traverser.h', 'boolean_variable_traverser', 'add_traverser_boolean_variables', 'core::traverser', class_map, all_classes, 'bes', 'bes::boolean_variable', boolean_variable_dependencies)

    # identifier_string_traverser
    make_traverser('../../data/include/mcrl2/data/traverser.h'        , 'identifier_string_traverser', 'add_traverser_identifier_strings', 'core::traverser'                              , class_map, all_classes, 'data'            , 'core::identifier_string', identifier_string_dependencies)
    make_traverser('../../lps/include/mcrl2/lps/traverser.h'          , 'identifier_string_traverser', 'add_traverser_identifier_strings', 'data::identifier_string_traverser'            , class_map, all_classes, 'lps'             , 'core::identifier_string', identifier_string_dependencies)
    make_traverser('../../process/include/mcrl2/process/traverser.h'  , 'identifier_string_traverser', 'add_traverser_identifier_strings', 'lps::identifier_string_traverser'             , class_map, all_classes, 'process'         , 'core::identifier_string', identifier_string_dependencies)
    make_traverser('../../pbes/include/mcrl2/pbes/traverser.h'        , 'identifier_string_traverser', 'add_traverser_identifier_strings', 'data::identifier_string_traverser'            , class_map, all_classes, 'pbes_system'     , 'core::identifier_string', identifier_string_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'identifier_string_traverser', 'add_traverser_identifier_strings', 'lps::identifier_string_traverser'             , class_map, all_classes, 'action_formulas' , 'core::identifier_string', identifier_string_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'identifier_string_traverser', 'add_traverser_identifier_strings', 'action_formulas::identifier_string_traverser' , class_map, all_classes, 'regular_formulas', 'core::identifier_string', identifier_string_dependencies)
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'identifier_string_traverser', 'add_traverser_identifier_strings', 'regular_formulas::identifier_string_traverser', class_map, all_classes, 'state_formulas'  , 'core::identifier_string', identifier_string_dependencies)

    # pbes_expression_traverser
    make_traverser('../../pbes/include/mcrl2/pbes/traverser.h', 'pbes_expression_traverser', 'add_traverser_pbes_expressions', 'pbes_system::pbes_expression_traverser_base', class_map, all_classes, 'pbes_system', 'pbes_system::pbes_expression', pbes_expression_dependencies)

    # boolean_expression_traverser
    make_traverser('../../bes/include/mcrl2/bes/traverser.h', 'boolean_expression_traverser', 'add_traverser_boolean_expressions', 'core::traverser', class_map, all_classes, 'bes', 'bes::boolean_expression', boolean_expression_dependencies)

    # process_expression_traverser
    make_traverser('../../process/include/mcrl2/process/traverser.h', 'process_expression_traverser', 'add_traverser_process_expressions', 'process::process_expression_traverser_base', class_map, all_classes, 'process', 'process::process_expression', process_expression_dependencies)

    # state_formula_traverser
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'state_formula_traverser', 'add_traverser_state_formula_expressions', 'state_formulas::state_formula_traverser_base', class_map, all_classes, 'state_formulas', 'state_formulas::state_formula', state_formula_dependencies)

    # action_formula_traverser
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'action_formula_traverser', 'add_traverser_action_formula_expressions', 'action_formulas::action_formula_traverser_base', class_map, all_classes, 'action_formulas', 'action_formulas::action_formula', action_formula_dependencies)

    # regular_formula_traverser
    make_traverser('../../lps/include/mcrl2/modal_formula/traverser.h', 'regular_formula_traverser', 'add_traverser_regular_formula_expressions', 'regular_formulas::regular_formula_traverser_base', class_map, all_classes, 'regular_formulas', 'regular_formulas::regular_formula', regular_formula_dependencies)
