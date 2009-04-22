#include "mcrl2/new_data/data.h"
#include <cassert>

using namespace mcrl2::new_data;

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  data_expression two   = sort_nat::nat(2);
  data_expression three = sort_nat::nat(3);
  data_expression five  = sort_nat::plus(two, three);

  assert(five.sort() == sort_nat::nat());
  return 0;
}
