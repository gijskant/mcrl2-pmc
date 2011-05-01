// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file nextstate_standard.cpp

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include "mcrl2/aterm/aterm2.h"
#include "mcrl2/core/detail/struct_core.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/representative_generator.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/lps/nextstate/standard.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/lps/specification.h"

using namespace mcrl2::core;
using namespace mcrl2::core::detail;
using namespace mcrl2::data;

#define ATAgetFirst(x) ((ATermAppl) ATgetFirst(x))
#define ATLgetFirst(x) ((ATermList) ATgetFirst(x))
#define ATAgetArgument(x,y) ((ATermAppl) ATgetArgument(x,y))
#define ATLgetArgument(x,y) ((ATermList) ATgetArgument(x,y))

#define ATisList(x) (ATgetType(x) == AT_LIST)

#ifdef MCRL2_NEXTSTATE_DEBUG

static std::string print_state(atermpp::aterm_appl s, ns_info const& info)
{
  if (info.stateformat==GS_STATE_TREE)
  { 
    ATfprintf(stderr,"GS_STATE_TREE... %t\n",(ATermAppl)s);
    return "";
  }

  std::string result("state(");
  size_t index = 0;
  for (atermpp::aterm_appl::const_iterator i = s.begin(); i != s.end(); ++i)
  {
    if (index++ != 0)
    {
      result.append(", ");
    }

    result.append(pp(atermpp::aterm(info.export_term(static_cast<ATerm>(*i)))));
  }
  result.append(")");
  return result;
}

static std::string print_assignments(atermpp::aterm_list a, ns_info const& info)
{
  atermpp::aterm_list res;
  for (atermpp::aterm_list::const_iterator i = a.begin(); i != a.end(); ++i)
  {
    ATermAppl t = atermpp::aterm_appl(*i);
    if (t == gsMakeNil())
    {
      res = atermpp::push_front(res, atermpp::aterm(t));
    }
    else
    {
      res = atermpp::push_front(res, atermpp::aterm(info.export_term((ATerm)t)));
    }
  }
  res = atermpp::reverse(res);
  return pp(res);
}
#endif // MCRL2_NEXTSTATE_DEBUG

/* Explanation of the tree building algorithm.
 *
 * If len(l) is a power of 2, than building the tree is easy. One just
 * pairs every element in the list with its neighbor, giving again a list
 * with the length a power of 2, repeating until there is only one element
 * (the result). For example, [0,1,2,3] -> [(0,1),(2,3)] -> ((0,1),(2,3)).
 *
 * With an odd number of elements, our first goal is to pair just a few
 * elements to get a list with the length a power of 2. We do this in such
 * a way that it is as easy as possible to lookup an element.
 *
 * The easiest way of looking up an element in a tree is probably when, for
 * a tree with n leafs, we know that numbers up to n/2 are on the left and
 * numbers above n/2 are on the right. This way we can just compare the
 * index we are looking for with n/2 and choose the right tree and repeat
 * the process.
 *
 * We will use an array tree_init to determine which elements to pair to
 * get a list with the length a power of 2. More specifically, if
 * tree_init[i] is true, then we pair element i-1 and i. It easy to
 * determine which elements should be true by recursively "dividing" the
 * array in two parts using /2.
 */
static void fill_tree_init(bool* init, size_t n, size_t l)
{
  if (l == 0)
  {
    return;
  }

  if (n > 2)
  {
    fill_tree_init(init,n/2,l/2);
    fill_tree_init(init+n/2,n-n/2,l-l/2);
  }
  else /* n == 2 */
  {
    init[1] = true;
  }
}
ATerm NextState::buildTree(ATerm* args)
{
  size_t n,m;

  if (info.statelen == 0)
  {
    return (ATerm) info.nil;
  }

  if (tree_init == NULL)
  {
    tree_init = (bool*) malloc(info.statelen*sizeof(bool));
    for (size_t i=0; i<info.statelen; i++)
    {
      tree_init[i] = false;
    }
    n = 1;
    while (n <= info.statelen)
    {
      n *= 2;
    }
    n /= 2;
    fill_tree_init(tree_init,info.statelen,info.statelen-n);
  }

  n = 0;
  m = 0;
  while (n < info.statelen)
  {
    if (tree_init[n])
    {
      args[m-1] = (ATerm) ATmakeAppl2(info.pairAFun,args[m-1],args[n]);
    }
    else
    {
      args[m] = args[n];
      m++;
    }
    n++;
  }

  n = m;
  while (n > 1)
  {
    for (size_t i=0; i<n; i+=2)
    {
      args[i/2] = (ATerm) ATmakeAppl2(info.pairAFun,args[i],args[i+1]);
    }

    n /= 2;
  }

  return args[0];
}

ATerm NextState::getTreeElement(ATerm tree, size_t index)
{
  size_t n = info.statelen;
  size_t m = 0;
  while (m+1 != n)
  {
    size_t t = (m+n)/2;

    assert((ATgetType(tree) == AT_APPL) && ATisEqualAFun(ATgetAFun((ATermAppl) tree),info.pairAFun));

    if (index < t)
    {
      tree = ATgetArgument((ATermAppl) tree,0);
      n = t;
    }
    else
    {
      tree = ATgetArgument((ATermAppl) tree,1);
      m = t;
    }
  }

  return tree;
}

size_t NextState::getStateLength()
{
  return info.statelen;
}

ATermAppl NextState::getStateArgument(ATerm state, size_t index)
{
  switch (info.stateformat)
  {
    case GS_STATE_VECTOR:
      return info.export_term(ATgetArgument((ATermAppl) state,index));
    case GS_STATE_TREE:
      return info.export_term(getTreeElement(state,index));
    default:
      return NULL;
  }
}

ATermAppl NextState::makeStateVector(ATerm state)
{
  if (!stateAFun_made)
  {
    stateAFun_made = true;
    info.stateAFun = ATmakeAFun("STATE",info.statelen,false);
    ATprotectAFun(info.stateAFun);
  }

  // XXX can be done more efficiently in some cases
  for (size_t i=0; i<info.statelen; i++)
  {
    stateargs[i] = (ATerm) getStateArgument(state,i);
  }
  return ATmakeApplArray(info.stateAFun,stateargs);
}

//Prototype
static bool statearg_match(ATermAppl arg, ATermAppl pat, ATermTable vars = NULL);

static bool statearg_match_list(ATermList arg, ATermList pat, ATermTable vars)
{
  assert(ATgetLength(arg) == ATgetLength(pat));
  bool r = true;

  while (!ATisEmpty(arg) && r)
  {
    r = r && statearg_match(ATAgetFirst(arg), ATAgetFirst(pat), vars);
    arg = ATgetNext(arg);
    pat = ATgetNext(pat);
  }

  return r;
}

static bool statearg_match(ATermAppl arg, ATermAppl pat, ATermTable vars)
{
  ATermTable tmp_vars = vars;
  if (vars == NULL)
  {
    tmp_vars = ATtableCreate(20,50);
  }


  bool r;
  if (gsIsDataAppl(pat))
  {
    if (!gsIsDataAppl(arg))
    {
      r = false;
    }
    else
    {
      r = statearg_match(ATAgetArgument(arg,0),ATAgetArgument(pat,0),tmp_vars)
          && statearg_match_list(ATLgetArgument(arg,1),ATLgetArgument(pat,1),tmp_vars);
    }
  }
  else if (gsIsDataVarId(pat))
  {
    ATerm val = ATtableGet(tmp_vars,(ATerm) pat);
    if (val == NULL)
    {
      ATtablePut(tmp_vars,(ATerm) pat, (ATerm) arg);
      r = true;
    }
    else if (ATisEqual(arg,val))
    {
      r = true;
    }
    else
    {
      r = false;
    }
  }
  else
  {
    r = ATisEqual(arg,pat);
  }

  if (vars == NULL)
  {
    ATtableDestroy(tmp_vars);
  }

  return r;
}

ATerm NextState::parseStateVector(ATermAppl state, ATerm match)
{
  if (!stateAFun_made)
  {
    stateAFun_made = true;
    info.stateAFun = ATmakeAFun("STATE",info.statelen,false);
    ATprotectAFun(info.stateAFun);
  }

  if (ATisEqualAFun(info.stateAFun,ATgetAFun(state)))
  {
    bool valid = true;
    ATermList l = info.procvars;
    for (size_t i=0; i<info.statelen; i++)
    {
      stateargs[i] = ATgetArgument(state,i);
      if (mcrl2::data::data_expression((ATermAppl) stateargs[i]).sort() != mcrl2::data::data_expression(ATAgetFirst(l)).sort())
      {
        valid = false;
        break;
      }
      if ((match != NULL) && !statearg_match((ATermAppl) stateargs[i],getStateArgument(match,i)))
      {
        valid = false;
        break;
      }
      stateargs[i] = (ATerm)(ATermAppl)info.import_term((ATermAppl) stateargs[i]);
      l = ATgetNext(l);
    }
    if (valid)
    {
      switch (info.stateformat)
      {
        case GS_STATE_VECTOR:
          return (ATerm) ATmakeApplArray(info.stateAFun,stateargs);
          break;
        case GS_STATE_TREE:
          return (ATerm) buildTree(stateargs);
          break;
        default:
          break;
      }
    }
  }

  return NULL;
}

ATerm NextState::SetVars(ATerm a, ATermList free_vars)
{
  if (!usedummies)
  {
    return a;
  }

  mcrl2::data::representative_generator generator(info.m_specification);

  if (ATisList(a))
  {
    ATermList l = (ATermList) a;
    ATermList m = ATmakeList0();
    for (; !ATisEmpty(l); l=ATgetNext(l))
    {
      m = ATinsert(m,SetVars(ATgetFirst(l),free_vars));
    }
    return (ATerm) ATreverse(m);
  }
  else if (gsIsDataVarId((ATermAppl) a))
  {
    if (ATindexOf(free_vars,a,0) != ATERM_NON_EXISTING_POSITION)
    {
      return reinterpret_cast< ATerm >(static_cast< ATermAppl >(
                                         generator(mcrl2::data::sort_expression(ATAgetArgument((ATermAppl) a,1)))));
    }
    else
    {
      return a;
    }
  }
  else if (gsIsDataAppl((ATermAppl) a))
  {
    return (ATerm) gsMakeDataAppl((ATermAppl) SetVars(ATgetArgument((ATermAppl) a,0),free_vars),(ATermList) SetVars(ATgetArgument((ATermAppl) a,1),free_vars));
  }
  else
  {
    return a;
  }
}

ATermList NextState::ListToFormat(ATermList l,ATermList free_vars)
{
  if (ATisEmpty(l))
  {
    return l;
  }
  else
  {
    return ATinsert(ListToFormat(ATgetNext(l),free_vars),(ATerm)(ATermAppl)info.import_term((ATermAppl) SetVars(ATgetFirst(l),free_vars)));
  }
}

ATermList NextState::ListFromFormat(ATermList l)
{
  if ( ATisEmpty(l) ) 
  { 
    return l; 
  } 
  else 
  { 
    return ATinsert(ListFromFormat(ATgetNext(l)),(ATerm) info.export_term(ATgetFirst(l))); 
  }
}

ATermList NextStateGenerator::ListFromFormat(ATermList l)
{
  if ( ATisEmpty(l) ) 
  { 
    return l; 
  } 
  else 
  { 
    return ATinsert(ListFromFormat(ATgetNext(l)),(ATerm) info.export_term(ATgetFirst(l)));
  }
}

ATermAppl NextState::ActionToRewriteFormat(ATermAppl act, ATermList free_vars)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextState::ActionToRewriteFormat(act, free_vars) called, with" << std::endl <<
            "  act = " << atermpp::aterm_appl(act) << std::endl <<
            "  act (human readable): " << pp(atermpp::aterm_appl(act)) << std::endl <<
            "  free_vars = " << atermpp::aterm_list(free_vars) << std::endl;
#endif
  ATermList l = ATLgetArgument(act,0);
  ATermList m = ATmakeList0();

  for (; !ATisEmpty(l); l=ATgetNext(l))
  {
    ATermAppl a = ATAgetFirst(l);
    a = gsMakeAction(ATAgetArgument(a,0),ListToFormat(ATLgetArgument(a,1),free_vars));
    m = ATinsert(m,(ATerm) a);
  }
  m = ATreverse(m);

  return gsMakeMultAct(m);
}

ATermList NextState::AssignsToRewriteFormat(ATermList assigns, ATermList free_vars)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextState::AssignsToRewriteFormat(assigns, free_vars) called, with: " << std::endl <<
            "  assigns = " << atermpp::aterm_list(assigns) << std::endl <<
            "  (human readable assigns): " << pp(atermpp::aterm_list(assigns)) << std::endl <<
            "  free_vars = " << atermpp::aterm_list(free_vars) << std::endl;
#endif
  size_t i = 0;
  for (ATermList l=pars; !ATisEmpty(l); l=ATgetNext(l),i++)
  {
    bool set = false;

    for (ATermList m=assigns; !ATisEmpty(m); m=ATgetNext(m))
    {
      if (ATisEqual(ATAgetArgument(ATAgetFirst(m),0),ATAgetFirst(l)))
      {
        stateargs[i] = (ATerm)(ATermAppl)info.import_term((ATermAppl) SetVars(ATgetArgument(ATAgetFirst(m),1),free_vars));
        set = true;
        break;
      }
    }
    if (!set)
    {
      stateargs[i] = (ATerm) gsMakeNil();
    }
  }

  ATermList r = ATmakeList0();
  i=info.statelen;
  while (i != 0)
  {
    i--;
    r = ATinsert(r,stateargs[i]);
  }
  return r;
}

NextState::NextState(mcrl2::lps::specification const& spec, 
                                     bool allow_free_vars, 
                                     int state_format,
                                     const mcrl2::data::detail::legacy_rewriter& rewriter) : info(spec.data(), rewriter)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextState::Nextate(spec, allow_free_vars, state_format, rewriter), with" <<  std::endl <<
            "  allow_free_vars = " << allow_free_vars << std::endl <<
            "  state_format = " << state_format << std::endl <<
            "  LPS= " << mcrl2::lps::pp(spec) << std::endl;
#endif
  ATermList l,m,n,free_vars;

  next_id = 0;
  info.current_id = &current_id;

  info.parent = this;

  usedummies = !allow_free_vars;

  tree_init = NULL;

  info.stateformat = state_format;
  info.pairAFun = ATmakeAFun("@STATE_PAIR@",2,false);
  ATprotectAFun(info.pairAFun);

  info.nil = NULL;
  ATprotectAppl(&info.nil);
  info.nil = gsMakeNil();

  // Declare all constructors to the rewriter to prevent unnecessary compilation.
  // This can be removed if the jittyc or innerc compilers are not in use anymore.
  // In certain cases it could be useful to add the mappings also, but this appears to
  // give a substantial performance penalty, due to the addition of symbols to the
  // rewriter that are not used.
  const mcrl2::data::function_symbol_vector constructors(spec.data().constructors());
  for (mcrl2::data::function_symbol_vector::const_iterator i = constructors.begin(); i != constructors.end(); ++i)
  {
    info.import_term(mcrl2::data::data_expression(*i));
  }

  free_vars = atermpp::convert< mcrl2::data::variable_list >(spec.global_variables());

  pars = NULL;
  ATprotectList(&pars);
  pars = spec.process().process_parameters();

  info.statelen = ATgetLength(pars);
  if (info.stateformat == GS_STATE_VECTOR)
  {
    stateAFun_made = true;
    info.stateAFun = ATmakeAFun("STATE",info.statelen,false);
    ATprotectAFun(info.stateAFun);
  }
  else
  {
    stateAFun_made = false;
  }

  info.procvars = NULL;
  ATprotectList(&info.procvars);
  info.procvars = spec.process().process_parameters();

  stateargs = (ATerm*) malloc(info.statelen*sizeof(ATerm));
  for (size_t i=0; i<info.statelen; i++)
  {
    stateargs[i] = NULL;
  }
  ATprotectArray(stateargs,info.statelen);

  smndAFun = ATmakeAFun("@SMND@",4,false);
  ATprotectAFun(smndAFun);
  ATermList sums = mcrl2::lps::deprecated::linear_process_summands(spec.process());
  l = ATmakeList0();
  for (bool b=true; !ATisEmpty(sums); sums=ATgetNext(sums))
  {
    if (b && !gsIsNil(ATAgetArgument(ATAgetFirst(sums),3)))   // Summand is timed
    {
      gsWarningMsg("specification uses time, which is (currently) not supported; ignoring timing\n");
      b = false;
    }
    if (!gsIsDelta(ATAgetArgument(ATAgetFirst(sums),2)))
    {
      l = ATinsert(l,ATgetFirst(sums));
    }
  }
  sums = ATreverse(l);
  info.num_summands = ATgetLength(sums);
  info.num_prioritised = 0;
  info.summands = (ATermAppl*) malloc(info.num_summands*sizeof(ATermAppl));
  for (size_t i=0; i<info.num_summands; i++)
  {
    info.summands[i] = NULL;
  }
  ATprotectArray((ATerm*) info.summands,info.num_summands);
  for (size_t i=0; !ATisEmpty(sums); sums=ATgetNext(sums),i++)
  {
    info.summands[i] =
      ATmakeAppl4(
        smndAFun,
        ATgetArgument(ATAgetFirst(sums),0),
        (ATerm)(ATermAppl)info.import_term((ATermAppl) SetVars(ATgetArgument(ATAgetFirst(sums),1),free_vars)),
        (ATerm) ActionToRewriteFormat(ATAgetArgument(ATAgetFirst(sums),2),free_vars),
        (ATerm) AssignsToRewriteFormat(ATLgetArgument(ATAgetFirst(sums),4),free_vars));
  }

  l = pars;
  m = spec.initial_process().assignments();

  for (size_t i=0; !ATisEmpty(l); l=ATgetNext(l), i++)
  {
    n = m;
    bool set = false;
    for (; !ATisEmpty(n); n=ATgetNext(n))
    {
      if (ATisEqual(ATAgetArgument(ATAgetFirst(n),0),ATAgetFirst(l)))
      {
        stateargs[i] = (ATerm)(ATermAppl)info.import_term((ATermAppl) SetVars(ATgetArgument(ATAgetFirst(n),1),free_vars));
        set = true;
        break;
      }
    }
    if (!set)
    {
      gsErrorMsg("Parameter '%T' does not have an initial value.",ATgetArgument(ATAgetFirst(l),0));
      initial_state = NULL;
      ATprotect(&initial_state);
      return;
    }
  }

  // Rewrite the state arguments en block, as otherwise the generation of new symbols in the
  // rewriter is intermingled with rewriting, causing the rewriter to rewrite too often.
  for (size_t i=0; i<info.statelen; i++)
  {
    stateargs[i] = (ATerm)(ATermAppl)info.m_rewriter.rewrite_internal((atermpp::aterm_appl)stateargs[i]);
  }

  initial_state = NULL;
  ATprotect(&initial_state);

  switch (info.stateformat)
  {
    case GS_STATE_VECTOR:
      initial_state = (ATerm) ATmakeApplArray(info.stateAFun,stateargs);
      break;
    case GS_STATE_TREE:
      initial_state = (ATerm) buildTree(stateargs);
      break;
    default:
      break;
  }
}

NextState::~NextState()
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextState::~NextState called" << std::endl;
#endif
  ATunprotect(&initial_state);

  ATunprotectArray(stateargs);

  ATunprotectAppl(&info.nil);

  ATunprotectAFun(info.pairAFun);

  ATunprotectList(&pars);
  if (stateAFun_made)
  {
    ATunprotectAFun(info.stateAFun);
  }

  ATunprotectList(&info.procvars);

  ATunprotectAFun(smndAFun);
  ATunprotectArray((ATerm*) info.summands);
  free(info.summands);

  free(tree_init);
}

static bool only_action(ATermList ma, const char* action)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "only_action(ma, action) called, with " << std::endl <<
            "  ma = " << atermpp::aterm_list(ma) << std::endl <<
            "  action = " << std::string(action) << std::endl;
#endif
  if (ATisEmpty(ma))
  {
    return false;
  }

  for (; !ATisEmpty(ma); ma=ATgetNext(ma))
  {
    if (strcmp(ATgetName(ATgetAFun(ATAgetArgument(ATAgetArgument(ATAgetFirst(ma),0),0))),action))
    {
      return false;
    }
  }
  return true;
}
void NextState::prioritise(const char* action)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "Nextstate::prioritise called" << std::endl;
#endif
  // XXX this function invalidates currently used generators!
  // perhaps
  bool is_tau = !strcmp(action,"tau");
  size_t pos = 0;
  size_t rest = 0;

  while (pos < info.num_summands)
  {
    ATermAppl s = info.summands[pos];
    ATermList ma = ATLgetArgument(ATAgetArgument(s,2),0);
    if ((is_tau && ATisEmpty(ma)) || (!is_tau && only_action(ma,action)))
    {
      //if ( rest < pos )
      //{
      info.summands[pos] = info.summands[rest];
      info.summands[rest] = s;
      //}
      rest++;
    }
    pos++;
  }

  info.num_prioritised += rest;
}

ATerm NextState::getInitialState()
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextState::getInitialState()." << std::endl <<
            "Result: " << atermpp::aterm(initial_state) << std::endl <<
            "Result (human readable): " << print_state(atermpp::aterm_appl(initial_state), info) << std::endl;
#endif
  return initial_state;
}

NextStateGenerator* NextState::getNextStates(ATerm state, NextStateGenerator* old)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextState::getNextStates(state, old) called, with " <<
            "  state = " << atermpp::aterm(state) << "old = " << old << std::endl <<
            "  (human readable state) " << print_state(atermpp::aterm_appl(state), info) << std::endl;
#endif
  if (old == NULL)
  {
    return new NextStateGenerator(state,info,next_id++);
  }

  static_cast< NextStateGenerator* >(old)->reset(state);

  return old;
}

class NextStateGeneratorSummand : public NextStateGenerator
{

  public:

    NextStateGeneratorSummand(size_t summand, ATerm state, ns_info& info, size_t identifier)
      : NextStateGenerator(state, info, identifier, true, summand)
    {

#ifdef MCRL2_NEXTSTATE_DEBUG
      std::clog << "NextStateGeneratorSummand(summand, state, info, identifier)" <<
                "  summand = " << summand << std::endl <<
                "  state = " << atermpp::aterm(state) << std::endl <<
                "  (human readable state) " << print_state(atermpp::aterm_appl(state), info) << std::endl <<
                "  identifier = " << identifier << std::endl;
#endif
      assert(summand < info.num_summands);
    }
};

NextStateGenerator* NextState::getNextStates(ATerm state, size_t index, NextStateGenerator* old)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextState::getNextStates(state, index, old) called, with " <<
            "  state = " << atermpp::aterm(state) << std::endl <<
            "  (human readable state) " << print_state(atermpp::aterm_appl(state), info) << std::endl <<
            "  index = " << index << std::endl <<
            "  old = " << old << std::endl;
#endif
  assert(index < info.num_summands);
  if (old != 0)
  {
    static_cast< NextStateGeneratorSummand* >(old)->reset(state, index);

    return old;
  }

  return new NextStateGeneratorSummand(index, state,info,next_id++);
}

ATerm NextStateGenerator::makeNewState(ATerm old, ATermList assigns)
{ 
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextStateGenerator::makeNewState(old, assigns) called, with " << std::endl <<
            "  old = " << atermpp::aterm(old) << std::endl <<
            "  (human readable old): " << print_state(atermpp::aterm_appl(old), info) << std::endl <<
            "  assigns = " << atermpp::aterm_list(assigns) << std::endl;
  std::clog <<
            "  (human readable assigns): " << print_assignments(atermpp::aterm_list(assigns), info) << std::endl;
#endif
  ATermList l;

  if (*info.current_id != id)
  {
    set_substitutions();
  }

  l = info.procvars;
  for (size_t i=0; i<info.statelen; i++)
  {
    ATerm a = ATgetFirst(assigns);
    assigns = ATgetNext(assigns);
    if (ATisEqual(a,info.nil))
    {
      switch (info.stateformat)
      {
        default:
        case GS_STATE_VECTOR:
          stateargs[i] = ATgetArgument((ATermAppl) old,i);
          break;
        case GS_STATE_TREE:
//                                      stateargs[i] = getTreeElement(old,i);
          stateargs[i] = (ATerm)(ATermAppl)info.m_rewriter.internally_associated_value(variable(ATgetFirst(l)));
          if (ATisEqual(stateargs[i], ATgetFirst(l)))   // Make sure substitutions were not reset by enumerator
          {
            set_substitutions();
            stateargs[i] = (ATerm)(ATermAppl)info.m_rewriter.internally_associated_value(variable(ATgetFirst(l)));
          }
          break;
      }
    }
    else
    {

      stateargs[i] = (ATerm)(ATermAppl)info.m_rewriter.rewrite_internal((atermpp::aterm_appl)a);
      // The assertion below is not true if there are global variables,
      // which is for instance the case for lpsxsim and lpssim.
      // assert(mcrl2::data::find_variables(atermpp::make_list(mcrl2::data::data_expression(info.export_term(stateargs[i])))).empty());
    }
    l = ATgetNext(l);
  }
  switch (info.stateformat)
  {
      ATerm r;
    case GS_STATE_VECTOR:
      r = (ATerm) ATmakeApplArray(info.stateAFun,stateargs);

      return r;
    case GS_STATE_TREE:
      return info.parent->buildTree(stateargs);
    default:
      return NULL;
  }
}

ATermAppl NextStateGenerator::rewrActionArgs(ATermAppl act)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextStateGenerator::rewrActionArgs(act) called, with " << std::endl <<
            "  act = " << atermpp::aterm_appl(act) << std::endl;
#endif
  ATermList l = ATLgetArgument(act,0);
  ATermList m = ATmakeList0();

  for (; !ATisEmpty(l); l=ATgetNext(l))
  {
    ATermAppl a = ATAgetFirst(l);
    const atermpp::term_list <atermpp::aterm_appl> l=ATLgetArgument(a,1);
    a = gsMakeAction(ATAgetArgument(a,0),ListFromFormat(info.m_rewriter.rewrite_internal(l)));
    m = ATinsert(m,(ATerm) a);
  }
  m = ATreverse(m);

  return gsMakeMultAct(m);
}

void NextStateGenerator::SetTreeStateVars(ATerm tree, ATermList* vars)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextStateGenerator::SetTreeStateVars(tree, vars) called, with " << std::endl <<
            "  tree = " << atermpp::aterm(tree) << std::endl <<
            "  *vars = " << atermpp::aterm_list(*vars) << std::endl;
#endif

  if (ATgetType(tree) == AT_APPL)
  {
    if (ATisEqual(tree,info.nil))
    {
      return;
    }
    else if (ATisEqualAFun(ATgetAFun((ATermAppl) tree),info.pairAFun))
    {
      SetTreeStateVars(ATgetArgument((ATermAppl) tree,0),vars);
      SetTreeStateVars(ATgetArgument((ATermAppl) tree,1),vars);
      return;
    }
  }

  info.m_rewriter.set_internally_associated_value(variable(ATgetFirst(*vars)), tree);
  *vars = ATgetNext(*vars);
}

NextStateGenerator::NextStateGenerator(ATerm State, ns_info& Info, size_t identifier, bool SingleSummand, size_t SingleSummandIndex) : info(Info)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextStateGenerator::NextStateGenerator(State, Info, identifier, SingleSummand) called, with " << std::endl <<
            "  State = " << atermpp::aterm(State) <<
            "  State (human readable): " << print_state(atermpp::aterm_appl(State), info) << std::endl <<
            "  identifier = " << identifier << std::endl <<
            "  SingleSummand = " << (SingleSummand?"true":"false") << std::endl <<
            "  SingleSummandIndex = " << SingleSummandIndex << std::endl;
#endif
  id = identifier;
  // error = false;
  single_summand = SingleSummand;

  cur_state = NULL;
  cur_act = NULL;
  cur_nextstate = NULL;
  ATprotect(&cur_state);
  ATprotect(&cur_act);
  ATprotectList(&cur_nextstate);

  stateargs = (ATerm*) malloc(info.statelen*sizeof(ATerm));
  for (size_t i=0; i<info.statelen; i++)
  {
    stateargs[i] = NULL;
  }
  ATprotectArray(stateargs,info.statelen);

  reset(State, SingleSummandIndex);
}

NextStateGenerator::~NextStateGenerator()
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextStateGenerator::~NextStateGenerator called" << std::endl;
#endif
  ATunprotectArray(stateargs);
  free(stateargs);

  ATunprotectList(&cur_nextstate);
  ATunprotect(&cur_act);
  ATunprotect(&cur_state);
}

void NextStateGenerator::set_substitutions()
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextStateGenerator::set_substitutions called" << std::endl;
#endif

  ATermList l = info.procvars;
  switch (info.stateformat)
  {
    case GS_STATE_VECTOR:
      for (size_t i=0; !ATisEmpty(l); l=ATgetNext(l),i++)
      {
        ATerm a = ATgetArgument((ATermAppl) cur_state,i);
        if (!ATisEqual(a,info.nil))
        {
          info.m_rewriter.set_internally_associated_value(variable(ATgetFirst(l)), a);
#ifdef MCRL2_NEXTSTATE_DEBUG
          std::cerr << "Set substitution " << pp(info.export_term(ATgetFirst(l))) << ":=" <<
                    pp(info.export_term(a)) << "\n";
#endif
        }
      }
      break;
    case GS_STATE_TREE:
      SetTreeStateVars(cur_state,&l);
      break;
  }

  *info.current_id = id;
}

void NextStateGenerator::reset(ATerm State, size_t SummandIndex)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextStateGenerator::reset(State, SummandIndex) called with:" << std::endl <<
            "  State = " << atermpp::aterm(State) << std::endl <<
            "  State (human readable) = " << print_state(atermpp::aterm_appl(State), info) << std::endl <<
            "  SummandIndex = " << SummandIndex << std::endl;
#endif
  // error = false;

  cur_state = State;

  set_substitutions();

  if (info.num_summands == 0)
  {
    enumerated_variables=variable_list();
    valuations = info.get_sols(ATmakeList0(),(ATerm)(ATermAppl)info.import_term(mcrl2::data::sort_bool::false_()));
  }
  else
  {
    assert(SummandIndex < info.num_summands);
#ifdef MCRL2_NEXTSTATE_DEBUG
    std::clog << "Getting solutions for this summand" << std::endl <<
              "  Sum variables: " << atermpp::aterm(ATLgetArgument(info.summands[SummandIndex],0)) << std::endl <<
              "                 " << pp(atermpp::aterm(ATLgetArgument(info.summands[SummandIndex],0))) << std::endl <<
              "  Condition: " << atermpp::aterm(ATgetArgument(info.summands[SummandIndex],1)) << std::endl <<
              "             " << pp(atermpp::aterm_appl(info.export_term(ATgetArgument(info.summands[SummandIndex],1)))) << std::endl;
#endif

    cur_act = ATgetArgument(info.summands[SummandIndex],2);
    cur_nextstate = (ATermList) ATgetArgument(info.summands[SummandIndex],3);

    enumerated_variables=ATLgetArgument(info.summands[SummandIndex],0);
    valuations = info.get_sols(ATLgetArgument(info.summands[SummandIndex],0),
                               ATgetArgument(info.summands[SummandIndex],1));
  }

  sum_idx = SummandIndex + 1;
}

bool NextStateGenerator::next(ATermAppl* Transition, ATerm* State, bool* prioritised)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextStateGenerator::next(Transition, State, prioritised) called" << std::endl;
#endif
  while (valuations == ns_info::enumerator_type::iterator_internal() && (sum_idx < info.num_summands))   // valuations is empty.
  {
    if (single_summand)
    {
#ifdef MCRL2_NEXTSTATE_DEBUG
      std::clog << "NextStateGenerator::next: single_summand is true, and no next state found in this summand;" << std::endl <<
                "  sum_idx was: " << sum_idx << std::endl;
#endif
      return false;
    }

    cur_act = ATgetArgument(info.summands[sum_idx],2);
    cur_nextstate = (ATermList) ATgetArgument(info.summands[sum_idx],3);

    if (*info.current_id != id)
    {
      set_substitutions();
    }

#ifdef MCRL2_NEXTSTATE_DEBUG
    std::clog << "Getting solutions for summand " << sum_idx << std::endl <<
              "  Sum variables: " << atermpp::aterm(ATLgetArgument(info.summands[sum_idx],0)) << std::endl <<
              "                 " << pp(atermpp::aterm(ATLgetArgument(info.summands[sum_idx],0))) << std::endl <<
              "  Condition: " << atermpp::aterm(ATgetArgument(info.summands[sum_idx],1)) << std::endl <<
              "             " << pp(atermpp::aterm_appl(info.export_term(ATgetArgument(info.summands[sum_idx],1)))) << std::endl;
#endif

    enumerated_variables=ATLgetArgument(info.summands[sum_idx],0);
// std::cerr << "ENUMERATED VARIABLES " << pp(enumerated_variables) << "\n";
    valuations = info.get_sols(ATLgetArgument(info.summands[sum_idx],0),
                               ATgetArgument(info.summands[sum_idx],1));

    ++sum_idx;
  }

  if (valuations != ns_info::enumerator_type::iterator_internal())   // valuations contains unprocessed valuations.
  {
    if (*info.current_id != id)
    {
      set_substitutions();
    }
    ATermList assignments= *valuations;
    assert(ATgetLength(assignments)==enumerated_variables.size());
    variable_list::const_iterator j=enumerated_variables.begin();
    for( ; !ATisEmpty(assignments) ;  assignments=ATgetNext(assignments), ++j)
    // for (ns_info::enumerator_type::substitution_type::const_iterator i(valuations->begin()); i != valuations->end(); ++i)
    {
      ATerm t=ATgetFirst(assignments);
// ATfprintf(stderr,"ASSIGNMENT %t   %t\n", (ATermAppl)*j,t);
      // info.m_rewriter.set_internally_associated_value(static_cast< ATermAppl >(i->first), i->second);
      info.m_rewriter.set_internally_associated_value(*j,t);
    }

    *Transition = rewrActionArgs((ATermAppl) cur_act);
    *State = (ATerm) makeNewState(cur_state,cur_nextstate);

    if (prioritised != NULL)
    {
      *prioritised = (sum_idx <= info.num_prioritised);
    }

    for(variable_list::const_iterator j=enumerated_variables.begin(); 
              j!=enumerated_variables.end(); ++j)
    {
      info.m_rewriter.clear_internally_associated_value(*j);
    }
    ++valuations;

    return true;
  }
  else
  {
    *Transition = NULL;
    *State = NULL;
  }

  return false;
}

ATerm NextStateGenerator::get_state() const
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "NextStateGenerator::get_state() called" << std::endl <<
            "  result: " << atermpp::aterm(cur_state) << std::endl <<
            "  result (human readable): " << print_state(atermpp::aterm_appl(cur_state), info) << std::endl;
#endif
  return cur_state;
}

NextState* createNextState(
  mcrl2::lps::specification const& spec,
  const mcrl2::data::rewriter &rewriter,
  bool allow_free_vars,
  int state_format,
  NextStateStrategy strategy
)
{
#ifdef MCRL2_NEXTSTATE_DEBUG
  std::clog << "createNextState(spec, e, allow_free_vars, state_format, strategy) called" << std::endl;
#endif
  switch (strategy)
  {
    default:
    case nsStandard:
      return new NextState(spec,allow_free_vars,state_format,rewriter);
  }

  return NULL;
}
