#include <boost/test/minimal.hpp>
#include "mcrl2/lps/mcrl22lps.h"
#include "mcrl2/pbes/bisimulation.h"

using namespace atermpp;
using namespace mcrl2::lps;
using namespace mcrl2::pbes_system;

const std::string ABP_SPECIFICATION =
"% This file contains the alternating bit protocol, as described in W.J.    \n"
"% Fokkink, J.F. Groote and M.A. Reniers, Modelling Reactive Systems.       \n"
"%                                                                          \n"
"% The only exception is that the domain D consists of two data elements to \n"
"% facilitate simulation.                                                   \n"
"                                                                           \n"
"sort                                                                       \n"
"  D     = struct d1 | d2;                                                  \n"
"  Error = struct e;                                                        \n"
"                                                                           \n"
"act                                                                        \n"
"  r1,s4: D;                                                                \n"
"  s2,r2,c2: D # Bool;                                                      \n"
"  s3,r3,c3: D # Bool;                                                      \n"
"  s3,r3,c3: Error;                                                         \n"
"  s5,r5,c5: Bool;                                                          \n"
"  s6,r6,c6: Bool;                                                          \n"
"  s6,r6,c6: Error;                                                         \n"
"  i;                                                                       \n"
"                                                                           \n"
"proc                                                                       \n"
"  S(b:Bool)     = sum d:D. r1(d).T(d,b);                                   \n"
"  T(d:D,b:Bool) = s2(d,b).(r6(b).S(!b)+(r6(!b)+r6(e)).T(d,b));             \n"
"                                                                           \n"
"  R(b:Bool)     = sum d:D. r3(d,b).s4(d).s5(b).R(!b)+                      \n"
"                  (sum d:D.r3(d,!b)+r3(e)).s5(!b).R(b);                    \n"
"                                                                           \n"
"  K             = sum d:D,b:Bool. r2(d,b).(i.s3(d,b)+i.s3(e)).K;           \n"
"                                                                           \n"
"  L             = sum b:Bool. r5(b).(i.s6(b)+i.s6(e)).L;                   \n"
"                                                                           \n"
"init                                                                       \n"
"  allow({r1,s4,c2,c3,c5,c6,i},                                             \n"
"    comm({r2|s2->c2, r3|s3->c3, r5|s5->c5, r6|s6->c6},                     \n"
"        S(true) || K || L || R(true)                                       \n"
"    )                                                                      \n"
"  );                                                                       \n";

void test_bisimulation()
{
  specification abp_spec = mcrl22lps(ABP_SPECIFICATION);
  pbes<> bb  = branching_bisimulation(abp_spec, abp_spec);
  pbes<> sb  = strong_bisimulation(abp_spec, abp_spec);
  pbes<> wb  = weak_bisimulation(abp_spec, abp_spec);
  pbes<> sbe = branching_simulation_equivalence(abp_spec, abp_spec);

  BOOST_CHECK(bb .is_well_typed());
  BOOST_CHECK(sb .is_well_typed());
  BOOST_CHECK(wb .is_well_typed());
  BOOST_CHECK(sbe.is_well_typed());

//  BOOST_CHECK(pbes2bool(bb ));
//  BOOST_CHECK(pbes2bool(sb ));
//  BOOST_CHECK(pbes2bool(wb ));
//  BOOST_CHECK(pbes2bool(sbe));
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT_DEBUG(argc, argv)
  test_bisimulation();

  return 0;
}
