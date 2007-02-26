#include <iostream>
#include <iterator>

#include "atermpp/atermpp.h"
#include "atermpp/algorithm.h"
#include "lpe/data.h"
#include "lpe/data_init.h"
#include "lpe/data_utility.h"
#include "lpe/sort.h"
#include "lpe/specification.h"
#include "lpe/detail/tools.h"
#include "test_specifications.h"

using namespace std;
using namespace atermpp;
using namespace lpe;
using namespace lpe::data_init;
using namespace lpe::detail;

struct compare_variable
{
  aterm d;

  compare_variable(data_variable d_)
    : d(d_)
  {}

  bool operator()(aterm t) const
  {
    return d == t;
  }
};

bool occurs_in(data_expression d, data_variable v)
{
  return find_if(aterm_appl(d), compare_variable(v)) != aterm();
}

int main()
{
  aterm bottom_of_stack;
  aterm_init(bottom_of_stack);
  gsEnableConstructorFunctions();

  specification spec = mcrl22lpe(ABP_SPECIFICATION);
  linear_process lpe = spec.lpe();
  std::set<aterm_string> ids = identifiers(aterm(lpe));
  for (std::set<aterm_string>::iterator i = ids.begin(); i != ids.end(); ++i)
  {
    cout << "- " << *i << endl;
  }
  cin.get();
  
  summand summand_ = *lpe.summands().begin();
  data_expression d = summand_.condition();
  cout << "d = " << d << endl;
  for (data_variable_list::iterator j = summand_.summation_variables().begin(); j != summand_.summation_variables().end(); ++j)
  {
    data_variable v = *j;
    bool b = occurs_in(d, v);
    cout << "v = " << v << endl;
    cout << "occurs: " << b << endl;
  }

  // find all labels in an LPE
  std::set<action_label> labels;
  find_all_if(lpe, is_action_label, inserter(labels, labels.end()));
  cout << "--- labels ---" << endl;
  for (std::set<action_label>::iterator i = labels.begin(); i != labels.end(); ++i)
  {
    cout << i->name() << endl;
  }

/*
  // find all "op id's" in an LPE
  cout << "--- op id's ---" << endl;
  std::set<aterm> s;
  find_all_if(lpe, is_op_id, std::inserter(s, s.end()));
  for (std::set<aterm>::iterator i = s.begin(); i != s.end(); ++i)
  {
    cout << *i << endl;
  }
*/

  return 0;
}
