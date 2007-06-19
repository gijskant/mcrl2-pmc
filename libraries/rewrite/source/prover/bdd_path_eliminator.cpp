// Implementation of class BDD_Path_Eliminator
// file: bdd_path_eliminator.cpp

#include "prover/bdd_path_eliminator.h"
#include "libstruct.h"
#include "print/messaging.h"
#include "mcrl2/utilities/aterm_ext.h"
#include "time.h"

using namespace ::mcrl2::utilities;

// Class BDD_Path_Eliminator ----------------------------------------------------------------------
  // Class BDD_Path_Eliminator - functions declared private ---------------------------------------

    /// \param a_bdd A binary decision diagram.
    /// \param a_path A list of guards and negated guards, representing a path in a BDD.

    ATermAppl BDD_Path_Eliminator::aux_simplify(ATermAppl a_bdd, ATermList a_path) {
      ATermList v_true_path;
      ATermList v_false_path;
      ATermList v_true_condition;
      ATermList v_false_condition;
      ATermAppl v_guard;
      ATermAppl v_negated_guard;
      bool v_true_branch_enabled;
      bool v_false_branch_enabled;

      if (f_deadline != 0 && (f_deadline - time(0)) < 0) {
        gsDebugMsg("The time limit has passed.\n");
        return a_bdd;
      }

      if (f_bdd_info.is_true(a_bdd) || f_bdd_info.is_false(a_bdd)) {
        return a_bdd;
      }

      v_guard = f_bdd_info.get_guard(a_bdd);
      v_negated_guard = gsMakeDataExprNot(v_guard);
      v_true_condition = create_condition(a_path, v_guard, true);
      v_true_branch_enabled = f_smt_solver->is_satisfiable(v_true_condition);
      if (!v_true_branch_enabled) {
        v_false_path = ATinsert(a_path, (ATerm) v_negated_guard);
        return aux_simplify(f_bdd_info.get_false_branch(a_bdd), v_false_path);
      } else {
        v_false_condition = create_condition(a_path, v_negated_guard, true);
        v_false_branch_enabled = f_smt_solver->is_satisfiable(v_false_condition);
        if (!v_false_branch_enabled) {
          v_true_path = ATinsert(a_path, (ATerm) v_guard);
          return aux_simplify(f_bdd_info.get_true_branch(a_bdd), v_true_path);
        } else {
          v_true_path = ATinsert(a_path, (ATerm) v_guard);
          v_false_path = ATinsert(a_path, (ATerm) v_negated_guard);
          return f_bdd_manipulator.make_reduced_if_then_else(
            v_guard,
            aux_simplify(f_bdd_info.get_true_branch(a_bdd), v_true_path),
            aux_simplify(f_bdd_info.get_false_branch(a_bdd), v_false_path)
          );
        }
      }
    }

    // --------------------------------------------------------------------------------------------

    /// \param a_path A list of guards and negated guards, representing a path in a BDD.
    /// \param a_guard A guard or a negated guard.
    /// \param a_minimal A boolean value indicating whether or not minimal sets of possibly inconsistent guards are constructed.

    ATermList BDD_Path_Eliminator::create_condition(ATermList a_path, ATermAppl a_guard, bool a_minimal) {
      if (!a_minimal) {
        return ATinsert(a_path, (ATerm) a_guard);
      } else {
        ATermList v_set;
        ATermList v_auxiliary_set;
        ATermList v_iterate_over_set;
        ATermList v_iterate_over_path;
        ATermAppl v_guard_from_set;
        ATermAppl v_guard_from_path;

        v_set = ATmakeList1((ATerm) a_guard);
        v_auxiliary_set = ATmakeList0();
        while (v_set != v_auxiliary_set) {
          v_auxiliary_set = v_set;
          v_iterate_over_set = v_set;
          while (!ATisEmpty(v_iterate_over_set)) {
            v_guard_from_set = ATAgetFirst(v_iterate_over_set);
            v_iterate_over_set = ATgetNext(v_iterate_over_set);
            v_iterate_over_path = a_path;
            while (!ATisEmpty(v_iterate_over_path)) {
              v_guard_from_path = ATAgetFirst(v_iterate_over_path);
              v_iterate_over_path = ATgetNext(v_iterate_over_path);
              if (variables_overlap(v_guard_from_set, v_guard_from_path)) {
                v_set = ATinsert(v_set, (ATerm) v_guard_from_path);
                a_path = ATremoveElement(a_path, (ATerm) v_guard_from_path);
              }
            }
          }
        }
        return v_set;
      }
    }

    // --------------------------------------------------------------------------------------------

    /// \param a_expression_1 An arbitrary expression.
    /// \param a_expression_2 An arbitrary expression.

    bool BDD_Path_Eliminator::variables_overlap(ATermAppl a_expression_1, ATermAppl a_expression_2) {
      int v_number_of_arguments;
      ATermAppl v_subexpression;
      bool v_result;

      if (gsIsOpId(a_expression_1)) {
        return false;
      } else if (gsIsDataVarId(a_expression_1)) {
        return gsOccurs((ATerm) a_expression_1, (ATerm) a_expression_2);
      } else {
        v_number_of_arguments = ATgetLength(gsGetDataExprArgs(a_expression_1));
        v_result = false;
        for (int i = 0; (i < v_number_of_arguments) && !v_result; i++) {
          v_subexpression = f_expression_info.get_argument(a_expression_1, i);
          v_result = variables_overlap(v_subexpression, a_expression_2);
        }
        return v_result;
      }
    }

  // Class BDD_Path_Eliminator - functions declared public ----------------------------------------

    /// \param a_solver_type A value of an enumerated type, representing an SMT solver.

    BDD_Path_Eliminator::BDD_Path_Eliminator(SMT_Solver_Type a_solver_type) {
#if !(defined(_MSC_VER) || defined(__MINGW32__))
      if (a_solver_type == solver_type_ario) {
        f_smt_solver = new SMT_Solver_Ario();
      } else if (a_solver_type == solver_type_cvc_lite) {
        f_smt_solver = new SMT_Solver_CVC_Lite();
      } else if (a_solver_type == solver_type_cvc_lite_fast) {
#ifdef CVC_LITE_LIB
        f_smt_solver = new SMT_Solver_CVC_Lite_Fast();
#else
        gsErrorMsg("The fast implementation of CVC Lite is not available.\n");
        exit(1);
#endif
      } else {
        gsErrorMsg("An unknown SMT solver type was passed as argument.\n");
      }
#else
      gsErrorMsg("An unknown SMT solver type was passed as argument.\n");
#endif // _MSC_VER
    }

    // --------------------------------------------------------------------------------------------

    /// \param a_bdd A binary decision diagram.

    ATermAppl BDD_Path_Eliminator::simplify(ATermAppl a_bdd) {
      return aux_simplify(a_bdd, ATmakeList0());
    }
