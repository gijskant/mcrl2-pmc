// Author(s): Jan Friso Groote
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file find_test.cpp
/// \brief Test the linear_inequality functionality

#include <boost/test/minimal.hpp>
#include "mcrl2/data/parse.h"
#include "mcrl2/data/linear_inequalities.h"

using namespace mcrl2;
using namespace mcrl2::core;
using namespace mcrl2::data;

void split_conjunction_of_inequalities_set(const data_expression& e, std::vector < linear_inequality >& v, const rewriter& r)
{
  if (sort_bool::is_and_application(e))
  {
    split_conjunction_of_inequalities_set(application(e)[0],v,r);
    split_conjunction_of_inequalities_set(application(e)[1],v,r);
  }
  else
  {
    v.push_back(linear_inequality(e,r));
  }
}

bool test_consistency_of_inequalities(const std::string& vars,
                                      const std::string& inequalities, 
                                      const bool expect_consistent)
{
  // Take care that reals are part of the data type.
  data_specification data_spec;
  variable_list variables=parse_variables(vars);
  data_spec.add_context_sort(sort_real::real_());
  const data_expression e=parse_data_expression(inequalities,variables,data_spec);

  rewriter r(data_spec);
  std::vector < linear_inequality > v_inequalities;
  split_conjunction_of_inequalities_set(e,v_inequalities,r);

  if (is_inconsistent(v_inequalities,r))
  {
    if (expect_consistent)
    {
      std::cout << "Expected consistent, found inconsistent\n";
      std::cout << variables << ": " << inequalities << "\n";
      std::cout << "Internal inequalities: " << pp_vector(v_inequalities) << "\n";
      return false;
    }
  }
  else
  {
    if (!expect_consistent)
    {
      std::cout << "Expected inconsistent, found consistent\n";
      std::cout << variables << ": " << inequalities << "\n";
      std::cout << "Internal inequalities: " << pp_vector(v_inequalities) << "\n";
      return false;
    }
  }
  return true;
}

bool test_application_of_Fourier_Motzkin(const std::string& vars,
                                         const std::string& variables_to_be_eliminated,
                                         const std::string& inequalities,
                                         const std::string& inconsistent_with)
{
  // Take care that reals are part of the data type.
  data_specification data_spec;
  data_spec.add_context_sort(sort_real::real_());
  const variable_list variables=parse_variables(vars);
  const data_expression e_in=parse_data_expression(inequalities,variables,data_spec);
  const variable_list v_elim=data::detail::parse_variables_new(variables_to_be_eliminated);

  rewriter r(data_spec);
  std::vector < linear_inequality > v_inequalities;
  split_conjunction_of_inequalities_set(e_in,v_inequalities,r);

  std::vector < linear_inequality> resulting_inequalities;
  fourier_motzkin(v_inequalities, v_elim.begin(), v_elim.end(), resulting_inequalities, r);

  std::vector < linear_inequality> inconsistent_inequalities=resulting_inequalities;
  inconsistent_inequalities.push_back(linear_inequality(parse_data_expression(inconsistent_with,variables,data_spec),r));
  if (!is_inconsistent(inconsistent_inequalities,r, false))
  {
    std::cout << "Expected set of inequations to be inconsisten with given inequality after applying Fourier-Motzkin elimination\n";
    std::cout << "Input: " << variables << ": " << inequalities << "\n";
    std::cout << "Parsed input : " << pp_vector(v_inequalities) << "\n";
    std::cout << "Variables to be eliminated: " << v_elim << "\n";
    std::cout << "Input after applying Fourier Motzkin: " << pp_vector(resulting_inequalities) << "\n";
    std::cout << "Should be inconsistent with " << inconsistent_with << "\n";
    std::cout << "Inconsistent inequality after parsing " << pp(linear_inequality(parse_data_expression(inconsistent_with,variables,data_spec),r)) << "\n";
    return false;
  }
  return true;
}

int test_main(int /* argc */, char** /* argv[]*/)
{
  // BOOST_CHECK(test_consistency_of_inequalities("x:Real;", "x<3  && x>=4", false));
  // BOOST_CHECK(test_consistency_of_inequalities("x:Real;", "x<3  && x>=2", true));
  // BOOST_CHECK(test_consistency_of_inequalities("x:Real;", "x<3  && x>=3", false));
  // BOOST_CHECK(test_consistency_of_inequalities("x:Real;", "x<=3  && x>=3", true));
  // BOOST_CHECK(test_consistency_of_inequalities("u:Real;","0 <= u && -u <= -4 && -u < 0",true));
  BOOST_CHECK(test_consistency_of_inequalities("u,t:Real;","u + -t <= 1 && -u <= -4 && t < u && -u < 0 && -t <= 0 ",true));
  // BOOST_CHECK(test_consistency_of_inequalities("u,t,l:Real;","u + -t <= 1 && -u <= -4 && -u + l < 0 && -u < 0 && -t <= 0 && -l + t <= 0",true));

  // BOOST_CHECK(test_application_of_Fourier_Motzkin("x,y:Real;", "y:Real;", "-y + x < 0 &&  y < 2", "x>=2"));
  return 0;
}

