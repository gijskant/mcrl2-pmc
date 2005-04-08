/* $Id: gsrewr_inner3.c,v 1.2 2005/04/08 12:33:51 muck Exp $ */

#define NAME "rewr_inner3"

#include <stdio.h>
#include <stdlib.h>
#include "aterm2.h"
#include "gslowlevel.h"
#include "gsfunc.h"
#include "gsrewr_inner3.h"
#include "libgsparse.h"

extern ATermList opid_eqns;
extern ATermList dataappl_eqns;

static ATermTable term2int;
static unsigned int num_opids;
static ATermAppl *int2term;
static ATermList *inner3_eqns;

static bool ATisList(ATerm a)
{
	return (ATgetType(a) == AT_LIST);
}

static bool ATisInt(ATerm a)
{
	return (ATgetType(a) == AT_INT);
}

static ATerm OpId2Int(ATermAppl Term, bool add_opids)
{
	ATermInt i;

	if ( (i = (ATermInt) ATtableGet(term2int,(ATerm) Term)) == NULL )
	{
		if ( !add_opids )
		{
			return (ATerm) Term;
		}
		i = ATmakeInt(num_opids);
		ATtablePut(term2int,(ATerm) Term,(ATerm) i);
		num_opids++;
	}

	return (ATerm) i;
}

static ATerm toInner(ATermAppl Term, bool add_opids)
{
	ATermList l;

	if ( !gsIsDataAppl(Term) )
	{
		if ( gsIsOpId(Term) )
		{
			return (ATerm) OpId2Int(Term,add_opids);
		} else {
			return (ATerm) Term;
		}
	}

	l = ATmakeList0();
	while ( gsIsDataAppl(Term) )
	{
		l = ATinsert(l,(ATerm) toInner(ATAgetArgument((ATermAppl) Term,1),add_opids));
		Term = ATAgetArgument(Term,0);
	}
	if ( gsIsOpId(Term) )
	{
		l = ATinsert(l,(ATerm) OpId2Int(Term,add_opids));
	} else {
		l = ATinsert(l,(ATerm) Term);
	}

	return (ATerm) l;
}

static ATermAppl fromInner(ATerm Term)
{
	ATermList l;
	ATerm t;
	ATermAppl a;

	if ( !ATisList(Term) )
	{
		if ( ATisInt(Term) )
		{
			return int2term[ATgetInt((ATermInt) Term)];
		} else {
			return (ATermAppl) Term;
		}
	}

	if ( ATisEmpty((ATermList) Term) )
	{
		ATfprintf(stderr,"%s: invalid inner format term (%t)\n",NAME,Term);
		exit(1);
	}
	
	l = (ATermList) Term;
	t = ATgetFirst(l);
	if ( ATisInt(t) )
	{
		a = int2term[ATgetInt((ATermInt) t)];
	} else {
		a = (ATermAppl) t;
	}
	l = ATgetNext(l);
	for (; !ATisEmpty(l); l=ATgetNext(l))
	{
		a = gsMakeDataAppl(a,fromInner(ATgetFirst(l)));
	}

	return a;
}

void rewrite_init_inner3()
{
	ATermList l,m,n;
	ATermTable tmp_eqns;
	ATermInt i;

	tmp_eqns = ATtableCreate(100,100); // XXX would be nice to know the number op OpIds
	term2int = ATtableCreate(100,100);

	l = opid_eqns;
	for (; !ATisEmpty(l); l=ATgetNext(l))
	{
		// XXX only adds the last rule where lhs is an opid; this might go "wrong" if this rule is removed later
		ATtablePut(tmp_eqns,OpId2Int(ATAgetArgument(ATAgetFirst(l),2),true),(ATerm) ATmakeList1((ATerm) ATmakeList4((ATerm) ATmakeList0(),ATgetArgument(ATAgetFirst(l),1),(ATerm) ATmakeList0(),toInner(ATAgetArgument(ATAgetFirst(l),3),true))));
	}

	l = dataappl_eqns;
	for (; !ATisEmpty(l); l=ATgetNext(l))
	{
		m = (ATermList) toInner(ATAgetArgument(ATAgetFirst(l),2),true);
		if ( (n = (ATermList) ATtableGet(tmp_eqns,ATgetFirst(m))) == NULL )
		{
			n = ATmakeList0();
		}
		n = ATinsert(n,(ATerm) ATmakeList4(ATgetArgument(ATAgetFirst(l),0),ATgetArgument(ATAgetFirst(l),1),(ATerm) ATgetNext(m),toInner(ATAgetArgument(ATAgetFirst(l),3),true)));
		ATtablePut(tmp_eqns,ATgetFirst(m),(ATerm) n);
	}

	int2term = (ATermAppl *) malloc(num_opids*sizeof(ATermAppl));
	inner3_eqns = (ATermList *) malloc(num_opids*sizeof(ATermList));

	l = ATtableKeys(term2int);
	for (; !ATisEmpty(l); l=ATgetNext(l))
	{
		i = (ATermInt) ATtableGet(term2int,ATgetFirst(l));
		int2term[ATgetInt(i)] = ATAgetFirst(l);
		if ( (m = (ATermList) ATtableGet(tmp_eqns,(ATerm) i)) == NULL )
		{
			inner3_eqns[ATgetInt(i)] = NULL;
		} else {
			inner3_eqns[ATgetInt(i)] = ATreverse(m);
		}
	}

	ATtableDestroy(tmp_eqns);
}

void rewrite_add_inner3(ATermAppl eqn)
{
	ATermList l,m;
	ATermAppl a;
	ATermInt i,j;
	unsigned int old_num;

	old_num = num_opids;

	a = ATAgetArgument(eqn,2);
	if ( gsIsOpId(a) )
	{
		j = (ATermInt) OpId2Int(a,true);
		m = ATmakeList4((ATerm) ATmakeList0(),ATgetArgument(eqn,1),(ATerm) ATmakeList0(),toInner(ATAgetArgument(eqn,3),true));
	} else {
		l = (ATermList) toInner(a,true);
		j = (ATermInt) ATgetFirst(l);
		m = ATmakeList4(ATgetArgument(eqn,0),ATgetArgument(eqn,1),(ATerm) ATgetNext(l),toInner(ATAgetArgument(eqn,3),true));
	}

	l = ATtableKeys(term2int);
	for (; !ATisEmpty(l); l=ATgetNext(l))
	{
		i = (ATermInt) ATtableGet(term2int,ATgetFirst(l));
		if ( ATgetInt(i) >= old_num )
		{
			int2term[ATgetInt(i)] = ATAgetFirst(l);
			inner3_eqns[ATgetInt(i)] = NULL;
		}
	}
	if ( inner3_eqns[ATgetInt(j)] == NULL )
	{
		inner3_eqns[ATgetInt(j)] = ATmakeList1((ATerm) m);
	} else {
		inner3_eqns[ATgetInt(j)] = ATappend(inner3_eqns[ATgetInt(j)],(ATerm) m);
	}
}

void rewrite_remove_inner3(ATermAppl eqn)
{
	ATermList l,m;
	ATermAppl a;
	ATerm t;

	a = ATAgetArgument(eqn,2);
	if ( gsIsOpId(a) )
	{
		t = OpId2Int(a,false);
		m = ATmakeList4((ATerm) ATmakeList0(),ATgetArgument(eqn,1),(ATerm) ATmakeList0(),toInner(ATAgetArgument(eqn,3),true));
	} else {
		l = (ATermList) toInner(a,false);
		t = ATgetFirst(l);
		m = ATmakeList4(ATgetArgument(eqn,0),ATgetArgument(eqn,1),(ATerm) ATgetNext(l),toInner(ATAgetArgument(eqn,3),true));
	}

	if ( ATisInt(t) )
	{
		l = inner3_eqns[ATgetInt((ATermInt) t)];
		m = ATmakeList0();
		for (; !ATisEmpty(l); l=ATgetNext(l))
		{
			if ( !ATisEqual(m,ATgetFirst(l)) )
			{
				ATinsert(m,ATgetFirst(l));
			}
		}
		if ( ATisEmpty(m) )
		{
			inner3_eqns[ATgetInt((ATermInt) t)] = NULL;
		} else {
			inner3_eqns[ATgetInt((ATermInt) t)] = ATreverse(m);
		}
	}
}

static ATerm subst_values(ATermList s, ATerm t)
{
	ATerm a,h;
	ATermList l,m;

	h = NULL;

	if ( ATisList(t) )
	{
		if ( ATisEmpty((ATermList) t) )
		{
			return t;
		}
		l = ATreverse((ATermList) t);
		m = ATmakeList0();
		while ( !ATisEmpty(l) )
		{
			a = subst_values(s,ATgetFirst(l));
			l = ATgetNext(l);
			if ( ATisEmpty(l) )
			{
				h = a;
			} else {
				m = ATinsert(m,a);
			}
		}
		if ( ATisList(h) )
		{
			return (ATerm) ATconcat((ATermList) h,m);
		} else {
			return (ATerm) ATinsert(m,h);
		}
	} else if ( gsIsDataVarId((ATermAppl) t) )
	{
		for (; !ATisEmpty(s); s=ATgetNext(s))
		{
			if ( ATisEqual(t,ATgetArgument(ATAgetFirst(s),0)) )
			{
				return ATgetArgument(ATAgetFirst(s),1);
			}
		}
		ATfprintf(stderr,"%s: variable %t not assigned\n",NAME,t);
		exit(1);
	} else {
		return t;
	}
}

static bool match_inner(ATerm t, ATerm p, ATermList *vars)
{
	bool b;
	ATermList l;

//ATfprintf(stderr,"match_inner(  %t  ,  %t  ,  %t   )\n\n",t,p,*vars);
	if ( ATisList(p) )
	{
		if ( ATisList(t) )
		{
			if ( ATgetLength((ATermList) t) == ATgetLength((ATermList) p) )
			{
				b = true;
				for (; !ATisEmpty((ATermList) t) && b; t=(ATerm) ATgetNext((ATermList) t),p=(ATerm) ATgetNext((ATermList) p))
				{
					b &= match_inner(ATgetFirst((ATermList) t),ATgetFirst((ATermList) p),vars);
				}
				return b;
			}
		}
		return false;
	} else if ( ATisInt(p) || gsIsOpId((ATermAppl) p) )
	{
		return ATisEqual(t,p);
	} else /* if ( gsIsDataVarId((ATermAppl) p) ) */ {
		if ( ATindexOf(*vars,(ATerm) gsMakeSubst(p,t),0) >=0 )
		{
			return true;
		} else {
			// XXX Check type!! (?)
			l = *vars;
			for (; !ATisEmpty(l); l=ATgetNext(l))
			{
				if ( ATisEqual(p,ATgetArgument(ATAgetFirst(l),0)) )
				{
					return false;
				}
			}
			*vars = ATappend(*vars,(ATerm) gsMakeSubst(p,t));
			return true;
		}
	}
}

static ATerm rewrite(ATerm Term, int *b, ATermList vars)
{
	ATerm t;
	ATermList l,l2,m,m2,n,o,o2,p;
//	ATermAppl v,s,t;
	int i,len,c,d;
	bool x;

//ATfprintf(stderr,"rewrite(%t)\n\n",Term);
//ATfprintf(stderr,"rewrite(");gsPrintPart(stderr,fromInner(Term),false,0);ATfprintf(stderr,")\n\n");

	l2 = NULL;

	if ( ATisList(Term) )
	{
		x = true;
		while ( x )
		{
//ATfprintf(stderr,"loop: %t\n\n",Term);
//ATfprintf(stderr,"loop: ");gsPrintPart(stderr,fromInner(Term),false,0);ATfprintf(stderr,"\n\n");
			x = false;
			d = 0;
			t = rewrite(ATgetFirst((ATermList) Term),&d,vars);
			l = ATgetNext((ATermList) Term);
			m = ATmakeList0();
			for (; !ATisEmpty(l); l=ATgetNext(l))
			{
				m = ATinsert(m,rewrite(ATgetFirst(l),&c,vars));
				d |= c;
			}
			l = ATreverse(m);
			if ( d ) 
			{
				*b = 1;
			}
			if ( ATisList(t) )
			{
				l = ATconcat(ATgetNext((ATermList) t),l);
				t = ATgetFirst((ATermList) t);
			}
			if ( !ATisInt(t) || (m = inner3_eqns[ATgetInt((ATermInt) t)]) == NULL )
			{
				Term = (ATerm) ATinsert(l,t);
				break;
			}
			o = ATmakeList0();
			len = ATgetLength(m);
			for (i=0; i<len; i++)
			{
				o = ATinsert(o,(ATerm) ATmakeList0());
			}
//ATfprintf(stderr,"Trying %t...\n\n",Term);
			while ( !ATisEmpty(m) )
			{
//ATfprintf(stderr,"%t matches? %t\n\n",l,m);
				if ( !ATisEmpty(l) )
				{
					t = ATgetFirst(l);
					l2 = ATgetNext(l);
				}
				n = ATmakeList0();
				o2 = ATmakeList0();
				for (; !ATisEmpty(m); m=ATgetNext(m),o=ATgetNext(o))
				{
					p = ATLgetFirst(o);
					if ( ATisEmpty(ATLelementAt(ATLgetFirst(m),2)) )
					{
						if ( gsIsNil(ATAelementAt(ATLgetFirst(m),1)) || ATisEqual(subst_values(p,ATelementAt(ATLgetFirst(m),1)),gsMakeDataExprTrue()) )
						{
//ATfprintf(stderr,"apply %t\n\n",ATgetFirst(m));
							*b = 1;
							Term = subst_values(p,ATelementAt(ATLgetFirst(m),3));
							if ( !ATisEmpty(l) )
							{
								if ( ATisList(Term) )
								{
									Term = (ATerm) ATconcat((ATermList) Term,l);
								} else {
									Term = (ATerm) ATinsert(l,Term);
								}
							}
							if ( ATisList(Term) )
							{
								x = true;
							} else {
								Term = rewrite(Term,&c,vars);
							}
							n = ATmakeList0();
							break;
						}		
					} else if ( !ATisEmpty(l) ) {
						m2 = ATLelementAt(ATLgetFirst(m),2);
						if ( match_inner(t,ATgetFirst(m2),&p) )
						{
							n = ATinsert(n,(ATerm) ATreplace(ATLgetFirst(m),(ATerm) ATgetNext(m2),2));
							o2 = ATinsert(o2,(ATerm) p);
						}
					}
				}
				m = ATreverse(n);
				o = ATreverse(o2);
				if ( !ATisEmpty(l) ) 
				{
					l = l2;
				}
			}
		}
//ATfprintf(stderr,"return: %t\n\n",Term);
//ATfprintf(stderr,"return: ");gsPrintPart(stderr,fromInner(Term),false,0);ATfprintf(stderr,"\n\n");
		return Term;
	} else if ( ATisInt(Term) )
	{
		if ( (m = inner3_eqns[ATgetInt((ATermInt) Term)]) != NULL )
		{
			for (; !ATisEmpty(m); m=ATgetNext(m))
			{
				if ( ATisEmpty(ATLelementAt(ATLgetFirst(m),2)) )
				{
					if ( gsIsNil(ATAelementAt(ATLgetFirst(m),1)) || ATisEqual(ATelementAt(ATLgetFirst(m),1),gsMakeDataExprTrue()) )
					{
						*b = 1;
//ATfprintf(stderr,"return: %t\n\n",ATelementAt(ATLgetFirst(m),3));
//ATfprintf(stderr,"return: ");gsPrintPart(stderr,fromInner(ATelementAt(ATLgetFirst(m),3)),false,0);ATfprintf(stderr,"\n\n");
						return ATelementAt(ATLgetFirst(m),3);
					}		
					
				}
			}
		}
//ATfprintf(stderr,"return: %t\n\n",Term);
//ATfprintf(stderr,"return: ");gsPrintPart(stderr,fromInner(Term),false,0);ATfprintf(stderr,"\n\n");
		return Term;
	} else {
//ATfprintf(stderr,"return: %t\n\n",Term);
//ATfprintf(stderr,"return: ");gsPrintPart(stderr,fromInner(Term),false,0);ATfprintf(stderr,"\n\n");
		return Term;
	}
}

ATerm rewrite_inner3(ATerm Term, int *b)
{
	ATermList l;
	int c;

	if ( ATisList(Term) )
	{
		l = ATmakeList0();
		*b = 0;
		for (; !ATisEmpty((ATermList) Term); Term=(ATerm)ATgetNext((ATermList) Term))
		{
			l = ATinsert(l,(ATerm) fromInner(rewrite(toInner((ATermAppl) ATgetFirst((ATermList) Term),false),&c,ATmakeList0())));
			*b |= c;
		}
		return (ATerm) ATreverse(l);
	}

	return (ATerm) fromInner(rewrite(toInner((ATermAppl) Term,false),b,ATmakeList0()));
}
