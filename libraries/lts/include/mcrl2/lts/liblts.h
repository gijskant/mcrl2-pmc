// Author(s): Muck van Weerdenburg
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lts/liblts.h

#ifndef __LIBLTS_H
#define __LIBLTS_H

#include <string>
#include <vector>
#include <iostream>
#include <aterm2.h>
#include "mcrl2/lps/specification.h"
#include "mcrl2/setup.h"

#ifdef MCRL2_BCG
#include "bcg_user.h"
#endif

namespace mcrl2
{
namespace lts
{

  enum lts_type { lts_none, lts_mcrl2, lts_aut, lts_mcrl, lts_svc, lts_fsm, lts_dot
#ifdef MCRL2_BCG
                , lts_bcg
#endif
                };

  enum lts_equivalence { lts_eq_none, lts_eq_trace, lts_eq_strong, lts_eq_weak_trace, lts_eq_branch, lts_eq_isomorph };
  typedef struct
  {
    struct {
      bool add_class_to_state;
      std::vector<std::string> tau_actions;
    } reduce;
  } lts_eq_options;

  enum lts_extra_type { le_nothing, le_mcrl1, le_mcrl2, le_dot };
  typedef struct
  {
    std::string const *name;
    bool print_states;
  } lts_dot_options;

  void set_eq_options_defaults(lts_eq_options &opts);
  void lts_reduce_add_tau_actions(lts_eq_options &opts, std::string act_names);

  bool is_timed_pair(ATermAppl t);
  ATermAppl make_timed_pair(ATermAppl action, ATermAppl time = NULL);

  #include "detail/liblts_private.h"
  
  class lts_extra : p_lts_extra
  {
    public:
      lts_extra();
      lts_extra(ATerm t);
      lts_extra(lps::specification *spec);
      lts_extra(lts_dot_options opts);

      lts_extra_type get_type();
      ATerm get_mcrl1_spec();
      lps::specification *get_mcrl2_spec();
      lts_dot_options get_dot_options();
  };
  extern lts_extra lts_no_extra;

  class lts;

  class state_iterator : p_state_iterator
  {
    public:
      state_iterator(lts *l);

      bool more();

      unsigned int operator *();
      void operator ++();
  };

  class label_iterator : p_label_iterator
  {
    public:
      label_iterator(lts *l);
      
      bool more();

      unsigned int operator *();
      void operator ++();
  };

  class transition_iterator : p_transition_iterator
  {
    public:
      transition_iterator(lts *l);
      
      bool more();

      unsigned int from();
      unsigned int label();
      unsigned int to();

      void operator ++();
  };

  class lts : p_lts
  {
    public:

      /** \brief Helper function to guess the lts format from a filename (by its extension) */
      static lts_type guess_format(std::string const& s);

      /** \brief Helper function to get storage format from a storage format specification */
      static lts_type parse_format(char const* s);

      /** \brief Helper function to get storage format for a given file extension */
      static char const* string_for_type(const lts_type s);

      /** \brief Helper function to get the extension associated to an element (of lts_type) */
      static char const* extension_for_type(const lts_type type);

      /** \brief Helper function to get equivalence type from a string */
      static lts_equivalence parse_equivalence(char const* s);
      
      /** \brief Helper function to get string from equivalence type */
      static char const* string_for_equivalence(const lts_equivalence s);

      /** \brief Helper function to get (descriptive) name from equivalence type */
      static char const* name_of_equivalence(const lts_equivalence s);

    public:
      lts(lts_type type = lts_mcrl2, bool state_info = true, bool label_info = true);
      lts(std::string &filename, lts_type type = lts_none);
      lts(std::istream &is, lts_type type = lts_none);
      ~lts();

      void reset(lts_type type = lts_mcrl2, bool state_info = true, bool label_info = true);

      bool read_from(std::string const& filename, lts_type type = lts_none, lts_extra extra = lts_no_extra);
      bool read_from(std::istream &is, lts_type type = lts_none, lts_extra extra = lts_no_extra);
      bool write_to(std::string const& filename, lts_type type = lts_mcrl2, lts_extra extra = lts_no_extra);
      bool write_to(std::ostream &os, lts_type type = lts_mcrl2, lts_extra extra = lts_no_extra);

      unsigned int num_states();
      unsigned int num_transitions();
      unsigned int num_labels();
      
      unsigned int initial_state();
      void set_initial_state(unsigned int state);
      
      unsigned int add_state(ATerm value = NULL);
      unsigned int add_label(ATerm value = NULL, bool is_tau = false);
      unsigned int add_label(bool is_tau);
      unsigned int add_transition(unsigned int from,
                                  unsigned int label,
                                  unsigned int to);

      void set_state(unsigned int state, ATerm value);
      void set_label(unsigned int label, ATerm value, bool is_tau = false);

      ATerm state_value(unsigned int state);
      ATerm label_value(unsigned int label);
      std::string state_value_str(unsigned int state);
      std::string label_value_str(unsigned int label);
      unsigned int transition_from(unsigned int transition);
      unsigned int transition_label(unsigned int transition);
      unsigned int transition_to(unsigned int transition);

      state_iterator get_states();
      label_iterator get_labels();
      transition_iterator get_transitions();

      bool is_tau(unsigned int label);
      void set_tau(unsigned int label, bool is_tau = true);

      bool has_creator();
      std::string get_creator();
      void set_creator(std::string creator);
      
      lts_type get_type();

      bool has_state_info();
      bool has_label_info();

      void remove_state_values();

      void sort_transitions();
      unsigned int* get_transition_indices();
      
      bool reduce(lts_equivalence eq, lts_eq_options &opts);
      bool compare(lts &l, lts_equivalence eq, lts_eq_options &opts);
      void determinise();
      /**
       * \brief  Check whether or not all states in the LTS are reachable from
       *         the initial state. (Worst case: O(nstates*ntransitions))
       *
       * \return  true iff all states are reachable from the initial state
       **/
      bool reachability_check();
      bool is_deterministic();

      friend class state_iterator;
      friend class label_iterator;
      friend class transition_iterator;
  };

}
}

#endif
