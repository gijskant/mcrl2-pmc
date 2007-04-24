#ifndef __REWR_JITTYC_H
#define __REWR_JITTYC_H

//#define _JITTYC_STORE_TREES
#include <utility>
#include <string>
#include "librewrite.h"

class RewriterCompilingJitty: public Rewriter
{
	public:
		RewriterCompilingJitty(lps::data_specification DataSpec);
		~RewriterCompilingJitty();

		RewriteStrategy getStrategy();

		ATermAppl rewrite(ATermAppl Term);

		ATerm toRewriteFormat(ATermAppl Term);
		ATermAppl fromRewriteFormat(ATerm Term);
		ATerm rewriteInternal(ATerm Term);
		ATermList rewriteInternalList(ATermList Terms);

		void setSubstitution(ATermAppl Var, ATerm Expr);
		ATerm getSubstitution(ATermAppl Var);
		void clearSubstitution(ATermAppl Var);
		void clearSubstitutions();

#ifndef NO_DYNLOAD
	private:
		int num_opids;

		int true_num;

		ATermTable term2int;
		ATermAppl *int2term;
		ATermList *jittyc_eqns;

		char *file_c;
		char *file_o;
		char *file_so;

		void (*so_rewr_init)();
		ATermAppl (*so_rewr)(ATermAppl);
		void (*so_set_subst)(ATermAppl, ATerm);
		ATerm (*so_get_subst)(ATermAppl);
		void (*so_clear_subst)(ATermAppl);
		void (*so_clear_substs)();

#ifdef _JITTYC_STORE_TREES
		int write_tree(FILE *f, ATermAppl tree, int *num_states);
		void tree2dot(ATermAppl tree, char *name, char *filename);
		ATermAppl create_tree(ATermList rules, int opid, int arity);
		ATermList create_strategy(ATermList rules, int opid);
#endif

                bool opid_is_nf(ATermInt opid, unsigned int num_args);
                std::pair<unsigned int,std::string> calc_inner_terms(ATermList args, int startarg, ATermList nnfvars, bool rewr);
                std::pair<bool,std::string> calc_inner_term(ATerm t, int startarg, ATermList nnfvars, bool rewr = true);
		void calcTerm(FILE *f, ATerm t, int startarg, ATermList nnfvars, bool rewr = true);
		void implement_tree_aux(FILE *f, ATermAppl tree, int cur_arg, int parent, int level, int cnt, int d, int arity, bool *used, ATermList nnfvars);
		void implement_tree(FILE *f, ATermAppl tree, int arity, int d, int opid, bool *used);
		void implement_strategy(FILE *f, ATermList strat, int arity, int d, int opid, unsigned int nf_args);
		void CompileRewriteSystem(lps::data_specification DataSpec);

		ATerm OpId2Int(ATermAppl Term, bool add_opids);
		ATerm toInner(ATermAppl Term, bool add_opids);
		ATermAppl fromInner(ATerm Term);
#endif
};

#endif
