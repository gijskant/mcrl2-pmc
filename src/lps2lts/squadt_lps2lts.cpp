#include <string>
#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include "libnextstate.h"
#include "librewrite.h"
#include "lps2lts.h"
#include "exploration.h"

using namespace std;
    
#ifdef ENABLE_SQUADT_CONNECTIVITY
#include <utilities/squadt_utility.h>

const char*  option_out_info             = "out_info";

const char*  option_usedummies           = "use_dummies";
const char*  option_state_format_tree    = "state_format_tree";
const char*  option_removeunused         = "remove_unused";

const char*  option_rewr_strat           = "rewrite_strategy";

const char*  option_expl_strat           = "expl_strat";
  
const char*  option_detect_deadlock      = "detect_deadlock";
const char*  option_detect_actions       = "detect_actions";
const char*  option_trace                = "trace";
const char*  option_max_traces           = "max_traces";
const char*  option_error_trace          = "error_trace";
  
const char*  option_confluence_reduction = "confluence_reduction";
const char*  option_confluent_tau        = "confluent_tau";
  
const char*  option_max_states           = "max_states";
  
const char*  option_bithashing           = "bithashing";
const char*  option_bithashsize          = "bithash_size";
  
const char*  option_init_tsize           = "init_tsize";

const char* lps_file_for_input_no_lts    = "lps_in_no_lts";
const char* lps_file_for_input_lts       = "lps_in";
const char* lts_file_for_output          = "lts_out";
const char* trc_file_for_output          = "trc_out";

void squadt_lps2lts::initialise()
{
}

void squadt_lps2lts::set_capabilities(sip::tool::capabilities &cp) const
{
  /* The tool has only one main input combination it takes an LPS and then behaves as a reporter */
  cp.add_input_combination(lps_file_for_input_no_lts, sip::mime_type("lps"), sip::tool::category::reporting);
  cp.add_input_combination(lps_file_for_input_lts, sip::mime_type("lps"), sip::tool::category::transformation);
}

void squadt_lps2lts::user_interactive_configuration(sip::configuration& c)
{
  using namespace sip;
  using namespace sip::layout;
  using namespace sip::layout::elements;
  
  bool make_lts = c.input_exists(lps_file_for_input_lts);

  /* Create and add the top layout manager */
  manager::aptr layout_manager(horizontal_box::create());

  /* First column */
  vertical_box* column = new vertical_box();

  
  char buf[21];

  checkbox* cb_aut = make_lts?(new checkbox("generate aut file", false)):NULL;
  checkbox* cb_out_info = new checkbox("save state information", true);
  
  checkbox* cb_usedummies = new checkbox("fill in free variables", true);
  checkbox* cb_state_format_tree = new checkbox("memory efficient state repr.", false);
  checkbox* cb_removeunused = new checkbox("remove unused data", true);
  
  label* lb_rewr_strat = new label("Rewriter:");
  
  label* lb_expl_strat = new label("Strategy:");
  
  checkbox* cb_deadlock = new checkbox("detect deadlocks", false);
  checkbox* cb_actions = new checkbox("detect actions:",false);
  text_field* tf_actions = new text_field("");
  checkbox* cb_trace = new checkbox("save action/deadlock traces, but at most:", false);
  sprintf(buf,"%lu",DEFAULT_MAX_TRACES);
  text_field* tf_max_traces = new text_field(buf);
  checkbox* cb_error_trace = new checkbox("save trace on error", false);
  
  checkbox* cb_confluence = new checkbox("confluence reduction with confluent tau:", false);
  text_field* tf_conf_tau = new text_field("ctau");
  
  
  checkbox* cb_max_states = new checkbox("maximum number of states:",false);
  sprintf(buf,"%llu",1000ULL);
  text_field* tf_max_states = new text_field(buf);

  checkbox* cb_bithashing = new checkbox("bit hashing; number of states:", false);
  sprintf(buf,"%llu",DEFAULT_BITHASHSIZE);
  text_field* tf_bithashsize = new text_field(buf);

  label* lb_init_tsize = new label("initial hash tables size:");
  sprintf(buf,"%lu",DEFAULT_INIT_TSIZE);
  text_field* tf_init_tsize = new text_field(buf);

  horizontal_box* cbsbox = new horizontal_box();
  vertical_box* cbslbox = new vertical_box();
  if ( make_lts )
  cbslbox->add(cb_aut, layout::left);
  cbslbox->add(cb_out_info, layout::left);
  cbslbox->add(cb_usedummies, layout::left);
  cbsbox->add(cbslbox, top);
  vertical_box* cbsrbox = new vertical_box();
  cbsrbox->add(cb_state_format_tree, layout::left);
  cbsrbox->add(cb_removeunused, layout::left);
  cbsbox->add(cbsrbox, top);
  column->add(cbsbox, center);

  column->add(new label(" "),layout::left);

  manager* rewrbox = new horizontal_box();
  rewrbox->add(lb_rewr_strat);
  squadt_utility::radio_button_helper<RewriteStrategy>
    rewr_selector(rewrbox,GS_REWR_INNER,"innermost");
  rewr_selector.associate(rewrbox,GS_REWR_JITTY,"JITty");
  rewr_selector.associate(rewrbox,GS_REWR_INNERC,"compiling innermost");
  rewr_selector.associate(rewrbox,GS_REWR_JITTYC,"compiling JITty");
  column->add(rewrbox,layout::left);

  column->add(new label(" "),layout::left);

  manager* explbox = new horizontal_box();
  explbox->add(lb_expl_strat);
  squadt_utility::radio_button_helper<exploration_strategy>
    expl_selector(explbox,es_breadth, "breadth-first");
  expl_selector.associate(explbox,es_depth, "depth-first");
  expl_selector.associate(explbox,es_random, "random");
  column->add(explbox,layout::left);

  column->add(new label(" "),layout::left);

  column->add(cb_deadlock, layout::left);
  horizontal_box* actionsbox = new horizontal_box();
  actionsbox->add(cb_actions, top);
  actionsbox->add(tf_actions, top);
  column->add(actionsbox, layout::left);
  horizontal_box* maxtracesbox = new horizontal_box();
  maxtracesbox->add(cb_trace,      top);
  maxtracesbox->add(tf_max_traces, top);
  column->add(maxtracesbox,layout::left);
  column->add(cb_error_trace, layout::left);

  column->add(new label(" "),layout::left);

  horizontal_box* confbox = new horizontal_box();
  confbox->add(cb_confluence, middle);
  confbox->add(tf_conf_tau,   middle);
  column->add(confbox, layout::left);

  column->add(new label(" "),layout::left);

  horizontal_box* maxstatesbox = new horizontal_box();
  maxstatesbox->add(cb_max_states, middle);
  maxstatesbox->add(tf_max_states, middle);
  column->add(maxstatesbox, layout::left);

  column->add(new label(" "),layout::left);

  horizontal_box* bithashbox = new horizontal_box();
  bithashbox->add(cb_bithashing,  middle);
  bithashbox->add(tf_bithashsize, middle);
  column->add(bithashbox, layout::left);
  
  column->add(new label(" "),layout::left);

  horizontal_box* tsizebox = new horizontal_box();
  tsizebox->add(lb_init_tsize, bottom);
  tsizebox->add(tf_init_tsize, bottom);
  column->add(tsizebox, layout::left);

  button* okay_button = new button("OK");

  column->add(okay_button, layout::right);

  /* Attach columns*/
  layout_manager->add(column, margins(0,5,0,5));

  send_display_layout(layout_manager);

  /* Wait until the ok button was pressed */
  okay_button->await_change();

  /* Values for the options */
  if ( make_lts )
  {
    std::string input_file_name = c.get_input(lps_file_for_input_lts).get_location();
    /* Add output file to the configuration */
    c.add_output(lts_file_for_output, sip::mime_type(cb_aut->get_status()?"aut":"svc"), c.get_output_name(cb_aut->get_status()?".aut":".svc"));
  }
  c.add_option(option_out_info).append_argument< sip::datatype::boolean >(cb_out_info->get_status());

  c.add_option(option_usedummies).append_argument< sip::datatype::boolean >(cb_usedummies->get_status());
  c.add_option(option_state_format_tree).append_argument< sip::datatype::boolean >(cb_state_format_tree->get_status());
  c.add_option(option_removeunused).append_argument< sip::datatype::boolean >(cb_removeunused->get_status());
  
  c.add_option(option_rewr_strat).append_argument< sip::datatype::integer >(rewr_selector.get_selection());
  
  c.add_option(option_expl_strat).append_argument< sip::datatype::integer >(expl_selector.get_selection());

  c.add_option(option_detect_deadlock).append_argument< sip::datatype::boolean >(cb_deadlock->get_status());
  c.add_option(option_detect_actions).append_argument< sip::datatype::string >(cb_actions->get_status()?tf_actions->get_text():"");
  c.add_option(option_trace).append_argument< sip::datatype::boolean >(cb_trace->get_status());
  c.add_option(option_max_traces).append_argument< sip::datatype::string >(tf_max_traces->get_text());
  c.add_option(option_error_trace).append_argument< sip::datatype::boolean >(cb_error_trace->get_status());
  
  c.add_option(option_confluence_reduction).append_argument< sip::datatype::boolean >(cb_confluence->get_status());
  c.add_option(option_confluent_tau).append_argument< sip::datatype::string >(tf_conf_tau->get_text());
  
  c.add_option(option_max_states).append_argument< sip::datatype::string >(cb_max_states->get_status()?tf_max_states->get_text():"");
  
  c.add_option(option_bithashing).append_argument< sip::datatype::boolean >(cb_bithashing->get_status());
  c.add_option(option_bithashsize).append_argument< sip::datatype::string >(tf_bithashsize->get_text());
  
  c.add_option(option_init_tsize).append_argument< sip::datatype::string >(tf_init_tsize->get_text());
  
  send_clear_display();
}

bool squadt_lps2lts::check_configuration(sip::configuration const &configuration) const
{
  return (
      configuration.input_exists(lps_file_for_input_no_lts) ||
      (configuration.input_exists(lps_file_for_input_lts) &&
       configuration.output_exists(lts_file_for_output))
      );
}

bool squadt_lps2lts::perform_task(sip::configuration &configuration)
{
  lts_generation_options lgopts;

  create_status_display();

  lgopts.squadt = this;

  if ( configuration.input_exists(lps_file_for_input_lts) )
  {
    lgopts.specification = configuration.get_input(lps_file_for_input_lts).get_location();
    lgopts.lts = configuration.get_output(lts_file_for_output).get_location();
  } else {
    lgopts.specification = configuration.get_input(lps_file_for_input_no_lts).get_location();
  }

  lgopts.outinfo = boost::any_cast <bool> (configuration.get_option_argument(option_out_info));

  lgopts.usedummies = boost::any_cast <bool> (configuration.get_option_argument(option_usedummies));
  lgopts.stateformat = (boost::any_cast <bool> (configuration.get_option_argument(option_state_format_tree)))?GS_STATE_TREE:GS_STATE_VECTOR;
  lgopts.removeunused = boost::any_cast <bool> (configuration.get_option_argument(option_removeunused));
  
  lgopts.max_traces = strtoul((boost::any_cast <string> (configuration.get_option_argument(option_max_traces))).c_str(),NULL,0);

  lgopts.strat = (RewriteStrategy) boost::any_cast <long int> (configuration.get_option_argument(option_rewr_strat));
  
  lgopts.expl_strat = (exploration_strategy) boost::any_cast <long int> (configuration.get_option_argument(option_expl_strat));
  
  lgopts.detect_deadlock = boost::any_cast <bool> (configuration.get_option_argument(option_detect_deadlock));
  string actions_str = boost::any_cast <string> (configuration.get_option_argument(option_detect_actions));
  if ( actions_str != "" )
  {
    lgopts.detect_action = true;
    lgopts.trace_actions = parse_action_list(actions_str.c_str(),&lgopts.num_trace_actions);
  }
  lgopts.trace = boost::any_cast <bool> (configuration.get_option_argument(option_trace));
  lgopts.max_traces = strtoul((boost::any_cast <string> (configuration.get_option_argument(option_max_traces))).c_str(),NULL,0);
  lgopts.save_error_trace = boost::any_cast <bool> (configuration.get_option_argument(option_error_trace));
  lgopts.trace = lgopts.trace || lgopts.save_error_trace;
  
  if ( boost::any_cast <bool> (configuration.get_option_argument(option_confluence_reduction)) )
  {
    lgopts.priority_action = strdup((boost::any_cast <string> (configuration.get_option_argument(option_confluent_tau))).c_str());
  }
  
  string max_states_str(boost::any_cast <string> (configuration.get_option_argument(option_max_states)));
  if ( max_states_str != "" )
  {
    lgopts.max_states = boost::lexical_cast < uint64_t > (max_states_str);
  } else {
    lgopts.max_states = DEFAULT_MAX_STATES;
  }

  lgopts.bithashing = boost::any_cast <bool> (configuration.get_option_argument(option_bithashing));

  std::string bithashsize_as_string = boost::any_cast< std::string > (configuration.get_option_argument(option_bithashsize));

  if (!bithashsize_as_string.empty()) {
    lgopts.bithashsize = boost::lexical_cast < uint64_t > (bithashsize_as_string);
  }
  else {
    lgopts.bithashsize = 0;
  }
  
  lgopts.initial_table_size = strtoul((boost::any_cast <string> (configuration.get_option_argument(option_init_tsize))).c_str(),0,0);

  config = &configuration;
  config_changed = false;
  output_count = 3;

  bool ok = false;
  if ( initialise_lts_generation(&lgopts) )
  {
    ok = generate_lts();
    ok = ok || lgopts.error_trace_saved;

    ok &= finalise_lts_generation();

    m_communicator.send_accept_configuration();
  }

  return ok;
}

void squadt_lps2lts::finalise()
{
}
#endif

void squadt_lps2lts::create_status_display()
{
#ifdef ENABLE_SQUADT_CONNECTIVITY
  if ( is_active() )
  {
    using namespace sip;
    using namespace sip::layout;
    using namespace sip::layout::elements;
  
    /* Create and add the top layout manager */
    layout_manager = layout::vertical_box::create();
  
    /* First column */
    layout::vertical_box* column1 = new layout::vertical_box();
    layout::vertical_box* column2 = new layout::vertical_box();
  
    label* lb_level_text = new label("Level:");
    lb_level = new label("0");
    label* lb_explored_text = new label("States explored:");
    lb_explored = new label("0");
    label* lb_seen_text = new label("States seen:");
    lb_seen = new label("0");
    label* lb_transitions_text = new label("Transitions:");
    lb_transitions = new label("0");
  
    column1->add(lb_level_text, layout::left);
    column2->add(lb_level, layout::right);
    column1->add(lb_explored_text, layout::left);
    column2->add(lb_explored, layout::right);
    column1->add(lb_seen_text, layout::left);
    column2->add(lb_seen, layout::right);
    column1->add(lb_transitions_text, layout::left);
    column2->add(lb_transitions, layout::right);
  
    progbar = new progress_bar(0,0,0);
  
    /* Attach columns*/
    layout::box* labels = new layout::horizontal_box();
    labels->add(column1, margins(0,5,0,5));
    labels->add(column2, margins(0,5,0,5));
    layout_manager->add(labels, margins(0,5,0,5));
    layout_manager->add(progbar, margins(0,5,0,5));
  
    send_display_layout(layout_manager);
  }
#endif
}

void squadt_lps2lts::update_status_display(unsigned long level, unsigned long long explored, unsigned long long seen, unsigned long long num_found_same, unsigned long long transitions)
{
#ifdef ENABLE_SQUADT_CONNECTIVITY
  if ( is_active() )
  {
    char buf[21];
    sprintf(buf,"%lu",level);
    lb_level->set_text(buf,&m_communicator);
    sprintf(buf,"%llu",explored);
    lb_explored->set_text(buf,&m_communicator);
    sprintf(buf,"%llu",seen);
    lb_seen->set_text(buf,&m_communicator);
    sprintf(buf,"%llu",transitions);
    lb_transitions->set_text(buf,&m_communicator);
    if ( seen > 1000000ULL )
    {
      explored = explored/(seen/1000000);
      seen = 1000000ULL;
    }
    if ( explored > seen )
    {
      seen = explored;
    }
    progbar->set_maximum(seen,&m_communicator);
    progbar->set_value(explored,&m_communicator);
  }
#endif
}

string squadt_lps2lts::add_output_file(string info, string ext)
{
#ifdef ENABLE_SQUADT_CONNECTIVITY
  string s(config->get_output_name("_"+info+"."+ext));

  config->add_output(trc_file_for_output + boost::lexical_cast < std::string> (output_count++) ,ext,s);

  config_changed = true;

  return s;
#else
  return "";
#endif
}
