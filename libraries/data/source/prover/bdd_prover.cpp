// Implementation of class BDD_Prover
// file: bdd_prover.cpp

#include "aterm2.h"
#include "mcrl2/data/rewrite.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/prover.h"
#include "mcrl2/data/prover/bdd_prover.h"
#include "mcrl2/utilities/utilities.h"
#include "libstruct.h"
#include "time.h"
#include <cstdlib>
#include "mcrl2/data/prover/induction.h"
#include "print/messaging.h"
#include "mcrl2/utilities/aterm_ext.h"

#ifdef __cplusplus
using namespace ::mcrl2::utilities;
#endif

// Class BDD_Prover -------------------------------------------------------------------------------
  // Class BDD_Prover - Functions declared private ------------------------------------------------

    void BDD_Prover::build_bdd() {
      f_formula_to_bdd = ATtableCreate(60000, 25);
      f_smallest = ATtableCreate(2000, 50);
      f_deadline = time(0) + f_time_limit;

      ATerm v_previous_1 = 0;
      ATerm v_previous_2 = 0;

      gsDebugMsg("Formula: %P\n", f_formula);

      f_internal_bdd = f_rewriter->toRewriteFormat(f_formula);
      f_internal_bdd = f_rewriter->rewriteInternal(f_internal_bdd);
      f_internal_bdd = f_manipulator->orient(f_internal_bdd);

      gsDebugMsg("Formula rewritten and oriented: %P\n", f_rewriter->fromRewriteFormat(f_internal_bdd));

      while (v_previous_1 != f_internal_bdd && v_previous_2 != f_internal_bdd) {
        v_previous_2 = v_previous_1;
        v_previous_1 = f_internal_bdd;
        f_internal_bdd = bdd_down(f_internal_bdd);
        gsDebugMsg("End of iteration.\n");
        gsDebugMsg("Intermediate BDD: %P\n", f_rewriter->fromRewriteFormat(f_internal_bdd));
      }

      f_bdd = f_rewriter->fromRewriteFormat(f_internal_bdd);
      gsDebugMsg("Resulting BDD: %P\n", f_bdd);

      ATtableDestroy(f_formula_to_bdd);
      ATtableDestroy(f_smallest);
    }

    // --------------------------------------------------------------------------------------------

    ATerm BDD_Prover::bdd_down(ATerm a_formula) {
      if (f_time_limit != 0 && (f_deadline - time(0)) <= 0) {
        gsDebugMsg("The time limit has passed.\n");
        return a_formula;
      }

      if (f_info->is_true(a_formula)) {
        return a_formula;
      }
      if (f_info->is_false(a_formula)) {
        return a_formula;
      }

      ATerm v_bdd;

      v_bdd = ATtableGet(f_formula_to_bdd, a_formula);
      if (v_bdd) {
        return v_bdd;
      }

      ATerm v_guard;

      v_guard = smallest(a_formula);
      if (!v_guard) {
        return a_formula;
      } else {
        gsDebugMsg("%sSmallest guard: %P\n", f_indent.blank_spaces, f_rewriter->fromRewriteFormat(v_guard));
      }

      ATerm v_term1, v_term2;

      v_term1 = f_manipulator->set_true(a_formula, v_guard);
      v_term1 = f_rewriter->rewriteInternal(v_term1);
      v_term1 = f_manipulator->orient(v_term1);
      gsDebugMsg("%sTrue-branch after rewriting and orienting: %P\n", f_indent.blank_spaces, f_rewriter->fromRewriteFormat(v_term1));
      f_indent.indent();
      v_term1 = bdd_down(v_term1);
      f_indent.deindent();
      gsDebugMsg("%sBDD of the true-branch: %P\n", f_indent.blank_spaces, f_rewriter->fromRewriteFormat(v_term1));

      v_term2 = f_manipulator->set_false(a_formula, v_guard);
      v_term2 = f_rewriter->rewriteInternal(v_term2);
      v_term2 = f_manipulator->orient(v_term2);
      gsDebugMsg("%sFalse-branch after rewriting and orienting: %P\n", f_indent.blank_spaces, f_rewriter->fromRewriteFormat(v_term2));
      f_indent.indent();
      v_term2 = bdd_down(v_term2);
      f_indent.deindent();
      gsDebugMsg("%sBDD of the false-branch: %P\n", f_indent.blank_spaces, f_rewriter->fromRewriteFormat(v_term2));

      v_bdd = f_manipulator->make_reduced_if_then_else(v_guard, v_term1, v_term2);
      ATtablePut(f_formula_to_bdd, a_formula, v_bdd);
      return v_bdd;
    }

    // --------------------------------------------------------------------------------------------

    ATerm BDD_Prover::smallest(ATerm a_formula) {
      if (f_info->is_variable(a_formula)) {
        if (f_info->has_type_bool(a_formula)) {
          return a_formula;
        } else {
          return 0;
        }
      }
      if (f_info->is_true(a_formula) || f_info->is_false(a_formula)) {
        return 0;
      }

      ATerm v_result;

      v_result = ATtableGet(f_smallest, a_formula);
      if (v_result) {
        return v_result;
      }

      int i;
      int v_length;
      ATerm v_small;

      v_length = f_info->get_number_of_arguments(a_formula);

      for (i = 0; i < v_length; i++) {
        v_small = smallest(f_info->get_argument(a_formula, i));
        if (v_small) {
          if (v_result) {
            if (f_info->lpo1(v_result, v_small)) {
              v_result = v_small;
            }
          } else {
            v_result = v_small;
          }
        }
      }
      if (!v_result && f_info->has_type_bool(a_formula)) {
        v_result = a_formula;
      }
      if (v_result) {
        ATtablePut(f_smallest, a_formula, v_result);
      }

      return v_result;
    }

    // --------------------------------------------------------------------------------------------

    void BDD_Prover::eliminate_paths() {
      int v_new_time_limit;

      v_new_time_limit = f_deadline - time(0);
      if (v_new_time_limit > 0 || f_time_limit == 0) {
        gsDebugMsg("Simplifying the BDD:\n");
        f_bdd_simplifier->set_time_limit(std::max(v_new_time_limit, 0));
        f_bdd = f_bdd_simplifier->simplify(f_bdd);
        gsDebugMsg("Resulting BDD: %P\n", f_bdd);
      }
    }

    // --------------------------------------------------------------------------------------------

    void BDD_Prover::update_answers() {
      ATermAppl v_original_formula;
      ATermAppl v_original_bdd;

      if (!f_processed) {
        build_bdd();
        eliminate_paths();
        v_original_formula = f_formula;
        v_original_bdd = f_bdd;
        if (f_apply_induction && !(f_bdd_info.is_true(f_bdd) || f_bdd_info.is_false(f_bdd))) {
          f_induction.initialize(v_original_formula);
          while (f_induction.can_apply_induction() && !f_bdd_info.is_true(f_bdd)) {
            gsDebugMsg("Applying induction.\n");
            f_formula = f_induction.apply_induction();
            build_bdd();
            eliminate_paths();
          }
          if (f_bdd_info.is_true(f_bdd)) {
            f_tautology = answer_yes;
            f_contradiction = answer_no;
          } else {
            v_original_formula = gsMakeDataExprNot(v_original_formula);
            f_bdd = v_original_bdd;
            f_induction.initialize(v_original_formula);
            while (f_induction.can_apply_induction() && !f_bdd_info.is_true(f_bdd)) {
              gsDebugMsg("Applying induction on the negated formula.\n");
              f_formula = f_induction.apply_induction();
              build_bdd();
              eliminate_paths();
            }
            if (f_bdd_info.is_true(f_bdd)) {
              f_bdd = gsMakeDataExprFalse();
              f_tautology = answer_no;
              f_contradiction = answer_yes;
            } else {
              f_bdd = v_original_bdd;
              f_tautology = answer_undefined;
              f_contradiction = answer_undefined;
            }
          }
        } else {
          if (f_bdd_info.is_true(f_bdd)) {
            f_tautology = answer_yes;
            f_contradiction = answer_no;
          } else if (f_bdd_info.is_false(f_bdd)) {
            f_tautology = answer_no;
            f_contradiction = answer_yes;
          } else {
            f_tautology = answer_undefined;
            f_contradiction = answer_undefined;
          }
        }
        f_processed = true;
      }
    }

    // --------------------------------------------------------------------------------------------

    ATermAppl BDD_Prover::get_branch(ATermAppl a_bdd, bool a_polarity) {
      ATermAppl v_result;

      if (f_bdd_info.is_if_then_else(a_bdd)) {
        ATermAppl v_guard = f_bdd_info.get_guard(a_bdd);
        ATermAppl v_true_branch = f_bdd_info.get_true_branch(a_bdd);
        ATermAppl v_false_branch = f_bdd_info.get_false_branch(a_bdd);
        ATermAppl v_branch = get_branch(v_true_branch, a_polarity);
        if (v_branch == 0) {
          v_branch = get_branch(v_false_branch, a_polarity);
          if (v_branch == 0) {
            v_result = 0;
          } else {
            ATermAppl v_term = gsMakeDataExprNot(v_guard);
            v_result = gsMakeDataExprAnd(v_branch, v_term);
          }
        } else {
          v_result = gsMakeDataExprAnd(v_branch, v_guard);
        }
      } else {
        if ((f_bdd_info.is_true(a_bdd) && a_polarity) || (f_bdd_info.is_false(a_bdd) && !a_polarity)) {
          v_result = gsMakeOpIdTrue();
        } else {
          v_result = 0;
        }
      }
      return v_result;
    }

  // Class BDD_Prover - Functions declared public -------------------------------------------------

    BDD_Prover::BDD_Prover(
      lps::data_specification data_spec, RewriteStrategy a_rewrite_strategy, int a_time_limit, bool a_path_eliminator, SMT_Solver_Type a_solver_type, bool a_apply_induction
    ):
      Prover(data_spec, a_rewrite_strategy, a_time_limit),
      f_induction(data_spec)
    {
      f_reverse = true;
      f_full = true;
      f_apply_induction = a_apply_induction;
      f_info->set_reverse(f_reverse);
      f_info->set_full(f_full);
      gsDebugMsg(
        "Flags:\n"
        "  Reverse: %s,\n"
        "  Full: %s,\n",
        bool_to_char_string(f_reverse),
        bool_to_char_string(f_full)
      );
      f_data_spec = data_spec;
      if (a_path_eliminator) {
        f_bdd_simplifier = new BDD_Path_Eliminator(a_solver_type);
      } else {
        f_bdd_simplifier = new BDD_Simplifier();
      }
    }

    // --------------------------------------------------------------------------------------------

    BDD_Prover::~BDD_Prover() {
      delete f_bdd_simplifier;
      f_bdd_simplifier = 0;
    }

    // --------------------------------------------------------------------------------------------

    Answer BDD_Prover::is_tautology() {
      update_answers();
      return f_tautology;
    }

    // --------------------------------------------------------------------------------------------

    Answer BDD_Prover::is_contradiction() {
      update_answers();
      return f_contradiction;
    }

    // --------------------------------------------------------------------------------------------

    ATermAppl BDD_Prover::get_bdd() {
      update_answers();
      return f_bdd;
    }

    // --------------------------------------------------------------------------------------------

    ATermAppl BDD_Prover::get_witness() {
      ATermAppl v_result;

      update_answers();
      if (!(is_contradiction() == answer_yes) && !(is_tautology() == answer_yes)) {
        gsDebugMsg("The formula appears to be satisfiable.\n");
        v_result = get_branch(f_bdd, true);
      } else {
        gsDebugMsg("The formula is a contradiction or a tautology.\n");
        v_result = 0;
      }
      return v_result;
    }

    // --------------------------------------------------------------------------------------------

    ATermAppl BDD_Prover::get_counter_example() {
      ATermAppl v_result;

      update_answers();
      if (!(is_contradiction() == answer_yes) && !(is_tautology() == answer_yes)) {
        gsDebugMsg("The formula appears to be satisfiable.\n");
        v_result = get_branch(f_bdd, false);
      } else {
        gsDebugMsg("The formula is a contradiction or a tautology.\n");
        v_result = 0;
      }
      return v_result;
    }
