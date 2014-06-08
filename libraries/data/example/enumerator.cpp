#include <iostream>
#include <set>
#include "mcrl2/data/classic_enumerator.h"
#include "mcrl2/data/substitutions/enumerator_substitution.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/rewriter.h"
#include "mcrl2/data/replace.h"

using namespace mcrl2;
using namespace mcrl2::data;

void enumerate(const variable_list& variables, const data_expression& condition)
{
  data_specification data_spec;
  rewriter evaluator(data_spec);
  mutable_indexed_substitution<> sigma;

  classic_enumerator<rewriter, 
                     mutable_indexed_substitution<>,
                     enumerator_list_element_with_substitution<data_expression> > enumerator(evaluator, data_spec);

  for (classic_enumerator<>::iterator i = enumerator.begin(
                   sigma, enumerator_list_element_with_substitution<data_expression>(variables, condition)); 
       i!= enumerator.end(); ++i)
  {
    i->add_assignments(variables,sigma,evaluator);
    std::cout << data::replace_free_variables(condition, sigma) << std::endl;
  }
}

void test1()
{
  variable b("b", sort_bool::bool_());
  variable c("c", sort_bool::bool_());
  data_expression T = sort_bool::true_();
  data_expression F = sort_bool::false_();

  data_expression condition = sort_bool::and_(b, c);

  variable_list variables;
  variables.push_front(b);

  enumerate(variables, condition);
}

int main(int argc, char* argv[])
{
  test1();

  return 0;
}
