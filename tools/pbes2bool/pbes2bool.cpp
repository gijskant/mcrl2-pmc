// Author(s): Jan Friso Groote // Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbes2bool.cpp
/// \brief Add your file description here.

#include "boost.hpp" // precompiled headers

// ======================================================================
//
// file          : pbes2bool
// date          : 15-04-2007
// version       : 0.1.3
//
// author(s)     : Alexander van Dam <avandam@damdonk.nl>
//                 Jan Friso Groote <J.F.Groote@tue.nl>
//
// ======================================================================


#define NAME "pbes2bool"
#define AUTHOR "Jan Friso Groote"

//C++
#include <iostream>
#include <string>
#include <utility>

#include <sstream>

//Tool framework
#include "mcrl2/utilities/input_tool.h"
#include "mcrl2/utilities/rewriter_tool.h"
#include "mcrl2/utilities/pbes_rewriter_tool.h"
#include "mcrl2/utilities/mcrl2_gui_tool.h"
#include "mcrl2/utilities/execution_timer.h"

//Data Framework
#include "mcrl2/data/enumerator.h"
#include "mcrl2/data/selection.h"
#include "mcrl2/data/data_equation.h" // for debug std::cerr

//Boolean equation systems
#include "mcrl2/pbes/utility.h"
#include "mcrl2/bes/bes_deprecated.h"
#include "mcrl2/bes/boolean_equation_system.h"
#include "mcrl2/bes/bes2pbes.h"
#include "mcrl2/pbes/pbesrewr.h"
#include "mcrl2/pbes/find.h"
#include "mcrl2/pbes/detail/instantiate_global_variables.h"
#include "mcrl2/atermpp/aterm_init.h"

using namespace std;
using namespace mcrl2::utilities;
using namespace mcrl2::core;
using bes::bes_expression;
using namespace ::bes;

// using atermpp::make_substitution;

//Function declarations used by main program
//------------------------------------------

using namespace mcrl2;
using utilities::tools::input_tool;
using utilities::tools::rewriter_tool;
using utilities::tools::pbes_rewriter_tool;
using namespace mcrl2::utilities::tools;

class pbes2bool_tool: public pbes_rewriter_tool<rewriter_tool<input_tool> >
{
  protected:
    // Tool options.
    ::bes::transformation_strategy opt_strategy; // The strategy
    bool opt_use_hashtables;                   // The hashtable option
    bool opt_construct_counter_example;        // The counter example option
    bool opt_store_as_tree;                    // The tree storage option
    bool opt_data_elm;                         // The data elimination option
    std::string opt_counter_example_file;      // The counter example file name

    typedef pbes_rewriter_tool<rewriter_tool<input_tool> > super;

    std::string default_rewriter() const
    {
      return "quantifier-all";
    }

    // Overload synopsis to cope with optional OUTFILE
    std::string synopsis() const
    {
      return "[OPTION]...[INFILE]\n";
    }

  public:
    pbes2bool_tool()
      : super(
        NAME,
        AUTHOR,
        "Generate a BES from a PBES and solve it. ",
        "Solves (P)BES from INFILE. "
        "If INFILE is not present, stdin is used. "),
      opt_strategy(::bes::lazy),
      opt_use_hashtables(false),
      opt_construct_counter_example(false),
      opt_store_as_tree(false),
      opt_data_elm(true),
      opt_counter_example_file("")
    {}

  protected:
    void parse_options(const command_line_parser& parser)
    {
      super::parse_options(parser);

      input_tool::parse_options(parser);

      opt_use_hashtables            = 0 < parser.options.count("hashtables");
      opt_construct_counter_example = 0 < parser.options.count("counter");
      opt_store_as_tree             = 0 < parser.options.count("tree");
      opt_data_elm                  = parser.options.count("unused-data") == 0;
      opt_strategy                  = lazy;

      if (parser.options.count("output")) // Output format is deprecated.
      {
        std::string format = parser.option_argument("output");
        parser.error("the option --output or -o is deprecated. Use the tool pbes2bes for this functionality. ");
      }


      if (parser.options.count("strategy"))   // Bes solving strategy (currently only one available)
      {
        int strategy = parser.option_argument_as< int >("strategy");

        switch (strategy)
        {
          case 0:
            opt_strategy = lazy;
            break;
          case 1:
            opt_strategy = optimize;
            break;
          case 2:
            opt_strategy = on_the_fly;
            break;
          case 3:
            opt_strategy = on_the_fly_with_fixed_points;
            break;
          default:
            parser.error("unknown strategy specified: available strategies are '0', '1', '2', and '3'");
        }
      }
    }

    void add_options(interface_description& desc)
    {
      super::add_options(desc);
      desc.
      add_option("strategy", make_mandatory_argument("STRAT"),
                 "use strategy STRAT (default '0');\n"
                 " 0) Compute all boolean equations which can be reached"
                 " from the initial state, without optimization"
                 " (default). This is is the most data efficient"
                 " option per generated equation.\n"
                 " 1) Optimize by immediately substituting the right"
                 " hand sides for already investigated variables"
                 " that are true or false when generating an"
                 " expression. This is as memory efficient as 0.\n"
                 " 2) In addition to 1, also substitute variables that"
                 " are true or false into an already generated right"
                 " hand side. This can mean that certain variables"
                 " become unreachable (e.g. X0 in X0 and X1, when X1"
                 " becomes false, assuming X0 does not occur"
                 " elsewhere. It will be maintained which variables"
                 " have become unreachable as these do not have to be"
                 " investigated. Depending on the PBES, this can"
                 " reduce the size of the generated BES substantially"
                 " but requires a larger memory footprint.\n"
                 " 3) In addition to 2, investigate for generated"
                 " variables whether they occur on a loop, such that"
                 " they can be set to true or false, depending on the"
                 " fixed point symbol. This can increase the time"
                 " needed to generate an equation substantially",
                 's').
      add_option("counter",
                 "print at the end a tree labelled with instantiations "
                 "of the left hand side of equations; this tree is an "
                 "indication of how pbes2bool came to the validity or "
                 "invalidity of the PBES",
                 'c').
      /* add_option("precompile",
        "precompile the pbes for faster rewriting; Deprecated. Does not "
        "work anymore. Only present for compatibility reasons",
        'p'). */
      add_option("hashtables",
                 "use hashtables when substituting in bes equations, "
                 "and translate internal expressions to binary decision "
                 "diagrams (discouraged, due to performance)",
                 'H').
      add_option("output",
                 make_mandatory_argument("FORMAT"),
                 "use output format FORMAT (this option is deprecated. Use the tool pbes2bes instead).\n",
                 'o').
      add_option("tree",
                 "store state in a tree (for memory efficiency)",
                 't').
      add_option("unused_data",
                 "do not remove unused parts of the data specification",
                 'u');
    }

  public:
    bool run()
    {
      using namespace pbes_system;
      using namespace utilities;

      if (core::gsVerbose)
      {
        std::cerr << "pbes2bool parameters:" << std::endl;
        std::cerr << "  input file:         " << m_input_filename << std::endl;
        std::cerr << "  data rewriter:      " << m_rewrite_strategy << std::endl;
        std::cerr << "  pbes rewriter:      " << m_pbes_rewriter_type << std::endl;
      }

      // load the pbes
      mcrl2::pbes_system::pbes<> p;
      try
      {
        p.load(m_input_filename);
      }
      catch (mcrl2::runtime_error& e)
      {
        try
        {
          mcrl2::bes::boolean_equation_system<> b;
          b.load(m_input_filename);
          p = mcrl2::bes::bes2pbes(b);
        }
        catch (mcrl2::runtime_error&) // Throw original exception after trying both pbes and bes fails
        {
          throw(e);
        }
      }
      p.normalize();
      pbes_system::detail::instantiate_global_variables(p);
      // data rewriter

      data::rewriter datar= (opt_data_elm) ?
                            data::rewriter(p.data(), mcrl2::data::used_data_equation_selector(p.data(), pbes_system::find_function_symbols(p), p.global_variables()), rewrite_strategy()) :
                            data::rewriter(p.data(), rewrite_strategy());

      timer().start("instantiation");
      ::bes::boolean_equation_system bes_equations=
        ::bes::boolean_equation_system(
          p,
          datar,
          opt_strategy,
          opt_store_as_tree,
          opt_construct_counter_example,
          opt_use_hashtables);
      timer().finish("instantiation");

      // pbes rewriter
      /* The code below can be reactivated, once the pbes_rewriters deliver acceptable performance.
         As it stands their performance is so bad, that they cannot be used.

      switch (rewriter_type())
      {
        case simplify:
        {
          simplifying_rewriter<pbes_expression, data::rewriter> pbesr(datar);
          pbesrewr(p,pbesr); // Simplify p such that it does not have to be done
                             // repeatedly.
          bes_equations=::bes::boolean_equation_system(
                            p,
                            pbesr,
                            opt_strategy,
                            opt_store_as_tree,
                            opt_construct_counter_example,
                            opt_use_hashtables);
          break;
        }
        case quantifier_finite:
        {
          data::number_postfix_generator generator("UNIQUE_PREFIX");
          data::data_enumerator<> datae(p.data(), datar, generator);
          data::rewriter_with_variables datarv(datar);
          bool enumerate_infinite_sorts = false;
          enumerate_quantifiers_rewriter<pbes_expression, data::rewriter_with_variables,
                                                             data::data_enumerator<> >
                          pbesr(datarv, datae, enumerate_infinite_sorts);
          pbesrewr(p,pbesr);  // Simplify p such that this does not need to be done
                              // repeatedly.
          bes_equations=::bes::boolean_equation_system(
                            p,
                            pbesr,
                            opt_strategy,
                            opt_store_as_tree,
                            opt_construct_counter_example,
                            opt_use_hashtables);
          break;
        }
        case quantifier_all:
        {
          data::number_postfix_generator generator("UNIQUE_PREFIX");
          data::data_enumerator<> datae(p.data(), datar, generator);
          data::rewriter_with_variables datarv(datar);
          const bool enumerate_infinite_sorts1 = false;
          enumerate_quantifiers_rewriter<pbes_expression, data::rewriter_with_variables,
                                                             data::data_enumerator<> >
                          pbesr1(datarv, datae, enumerate_infinite_sorts1);
          pbesrewr(p,pbesr1);  // Simplify p such that this does not need to be done
                               // repeatedly, without expanding quantifiers over infinite
                               // domains.
          const bool enumerate_infinite_sorts2 = true;
          enumerate_quantifiers_rewriter<pbes_expression, data::rewriter_with_variables,
                                                             data::data_enumerator<> >
                          pbesr2(datarv, datae, enumerate_infinite_sorts2);
          bes_equations=::bes::boolean_equation_system(
                            p,
                            pbesr2,
                            opt_strategy,
                            opt_store_as_tree,
                            opt_construct_counter_example,
                            opt_use_hashtables);
          break;
        }
        case pfnf:
        {
          throw mcrl2::runtime_error("The pfnf boolean equation rewriter cannot be used\n");
        }
        case prover:
        {
          throw mcrl2::runtime_error("The prover based rewriter cannot be used\n");
        }
      } */

      timer().start("solving");
      bool result=solve_bes(bes_equations,
                            opt_use_hashtables,
                            opt_construct_counter_example);
      timer().finish("solving");

      core::gsMessage("The solution for the initial variable of the pbes is %s\n",
                      (result ? "true" : "false"));

      if (opt_construct_counter_example)
      {
        bes_equations.print_counter_example(opt_counter_example_file);
      }
      return true;
    }

};

class pbes2bool_gui_tool: public mcrl2_gui_tool<pbes2bool_tool>
{
  public:
    pbes2bool_gui_tool()
    {

      std::vector<std::string> values;

      m_gui_options["counter"] = create_checkbox_widget();
      m_gui_options["hashtables"] = create_checkbox_widget();

      values.clear();
      values.push_back("simplify");
      values.push_back("quantifier-all");
      values.push_back("quantifier-finite");
      values.push_back("pfnf");
      m_gui_options["pbes-rewriter"] = create_radiobox_widget(values);

      add_rewriter_widget();

      values.clear();
      values.push_back("0");
      values.push_back("1");
      values.push_back("2");
      values.push_back("3");
      m_gui_options["strategy"] = create_radiobox_widget(values);

      m_gui_options["tree"] = create_checkbox_widget();
      m_gui_options["unused_data"] = create_checkbox_widget();
    }
};

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  return pbes2bool_gui_tool().execute(argc, argv);
}
