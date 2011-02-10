// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file std_container.cpp
/// \brief Add your file description here.

#define ATERM_DEBUG_PROTECTION

#include <algorithm>
#include <iostream>
#include <iterator>
#include "mcrl2/atermpp/aterm_traits.h"
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/atermpp/utility.h"
#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/atermpp/deque.h"
#include "mcrl2/atermpp/list.h"
#include "mcrl2/atermpp/map.h"
#include "mcrl2/atermpp/set.h"
#include "mcrl2/atermpp/vector.h"
#include "gc.h"               // AT_collect

using namespace std;
using namespace atermpp;

// user defined class containing ATerms
class A
{
    template <typename T>
    friend struct atermpp::aterm_traits;

  protected:
    ATerm x;

    const ATerm& term() const
    {
      return x;
    }

    ATerm& term()
    {
      return x;
    }

  public:
    A()
    {
      x = aterm_traits<aterm_int>::term(aterm_int(10));
    }

    A(int i)
    {
      x = aterm_traits<aterm_int>::term(aterm_int(i));
    }

    int value() const
    {
      return aterm_int(x).value();
    }

    bool operator<(A other) const
    {
      return value() < other.value();
    }

    void protect()
    {
      ATprotect(&x);
    }

    void unprotect()
    {
      ATunprotect(&x);
    }

    void mark()
    {
      ATmarkTerm(x);
    }
};

std::ostream& operator<<(std::ostream& out, const A& t)
{
  return out << t.value();
}

// specify how the ATerms in A need to be protected using a traits class
namespace atermpp
{
template<>
struct aterm_traits<A>
{
  typedef ATermAppl aterm_type;
  static void protect(A t)
  {
    t.protect();
    std::cout << "aterm_protect_traits<A>::protect() " << t << std::endl;
  }
  static void unprotect(A t)
  {
    t.unprotect();
    std::cout << "aterm_protect_traits<A>::unprotect() " << t << std::endl;
  }
  static void mark(A t)
  {
    t.mark();
    std::cout << "aterm_protect_traits<A>::mark() " << t << std::endl;
  }
  static ATerm term(A t)
  {
    return t.term();
  }
  static ATerm* ptr(A& t)
  {
    return &t.term();
  }
};
} // namespace atermpp

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  // deque
  atermpp::deque<aterm_appl> d;
  d.assign(3, make_term("f(1)"));
  d.push_back(make_term("f(2)"));
  d.push_front(make_term("f(3)"));
  copy(d.begin(), d.end(), ostream_iterator<aterm_appl>(cout, "\n"));
  cout << endl;

  // vector
  atermpp::vector<aterm_appl> v;
  v.reserve(5);
  v.push_back(make_term("g(1)"));
  v.push_back(make_term("g(2)"));
  v.push_back(make_term("g(3)"));
  swap(v[0], v[2]);
  v.insert(v.begin()+1, make_term("a(0)"));

  // list
  atermpp::list<aterm_appl> l;
  l.push_back(make_term("l(1)"));
  l.push_front(make_term("l(2)"));
  l.push_front(make_term("l(4)"));

  // set
  atermpp::set<A> s;
  for (int i = 0; i < 5; i++)
  {
    s.insert(A(5-i));
  }
  s.insert(A(1));

  // multiset
  atermpp::multiset<A> s1;
  for (int i = 0; i < 5; i++)
  {
    s1.insert(A(5-i));
  }
  s1.insert(A(1));

  // map
  atermpp::map<int, aterm_int> m;
  for (int i = 0; i < 5; i++)
  {
    m[i] = aterm_int(i);
  }

  // multimap
  atermpp::multimap<int, aterm_int> m1;
  for (int i = 0; i < 5; i++)
  {
    m1.insert(make_pair(i, aterm_int(i)));
  }
  m1.insert(make_pair(1, aterm_int(2)));

  AT_collect(); // force garbage collection
  AT_collect(); // force garbage collection

  return 0;
}
