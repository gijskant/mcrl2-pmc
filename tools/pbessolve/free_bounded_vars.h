
//  Copyright 2007 Simona Orzan. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ./free_bounded_vars.h

#include "atermpp/substitute.h"
#include "util.h"


std::set<std::string> context;

// auxiliary function that replaces all variables
// from the given list bv with new variables
pbes_expression newnames(pbes_expression p, data_variable_list bv)
{
  if (bv.empty()) return p;
  data_variable_list newbv = fresh_variables(bv,context,"%d");
  for (data_variable_list::iterator x = newbv.begin(); x != newbv.end(); x++)
    context.insert(x->name());
  //  std::cout<<"\nNEWNAMES bv="<< pp(bv).c_str()<<" , newbv=" << pp(newbv).c_str()<<"\n";
  return p.substitute(make_list_substitution(bv, newbv));
}




/// Renames some bounded variable occurences with new variables,
/// such that, in the returned expression: 
///  . every variable is bounded by only one quantifier
///  . no variable occurs both bounded and free
/// If necessary, more than one new names for the same old name will be introduced. 
/// For instance, forall x.A(x) /\ exists x.B(x) 
/// becomes forall x0.A(x0) /\ exists x1.B(x1).
///
/// As side effect, two variable lists are returned:
/// the free and the bounded occurences
pbes_expression remove_double_variables_rec
(pbes_expression p, data_variable_list* fv, data_variable_list* bv)
{
  //  std::cout<<"RDV: start "<<pp(p).c_str()<<"\n";

  if ((is_and(p)) || (is_or(p)) || (is_imp(p))) {
    data_variable_list fvl,fvr,bvl,bvr;
    pbes_expression pleft = remove_double_variables_rec(lhs(p),&fvl,&bvl);
    pbes_expression pright = remove_double_variables_rec(rhs(p),&fvr,&bvr);
    data_variable_list toreplace = intersect(bvl,fvr);
    dunion(toreplace,intersect(bvl,bvr));
    pbes_expression pleft_ok = newnames(pleft,toreplace);
    toreplace = intersect(bvr,fvl);
    pbes_expression pright_ok = newnames(pright,toreplace);
    *fv = dunion(fvl,fvr); // the free vars are never renamed 
    *bv = dunion(bvl,bvr); // no need to add the new names, because they will
                           // not occur again later
    //   std::cout<<"RDV: end "<<pp(pleft_ok).c_str()<<"     *     "<<pp(pright_ok).c_str()<<"\n";
   
    if (is_and(p)) return and_(pleft_ok,pright_ok);
    else if (is_or(p)) return or_(pleft_ok,pright_ok);    
    else return imp(pleft_ok,pright_ok);
  }
  else if ((is_forall(p))||(is_exists(p))) {
    pbes_expression punder = remove_double_variables_rec(quant_expr(p),fv,bv);
    data_variable_list qv = quant_vars(p);
    // if the quantifier is useless, dump it
    if (intersect(qv,*fv).empty()) return punder;
    if (!intersect(qv,*bv).empty()) return punder;
    // otherwise, extend the bv list and leave the quantifier unchanged
    dunion(bv,qv);
    //    std::cout<<"RDV: end  Q"<< pp(qv).c_str()<<pp(punder).c_str()<<"\n";
    return (is_forall(p)? forall(qv,punder):exists(qv,punder));
  }
  else if (is_not(p)){
    return not_(remove_double_variables_rec(not_arg(p),fv,bv));
  }
  else if (is_data(p)){
    // fill in the list of occuring variables
    std::set<data_variable> setfv = find_variables(p);
    for (std::set<data_variable>::iterator i=setfv.begin(); i!=setfv.end();i++)
      *fv = push_back(*fv,*i);
    //    std::cout<<"RDV: end " <<pp(p).c_str()<<"\n";
    return p;
  }
  else // true,false or propositional variable instantiation
    {
      //    std::cout<<"RDV: end " <<pp(p).c_str()<<"\n";
      return p;
    }
}


/// Renames some bounded variable occurences with new variables,
/// such that, in the returned expression: 
///  . every variable is bounded by only one quantifier
///  . no variable occurs both bounded and free
/// If necessary, more than one new names for the same old name will be introduced. 
/// For instance, forall x.A(x) /\ exists x.B(x) 
/// becomes forall x0.A(x0) /\ exists x1.B(x1).
///
/// As side effect, two variable lists are returned:
/// the free and the bounded occurences
pbes_expression remove_double_variables(pbes_expression p)
{
  data_variable_list fv;
  data_variable_list bv;
  context = find_variable_name_strings(p);
  return (remove_double_variables_rec(p,&fv,&bv));
}





/// Same effect as remove_double_variables,
/// but differently implemented (BRUTE FORCE): 
/// all quantified variables are simply renamed with new names, 
/// within the scope of their quantifier.
pbes_expression remove_double_variables_fast(pbes_expression p)
{
 
  // !!!! IMPLEMENT THIS

 return p;
}
