// Author(s): Jan Friso Groote
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl22lps.cpp
/// \brief This tool linearises mcrl2 specifications into linear
///         form.

#define TOOLNAME "mcrl22lps"
#define AUTHOR "Jan Friso Groote"

#include <fstream>
#include <iostream>
#include <string>
#include "mcrl2/utilities/logger.h"
#include "mcrl2/lps/io.h"
#include "mcrl2/lps/linearise.h"
#include "mcrl2/utilities/input_output_tool.h"
#include "mcrl2/utilities/rewriter_tool.h"
#include "mcrl2/process/parse.h"

// #include "gc.h"  Required for ad hoc garbage collection. This is possible with ATcollect,
// useful to find garbage collection problems.

using mcrl2::utilities::tools::input_output_tool;
using mcrl2::utilities::tools::rewriter_tool;

class mcrl22lps_tool : public rewriter_tool< input_output_tool >
{
    typedef rewriter_tool< input_output_tool > super;

  private:
    mcrl2::lps::t_lin_options m_linearisation_options;
    bool noalpha;   // indicates whether alpha reduction is needed.
    bool opt_check_only;

  protected:

    void add_options(mcrl2::utilities::interface_description& desc)
    {
      super::add_options(desc);
      desc.add_option("lin-method",
                      mcrl2::utilities::make_enum_argument<mcrl2::lps::t_lin_method>("NAME").
                      add_value(mcrl2::lps::lmRegular, true).
                      add_value(mcrl2::lps::lmRegular2).
                      add_value(mcrl2::lps::lmStack),
                      "use linearisation method NAME:"
                      , 'l');
      desc.add_option("cluster",
                      "all actions in the final LPS are clustered. "
                      "Clustering means that summands with the same action labels are "
                      "grouped together. For instance, a(f1) . P(g1) + a(f2) . P(g2) "
                      "is replaced by sum b: Bool . a(if(b, f1, f2)) . P(if(b, f2, g2)). "
                      "The advantage is that the number of summands can be reduced "
                      "subtantially in this way. The disadvantage is that sum operators "
                      "are introduced and new data sorts with auxiliary functions are generated. "
                      "In order to avoid the generation of new sorts, the option -b/--binary "
                      "can be used.", 'c');
      desc.add_option("no-cluster",
                      "the actions in intermediate LPSs are not clustered before "
                      "they are put in parallel. By default these processes are "
                      "clustered to avoid a blow-up in the number of summands when "
                      "transforming two parallel linear processes into a single linear "
                      "process. If a linear process with M summands is put in parallel "
                      "with a linear process with N summands the resulting process has "
                      "M×N + M + N summands. Both M and N can be substantially reduced "
                      "by clustering at the cost of introducing new sorts and functions. "
                      "See -c/--cluster, esp. for a short explanation of the clustering "
                      "process.", 'n');
      desc.add_option("no-alpha",
                      "alphabet reductions are not applied. "
                      "By default mcrl22lps attempts to distribute communication, hiding "
                      "and allow operators over the parallel composition operator as "
                      "this reduces the size of intermediate linear processes. By using "
                      "this option, this step can be avoided. The name stems from the "
                      "alphabet axioms in process algebra.", 'z');
      desc.add_option("newstate",
                      "state variables are encoded using enumerated types instead "
                      "of positive natural numbers (Pos). By using this option new "
                      "finite sorts named Enumk are generated where k is the size of "
                      "the domain. Also, auxiliary case functions and equalities are "
                      "defined. In combination with the option --binary the finite "
                      "sorts are encoded by booleans. "
                      "(requires linearisation method 'regular' or 'regular2').", 'w');
      desc.add_option("binary",
                      "when clustering use binary case functions instead of "
                      "n-ary; in the presence of -w/--newstate, state variables are "
                      "encoded by a vector of boolean variables", 'b');
      desc.add_option("statenames",
                      "the names of generated data parameters are extended with the "
                      "name of the process in which they occur. This makes it "
                      "easier to determine where the parameter comes from.", 'a');
      desc.add_option("no-rewrite",
                      "do not rewrite data terms while linearising; useful when the rewrite "
                      "system does not terminate. This option also switches off the application of "
                      "constant elimination.", 'o');
      desc.add_option("no-globvars",
                      "instantiate don't care values with arbitrary constants, "
                      "instead of modelling them by global variables. This has no effect"
                      "on global variables that are declared in the specification.", 'f');
      desc.add_option("no-sumelm",
                      "avoid applying sum elimination in parallel composition", 'm');
      desc.add_option("no-deltaelm",
                      "avoid removing spurious delta summands. "
                      "Due to the existence of time, delta summands cannot be omitted. "
                      "Due to the presence of multi-actions the number of summands can "
                      "be huge. The algorithm for removing delta summands simply works "
                      "by comparing each delta summand with each other summand to see "
                      "whether the condition of the one implies the condition of the "
                      "other. Clearly, this has quadratic complexity, and can take a "
                      "long time.", 'g');
      desc.add_option("delta",
                      "add a true->delta summands to each state in each process; "
                      "these delta's subsume all other conditional timed delta's, "
                      "effectively reducing the number of delta summands drastically "
                      "in the resulting linear process; speeds up linearisation. This is the default, "
                      "but it does not deal correctly with time.", 'D');
      desc.add_option("timed",
                      "Translate the process to linear form preserving all timed information. In parallel processes the number "
                      "of possible time constraints can be large, slowing down linearisation. Confer the --delta option "
                      "which yiels a much faster translation that does not preserve timing correctly", 'T');
      desc.add_option("no-constelm",
                      "do not try to apply constant elimination when generating a linear "
                      "process.");
      desc.add_option("check-only",
                      "check syntax and static semantics; do not linearise", 'e');
    }

    void parse_options(const mcrl2::utilities::command_line_parser& parser)
    {
      super::parse_options(parser);

      opt_check_only                                  = 0 < parser.options.count("check-only");
      noalpha                                         = 0 < parser.options.count("no-alpha");
      m_linearisation_options.final_cluster           = 0 < parser.options.count("cluster");
      m_linearisation_options.no_intermediate_cluster = 0 < parser.options.count("no-cluster");
      m_linearisation_options.newstate                = 0 < parser.options.count("newstate");
      m_linearisation_options.binary                  = 0 < parser.options.count("binary");
      m_linearisation_options.statenames              = 0 < parser.options.count("statenames");
      m_linearisation_options.norewrite               = 0 < parser.options.count("no-rewrite");
      m_linearisation_options.noglobalvars            = 0 < parser.options.count("no-globvars");
      m_linearisation_options.nosumelm                = 0 < parser.options.count("no-sumelm");
      m_linearisation_options.nodeltaelimination      = 0 < parser.options.count("no-deltaelm");
      m_linearisation_options.add_delta               = 0 == parser.options.count("timed");
      m_linearisation_options.do_not_apply_constelm   = 0 < parser.options.count("no-constelm") ||
                                                        0 < parser.options.count("no-rewrite");

      m_linearisation_options.lin_method = parser.option_argument_as< mcrl2::lps::t_lin_method >("lin-method");

      //check for dangerous and illegal option combinations
      if (m_linearisation_options.newstate && m_linearisation_options.lin_method == mcrl2::lps::lmStack)
      {
        throw parser.error("option -w/--newstate cannot be used with -lstack/--lin-method=stack");
      }

      m_linearisation_options.rewrite_strategy = rewrite_strategy();
    }

  public:

    mcrl22lps_tool() : super(
        TOOLNAME,
        AUTHOR,
        "translate an mCRL2 specification to an LPS",
        "Linearises the mCRL2 specification in INFILE and writes the resulting LPS to "
        "OUTFILE. If OUTFILE is not present, stdout is used. If INFILE is not present, "
        "stdin is used."), noalpha(false), opt_check_only(false)
    {}

    bool run()
    {
      //linearise infilename with options
      mcrl2::process::process_specification spec;
      if (input_filename().empty())
      {
        //parse specification from stdin
        mCRL2log(mcrl2::log::verbose) << "reading input from stdin..." << std::endl;
        spec = mcrl2::process::parse_process_specification(std::cin, !noalpha);
      }
      else
      {
        //parse specification from infilename
        mCRL2log(mcrl2::log::verbose) << "reading input from file '"
                                      <<  input_filename() << "'..." << std::endl;
        std::ifstream instream(input_filename().c_str(), std::ifstream::in|std::ifstream::binary);
        if (!instream.is_open())
        {
          throw mcrl2::runtime_error("cannot open input file: " + input_filename());
        }
        spec = mcrl2::process::parse_process_specification(instream, !noalpha);
        instream.close();
      }
      //report on well-formedness (if needed)
      if (opt_check_only)
      {
        if (input_filename().empty())
        {
          mCRL2log(mcrl2::log::info) << "stdin contains a well-formed mCRL2 specification"
                                     << std::endl;
        }
        else
        {
          mCRL2log(mcrl2::log::info) << "the file '" << input_filename()
                                     << "' contains a well-formed mCRL2 specification" << std::endl;
        }
        return true;
      }
      //store the result
      mcrl2::lps::specification linear_spec(mcrl2::lps::linearise(spec, m_linearisation_options));
      mCRL2log(mcrl2::log::verbose) << "writing LPS to "
                                    << (output_filename().empty() ? "stdout"
                                                                  : "file" + output_filename())
                                    << "..." << std::endl;
      mcrl2::lps::save_lps(linear_spec, output_filename());
      return true;
    }
};

int main(int argc, char** argv)
{
  return mcrl22lps_tool().execute(argc, argv);
}

