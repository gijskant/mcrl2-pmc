// Author(s): Luc Engelen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING).
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file source/bdd2dot.cpp
/// \brief Add your file description here.

// Implementation of class BDD2Dot
// file: bdd2dot.cpp

#include "mcrl2/core/struct.h"
#include "mcrl2/core/print.h"
#include "mcrl2/utilities/bdd2dot.h"
#include "mcrl2/core/messaging.h"

#ifdef __cplusplus
using namespace ::mcrl2::utilities;
using namespace mcrl2::core;
#endif

// Class BDD2Dot --------------------------------------------------------------------------------
  // Class BDD2Dot - Functions declared private -------------------------------------------------

    /// Writes the bdd it receives to BDD2Dot::f_dot_file.
    /// \param a_bdd A binary decision diagram.

    void BDD2Dot::aux_output_bdd(ATermAppl a_bdd) {
      ATermAppl v_true_branch, v_false_branch, v_guard;
      int v_true_number, v_false_number;

      if (ATtableGet(f_visited, (ATerm) a_bdd)) {
        return;
      }

      if (f_bdd_info.is_true(a_bdd)) {
        fprintf(f_dot_file, "  %d [shape=box, label=\"T\"];\n", f_node_number);
      } else if (f_bdd_info.is_false(a_bdd)) {
        fprintf(f_dot_file, "  %d [shape=box, label=\"F\"];\n", f_node_number);
      } else if (f_bdd_info.is_if_then_else(a_bdd)) {
        v_true_branch = f_bdd_info.get_true_branch(a_bdd);
        v_false_branch = f_bdd_info.get_false_branch(a_bdd);
        aux_output_bdd(v_true_branch);
        aux_output_bdd(v_false_branch);
        v_true_number = ATgetInt((ATermInt) ATtableGet(f_visited, (ATerm) v_true_branch));
        v_false_number = ATgetInt((ATermInt) ATtableGet(f_visited, (ATerm) v_false_branch));
        v_guard = f_bdd_info.get_guard(a_bdd);
        gsfprintf(f_dot_file, "  %d [label=\"%P\"];\n", f_node_number, v_guard);
        fprintf(f_dot_file, "  %d -> %d;\n", f_node_number, v_true_number);
        fprintf(f_dot_file, "  %d -> %d [style=dashed];\n", f_node_number, v_false_number);
      } else {
        gsfprintf(f_dot_file, "  %d [shape=box, label=\"%P\"];\n", f_node_number, a_bdd);
      }
      ATtablePut(f_visited, (ATerm) a_bdd, (ATerm) ATmakeInt(f_node_number++));
    }

  // Class BDD2Dot - Functions declared public --------------------------------------------------

    /// Initializes the fields of the class and writes the received BDD to a file with the name
    /// a_file_name.
    /// \param a_bdd A binary decision diagram.
    /// \param a_file_name A file name.

    void BDD2Dot::output_bdd(ATermAppl a_bdd, char const* a_file_name) {
      f_visited = ATtableCreate(200, 75);
      f_node_number = 0;
      f_dot_file = fopen(a_file_name, "w");
      fprintf(f_dot_file, "digraph BDD {\n");
      aux_output_bdd(a_bdd);
      fprintf(f_dot_file, "}\n");
      fclose(f_dot_file);
      ATtableDestroy(f_visited);
    }
