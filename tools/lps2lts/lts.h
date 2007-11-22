// Author(s): Muck van Weerdenburg
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lts.h

#ifndef _LTS_H
#define _LTS_H

#include <aterm2.h>
#include <mcrl2/lps/nextstate.h>

#define OF_UNKNOWN  0
#define OF_AUT      1
#define OF_SVC      2
	
typedef struct {
  int outformat;
  bool outinfo;
  NextState *nstate;
} lts_options;

void open_lts(const char *filename, lts_options &opts);
void save_initial_state(unsigned long long idx, ATerm state);
void save_transition(unsigned long long idx_from, ATerm from, ATermAppl action, unsigned long long idx_to, ATerm to);
void close_lts(unsigned long long num_states, unsigned long long num_trans);
void remove_lts();

#endif
