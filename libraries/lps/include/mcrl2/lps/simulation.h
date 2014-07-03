// Author(s): Ruud Koolen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MCRL2_LPS_SIMULATION_H
#define MCRL2_LPS_SIMULATION_H

#include <string>
#include <deque>

#include "mcrl2/data/rewriter.h"
#include "mcrl2/lps/multi_action.h"
#include "mcrl2/lps/next_state_generator.h"
#include "mcrl2/lps/specification.h"
// #include "mcrl2/lps/state.h"

namespace mcrl2
{

namespace lps
{

/// \brief Simulation process.
// A simulation is effectively a trace annotated with outgoing transition information
// and an operation to extend the trace with an outgoing transition from the last state.
class simulation
{
  public:

    typedef atermpp::term_balanced_tree<data::data_expression> state;
    struct transition_t
    {
      state destination;
      multi_action action;
    };

    struct state_t
    {
      state source_state;
      std::vector<transition_t> transitions;
      size_t transition_number; // Undefined for the last state in the trace
    };

    /// Constructor.
    simulation(const specification& specification, data::rewrite_strategy strategy = data::rewrite_strategy());

    /// Returns the current annotated state vector.
    const std::deque<state_t> &trace() const { return m_tau_prioritization ? m_prioritized_trace : m_full_trace; }

    /// Remove states from the end of the simulation, making \a state_number the last state.
    void truncate(size_t state_number);

    /// Choose outgoing transition \a transition_number and add its state to the state vector.
    void select(size_t transition_number);

    /// If enabled, tau prioritization is applied to all outgoing transitions, and in-between states are hidden from the state vector.
    void enable_tau_prioritization(bool enable, std::string action = "ctau");

    /// Save the trace to a file.
    void save(const std::string &filename);

    /// Load a trace from a file.
    void load(const std::string &filename);

  private:
    std::vector<transition_t> transitions(state source_state);
    std::vector<transition_t> prioritize(const std::vector<transition_t> &transitions);
    bool is_prioritized(const multi_action &action);
    void prioritize_trace();
    std::deque<state_t> match_trace(std::deque<state_t> trace, const std::vector<transition_t> &transitions, size_t transition_number);
    bool match(const state &left, const state &right);

  private:
    specification m_specification;
    data::rewriter m_rewriter;
    next_state_generator m_generator;
    next_state_generator::substitution_t m_substitution;

    bool m_tau_prioritization;
    std::string m_prioritized_action;

    // The complete trace.
    std::deque<state_t> m_full_trace;
    // The trace with all prioritized in-between states removed.
    std::deque<state_t> m_prioritized_trace;
    std::deque<size_t> m_prioritized_originals;
};

} // namespace lps

} // namespace mcrl2

#endif
