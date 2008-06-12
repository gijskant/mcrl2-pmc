// Author(s): Aad Mathijssen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file data_implementation.h
///
/// \brief Implement data types in type checked mCRL2 specifications and expressions.

#ifndef MCRL2_DATAIMPL_H
#define MCRL2_DATAIMPL_H

#include <aterm2.h>
#include "mcrl2/core/detail/data_implementation_concrete.h"
#include "mcrl2/core/detail/struct.h"

namespace mcrl2 {
  namespace core {


/** \brief     Implement data types of a type checked mCRL2 data
 *             specification.
 *  \param[in] spec An ATerm representation of an mCRL2 data
 *             specification that adheres to the internal ATerm structure
 *             after the type checking phase.
 *  \post      The datatypes of spec are implemented as higher-order
 *             abstract data types.
 *  \return    If the data implementation went well, an equivalent
 *             version of spec is returned that adheres to the internal
 *             ATerm structure after data implementation.  If something
 *             went wrong, an appropriate error message is printed and
 *             NULL is returned.
 **/
inline ATermAppl implement_data_data_spec(ATermAppl spec)
{
  assert(detail::gsIsDataSpec(spec));
  return detail::implement_data_spec(spec);
}

/** \brief     Implement data types of a type checked mCRL2 process
 *             specification.
 *  \param[in] spec An ATerm representation of an mCRL2 process
 *             specification that adheres to the internal ATerm structure
 *             after the type checking phase.
 *  \post      The datatypes of spec are implemented as higher-order
 *             abstract data types.
 *  \return    If the data implementation went well, an equivalent
 *             version of spec is returned that adheres to the internal
 *             ATerm structure after data implementation.  If something
 *             went wrong, an appropriate error message is printed and
 *             NULL is returned.
**/
inline ATermAppl implement_data_proc_spec(ATermAppl spec)
{
  assert(detail::gsIsSpecV1(spec));
  return detail::implement_data_spec(spec);
}

/** \brief     Implement data types of a type checked mCRL2 parameterised
 *             boolean equation system (PBES) specification.
 *  \param[in] spec An ATerm representation of an mCRL2 PBES
 *             specification that adheres to the internal ATerm structure
 *             after the type checking phase.
 *  \post      The datatypes of spec are implemented as higher-order
 *             abstract data types.
 *  \return    If the data implementation went well, an equivalent
 *             version of spec is returned that adheres to the internal
 *             ATerm structure after data implementation.  If something
 *             went wrong, an appropriate error message is printed and
 *             NULL is returned.
**/
inline ATermAppl implement_data_pbes_spec(ATermAppl spec)
{
  assert(detail::gsIsPBES(spec));
  return detail::implement_data_spec(spec);
}

/** \brief     Implement data types of a type checked mCRL2 sort
 *             expression with respect to a type checked mCRL2
 *             specification.
 *  \param[in] sort_expr An ATerm representation of an mCRL2 sort
 *             expression that adheres to the internal ATerm structure
 *             after the type checking phase.
 *  \param[in] spec An ATerm representation of an mCRL2 LPS, PBES or
 *             data specification that adheres to the internal ATerm
 *             structure after the type checking phase.
 *  \post      The data types of sort_expr are implemented as
 *             higher-order abstract data types in spec.
 *             spec is updated to be the equivalent of spec in the
 *             internal format after data implementation.
 *  \return    If the data implementation went well, an equivalent
 *             version of sort_expr is returned that adheres to the
 *             internal ATerm structure after data implementation.  If
 *             something went wrong, an appropriate error message is
 *             printed and NULL is returned.
**/
inline ATermAppl implement_data_sort_expr(ATermAppl sort_expr, ATermAppl& spec)
{
  assert(detail::gsIsSortExpr(sort_expr));
  return detail::impl_exprs_with_spec(sort_expr, spec);
}

/** \brief     Implement data types of a type checked mCRL2 data expression
 *             with respect to a type checked mCRL2 specification.
 *  \param[in] data_expr An ATerm representation of an mCRL2 data
 *             expression that adheres to the internal ATerm structure
 *             after the type checking phase.
 *  \param[in] spec An ATerm representation of an mCRL2 LPS, PBES or
 *             data specification that adheres to the internal ATerm
 *             structure after the type checking phase.
 *  \post      The data types of data_expr are implemented as
 *             higher-order abstract data types in spec.
 *             spec is updated to be the equivalent of spec in the
 *             internal format after data implementation.
 *  \return    If the data implementation went well, an equivalent
 *             version of data_expr is returned that adheres to the
 *             internal ATerm structure after data implementation.  If
 *             something went wrong, an appropriate error message is
 *             printed and NULL is returned.
**/
inline ATermAppl implement_data_data_expr(ATermAppl data_expr, ATermAppl& spec)
{
  assert(detail::gsIsDataExpr(data_expr));
  return detail::impl_exprs_with_spec(data_expr, spec);
}

/** \brief     Implement data types of a type checked mCRL2 multiaction
 *             with respect to a type checked mCRL2 specification.
 *  \param[in] mult_act An ATerm representation of an mCRL2 multiaction
 *             that adheres to the internal ATerm structure after the
 *             type checking phase.
 *  \param[in] spec An ATerm representation of an mCRL2 LPS, PBES or
 *             data specification that adheres to the internal ATerm
 *             structure after the type checking phase.
 *  \post      The data types of mult_act are implemented as
 *             higher-order abstract data types in spec.
 *             spec is updated to be the equivalent of spec in the
 *             internal format after data implementation.
 *  \return    If the data implementation went well, an equivalent
 *             version of mult_act is returned that adheres to the
 *             internal ATerm structure after data implementation.  If
 *             something went wrong, an appropriate error message is
 *             printed and NULL is returned.
**/
inline ATermAppl implement_data_mult_act(ATermAppl mult_act, ATermAppl& spec)
{
  assert(detail::gsIsMultAct(mult_act));
  return detail::impl_exprs_with_spec(mult_act, spec);
}

/** \brief     Implement data types of a type checked mCRL2 process
 *             expression with respect to a type checked mCRL2
 *             specification.
 *  \param[in] proc_expr An ATerm representation of an mCRL2 process
 *             expression that adheres to the internal ATerm structure
 *             after the type checking phase.
 *  \param[in] spec An ATerm representation of an mCRL2 LPS, PBES or
 *             data specification that adheres to the internal ATerm
 *             structure after the type checking phase.
 *  \post      The data types of proc_expr are implemented as
 *             higher-order abstract data types in spec.
 *             spec is updated to be the equivalent of spec in the
 *             internal format after data implementation.
 *  \return    If the data implementation went well, an equivalent
 *             version of proc_expr is returned that adheres to the
 *             internal ATerm structure after data implementation.  If
 *             something went wrong, an appropriate error message is
 *             printed and NULL is returned.
**/
inline ATermAppl implement_data_proc_expr(ATermAppl proc_expr, ATermAppl& spec)
{
  assert(detail::gsIsProcExpr(proc_expr));
  return detail::impl_exprs_with_spec(proc_expr, spec);
}

/** \brief     Implement data types of a type checked mCRL2 state formula
 *             with respect to a type checked mCRL2 specification.
 *  \param[in] state_frm An ATerm representation of an mCRL2 state
 *             formula that adheres to the internal ATerm structure
 *             after the type checking phase.
 *  \param[in] spec An ATerm representation of an mCRL2 LPS, PBES or
 *             data specification that adheres to the internal ATerm
 *             structure after the type checking phase.
 *  \post      The data types of state_frm are implemented as
 *             higher-order abstract data types in spec.
 *             spec is updated to be the equivalent of spec in the
 *             internal format after data implementation.
 *  \return    If the data implementation went well, an equivalent
 *             version of state_frm is returned that adheres to the
 *             internal ATerm structure after data implementation.  If
 *             something went wrong, an appropriate error message is
 *             printed and NULL is returned.
**/
inline ATermAppl implement_data_state_frm(ATermAppl state_frm, ATermAppl& spec)
{
  assert(detail::gsIsStateFrm(state_frm));
  return detail::impl_exprs_with_spec(state_frm, spec);
}

/** \brief     Implement data types of a type checked mCRL2 action rename
 *             specification with respect to a type checked mCRL2 linear
 *             process specification (LPS).
 *  \param[in] ar_spec An ATerm representation of an mCRL2 action rename
 *             specification that adheres to the internal ATerm
 *             structure after the type checking phase.
 *  \param[in] lps_spec An ATerm representation of an mCRL2 LPS that adheres
 *             to the internal ATerm structure after the type checking phase.
 *  \post      The data types in action_rename_spec are implemented as
 *             higher-order abstract data types types and the data types
 *             of lps_spec are added to the data types of ar_spec.  The
 *             datatypes of lps_spec are implemented as higher-order
 *             abstract data types in lps_spec
 *  \return    If the data implementation went well, an equivalent
 *             version of ar_spec is returned that adheres to the
 *             internal ATerm structure after data implementation, also
 *             containing the data specification of lps_spec If
 *             something went wrong, an appropriate error message is
 *             printed and NULL is returned.
**/
inline ATermAppl implement_data_action_rename_spec(ATermAppl ar_spec, ATermAppl& lps_spec)
{
  assert(detail::gsIsActionRenameSpec(ar_spec));
  assert(detail::gsIsSpecV1(lps_spec));
  return detail::impl_data_action_rename_spec_detail(ar_spec, lps_spec);
}

  }
}
#endif // MCRL2_DATAIMPL_H
