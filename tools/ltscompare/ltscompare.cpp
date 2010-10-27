// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ltscompare.cpp

#define NAME "ltscompare"
#define AUTHOR "Muck van Weerdenburg"

#include <string>
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/exception.h"

#include "mcrl2/utilities/input_tool.h"
#include "mcrl2/utilities/mcrl2_gui_tool.h"
#include "mcrl2/utilities/tool.h"

#include "mcrl2/lts/lts_algorithm.h"
#include "mcrl2/lts/lts_io.h"

#include "mcrl2/lts/lts_fsm.h"
#include "mcrl2/lts/lts_lts.h"
#include "mcrl2/lts/lts_aut.h"
#include "mcrl2/lts/lts_bcg.h"
#include "mcrl2/lts/lts_dot.h"


using namespace std;
using namespace mcrl2::lts;
using namespace mcrl2::lts::detail;
using namespace mcrl2::utilities::tools;
using namespace mcrl2::utilities;
using namespace mcrl2::core;

static const char *equivalent_string(lts_equivalence eq)
{
  switch ( eq )
  {
    case lts_eq_bisim:
      return "strongly bisimilar";
    case lts_eq_branching_bisim:
      return "branching bisimilar";
    case lts_eq_divergence_preserving_branching_bisim:
      return "divergence preserving branching bisimilar";
    case lts_eq_sim:
      return "strongly simulation equivalent";
    case lts_eq_trace:
      return "strongly trace equivalent";
    case lts_eq_weak_trace:
      return "weak trace equivalent";
    default:
      return "equivalent";
  }
}

static const char *preorder_string(lts_preorder pre)
{
  switch ( pre )
  {
    case lts_pre_sim:
      return "strongly simulated by";
    case lts_pre_trace:
      return "strongly trace-included in";
    case lts_pre_weak_trace:
      return "weakly trace-included in";
    default:
      return "included in";
  }
}

static const std::set<lts_equivalence> &initialise_allowed_eqs()
{
  static std::set<lts_equivalence> s;
  s.insert(lts_eq_bisim);
  s.insert(lts_eq_branching_bisim);
  s.insert(lts_eq_divergence_preserving_branching_bisim);
  s.insert(lts_eq_sim);
  s.insert(lts_eq_trace);
  s.insert(lts_eq_weak_trace);
  return s;
}
static const std::set<lts_equivalence> &allowed_eqs()
{
  static const std::set<lts_equivalence> &s = initialise_allowed_eqs();
  return s;
}

struct t_tool_options 
{
  // defaults
  t_tool_options():
    name_for_first(""),
    name_for_second(""),
    format_for_first(lts_none),
    format_for_second(lts_none),
    equivalence(lts_eq_none),
    preorder(lts_pre_none),
    generate_counter_examples(false)
  {}

  std::string     name_for_first;
  std::string     name_for_second;
  lts_type        format_for_first;
  lts_type        format_for_second;
  lts_equivalence equivalence;
  lts_preorder    preorder;
  std::vector<std::string> tau_actions;   // Actions with these labels must be considered equal to tau.
  bool generate_counter_examples;
};

typedef  input_tool ltscompare_base;
class ltscompare_tool : public ltscompare_base
{
  private:
    t_tool_options tool_options;

    // Check whether preconditions w.r.t. equivalence and pre-order are satisfied.
    // This is needed to make sure wrappers can call this tool without the --help,
    // --equivalence or --preorder options
    void check_preconditions()
    {
      if(tool_options.equivalence != lts_eq_none && tool_options.preorder != lts_pre_none)
      {
        throw mcrl2::runtime_error("options -e/--equivalence and -p/--preorder cannot be used simultaneously");
      }

      if (tool_options.equivalence == lts_eq_none && tool_options.preorder == lts_pre_none)
      {
        throw mcrl2::runtime_error("one of the options -e/--equivalence and -p/--preorder must be used");
      }

      if (tool_options.name_for_first.empty() && tool_options.name_for_second.empty())
      {
        throw mcrl2::runtime_error("too few file arguments");
      }
    }

  public:
    ltscompare_tool() :
      ltscompare_base(NAME,AUTHOR,
        "compare two LTSs",
        "Determine whether or not the labelled transition systems (LTSs) in INFILE1 and INFILE2 are related by some equivalence or preorder. "
        "If INFILE1 is not supplied, stdin is used.\n"
        "\n"
        "The input formats are determined by the contents of INFILE1 and INFILE2. "
        "Options --in1 and --in2 can be used to force the input format of INFILE1 and INFILE2, respectively. "
        "The supported formats are:\n"
        + mcrl2::lts::detail::supported_lts_formats_text()
      )
    {
    }

  private:

    template <class LTS_TYPE>
    bool lts_compare(void)
    {
      LTS_TYPE l1,l2;
      l1.load(tool_options.name_for_first);
      l2.load(tool_options.name_for_second);

      l1.hide_actions(tool_options.tau_actions);
      l2.hide_actions(tool_options.tau_actions);

      bool result = true;
      if ( tool_options.equivalence != lts_eq_none )
      {
        gsVerboseMsg("comparing LTSs using %s...\n",
            name_of_equivalence(tool_options.equivalence).c_str());

        result = compare(l1,l2,tool_options.equivalence,tool_options.generate_counter_examples);

        gsMessage("LTSs are %s%s\n",
            ((result) ? "" : "not "),
            equivalent_string(tool_options.equivalence));
      }

      if ( tool_options.preorder != lts_pre_none )
      {
        gsVerboseMsg("comparing LTSs using %s...\n",
            name_of_preorder(tool_options.preorder).c_str());

        result = compare(l1,l2,tool_options.preorder);

        gsMessage("LTS in %s is %s%s LTS in %s\n",
            tool_options.name_for_first.c_str(),
            ((result) ? "" : "not "),
            preorder_string(tool_options.preorder),
            tool_options.name_for_second.c_str() );
      }

      return result;
    }

  public:
    bool run()
    {

      check_preconditions();

      if (tool_options.format_for_first==lts_none)
      {
        tool_options.format_for_first = guess_format(tool_options.name_for_first);
      }

      if (tool_options.format_for_second==lts_none)
      {
        tool_options.format_for_second = guess_format(tool_options.name_for_second);
      }

      if (tool_options.format_for_first!=tool_options.format_for_second)
      { 
        throw mcrl2::runtime_error("The input labelled transition systems have different types");
      }

      switch (tool_options.format_for_first)
      {
        case lts_lts:
        {
          return lts_compare<lts_lts_t>();
        }
        case lts_none:
          cerr << "No input format is specified. Assuming .aut format.\n";
        case lts_aut:
        {
          return lts_compare<lts_aut_t>();
        }
        /* case lts_svc:
        {
          return lts_compare<lts_svc_t>();
        } */
        case lts_fsm:
        {
          return lts_compare<lts_fsm_t>();
        }
#ifdef USE_BCG
        case lts_bcg:
        {
          return lts_compare<lts_bcg_t>();
        }
#endif
        case lts_dot:
        {
          return lts_compare<lts_dot_t>();
        }
      }
      return true;
    }

  protected:
    std::string synopsis() const
    {
      return "[OPTION]... [INFILE1] INFILE2";
    }

    void check_positional_options(const command_line_parser &parser)
    {
      if (2 < parser.arguments.size())
      {
        parser.error("too many file arguments");
      }
    }

    void set_tau_actions(std::vector <std::string>& tau_actions, std::string const& act_names)
    {
      std::string::size_type lastpos = 0, pos;
      while ( (pos = act_names.find(',',lastpos)) != std::string::npos )
      {
        tau_actions.push_back(act_names.substr(lastpos,pos-lastpos));
        lastpos = pos+1;
      }
      tau_actions.push_back(act_names.substr(lastpos));
    }

    void add_options(interface_description &desc)
    {
      ltscompare_base::add_options(desc);

      desc.
        add_option("in1", make_mandatory_argument("FORMAT"),
          "use FORMAT as the format for INFILE1 (or stdin)", 'i').
        add_option("in2", make_mandatory_argument("FORMAT"),
          "use FORMAT as the format for INFILE2", 'j').
        add_option("equivalence", make_mandatory_argument("NAME"),
          "use equivalence NAME:\n"
          +supported_lts_equivalences_text(allowed_eqs())+"\n"
          "(not allowed in combination with -p/--preorder)"
          , 'e').
        add_option("preorder", make_mandatory_argument("NAME"),
          "use preorder NAME:\n"
          +supported_lts_preorders_text()+"\n"
          "(not allowed in combination with -e/--equivalence)"
          , 'p').
        add_option("tau", make_mandatory_argument("ACTNAMES"),
          "consider actions with a name in the comma separated list ACTNAMES to "
          "be internal (tau) actions in addition to those defined as such by "
          "the input").
        add_option("counter-example",
          "generate counter example traces if the input lts's are not equivalent",'c');
    }

    void parse_options(const command_line_parser &parser)
    {
      ltscompare_base::parse_options(parser);

      if (parser.options.count("equivalence") > 1)
      {
        parser.error("multiple use of option -e/--equivalence; only one occurrence is allowed");
      }
  
      if (parser.options.count("preorder") > 1)
      {
        parser.error("multiple use of option -p/--preorder; only one occurrence is allowed");
      }
  
      if (parser.options.count("counter-example")>0 && parser.options.count("equivalence")==0)
      { 
        parser.error("counter examples can only be used in combination with an equivalence");
      }
  
      tool_options.equivalence = lts_eq_none;
  
      if (parser.options.count("equivalence")) {
  
        tool_options.equivalence = parse_equivalence(
            parser.option_argument("equivalence"));
  
        if ( allowed_eqs().count(tool_options.equivalence) == 0 )
        {
          parser.error("option -e/--equivalence has illegal argument '" +
              parser.option_argument("equivalence") + "'");
        }
      }
  
      tool_options.preorder = lts_pre_none;
  
      if (parser.options.count("preorder")) {
  
        tool_options.preorder = parse_preorder(
            parser.option_argument("preorder"));
  
        if (tool_options.preorder == lts_pre_none)
        {
          parser.error("option -p/--preorder has illegal argument '" +
              parser.option_argument("preorder") + "'");
        }
      }
  
      if (parser.options.count("tau")) 
      { set_tau_actions(tool_options.tau_actions, parser.option_argument("tau"));
      }
 
      tool_options.generate_counter_examples=parser.options.count("counter-example")>0;
  
      if (parser.arguments.size() == 1) {
        tool_options.name_for_second = parser.arguments[0];
      } else if (parser.arguments.size() == 2) {
        tool_options.name_for_first  = parser.arguments[0];
        tool_options.name_for_second = parser.arguments[1];
      } // else something strange going on, caught in check_preconditions()
  
      if (parser.options.count("in1")) {
        if (1 < parser.options.count("in1")) {
          std::cerr << "warning: multiple input formats specified for first LTS; can only use one\n";
        }
  
        tool_options.format_for_first = mcrl2::lts::detail::parse_format(parser.option_argument("in1"));
  
        if (tool_options.format_for_first == lts_none) {
          std::cerr << "warning: format '" << parser.option_argument("in1") <<
                       "' is not recognised; option ignored" << std::endl;
        }
      }
      else if (!tool_options.name_for_first.empty()) {
        tool_options.format_for_first = mcrl2::lts::detail::guess_format(tool_options.name_for_first);
      } else {
        gsWarningMsg("cannot detect format from stdin and no input format specified; assuming aut format\n");
        tool_options.format_for_first = lts_aut;
      }
      if (parser.options.count("in2")) {
        if (1 < parser.options.count("in2")) {
          std::cerr << "warning: multiple input formats specified for second LTS; can only use one\n";
        }
  
        tool_options.format_for_second = mcrl2::lts::detail::parse_format(parser.option_argument("in2"));
  
        if (tool_options.format_for_second == lts_none) {
          std::cerr << "warning: format '" << parser.option_argument("in2") <<
                       "' is not recognised; option ignored" << std::endl;
        }
      }
      else if (!tool_options.name_for_first.empty()) {
        tool_options.format_for_second = mcrl2::lts::detail::guess_format(tool_options.name_for_second);
      }
    }

};

class ltscompare_gui_tool: public mcrl2_gui_tool<ltscompare_tool> 
{
  public:
	ltscompare_gui_tool() {

		std::vector<std::string> values;

		m_gui_options["counter-example"] = create_checkbox_widget();

		values.clear();
		values.push_back("bisim");
		values.push_back("branching-bisim");
		values.push_back("dpbranching-bisim");
		values.push_back("sim");
		values.push_back("trace");
		values.push_back("weak-trace");
		m_gui_options["equivalence"] = create_radiobox_widget(values);

		values.clear();
		values.push_back("sim");
		values.push_back("trace");
		values.push_back("weak-trace");
		m_gui_options["preorder"] = create_radiobox_widget(values);
		m_gui_options["tau"] = create_textctrl_widget();

		//-iFORMAT, --in1=FORMAT   use FORMAT as the format for INFILE1 (or stdin)
		//-jFORMAT, --in2=FORMAT   use FORMAT as the format for INFILE2

	}
};

int main(int argc, char **argv) {
  MCRL2_ATERMPP_INIT(argc, argv)

  return ltscompare_gui_tool().execute(argc,argv);
}
