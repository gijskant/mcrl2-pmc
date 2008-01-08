#include "mcrl2/lps/parelm.h"

using namespace lps;

int main(int argc, char* argv[])
{
  MCRL2_ATERM_INIT(argc, argv)

  string infile(argv[1]);
  specification spec1;
  spec1.load(argv[1]);
  specification spec2 = parelm(spec1);
  spec2.save(argv[2]);

  return 0;
}
