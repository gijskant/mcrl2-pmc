// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbes_solve_test.cpp
/// \brief Add your file description here.

#include <iostream>
#include <boost/test/minimal.hpp>
#include "mcrl2/modal_formula/detail/algorithms.h" // mcf2statefrm
#include "mcrl2/lps/mcrl22lps.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/pbes2bool.h"
#include "mcrl2/pbes/lps2pbes.h"
#include "mcrl2/pbes/bes_algorithms.h"

using namespace mcrl2::data;
using namespace mcrl2::lps;
using namespace mcrl2::modal;
using namespace mcrl2::pbes_system;

/// check the given formula
void solve_formula(std::string title, std::string spec_text, std::string formula_text, bool expected_result, bool timed = false)
{
  pbes<> p = lps2pbes(spec_text, formula_text, timed);

  bool result = pbes2bool(p);
  BOOST_CHECK(result = expected_result);
  
  int result2 = bes_gauss_elimination(p);
  std::cout << "<result2>" << result2 << std::endl;
  BOOST_CHECK((result2 == 2) || (expected_result == (result2 == 1)));
  
  pbes<> p3 = pbes2bes(p, false);
  bool result3 = pbes2bool(p3);
  BOOST_CHECK(result3 = expected_result);

  pbes<> p4 = pbes2bes(p, true);
  bool result4 = pbes2bool(p4);
  BOOST_CHECK(result4 = expected_result);
}

/// hblc test case from Muhammed en Mohammad (18-5-2008)
void test_hblc()
{
  std::string HBLC =
    "% satisfies r1, r2, r3                                                                                                              \n"
    "                                                                                                                                    \n"
    "                                                                                                                                    \n"
    "sort                                                                                                                                \n"
    "  p2q     = struct hb0;                                                                                                             \n"
    "  q2p     = struct hb1;                                                                                                             \n"
    "  Error = struct e;                                                                                                                 \n"
    "act                                                                                                                                 \n"
    "timeout_q,inactivate_p,inactivate_q;                                                                                                \n"
    "rcv_from_p,send_to_q,for_q,from_p,sent_by_p,rcv_by_q:p2q;                                                                           \n"
    "rcv_from_q,send_to_p,for_p,from_q,sent_by_q,rcv_by_p:q2p;                                                                           \n"
    "rcv_from_p,send_to_q,for_q,from_p,sent_by_p,rcv_by_q:Error;                                                                         \n"
    "rcv_from_q,send_to_p,for_p,from_q,sent_by_q,rcv_by_p:Error;                                                                         \n"
    "timeout : Nat;                                                                                                                      \n"
    "                                                                                                                                    \n"
    "i;                                                                                                                                  \n"
    "proc                                                                                                                                \n"
    "% -------------  channels ------------                                                                                              \n"
    "                                                                                                                                    \n"
    " Channel_p2q= sum message:p2q. rcv_from_p(message).((i.send_to_q(message)) + i).Channel_p2q;                                        \n"
    " Channel_q2p= sum message:q2p. rcv_from_q(message).((i.send_to_p(message)) + i).Channel_p2q;                                        \n"
    "                                                                                                                                    \n"
    "%%%%%%%%%%%%% Process P %%%%%%%%%%%%%                                                                                               \n"
    "                                                                                                                                    \n"
    "  P (t:Nat,active,rcvd:Bool,tmin,tmax:Pos)=                                                                                         \n"
    "                       (active) ->  inactivate_p.P(t,false,rcvd,tmin,tmax)                                                          \n"
    "                       %+ tau.P(t,avtive,rcvd,tmin,tmax) )                                                                          \n"
    "                       +                                                                                                            \n"
    "                        (active) -> (timeout(t).                                                                                    \n"
    "                                      ( (rcvd) -> for_q (hb0).P(tmax,active,false,tmin,tmax)                                        \n"
    "                                                  <>  ( (t div 2 >= tmin) -> for_q(hb0).P(t div 2 ,active,false,tmin,tmax)          \n"
    "                                                                          <> inactivate_p.P(t div 2,false,rcvd,tmin,tmax)  ) ) )    \n"
    "                       +                                                                                                            \n"
    "                       from_q(hb1).( (active) -> P(t,active,true,tmin,tmax) <> P(t,active,rcvd,tmin,tmax) );                        \n"
    "                                                                                                                                    \n"
    "%%%%%%%%%%%%% Process Q %%%%%%%%%%%%%                                                                                               \n"
    "                                                                                                                                    \n"
    "Q (tmin,tmax:Pos,active:Bool)=  (active) ->   inactivate_q.Q(tmin,tmax,false)                                                       \n"
    "                %+ tau.Q(tmin,tmax,active) )                                                                                        \n"
    "                +                                                                                                                   \n"
    "                from_p(hb0).((active) -> for_p(hb1).Q (tmin,tmax,active) <> Q(tmin,tmax,active))                                    \n"
    "                +                                                                                                                   \n"
    "                (active) -> timeout_q.Q(tmin,tmax,false);                                                                           \n"
    "                                                                                                                                    \n"
    "%%%%%%%%%%%%% Process for Heartbeat Protocol %%%%%%%%%%%%%                                                                          \n"
    "                                                                                                                                    \n"
    "  % HeartBeat_Binary= allow( {sent_by_p,rcv_by_q,sent_by_q,rcv_by_p,timeout,timeout_q,inactivate_p,inactivate_q},                   \n"
    "  HeartBeat_Binary= allow( {sent_by_p,timeout},                                                                                     \n"
    "   hide ({i},                                                                                                                       \n"
    "                      comm ( {                                                                                                      \n"
    "                                                                                                                                    \n"
    "                rcv_from_p | for_q ->sent_by_p,                                                                                     \n"
    "                send_to_q   | from_p -> rcv_by_q,                                                                                   \n"
    "                rcv_from_q | for_p ->sent_by_q,                                                                                     \n"
    "                send_to_p   | from_q -> rcv_by_p                                                                                    \n"
    "                     } ,                                                                                                            \n"
    "                         P(3,true,true,1,3) || Q(1,3,true) || Channel_p2q || Channel_q2p ) ));                                      \n"
    "                                                                                                                                    \n"
    "  Process_Channel= allow( {sent_by_p,send_to_q},                                                                                    \n"
    "   hide ({i,timeout_q,inactivate_q,inactivate_p,sent_by_q,rcv_by_p,timeout},                                                        \n"
    "                      comm ( {                                                                                                      \n"
    "                                                                                                                                    \n"
    "                rcv_from_p | for_q ->sent_by_p,                                                                                     \n"
    "                send_to_q   | from_p -> rcv_by_q,                                                                                   \n"
    "                rcv_from_q | for_p ->sent_by_q,                                                                                     \n"
    "                send_to_p   | from_q -> rcv_by_p                                                                                    \n"
    "                     } ,                                                                                                            \n"
    "                         P(3,true,true,1,3) || Channel_p2q  )) );                                                                   \n"
    "                                                                                                                                    \n"
    "                                                                                                                                    \n"
    "init                                                                                                                                \n"
    "% Process_Channel ;                                                                                                                 \n"
    "HeartBeat_Binary;                                                                                                                   \n"
    "%P(3,true,true,1,3);% intially t=tmax                                                                                               \n"
    "%Q(1,3,true);                                                                                                                       \n"
    "%Channel_p2q;                                                                                                                       \n"
  ;
  
  std::string R4 = "[true][sent_by_p(hb0)] false";
  bool expected_result = false;
  solve_formula("HBLC", HBLC, R4, expected_result);
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT(argc, argv)
  test_hblc();

  return 0;
}
