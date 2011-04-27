// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef MCRL2_LTS_DETAIL_EXPLORATION_H
#define MCRL2_LTS_DETAIL_EXPLORATION_H

#include <string>
#include <limits>
#include <memory>

#include "boost/bind.hpp"
#include "boost/function.hpp"

#include "mcrl2/aterm/aterm2.h"
#include "mcrl2/atermpp/indexed_set.h"
#include "mcrl2/lts/detail/lps2lts_lts.h"
#include "mcrl2/lts/detail/bithashtable.h"
#include "mcrl2/lts/detail/queue.h"
#include "mcrl2/lps/nextstate.h"

#include "workarounds.h"

namespace mcrl2
{
namespace lts
{

#define DEFAULT_MAX_STATES ULONG_MAX
#define DEFAULT_MAX_TRACES ULONG_MAX
#define DEFAULT_BITHASHSIZE 209715200ULL // ~25 MB
#define DEFAULT_INIT_TSIZE 10000UL

enum exploration_strategy { es_none,
                            es_breadth,
                            es_depth,
                            es_random,
                            es_value_prioritized,
                            es_value_random_prioritized
                          };

exploration_strategy str_to_expl_strat(const std::string s);
const std::string expl_strat_to_str(exploration_strategy es);

struct lts_generation_options
{
  lts_generation_options() :
    strat(mcrl2::data::rewriter::jitty),
    usedummies(true),
    removeunused(true),
    stateformat(GS_STATE_VECTOR),
    outformat(mcrl2::lts::lts_none),
    outinfo(true),
    suppress_progress_messages(false),
    max_states(DEFAULT_MAX_STATES),
    trace(false),
    max_traces(DEFAULT_MAX_TRACES),
    detect_deadlock(false),
    detect_divergence(false),
    detect_action(false),
    save_error_trace(false),
    expl_strat(es_breadth),
    bithashing(false),
    bithashsize(DEFAULT_BITHASHSIZE),
    todo_max((std::numeric_limits< size_t >::max)()),
    initial_table_size(DEFAULT_INIT_TSIZE)
  {
    generate_filename_for_trace = boost::bind(&lts_generation_options::generate_trace_file_name, this, _1, _2, _3);
  }



  /* Method that takes an info string and an extension to produce a unique filename */
  boost::function< std::string(std::string const&, std::string const&, std::string const&) >
  generate_filename_for_trace;

  /* Default function for generate_filename_for_trace */
  std::string generate_trace_file_name(std::string const& basefilename, std::string const& info, std::string const& extension)
  {
    return basefilename + std::string("_") + info + std::string(".") + extension;
  }

  mcrl2::data::rewriter::strategy strat;
  bool usedummies;
  bool removeunused;
  int stateformat;
  mcrl2::lts::lts_type outformat;
  bool outinfo;
  bool suppress_progress_messages;
  size_t max_states;
  std::string priority_action;
  bool trace;
  atermpp::set < mcrl2::core::identifier_string > trace_actions;
  size_t max_traces;
  bool detect_deadlock;
  bool detect_divergence;
  bool detect_action;
  bool save_error_trace;
  exploration_strategy expl_strat;
  bool bithashing;
  size_t bithashsize;
  size_t todo_max;
  size_t initial_table_size;
  std::auto_ptr< mcrl2::data::rewriter > m_rewriter;
  mcrl2::lps::specification specification;
  std::string trace_prefix;
  std::string lts;
};

class lps2lts_algorithm
{
    typedef atermpp::aterm state_t; // Type of a state.

  private:
    // lps2lts_algorithm may be initialised only once
    bool initialised;
    bool premature_termination_handler_called;
    bool finalised;
    bool completely_generated;

    lts_generation_options* lgopts;
    NextState* nstate;
    atermpp::indexed_set states;
    lps2lts_lts lts;

    size_t num_states;
    size_t trans;
    size_t level;
    size_t num_found_same;
    size_t current_state;
    size_t initial_state;

    atermpp::map<atermpp::aterm,atermpp::aterm> backpointers;
    bit_hash_table bithash_table;

    size_t tracecnt;

    bool lg_error;

    bool apply_confluence_reduction;
    NextStateGenerator* repr_nsgen;

  public:
    lps2lts_algorithm() :
      initialised(false),
      premature_termination_handler_called(false),
      finalised(false),
      completely_generated(false),
      states(0,0), //Table of size 0 initially
      bithash_table(),
      lg_error(false)
    {
    }

    ~lps2lts_algorithm()
    {
      if (initialised && !finalised)
      {
        finalise_lts_generation();
      }
    }

    bool initialise_lts_generation(lts_generation_options* opts);
    bool generate_lts();
    bool finalise_lts_generation();

    void premature_termination_handler()
    {
      if (!premature_termination_handler_called)
      {
        premature_termination_handler_called = true;
        std::cerr << "Warning: state space generation was aborted prematurely" << std::endl;
        if (initialised && !finalised)
        {
          finalise_lts_generation();
        }
      }
    }


  private:

    void initialise_representation(bool confluence_reduction);
    void cleanup_representation();
    bool search_divergence_recursively(
      const state_t current_state,
      std::set < state_t > &on_current_depth_first_path,
      atermpp::set<state_t> &repr_visited);
    void check_divergence(const state_t state);
    state_t get_repr(const state_t state);

    // trace functions
    bool occurs_in(atermpp::aterm_appl const& name, atermpp::term_list< atermpp::aterm_appl > const& ma);
    bool savetrace(std::string const& info,
                   const state_t state,
                   NextState* nstate,
                   const state_t extra_state = state_t(),
                   ATermAppl extra_transition = NULL);
    void check_actiontrace(const state_t OldState, ATermAppl Transition, const state_t NewState);
    void save_error_trace(const state_t state);
    void check_deadlocktrace(const state_t state);

    size_t add_state(const state_t state, bool& is_new);
    size_t state_index(const state_t state);

    // Main routine
    bool add_transition(const state_t from, ATermAppl action, const state_t to);
};

}
}

#endif // MCRL2_LTS_DETAIL_EXPLORATION_H
