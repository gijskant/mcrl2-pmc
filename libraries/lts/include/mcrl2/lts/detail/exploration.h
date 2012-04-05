// Author(s): Ruud Koolen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MCRL2_LTS_DETAIL_EXPLORATION_NEW_H
#define MCRL2_LTS_DETAIL_EXPLORATION_NEW_H

#include <string>
#include <limits>
#include <memory>

#include "mcrl2/atermpp/indexed_set.h"
#include "mcrl2/data/rewrite_strategy.h"
#include "mcrl2/lps/next_state_generator.h"
#include "mcrl2/lps/nextstate/nextstate_options.h"
#include "mcrl2/lts/lts_lts.h"
#include "mcrl2/lts/detail/bithashtable.h"
#include "mcrl2/lts/detail/queue.h"
#include "mcrl2/lts/detail/lts_generation_options.h"
#include "mcrl2/atermpp/list.h"

#include "workarounds.h"

namespace mcrl2
{

namespace lts
{

class lps2lts_algorithm: public lps2lts_algorithm_base
{
  typedef lps::next_state_generator next_state_generator;
  typedef next_state_generator::internal_state_t state_t;
  typedef lps::multi_action action_label_t;

  struct state_info
  {
    state_t state;
    atermpp::list<next_state_generator::transition_t> transitions;
  };
  friend struct atermpp::aterm_traits<state_info>;

  private:
    lts_generation_options m_options;
    next_state_generator *m_generator;
    next_state_generator *m_confluence_generator;
    next_state_generator::substitution_t m_substitution;

    atermpp::indexed_set m_state_numbers;
    bit_hash_table m_bit_hash_table;

    lts_lts_t m_output_lts;
    atermpp::indexed_set m_action_label_numbers;
    std::ofstream m_aut_file;

    bool m_maintain_traces;
    bool m_value_prioritize;

    std::vector<size_t> m_tau_summands;

    atermpp::map<state_t, state_t> m_backpointers;
    size_t m_traces_saved;

    size_t m_num_states;
    size_t m_num_transitions;
    size_t m_level;

    volatile bool m_must_abort;

  public:
    lps2lts_algorithm() :
      m_generator(0),
      m_confluence_generator(0),
      m_must_abort(false)
    {
    }

    virtual ~lps2lts_algorithm()
    {
      delete m_generator;
      delete m_confluence_generator;
    }

    virtual bool initialise_lts_generation(lts_generation_options* options);
    virtual bool generate_lts();
    virtual bool finalise_lts_generation();

    virtual void abort()
    {
      // Stops the exploration algorithm if it is running by making sure
      // not a single state can be generated anymore.
      if (!m_must_abort)
      {
        m_must_abort = true;
        mCRL2log(log::warning) << "state space generation was aborted prematurely" << std::endl;
      }
    }

  private:
    state_t get_prioritised_representative(state_t state);
    void value_prioritize(atermpp::list<next_state_generator::transition_t> &transitions);
    bool save_trace(state_t state, std::string filename);
    bool search_divergence(state_t state, std::set<state_t> &current_path, atermpp::set<state_t> &visited);
    void check_divergence(state_t state);
    void check_action(state_t state, next_state_generator::transition_t &transition);
    void save_deadlock(state_t state);
    void save_error(state_t state);
    bool add_transition(state_t state, next_state_generator::transition_t &transition);
    atermpp::list<next_state_generator::transition_t> get_transitions(state_t state);

    void generate_lts_breadth(state_t initial_state);
    void generate_lts_breadth_bithashing(state_t initial_state);
    void generate_lts_depth(state_t initial_state);
    void generate_lts_random(state_t initial_state);
};

} // namespace lps

} // namespace mcrl2

namespace atermpp
{
  template<> struct aterm_traits<mcrl2::lts::lps2lts_algorithm::state_info>
  {
    static void protect(const mcrl2::lts::lps2lts_algorithm::state_info &state_info) { state_info.state.protect(); }
    static void unprotect(const mcrl2::lts::lps2lts_algorithm::state_info &state_info) { state_info.state.unprotect(); }
    static void mark(const mcrl2::lts::lps2lts_algorithm::state_info &state_info) { state_info.state.mark(); }
  };
}

#endif // MCRL2_LTS_DETAIL_EXPLORATION_H
