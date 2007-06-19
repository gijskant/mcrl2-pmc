#if !(defined(_MSC_VER) || defined(__MINGW32__))

// Implementation of class SMT_Solver_CVC_Lite
// file: smt_solver_cvc_lite.cpp

#include "prover/smt_solver_cvc_lite.h"
#include "pstream.h"
#include "print/messaging.h"
#include <iostream>

using namespace ::mcrl2::utilities;
using namespace std;

// Class SMT_Solver_CVC_Lite ----------------------------------------------------------------------
  // Class SMT_Solver_CVC_Lite - Functions declared public ----------------------------------------

    SMT_Solver_CVC_Lite::SMT_Solver_CVC_Lite() {
      string v_string_out;

      redi::pstream v_pstream("cvcl -h", redi::pstreams::pstdin | redi::pstreams::pstderr | redi::pstreams::pstdout);
      getline(v_pstream.out(), v_string_out);
      if (v_string_out == "Usage: cvcl [options]") {
        gsVerboseMsg("The SMT solver CVC Lite is available.\n");
      } else {
        gsErrorMsg(
          "The SMT solver CVC Lite is not available.\n"
          "Consult the manual of the tool you are using for instructions on how to obtain CVC Lite.\n"
        );
        exit(1);
      }
      v_pstream.close();
    }

    // --------------------------------------------------------------------------------------------

    bool SMT_Solver_CVC_Lite::is_satisfiable(ATermList a_formula) {
      translate(a_formula);

      redi::pstream v_pstream("cvcl -lang smt-lib", redi::pstreams::pstdin | redi::pstreams::pstdout | redi::pstreams::pstderr);
      v_pstream << f_benchmark << endl << redi::peof;

      string v_string_out;
      getline(v_pstream.out(), v_string_out);
      v_pstream.close();
      if (v_string_out == "Unsatisfiable.") {
        gsVerboseMsg("The formula is unsatisfiable\n");
        return false;
      } else if (v_string_out == "Satisfiable.") {
        gsVerboseMsg("The formula is satisfiable\n");
        return true;
      } else if (v_string_out == "Unknown.") {
        gsVerboseMsg("CVC Lite cannot determine whether this formula is satisfiable or not.\n");
        return true;
      } else {
        gsErrorMsg("CVC Lite reported an error while solving the formula.\n");
        redi::pstream v_pstream("cvcl -lang smt-lib", redi::pstreams::pstdin | redi::pstreams::pstdout | redi::pstreams::pstderr);
        v_pstream << f_benchmark << endl << redi::peof;
        while (getline(v_pstream.err(), v_string_out)) {
          v_string_out = "CVC Lite: " + v_string_out + "\n";
          gsErrorMsg((char*) v_string_out.c_str());
        }
        v_pstream.close();
        exit(1);
      }
    }

#endif // _MSC_VER
