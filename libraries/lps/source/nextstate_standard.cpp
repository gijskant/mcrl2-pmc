// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file nextstate_standard.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aterm2.h>
#include "mcrl2/core/detail/struct.h"
#include <mcrl2/data/enum.h>
#include <mcrl2/data/rewrite.h>
#include <mcrl2/lps/nextstate/standard.h>
#include "mcrl2/core/messaging.h"

using namespace mcrl2::core;
using namespace mcrl2::core::detail;

#define ATAgetFirst(x) ((ATermAppl) ATgetFirst(x))
#define ATLgetFirst(x) ((ATermList) ATgetFirst(x))
#define ATAgetArgument(x,y) ((ATermAppl) ATgetArgument(x,y))
#define ATLgetArgument(x,y) ((ATermList) ATgetArgument(x,y))

#define ATisList(x) (ATgetType(x) == AT_LIST)


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
static void fill_tree_init(bool *init, int n, int l)
{
	if ( l == 0 )
		return;

	if ( n > 2 )
	{
		fill_tree_init(init,n/2,l/2);
		fill_tree_init(init+n/2,n-n/2,l-l/2);
	} else /* n == 2 */ {
		init[1] = true;
	}
}
ATerm NextStateStandard::buildTree(ATerm *args)
{
	int n,m;

	if ( info.statelen == 0 )
		return (ATerm) info.nil;

	if ( tree_init == NULL )
	{
		tree_init = (bool *) malloc(info.statelen*sizeof(bool));
		for (int i=0; i<info.statelen; i++)
			tree_init[i] = false;
		n = 1;
		while ( n < info.statelen )
		{
			n *= 2;
		}
		n /= 2;
		fill_tree_init(tree_init,info.statelen,info.statelen-n);
	}

	n = 0;
	m = 0;
	while ( n < info.statelen )
	{
		if ( tree_init[n] )
		{
			args[m-1] = (ATerm) ATmakeAppl2(info.pairAFun,args[m-1],args[n]);
		} else {
			args[m] = args[n];
			m++;
		}
		n++;
	}

	n = m;
	while ( n > 1 )
	{
		for (int i=0; i<n; i+=2)
		{
			args[i/2] = (ATerm) ATmakeAppl2(info.pairAFun,args[i],args[i+1]);
		}
		
		n /= 2;
	}
	
	return args[0];
}

ATerm NextStateStandard::getTreeElement(ATerm tree, int index)
{
	int n = info.statelen;
	int m = 0;
	while ( m+1 != n )
	{
		int t = (m+n)/2;

		assert( (ATgetType(tree) == AT_APPL) && ATisEqualAFun(ATgetAFun((ATermAppl) tree),info.pairAFun) );

		if ( index < t )
		{
			tree = ATgetArgument((ATermAppl) tree,0);
			n = t;
		} else {
			tree = ATgetArgument((ATermAppl) tree,1);
			m = t;
		}
	}

	return tree;
}

int NextStateStandard::getStateLength()
{
	return info.statelen;
}

ATermAppl NextStateStandard::getStateArgument(ATerm state, int index)
{
	switch ( info.stateformat )
	{
		case GS_STATE_VECTOR:
			return info.rewr_obj->fromRewriteFormat(ATgetArgument((ATermAppl) state,index));
		case GS_STATE_TREE:
			return info.rewr_obj->fromRewriteFormat(getTreeElement(state,index));
		default:
			return NULL;
	}
}

ATermAppl NextStateStandard::makeStateVector(ATerm state)
{
	if ( !stateAFun_made )
	{
		stateAFun_made = true;
		info.stateAFun = ATmakeAFun("STATE",info.statelen,ATfalse);
		ATprotectAFun(info.stateAFun);
	}

	// XXX can be done more efficiently in some cases
	for (int i=0; i<info.statelen; i++)
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
	if ( vars == NULL )
	{
		tmp_vars = ATtableCreate(20,50);
	}


	bool r;
	if ( gsIsDataAppl(pat) )
	{
		if ( !gsIsDataAppl(arg) )
		{
			r = false;
		} else {
			r = statearg_match(ATAgetArgument(arg,0),ATAgetArgument(pat,0),tmp_vars)
			 && statearg_match_list(ATLgetArgument(arg,1),ATLgetArgument(pat,1),tmp_vars);
		}
	} else if  ( gsIsDataVarId(pat) )
	{
		ATerm val = ATtableGet(tmp_vars,(ATerm) pat);
		if ( val == NULL )
		{
			ATtablePut(tmp_vars,(ATerm) pat, (ATerm) arg);
			r = true;
		} else if ( ATisEqual(arg,val) )
		{
			r = true;
		} else {
			r = false;
		}
	} else {
		r = ATisEqual(arg,pat);
	}

	if ( vars == NULL )
	{
		ATtableDestroy(tmp_vars);
	}

	return r;
}

ATerm NextStateStandard::parseStateVector(ATermAppl state, ATerm match)
{
	if ( !stateAFun_made )
	{
		stateAFun_made = true;
		info.stateAFun = ATmakeAFun("STATE",info.statelen,ATfalse);
		ATprotectAFun(info.stateAFun);
	}

	if ( ATisEqualAFun(info.stateAFun,ATgetAFun(state)) )
	{
		bool valid = true;
		ATermList l = info.procvars;
		for (int i=0; i<info.statelen; i++)
		{
			stateargs[i] = ATgetArgument(state,i);
			if ( !ATisEqual(gsGetSort((ATermAppl) stateargs[i]),gsGetSort(ATAgetFirst(l))) )
			{
				valid = false;
				break;
			}
			if ( (match != NULL) && !statearg_match((ATermAppl) stateargs[i],getStateArgument(match,i)) )
			{
				valid = false;
				break;
			}
			stateargs[i] = info.rewr_obj->toRewriteFormat((ATermAppl) stateargs[i]);
			l = ATgetNext(l);
		}
		if ( valid )
		{
			switch ( info.stateformat )
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

ATermAppl NextStateStandard::FindDummy(ATermAppl sort, ATermList no_dummy)
{
	ATermList l;

	no_dummy = ATinsert(no_dummy,(ATerm) sort);

	if ( gsIsSortArrow(sort) )
	{
                // Take dataspec from current_spec, then take the consspec from the dataspec
                // and take the list of opids (l) from this consspec
		l = ATLgetArgument(ATAgetArgument(ATAgetArgument(current_spec,0),1),0);
                
		for (; !ATisEmpty(l); l=ATgetNext(l))
		{
			ATermAppl conssort = ATAgetArgument(ATAgetFirst(l),1);
			if ( ATisEqual(conssort,sort) )
			{
				return ATAgetFirst(l);
			}
		}

                // Take dataspec from current_spec, then take the mapspec from the dataspec
                // and take the list of opids (l) from this mapspec
		l = ATLgetArgument(ATAgetArgument(ATAgetArgument(current_spec,0),2),0);
		for (; !ATisEmpty(l); l=ATgetNext(l))
		{
			ATermAppl mapsort = ATAgetArgument(ATAgetFirst(l),1);
			if ( ATisEqual(mapsort,sort) )
			{
				return ATAgetFirst(l);
			}
		}
	} else {
		l = ATLgetArgument(ATAgetArgument(ATAgetArgument(current_spec,0),1),0);
		for (; !ATisEmpty(l); l=ATgetNext(l))
		{
			ATermAppl conssort = ATAgetArgument(ATAgetFirst(l),1);
			if ( ATisEqual(gsGetSortExprResult(conssort),sort) )
			{
				ATermList domains = gsGetSortExprDomains(conssort);
				ATermAppl t = ATAgetFirst(l);
	
				bool found = true;
				for (; !ATisEmpty(domains); domains=ATgetNext(domains))
				{
                                        ATermList domain = ATLgetFirst(domains);
                                        ATermList dummies = ATmakeList0();
                                        for (; !ATisEmpty(domain); domain=ATgetNext(domain))
                                        {
					        if ( ATindexOf(no_dummy,ATgetFirst(domain),0) >= 0 )
					        {
					  	        found = false;
						        break;
					        }
                                                dummies = ATinsert(dummies, (ATerm) FindDummy(ATAgetFirst(domain),no_dummy));
                                        }
                                        dummies = ATreverse(dummies);
					t = gsMakeDataAppl(t,dummies);
				}
	
				if ( found )
				{
					return t;
				}
			}
		}
	
		l = ATLgetArgument(ATAgetArgument(ATAgetArgument(current_spec,0),2),0);
		for (; !ATisEmpty(l); l=ATgetNext(l))
		{
			ATermAppl mapsort = ATAgetArgument(ATAgetFirst(l),1);
			if ( ATisEqual(gsGetSortExprResult(mapsort),sort) )
			{
				ATermList domain = gsGetSortExprDomain(mapsort);
				ATermAppl t = ATAgetFirst(l);
	
				bool found = true;
				for (; !ATisEmpty(domain); domain=ATgetNext(domain))
				{
					if ( ATindexOf(no_dummy,ATgetFirst(domain),0) >= 0 )
					{
						found = false;
						break;
					}
					t = gsMakeDataAppl1(t,FindDummy(ATAgetFirst(domain),no_dummy));
				}
	
				if ( found )
				{
					return t;
				}
			}
		}
	}

	gsErrorMsg("could not find dummy of type %T\n",sort);
	exit(1);
}

ATerm NextStateStandard::SetVars(ATerm a, ATermList free_vars)
{
	ATermList l,m;

	if ( !usedummies )
	{
		return a;
	}

	if ( ATisList(a) )
	{
		l = (ATermList) a;
		m = ATmakeList0();
		for (; !ATisEmpty(l); l=ATgetNext(l))
		{
			m = ATinsert(m,SetVars(ATgetFirst(l),free_vars));
		}
		return (ATerm) ATreverse(m);
	} else if ( gsIsDataVarId((ATermAppl) a) )
	{
		if ( ATindexOf(free_vars,a,0) >= 0 )
		{
			return (ATerm) FindDummy(ATAgetArgument((ATermAppl) a,1));
		} else {
			return a;
		}
	} else if ( gsIsDataAppl((ATermAppl) a) )
	{
		return (ATerm) gsMakeDataAppl((ATermAppl) SetVars(ATgetArgument((ATermAppl) a,0),free_vars),(ATermList) SetVars(ATgetArgument((ATermAppl) a,1),free_vars));
	} else {
		return a;
	}
}

ATermList NextStateStandard::ListToFormat(ATermList l,ATermList free_vars)
{
	if ( ATisEmpty(l) )
	{
		return l;
	} else {
		return ATinsert(ListToFormat(ATgetNext(l),free_vars),info.rewr_obj->toRewriteFormat((ATermAppl) SetVars(ATgetFirst(l),free_vars)));
	}
}

#define LISTFROMFORMAT_IMPLEMENTATION \
	if ( ATisEmpty(l) ) \
	{ \
		return l; \
	} else { \
		return ATinsert(ListFromFormat(ATgetNext(l)),(ATerm) info.rewr_obj->fromRewriteFormat(ATgetFirst(l))); \
	}
ATermList NextStateStandard::ListFromFormat(ATermList l)
{
	LISTFROMFORMAT_IMPLEMENTATION
}

ATermList NextStateGeneratorStandard::ListFromFormat(ATermList l)
{
	LISTFROMFORMAT_IMPLEMENTATION
}

ATermAppl NextStateStandard::ActionToRewriteFormat(ATermAppl act, ATermList free_vars)
{
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

ATermList NextStateStandard::AssignsToRewriteFormat(ATermList assigns, ATermList free_vars)
{
	int i = 0;
	for (ATermList l=pars; !ATisEmpty(l); l=ATgetNext(l),i++)
	{
		bool set = false;

		for (ATermList m=assigns; !ATisEmpty(m); m=ATgetNext(m))
		{
			if ( ATisEqual(ATAgetArgument(ATAgetFirst(m),0),ATAgetFirst(l)) )
			{
				stateargs[i] = info.rewr_obj->toRewriteFormat((ATermAppl) SetVars(ATgetArgument(ATAgetFirst(m),1),free_vars));
				set = true;
				break;
			}
		}
		if ( !set )
		{
			stateargs[i] = (ATerm) gsMakeNil();
		}
	}

	ATermList r = ATmakeList0();
	i=info.statelen;
	while ( i != 0 )
	{
		i--;
		r = ATinsert(r,stateargs[i]);
	}
	return r;
}

NextStateStandard::NextStateStandard(ATermAppl spec, bool allow_free_vars, int state_format, Enumerator *e, bool clean_up_enumerator)
{
	ATermList l,m,n,free_vars;

	next_id = 0;
	info.current_id = &current_id;

	info.parent = this;

	info.enum_obj = e;
	clean_up_enum_obj = clean_up_enumerator;
	info.rewr_obj = e->getRewriter();

	usedummies = !allow_free_vars;

	tree_init = NULL;

	current_spec = spec;
	ATprotectAppl(&current_spec);

	info.stateformat = state_format;
	info.pairAFun = ATmakeAFun("@STATE_PAIR@",2,ATfalse);
	ATprotectAFun(info.pairAFun);

	info.nil = gsMakeNil();
	ATprotectAppl(&info.nil);

	free_vars = ATLgetArgument(ATAgetArgument(current_spec,2),0);

	pars = ATLgetArgument(ATAgetArgument(current_spec,2),1);
	ATprotectList(&pars);

	info.statelen = ATgetLength(pars);
	if ( info.stateformat == GS_STATE_VECTOR )
	{
		stateAFun_made = true;
		info.stateAFun = ATmakeAFun("STATE",info.statelen,ATfalse);
		ATprotectAFun(info.stateAFun);
	} else {
		stateAFun_made = false;
	}

	info.procvars = ATLgetArgument(ATAgetArgument(current_spec,2),1);
	ATprotectList(&info.procvars);
	
	stateargs = (ATerm *) malloc(info.statelen*sizeof(ATerm));
	for (int i=0; i<info.statelen; i++)
	{
		stateargs[i] = NULL;
	}
	ATprotectArray(stateargs,info.statelen);

	smndAFun = ATmakeAFun("@SMND@",4,ATfalse);
	ATprotectAFun(smndAFun);
	ATermList sums = ATLgetArgument(ATAgetArgument(current_spec,2),2);
	l = ATmakeList0();
	for (bool b=true; !ATisEmpty(sums); sums=ATgetNext(sums))
	{
		if ( b && !gsIsNil(ATAgetArgument(ATAgetFirst(sums),3)) )
		{
			gsWarningMsg("specification uses time, which is (currently) not supported; ignoring timing\n");
			b = false;
		}
		if ( !gsIsDelta(ATAgetArgument(ATAgetFirst(sums),2)) )
		{
			l = ATinsert(l,ATgetFirst(sums));
		}
	}
	sums = ATreverse(l);
	info.num_summands = ATgetLength(sums);
	info.num_prioritised = 0;
	info.summands = (ATermAppl *) malloc(info.num_summands*sizeof(ATermAppl));
	for (int i=0; i<info.num_summands; i++)
	{
		info.summands[i] = NULL;
	}
	ATprotectArray((ATerm *) info.summands,info.num_summands);
	for (int i=0; !ATisEmpty(sums); sums=ATgetNext(sums),i++)
	{
		info.summands[i] = ATmakeAppl4(smndAFun,ATgetArgument(ATAgetFirst(sums),0),info.rewr_obj->toRewriteFormat((ATermAppl) SetVars(ATgetArgument(ATAgetFirst(sums),1),free_vars)),(ATerm) ActionToRewriteFormat(ATAgetArgument(ATAgetFirst(sums),2),free_vars),(ATerm) AssignsToRewriteFormat(ATLgetArgument(ATAgetFirst(sums),4),free_vars));
	}

	l = pars;
	free_vars = ATLgetArgument(ATAgetArgument(current_spec,3),0);
	m = ATLgetArgument(ATAgetArgument(current_spec,3),1);

	for (int i=0; !ATisEmpty(l); l=ATgetNext(l), i++)
	{
		n = m;
		bool set = false;
		for (; !ATisEmpty(n); n=ATgetNext(n))
		{
			if ( ATisEqual(ATAgetArgument(ATAgetFirst(n),0),ATAgetFirst(l)) )
			{
				stateargs[i] = info.rewr_obj->rewriteInternal(info.rewr_obj->toRewriteFormat((ATermAppl) SetVars(ATgetArgument(ATAgetFirst(n),1),free_vars)));
				set = true;
				break;
			}
		}
		if ( !set )
		{
			gsErrorMsg("Parameter '%T' does not have an initial value.",ATgetArgument(ATAgetFirst(l),0));
			initial_state = NULL;
			ATprotect(&initial_state);
			return;
		}
	}

	switch ( info.stateformat )
	{
		case GS_STATE_VECTOR:
			initial_state = (ATerm) ATmakeApplArray(info.stateAFun,stateargs);
			break;
		case GS_STATE_TREE:
			initial_state = (ATerm) buildTree(stateargs);
			break;
		default:
			initial_state = NULL;
			break;
	}
	ATprotect(&initial_state);
}

NextStateStandard::~NextStateStandard()
{
	ATunprotect(&initial_state);

	ATunprotectArray(stateargs);

	ATunprotectAppl(&current_spec);
	ATunprotectAppl(&info.nil);

	if ( clean_up_enum_obj )
	{
		delete info.enum_obj;
	}
	
	ATunprotectAFun(info.pairAFun);

	ATunprotectList(&pars);
	if ( stateAFun_made )
	{
		ATunprotectAFun(info.stateAFun);
	}

	ATunprotectList(&info.procvars);
	
	ATunprotectAFun(smndAFun);
	ATunprotectArray((ATerm *) info.summands);
	free(info.summands);

	free(tree_init);
}

static bool only_action(ATermList ma, const char *action)
{
	if ( ATisEmpty(ma) )
	{
		return false;
	}

	for (;!ATisEmpty(ma); ma=ATgetNext(ma))
	{
		if ( strcmp(ATgetName(ATgetAFun(ATAgetArgument(ATAgetArgument(ATAgetFirst(ma),0),0))),action) )
		{
			return false;
		}
	}
	return true;
}
void NextStateStandard::prioritise(const char *action)
{
	// XXX this function invalidates currently used generators!
	// perhaps
	bool is_tau = !strcmp(action,"tau");
	int pos = 0;
	int rest = 0;

	while ( pos < info.num_summands )
	{
		ATermAppl s = info.summands[pos];
		ATermList ma = ATLgetArgument(ATAgetArgument(s,2),0);
		if ( (is_tau && ATisEmpty(ma)) || (!is_tau && only_action(ma,action)) )
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

ATerm NextStateStandard::getInitialState()
{
	return initial_state;
}

NextStateGenerator *NextStateStandard::getNextStates(ATerm state, NextStateGenerator *old)
{
	if ( old == NULL )
	{
		return new NextStateGeneratorStandard(state,info,next_id++);
	} else {
		((NextStateGeneratorStandard *) old)->reset(state);
		return old;
	}
}

Rewriter *NextStateStandard::getRewriter()
{
	return info.rewr_obj;
}


Enumerator *NextStateStandard::getEnumerator()
{
	return info.enum_obj;
}



ATerm NextStateGeneratorStandard::makeNewState(ATerm old, ATermList assigns)
{
	ATermList l;
		
	if ( *info.current_id != id )
	{
		set_substitutions();
	}

	l = info.procvars;
	for (int i=0; i<info.statelen; i++)
	{
		ATerm a = ATgetFirst(assigns);
		assigns = ATgetNext(assigns);
		if ( ATisEqual(a,info.nil) )
		{
			switch ( info.stateformat )
			{
				default:
				case GS_STATE_VECTOR:
					stateargs[i] = ATgetArgument((ATermAppl) old,i);
					break;
				case GS_STATE_TREE:
//					stateargs[i] = getTreeElement(old,i);
					stateargs[i] = info.rewr_obj->getSubstitution((ATermAppl) ATgetFirst(l));
					if ( ATisEqual(stateargs[i], ATgetFirst(l)) ) // Make sure substitutions where not reset by enumerator
					{
						set_substitutions();
						stateargs[i] = info.rewr_obj->getSubstitution((ATermAppl) ATgetFirst(l));
					}
					break;
			}
		} else {
			stateargs[i] = info.rewr_obj->rewriteInternal(a);
//			stateargs[i] = rewr_obj->rewriteInternal(SetVars(a));
		}
		l = ATgetNext(l);
	}
	switch ( info.stateformat )
	{
		case GS_STATE_VECTOR:
			return (ATerm) ATmakeApplArray(info.stateAFun,stateargs);
		case GS_STATE_TREE:
			return info.parent->buildTree(stateargs);
		default:
			return NULL;
	}
}

ATermAppl NextStateGeneratorStandard::rewrActionArgs(ATermAppl act)
{
	ATermList l = ATLgetArgument(act,0);
	ATermList m = ATmakeList0();

	for (; !ATisEmpty(l); l=ATgetNext(l))
	{
		ATermAppl a = ATAgetFirst(l);
		a = gsMakeAction(ATAgetArgument(a,0),ListFromFormat(info.rewr_obj->rewriteInternalList(ATLgetArgument(a,1))));
		m = ATinsert(m,(ATerm) a);
	}
	m = ATreverse(m);

	return gsMakeMultAct(m);
}

void NextStateGeneratorStandard::SetTreeStateVars(ATerm tree, ATermList *vars)
{
	if ( ATgetType(tree) == AT_APPL )
	{
		if ( ATisEqual(tree,info.nil) )
		{
			return;
		} else if ( ATisEqualAFun(ATgetAFun((ATermAppl) tree),info.pairAFun) )
		{
			SetTreeStateVars(ATgetArgument((ATermAppl) tree,0),vars);
			SetTreeStateVars(ATgetArgument((ATermAppl) tree,1),vars);
			return;
		}
	}

	info.rewr_obj->setSubstitution((ATermAppl) ATgetFirst(*vars),tree);
	*vars = ATgetNext(*vars);
}

NextStateGeneratorStandard::NextStateGeneratorStandard(ATerm State, ns_info &Info, unsigned int identifier)
{
	info = Info;
	id = identifier;

	cur_state = NULL;
	cur_act = NULL;
	cur_nextstate = NULL;
	ATprotect(&cur_state);
	ATprotect(&cur_act);
	ATprotectList(&cur_nextstate);

	stateargs = (ATerm *) malloc(info.statelen*sizeof(ATerm));
	for (int i=0; i<info.statelen; i++)
	{
		stateargs[i] = NULL;
	}
	ATprotectArray(stateargs,info.statelen);

	sols = NULL;

	reset(State);
}

NextStateGeneratorStandard::~NextStateGeneratorStandard()
{
	delete sols;

	ATunprotectArray(stateargs);
	free(stateargs);

	ATunprotectList(&cur_nextstate);
	ATunprotect(&cur_act);
	ATunprotect(&cur_state);
}

void NextStateGeneratorStandard::set_substitutions()
{
	ATermList l = info.procvars;
	switch ( info.stateformat )
	{
		case GS_STATE_VECTOR:
			for (int i=0; !ATisEmpty(l); l=ATgetNext(l),i++)
			{
				ATerm a = ATgetArgument((ATermAppl) cur_state,i);

				if ( !ATisEqual(a,info.nil) )
				{
					info.rewr_obj->setSubstitution((ATermAppl) ATgetFirst(l),a);
				}
			}
			break;
		case GS_STATE_TREE:
			SetTreeStateVars(cur_state,&l);
			break;
	}

	*info.current_id = id;
}

void NextStateGeneratorStandard::reset(ATerm State)
{
	error = false;

	cur_state = State;

	set_substitutions();

	if ( info.num_summands == 0 )
	{
		sols = info.enum_obj->findSolutions(ATmakeList0(),info.rewr_obj->toRewriteFormat(gsMakeDataExprFalse()),sols);
	} else {
		cur_act = ATgetArgument(info.summands[0],2);
		cur_nextstate = (ATermList) ATgetArgument(info.summands[0],3);
		sols = info.enum_obj->findSolutions(ATLgetArgument(info.summands[0],0),ATgetArgument(info.summands[0],1),sols);
	}
	sum_idx = 1;
}

bool NextStateGeneratorStandard::next(ATermAppl *Transition, ATerm *State, bool *prioritised)
{
	ATermList sol;

	while ( !sols->next(&sol) && !error && (sum_idx < info.num_summands) )
	{
		error |= sols->errorOccurred();
		if ( !error )
		{
			cur_act = ATgetArgument(info.summands[sum_idx],2);
			cur_nextstate = (ATermList) ATgetArgument(info.summands[sum_idx],3);

			if ( *info.current_id != id )
			{
				set_substitutions();
			}

			sols = info.enum_obj->findSolutions(ATLgetArgument(info.summands[sum_idx],0),ATgetArgument(info.summands[sum_idx],1),sols);
		
			sum_idx++;
		}
	}
	error |= sols->errorOccurred();

	if ( sol != NULL )
	{
		if ( *info.current_id != id )
		{
			set_substitutions();
		}
		for (ATermList m=sol; !ATisEmpty(m); m=ATgetNext(m))
		{
			info.rewr_obj->setSubstitution((ATermAppl) ATgetArgument((ATermAppl) ATgetFirst(m),0),ATgetArgument((ATermAppl) ATgetFirst(m),1));
		}
		*Transition = rewrActionArgs((ATermAppl) cur_act);
		*State = (ATerm) makeNewState(cur_state,cur_nextstate);
		if ( prioritised != NULL )
		{
			*prioritised = (sum_idx <= info.num_prioritised);
		}
		for (ATermList m=sol; !ATisEmpty(m); m=ATgetNext(m))
		{
			info.rewr_obj->clearSubstitution((ATermAppl) ATgetArgument((ATermAppl) ATgetFirst(m),0));
		}
		return true;
	} else {
		*Transition = NULL;
		*State = NULL;
		return false;
	}
}

bool NextStateGeneratorStandard::errorOccurred()
{
	return error;
}

ATerm NextStateGeneratorStandard::get_state()
{
	return cur_state;
}
