// Author(s): Muck van Weerdenburg, Jan Friso Groote
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

// This file contains the merge algorithm that merges two lts's.
// Merges an LTS L with this LTS (say K) and stores the resulting LTS
// (say M) in this LTS datastructure, effectively replacing K.
// Conceptually, we just take the union of the sets of states and the
// sets of transitions of K and L:
//   States_M      = States_K + States_L
//   Transitions_M = Transitions_K + Transitions_L
// where + denotes set union.
// However, this assumes that States_K and States_L are disjoint,
// which is generally not the case. More specifically we have:
//   States_K = { 0, ..., N_K - 1 }   and
//   States_L = { 0, ..., N_L - 1 }
// for some N_K, N_L > 0.
// Therefore, state i of L will be numbered |N_K| + i in the resulting
// LTS M and state i of K will be numbered i in M. This yields:
//   States_M = { 0, ..., N_K + N_L - 1 }.


#ifndef MCRL2_LTS_LIBLTS_MERGE_H
#define MCRL2_LTS_LIBLTS_MERGE_H

#include "mcrl2/atermpp/map.h"
#include "mcrl2/lts/lts.h"

namespace mcrl2
{
namespace lts
{
namespace detail
{

template <class LTS_TYPE>
void merge(LTS_TYPE &l1, const LTS_TYPE &l2)
{ 
  const size_t old_nstates=l1.num_states();
  l1.set_num_states(l1.num_states() + l2.num_states());
  
  // The resulting LTS will have state information only if BOTH LTSs
  // currently have state information.
  if ( l1.has_state_info() && l2.has_state_info() )
  {
    for (size_t i=0; i<l2.num_states(); ++i)
    {
      l1.add_state(l2.state_label(i));
    }
  }
  else
  {
    // remove state information from this LTS, if any
    l1.clear_state_labels();
  }

  // Before we can set the label data in a new transitions
  // array, we first have to collect the labels of both LTSs in a
  // map, of which the second element indicates the new index of each action label.
  
  typedef typename LTS_TYPE::action_label_t type1;
  typedef typename LTS_TYPE::labels_size_type type2;
  atermpp::map < type1,type2 > labs;

  // Put the labels of the LTS l1 in a map.
  for (size_t i = 0; i < l1.num_action_labels(); ++i)
  {
    labs.insert(std::pair <typename LTS_TYPE::action_label_t,typename LTS_TYPE::labels_size_type> 
             (l1.action_label(i),i));
  }
  // Add the labels for the LTS l2, and put them there with a new index if it was
  // not added yet.
  
  for (size_t i=0; i<l2.num_action_labels(); ++i)
  {
    if (labs.insert(std::pair <type1,type2> 
             (l2.action_label(i),l1.num_action_labels())).second)
    {
      l1.add_action(l2.action_label(i),l2.is_tau(i));
    }
  }

  // Update the label numbers of all transitions of the LTS l1 to reflect
  // the new indices as given by labs.
  for (transition_range r = l1.get_transitions(); !r.empty(); r.advance_begin(1))
  {
    r.front().set_label(labs[l1.action_label(r.front().label())]);
  }
    
  // Now add the transition labels of LTS l2
  // Now add the source and target states of the transitions of LTS l2.
  // The labels will be added below, depending on whether there is label
  // information in both LTSs.
  for (transition_const_range r = l2.get_transitions(); !r.empty(); r.advance_begin(1))
  { const transition transition_to_add=r.front();
    l1.add_transition(transition(transition_to_add.from()+old_nstates,
                                labs[l2.action_label(transition_to_add.label())],
                                transition_to_add.to()+old_nstates));
  }
}
} // namespace detail
} // namespace lts
} // namespace mcrl2


#endif
