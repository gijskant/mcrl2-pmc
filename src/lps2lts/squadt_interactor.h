#ifndef _SQUADT_LPS2LTS_H
#define _SQUADT_LPS2LTS_H

#ifdef ENABLE_SQUADT_CONNECTIVITY
#include "utilities/mcrl2_squadt.h"

class squadt_interactor: public mcrl2_squadt::tool_interface {
  friend class status_display;
  friend class storage_configuration;

  public:
    squadt_interactor();
  
  public:

    class status_display;
    class storage_configuration;

  private:

    static const char*  option_as_aut;
    static const char*  option_out_info;

    static const char*  option_usedummies;
    static const char*  option_state_format_tree;
    static const char*  option_removeunused;

    static const char*  option_rewrite_strategy;
    static const char*  option_exploration_strategy;
      
    static const char*  option_detect_deadlock;
    static const char*  option_detect_actions;
    static const char*  option_trace;
    static const char*  option_max_traces;
    static const char*  option_error_trace;
      
    static const char*  option_confluence_reduction;
    static const char*  option_confluent_tau;
      
    static const char*  option_max_states;
      
    static const char*  option_bithashing;
    static const char*  option_bithashsize;
      
    static const char*  option_init_tsize;

    static const char* lps_file_for_input;
    static const char* lts_file_for_output;
    static const char* trc_file_for_output;

  protected:

    void set_capabilities(sip::tool::capabilities &capabilities) const;

    void user_interactive_configuration(sip::configuration &configuration);

    bool check_configuration(sip::configuration const &configuration) const;

    bool perform_task(sip::configuration &configuration);
};
#endif

#endif
