// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file jittyc.cpp

#include "mcrl2/data/detail/rewrite.h" // Required fo MCRL2_JITTTYC_AVAILABLE.

#ifdef MCRL2_JITTYC_AVAILABLE

#define NAME "rewr_jittyc"

#include <utility>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <sstream>
#include <sys/stat.h>
#include "mcrl2/utilities/file_utility.h"
#include "mcrl2/utilities/detail/memory_utility.h"
#include "mcrl2/utilities/basename.h"
#include "mcrl2/utilities/logger.h"
#include "mcrl2/core/print.h"
#include "mcrl2/core/detail/function_symbols.h"
#include "mcrl2/data/detail/rewrite/jittyc.h"
#include "mcrl2/data/detail/rewrite/jitty_jittyc.h"
#include "mcrl2/data/replace.h"
#include "mcrl2/data/traverser.h"
#include "mcrl2/data/substitutions/mutable_map_substitution.h"

#ifdef MCRL2_DISPLAY_REWRITE_STATISTICS
#include "mcrl2/data/detail/rewrite_statistics.h"
#endif

using namespace mcrl2::core;
using namespace mcrl2::core::detail;
using namespace std;
using namespace atermpp;
using namespace mcrl2::log;

namespace mcrl2
{
namespace data
{
namespace detail
{

typedef atermpp::term_list<variable_list> variable_list_list;

static const match_tree dummy=match_tree();
static atermpp::function_symbol afunARtrue, afunARfalse, afunARand, afunARor, afunARvar;
static atermpp::aterm_appl ar_true, ar_false;

static bool is_initialised = false;

static void initialise_common()
{
  if (!is_initialised)
  {
    is_initialised=true;

    afunARtrue = atermpp::function_symbol("@@true",0);
    afunARfalse = atermpp::function_symbol("@@false",0);
    afunARand = atermpp::function_symbol("@@and",2);
    afunARor = atermpp::function_symbol("@@or",2);
    afunARvar = atermpp::function_symbol("@@var",1);
    ar_true = atermpp::aterm_appl(afunARtrue);
    ar_false = atermpp::aterm_appl(afunARfalse);
  }
}

#define is_ar_true(x) (x==ar_true)
#define is_ar_false(x) (x==ar_false)
#define is_ar_and(x) (x.function()==afunARand)
#define is_ar_or(x) (x.function()==afunARor)
#define is_ar_var(x) (x.function()==afunARvar)

size_t RewriterCompilingJitty::ar_index(
                const data::function_symbol& f,
                const size_t arity,
                const size_t arg)
{
  assert(arg<arity);
  assert(int2ar_idx[f]+((arity-1)*arity)/2+arg<ar.size());
  return int2ar_idx[f]+((arity-1)*arity)/2+arg;

}

atermpp::aterm_appl RewriterCompilingJitty::get_ar_array(
                const data::function_symbol& f,
                const size_t arity,
                const size_t arg)
{
  return ar[ar_index(f,arity,arg)];
}


void RewriterCompilingJitty::set_ar_array(
                const data::function_symbol& f,
                const size_t arity,
                const size_t arg,
                const atermpp::aterm_appl ar_expression)
{
  ar[ar_index(f,arity,arg)]=ar_expression;
}


static atermpp::aterm_appl make_ar_true()
{
  return ar_true;
}

static atermpp::aterm_appl make_ar_false()
{
  return ar_false;
}

static atermpp::aterm_appl make_ar_and(const atermpp::aterm_appl& x, const atermpp::aterm_appl& y)
{
  if (is_ar_true(x))
  {
    return y;
  }
  else if (is_ar_true(y))
  {
    return x;
  }
  else if (is_ar_false(x) || is_ar_false(y))
  {
    return make_ar_false();
  }

  return atermpp::aterm_appl(afunARand,x,y);
}

static atermpp::aterm_appl make_ar_or(atermpp::aterm_appl x, atermpp::aterm_appl y)
{
  if (is_ar_false(x))
  {
    return y;
  }
  else if (is_ar_false(y))
  {
    return x;
  }
  else if (is_ar_true(x) || is_ar_true(y))
  {
    return make_ar_true();
  }

  return atermpp::aterm_appl(afunARor,x,y);
}

static atermpp::aterm_appl make_ar_var(size_t var)
{
  return atermpp::aterm_appl(afunARvar,atermpp::aterm_int(var));
}

static char* whitespace_str = NULL;
static size_t whitespace_len;
static size_t whitespace_pos;
static char* whitespace(size_t len)
{
  if (whitespace_str == NULL)
  {
    whitespace_str = (char*) malloc((2*len+1)*sizeof(char));
    for (size_t i=0; i<2*len; i++)
    {
      whitespace_str[i] = ' ';
    }
    whitespace_len = 2*len;
    whitespace_pos = len;
    whitespace_str[whitespace_pos] = 0;
  }
  else
  {
    if (len > whitespace_len)
    {
      whitespace_str = (char*) realloc(whitespace_str,(2*len+1)*sizeof(char));
      for (size_t i=whitespace_len; i<2*len; i++)
      {
        whitespace_str[i] = ' ';
      }
      whitespace_len = 2*len;
    }

    whitespace_str[whitespace_pos] = ' ';
    whitespace_pos = len;
    whitespace_str[whitespace_pos] = 0;
  }

  return whitespace_str;
}


///
/// \brief arity_is_allowed yields true if the function indicated by the function index can
///        legitemately be used with a arguments. A function f:D1x...xDn->D can be used with 0 and
///        n arguments. A function f:(D1x...xDn)->(E1x...Em)->F can be used with 0, n, and n+m
///        arguments.
/// \param s A function sort
/// \param a The desired number of arguments
/// \return A boolean indicating whether a term of sort s applied to a arguments is a valid term.
///
static bool arity_is_allowed(const sort_expression& s, const size_t a)
{
  if (a == 0)
  {
    return true;
  }
  if (is_function_sort(s))
  {
    const function_sort& fs = atermpp::down_cast<function_sort>(s);
    size_t n = fs.domain().size();
    return n <= a && arity_is_allowed(fs.codomain(), a - n);
  }
  return false;
}

static void term2seq(const data_expression& t, match_tree_list& s, size_t *var_cnt, const bool ommit_head)
{
  if (is_function_symbol(t))
  {
    const function_symbol f(t);
    s.push_front(match_tree_F(f,dummy,dummy));
    s.push_front(match_tree_D(dummy,0));
    return;
  }

  if (is_variable(t))
  {
    const variable v(t);
    match_tree store = match_tree_S(v,dummy);

    if (std::find(s.begin(),s.end(),store) != s.end())
    {
      s.push_front(match_tree_M(v,dummy,dummy));
    }
    else
    {
      (*var_cnt)++;
      s.push_front(store);
    }
    return;
  }

  assert(is_application(t));
  const application ta(t);
  size_t arity = ta.size();

  if (is_application(ta.head()))
  {
    term2seq(ta.head(),s,var_cnt,ommit_head);
    s.push_front(match_tree_N(dummy,0));
  }
  else if (!ommit_head)
  {
    {
      s.push_front(match_tree_F(function_symbol(ta.head()),dummy,dummy));
    }
  }

  size_t j=1;
  for (application::const_iterator i=ta.begin(); i!=ta.end(); ++i,++j)
  {
    term2seq(*i,s,var_cnt,false);
    if (j<arity)
    {
      s.push_front(match_tree_N(dummy,0));
    }
  }

  if (!ommit_head)
  {
    s.push_front(match_tree_D(dummy,0));
  }
}

static variable_or_number_list get_used_vars(const data_expression& t)
{
  std::set <variable> vars = find_free_variables(t);
  return variable_or_number_list(vars.begin(),vars.end());
}

static match_tree_list create_sequence(const data_equation& rule, size_t* var_cnt)
{
  const data_expression lhs_inner = rule.lhs();
  const data_expression cond = rule.condition();
  const data_expression rslt = rule.rhs();
  match_tree_list rseq;

  if (!is_function_symbol(lhs_inner))
  {
    const application lhs_innera(lhs_inner);
    size_t lhs_arity = lhs_innera.size();

    if (is_application(lhs_innera.head()))
    {
      term2seq(lhs_innera.head(),rseq,var_cnt,true);
      rseq.push_front(match_tree_N(dummy,0));
    }

    size_t j=1;
    for (application::const_iterator i=lhs_innera.begin(); i!=lhs_innera.end(); ++i,++j)
    {
      term2seq(*i,rseq,var_cnt,false);
      if (j<lhs_arity)
      {
        rseq.push_front(match_tree_N(dummy,0));
      }
    }
  }

  if (cond==sort_bool::true_())
  {
    rseq.push_front(match_tree_Re(rslt,get_used_vars(rslt)));
  }
  else
  {
    rseq.push_front(match_tree_CRe(cond,rslt, get_used_vars(cond), get_used_vars(rslt)));
  }

  return reverse(rseq);
}


// Structure for build_tree parameters
typedef struct
{
  match_tree_list_list Flist;       // List of sequences of which the first action is an F
  match_tree_list_list Slist;       // List of sequences of which the first action is an S
  match_tree_list_list Mlist;       // List of sequences of which the first action is an M
  match_tree_list_list_list stack;  // Stack to maintain the sequences that do not have to
                                    // do anything in the current term
  match_tree_list_list upstack;     // List of sequences that have done an F at the current
                                    // level
} build_pars;

static void initialise_build_pars(build_pars* p)
{
  p->Flist = match_tree_list_list();
  p->Slist = match_tree_list_list();
  p->Mlist = match_tree_list_list();
  p->stack = make_list<match_tree_list_list>(match_tree_list_list());
  p->upstack = match_tree_list_list();
}

static match_tree_list_list_list add_to_stack(const match_tree_list_list_list& stack, const match_tree_list_list& seqs, match_tree_Re& r, match_tree_list& cr)
{
  if (stack.empty())
  {
    return stack;
  }

  match_tree_list_list l;
  match_tree_list_list h = stack.front();

  for (match_tree_list_list:: const_iterator e=seqs.begin(); e!=seqs.end(); ++e)
  {
    if (e->front().isD())
    {
      l.push_front(e->tail());
    }
    else if (e->front().isN())
    {
      h.push_front(e->tail());
    }
    else if (e->front().isRe())
    {
      r = match_tree_Re(e->front());
    }
    else
    {
      cr.push_front(e->front());
    }
  }

  match_tree_list_list_list result=add_to_stack(stack.tail(),l,r,cr);
  result.push_front(h);
  return result;
}

static void add_to_build_pars(build_pars* pars,  const match_tree_list_list& seqs, match_tree_Re& r, match_tree_list& cr)
{
  match_tree_list_list l;

  for (match_tree_list_list:: const_iterator e=seqs.begin(); e!=seqs.end(); ++e)
  {
    if (e->front().isD() || e->front().isN())
    {
      l.push_front(*e);
    }
    else if (e->front().isS())
    {
      pars->Slist.push_front(*e);
    }
    else if (e->front().isMe())     // M should not appear at the head of a seq
    {
      pars->Mlist.push_front(*e);
    }
    else if (e->front().isF())
    {
      pars->Flist.push_front(*e);
    }
    else if (e->front().isRe())
    {
      r = e->front();
    }
    else
    {
      cr.push_front(e->front());
    }
  }

  pars->stack = add_to_stack(pars->stack,l,r,cr);
}

static char tree_var_str[20];
static variable createFreshVar(const sort_expression& sort, size_t* i)
{
  sprintf(tree_var_str,"@var_%lu",(*i)++);
  return data::variable(tree_var_str, sort);
}

static match_tree_list subst_var(const match_tree_list& l,
                                 const variable& old,
                                 const variable& new_val,
                                 const size_t num,
                                 const mutable_map_substitution<>& substs)
{
  match_tree_vector result;
  for(match_tree_list::const_iterator i=l.begin(); i!=l.end(); ++i)
  {
    match_tree head=*i;
    if (head.isM())
    {
      const match_tree_M headM(head);
      if (headM.match_variable()==old)
      {
        assert(headM.true_tree()==dummy);
        assert(headM.false_tree()==dummy);
        head = match_tree_Me(new_val,num);
      }
    }
    else if (head.isCRe())
    {
      const match_tree_CRe headCRe(head);
      variable_or_number_list l = headCRe.variables_condition(); // This is a list with variables and aterm_ints.
      variable_or_number_list m ;                                // Idem.
      for (; !l.empty(); l=l.tail())
      {
        if (l.front()==old)
        {
          m.push_front(atermpp::aterm_int(num));
        }
        else
        {
          m.push_front(l.front());
        }
      }
      l = headCRe.variables_result();
      variable_or_number_list n;
      for (; !l.empty(); l=l.tail())
      {
        if (l.front()==old)
        {
          n.push_front(atermpp::aterm_int(num));
        }
        else
        {
          n.push_front(l.front());
        }
      }
      head = match_tree_CRe(replace_free_variables(headCRe.condition(),substs),replace_free_variables(headCRe.result(),substs),m, n);
    }
    else if (head.isRe())
    {
      const match_tree_Re& headRe(head);
      variable_or_number_list l = headRe.variables();
      variable_or_number_list m ;
      for (; !l.empty(); l=l.tail())
      {
        if (l.front()==old)
        {
          m.push_front(atermpp::aterm_int(num));
        }
        else
        {
          m.push_front(l.front());
        }
      }
      head = match_tree_Re(replace_free_variables(headRe.result(),substs),m);
    }
    result.push_back(head);
  }
  return match_tree_list(result.begin(),result.end());
}

static std::vector < size_t> treevars_usedcnt;

static void inc_usedcnt(const variable_or_number_list& l)
{
  for (variable_or_number_list::const_iterator i=l.begin(); i!=l.end(); ++i)
  {
    if (i->type_is_int())
    {
      treevars_usedcnt[deprecated_cast<atermpp::aterm_int>(*i).value()]++;
    }
  }
}

static match_tree build_tree(build_pars pars, size_t i)
{
  if (!pars.Slist.empty())
  {
    match_tree_list l;
    match_tree_list_list m;

    size_t k = i;
    const variable v = createFreshVar(match_tree_S(pars.Slist.front().front()).target_variable().sort(),&i);
    treevars_usedcnt[k] = 0;

    for (; !pars.Slist.empty(); pars.Slist=pars.Slist.tail())
    {
      match_tree_list e = pars.Slist.front();

      mutable_map_substitution<> sigma;
      sigma[match_tree_S(e.front()).target_variable()]=v;
      e = subst_var(e,
                    match_tree_S(e.front()).target_variable(),
                    v,
                    k,
                    sigma);

      l.push_front(e.front());
      m.push_front(e.tail());
    }

    match_tree_Re r;
    match_tree ret;
    match_tree_list readies;

    pars.stack = add_to_stack(pars.stack,m,r,readies);

    if (!r.is_defined())
    {
      match_tree tree = build_tree(pars,i);
      for (match_tree_list::const_iterator i=readies.begin(); i!=readies.end(); ++i)
      {
        match_tree_CRe t(*i);
        inc_usedcnt(t.variables_condition());
        inc_usedcnt(t.variables_result());
        tree = match_tree_C(t.condition(),match_tree_R(t.result()),tree);
      }
      ret = tree;
    }
    else
    {

      inc_usedcnt(r.variables());
      ret = match_tree_R(r.result());
    }

    if ((treevars_usedcnt[k] > 0) || ((k == 0) && ret.isR()))
    {
       return match_tree_S(v,ret);
    }
    else
    {
       return ret;
    }
  }
  else if (!pars.Mlist.empty())
  {
    match_tree_Me M(pars.Mlist.front().front());

    match_tree_list_list l;
    match_tree_list_list m;
    for (; !pars.Mlist.empty(); pars.Mlist=pars.Mlist.tail())
    {
      if (M==pars.Mlist.front().front())
      {
        l.push_front(pars.Mlist.front().tail());
      }
      else
      {
        m.push_front(pars.Mlist.front());
      }
    }
    pars.Mlist = m;

    match_tree true_tree,false_tree;
    match_tree_Re r ;
    match_tree_list readies;

    match_tree_list_list_list newstack = add_to_stack(pars.stack,l,r,readies);

    false_tree = build_tree(pars,i);

    if (!r.is_defined())
    {
      pars.stack = newstack;
      true_tree = build_tree(pars,i);
      for (; !readies.empty(); readies=readies.tail())
      {
        match_tree_CRe t(readies.front());
        inc_usedcnt(t.variables_condition());
        inc_usedcnt(t.variables_result());
        true_tree = match_tree_C(t.condition(), match_tree_R(t.result()),true_tree);
      }
    }
    else
    {
      inc_usedcnt(r.variables());
      true_tree = match_tree_R(r.result());
    }

    if (true_tree==false_tree)
    {
       return true_tree;
    }
    else
    {
      treevars_usedcnt[M.variable_index()]++;
      return match_tree_M(M.match_variable(),true_tree,false_tree);
    }
  }
  else if (!pars.Flist.empty())
  {
    match_tree_list F = pars.Flist.front();
    match_tree true_tree,false_tree;

    match_tree_list_list newupstack = pars.upstack;
    match_tree_list_list l;

    for (; !pars.Flist.empty(); pars.Flist=pars.Flist.tail())
    {
      if (pars.Flist.front().front()==F.front())
      {
        newupstack.push_front(pars.Flist.front().tail());
      }
      else
      {
        l.push_front(pars.Flist.front());
      }
    }

    pars.Flist = l;
    false_tree = build_tree(pars,i);
    pars.Flist = match_tree_list_list();
    pars.upstack = newupstack;
    true_tree = build_tree(pars,i);

    if (true_tree==false_tree)
    {
      return true_tree;
    }
    else
    {
      return match_tree_F(match_tree_F(F.front()).function(),true_tree,false_tree);
    }
  }
  else if (!pars.upstack.empty())
  {
    match_tree_list_list l;

    match_tree_Re r;
    match_tree_list readies;

    pars.stack.push_front(match_tree_list_list());
    l = pars.upstack;
    pars.upstack = match_tree_list_list();
    add_to_build_pars(&pars,l,r,readies);


    if (!r.is_defined())
    {
      match_tree t = build_tree(pars,i);

      for (; !readies.empty(); readies=readies.tail())
      {
        match_tree_CRe u(readies.front());
        inc_usedcnt(u.variables_condition());
        inc_usedcnt(u.variables_result());
        t = match_tree_C(u.condition(), match_tree_R(u.result()),t);
      }

      return t;
    }
    else
    {
      inc_usedcnt(r.variables());
      return match_tree_R(r.result());
    }
  }
  else
  {
    if (pars.stack.front().empty())
    {
      if (pars.stack.tail().empty())
      {
        return match_tree_X();
      }
      else
      {
        pars.stack = pars.stack.tail();
        return match_tree_D(build_tree(pars,i),0);
      }
    }
    else
    {
      match_tree_list_list l = pars.stack.front();
      match_tree_Re r ;
      match_tree_list readies;

      pars.stack = pars.stack.tail();
      pars.stack.push_front(match_tree_list_list());
      add_to_build_pars(&pars,l,r,readies);

      match_tree tree;
      if (!r.is_defined())
      {
        tree = build_tree(pars,i);
        for (; !readies.empty(); readies=readies.tail())
        {
          match_tree_CRe t(readies.front());
          inc_usedcnt(t.variables_condition());
          inc_usedcnt(t.variables_result());
          tree = match_tree_C(t.condition(), match_tree_R(t.result()),tree);
        }
      }
      else
      {
        inc_usedcnt(r.variables());
        tree = match_tree_R(r.result());
      }

      return match_tree_N(tree,0);
    }
  }
}

static match_tree create_tree(const data_equation_list& rules)
// Create a match tree for OpId int2term[opid] and update the value of
// *max_vars accordingly.
//
// Pre:  rules is a list of rewrite rules for some function symbol f.
//       max_vars is a valid pointer to an integer
// Post: *max_vars is the maximum of the original *max_vars value and
//       the number of variables in the result tree
// Ret:  A match tree for function symbol f.
{
  // Create sequences representing the trees for each rewrite rule and
  // store the total number of variables used in these sequences.
  // (The total number of variables in all sequences should be an upper
  // bound for the number of variable in the final tree.)
  match_tree_list_list rule_seqs;
  size_t total_rule_vars = 0;
  for (data_equation_list::const_iterator it=rules.begin(); it!=rules.end(); ++it)
  {
    rule_seqs.push_front(create_sequence(*it,&total_rule_vars));
  }
  // Generate initial parameters for built_tree
  build_pars init_pars;
  match_tree_Re r;
  match_tree_list readies;

  initialise_build_pars(&init_pars);
  add_to_build_pars(&init_pars,rule_seqs,r,readies);
  match_tree tree;
  if (!r.is_defined())
  {
    treevars_usedcnt=std::vector < size_t> (total_rule_vars);
    tree = build_tree(init_pars,0);
    for (; !readies.empty(); readies=readies.tail())
    {
      match_tree_CRe u(readies.front());
      tree = match_tree_C(u.condition(), match_tree_R(u.result()), tree);
    }
  }
  else
  {
    tree = match_tree_R(r.result());
  }

  return tree;
}

template < template <class> class Traverser >
struct auxiliary_count_variables_class: public Traverser < auxiliary_count_variables_class < Traverser > >
{
  typedef Traverser< auxiliary_count_variables_class < Traverser > > super;
  using super::enter;
  using super::leave;
  using super::apply;

  std::map <variable,size_t> m_map;

  void apply(const variable& v)
  {
    if (m_map.count(v)==0)
    {
      m_map[v]=1;
    }
    else
    {
      m_map[v]=m_map[v]+1;
    }
  }

  std::map <variable,size_t> get_map()
  {
    return m_map;
  }
};

static variable_list get_doubles(const data_expression& t)
{
  typedef std::map <variable,size_t> t_variable_map;
  auxiliary_count_variables_class<data::variable_traverser> acvc;
  acvc.apply(t);
  t_variable_map variable_map=acvc.get_map();
  variable_list result;
  for(t_variable_map::const_iterator i=variable_map.begin();
         i!=variable_map.end(); ++i)
  {
    if (i->second>1)
    {
      result.push_front(i->first);
    }
  }
  return result;
}

static variable_list dep_vars(const data_equation& eqn)
{
  size_t rule_arity=recursive_number_of_args(eqn.lhs());

  std::vector < bool > bs(rule_arity);

  const data_expression& lhs_internal = eqn.lhs();
  variable_list_list vars = make_list<variable_list>( get_doubles(eqn.rhs())+ get_vars(eqn.condition())
                               ); // List of variables occurring in each argument of the lhs
                                   // (except the first element which contains variables from the
                                   // condition and variables which occur more than once in the result)

  // Indices of arguments that need to be rewritten
  for (size_t i = 0; i < rule_arity; i++)
  {
    bs[i] = false;
  }

  // Check all arguments
  for (size_t i = 0; i < rule_arity; i++)
  {
    if (!is_variable(get_argument_of_higher_order_term(lhs_internal,i)))
    {
      // Argument is not a variable, so it needs to be rewritten
      bs[i] = true;
      variable_list evars = get_vars(get_argument_of_higher_order_term(lhs_internal,i));
      for (; !evars.empty(); evars=evars.tail())
      {
        int j=i-1; // vars.tail().size()-1
        for (variable_list_list o=vars.tail(); !o.empty(); o=o.tail())
        {
          const variable_list l=o.front();
          if (std::find(l.begin(),l.end(),evars.front()) != l.end())
          {
            bs[j] = true;
          }
          --j;
        }
      }
    }
    else
    {
      // Argument is a variable; check whether it occurred before
      int j = i-1; // vars.size()-1-1
      bool b = false;
      for (variable_list_list o=vars; !o.empty(); o=o.tail())
      {
        const variable_list l=o.front();
        if (std::find(o.begin(),o.end(),get_argument_of_higher_order_term(lhs_internal,i)) != o.end())
        {
          // Same variable, mark it
          if (j >= 0)
          {
            bs[j] = true;
          }
          b = true;
        }
        --j;
      }
      if (b)
      {
        // Found same variable(s), so mark this one as well
        bs[i] = true;
      }
    }
    // Add vars used in expression
    vars.push_front(get_vars(get_argument_of_higher_order_term(lhs_internal,i)));
  }

  variable_list deps;
  for (size_t i = 0; i < rule_arity; i++)
  {
    if (bs[i] && is_variable(get_argument_of_higher_order_term(lhs_internal,i)))
    {
      deps.push_front(atermpp::down_cast<variable>(get_argument_of_higher_order_term(lhs_internal,i)));
    }
  }

  return deps;
}

// This function assigns a unique index to variable v and stores
// v at this position in the vector rewriter_bound_variables. This is
// used in the compiling rewriter to obtain this variable again.
// Note that the static variable variable_indices is not cleared
// during several runs, as generally the variables bound in rewrite
// rules do not change.
size_t RewriterCompilingJitty::bound_variable_index(const variable& v)
{
  if (variable_indices0.count(v)>0)
  {
    return variable_indices0[v];
  }
  const size_t index_for_v=rewriter_bound_variables.size();
  variable_indices0[v]=index_for_v;
  rewriter_bound_variables.push_back(v);
  return index_for_v;
}

// This function assigns a unique index to variable list vl and stores
// vl at this position in the vector rewriter_binding_variable_lists. This is
// used in the compiling rewriter to obtain this variable again.
// Note that the static variable variable_indices is not cleared
// during several runs, as generally the variables bound in rewrite
// rules do not change.
size_t RewriterCompilingJitty::binding_variable_list_index(const variable_list& vl)
{
  if (variable_list_indices1.count(vl)>0)
  {
    return variable_list_indices1[vl];
  }
  const size_t index_for_vl=rewriter_binding_variable_lists.size();
  variable_list_indices1[vl]=index_for_vl;
  rewriter_binding_variable_lists.push_back(vl);
  return index_for_vl;
}

// Put the sorts with indices between actual arity and requested arity in a vector.
sort_list_vector RewriterCompilingJitty::get_residual_sorts(const sort_expression& s1, size_t actual_arity, size_t requested_arity)
{
  sort_expression s=s1;
  sort_list_vector result;
  while (requested_arity>0)
  {
    const function_sort fs(s);
    if (actual_arity==0)
    {
      result.push_back(fs.domain());
      assert(fs.domain().size()<=requested_arity);
      requested_arity=requested_arity-fs.domain().size();
    }
    else
    {
      assert(fs.domain().size()<=actual_arity);
      actual_arity=actual_arity-fs.domain().size();
      requested_arity=requested_arity-fs.domain().size();
    }
    s=fs.codomain();
  }
  return result;
}

/// Rewrite the equation e such that it has exactly a arguments.
/// If the rewrite rule has too many arguments, false is returned, otherwise
/// additional arguments are added.

bool RewriterCompilingJitty::lift_rewrite_rule_to_right_arity(data_equation& e, const size_t requested_arity)
{
  data_expression lhs=e.lhs();
  data_expression rhs=e.rhs();
  variable_list vars=e.variables();

  function_symbol f;
  if (!head_is_function_symbol(lhs,f))
  {
    throw mcrl2::runtime_error("Equation " + pp(e) + " does not start with a function symbol in its left hand side.");
  }

  size_t actual_arity=recursive_number_of_args(lhs);
  if (arity_is_allowed(f.sort(),requested_arity) && actual_arity<=requested_arity)
  {
    if (actual_arity<requested_arity)
    {
      // Supplement the lhs and rhs with requested_arity-actual_arity extra variables.
      sort_list_vector requested_sorts=get_residual_sorts(f.sort(),actual_arity,requested_arity);
      for(sort_list_vector::const_iterator sl=requested_sorts.begin(); sl!=requested_sorts.end(); ++sl)
      {
        variable_vector var_vec;
        for(sort_expression_list::const_iterator s=sl->begin(); s!=sl->end(); ++s)
        {
          variable v=variable(jitty_rewriter.generator("v@r"),*s); // Find a new name for a variable that is temporarily in use.
          var_vec.push_back(v);
          vars.push_front(v);
        }
        lhs=application(lhs,var_vec.begin(),var_vec.end());
        rhs=application(rhs,var_vec.begin(),var_vec.end());
      }
    }
  }
  else
  {
    return false; // This is not an allowed arity, or the actual number of arguments is larger than the requested number.
  }

  e=data_equation(vars,e.condition(),lhs,rhs);
  return true;
}

/// Adapt the equation in eqns such that they have exactly arity arguments.
data_equation_list RewriterCompilingJitty::lift_rewrite_rules_to_right_arity(const data_equation_list& eqns,const size_t arity)
{
  data_equation_vector result;
  for(data_equation_list::const_iterator i=eqns.begin(); i!=eqns.end(); ++i)
  {
    data_equation e=*i;
    if (lift_rewrite_rule_to_right_arity(e,arity))
    {
      result.push_back(e);
    }
  }
  return data_equation_list(result.begin(),result.end());
}

match_tree_list RewriterCompilingJitty::create_strategy(
        const data_equation_list& rules,
        const size_t arity,
        nfs_array& nfs)
{
  match_tree_list strat;
  // Array to keep note of the used parameters
  std::vector <bool> used;
  for (size_t i = 0; i < arity; i++)
  {
    used.push_back(nfs.get(i));
  }

  // Maintain dependency count (i.e. the number of rules that depend on a given argument)
  std::vector<int> args(arity,-1);
  // Process all (applicable) rules
  std::vector<bool> bs(arity);
  atermpp::aterm_list dep_list;
  for (data_equation_list::const_iterator it=rules.begin(); it!=rules.end(); ++it)
  {
    size_t rule_arity = recursive_number_of_args(it->lhs());
    if (rule_arity > arity)
    {
      continue;
    }

    const data_expression& lhs_internal = it->lhs();
    variable_list_list vars = make_list<variable_list>( get_doubles(it->rhs())+ get_vars(it->condition())
                                 ); // List of variables occurring in each argument of the lhs
                                    // (except the first element which contains variables from the
                                    // condition and variables which occur more than once in the result)

    // Indices of arguments that need to be rewritten
    for (size_t i = 0; i < rule_arity; i++)
    {
      bs[i] = false;
    }

    // Check all arguments
    for (size_t i = 0; i < rule_arity; i++)
    {
      if (!is_variable(get_argument_of_higher_order_term(lhs_internal,i)))
      {
        // Argument is not a variable, so it needs to be rewritten
        bs[i] = true;
        variable_list evars = get_vars(get_argument_of_higher_order_term(lhs_internal,i));
        for (; !evars.empty(); evars=evars.tail())
        {
          int j=i-1;
          for (variable_list_list o=vars; !o.tail().empty(); o=o.tail())
          {
            const variable_list l=o.front();
            if (std::find(l.begin(),l.end(),evars.front()) != l.end())
            {
              bs[j] = true;
            }
            --j;
          }
        }
      }
      else
      {
        // Argument is a variable; check whether it occurred before
        int j = i-1; // vars.size()-1-1
        bool b = false;
        for (variable_list_list o=vars; !o.empty(); o=o.tail())
        {
          const variable_list l=o.front();
          if (std::find(l.begin(),l.end(),get_argument_of_higher_order_term(lhs_internal,i)) != l.end())
          {
            // Same variable, mark it
            if (j >= 0)
            {
              bs[j] = true;
            }
            b = true;
          }
          --j;
        }
        if (b)
        {
          // Found same variable(s), so mark this one as well
          bs[i] = true;
        }
      }
      // Add vars used in expression
      vars.push_front(get_vars(get_argument_of_higher_order_term(lhs_internal,i)));
    }

    // Create dependency list for this rule
    atermpp::aterm_list deps;
    for (size_t i = 0; i < rule_arity; i++)
    {
      // Only if needed and not already rewritten
      if (bs[i] && !used[i])
      {
        deps.push_front(atermpp::aterm_int(i));
        // Increase dependency count
        args[i] += 1;
        //fprintf(stderr,"dep of arg %i\n",i);
      }
    }
    deps = reverse(deps);

    // Add rule with its dependencies
    dep_list.push_front(make_list<aterm>( deps, (atermpp::aterm_appl)*it));
  }

  // Process all rules with their dependencies
  while (1)
  {
    // First collect rules without dependencies to the strategy
    data_equation_list no_deps;
    atermpp::aterm_list has_deps;
    for (; !dep_list.empty(); dep_list=dep_list.tail())
    {
      if (down_cast<atermpp::aterm_list>(down_cast<atermpp::aterm_list>(dep_list.front()).front()).empty())
      {
        no_deps.push_front(data_equation(down_cast<atermpp::aterm_list>(dep_list.front()).tail().front()));
      }
      else
      {
        has_deps.push_front(dep_list.front());
      }
    }
    dep_list = reverse(has_deps);

    // Create and add tree of collected rules
    if (!no_deps.empty())
    {
      strat.push_front(create_tree(lift_rewrite_rules_to_right_arity(no_deps,arity)));
    }

    // Stop if there are no more rules left
    if (dep_list.empty())
    {
      break;
    }

    // Otherwise, figure out which argument is most useful to rewrite
    int max = -1;
    int maxidx = -1;
    for (size_t i = 0; i < arity; i++)
    {
      if (args[i] > max)
      {
        maxidx = i;
        max = args[i];
      }
    }

    // If there is a maximum (which should always be the case), add it to the strategy and remove it from the dependency lists
    assert(maxidx >= 0);
    if (maxidx >= 0)
    {
      args[maxidx] = -1;
      used[maxidx] = true;
      atermpp::aterm_int rewr_arg = atermpp::aterm_int(maxidx);

      strat.push_front(match_tree_A(maxidx));

      atermpp::aterm_list l;
      for (; !dep_list.empty(); dep_list=dep_list.tail())
      {
        atermpp::aterm_list temp= atermpp::down_cast<atermpp::aterm_list>(dep_list.front()).tail();
        temp.push_front(remove_one_element<aterm>(down_cast<atermpp::aterm_list>(down_cast<atermpp::aterm_list>(dep_list.front()).front()), rewr_arg));
        l.push_front(temp);
      }
      dep_list = reverse(l);
    }
  }
  return reverse(strat);
}

void RewriterCompilingJitty::add_base_nfs(nfs_array& nfs, const function_symbol& opid, size_t arity)
{
  for (size_t i=0; i<arity; i++)
  {
    if (always_rewrite_argument(opid,arity,i))
    {
      nfs.set(i);
    }
  }
}

void RewriterCompilingJitty::extend_nfs(nfs_array& nfs, const function_symbol& opid, size_t arity)
{
  data_equation_list eqns = jittyc_eqns[opid];
  if (eqns.empty())
  {
    nfs.fill(arity);
    return;
  }
  match_tree_list strat = create_strategy(eqns,arity,nfs);
  while (!strat.empty() && strat.front().isA())
  {
    nfs.set(match_tree_A(strat.front()).variable_index());
    strat = strat.tail();
  }
}

// Determine whether the opid is a normal form, with the given number of arguments.
bool RewriterCompilingJitty::opid_is_nf(const function_symbol& opid, size_t num_args)
{
  // Check whether there are applicable rewrite rules.
  data_equation_list l = jittyc_eqns[opid];

  if (l.empty())
  {
    return true;
  }

  for (data_equation_list::const_iterator it=l.begin(); it!=l.end(); ++it)
  {
    if (recursive_number_of_args(it->lhs()) <= num_args)
    {
      return false;
    }
  }

  return true;
}

void RewriterCompilingJitty::calc_nfs_list(
                nfs_array& nfs,
                const application& appl,
                variable_or_number_list nnfvars)
{
  for(size_t i=0; i<recursive_number_of_args(appl); ++i)
  {
    nfs.set(i,calc_nfs(get_argument_of_higher_order_term(appl,i),nnfvars));
  }
}

/// This function returns true if it knows for sure that the data_expresson t is in normal form.
bool RewriterCompilingJitty::calc_nfs(const data_expression& t, variable_or_number_list nnfvars)
{
  if (is_function_symbol(t))
  {
    return opid_is_nf(down_cast<function_symbol>(t),0);
  }
  else if (is_variable(t))
  {
    return (std::find(nnfvars.begin(),nnfvars.end(),variable(t)) == nnfvars.end());
  }
  else if (is_abstraction(t))
  {
    // It the term has the shape lambda x:D.t and t is a normal form, then the whole
    // term is a normal form. An expression with an exists/forall is never a normal form.
    const abstraction& ta(t);
    if (is_lambda_binder(ta.binding_operator()))
    {
      return calc_nfs(ta.body(),nnfvars);
    }
    return false;
  }
  else if (is_where_clause(t))
  {
    return false; // I assume that a where clause is not in normal form by default.
                  // This might be too weak, and may require to be reinvestigated later.
  }

  // t has the shape application(head,t1,...,tn)
  const application ta(t);
  const size_t arity = recursive_number_of_args(ta);
  const data_expression& head=ta.head();
  function_symbol dummy;
  if (head_is_function_symbol(head,dummy))
  {
    assert(arity!=0);
    if (opid_is_nf(down_cast<function_symbol>(head),arity))
    {
      nfs_array args(arity);
      calc_nfs_list(args,ta,nnfvars);
      bool b = args.is_filled();
      return b;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

string RewriterCompilingJitty::calc_inner_terms(
              nfs_array& nfs,
              const application& appl,
              const size_t startarg,
              variable_or_number_list nnfvars,
              const nfs_array& rewr)
{
  size_t j=0;
  string result="";
  for(application::const_iterator i=appl.begin(); i!=appl.end(); ++i, ++j)
  {
    pair<bool,string> head = calc_inner_term(*i, startarg+j,nnfvars,rewr.get(j));
    nfs.set(j,head.first);

    result=result + (j==0?"":",") + head.second;
  }
  return result;
}

// arity is one if there is a single head. Arity is two is there is a head and one argument, etc.
static string calc_inner_appl_head(size_t arity)
{
  stringstream ss;
  if (arity == 1)
  {
    ss << "pass_on(";  // This is to avoid confusion with atermpp::aterm_appl on a function symbol and two iterators.
  }
  else if (arity <= 5)
  {
    ss << "application(";
  }
  else
  {
    ss << "make_term_with_many_arguments(";
  }
  return ss.str();
}

/// This function generates a string of C++ code to calculate the data_expression t.
/// If the result is a normal form the resulting boolean is true, otherwise it is false.
/// The data expression t is the term for which C code is generated.
/// The size_t start_arg gives the index of the position of the current term in the surrounding application.
//                 The head has index 0. The first argument has index 1, etc.
/// The variable_or_number_list nnfvars contains variables that are in normal form, and indices that are not in normal form.
/// The bool rewr indicates whether a normal form is requested. If rewr is valid, then yes.

pair<bool,string> RewriterCompilingJitty::calc_inner_term(
                             const data_expression& t,
                             const size_t startarg,
                             variable_or_number_list nnfvars,
                             const bool rewr)
{
  stringstream ss;
  // Experiment: if the term has no free variables, deliver the normal form directly.
  // This code can be removed, when it does not turn out to be useful.
  // This requires the use of the jitty rewriter to calculate normal forms.

  if (find_free_variables(t).empty())
  {
    return pair<bool,string>(true, m_nf_cache.insert(t));
  }

  if (is_function_symbol(t))
  {
    const function_symbol& f = atermpp::down_cast<function_symbol>(t);
    bool b = opid_is_nf(f,0);

    if (rewr && !b)
    {
      ss << "rewr_" << core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(f) << "_0_0()";
    }
    else
    {
      ss << "atermpp::down_cast<const data_expression>(aterm(reinterpret_cast<const atermpp::detail::_aterm*>(" << atermpp::detail::address(t) << ")))";
    }
    return pair<bool,string>(rewr || b, ss.str());

  }
  else if (is_variable(t))
  {
    const variable v(t);
    const bool b = (std::find(nnfvars.begin(),nnfvars.end(),v) != nnfvars.end());
    const string variable_name=v.name();
    // Remove the initial @ if it is present in the variable name, because then it is an variable introduced
    // by this rewriter.
    if (variable_name[0]=='@')
    {
      if (rewr && b)
      {
        ss << "rewrite(" << variable_name.substr(1) << ")";
      }
      else
      {
        ss << variable_name.substr(1);
      }
    }
    else
    {
      ss << "this_rewriter->bound_variable_get(" << bound_variable_index(v) << ")";
    }
    return pair<bool,string>(rewr || !b, ss.str());
  }
  else if (is_abstraction(t))
  {
    const abstraction& ta(t);
    if (is_lambda_binder(ta.binding_operator()))
    {
      if (rewr)
      {
        // The resulting term must be rewritten.
        pair<bool,string> r=calc_inner_term(ta.body(),startarg,nnfvars,true);
        ss << "this_rewriter->rewrite_single_lambda(" <<
               "this_rewriter->binding_variable_list_get(" << binding_variable_list_index(ta.variables()) << ")," <<
               r.second << "," << r.first << ",*(this_rewriter->global_sigma))";
        return pair<bool,string>(true,ss.str());
      }
      else
      {
        pair<bool,string> r=calc_inner_term(ta.body(),startarg,nnfvars,false);
        ss << "abstraction(lambda_binder()," <<
               "this_rewriter->binding_variable_list_get(" << binding_variable_list_index(ta.variables()) << ")," <<
               r.second << ")";
        return pair<bool,string>(false,ss.str());
      }
    }
    else if (is_forall_binder(ta.binding_operator()))
    {
      if (rewr)
      {
        // A result in normal form is requested.
        pair<bool,string> r=calc_inner_term(ta.body(),startarg,nnfvars,false);
        ss << "this_rewriter->universal_quantifier_enumeration(" <<
               "this_rewriter->binding_variable_list_get(" << binding_variable_list_index(ta.variables()) << ")," <<
               r.second << "," << r.first << "," << "*(this_rewriter->global_sigma))";
        return pair<bool,string>(true,ss.str());
      }
      else
      {
        // A result which is not a normal form is requested.
        pair<bool,string> r=calc_inner_term(ta.body(),startarg,nnfvars,false);
        ss << "abstraction(forall_binder()," <<
               "this_rewriter->binding_variable_list_get(" << binding_variable_list_index(ta.variables()) << ")," <<
               r.second << ")";
        return pair<bool,string>(false,ss.str());
      }
    }
    else if (is_exists_binder(ta.binding_operator()))
    {
      if (rewr)
      {
        // A result in normal form is requested.
        pair<bool,string> r=calc_inner_term(ta.body(),startarg,nnfvars,false);
        ss << "this_rewriter->existential_quantifier_enumeration(" <<
               "this_rewriter->binding_variable_list_get(" << binding_variable_list_index(ta.variables()) << ")," <<
               r.second << "," << r.first << "," << "*(this_rewriter->global_sigma))";
        return pair<bool,string>(true,ss.str());
      }
      else
      {
        // A result which is not a normal form is requested.
        pair<bool,string> r=calc_inner_term(ta.body(),startarg,nnfvars,false);
        ss << "abstraction(exists_binder()," <<
               "this_rewriter->binding_variable_list_get(" << binding_variable_list_index(ta.variables()) << ")," <<
               r.second << ")";
        return pair<bool,string>(false,ss.str());
      }
    }
  }
  else if (is_where_clause(t))
  {
    const where_clause& w = atermpp::down_cast<where_clause>(t);

    if (rewr)
    {
      // A rewritten result is expected.
      pair<bool,string> r=calc_inner_term(w.body(),startarg,nnfvars,true);

      ss << "this_rewriter->rewrite_where(mcrl2::data::where_clause(" << r.second << ",";

      const assignment_list& assignments(w.assignments());
      for( size_t no_opening_brackets=assignments.size(); no_opening_brackets>0 ; no_opening_brackets--)
      {
        ss << "jittyc_local_push_front(";
      }
      ss << "mcrl2::data::assignment_expression_list()";
      for(assignment_list::const_iterator i=assignments.begin() ; i!=assignments.end(); ++i)
      {
        pair<bool,string> r=calc_inner_term(i->rhs(),startarg,nnfvars,true);
        ss << ",mcrl2::data::assignment(" <<
                 "this_rewriter->bound_variable_get(" << bound_variable_index(i->lhs()) << ")," <<
                 r.second << "))";
      }

      ss << "),*(this_rewriter->global_sigma))";

      return pair<bool,string>(true,ss.str());
    }
    else
    {
      // The result does not need to be rewritten.
      pair<bool,string> r=calc_inner_term(w.body(),startarg,nnfvars,false);
      ss << "mcrl2::data::where_clause(" << r.second << ",";

      const assignment_list& assignments(w.assignments());
      for( size_t no_opening_brackets=assignments.size(); no_opening_brackets>0 ; no_opening_brackets--)
      {
        ss << "jittyc_local_push_front(";
      }
      ss << "mcrl2::data::assignment_expression_list()";
      for(assignment_list::const_iterator i=assignments.begin() ; i!=assignments.end(); ++i)
      {
        pair<bool,string> r=calc_inner_term(i->rhs(),startarg,nnfvars,true);
        ss << ",mcrl2::data::assignment(" <<
                 "this_rewriter->bound_variable_get(" << bound_variable_index(i->lhs()) << ")," <<
                 r.second << "))";
      }

      ss << ")";

      return pair<bool,string>(false,ss.str());
    }
  }

  // t has the shape application(head,t1,...,tn)
  const application& ta = atermpp::down_cast<application>(t);
  bool b;
  size_t arity = ta.size();

  if (is_function_symbol(ta.head()))  // Determine whether the topmost symbol is a function symbol.
  {
    const function_symbol& headfs = atermpp::down_cast<function_symbol>(ta.head());
    size_t cumulative_arity = ta.size();
    b = opid_is_nf(headfs,cumulative_arity+1); // b indicates that headfs is in normal form.

    if (b || !rewr)
    {
      ss << calc_inner_appl_head(arity+1);
    }

    if (arity == 0)
    {
      if (b || !rewr)
      {
        // TODO: This expression might be costly with two increase/decreases of reference counting.
        // Should probably be resolved by a table of function symbols.
        ss << "atermpp::down_cast<data_expression>(atermpp::aterm((const atermpp::detail::_aterm*) " << (void*) atermpp::detail::address(headfs) << "))";
      }
      else
      {
        ss << "rewr_" << core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(headfs) << "_0_0()";
      }
    }
    else
    {
      // arity != 0
      nfs_array args_nfs(arity);
      calc_nfs_list(args_nfs,ta,nnfvars);

      if (!(b || !rewr))
      {
        ss << "rewr_";
        add_base_nfs(args_nfs,headfs,arity);
        extend_nfs(args_nfs,headfs,arity);
      }
      if (arity > NF_MAX_ARITY)
      {
        args_nfs.fill(false);
      }
      if (args_nfs.is_clear() || b || rewr || (arity > NF_MAX_ARITY))
      {
        if (b || !rewr)
        {
          ss << "atermpp::down_cast<data_expression>(atermpp::aterm((const atermpp::detail::_aterm*) " << (void*) atermpp::detail::address(headfs) << "))";
        }
        else
        {
          ss << core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(headfs);
        }
      }
      else
      {
        if (b || !rewr)
        {
          if (data_equation_selector(headfs))
          {
            const size_t index=core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(headfs);
            const data::function_symbol old_head=headfs;
            std::stringstream new_name;
            new_name << "@_rewr" << "_" << index << "_@@@_" << (getArity(headfs)>NF_MAX_ARITY?0:args_nfs.get_encoded_number())
                                 << "_term";
            const data::function_symbol f(new_name.str(),old_head.sort());
            if (partially_rewritten_functions.count(f)==0)
            {
              partially_rewritten_functions.insert(f);
            }

            ss << "atermpp::down_cast<data_expression>(atermpp::aterm((const atermpp::detail::_aterm*) " << (void*) atermpp::detail::address(f) << "))";
          }
        }
        else
        {
          ss << (core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(headfs)+((1 << arity)-arity-1)+args_nfs.get_encoded_number());
        }
      }
      nfs_array args_first(arity);
      if (rewr && b)
      {
        args_nfs.fill(arity);
      }
      string args_second = calc_inner_terms(args_first,ta,startarg,nnfvars,args_nfs);

      assert(!rewr || b || (arity > NF_MAX_ARITY) || args_first>=args_nfs);
      if (rewr && !b)
      {
        ss << "_" << arity << "_";
        if (arity <= NF_MAX_ARITY)
        {
          ss << args_first.get_encoded_number();
        }
        else
        {
          ss << "0";
        }
        ss << "(";
      }
      else
      {
        ss << ",";
      }
      ss << args_second << ")";
      if (!args_first.is_filled())
      {
        b = false;
      }
    }
    b = b || rewr;

  }
  else
  {
    // ta.head() is not function symbol. So the first element of this list is
    // either an application, variable, or a lambda term. It cannot be a forall or exists, because in
    // that case there would be a type error.
    assert(arity > 0);

    if (is_abstraction(ta.head()))
    {
      const abstraction& ta1(ta.head());
      assert(is_lambda_binder(ta1.binding_operator()));

      b = rewr;
      nfs_array args_nfs(arity);
      calc_nfs_list(args_nfs,ta,nnfvars);
      if (arity > NF_MAX_ARITY)
      {
        args_nfs.fill(false);
      }

      nfs_array args_first(arity);
      if (rewr && b)
      {
        args_nfs.fill();
      }

      pair<bool,string> head = calc_inner_term(ta1,startarg,nnfvars,false);

      if (rewr)
      {
        ss << "rewrite(";
      }

      ss << calc_inner_appl_head(arity+1) << head.second << "," <<
               calc_inner_terms(args_first,ta,startarg,nnfvars,args_nfs) << ")";
      if (rewr)
      {
        ss << ")";
      }

    }
    else if (is_application(ta.head()))
    {
      const size_t arity=ta.size();
      const size_t total_arity=recursive_number_of_args(ta);
      b = rewr;
      pair<bool,string> head = calc_inner_term(ta.head(),startarg,nnfvars,false);  // XXXX TODO TAKE CARE THAT NORMAL FORMS ADMINISTRATION IS DEALT WITH PROPERLY FOR HIGHER ORDER TERMS.
      nfs_array tail_first(total_arity);
      const nfs_array dummy(total_arity);
      string tail_second = calc_inner_terms(tail_first,ta,startarg,nnfvars,dummy);
      if (rewr)
      {
          ss << "rewrite(";
      }
      ss << calc_inner_appl_head(arity+1) << head.second << "," << tail_second << ")";
      if (rewr)
      {
        ss << ")";
      }
    }
    else // headfs is a single variable.
    {
      // So, the first element of t must be a single variable.
      assert(is_variable(ta.head()));
      const size_t arity=ta.size();
      b = rewr;
      pair<bool,string> head = calc_inner_term(ta.head(),startarg,nnfvars,false);
      nfs_array tail_first(arity);
      const nfs_array dummy(arity);
      string tail_second = calc_inner_terms(tail_first,ta,startarg,nnfvars,dummy);
      ss << "!is_variable(" << head.second << ")?";
      if (rewr)
      {
          ss << "rewrite(";
      }
      ss << calc_inner_appl_head(arity+1) << head.second << "," << tail_second << ")";
      if (rewr)
      {
        ss << ")";
      }
      ss << ":";
      bool c = rewr;
      if (rewr && std::find(nnfvars.begin(),nnfvars.end(), atermpp::aterm_int(startarg)) != nnfvars.end())
      {
        ss << "rewrite(";
        c = false;
      }
      else
      {
        ss << "pass_on(";
      }
      ss << calc_inner_appl_head(arity+1) << head.second << ",";
      if (c)
      {
        tail_first.fill(false);
        nfs_array rewrall(arity);
        rewrall.fill();
        tail_second = calc_inner_terms(tail_first,ta,startarg,nnfvars,rewrall);
      }
      ss << tail_second << ")";
      ss << ")";
    }
  }

  return pair<bool,string>(b,ss.str());
}

class RewriterCompilingJitty::ImplementTree
{
private:
  class padding
  {
  private:
    size_t m_indent;
  public:
    padding(size_t indent) : m_indent(indent) { }
    void indent() { m_indent += 2; }
    void unindent() { m_indent -= 2; }
    
    friend
    std::ostream& operator<<(std::ostream& stream, const padding& p)
    {
      for (size_t i = p.m_indent; i != 0; --i)
      {
        stream << ' ';
      }
      return stream;
    }
  };
  
  RewriterCompilingJitty& m_rewriter;
  std::ostream& m_stream;
  const std::vector<bool>& m_used;
  std::vector<int> m_stack;
  padding m_padding;
  variable_or_number_list m_nnfvars;
  
  // Print code representing tree to f.
  //
  // cur_arg   Indices refering to the variable that contains the current
  // parent    term. For level 0 this means arg<cur_arg>, for level 1 it
  //           means arg<parent>(<cur_arg) and for higher
  //           levels it means t<parent>(<cur_arg>)
  //
  // parent    Index of cur_arg in the previous level
  //
  // level     Indicates the how deep we are in the term (e.g. in
  //           f(.g(x),y) . indicates level 0 and in f(g(.x),y) level 1
  //
  // cnt       Counter indicating the number of variables t<i> (0<=i<cnt)
  //           used so far (in the current scope)
  void implement_tree_aux(const match_tree& tree, size_t cur_arg, size_t parent, size_t level, size_t cnt)
  {
    if (tree.isS())
    {
      implement_tree_aux(atermpp::down_cast<match_tree_S>(tree), cur_arg, parent, level, cnt);
    }
    else if (tree.isM())
    {
      implement_tree_aux(atermpp::down_cast<match_tree_M>(tree), cur_arg, parent, level, cnt);
    }
    else if (tree.isF())
    {
      implement_tree_aux(atermpp::down_cast<match_tree_F>(tree), cur_arg, parent, level, cnt);
    }
    else if (tree.isD())
    {
      implement_tree_aux(atermpp::down_cast<match_tree_D>(tree), level, cnt);
    }
    else if (tree.isN())
    {
      implement_tree_aux(atermpp::down_cast<match_tree_N>(tree), cur_arg, parent, level, cnt);
    }
    else if (tree.isC())
    {
      implement_tree_aux(atermpp::down_cast<match_tree_C>(tree), cur_arg, parent, level, cnt);
    }
    else if (tree.isR())
    {
      implement_tree_aux(atermpp::down_cast<match_tree_R>(tree), cur_arg, level);
    }
    else 
    {
      // These are the only remaining case, where we do not have to do anything.
      assert(tree.isA() || tree.isX() || tree.isMe()); 
    }    
  }
  
  void implement_tree_aux(const match_tree_S& tree, size_t cur_arg, size_t parent, size_t level, size_t cnt)
  {
    const match_tree_S& treeS(tree);
    m_stream << m_padding << "const data_expression& " << string(treeS.target_variable().name()).c_str() + 1 << " = ";
    if (level == 0)
    {
      if (m_used[cur_arg])
      {
        m_stream << "arg" << cur_arg << "; // S1\n";
      }
      else
      {
        m_stream << "arg_not_nf" << cur_arg << "; // S1\n";
        m_nnfvars.push_front(treeS.target_variable());
      }
    }
    else
    {
      m_stream << "atermpp::down_cast<data_expression>(" 
               << (level == 1 ? "arg" : "t") << parent << "[" << cur_arg << "]"
               << "); // S2\n";
    }
    implement_tree_aux(tree.subtree(), cur_arg, parent, level, cnt);
  }
  
  void implement_tree_aux(const match_tree_M& tree, size_t cur_arg, size_t parent, size_t level, size_t cnt)
  {
    m_stream << m_padding << "if (" << string(tree.match_variable().name()).c_str() + 1 << " == ";
    if (level == 0)
    {
      m_stream << "arg" << cur_arg;
    }
    else
    {
      m_stream << (level == 1 ? "arg" : "t") << parent << "[" << cur_arg << "]";
    }
    m_stream << ") // M\n" << m_padding 
             << "{\n";
    m_padding.indent();
    implement_tree_aux(tree.true_tree(), cur_arg, parent, level, cnt);
    m_padding.unindent();
    m_stream << m_padding
             << "}\n" << m_padding 
             << "else\n" << m_padding
             << "{\n";
    m_padding.indent();
    implement_tree_aux(tree.false_tree(), cur_arg, parent, level, cnt);
    m_padding.unindent();
    m_stream << m_padding
             << "}\n";
  }
  
  void implement_tree_aux(const match_tree_F& tree, size_t cur_arg, size_t parent, size_t level, size_t cnt)
  {
    const void* func = (void*)(atermpp::detail::address(tree.function()));
    const char* s_arg = "arg";
    const char* s_t = "t";
    m_stream << m_padding;
    if (level == 0)
    {
      if (!is_function_sort(tree.function().sort()))
      {
        m_stream << "if (atermpp::detail::address(arg" << cur_arg << ") == "
                 <<     "reinterpret_cast<const atermpp::detail::_aterm*>(" << func << ")) // F1\n" << m_padding
                 << "{\n";
      }
      else
      {
        m_stream << "if (atermpp::detail::address(\n" << m_padding
                 << "     (mcrl2::data::is_function_symbol(arg" << cur_arg <<  ") ? arg" << cur_arg << " : arg" << cur_arg << "[0])) == "
                 <<     "reinterpret_cast<const atermpp::detail::_aterm*>(" << func << ")) // F1\n" << m_padding
                 << "{\n";
      }
    }
    else
    {
      const char* array = level == 1 ? s_arg : s_t;
      if (!is_function_sort(tree.function().sort()))
      {
        m_stream << "if (atermpp::detail::address(down_cast<data_expression>(" << array << parent << "[" << cur_arg << "])) == "
                 <<     "reinterpret_cast<const atermpp::detail::_aterm*>(" << func << ")) // F2a " << tree.function().name() << "\n" << m_padding
                 << "{\n" << m_padding
                 << "  const data_expression& t" << cnt << " = down_cast<data_expression>(" << array << parent << "[" << cur_arg << "]);\n";
      }
      else
      { 
        m_stream << "if (is_application_no_check(down_cast<atermpp::aterm_appl>(" << array << parent << "[" << cur_arg << "])) && "
                 <<     "atermpp::detail::address(down_cast<data_expression>(" << array << parent << "[" << cur_arg << "])[0]) == "
                 <<     "reinterpret_cast<const atermpp::detail::_aterm*>(" << func << ")) // F2b " << tree.function().name() << "\n" << m_padding
                 << "{\n" << m_padding
                 << "  const data_expression& t" << cnt << " = down_cast<data_expression>(" << array << parent << "[" << cur_arg << "]);\n";
      }
    }
    m_stack.push_back(cur_arg);
    m_stack.push_back(parent);
    m_padding.indent();
    implement_tree_aux(tree.true_tree(), 1, level == 0 ? cur_arg : cnt, level + 1, cnt + 1);
    m_padding.unindent();
    m_stack.pop_back();
    m_stack.pop_back();
    m_stream << m_padding
             << "}\n" << m_padding
             << "else\n" << m_padding
             << "{\n";
    m_padding.indent();
    implement_tree_aux(tree.false_tree(), cur_arg, parent, level, cnt);
    m_padding.unindent();
    m_stream << m_padding
             << "}\n";
  }
  
  void implement_tree_aux(const match_tree_D& tree, size_t level, size_t cnt)
  {
    int i = m_stack.back();
    m_stack.pop_back();
    int j = m_stack.back();
    m_stack.pop_back();
    implement_tree_aux(tree.subtree(), j, i, level - 1, cnt);
    m_stack.push_back(j);
    m_stack.push_back(i);
  }

  void implement_tree_aux(const match_tree_N& tree, size_t cur_arg, size_t parent, size_t level, size_t cnt)
  {
    implement_tree_aux(tree.subtree(), cur_arg + 1, parent, level, cnt);
  }

  void implement_tree_aux(const match_tree_C& tree, size_t cur_arg, size_t parent, size_t level, size_t cnt)
  {
    pair<bool, string> p = m_rewriter.calc_inner_term(tree.condition(), 0, m_nnfvars);
    m_stream << m_padding
             << "if (" << p.second << " == data_expression((const atermpp::detail::_aterm*)" 
             <<      (void*)atermpp::detail::address(sort_bool::true_()) << ")) // C\n" << m_padding
             << "{\n";
    
    m_padding.indent();
    implement_tree_aux(tree.true_tree(), cur_arg, parent, level, cnt);
    m_padding.unindent();
    
    m_stream << m_padding
             << "}\n" << m_padding
             << "else\n" << m_padding
             << "{\n";
          
    m_padding.indent();
    implement_tree_aux(tree.false_tree(), cur_arg, parent, level, cnt);
    m_padding.unindent();
    
    m_stream << m_padding
             << "}\n";
  }
  
  void implement_tree_aux(const match_tree_R& tree, size_t cur_arg, size_t level)
  {
    if (level > 0)
    {
      cur_arg = m_stack[2 * level - 1];
    }
    pair<bool, string> p = m_rewriter.calc_inner_term(tree.result(), cur_arg + 1, m_nnfvars);
    m_stream << m_padding << "return " << p.second << "; // R1\n";
  }

  const match_tree& implement_tree(const match_tree_C& tree)
  {
    assert(tree.true_tree().isR());
    pair<bool, string> p = m_rewriter.calc_inner_term(tree.condition(), 0, variable_or_number_list());
    pair<bool, string> r = m_rewriter.calc_inner_term(match_tree_R(tree.true_tree()).result(), 0, m_nnfvars);
    m_stream << m_padding
             << "if (" << p.second << " == atermpp::aterm_appl((const atermpp::detail::_aterm*)" << (void*)atermpp::detail::address(sort_bool::true_()) << ")) // C\n" << m_padding
             << "{\n" << m_padding
             << "  return " << r.second << ";\n" << m_padding
             << "}\n" << m_padding
             << "else\n" << m_padding
             << "{\n" << m_padding;
    m_padding.indent();
    return tree.false_tree();
  }
  
  void implement_tree(const match_tree_R& tree, size_t arity)
  {
    pair<bool, string> p = m_rewriter.calc_inner_term(tree.result(), 0, m_nnfvars);
    if (arity == 0)
    { 
      // return a reference to an atermpp::aterm_appl
      m_stream << m_padding
               << "static data_expression static_term(rewrite(" << p.second << "));\n" << m_padding
               << "return static_term; // R2a\n";
    }
    else
    { 
      // arity>0
      m_stream << m_padding
               << "return " << p.second << "; // R2b\n";
    }
  }
  
public:
  ImplementTree(RewriterCompilingJitty& rewr, std::ostream& stream, const std::vector<bool>& used, size_t padding)
    : m_rewriter(rewr), m_stream(stream), m_used(used), m_padding(padding)
  { }
  
  void implement_tree(match_tree tree, const size_t arity)
  {
    size_t l = 0;
    
    for (size_t i = 0; i < arity; ++i)
    {
      if (!m_used[i])
      {
        m_nnfvars.push_front(atermpp::aterm_int(i));
      }
    }
    
    while (tree.isC())
    {
      tree = implement_tree(down_cast<match_tree_C>(tree));
      l++;
    }
    
    if (tree.isR())
    {
      implement_tree(down_cast<match_tree_R>(tree), arity);
    }
    else
    {
      implement_tree_aux(tree, 0, 0, 0, 0);
    }
    
    // Close braces opened by implement_tree(const match_tree_C&)
    while (0 < l--)
    {
      m_padding.unindent();
      m_stream << m_padding << "}\n";
    }
  }
};

void RewriterCompilingJitty::implement_tree(
            FILE* f,
            const match_tree& tree,
            const size_t arity,
            size_t d,
            const std::vector<bool>& used)
{
  std::stringstream s;
  ImplementTree(*this, s, used, 2 * d).implement_tree(tree, arity);
  fputs(s.str().c_str(), f);
}

static std::string finish_function_return_term(const size_t arity,
                                               const std::string& head,
                                               const sort_expression& s,
                                               const std::vector<bool>& used,
                                               size_t& used_arguments)
{
  stringstream ss;
  if (!is_function_sort(s) || arity==0)
  {
    return head;
  }

  // There is no nested argument. The head is a single function symbol.
  assert(arity>0);
  const sort_expression_list arg_sorts=function_sort(s).domain();
  const sort_expression& target_sort=function_sort(s).codomain();

  if (arg_sorts.size() > 5)
  {
    ss << "make_term_with_many_arguments(" << head;
  }
  else
  {
    ss << "application(" << head;
  }

  for (size_t i=0; i<arg_sorts.size(); i++)
  {
    if (used[i+used_arguments])
    {
      ss << ", arg" << i+used_arguments;
    }
    else
    {
      ss << ", rewrite(arg_not_nf" << i+used_arguments << ")";
    }
  }
  ss << ")";
  used_arguments=used_arguments+arg_sorts.size();

  return finish_function_return_term(arity-arg_sorts.size(),ss.str(),target_sort,used,used_arguments);
}

void RewriterCompilingJitty::finish_function(FILE* f,
                                             size_t arity,
                                             const data::function_symbol& opid,
                                             const std::vector<bool>& used)
{
  // Note that arity is the total arity, of all function symbols.
  if (arity == 0)
  {
    fprintf(f, "return %s;\n", m_nf_cache.insert(opid).c_str());
  }
  else
  {
    size_t used_arguments=0;
    stringstream ss;
    ss << "data_expression((const atermpp::detail::_aterm*)"
       << (void*)atermpp::detail::address(opid)
       << ")";

    fprintf(f,"return %s;\n",finish_function_return_term(arity,ss.str(),function_sort(opid.sort()),used,used_arguments).c_str());
    assert(used_arguments==arity);
  }
}

void RewriterCompilingJitty::implement_strategy(
               FILE* f,
               match_tree_list strat,
               size_t arity,
               size_t d,
               const function_symbol& opid,
               const nfs_array& nf_args)
{
  std::vector<bool> used=nf_args; // This vector maintains which arguments are in normal form. Initially only those in nf_args are in normal form.
  stringstream ss;
  ss << whitespace(2*d) << "//" << strat << "\n";
  fprintf(f,"%s",ss.str().c_str());
  while (!strat.empty())
  {
    if (strat.front().isA())
    {
      size_t arg = match_tree_A(strat.front()).variable_index();

      if (!used[arg])
      {
        fprintf(f,"%sconst data_expression arg%lu = rewrite(arg_not_nf%lu);\n",whitespace(2*d),arg,arg);

        used[arg] = true;
      }
      fprintf(f,"%s// Considering argument  %ld\n",whitespace(2*d),arg);
    }
    else
    {
      fprintf(f,"%s{\n",whitespace(2*d));
      implement_tree(f,strat.front(),arity,d+1,used);
      fprintf(f,"%s}\n",whitespace(2*d));
    }

    strat = strat.tail();
  }

  finish_function(f,arity,opid,used);
}



// The function build_ar_internal returns an and/or tree indicating on which function symbol with which
// arity the variable var depends. The idea is that if var must be a normal form if any of these arguments
// must be a normal form. In this way the reduction to normal forms of these argument is not unnecessarily
// postponed. For example in the expression if(!b,1,0) the typical result is @@and(@@var(149),@@var(720))
// indicating that b must not be rewritten to a normal form anyhow if the first argument of ! must not always be rewritten
// to a normalform and the first argument of if must not always be rewritten to normal form.

atermpp::aterm_appl RewriterCompilingJitty::build_ar_expr_internal(const data_expression& expr, const variable& var)
{
  if (is_function_symbol(expr))
  {
    return make_ar_false();
  }

  if (is_variable(expr))
  {
    if (expr==var)
    {
      return make_ar_true();
    }
    else
    {
      return make_ar_false();
    }
  }

  if (is_where_clause(expr) || is_abstraction(expr))
  {
    return make_ar_false();
  }

  // expr has shape application(t,t1,...,tn);
  const application& expra = atermpp::down_cast<application>(expr);
  function_symbol head;
  if (!head_is_function_symbol(expra,head))
  {
    if (head_is_variable(expra))
    {
      if (get_variable_of_head(expra)==var)
      {
        return make_ar_true();
      }
    }
    return make_ar_false();
  }

  atermpp::aterm_appl result = make_ar_false();

  size_t arity = recursive_number_of_args(expra);
  for (size_t i=0; i<arity; i++)
  {
    const size_t idx = ar_index(head,arity,i);
    atermpp::aterm_appl t = build_ar_expr_internal(get_argument_of_higher_order_term(expra,i),var);
    result = make_ar_or(result,make_ar_and(make_ar_var(idx),t));
  }
  return result;
}

atermpp::aterm_appl RewriterCompilingJitty::build_ar_expr_aux(const data_equation& eqn, const size_t arg, const size_t arity)
{
  const function_symbol head=get_function_symbol_of_head(eqn.lhs());
  size_t eqn_arity = recursive_number_of_args(eqn.lhs());
  if (eqn_arity > arity)
  {
    return make_ar_true();
  }
  if (eqn_arity <= arg)
  {
    const data_expression& rhs = eqn.rhs();
    function_symbol head;
    if (is_function_symbol(rhs))
    {
      const size_t idx = ar_index(down_cast<function_symbol>(rhs),arity,arg);
      assert(idx<ar.size());
      return make_ar_var(idx);
    }
    else if (head_is_function_symbol(rhs,head))
    {
      int rhs_arity = recursive_number_of_args(rhs)-1;
      size_t diff_arity = arity-eqn_arity;
      int rhs_new_arity = rhs_arity+diff_arity;
      size_t idx = ar_index(head,rhs_new_arity,(arg - eqn_arity + rhs_arity));
      assert(idx<ar.size());
      return make_ar_var(idx);
    }
    else
    {
      return make_ar_false();
    }
  }

  // Here we know that eqn.lhs() must be an application. If it were a function symbol
  // it would have been dealt with above.
  const application& lhs = atermpp::down_cast<application>(eqn.lhs());
  const data_expression& arg_term = get_argument_of_higher_order_term(lhs,arg);
  if (!is_variable(arg_term))
  {
    return make_ar_true();
  }

  const variable v(arg_term);
  const variable_list l=dep_vars(eqn);
  if (std::find(l.begin(),l.end(), v) != l.end())
  {
    return make_ar_true();
  }

  return build_ar_expr_internal(eqn.rhs(),v);
}

atermpp::aterm_appl RewriterCompilingJitty::build_ar_expr(const data_equation_list& eqns, const size_t arg, const size_t arity)
{
  atermpp::aterm_appl result=make_ar_true();
  for(data_equation_list::const_iterator i=eqns.begin(); i!=eqns.end(); ++i)
  {
    result=make_ar_and(build_ar_expr_aux(*i,arg,arity),result);
  }
  return result;
}

bool RewriterCompilingJitty::always_rewrite_argument(
     const function_symbol& opid,
     const size_t arity,
     const size_t arg)
{
  return !is_ar_false(get_ar_array(opid,arity,arg));
}

bool RewriterCompilingJitty::calc_ar(const atermpp::aterm_appl& expr)
{
  if (is_ar_true(expr))
  {
    return true;
  }
  else if (is_ar_false(expr))
  {
    return false;
  }
  else if (is_ar_and(expr))
  {
    return calc_ar(down_cast<atermpp::aterm_appl>(expr[0])) && calc_ar(down_cast<atermpp::aterm_appl>(expr[1]));
  }
  else if (is_ar_or(expr))
  {
    return calc_ar(down_cast<atermpp::aterm_appl>(expr[0])) || calc_ar(down_cast<atermpp::aterm_appl>(expr[1]));
  }
  else     // is_ar_var(expr)
  {
    return !is_ar_false(ar[static_cast<atermpp::aterm_int>(expr[0]).value()]);
  }
}

void RewriterCompilingJitty::fill_always_rewrite_array()
{
  ar=std::vector <atermpp::aterm_appl> (ar_size);
  for(std::map <data::function_symbol,size_t> ::const_iterator it=int2ar_idx.begin(); it!=int2ar_idx.end(); ++it)
  {
    size_t arity = getArity(it->first);
    const data_equation_list& eqns = jittyc_eqns[it->first];
    for (size_t i=1; i<=arity; i++)
    {
      for (size_t j=0; j<i; j++)
      {
        set_ar_array(it->first,i,j,build_ar_expr(eqns,j,i));
      }
    }
  }

  bool notdone = true;
  while (notdone)
  {
    notdone = false;
    for (size_t i=0; i<ar_size; i++)
    {
      if (!is_ar_false(ar[i]) && !calc_ar(ar[i]))
      {
        ar[i] = make_ar_false();
        notdone = true;
      }
    }
  }
}



void RewriterCompilingJitty::CleanupRewriteSystem()
{
  m_nf_cache.clear();
  if (so_rewr_cleanup != NULL)
  {
    so_rewr_cleanup();
  }
  if (rewriter_so != NULL)
  {
    delete rewriter_so;
    rewriter_so = NULL;
  }
}

/* Opens a .cpp file, saves filenames to file_c, file_o and file_so.
 *
 */
FILE* RewriterCompilingJitty::MakeTempFiles()
{
	FILE* result;

	std::ostringstream file_base;
        char* file_dir = getenv("MCRL2_COMPILEDIR");
        if (file_dir != NULL)
        {
          size_t l = strlen(file_dir);
          if (file_dir[l - 1] == '/')
          {
            file_dir[l - 1] = 0;
          }
          file_base << file_dir;
        }
        else
        {
          file_base << ".";
        }
	file_base << "/jittyc_" << getpid() << "_" << reinterpret_cast< long >(this) << ".cpp";

	rewriter_source = file_base.str();

	result = fopen(const_cast< char* >(rewriter_source.c_str()),"w");
	if (result == NULL)
	{
		perror("fopen");
		throw mcrl2::runtime_error("Could not create temporary file for rewriter.");
	}

	return result;
}

static bool arity_is_allowed(
                     const data::function_symbol& func,
                     const size_t a)
{
  return arity_is_allowed(func.sort(),a);
}

static std::string get_heads(const sort_expression& s, const std::string& base_string, const size_t number_of_arguments)
{
  std::stringstream ss;
  if (is_function_sort(s) && number_of_arguments>0)
  {
    const function_sort fs(s);
    ss << "atermpp::down_cast<const application>(" << get_heads(fs.codomain(),base_string,number_of_arguments-fs.domain().size()) << ".head())";
    return ss.str();
  }
  return base_string;
}

static std::string get_recursive_argument(const sort_expression& s, const size_t index, const std::string& base_string, const size_t number_of_arguments)
{
  /* This function provides the index-th argument of an expression provided in base_string, given that its head
     symbol has type s and there are number_of_arguments arguments. Example: if f:D->E->F and index is 0, base_string
     is "t", base_string is set to "atermpp::down_cast<application>(t[0])[0] */
  assert(is_function_sort(s));

  std::stringstream ss;
  const function_sort& fs = atermpp::down_cast<function_sort>(s);
  const sort_expression_list& source_type=fs.domain();
  const sort_expression& target_type=fs.codomain();
  if (index>=source_type.size())
  {
    return get_recursive_argument(target_type, index-source_type.size(), base_string,number_of_arguments-source_type.size());
  }
  // ss << "atermpp::down_cast<application>(" << get_heads(target_type,base_string,number_of_arguments-source_type.size()) << ")[" << index << "]";
  ss << get_heads(target_type,base_string,number_of_arguments-source_type.size()) << "[" << index << "]";
  return ss.str();
}


inline
void declare_rewr_functions(FILE* f, const data::function_symbol& func, const size_t arity)
{
  for (size_t a=0; a<=arity; a++)
  {
    if (arity_is_allowed(func,a))
    {
      const size_t b = (a<=NF_MAX_ARITY)?a:0;
      for (size_t nfs=0; (nfs >> b) == 0; nfs++)
      {
        if (a==0)
        {
          // This is a constant function; result can be derived by reference.
          fprintf(f,  "static inline const data_expression& rewr_%zu_%zu_%zu(",core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(func),a,nfs);
        }
        else
        {
          fprintf(f,  "static inline data_expression rewr_%zu_%zu_%zu(",core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(func),a,nfs);
        }
        for (size_t i=0; i<a; i++)
        {
          if (((nfs >> i) & 1) ==1) // nfs indicates in binary form which arguments are in normal form.
          {
            fprintf(f, (i==0)?"const data_expression& arg%zu":", const data_expression& arg%zu",i);
          }
          else
          {
            fprintf(f, (i==0)?"const data_expression& arg_not_nf%zu":", const data_expression& arg_not_nf%zu",i);
          }
        }
        fprintf(f,  ");\n");

        fprintf(f,  "static inline data_expression rewr_%zu_%zu_%zu_term(const application& %s){ return rewr_%zu_%zu_%zu(",
            core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(func),
            a,
            nfs,
            (a==0?"":"t"),
            core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(func),
            a,
            nfs);
        for(size_t i = 0; i < a; ++i)
        {
          // fprintf(f,  "%st[%zu]", (i == 0?"":", "), i);
          fprintf(f,  "%s%s", (i == 0?"":", "), get_recursive_argument(func.sort(),i,"t",a).c_str());
        }
        fprintf(f,  "); }\n");
      }
    }
  }
}

void RewriterCompilingJitty::BuildRewriteSystem()
{
  FILE* f;
  CleanupRewriteSystem();

  // Try to find out from environment which compile script to use.
  // If not set, choose one of the following two:
  // * if "mcrl2compilerewriter" is in the same directory as the executable,
  //   this is the version we favour. This is especially needed for single
  //   bundle applications on MacOSX. Furthermore, it is the more foolproof
  //   approach on other platforms.
  // * by default, fall back to the system provided mcrl2compilerewriter script.
  //   in this case, we rely on the script being available in the user's
  //   $PATH environment variable.
  std::string compile_script;
  char* env_compile_script = getenv("MCRL2_COMPILEREWRITER");
  if (env_compile_script != NULL)
  {
    compile_script = env_compile_script;
  }
  else if(mcrl2::utilities::file_exists(mcrl2::utilities::get_executable_basename() + "/mcrl2compilerewriter"))
  {
    compile_script = mcrl2::utilities::get_executable_basename() + "/mcrl2compilerewriter";
  }
  else
  {
    compile_script = "mcrl2compilerewriter";
  }

  rewriter_so = new uncompiled_library(compile_script);
  mCRL2log(verbose) << "using '" << compile_script << "' to compile rewriter." << std::endl;

  jittyc_eqns.clear();


  // - Store all function symbols in a vector
  function_symbol_vector all_function_symbols = m_data_specification_for_enumeration.constructors();
  all_function_symbols.insert(all_function_symbols.begin(),
                              m_data_specification_for_enumeration.mappings().begin(),
                              m_data_specification_for_enumeration.mappings().end());

  // - Calculate maximum occurring arity
  // - Construct int2ar_idx
  size_t max_arity = 0;
  ar_size = 0;
  int2ar_idx.clear();
  for(function_symbol_vector::const_iterator l = all_function_symbols.begin();
      l != all_function_symbols.end(); ++l)
  {
    const data::function_symbol fs = *l;
    if (int2ar_idx.count(fs) == 0)
    {
      size_t arity = getArity(fs);
      int2ar_idx[fs] = ar_size;
      ar_size += (arity * (arity + 1)) / 2;
      if (arity > max_arity)
      {
        max_arity = arity;
      }
    }
  }

  for(std::set < data_equation >::const_iterator it=rewrite_rules.begin();
                   it!=rewrite_rules.end(); ++it)
  {
    jittyc_eqns[get_function_symbol_of_head(it->lhs())].push_front(*it);
  }
  fill_always_rewrite_array();

  f = MakeTempFiles();

  //  Print includes
  fprintf(f, "#define ADDR_OPID %ld\n"
             "#define INDEX_BOUND %zu\n"
             "#define ARITY_BOUND %zu\n"
             "#include \"mcrl2/data/detail/rewrite/jittycpreamble.h\"\n"
             "\n",
          atermpp::detail::addressf(sort_bool::true_().function()),
          core::index_traits<data::function_symbol,function_symbol_key_type, 2>::max_index()+1,
          max_arity+1);

  // - Forward declarations of rewr_* functions
  for(function_symbol_vector::const_iterator l = all_function_symbols.begin();
      l != all_function_symbols.end(); ++l)
  {
    const data::function_symbol fs=*l;
    if (data_equation_selector(fs))
    {
      declare_rewr_functions(f, fs, getArity(fs));
    }
  }

  // Make functions that construct applications with arity n where 5< n <= max_arity.
  for (size_t i=5; i<=max_arity; ++i)
  {
    fprintf(f, "static application make_term_with_many_arguments(const data_expression& head");
    for (size_t j=1; j<=i; ++j)
    {
      fprintf(f, ", const data_expression& arg%zu",j);
    }
    fprintf(f, ")\n"
               "{\n");

    // Currently data_expressions are stored in this array. If reference or pointers are stored,
    // no explicit destroy is needed anymore.
    fprintf(f,
      "  MCRL2_SYSTEM_SPECIFIC_ALLOCA(buffer,const atermpp::detail::_aterm*, %ld);\n ",i);

    for (size_t j=0; j<i; ++j)
    {
      fprintf(f, "  buffer[%ld]=atermpp::detail::address(arg%ld);\n",j,j+1);
    }

    fprintf(f, "  return application(head,(mcrl2::data::data_expression*)&buffer[0],(mcrl2::data::data_expression*)&buffer[%ld]);\n",i);
    fprintf(f, "}\n\n");
  }

  // Implement the equations of every operation. -------------------------------
  //
  for (function_symbol_vector::const_iterator j=all_function_symbols.begin();
              j!=all_function_symbols.end(); ++j)
  {
    const data::function_symbol fs=*j;
    const size_t arity = getArity(fs);
    if (data_equation_selector(fs))
    {
      stringstream ss;
      ss << fs.sort();
      fprintf(f,  "// %ld %s %s\n",core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(fs),to_string(fs).c_str(),ss.str().c_str());

      for (size_t a=0; a<=arity; a++)
      {
        if (arity_is_allowed(fs,a))
        {
          int b = (a<=NF_MAX_ARITY)?a:0;
          for (size_t nfs=0; (nfs >> b) == 0; nfs++)
          {
            if (a==0)
            {
              fprintf(f,  "static const data_expression& rewr_%zu_%zu_%zu(",core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(fs),a,nfs);
            }
            else
            {
              fprintf(f,  "static data_expression rewr_%zu_%zu_%zu(",core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(fs),a,nfs);
            }
            for (size_t i=0; i<a; i++)
            {
              if (((nfs >> i) & 1) ==1) // nfs indicates in binary form which arguments are in normal form.
              {
                fprintf(f, (i==0)?"const data_expression& arg%zu":", const data_expression& arg%zu",i);
              }
              else
              {
                fprintf(f, (i==0)?"const data_expression& arg_not_nf%zu":", const data_expression& arg_not_nf%zu",i);
              }
            }
            fprintf(f,  ")\n"
                    "{\n"
                   );
            if (!jittyc_eqns[fs].empty() )
            {
              // Implement strategy
              nfs_array nfs_a(a);
              nfs_a.set_encoded_number(nfs);
              implement_strategy(f,create_strategy(jittyc_eqns[fs], a,nfs_a),a,1,fs,nfs_a);
            }
            else
            {
              std::vector<bool> used;
              for (size_t k=0; k<a; k++)
              {
                used.push_back((nfs & ((size_t)1 << k)) != 0);
              }
              finish_function(f,a,fs,used);
            }

            fprintf(f,"}\n\n");
          }
        }
      }
    }
  }

  fprintf(f,
          "void rewrite_init(RewriterCompilingJitty *r)\n"
          "{\n"
          "  this_rewriter=r;\n"
          "  assert(this_rewriter->rewriter_binding_variable_lists.size()==%zu);\n"
          "  assert(this_rewriter->rewriter_bound_variables.size()==%zu);\n",
          rewriter_binding_variable_lists.size(),rewriter_bound_variables.size()
         );
  fprintf(f,  "\n");

  // Also add the created function symbols that represent partly rewritten functions.
  all_function_symbols.insert(all_function_symbols.begin(),
                              partially_rewritten_functions.begin(),
                              partially_rewritten_functions.end());

  /* put the functions that start the rewriting in the array int2func */
  fprintf(f,  "\n");
  fprintf(f,  "\n");
  // Generate the entries for int2func.
  for (size_t i=0; i<=max_arity; i++)
  {
    fprintf(f,  "  int2func[%zu] = new rewriter_function[%zu];\n",
                          i,core::index_traits<data::function_symbol,function_symbol_key_type, 2>::max_index()+1);
    fprintf(f,  "for(size_t j=0; j<%zu; ++j){ int2func[%zu][j] = do_nothing; };\n",
                          core::index_traits<data::function_symbol,function_symbol_key_type, 2>::max_index()+1,i);
    for (function_symbol_vector::const_iterator j=all_function_symbols.begin();
                        j!=all_function_symbols.end(); ++j)
    {
      const data::function_symbol fs=*j;

      if (partially_rewritten_functions.count(fs)>0)
      {
        if (arity_is_allowed(fs,i) && i>0)
        // if (i==total_arity_of_partially_rewritten_functions[fs])
        {
          // We are dealing with a partially rewritten function here. Remove the "@_" at
          // the beginning of the string.
          const string function_name=core::pp(fs.name());
          const size_t aaa_position=function_name.find("@@@");
          std::stringstream ss;
          ss << function_name.substr(2,aaa_position-2) << i << function_name.substr(aaa_position+3);
          fprintf(f,  "  int2func[%zu][%zu] = (rewriter_function)%s;   //Part.rewritten %s\n",
                                         i,
                                         core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(fs),
                                         // c_function_name.substr(2,c_function_name.size()-2).c_str(),
                                         ss.str().c_str(),
                                         function_name.c_str());
        }
      }
      else if (data_equation_selector(fs) && arity_is_allowed(fs,i))
      {
        fprintf(f,  "  int2func[%zu][%zu] = (rewriter_function)rewr_%zu_%zu_0_term;\n",
                        i,
                        core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(fs),
                        core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(fs),
                        i);
      }
    }
  }
  // Generate the entries for int2func_head_in_nf. Entries for constants (with arity 0) are not required.
  for (size_t i=1; i<=max_arity; i++)
  {
    fprintf(f,  "  int2func_head_in_nf[%zu] = new rewriter_function[%zu];\n",
                               i,core::index_traits<data::function_symbol,function_symbol_key_type, 2>::max_index()+1);
    fprintf(f,  "for(size_t j=0; j<%zu; ++j){ int2func_head_in_nf[%zu][j] = do_nothing; };\n",
                               core::index_traits<data::function_symbol,function_symbol_key_type, 2>::max_index()+1,i);
    for (function_symbol_vector::const_iterator j=all_function_symbols.begin();
                        j!=all_function_symbols.end(); ++j)
    {
      const data::function_symbol fs=*j;
      if (partially_rewritten_functions.count(fs)>0)
      {
        if (arity_is_allowed(fs,i) && i>0)
        // if (i==total_arity_of_partially_rewritten_functions[fs])
        {
          // We are dealing with a partially rewritten function here.
          // This cannot be invoked for any normal function.
        }
      }
      else if (data_equation_selector(fs) && arity_is_allowed(fs,i))
      {
        if (i<=NF_MAX_ARITY)
        {
          fprintf(f,  "  int2func_head_in_nf[%zu][%zu] = (rewriter_function)rewr_%zu_%zu_1_term;\n",i,
                        core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(fs),
                        core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(fs),i);
        }
        else
        {
          // If i>NF_MAX_ARITY no compiled rewrite function where the head is already in nf is available.
          fprintf(f,  "  int2func_head_in_nf[%zu][%zu] = rewr_%zu_%zu_0_term;\n",i,
                                core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(fs),
                                core::index_traits<data::function_symbol,function_symbol_key_type, 2>::index(fs),i);
        }
      }
    }
  }
  fprintf(f, "}\n");


  fclose(f);

  mCRL2log(verbose) << "compiling rewriter..." << std::endl;

  try
  {
    rewriter_so->compile(rewriter_source);
  }
  catch(std::runtime_error& e)
  {
    rewriter_so->leave_files();
    throw mcrl2::runtime_error(std::string("Could not compile rewriter: ") + e.what());
  }

  mCRL2log(verbose) << "loading rewriter..." << std::endl;

  bool (*init)(rewriter_interface*);
  rewriter_interface interface = { mcrl2::utilities::get_toolset_version(), "Unknown error when loading rewriter.", this, NULL, NULL };
  try
  {
    init = reinterpret_cast<bool(*)(rewriter_interface *)>(rewriter_so->proc_address("init"));
  }
  catch(std::runtime_error& e)
  {
    rewriter_so->leave_files();
    throw mcrl2::runtime_error(std::string("Could not load rewriter: ") + e.what());
  }

#ifdef NDEBUG // In non debug mode clear compiled files directly after loading.
  try
  {
    rewriter_so->cleanup();
  }
  catch (std::runtime_error& error)
  {
    mCRL2log(mcrl2::log::error) << "Could not cleanup temporary files: " << error.what() << std::endl;
  }
#endif

  if (!init(&interface))
  {
    throw mcrl2::runtime_error(std::string("Could not load rewriter: ") + interface.status);
  }
  so_rewr_cleanup = interface.rewrite_cleanup;
  so_rewr = interface.rewrite_external;

  mCRL2log(verbose) << interface.status << std::endl;
}

RewriterCompilingJitty::RewriterCompilingJitty(
                          const data_specification& data_spec,
                          const used_data_equation_selector& equation_selector)
  : Rewriter(data_spec,equation_selector),
    jitty_rewriter(data_spec,equation_selector),
    m_nf_cache(jitty_rewriter)
{
  so_rewr_cleanup = NULL;
  rewriter_so = NULL;
  initialise_common();

  made_files = false;
  rewrite_rules.clear();

  const data_equation_vector& l=data_spec.equations();
  for (data_equation_vector::const_iterator j=l.begin(); j!=l.end(); ++j)
  {
    if (data_equation_selector(*j))
    {
      const data_equation rule=*j;
      try
      {
        CheckRewriteRule(rule);
        if (rewrite_rules.insert(rule).second)
        {
          // The equation has been added as a rewrite rule, otherwise the equation was already present.
          // data_equation_selector.add_function_symbols(rule.lhs());
        }
      }
      catch (std::runtime_error& e)
      {
        mCRL2log(warning) << e.what() << std::endl;
      }
    }
  }

  int2ar_idx.clear();
  ar=std::vector<atermpp::aterm_appl>();
  BuildRewriteSystem();
}

RewriterCompilingJitty::~RewriterCompilingJitty()
{
  CleanupRewriteSystem();
}

data_expression RewriterCompilingJitty::rewrite(
     const data_expression& term,
     substitution_type& sigma)
{
#ifdef MCRL2_DISPLAY_REWRITE_STATISTICS
  data::detail::increment_rewrite_count();
#endif
  // Save global sigma and restore it afterwards, as rewriting might be recursive with different
  // substitutions, due to the enumerator.
  substitution_type *saved_sigma=global_sigma;
  global_sigma=&sigma;
  const data_expression result=so_rewr(term);
  global_sigma=saved_sigma;
  return result;
}

rewrite_strategy RewriterCompilingJitty::getStrategy()
{
  return jitty_compiling;
}

}
}
}

#endif
