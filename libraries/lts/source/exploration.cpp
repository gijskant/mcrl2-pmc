// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file exploration.cpp

#include <aterm2.h>
#include <cassert>
#include <time.h>
#include <sstream>
#include <set>
#include <stack>
#include "mcrl2/core/messaging.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/selection.h"
#include "mcrl2/data/nat.h"
#include "mcrl2/atermpp/aterm_access.h"
#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/aterm_list.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/lps/nextstate.h"
#include "mcrl2/trace/trace.h"
#include "mcrl2/lts/exploration.h"
#include "mcrl2/lts/lps2lts_lts.h"


using namespace std;
using namespace mcrl2::core;
using namespace mcrl2::core::detail;
using namespace mcrl2::trace;

namespace mcrl2
{
  namespace lts
  {

    exploration_strategy str_to_expl_strat(const char *s)
    {
      if ( !strcmp(s,"b") || !strcmp(s,"breadth") )
      {
        return es_breadth;
      }
      if ( !strcmp(s,"d") || !strcmp(s,"depth") )
      {
        return es_depth;
      }
      if ( !strcmp(s,"r") || !strcmp(s,"random") )
      {
        return es_random;
      }
      if ( !strcmp(s,"p") || !strcmp(s,"priority") )
      {
        return es_value_prioritized;
      }
      if ( !strcmp(s,"q") || !strcmp(s,"rpriority") )
      {
        return es_value_random_prioritized;
      }
      return es_none;
    }

    const char * expl_strat_to_str(exploration_strategy es)
    {
      switch ( es )
      {
        case es_breadth:
          return "breadth";
        case es_depth:
          return "depth";
        case es_random:
          return "random";
        case es_value_prioritized:
          return "priority";
        case es_value_random_prioritized:
          return "rpriority";
        default:
          return "unknown";
      }
    }

    bool lps2lts_algorithm::initialise_lts_generation(lts_generation_options *opts)
    {
      using namespace mcrl2;

      if(initialised)
      {
        throw mcrl2::runtime_error("lps2lts algorithm class may be instantiated only once.");
      }

      gsVerboseMsg("initialising...\n");

      lgopts = opts;

      lg_error = false;

      if(lgopts->specification == lps::specification())
      {
        throw mcrl2::runtime_error("lps2lts algorithm class instantiated without linear process.");
      }

      lgopts->specification.instantiate_global_variables();

      if ( lgopts->bithashing )
      {
        bithash_table = bit_hash_table(lgopts->bithashsize);
      }
      else
      {
        states = atermpp::indexed_set(lgopts->initial_table_size,50);
      }

      assert( backpointers.empty() );
      if ( lgopts->trace || lgopts->save_error_trace )
      {
        trace_support = true;
        backpointers.push_back(atermpp::aterm());
      } else {
        trace_support = false;
      }

      if (lgopts->removeunused) {
        gsVerboseMsg("removing unused parts of the data specification.\n");

        lgopts->m_rewriter.reset(
          new mcrl2::data::rewriter(lgopts->specification.data(),
        mcrl2::data::used_data_equation_selector(lgopts->specification.data(), mcrl2::lps::specification_to_aterm(lgopts->specification)), lgopts->strat));
      }
      else {
        lgopts->m_rewriter.reset(new mcrl2::data::rewriter(lgopts->specification.data(), lgopts->strat));
      }

      lgopts->m_enumerator_factory.reset(new mcrl2::data::enumerator_factory< mcrl2::data::classic_enumerator< > >(lgopts->specification.data(), *(lgopts->m_rewriter)));

      nstate = createNextState(lgopts->specification,*(lgopts->m_enumerator_factory),!lgopts->usedummies,lgopts->stateformat);

      if ( lgopts->priority_action != "" )
      {
        gsVerboseMsg("applying confluence reduction with tau action '%s'...\n",lgopts->priority_action.c_str());
        nstate->prioritise(lgopts->priority_action.c_str());
        initialise_representation(true);
      }
      else
      {
        initialise_representation(false);
      }

      if ( lgopts->detect_deadlock && gsVerbose)
      { cerr << "Detect deadlocks.\n" ;
      }

      if ( lgopts->detect_divergence && gsVerbose)
      { cerr << "Detect divergences with tau action is `" << lgopts->priority_action << "'.\n";
      }

      num_states = 0;
      trans = 0;
      level = 1;

      if ( lgopts->lts != "" )
      {
        lps2lts_lts_options lts_opts;
        lts_opts.outformat = lgopts->outformat;
        lts_opts.outinfo = lgopts->outinfo;
        lts_opts.nstate = nstate;
        lts_opts.spec.reset(new mcrl2::lps::specification(lgopts->specification));
        lts.open_lts(lgopts->lts.c_str(),lts_opts);
      } else {
        lgopts->outformat = mcrl2::lts::lts_none;
        gsVerboseMsg("not saving state space.\n");
      }

      initialised = true;
      return true;
    }

    bool lps2lts_algorithm::finalise_lts_generation()
    {
      if ( lg_error )
      {
        lts.remove_lts();
      } else {
        lts.close_lts(num_states,trans);
      }

      if ( !lg_error && gsVerbose )
      {
        if ( lgopts->expl_strat == es_random )
        {
          gsVerboseMsg(
            "done with random walk of %llu transition%s (visited %llu unique state%s).\n",
            trans,
            (trans==1)?"":"s",
            num_states,
            (num_states==1)?"":"s"
          );
        } else if ( lgopts->expl_strat == es_value_prioritized )
        {
          gsVerboseMsg(
            "done with value prioritized walk of %llu transition%s (visited %llu unique state%s).\n",
            trans,
            (trans==1)?"":"s",
            num_states,
            (num_states==1)?"":"s"
          );
        } else if ( lgopts->expl_strat == es_value_random_prioritized )
        {
          gsVerboseMsg(
            "done with random value prioritized walk of %llu transition%s (visited %llu unique state%s).\n",
            trans,
            (trans==1)?"":"s",
            num_states,
            (num_states==1)?"":"s"
          );
        } else if ( lgopts->expl_strat == es_breadth )
        {
          gsVerboseMsg(
            "done with state space generation (%lu level%s, %llu state%s and %llu transition%s).\n",
            level-1,
            (level==2)?"":"s",
            num_states,
            (num_states==1)?"":"s",
            trans,
            (trans==1)?"":"s"
          );
        } else if ( lgopts->expl_strat == es_depth )
        {
          gsVerboseMsg(
            "done with state space generation (%llu state%s and %llu transition%s).\n",
            num_states,
            (num_states==1)?"":"s",
            trans,
            (trans==1)?"":"s"
          );
        }
      }

      delete nstate;
      backpointers.clear();

      cleanup_representation();

      finalised = true;
      return true;
    }

    ////////////////////////////////////////////////////////////////////////////////
    //                              Trace functions                               //
    ////////////////////////////////////////////////////////////////////////////////

    bool lps2lts_algorithm::occurs_in(atermpp::aterm_appl const& name, atermpp::term_list< atermpp::aterm_appl > const& ma)
    {
      for (atermpp::term_list< atermpp::aterm_appl >::const_iterator i = ma.begin(); i != ma.end(); ++i)
      {
        if (name == atermpp::aterm_appl((*i)(0))(0))
        {
          return true;
        }
      }
      return false;
    }

    bool lps2lts_algorithm::savetrace(std::string const &info, ATerm state, NextState *nstate, ATerm extra_state, ATermAppl extra_transition)
    {
      ATerm s = state;
      ATerm ns;
      ATermList tr = ATmakeList0();
      NextStateGenerator *nsgen = NULL;

      if ( extra_state != NULL )
      {
        tr = ATinsert(tr,(ATerm) ATmakeList2((ATerm) extra_transition,extra_state));
      }
      while ( (ns = backpointers[states.index(s)]) != atermpp::aterm() )
      {
        ATermAppl trans;
        ATerm t;
        bool priority;
        nsgen = nstate->getNextStates(ns,nsgen);
        try {
        while ( nsgen->next(&trans,&t,&priority) )
        {
          if ( !priority && ATisEqual(s,get_repr(t)) )
          {
            break;
          }
        }
        } catch (mcrl2::runtime_error e)
        { delete nsgen;
          throw e;
        }
        tr = ATinsert(tr, (ATerm) ATmakeList2((ATerm) trans,s));
        s = ns;
      }

      Trace trace;
      trace.setState(nstate->makeStateVector(s));
      for (; !ATisEmpty(tr); tr=ATgetNext(tr))
      {
        ATermList e = (ATermList) ATgetFirst(tr);
        trace.addAction((ATermAppl) ATgetFirst(e));
        e = ATgetNext(e);
        trace.setState(nstate->makeStateVector(ATgetFirst(e)));
      }

      try
      {
        trace.save(lgopts->generate_filename_for_trace(lgopts->trace_prefix, info, "trc"));
      } catch ( ... )
      {
        return false;
      }

      return true;
    }

    void lps2lts_algorithm::check_actiontrace(ATerm OldState, ATermAppl Transition, ATerm NewState)
    {
      // if ( lgopts->detect_action )
      for (int j=0; j<lgopts->num_trace_actions; j++)
      {
        if ( occurs_in(lgopts->trace_actions[j],atermpp::list_arg1(Transition)) )
        {
          if ( lgopts->trace && (tracecnt < lgopts->max_traces) )
          {
            if ( lgopts->trace_prefix.empty() )
            {
            }
            std::ostringstream ss;
            ss << "act_" << tracecnt << "_" << ATgetName(ATgetAFun(lgopts->trace_actions[j]));
            string sss(ss.str());
            bool saved_ok = savetrace(sss,OldState,nstate,NewState,Transition);

            if ( lgopts->detect_action || gsVerbose )
            {
              if ( saved_ok )
              {
                gsMessage("detect: action '%P' found and saved to '%s_act_%lu_%P.trc' (state index: %lu).\n",
                               Transition,
                               const_cast< char* >(lgopts->trace_prefix.c_str()),
                               tracecnt,
                               lgopts->trace_actions[j],
                               states.index(OldState));
              }
              else
              {
                gsMessage("detect: action '%P' found, but could not be saved to '%s_act_%lu_%P.trc' (state index: %lu).\n",
                               Transition,
                               const_cast< char* >(lgopts->trace_prefix.c_str()),
                               tracecnt,
                               lgopts->trace_actions[j],
                               states.index(OldState));
              }
            }
            tracecnt++;
          } 
          else 
          {
            gsMessage("detect: action '%P' found (state index: %lu).\n",
                               Transition,
                               states.index(OldState));
          }
        }
      }
    }


    void lps2lts_algorithm::save_error_trace(ATerm state)
    {
      if ( lgopts->save_error_trace )
      {
        bool saved_ok = savetrace("error",state,nstate);

        if ( saved_ok )
        {
          lgopts->error_trace_saved = true;
          if (gsVerbose)
          { cerr << "saved trace to error in '" << lgopts->trace_prefix << "_error.trc'.\n";
          }
        }
        else
        {
          if (gsVerbose)
          { cerr << "trace to error could not be saved in '" << lgopts->trace_prefix << "_error.trc'.\n";
          }
        }
      }
    }


    void lps2lts_algorithm::check_deadlocktrace(ATerm state)
    {
      if ( lgopts->detect_deadlock )
      {
        if ( lgopts->trace && (tracecnt < lgopts->max_traces) )
        {
          std::ostringstream ss;
          ss << "dlk_" << tracecnt;
          string sss(ss.str());
          bool saved_ok = savetrace(sss,state,nstate);

          if ( lgopts->detect_deadlock || gsVerbose )
          {
            if ( saved_ok )
            {
              cerr << "deadlock-detect: deadlock found and saved to '" << lgopts->trace_prefix << "_dlk_" << tracecnt << ".trc' (state index: " <<
                       states.index(state) << ").\n";
            }
            else
            {
              cerr << "deadlock-detect: deadlock found, but could not be saved to '" << lgopts->trace_prefix << "_dlk_" << tracecnt <<
                      ".trc' (state index: " << states.index(state) <<  ").\n";
            }
          }
          tracecnt++;
        } 
        else
        {
          cerr << "deadlock-detect: deadlock found (state index: " << states.index(state) <<  ").\n";
        }
      }
    }



    ////////////////////////////////////////////////////////////////////////////////
    //                         Main exploration functions                         //
    ////////////////////////////////////////////////////////////////////////////////

    // Confluence reduction based on S.C.C. Blom, Partial tau-confluence for
    // Efficient State Space Generation, Technical Report SEN-R0123, CWI,
    // Amsterdam, 2001

    void lps2lts_algorithm::initialise_representation(bool confluence_reduction)
    {
      apply_confluence_reduction = confluence_reduction;
      if ( confluence_reduction )
      {
        repr_visited = ATindexedSetCreate(1000,50);
        repr_number = ATtableCreate(1000,50);
        repr_low = ATtableCreate(1000,50);
        repr_next = ATtableCreate(1000,50);
        repr_back = ATtableCreate(1000,50);
        repr_nsgen = NULL;
      }
      else if ( lgopts->detect_divergence )
      { repr_visited = ATindexedSetCreate(1000,50);
        lgopts->priority_action = "tau";
      }
    }

    void lps2lts_algorithm::cleanup_representation()
    {
      if ( apply_confluence_reduction )
      {
        delete repr_nsgen;
        ATtableDestroy(repr_back);
        ATtableDestroy(repr_next);
        ATtableDestroy(repr_low);
        ATtableDestroy(repr_number);
        ATindexedSetDestroy(repr_visited);
      }
    }

    bool lps2lts_algorithm::search_divergence_recursively(
                    const long current_state_index,
                    set < long > &on_current_depth_first_path)
    {
      ATerm state=ATindexedSetGetElem(repr_visited,current_state_index);
      on_current_depth_first_path.insert(current_state_index);
      vector < long > new_states;
      repr_nsgen = nstate->getNextStates(state,repr_nsgen);
      ATermAppl Transition;
      ATerm NewState;
      while ( repr_nsgen->next(&Transition,&NewState))
      {
        if ((ATermList)ATgetArgument(Transition,0)==ATempty) // This is a tau transition.
        { ATbool b;
          long n=ATindexedSetPut(repr_visited,NewState,&b);
          if ( b == ATtrue )
          {
            new_states.push_back(n);
          }
          else
          { if (on_current_depth_first_path.find(n)!=on_current_depth_first_path.end())
            { // divergence found
              return true;
            }
          }
        }
      }

      for(vector <long>::const_iterator i=new_states.begin();
                i!=new_states.end(); ++i)
      { if (search_divergence_recursively(*i,on_current_depth_first_path))
        { return true;
        }
      }
      on_current_depth_first_path.erase(current_state_index);
      return false;
    }

    void lps2lts_algorithm::check_divergence(ATerm state)
    { 
      if ( lgopts->detect_divergence )
      {
        ATindexedSetReset(repr_visited);
        set < long > on_current_depth_first_path;
        ATbool b;
        ATindexedSetPut(repr_visited,state,&b);

        if (search_divergence_recursively(0,on_current_depth_first_path))
        {
          if ( lgopts->trace && (tracecnt < lgopts->max_traces) )
          {
            std::ostringstream ss;
            ss << "divergence_" << tracecnt;
            string sss(ss.str());
            bool saved_ok = savetrace(sss,state,nstate);

            if ( lgopts->detect_divergence || gsVerbose )
            {
              if ( saved_ok )
              {
                cerr << "divergence-detect: divergence found and saved to '" << lgopts->trace_prefix << "_dlk_" << tracecnt <<
                        ".trc' (state index: " << states.index(state) <<  ").\n";
              }
              else
              {
                cerr << "divergence-detect: divergence found, but could not be saved to '" << lgopts->trace_prefix << "_dlk_" << tracecnt <<
                        ".trc' (state index: " << states.index(state) <<  ").\n";
              }
            }
            tracecnt++;
          } 
          else
          {
            cerr << "divergence-detect: divergence found (state index: " << states.index(state) <<  ").\n";
          }
        }
      }
    }

    ATerm lps2lts_algorithm::get_repr(ATerm state)
    {
      if ( !apply_confluence_reduction )
      {
        return state;
      }

      ATerm v = state;
      ATindexedSetReset(repr_visited);
      int num_visited = 0;
      ATtableReset(repr_number);
      ATtableReset(repr_low);
      ATtableReset(repr_next);
      ATtableReset(repr_back);
      int count;

      ATtablePut(repr_number,v,(ATerm) ATmakeInt(0));
      count = 0;
      bool notdone = true;
      while ( notdone )
      { 
        if ( ATgetInt((ATermInt) ATtableGet(repr_number,v)) == 0 )
        {
          count++;
          ATtablePut(repr_number,v,(ATerm) ATmakeInt(count));
          ATtablePut(repr_low,v,(ATerm) ATmakeInt(count));
          ATermList nextl = ATmakeList0();
          repr_nsgen = nstate->getNextStates(v,repr_nsgen);
          ATermAppl Transition;
          ATerm NewState;
          bool prioritised_action;
          while ( repr_nsgen->next(&Transition,&NewState,&prioritised_action) && prioritised_action )
          {
            ATbool b;
            ATindexedSetPut(repr_visited,NewState,&b);
            if ( b == ATtrue )
            {
              num_visited++;
            }
            nextl = ATinsert(nextl,NewState);
            if ( ATtableGet(repr_number,NewState) == NULL ) // This condition was missing in the report
            {
              ATtablePut(repr_number,NewState,(ATerm) ATmakeInt(0));
            }
          }
          if ( !notdone )
          {
            break;
          }
          ATtablePut(repr_next,v,(ATerm) nextl);
        }
        ATermList nextl = (ATermList) ATtableGet(repr_next,v);
        if ( ATisEmpty(nextl) )
        {
          if ( ATisEqual(ATtableGet(repr_number,v),ATtableGet(repr_low,v)) )
          {
            break;
          }
          ATerm backv = ATtableGet(repr_back,v);
          int a = ATgetInt((ATermInt) ATtableGet(repr_low,backv));
          int b = ATgetInt((ATermInt) ATtableGet(repr_low,v));
          if ( a < b )
          {
            ATtablePut(repr_low,backv,(ATerm) ATmakeInt(a));
          } else {
            ATtablePut(repr_low,backv,(ATerm) ATmakeInt(b));
          }
          v = backv;
        }
        else
        {
          ATerm u = ATgetFirst(nextl);
          ATtablePut(repr_next,v,(ATerm) ATgetNext(nextl));
          int nu = ATgetInt((ATermInt) ATtableGet(repr_number,u));
          if ( nu == 0 )
          {
            ATtablePut(repr_back,u,v);
            v = u;
          }
          else
          {
            if ( nu < ATgetInt((ATermInt) ATtableGet(repr_number,v)) )
            {
              int lv = ATgetInt((ATermInt) ATtableGet(repr_low,v));
              if ( nu < lv )
              {
                ATtablePut(repr_low,v,(ATerm) ATmakeInt(nu));
              }
            }
          }
        }
      }

      return v;
    }

    boost::uint64_t lps2lts_algorithm::add_state(ATerm state, bool &is_new)
    {
      if ( lgopts->bithashing )
      {
        boost::uint64_t i = bithash_table.add_state(state, is_new);
        return i;
      }
      else
      {
        long i;
        std::pair<long, bool> result = states.put(state);
        is_new = result.second;
        return result.first;
      }
    }

    boost::uint64_t lps2lts_algorithm::state_index(ATerm state)
    {
      if ( lgopts->bithashing )
      {
        return bithash_table.state_index(state);
      } else {
        return states.index(state);
      }
    }

    bool lps2lts_algorithm::add_transition(ATerm from, ATermAppl action, ATerm to)
    {
      bool new_state;
      boost::uint64_t i;

      i = add_state(to, new_state);

      if ( new_state )
      {
        num_states++;
        if ( trace_support )
        {
          backpointers.push_back(from);
        }
      } else {
        num_found_same++;
      }

      check_actiontrace(from,action,to);

      lts.save_transition(state_index(from),from,action,i,to);
      trans++;

      return new_state;
    }

    bool lps2lts_algorithm::generate_lts()
    {
      ATerm state = get_repr(nstate->getInitialState());
      lts.save_initial_state(initial_state,state);

      bool new_state;
      initial_state = add_state(state,new_state);
      current_state = 0;
      ++num_states;

      lgopts->display_status(level,current_state,num_states, static_cast < boost::uint64_t > (0),trans);

      if ( lgopts->max_states != 0 )
      {
        boost::uint64_t endoflevelat = 1;
        boost::uint64_t prevtrans = 0;
        boost::uint64_t prevcurrent = 0;
        num_found_same = 0;
        tracecnt = 0;
        gsVerboseMsg("generating state space with '%s' strategy...\n",expl_strat_to_str(lgopts->expl_strat));

        if ( lgopts->expl_strat == es_random )
        {
          srand((unsigned)time(NULL)+getpid());
          NextStateGenerator *nsgen = NULL;
          while (( current_state < lgopts->max_states ) && ( !lgopts->trace || (tracecnt < lgopts->max_traces) ))
          {
            ATermList tmp_trans = ATmakeList0();
            ATermList tmp_states = ATmakeList0();
            ATermAppl Transition;
            ATerm NewState;

            check_divergence(state);

            try
            { nsgen = nstate->getNextStates(state,nsgen);
              bool priority;
              while ( nsgen->next(&Transition,&NewState,&priority) )
              {
                NewState = get_repr(NewState);
                if ( !priority ) // don't store confluent self loops
                { tmp_trans = ATinsert(tmp_trans,(ATerm) Transition);
                  tmp_states = ATinsert(tmp_states,NewState);
                }
              }
            }
            catch (mcrl2::runtime_error &e)
            { cerr << "Error while exploring state space: " << e.what() << "\n";
              lg_error = true;
              save_error_trace(state);
              break;
            }

            int len = ATgetLength(tmp_trans);
            if ( len > 0 )
            {
              int r = rand()%len;
              ATerm new_state = NULL;
              for (int i=0; i<len; i++)
              {
                add_transition(state,(ATermAppl) ATgetFirst(tmp_trans),ATgetFirst(tmp_states));
                if ( r == i )
                {
                  new_state = ATgetFirst(tmp_states);
                }
                tmp_trans = ATgetNext(tmp_trans);
                tmp_states = ATgetNext(tmp_states);
              }
              state = new_state;
            }
            else
            {
              check_deadlocktrace(state);
              break;
            }

            current_state++;
            if ( (current_state%200) == 0 ) {
              lgopts->display_status(level,current_state,num_states,num_found_same,trans);
            }
            if ( !lgopts->suppress_progress_messages && gsVerbose && ((current_state%1000) == 0) )
            {
              gsVerboseMsg(
                "monitor: currently explored %llu transition%s and encountered %llu unique state%s.\n",
                trans,
                (trans==1)?"":"s",
                num_states,
                (num_states==1)?"":"s"
              );
            }
          }
          lgopts->display_status(level-1,num_states,num_states,num_found_same,trans);
          delete nsgen;
        } 
        else if ( lgopts->expl_strat == es_value_prioritized )
        {
          mcrl2::data::rewriter& rewriter=nstate->getRewriter();
          NextStateGenerator *nsgen = NULL;
          srand((unsigned)time(NULL)+getpid());
          while (( current_state < num_states ) && ( !lgopts->trace || (tracecnt < lgopts->max_traces) ))
          // while ( current_state < lgopts->max_states )
          {
            ATermList tmp_trans = ATmakeList0();
            ATermList tmp_states = ATmakeList0();
            ATermAppl Transition;
            ATerm NewState;
            state = states.get(current_state);
            check_divergence(state);
            try
            {
              nsgen = nstate->getNextStates(state,nsgen);
              bool priority;
              while ( nsgen->next(&Transition,&NewState,&priority) )
              {
                NewState = get_repr(NewState);
                if ( !priority ) // don't store confluent self loops
                {
                  tmp_trans = ATinsert(tmp_trans,(ATerm) Transition);
                  tmp_states = ATinsert(tmp_states,NewState);
                }
              }

              // Filter the transitions by only taking the actions that either have no
              // positive number as first parameter, or that has the lowest positive number.
              // This can be non-deterministic, as there can be more actions with this low number
              // as first parameter.
              //
              // First find the lowest index.

              ATermAppl lowest_first_action_parameter=NULL;

              for(ATermList tmp_trans_walker=tmp_trans; !ATisEmpty(tmp_trans_walker);
                         tmp_trans_walker=ATgetNext(tmp_trans_walker))
              { ATermList multi_action_list=(ATermList)ATgetArgument(ATgetFirst(tmp_trans_walker),0);
                if (ATgetLength(multi_action_list)==1)
                { ATermAppl first_action=(ATermAppl)ATgetFirst(multi_action_list);
                  ATermList action_arguments=(ATermList)ATgetArgument(first_action,1);
                  ATermList action_sorts=(ATermList)ATgetArgument(ATgetArgument(first_action,0),1);
                  if (ATgetLength(action_arguments)>0)
                  { ATermAppl first_argument=(ATermAppl)ATgetFirst(action_arguments);
                    ATermAppl first_sort=(ATermAppl)ATgetFirst(action_sorts);
                    if (mcrl2::data::sort_nat::is_nat(mcrl2::data::sort_expression(first_sort)))
                    {
                      if (lowest_first_action_parameter==NULL)
                      { lowest_first_action_parameter=first_argument;
                      }
                      else
                      { ATermAppl result=rewriter(mcrl2::data::greater(mcrl2::data::data_expression(lowest_first_action_parameter),mcrl2::data::data_expression(first_argument)));
                        if (mcrl2::data::sort_bool::is_true_function_symbol(mcrl2::data::data_expression(result)))
                        { lowest_first_action_parameter=first_argument;
                        }
                        else if (!mcrl2::data::sort_bool::is_false_function_symbol(mcrl2::data::data_expression(result)))
                        { assert(0);
                        }
                      }
                    }
                  }
                }
              }

              // Now carry out the actual filtering;
              ATermList new_tmp_trans = ATmakeList0();
              ATermList new_tmp_states = ATmakeList0();
              ATermList tmp_state_walker = tmp_states;
              for(ATermList tmp_trans_walker=tmp_trans; !ATisEmpty(tmp_trans_walker);
                         tmp_trans_walker=ATgetNext(tmp_trans_walker))
              { ATermAppl multi_action=(ATermAppl)ATgetFirst(tmp_trans_walker);
                ATermAppl state=(ATermAppl)ATgetFirst(tmp_state_walker);
                tmp_state_walker=ATgetNext(tmp_state_walker);
                ATermList multi_action_list=(ATermList)ATgetArgument(ATgetFirst(tmp_trans_walker),0);
                if (ATgetLength(multi_action_list)==1)
                { ATermAppl first_action=(ATermAppl)ATgetFirst(multi_action_list);
                  ATermList action_arguments=(ATermList)ATgetArgument(first_action,1);
                  ATermList action_sorts=(ATermList)ATgetArgument(ATgetArgument(first_action,0),1);
                  if (ATgetLength(action_arguments)>0)
                  { ATermAppl first_argument=(ATermAppl)ATgetFirst(action_arguments);
                    ATermAppl first_sort=(ATermAppl)ATgetFirst(action_sorts);
                    if (mcrl2::data::sort_nat::is_nat(mcrl2::data::sort_expression(first_sort)))
                    { ATermAppl result=rewriter(mcrl2::data::equal_to(mcrl2::data::data_expression(lowest_first_action_parameter),mcrl2::data::data_expression(first_argument)));
                      if (mcrl2::data::sort_bool::is_true_function_symbol(mcrl2::data::data_expression(result)))
                      { new_tmp_trans=ATinsert(new_tmp_trans,(ATerm)multi_action);
                        new_tmp_states=ATinsert(new_tmp_states,(ATerm)state);
                      }
                      else
                      { assert(mcrl2::data::sort_bool::is_false_function_symbol(mcrl2::data::data_expression(result)));
                        // The transition is omitted!
                      }
                    }
                    else
                    {
                      new_tmp_trans=ATinsert(new_tmp_trans,(ATerm)multi_action);
                      new_tmp_states=ATinsert(new_tmp_states,(ATerm)state);
                    }
                  }
                  else
                  {
                    new_tmp_trans=ATinsert(new_tmp_trans,(ATerm)multi_action);
                    new_tmp_states=ATinsert(new_tmp_states,(ATerm)state);
                  }
                }
                else
                {
                  new_tmp_trans=ATinsert(new_tmp_trans,(ATerm)multi_action);
                  new_tmp_states=ATinsert(new_tmp_states,(ATerm)state);
                }
              }
              tmp_trans=ATreverse(new_tmp_trans);
              tmp_states=ATreverse(new_tmp_states);
            }
            catch (mcrl2::runtime_error &e)
            { cerr << "Error while exploring state space: " << e.what() << "\n";
              lg_error = true;
              save_error_trace(state);
              break;
            }

            int len = ATgetLength(tmp_trans);
            if ( len > 0 )
            {
              // ATerm new_state = NULL;
              for (int i=0; i<len; i++)
              {
                if (num_states-current_state <= lgopts->todo_max)
                { add_transition(state,(ATermAppl) ATgetFirst(tmp_trans),ATgetFirst(tmp_states));
                }
                else if (rand()%2==0)  // with 50 % probability
                { current_state++;    // ignore the current state
                  add_transition(state,(ATermAppl) ATgetFirst(tmp_trans),ATgetFirst(tmp_states));
                }
                else
                { // Ignore the new state.
                }

                tmp_trans = ATgetNext(tmp_trans);
                tmp_states = ATgetNext(tmp_states);
              }
            } else {
              check_deadlocktrace(state);
              break;
            }

            current_state++;
            if ( (current_state%200) == 0 ) {
              lgopts->display_status(level,current_state,num_states,num_found_same,trans);
            }
            if ( !lgopts->suppress_progress_messages && gsVerbose && ((current_state%1000) == 0) )
            {
              gsVerboseMsg(
                "monitor: currently explored %llu transition%s and encountered %llu unique state%s [MAX %d].\n",
                trans,
                (trans==1)?"":"s",
                num_states,
                (num_states==1)?"":"s",
                lgopts->todo_max
              );
            }
          }
          lgopts->display_status(level-1,num_states,num_states,num_found_same,trans);
          delete nsgen;
        }
        else if ( lgopts->expl_strat == es_value_random_prioritized )
        {
          srand((unsigned)time(NULL)+getpid());
          mcrl2::data::rewriter& rewriter=nstate->getRewriter();
          NextStateGenerator *nsgen = NULL;
          while (( current_state < lgopts->max_states ) && ( !lgopts->trace || (tracecnt < lgopts->max_traces) ))
          {
            ATermList tmp_trans = ATmakeList0();
            ATermList tmp_states = ATmakeList0();
            ATermAppl Transition;
            ATerm NewState;

            check_divergence(state);

            try
            {
              // state = ATindexedSetGetElem(states,current_state);
              nsgen = nstate->getNextStates(state,nsgen);
              bool priority;
              while ( nsgen->next(&Transition,&NewState,&priority) )
              {
                NewState = get_repr(NewState);
                if ( !priority ) // don't store confluent self loops
                {
                  tmp_trans = ATinsert(tmp_trans,(ATerm) Transition);
                  tmp_states = ATinsert(tmp_states,NewState);
                }
              }

              // Filter the transitions by only taking the actions that either have no
              // positive number as first parameter, or that has the lowest positive number.
              // This can be non-deterministic, as there can be more actions with this low number
              // as first parameter.
              //
              // First find the lowest index.

              ATermAppl lowest_first_action_parameter=NULL;

              for(ATermList tmp_trans_walker=tmp_trans; !ATisEmpty(tmp_trans_walker);
                         tmp_trans_walker=ATgetNext(tmp_trans_walker))
              { ATermList multi_action_list=(ATermList)ATgetArgument(ATgetFirst(tmp_trans_walker),0);
                if (ATgetLength(multi_action_list)==1)
                { ATermAppl first_action=(ATermAppl)ATgetFirst(multi_action_list);
                  ATermList action_arguments=(ATermList)ATgetArgument(first_action,1);
                  ATermList action_sorts=(ATermList)ATgetArgument(ATgetArgument(first_action,0),1);
                  if (ATgetLength(action_arguments)>0)
                  { ATermAppl first_argument=(ATermAppl)ATgetFirst(action_arguments);
                    ATermAppl first_sort=(ATermAppl)ATgetFirst(action_sorts);
                    if (mcrl2::data::sort_nat::is_nat(mcrl2::data::sort_expression(first_sort)))
                    {
                      if (lowest_first_action_parameter==NULL)
                      { lowest_first_action_parameter=first_argument;
                      }
                      else
                      {
                        ATermAppl result=rewriter(mcrl2::data::greater(mcrl2::data::data_expression(lowest_first_action_parameter),mcrl2::data::data_expression(first_argument)));
                        if (mcrl2::data::sort_bool::is_true_function_symbol(mcrl2::data::data_expression(result)))
                        { lowest_first_action_parameter=first_argument;
                        }
                        else if (!mcrl2::data::sort_bool::is_false_function_symbol(mcrl2::data::data_expression(result)))
                        { assert(0);
                        }
                      }
                    }
                  }
                }
              }

              // Now carry out the actual filtering;
              ATermList new_tmp_trans = ATmakeList0();
              ATermList new_tmp_states = ATmakeList0();
              ATermList tmp_state_walker = tmp_states;
              for(ATermList tmp_trans_walker=tmp_trans; !ATisEmpty(tmp_trans_walker);
                         tmp_trans_walker=ATgetNext(tmp_trans_walker))
              { ATermAppl multi_action=(ATermAppl)ATgetFirst(tmp_trans_walker);
                ATermAppl state=(ATermAppl)ATgetFirst(tmp_state_walker);
                tmp_state_walker=ATgetNext(tmp_state_walker);
                ATermList multi_action_list=(ATermList)ATgetArgument(ATgetFirst(tmp_trans_walker),0);
                if (ATgetLength(multi_action_list)==1)
                { ATermAppl first_action=(ATermAppl)ATgetFirst(multi_action_list);
                  ATermList action_arguments=(ATermList)ATgetArgument(first_action,1);
                  ATermList action_sorts=(ATermList)ATgetArgument(ATgetArgument(first_action,0),1);
                  if (ATgetLength(action_arguments)>0)
                  { ATermAppl first_argument=(ATermAppl)ATgetFirst(action_arguments);
                    ATermAppl first_sort=(ATermAppl)ATgetFirst(action_sorts);
                    if (mcrl2::data::sort_nat::is_nat(mcrl2::data::sort_expression(first_sort)))
                    { ATermAppl result=rewriter(mcrl2::data::equal_to(mcrl2::data::data_expression(lowest_first_action_parameter),mcrl2::data::data_expression(first_argument)));
                      if (mcrl2::data::sort_bool::is_true_function_symbol(mcrl2::data::data_expression(result)))
                      { new_tmp_trans=ATinsert(new_tmp_trans,(ATerm)multi_action);
                        new_tmp_states=ATinsert(new_tmp_states,(ATerm)state);
                      }
                      else
                      {
                        assert(mcrl2::data::sort_bool::is_false_function_symbol(mcrl2::data::data_expression(result)));
                      }
                    }
                    else
                    {
                      new_tmp_trans=ATinsert(new_tmp_trans,(ATerm)multi_action);
                      new_tmp_states=ATinsert(new_tmp_states,(ATerm)state);
                    }
                  }
                  else
                  {
                    new_tmp_trans=ATinsert(new_tmp_trans,(ATerm)multi_action);
                    new_tmp_states=ATinsert(new_tmp_states,(ATerm)state);
                  }
                }
                else
                {
                  new_tmp_trans=ATinsert(new_tmp_trans,(ATerm)multi_action);
                  new_tmp_states=ATinsert(new_tmp_states,(ATerm)state);
                }
              }

              // Randomly select one element from the list for experiments.
              if (ATgetLength(new_tmp_trans)>0)
              { int r = rand()%ATgetLength(new_tmp_trans);
                tmp_trans=ATgetSlice(new_tmp_trans,r,r+1);
                tmp_states=ATgetSlice(new_tmp_states,r,r+1);
              }
              else
              { tmp_trans=ATempty;
                tmp_states=ATempty;
              }
            }
            catch (mcrl2::runtime_error &e)
            { cerr << "Error while exploring state space: " << e.what() << "\n";
              lg_error = true;
              save_error_trace(state);
              break;
            }

            int len = ATgetLength(tmp_trans);
            if ( len > 0 )
            {
              ATerm new_state = NULL;
              for (int i=0; i<len; i++)
              {
                add_transition(state,(ATermAppl) ATgetFirst(tmp_trans),ATgetFirst(tmp_states));
                new_state = ATgetFirst(tmp_states);

                tmp_trans = ATgetNext(tmp_trans);
                tmp_states = ATgetNext(tmp_states);
              }
              state = new_state;
            } else {
              check_deadlocktrace(state);
              break;
            }

            current_state++;
            if ( (current_state%200) == 0 ) {
              lgopts->display_status(level,current_state,num_states,num_found_same,trans);
            }
            if ( !lgopts->suppress_progress_messages && gsVerbose && ((current_state%1000) == 0) )
            {
              gsVerboseMsg(
                "monitor: currently explored %llu transition%s and encountered %llu unique state%s.\n",
                trans,
                (trans==1)?"":"s",
                num_states,
                (num_states==1)?"":"s"
              );
            }
          }
          lgopts->display_status(level-1,num_states,num_states,num_found_same,trans);
          delete nsgen;
        }
        else if ( lgopts->expl_strat == es_breadth )
        {
          queue state_queue;

          NextStateGenerator *nsgen = NULL;
          boost::uint64_t limit = lgopts->max_states;
          if ( lgopts->bithashing )
          {
            lgopts->max_states = ULLONG_MAX;
            state_queue.set_max_size(((limit-1)>lgopts->todo_max)?lgopts->todo_max:limit-1);
            srand((unsigned)time(NULL)+getpid());
            state_queue.add_to_queue(state);
            state_queue.swap_queues();
          }
          // E is the set of explored states
          // S is the set of "seen" states
          //
          // normal:     S = [0..num_states), E = [0..current_state)
          // bithashing: S = { h | get_bithash(h) }, E = S \ "items left in queues"
          //
          // both:       |E| <= limit
          while (( current_state < endoflevelat ) && ( !lgopts->trace || (tracecnt < lgopts->max_traces) ))
          {
            if ( lgopts->bithashing )
            {
              state = state_queue.get_from_queue();
              assert(state != NULL);
            }
            else
            {
              state = states.get(current_state);
            }

            check_divergence(state);

            bool deadlockstate = true;

            try
            {
              nsgen = nstate->getNextStates(state,nsgen);
              ATermAppl Transition;
              ATerm NewState;
              bool priority;
              while ( nsgen->next(&Transition,&NewState,&priority) )
              {
                NewState = get_repr(NewState);
                if ( !priority ) // don't store confluent self loops
                {
                  deadlockstate = false;
                  bool b = add_transition(state,Transition,NewState);
                  if ( lgopts->bithashing && b )
                  {
                    ATerm removed_state = state_queue.add_to_queue(NewState);
                    if ( removed_state != NULL )
                    {
                      bithash_table.remove_state_from_bithash(removed_state);
                      num_states--;
                    }
                  }
                }
              }
            }
            catch (mcrl2::runtime_error &e)
            { cerr << "Error while exploring state space: " << e.what() << "\n";
              lg_error = true;
              save_error_trace(state);
              break;
            }

            if ( deadlockstate )
            {
              check_deadlocktrace(state);
            }

            current_state++;
            if ( (current_state%200) == 0 ) {
              lgopts->display_status(level,current_state,num_states,num_found_same,trans);
            }
            if ( !lgopts->suppress_progress_messages && gsVerbose && ((current_state%1000) == 0) )
            {
              gsVerboseMsg(
                "monitor: currently at level %lu with %llu state%s and %llu transition%s explored and %llu state%s seen.\n",
                level,
                current_state,
                (current_state==1)?"":"s",
                trans,
                (trans==1)?"":"s",
                num_states,
                (num_states==1)?"":"s"
              );
            }
            if ( current_state == endoflevelat )
            {
              if ( lgopts->bithashing )
              {
                state_queue.swap_queues();
              }
              lgopts->display_status(level,current_state,num_states,num_found_same,trans);
              if ( !lgopts->suppress_progress_messages && gsVerbose )
              {
                gsVerboseMsg(
                  "monitor: level %lu done. (%llu state%s, %llu transition%s)\n",
                  level,current_state-prevcurrent,
                  ((current_state-prevcurrent)==1)?"":"s",
                  trans-prevtrans,
                  ((trans-prevtrans)==1)?"":"s"
                );
                fflush(stderr);
              }
              level++;
              boost::uint64_t nextcurrent = endoflevelat;
              endoflevelat = (limit>num_states)?num_states:limit;
              if ( lgopts->bithashing )
              {
                if ( (limit - num_states) < state_queue.max_size() )
                {
                  state_queue.set_max_size(limit - num_states);
                }
              }
              current_state = nextcurrent;
              prevcurrent = current_state;
              prevtrans = trans;
            }
          }
          lgopts->display_status(level-1,num_states,num_states,num_found_same,trans);
          delete nsgen;
        }
        else if ( lgopts->expl_strat == es_depth )
        {
          unsigned long nsgens_size = (lgopts->todo_max<128)?lgopts->todo_max:128;
          NextStateGenerator **nsgens = (NextStateGenerator **) malloc(nsgens_size*sizeof(NextStateGenerator *));
          if ( nsgens == NULL )
          {
            throw mcrl2::runtime_error("cannot create state stack");
          }
          nsgens[0] = nstate->getNextStates(state);
          for (unsigned long i=1; i<nsgens_size; i++)
          {
            nsgens[i] = NULL;
          }
          unsigned long nsgens_num = 1;

          bool top_trans_seen = false;
          // trans_seen(s) := we have seen a transition from state s
          // inv:  forall i : 0 <= i < nsgens_num-1 : trans_seen(nsgens[i]->get_state())
          //       nsgens_num > 0  ->  top_trans_seen == trans_seen(nsgens[nsgens_num-1])
          while (( nsgens_num > 0 ) && ( ! lgopts->trace || (tracecnt < lgopts->max_traces) ))
          {
            NextStateGenerator *nsgen = nsgens[nsgens_num-1];
            state = nsgen->get_state();
            check_divergence(state);
            ATermAppl Transition;
            ATerm NewState;
            bool new_state = false;
            bool add_new_states = (current_state < lgopts->max_states);
            bool state_is_deadlock = !top_trans_seen /* && !nsgen->next(...) */ ;
            bool priority;
            try
            {
              if ( nsgen->next(&Transition,&NewState,&priority) )
              {
                NewState = get_repr(NewState);
                if ( !priority ) // don't store confluent self loops
                {
                  top_trans_seen = true;
                  // inv
                  state_is_deadlock = false;
                  if ( add_transition(state,Transition,NewState) )
                  {
                    if ( add_new_states )
                    {
                      new_state = true;
                      if ( (nsgens_num == nsgens_size) && (nsgens_size < lgopts->todo_max) )
                      {
                        nsgens_size = nsgens_size*2;
                        if ( nsgens_size > lgopts->todo_max )
                        {
                          nsgens_size = lgopts->todo_max;
                        }
                        nsgens = (NextStateGenerator **) realloc(nsgens,nsgens_size*sizeof(NextStateGenerator *));
                        if ( nsgens == NULL )
                        {
                          throw mcrl2::runtime_error("cannot enlarge state stack");
                        }
                        for (unsigned long i=nsgens_num; i<nsgens_size; i++)
                        {
                          nsgens[i] = NULL;
                        }
                      }
                      if ( nsgens_num < nsgens_size )
                      {
                        nsgens[nsgens_num] = nstate->getNextStates(NewState,nsgens[nsgens_num]);
                        nsgens_num++;
                        top_trans_seen = false;
                        // inv
                      }
                    }
                  }
                }
              } else {
                nsgens_num--;
                top_trans_seen = true;
                // inv
              }
              // inv

            }
            catch (mcrl2::runtime_error &e)
            { cerr << "Error while exploring state space: " << e.what() << "\n";
              lg_error = true;
              save_error_trace(state);
              break;
            }

            if ( state_is_deadlock )
            {
              check_deadlocktrace(state);
            }

            if ( new_state )
            {
              current_state++;
              if ( (current_state%200) == 0 ) {
                lgopts->display_status(level,current_state,num_states,num_found_same,trans);
              }
              if ( !lgopts->suppress_progress_messages && gsVerbose && ((current_state%1000) == 0) )
              {
                gsVerboseMsg(
                  "monitor: currently explored %llu state%s and %llu transition%s (stacksize is %d).\n",
                  current_state,
                  (current_state==1)?"":"s",
                  trans,
                  (trans==1)?"":"s",
                  nsgens_num
                );
              }
            }
          }
          lgopts->display_status(level-1,num_states,num_states,num_found_same,trans);

          for (unsigned long i=0; i<nsgens_size; i++)
          {
            delete nsgens[i];
          }

          free(nsgens);
        }
        else
        {
          gsErrorMsg("unknown exploration strategy\n");
        }
      }

      completely_generated = true;

      return !lg_error;
    }

  }
}
