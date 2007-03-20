#include <algorithm>
#include <functional>

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/foreach.hpp>

#include <utility/generic_visitor.h>

#include <sip/detail/controller.tcc>

#include "settings_manager.h"
#include "tool_manager.tcc"
#include "task_monitor.h"
#include "executor.h"
#include "command.h"
#include "extractor.h"
#include "processor.h"
#include "executor.h"

#include "setup.h"

namespace squadt {

  using execution::command;

  namespace bf = boost::filesystem;

  /** \brief Socket connection option scheme for easy command generation */
  const char* socket_connect_pattern    = "--si-connect=sip://%s:%s";

  /** \brief Identifier option scheme for easy command generation */
  const char* identifier_pattern        = "--si-identifier=%s";

  /** \brief Identifier option scheme for easy command generation */
  const char* log_filter_level_pattern  = "--si-log-filter-level=%s";

  const long tool_manager_impl::default_port = 10947;

  const boost::shared_ptr < sip::tool::capabilities > tool::no_capabilities(new sip::tool::capabilities());

  char const* tool_manager_impl::default_tools[] = {"lpsbinary", "lpsconstelm", "lpsdecluster", "lpsinfo", "lpsparelm", "lpsuntime", "lps2lts",                                                     "lpssumelm", "ltsconvert", "ltsinfo", "ltsgraph", "ltsview", "mcrl22lps", "pnml2mcrl2", "xsim", 0};


  tool_manager_impl::tool_manager_impl() : sip::controller::communicator(), free_identifier(0) {
    /* Listen for incoming socket connections on the loopback interface with the default port */
    impl->add_listener(transport::ip_any, default_port);

    /* Set handler for incoming instance identification messages */
    add_handler(sip::message_instance_identification, boost::bind(&tool_manager_impl::handle_relay_connection, this, _1));
  }

  /**
   * \param[in] c the command to run
   * \param[in] b whether or not to circumvent the executor restriction mechanism
   * \param[in] p pointer to the associated monitor
   **/
  void tool_manager_impl::execute(execution::command const* c, execution::task_monitor::sptr p, bool b) {
    global_build_system.get_executor()->execute(*c, p, b);
  }

  /**
   * \param[in] t the tool that is to be run
   * \param[in] p the monitor that should be passed the feedback of execution
   * \param[in] b whether or not to circumvent the executor restriction mechanism
   * \param[in] w the directory in which execution should take place
   **/
  void tool_manager_impl::execute(tool& t, std::string const& w, execution::task_monitor::sptr p, bool b) {
    instance_identifier id = free_identifier++;

    execution::command c(t.get_location(), w);

    c.append_argument(boost::str(boost::format(socket_connect_pattern)
                            % transport::ip_loopback % default_port));
    c.append_argument(boost::str(boost::format(identifier_pattern)
                            % id));
    c.append_argument(boost::str(boost::format(log_filter_level_pattern)
                            % boost::lexical_cast < std::string > (static_cast < unsigned int > (get_standard_logger()->get_filter_level()))));

    instances[id] = p;

    global_build_system.get_executor()->execute(c, p, b);
  }

  /**
   * \param[in] t the tool that is to be run
   *
   * \attention This function blocks.
   **/
  bool tool_manager_impl::query_tool(tool& t) {
    /* Sanity check: establish tool existence */
    if (t.get_location().empty() || !boost::filesystem::exists(boost::filesystem::path(t.get_location()))) {
      return (false);
    }

    /* Create extractor object, that will retrieve the data from the running tool process */
    boost::shared_ptr < extractor > e(new extractor(t));

    execute(t, boost::filesystem::current_path().native_file_string(),
               boost::dynamic_pointer_cast < execution::task_monitor > (e), false);

    /* Wait until the process has been identified */
    execution::process::sptr p(e->get_process(true));

    if (p.get() != 0) {
      /* Start extracting */
      e->extract();

      global_build_system.get_executor()->terminate(p);

      return (true);
    }

    return (false);
  }

  void tool_manager_impl::terminate() {
    using namespace execution;

    /* Request the local tool executor to terminate the running processes known to this tool manager */
    for (validated_instance_list::const_iterator i = validated_instances.begin(); i != validated_instances.end(); ++i) {
      global_build_system.get_executor()->terminate((*i)->get_process());
    }
  }
  
  void tool_manager_impl::factory_configuration() {
    const boost::filesystem::path default_path(global_build_system.get_settings_manager()->path_to_default_binaries());
       
    for (char const** t = tool_manager_impl::default_tools; *t != 0; ++t) {
#if defined(__WIN32__) || defined(__CYGWIN__) || defined(__MINGW32__)
      boost::filesystem::path file_name(std::string(*t) + ".exe");
#else
      boost::filesystem::path file_name(*t);
#endif

      tools.push_back(boost::shared_ptr < tool > (new tool(*t, (default_path / file_name).native_file_string())));
    }

  }

  /**
   * \param[in] n the name of the tool
   **/
  tool::sptr tool_manager_impl::find(const std::string& n) const {
    using namespace boost;

    tool::sptr t;

    for (tool_list::const_iterator i = tools.begin(); i != tools.end(); ++i) {
      if ((*i)->get_name() == n) {
        t = *i;

        break;
      }
    }

    return (t);
  }

  /**
   * \param[in] n the name of the tool
   **/
  bool tool_manager_impl::exists(std::string const& n) const {
    using namespace boost;

    return (find(n).get() != 0);
  }

  /**
   * \param[in] n the name of the tool
   * \param[in] l the location of the tool
   *
   * \return whether the tool was added or not
   **/
  bool tool_manager_impl::add_tool(const std::string& n, const std::string& l) {
    bool b = exists(n);

    if (!b) {
      tools.push_back(tool::sptr(new tool(n, l)));
    }

    return (b);
  }

  /**
   * \param[in] t the name of the tool
   **/
  bool tool_manager_impl::add_tool(tool const& t) {
    bool b = exists(t.get_name());

    if (!b) {
      tools.push_back(tool::sptr(new tool(t)));
    }

    return (b);
  }

  /** \brief Get the list of known tools */
  tool_manager::tool_const_sequence tool_manager_impl::get_tools() const {
    return (boost::make_iterator_range(tools));
  }
 
  /**
   * \param[in] m the message that was just delivered
   **/
  void tool_manager_impl::handle_relay_connection(sip::message_ptr const& m) {
    instance_identifier id = atol(m->to_string().c_str());

    if (instances.find(id) == instances.end()) {
      throw std::runtime_error("Peer provided invalid instance identifier; the connection has been terminated.");
    }

    execution::task_monitor::sptr p(instances[id]);

    impl->relay_connection(p->impl.get(), const_cast < transport::transceiver::basic_transceiver* > (m->get_originator()));

    /* Signal the listener that a connection has been established */
    p->signal_connection(m->get_originator());

    instances.erase(id);
  }

  tool_manager::tool_manager() : impl(new tool_manager_impl) {
  }

  /**
   * \param[in] n the name of the tool
   **/
  bool tool_manager::exists(std::string const& n) const {
    return (impl->exists(n));
  }

  /**
   * \param[in] n the name of the tool
   **/
  tool::sptr tool_manager::find(const std::string& n) const {
    return (impl->find(n));
  }

  /**
   * @param n the name of the tool
   *
   * \pre a tool with this name must be among the known tools
   **/
  boost::shared_ptr < tool > tool_manager::get_tool_by_name(std::string const& n) const {
    tool::sptr t = impl->find(n);

    /* Check tool existence */
    if (!t) {
      throw std::runtime_error("No tool has been registered by the name " + n);
    }

    return (t);
  }

  /**
   * \param[in] n the name of the tool
   * \param[in] l the location of the tool
   *
   * \return whether the tool was added or not
   **/
  bool tool_manager::add_tool(const std::string& n, const std::string& l) {
    return (impl->add_tool(n, l));
  }

  /**
   * \param[in] t the name of the tool
   **/
  bool tool_manager::add_tool(tool const& t) {
    return (impl->add_tool(t));
  }
 
  void tool_manager::query_tools() {
    tool_manager::tool_const_sequence tools(get_tools());

    BOOST_FOREACH(tool_list::value_type t, tools) {
      impl->query_tool(*t);
    }
  }

  /**
   * \param[in] h a function that is called with the name of a tool before it is queried
   **/
  void tool_manager::query_tools(boost::function < void (const std::string&) > h) {
    using namespace boost;

    tool_manager::tool_const_sequence tools(get_tools());

    tool_list retry_list;

    BOOST_FOREACH(tool_list::value_type t, tools) {
      h(t->get_name());

      if (!impl->query_tool(*t)) {
        retry_list.push_back(t);
      }
    }

    /* Retry initialisation of failed tools */
    BOOST_FOREACH(tool_list::value_type t, retry_list) {
      h(t->get_name());

      if (!impl->query_tool(*t)) {
        /* TODO log failure */
      }
    }
  }

  /**
   * \param[in] t the tool that is to be run
   *
   * \attention This function blocks.
   **/
  bool tool_manager::query_tool(tool& t) {
    return (impl->query_tool(t));
  }

  void tool_manager::factory_configuration() {
    impl->factory_configuration();
  }

  /** \brief Get the list of known tools */
  tool_manager::tool_const_sequence tool_manager::get_tools() const {
    return (boost::make_iterator_range(impl->tools));
  }
 
  /** \brief Get the number of known tools */
  unsigned int tool_manager::number_of_tools() const {
    return (impl->tools.size());
  }

  /** \brief Have the tool executor terminate all running tools */
  void tool_manager::terminate() {
    impl->terminate();
  }
}

