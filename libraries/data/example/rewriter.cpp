#include <iostream>
#include <string>
#include <cassert>
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/data/parser.h"
#include "mcrl2/data/rewriter.h"

using namespace mcrl2;
using namespace mcrl2::data;

void rewrite1()
{
  rewriter r = default_data_rewriter();

  // Rewrite two data expressions, and check if they are the same
  data_expression d1 = parse_data_expression("2+7");
  data_expression d2 = parse_data_expression("4+5");
  assert(d1 != d2);
  assert(r(d1) == r(d2));
}

void rewrite2()
{
  rewriter r = default_data_rewriter();

  // Create a substitution sequence sigma with two substitutions: [m:=3, n:=4]
  std::string var_decl = "m, n: Pos;\n";
  atermpp::vector<rewriter::substitution> sigma;
  sigma.push_back(rewriter::substitution(r, parse_data_expression("m", var_decl), parse_data_expression("3")));
  sigma.push_back(rewriter::substitution(r, parse_data_expression("n", var_decl), parse_data_expression("4")));

  // Rewrite two data expressions, and check if they are the same
  data::data_expression d1 = parse_data_expression("m+n", var_decl);
  data::data_expression d2 = parse_data_expression("7");
  assert(r(d1, sigma) == r(d2));
}

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv);

  rewrite1();
  rewrite2();

  return 0;
}

/*--- unfortunately this doesn't work yet ---
struct substitution_function
{
  const std::map<data::data_variable, data::data_expression>& s; 

  substitution_function(const std::map<data::data_variable, data::data_expression>& s_)
    : s(s_)
  {}

  data::data_expression operator()(data::data_variable v) const
  {
    std::map<data::data_variable, data::data_expression>::const_iterator i = s.find(v);
    return i == s.end() ? v : i->second;
  }
};

void rewrite2()
{
  rewriter r = default_data_rewriter();

  // Create a substitution function sigma
  std::string var_decl = "var m, n: Pos;\n";
  std::map<data::data_variable, data::data_expression> m;
  m[parse_data_expression("m", var_decl)] = parse_data_expression("3");
  m[parse_data_expression("n", var_decl)] = parse_data_expression("4");
  substitution_function sigma(m);

  // Rewrite two data expressions, and check if they are the same
  data::data_expression d1 = r(parse_data_expression("m+n", var_decl), sigma);
  data::data_expression d2 = r(parse_data_expression("7", var_decl), sigma);
  assert(r(d1) == r(d2));
}
*/
