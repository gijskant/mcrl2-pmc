// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/action_rename.h
/// \brief Action rename specifications.

#ifndef MCRL2_LPS_ACTION_RENAME_H
#define MCRL2_LPS_ACTION_RENAME_H

#include <sstream>
#include "mcrl2/exception.h"
#include "mcrl2/core/detail/struct.h"
#include "mcrl2/core/print.h"
#include "mcrl2/core/parse.h"
#include "mcrl2/core/typecheck.h"
#include "mcrl2/core/data_implementation.h"
#include "mcrl2/core/data_reconstruct.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/core/aterm_ext.h"
#include "mcrl2/lps/rename.h"
#include "mcrl2/lps/sumelm.h"
#include "mcrl2/data/find.h"
#include "mcrl2/data/rewrite.h"
#include "mcrl2/data/sort_identifier.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/data/data_expression.h"
// #include "mcrl2/utilities/command_line_interface.h"
// #include "mcrl2/utilities/command_line_messaging.h"
// #include "mcrl2/utilities/command_line_rewriting.h"


// //Action rename rules
// <ActionRenameRules>
//                ::= ActionRenameRules(<ActionRenameRule>*)
//
// //Action rename rule
// <ActionRenameRule>
//                ::= ActionRenameRule(<DataVarId>*, <DataExprOrNil>,
//                      <ParamIdOrAction>, <ActionRenameRuleRHS>)
//
// //Right-hand side of an action rename rule
// <ActionRenameRuleRHS>
//                ::= <ParamId>                                             [- tc]
//                  | <Action>                                              [+ tc]
//                  | Delta
//                  | Tau
//
// //Action rename action_rename_specification
// <ActionRenameSpec>
//                ::= ActionRenameSpec(<DataSpec>, <ActSpec>, <ActionRenameRules>)

namespace mcrl2 {

namespace lps {

  /// \brief Right hand side of an action rename rule
  class action_rename_rule_rhs: public atermpp::aterm_appl
  {
    public:
      /// \brief Constructor.
      action_rename_rule_rhs()
        : atermpp::aterm_appl(core::detail::constructActionRenameRuleRHS())
      { }

      /// \brief Constructor.
      action_rename_rule_rhs(atermpp::aterm_appl t)
        : atermpp::aterm_appl(t)
      {
        assert(core::detail::check_rule_ActionRenameRuleRHS(m_term));
      }

      /// \brief Returns true if the right hand side is equal to delta.
      bool is_delta() const
      {
        return core::detail::gsIsDelta(*this);
      }

      /// \brief Returns true if the right hand side is equal to tau.
      bool is_tau() const
      {
        return core::detail::gsIsTau(*this);
      }

      /// \brief Returns the action.
      /// \pre The right hand side must be an action
      action act() const
      {
        return *this;
      }
  };

//                ::= ActionRenameRule(<DataVarId>*, <DataExprOrNil>,
//                      <ParamIdOrAction>, <ActionRenameRuleRHS>)

  /// \brief Action rename rule
  class action_rename_rule: public atermpp::aterm_appl
  {
    protected:
      data::data_variable_list m_variables;
      data::data_expression    m_condition;
      action                   m_lhs;
      action_rename_rule_rhs   m_rhs;

      /// \brief Initialize the action rename rule with an aterm_appl.
      void init_term(atermpp::aterm_appl t)
      {
        m_term = atermpp::aterm_traits<atermpp::aterm_appl>::term(t);
        atermpp::aterm_appl::iterator i = t.begin();
        m_variables       = atermpp::aterm_list(*i++);
        m_condition       = atermpp::aterm_appl(*i++);
        m_lhs             = atermpp::aterm_appl(*i++);
        m_rhs             = atermpp::aterm_appl(*i);
      }

    public:
      /// \brief Constructor.
      action_rename_rule()
        : atermpp::aterm_appl(core::detail::constructActionRenameRule())
      { }

      /// \brief Constructor.
      action_rename_rule(atermpp::aterm_appl t)
        : atermpp::aterm_appl(t)
      {
        assert(core::detail::check_rule_ActionRenameRule(m_term));
        init_term(t);
      }

      /// \brief Returns the variables of the rule.
      data::data_variable_list variables() const
      {
        return m_variables;
      }

      /// \brief Returns the condition of the rule.
      data::data_expression condition() const
      {
        return m_condition;
      }

      /// \brief Returns the left hand side of the rule.
      action lhs() const
      {
        return m_lhs;
      }

      /// \brief Returns the right hand side of the rule.
      action_rename_rule_rhs rhs() const
      {
        return m_rhs;
      }
  };

  /// \brief Read-only singly linked list of action rename rules
  typedef atermpp::term_list<action_rename_rule> action_rename_rule_list;

  /// \brief Action rename specification
  class action_rename_specification: public atermpp::aterm_appl
  {
    protected:
      data::data_specification m_data;
      action_label_list        m_action_labels;
      action_rename_rule_list  m_rules;

      /// \brief Initialize the action_rename_specification with an aterm_appl.
      void init_term(atermpp::aterm_appl t)
      {
        m_term = atermpp::aterm_traits<atermpp::aterm_appl>::term(t);
        atermpp::aterm_appl::iterator i = t.begin();
        m_data            = atermpp::aterm_appl(*i++);
        m_action_labels   = atermpp::aterm_appl(*i++)(0);
        m_rules           = atermpp::aterm_appl(*i)(0);
      }

    public:
      /// \brief Constructor.
      action_rename_specification()
        : atermpp::aterm_appl(core::detail::constructActionRenameSpec())
      { }

      /// \brief Constructor.
      action_rename_specification(atermpp::aterm_appl t)
        : atermpp::aterm_appl(t)
      {
        assert(core::detail::check_rule_ActionRenameSpec(m_term));
        init_term(t);
      }

      /// \brief Constructor.
      action_rename_specification(
          data::data_specification  data,
          action_label_list         action_labels,
          action_rename_rule_list   rules
         )
        :
          m_data(data),
          m_action_labels(action_labels),
          m_rules(rules)
      {
        m_term = reinterpret_cast<ATerm>(
          core::detail::gsMakeActionRenameSpec(
            data,
            core::detail::gsMakeActSpec(action_labels),
            core::detail::gsMakeActionRenameRules(rules)
          )
        );
      }

      /// \brief Reads the action rename specification from file.
      /// \param[in] filename
      /// If filename is nonempty, input is read from the file named filename.
      /// If filename is empty, input is read from stdin.
      void load(const std::string& filename)
      {
        atermpp::aterm t = core::detail::load_aterm(filename);
        if (!t || t.type() != AT_APPL || !core::detail::gsIsActionRenameSpec(atermpp::aterm_appl(t)))
        {
          throw runtime_error(((filename.empty())?"stdin":("'" + filename + "'")) + " does not contain an action rename specification");
        }
        init_term(atermpp::aterm_appl(t));
        if (!is_well_typed())
        {
          throw runtime_error("action rename specification is not well typed (action_rename_specification::load())");
        }
      }

      /// \brief Writes the action rename specification to file.
      /// \param[in] filename
      /// If filename is nonempty, output is written to the file named filename.
      /// If filename is empty, output is written to stdout.
      /// \param[in] binary
      /// If binary is true the linear process is saved in compressed binary format.
      /// Otherwise an ascii representation is saved. In general the binary format is
      /// much more compact than the ascii representation.
      void save(const std::string& filename, bool binary = true)
      {
        if (!is_well_typed())
        {
          throw runtime_error("action rename specification is not well typed (action_rename_specification::save())");
        }
        core::detail::save_aterm(m_term, filename, binary);
      }

      /// \brief Returns the data action_rename_specification.
      data::data_specification data() const
      { return m_data; }

      /// \brief Returns a sequence of action labels containing all action
      /// labels occurring in the action_rename_specification (but it can have more).
      action_label_list action_labels() const
      { return m_action_labels; }

      /// \brief Returns the action rename rules.
      action_rename_rule_list rules() const
      {
        return m_rules;
      }

      /// \brief Indicates whether the action_rename_specification is well typed.
      /// \return Always returns true.
      bool is_well_typed() const
      {
        return true;
      }
  };

/// \cond INTERNAL_DOCS
  namespace detail {
    inline
    ATermAppl parse_action_rename_specification(std::istream& from)
    {
      ATermAppl result = core::parse_action_rename_spec(from);
      if (result == NULL)
         throw runtime_error("parse error");
      return result;
    }

    inline
    ATermAppl type_check_action_rename_specification(ATermAppl ar_spec, ATermAppl spec)
    {
      ATermAppl result = core::type_check_action_rename_spec(ar_spec, spec);
      if (result == NULL)
        throw runtime_error("type check error");
      return result;
    }

    inline
    ATermAppl implement_action_rename_specification(ATermAppl ar_spec, ATermAppl& lps_spec)
    {
      ATermAppl result = core::implement_data_action_rename_spec(ar_spec, lps_spec);
      if (result == NULL)
        throw runtime_error("process data implementation error");
      return result;
    }
    
    using namespace mcrl2::data;
    using namespace mcrl2::lps;

    template <typename IdentifierGenerator>
    void rename_renamerule_variables(data_expression& rcond, action& rleft, action& rright, IdentifierGenerator& generator)
    {
  
      std::vector<data_variable> src;  // contains the variables that need to be renamed
      std::vector<data_variable> dest; // contains the corresponding replacements
    
      std::set<data_variable> new_vars;
      std::set<data_variable> found_vars;
      for(data_expression_list::iterator rleft_argument_i = rleft.arguments().begin();
                                              rleft_argument_i != rleft.arguments().end();
                                            ++rleft_argument_i){
        found_vars = find_all_data_variables(*rleft_argument_i);
        new_vars.insert(found_vars.begin(), found_vars.end());
      }
    
      for (std::set<data_variable>::iterator i = new_vars.begin(); i != new_vars.end(); ++i)
      { mcrl2::core::identifier_string new_name = generator(i->name());
        if (new_name != i->name())
        { 
          src.push_back(*i);
          dest.push_back(data_variable(new_name, i->sort()));
        }
      }
  
      rcond = atermpp::partial_replace(rcond, mcrl2::lps::detail::make_data_variable_replacer(src, dest));
      rleft = atermpp::partial_replace(rleft, mcrl2::lps::detail::make_data_variable_replacer(src, dest));
      rright = atermpp::partial_replace(rright, mcrl2::lps::detail::make_data_variable_replacer(src, dest));
    } 

  } // namespace detail
/// \endcond

  /// \brief Parses an action rename specification.
  /// \detail Parses an acion rename specification.
  /// If the action rename specification contains data types that are not
  /// present in the data specification of \p spec they are added to it.
  /// \param text A string containing an action rename specification
  /// \param spec A linear process specification
  /// \return An action rename specification
  inline
  // action_rename_specification parse_action_rename_specification(const std::string& text, lps::specification& spec)
  action_rename_specification parse_action_rename_specification(std::istream& in, lps::specification& spec)
  {
    //std::istringstream in(text);
    ATermAppl lps_spec = spec;
    ATermAppl result = detail::parse_action_rename_specification(in);
    result           = detail::type_check_action_rename_specification(result, lps_spec);   
    result           = detail::implement_action_rename_specification(result, lps_spec);
    spec = lps::specification(lps_spec);
    return action_rename_specification(result);
  }


/// \brief  Rename the actions in a linear specification using a given action rename spec
/// \detail Rename the actions in a linear specification using a given action rename spec.
///         Note that the rules are applied in the order they appear in the specification.
///         This yield quite elaborate conditions in the resulting lps, as a latter rule
///         can only be applied if an earlier rule is not applicable. Note also that 
///         there is always a default summand, where the action is not renamed. Using 
///         sum elimination and rewriting a substantial reduction of the conditions that
///         are generated can be obtained, often allowing many summands to be removed.
/// \param  action_rename_spec The action rename specification to be used.
/// \param  lps_old_spec The input linear specification.
/// \return The lps_old_spec where all actions have been renamed according
///         to action_rename_spec.
lps::specification action_rename(
            const action_rename_specification &action_rename_spec,
            const lps::specification &lps_old_spec)
{
  using namespace mcrl2::utilities;
  using namespace mcrl2::core;
  using namespace mcrl2::data::data_expr;
  using namespace mcrl2::data;
  using namespace mcrl2::lps;
  using namespace std;

  action_rename_rule_list rename_rules = action_rename_spec.rules();
  summand_list lps_old_summands = lps_old_spec.process().summands();
  summand_list lps_summands = summand_list(); //for changes in lps_old_summands
  action_list lps_new_actions = action_list();;

  data::postfix_identifier_generator generator("");
  generator.add_to_context(lps_old_spec);

  bool to_tau=false;
  bool to_delta=false;

  //go through the rename rules of the rename file
  gsVerboseMsg("rename rules found: %i\n", rename_rules.size());
  for(action_rename_rule_list::iterator i = rename_rules.begin(); i != rename_rules.end(); ++i)
  {
    summand_list lps_new_summands;
    
    data_expression rule_condition = i->condition();     
    action rule_old_action =  i->lhs();      
    action rule_new_action;
    action_rename_rule_rhs new_element = i->rhs();

    if(is_action(new_element))
    {
      rule_new_action =  action(new_element);
      to_tau = false;
      to_delta = false;
    }
    else
    {
      rule_new_action = action();
      if(mcrl2::core::detail::gsIsTau(new_element)){ to_tau = true; to_delta = false;}
      else if (mcrl2::core::detail::gsIsDelta(new_element)){ to_tau = false; to_delta = true;}
    }

    // Check here that the arguments of the rule_old_action only consist
    // of uniquely occurring variables or closed terms. Furthermore, check that the variables
    // in rule_new_action and in rule_condition are a subset of those in
    // rule_old_action. This check ought to be done in the static checking
    // part of the renaming rules, but as yet it has nog been done. Ultimately
    // this check should be moved there.

    // first check that the arguments of rule_old_action are variables or closed 
    // terms.

    for(data_expression_list::iterator
                       rule_old_argument_i = rule_old_action.arguments().begin();
                       rule_old_argument_i != rule_old_action.arguments().end();
                       rule_old_argument_i++)
    { if ((!is_data_variable(*rule_old_argument_i)) &&
          (!(find_all_data_variables(*rule_old_argument_i).empty())))
      { std::cerr << "The arguments of the lhs " << pp(rule_old_action) << 
                          " are not variables or closed expressions\n";
        exit(1);
      }
    }
  
    // Check whether the variables in rhs are included in the lefthandside.
    std::set < data_variable > variables_in_old_rule = find_all_data_variables(rule_old_action);
    std::set < data_variable > variables_in_new_rule = find_all_data_variables(rule_new_action);

    if (!includes(variables_in_old_rule.begin(),variables_in_old_rule.end(),
                  variables_in_new_rule.begin(),variables_in_new_rule.end()))
    { std::cerr << "There are variables occurring in rhs " << pp(rule_new_action) << 
                   " of a rename rule not occurring in lhs " << pp(rule_old_action) << "\n";
      exit(1);
    }

    // Check whether the variables in condition are included in the lefthandside.
    std::set < data_variable > variables_in_condition = find_all_data_variables(rule_condition);
    if (!includes(variables_in_old_rule.begin(),variables_in_old_rule.end(),
                  variables_in_condition.begin(),variables_in_condition.end()))
    { std::cerr << "There are variables occurring in the condition " << pp(rule_condition) << 
                   " of a rename rule not occurring in lhs " << pp(rule_old_action) << "\n";
      exit(1);
    }

    // check for double occurrences of variables in the lhs. Note that variables_in_old_rule
    // is empty at the end.
    for(data_expression_list::iterator i=rule_old_action.arguments().begin() ;
                     i!=rule_old_action.arguments().end() ; i++)
    { if (is_data_variable(*i))
      { if (variables_in_old_rule.find(*i)==variables_in_old_rule.end())
        { std::cerr << "Variable " << pp(*i) << " occurs more than once in lhs " << 
                       pp(rule_old_action) << " of an action rename rule\n";
          exit(1);
        }
        else
        { variables_in_old_rule.erase(*i);
        }
      }
    }
    assert(variables_in_old_rule.empty());
 

    lps_summands = summand_list();
    //go through the summands of the old lps
    gsVerboseMsg("summands found: %i\n", lps_old_summands.size());
    for(summand_list::iterator losi = lps_old_summands.begin(); 
                                    losi != lps_old_summands.end(); ++losi)
    {
      summand lps_old_summand = *losi;
      action_list lps_old_actions = lps_old_summand.actions();

      /* For each individual action in the multi-action, for which the 
         rename rule applies, two new summands must be made, namely one
         where the rule does not match with the parameters of the action,
         and one where it actually does. This means that for a multiaction
         with k summands 2^k new summands can result. */

      atermpp::vector < data_variable_list >  
                           lps_new_sum_vars(1,lps_old_summand.summation_variables());
      atermpp::vector < data_expression > 
                         lps_new_condition(1,lps_old_summand.condition());
      atermpp::vector < std::pair <bool, action_list > >
                           lps_new_actions(1,std::make_pair(lps_old_summand.is_delta(),action_list()));
      
      gsVerboseMsg("actions in summand found: %i\n", lps_old_actions.size());
      for(action_list::iterator loai = lps_old_actions.begin(); 
                loai != lps_old_actions.end(); loai++)
      { 
        action lps_old_action = *loai;

        if (equal_signatures(lps_old_action, rule_old_action)) 
        {
          gsVerboseMsg("renaming action %P\n",(ATermAppl)rule_old_action);

          //rename all previously used variables
          data_expression renamed_rule_condition=rule_condition;
          action renamed_rule_old_action=rule_old_action;
          action renamed_rule_new_action=rule_new_action;
          detail::rename_renamerule_variables(renamed_rule_condition, renamed_rule_old_action, renamed_rule_new_action, generator);

          if (is_nil(renamed_rule_condition))
          { renamed_rule_condition=true_();
          }

          //go through the arguments of the action
          data_expression_list::iterator 
                    lps_old_argument_i = lps_old_action.arguments().begin();
          data_expression new_equalities_condition=true_();
          for(data_expression_list::iterator 
                       rule_old_argument_i = renamed_rule_old_action.arguments().begin();
                       rule_old_argument_i != renamed_rule_old_action.arguments().end();
                       rule_old_argument_i++)
          { if (is_data_variable(*rule_old_argument_i))
            { 
              new_equalities_condition=optimized::and_(new_equalities_condition,
                               data_expr::equal_to(*rule_old_argument_i, *lps_old_argument_i));
            }
            else 
            { assert((find_all_data_variables(*rule_old_argument_i).empty())); // the argument must be closed, 
                                                                               // which is checked above.
              renamed_rule_condition=
                        optimized::and_(renamed_rule_condition,
                             data_expr::equal_to(*rule_old_argument_i, *lps_old_argument_i));
            }
            lps_old_argument_i++;
          }

          /* insert the new equality condition in all the newly generated summands */
          for (atermpp::vector < data_expression > :: iterator i=lps_new_condition.begin() ;
                       i!=lps_new_condition.end() ; i++ )
          { *i=optimized::and_(*i,new_equalities_condition);
          }

          /* insert the new sum variables in all the newly generated summands */
          std::set<data_variable> new_vars = find_all_data_variables(renamed_rule_old_action);
          for(std::set<data_variable>::iterator sdvi = new_vars.begin(); 
                         sdvi != new_vars.end(); sdvi++)
          { 
            for ( atermpp::vector < data_variable_list > :: iterator i=lps_new_sum_vars.begin() ;
                        i!=lps_new_sum_vars.end() ; i++ )
            { *i = push_front(*i, *sdvi);
            }
          }

          if (is_true(renamed_rule_condition))
          { 
            if (to_delta)
            { for(atermpp::vector < std::pair <bool, action_list > > :: iterator 
                      i=lps_new_actions.begin() ;
                      i!=lps_new_actions.end() ; i++ )
              { *i=std::make_pair(true,action_list()); /* the action becomes delta */
              }
            }
            else if (!to_tau)
            { for(atermpp::vector < std::pair <bool, action_list > > :: iterator i=lps_new_actions.begin() ;
                        i!=lps_new_actions.end() ; i++ )
              { if (!((*i).first)) // the action is not delta
                { *i=std::make_pair(false,push_front((*i).second,renamed_rule_new_action));
                }
              }
            }
          }
          else if (is_false(renamed_rule_condition))
          {
            for(atermpp::vector < std::pair <bool, action_list > > :: iterator i=lps_new_actions.begin() ;
                        i!=lps_new_actions.end() ; i++ )
            { if (!((*i).first)) // The action does not equal delta.
              { *i=std::make_pair(false,push_front((*i).second,lps_old_action));
              }
            }

          }
          else
          { /* Duplicate summands, one where the renaming is applied, and one where it is not
               applied. */

            atermpp::vector < std::pair <bool, action_list > > lps_new_actions_temp(lps_new_actions);

            if (!to_tau) // if the new element is tau, we do not insert it in the multi-action.
            { for(atermpp::vector < std::pair <bool, action_list > > :: iterator 
                        i=lps_new_actions.begin() ;
                        i!=lps_new_actions.end() ; i++ )
              { if (to_delta) 
                { *i=std::make_pair(true,action_list());
                }
                else 
                { *i=std::make_pair(false,push_front(i->second,renamed_rule_new_action));
                }
              }
            }

            for(atermpp::vector < std::pair <bool, action_list > > :: iterator      
                        i=lps_new_actions_temp.begin() ;
                        i!=lps_new_actions_temp.end() ; i++ )
            { if (!(i->first)) // The element is not equal to delta 
              { *i=std::make_pair(false,push_front(i->second,lps_old_action));
              }
            }

            lps_new_actions.insert(lps_new_actions.end(),
                                   lps_new_actions_temp.begin(),
                                   lps_new_actions_temp.end()); 
              

            /* lps_new_condition_temp will contain the conditions in conjunction with
               the negated new_condition. It will be concatenated to lps_new_condition,
               in which the terms will be conjoined with the non-negated new_condition */

            atermpp::vector < data_expression > lps_new_condition_temp(lps_new_condition);

            for (atermpp::vector < data_expression > :: iterator i=lps_new_condition.begin() ;
                         i!=lps_new_condition.end() ; i++ )
            { *i=optimized::and_(*i,renamed_rule_condition);
            }

            for (atermpp::vector < data_expression > :: iterator i=lps_new_condition_temp.begin() ;
                         i!=lps_new_condition_temp.end() ; i++ )
            { *i=optimized::and_(*i,not_(renamed_rule_condition));
            }

            lps_new_condition.insert(lps_new_condition.end(),
                                     lps_new_condition_temp.begin(),
                                     lps_new_condition_temp.end());
            
            lps_new_sum_vars.insert(lps_new_sum_vars.end(),lps_new_sum_vars.begin(),lps_new_sum_vars.end());
          }

        }//end if(equal_signatures(...))
        else
        { for ( atermpp::vector < std::pair <bool, action_list > > :: iterator i=lps_new_actions.begin() ;
                i!=lps_new_actions.end() ; i++ )
          { *i = std::make_pair((*i).first,push_front((*i).second, lps_old_action));
          }
        }
        gsVerboseMsg("action done\n");

      } //end of action list iterator

      /* Add the summands to lps_new_summands */

      atermpp::vector < std::pair <bool, action_list > > :: iterator i_act=lps_new_actions.begin();
      atermpp::vector < data_variable_list > :: iterator i_sumvars=lps_new_sum_vars.begin();
      for( atermpp::vector < data_expression > :: iterator i_cond=lps_new_condition.begin() ;
           i_cond!=lps_new_condition.end() ; i_cond++)
      { 
        //create a summand for the new lps
        summand lps_new_summand = summand(
                                           *i_sumvars,
                                           *i_cond,
                                           (*i_act).first,
                                           reverse((*i_act).second), 
                                           lps_old_summand.time(),
                                           lps_old_summand.assignments());
        lps_new_summands = push_front(lps_new_summands, lps_new_summand);
        i_act++;
        i_sumvars++;
      }
    } // end of summand list iterator
    lps_old_summands = lps_new_summands;
  } //end of rename rule iterator

  gsVerboseMsg("simplifying the result...\n");

  specification lps_new_spec = specification(
                                          lps_old_spec.data(),
                                          lps_old_spec.action_labels(),
                                          linear_process(
                                                      lps_old_spec.process().free_variables(),
                                                      lps_old_spec.process().process_parameters(),
                                                      lps_old_summands), // These are the renamed sumands.
                                          lps_old_spec.initial_process());

  gsVerboseMsg("new lps complete\n");
  return lps_new_spec;
} //end of rename(...)

} // namespace lps

} // namespace mcrl2


#endif // MCRL2_LPS_ACTION_RENAME_H

