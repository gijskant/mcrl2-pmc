// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ltsmin_test.cpp
/// \brief Test for the pins class that connects mCRL2 with LTSMin (http://fmt.cs.utwente.nl/tools/ltsmin/).

#include <iostream>

#include <boost/test/minimal.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/core/garbage_collection.h"
#include "mcrl2/lps/linearise.h"
#include "mcrl2/lps/ltsmin.h"
#include "mcrl2/lps/detail/test_input.h"

using namespace mcrl2;

typedef int* state_vector;

inline
std::string print_state(const state_vector& s, std::size_t size, const std::string& msg = "state: ")
{
  int* first = s;
  int* last = s + size;

  std::ostringstream out;
  out << msg << "[";
  for (int* i = first; i != last; ++i)
  {
    if (i != first)
    {
      out << ", ";
    }
    out << *i;
  }
  out << "]";
  return out.str();
}

struct state_callback_function
{
  std::size_t state_size;
  std::size_t state_count;
  
  state_callback_function(std::size_t size)
    : state_size(size),
      state_count(0)
  {}
  
  void operator()(state_vector const& next_state, int* const& labels, int group=-1)
  {
    state_count++;
    std::cout << print_state(next_state, state_size, "visit state: ") << " edge label = " << labels[0] << " group = " << group << std::endl;
  }
};

void test_data_type(lps::pins_data_type& type, const std::string& name)
{
  std::cout << "test_data_type(" << name << ")" << std::endl; 
  BOOST_CHECK(type.name() == name);

  // check serialize/deserialize
  for (std::size_t i = 0; i < type.size(); ++i)
  {
    std::string s = type.serialize(i);
    std::cout << "serialize(" << i << ") = " << type.serialize(i) << std::endl;
    std::size_t j = type.deserialize(s);
    BOOST_CHECK(i == j);
  }

  // check print/parse
  for (std::size_t i = 0; i < type.size(); ++i)
  {
    std::string s = type.print(i);
    std::cout << "print(" <<  i << ") = " << s << std::endl;
    std::size_t j = type.parse(s);
    std::cout << "parse(" <<  s << ") = " << j << std::endl;
  }

  // test iterator
  for (lps::pins_data_type::index_iterator i = type.index_begin(); i != type.index_end(); ++i)
  {
    std::cout << "iterator " << (i - type.index_begin()) << " -> " << *i << std::endl;
    BOOST_CHECK(*i == i - type.index_begin());
  }
}

void test_ltsmin()
{
  // create an input file
  lps::specification spec = lps::linearise(lps::detail::ABP_SPECIFICATION());
  std::string abp_filename = "temporary_abp.lps";
  spec.save(abp_filename);
  
  lps::pins p(abp_filename, "jitty");
  std::size_t N = p.process_parameter_count();
  
  BOOST_CHECK(p.edge_label_count() == 1);
  BOOST_CHECK(p.edge_label_name(0) == "action");
  BOOST_CHECK(p.edge_label_type(0) == 1);

  BOOST_CHECK(p.process_parameter_count() == 11);
  for (std::size_t i = 0; i < p.process_parameter_count(); i++)
  {
    BOOST_CHECK(p.process_parameter_type(i) == 0);
  }

  BOOST_CHECK(p.datatype_count() == 2);
  BOOST_CHECK(p.group_count() == 10);
  
  std::cout << p.info() << std::endl;

  // get the initial state
  state_vector initial_state = new int[N];
  state_vector dest_state = new int[N];
  int* labels = new int[p.edge_label_count()];
  p.get_initial_state(initial_state);
  std::cout << print_state(initial_state, N, "initial state: ") << std::endl;
  
  // find all successors of the initial state
  state_callback_function f_all(p.process_parameter_count()); 
  p.next_state_all(initial_state, f_all, dest_state, labels);
  std::cout << "f_all.state_count = " << f_all.state_count << std::endl;

  // find successors of the initial state for individual groups
  std::size_t count = 0;
  for (std::size_t group = 0; group < p.group_count(); group++)
  {
    state_callback_function f_long(p.process_parameter_count());
    p.next_state_long(initial_state, group, f_long, dest_state, labels);
    std::cout << "group " << group << " count = " << f_long.state_count << std::endl;
    count += f_long.state_count;
  }
  BOOST_CHECK(count == f_all.state_count);

  lps::pins_data_type& state_type = p.data_type(0);
  test_data_type(state_type, "state");
  BOOST_CHECK(state_type.print(0) == "1");
  BOOST_CHECK(state_type.print(1) == "d1");

  lps::pins_data_type& action_label_type = p.data_type(1);
  test_data_type(action_label_type, "action_label");

  // cleanup temporary files
  boost::filesystem::remove(boost::filesystem::path(abp_filename));
    
  delete[] initial_state;
  delete[] dest_state;
  delete[] labels;

  std::cout << p.info() << std::endl; 
  
#ifdef MCRL2_FORCE_LTSMIN_TEST_FAILURE
  BOOST_CHECK(false);
#endif
}

int test_main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT_DEBUG(argc, argv)
  
  test_ltsmin();

  return 0;
}
