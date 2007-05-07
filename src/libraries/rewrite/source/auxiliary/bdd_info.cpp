// Implementation of class BDD_Info
// file: bdd_info.cpp

#include "auxiliary/bdd_info.h"
#include "libstruct.h"
#include "liblowlevel.h"
#include "libprint_c.h"

// Class BDD_Info ---------------------------------------------------------------------------------
  // Class BDD_Info - Functions declared public ---------------------------------------------------

    BDD_Info::BDD_Info() {
    }

    // --------------------------------------------------------------------------------------------

    /// \param a_bdd A binary decision diagram.
    /// \return The guard at the root of the BDD.

    ATermAppl BDD_Info::get_guard(ATermAppl a_bdd) {
      return ATAgetArgument(ATAgetArgument(ATAgetArgument(a_bdd, 0), 0), 1);
    }

    // --------------------------------------------------------------------------------------------

    /// \param a_bdd A binary decision diagram.
    /// \return The true-branch of the BDD.

    ATermAppl BDD_Info::get_true_branch(ATermAppl a_bdd) {
      return ATAgetArgument(ATAgetArgument(a_bdd, 0), 1);
    }

    // --------------------------------------------------------------------------------------------

    /// \param a_bdd A binary decision diagram.
    /// \return The false-branch of the BDD.

    ATermAppl BDD_Info::get_false_branch(ATermAppl a_bdd) {
      return ATAgetArgument(a_bdd, 1);
    }

    // --------------------------------------------------------------------------------------------

    /// \param a_bdd A binary decision diagram.
    /// \return True, if the BDD equals true.
    ///         False, if the BDD does not equal true.

    bool BDD_Info::is_true(ATermAppl a_bdd) {
      return gsIsDataExprTrue(a_bdd);
    }

    // --------------------------------------------------------------------------------------------

    /// \param a_bdd A binary decision diagram.
    /// \return True, if the BDD equals false.
    ///         False, if the BDD does not equal true.

    bool BDD_Info::is_false(ATermAppl a_bdd) {
      return gsIsDataExprFalse(a_bdd);;
    }

    // --------------------------------------------------------------------------------------------

    /// \param a_bdd A binary decision diagram.
    /// \return True, if the root of the BDD is a guard node.
    ///         False, if the BDD equals true or if the BDD equals false.

    bool BDD_Info::is_if_then_else(ATermAppl a_bdd) {
      return gsIsDataExprIf(a_bdd); 
    }
