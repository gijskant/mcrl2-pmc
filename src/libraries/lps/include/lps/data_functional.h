///////////////////////////////////////////////////////////////////////////////
/// \file data_functional.h

#ifndef LPS_DATA_FUNCTIONAL_H
#define LPS_DATA_FUNCTIONAL_H

#include <boost/config.hpp>
#include "lps/data.h"
#include "lps/detail/utility.h"

namespace lps {

// namespace data {

  inline bool is_negate       (data_expression t) { return gsIsDataExprNeg(t); }
  inline bool is_plus         (data_expression t) { return gsIsDataExprAdd(t); }
  inline bool is_minus        (data_expression t) { return gsIsDataExprSubt(t); }
  inline bool is_multiplies   (data_expression t) { return gsIsDataExprMult(t); }
  inline bool is_divides      (data_expression t) { return gsIsDataExprDiv(t); }
  inline bool is_modulus      (data_expression t) { return gsIsDataExprMod(t); }
  inline bool is_equal_to     (data_expression t) { return gsIsDataExprEq(t); }
  inline bool is_not_equal_to (data_expression t) { return gsIsDataExprNeq(t); }
  inline bool is_less         (data_expression t) { return gsIsDataExprLT(t); }
  inline bool is_greater      (data_expression t) { return gsIsDataExprGT(t); }
  inline bool is_less_equal   (data_expression t) { return gsIsDataExprLTE(t); }
  inline bool is_greater_equal(data_expression t) { return gsIsDataExprGTE(t); }
  inline bool is_min          (data_expression t) { return gsIsDataExprMin(t); }
  inline bool is_max          (data_expression t) { return gsIsDataExprMax(t); }
  inline bool is_abs          (data_expression t) { return gsIsDataExprAbs(t); }
  
  inline
  data_expression negate(data_expression d)
  {
    return gsMakeDataExprNeg(d);
  }
  
  inline
  data_expression plus(data_expression d, data_expression e)
  {
    return gsMakeDataExprAdd(d, e);
  }
  
  inline
  data_expression minus(data_expression d, data_expression e)
  {
    return gsMakeDataExprSubt(d, e);
  }
  
  inline
  data_expression multiplies(data_expression d, data_expression e)
  {
    return gsMakeDataExprMult(d, e);
  }
  
  inline
  data_expression divides(data_expression d, data_expression e)
  {
    return gsMakeDataExprDiv(d, e);
  }
  
  inline
  data_expression modulus(data_expression d, data_expression e)
  {
    return gsMakeDataExprMod(d, e);
  }
  
  inline
  data_expression equal_to(data_expression d, data_expression e)
  {
    return gsMakeDataExprEq(d, e);
  }
  
  inline
  data_expression not_equal_to(data_expression d, data_expression e)
  {
    return gsMakeDataExprNeq(d, e);
  }
  
  inline
  data_expression less(data_expression d, data_expression e)
  {
    return gsMakeDataExprLT(d, e);
  }
  
  inline
  data_expression greater(data_expression d, data_expression e)
  {
    return gsMakeDataExprGT(d, e);
  }
  
  inline
  data_expression less_equal(data_expression d, data_expression e)
  {
    return gsMakeDataExprLTE(d, e);
  }
  
  inline
  data_expression greater_equal(data_expression d, data_expression e)
  {
    return gsMakeDataExprGTE(d, e);
  }
  
  // MSVC is broken with respect to using 'min' and 'max' as identifiers.
  inline
  data_expression min_(data_expression d, data_expression e)
  {
    return gsMakeDataExprMin(d, e);
  }
  
  // MSVC is broken with respect to using 'min' and 'max' as identifiers.
  inline
  data_expression max_(data_expression d, data_expression e)
  {
    return gsMakeDataExprMax(d, e);
  }
  
  inline
  data_expression abs(data_expression d)
  {
    return gsMakeDataExprAbs(d);
  }

  inline
  data_expression if_(data_expression i, data_expression t, data_expression e)
  {
    return gsMakeDataExprIf(i, t, e);
  }

// } // namespace data

} // namespace lps

#endif // LPS_DATA_FUNCTIONAL_H
