#ifndef PROCESSOR_TCC
#define PROCESSOR_TCC

#include <algorithm>
#include <functional>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

#include "task_monitor.h"
#include "processor.h"

namespace squadt {

  /**
   * @param[in] o the processor that owns of this object
   **/
  processor::monitor::monitor(processor& o) : owner(o) {
    on_status_change = status_change_dummy;
    on_layout_change = display_layout_change_dummy;
    on_state_change  = display_data_change_dummy;

    /* Set the handler for incoming layout messages */
    activate_display_layout_handler(on_layout_change);

    /* Set the handler for incoming layout messages */
    activate_display_data_handler(sip::layout::tool_display::sptr(), on_state_change);
  }

  inline processor::sptr processor::create() {
    processor::sptr n(new processor());

    n->this_object = processor::wptr(n);

    return (n);
  }

  /**
   * @param[in] t the tool to use
   **/
  inline processor::sptr processor::create(tool::sptr t) {
    processor::sptr n(new processor(t));

    n->this_object = processor::wptr(n);

    return (n);
  }

  /**
   * @param[in] h the function or functor that is invoked at layout change
   **/
  inline void processor::monitor::set_display_layout_handler(display_layout_callback_function h) {
    /* Set the handler for incoming layout messages */
    activate_display_layout_handler(h);
  }

  /**
   * @param[in] d the tool display associated with this monitor
   * @param[in] h the function or functor that is invoked at layout change
   **/
  inline void processor::monitor::set_display_data_handler(sip::layout::tool_display::sptr d, display_data_callback_function h) {
    /* Set the handler for incoming layout messages */
    activate_display_data_handler(d, h);
  }

  /**
   * @param[in] h the function or functor that is invoked at status change
   **/
  inline void processor::monitor::set_status_handler(status_callback_function h) {
    on_status_change = h;
  }

  /**
   * @param[in] s the new status
   **/
  inline void processor::monitor::report_change(execution::process::status s) {
    using namespace execution;

    switch (s) {
      case process::stopped:
        owner.set_output_status(not_up_to_date);
        break;
      case process::running:
        owner.set_output_status(being_computed);
        break;
      case process::completed:
        owner.set_output_status(up_to_date);
        break;
      case process::aborted:
        owner.set_output_status(non_existent);
        break;
    }
  }

  /**
   * @param b whether or not to send the start signal after the configuration is accepted
   **/
  inline void processor::monitor::start_pilot(bool b) {
    boost::thread thread(boost::bind(&processor::monitor::pilot, this, b));
  }

  /**
   * @param b whether or not to send the start signal after the configuration is accepted
   **/
  inline void processor::monitor::pilot(bool b) {
    /* Wait until the tool has connected and identified itself */
    await_connection();

    if (connected) {
      send_configuration();
     
      /* Wait until configuration is accepted, or the tool has terminated */
      if (await_message(sip::send_accept_configuration).get() != 0 && b) {
        send_start_signal();
      }
      else {
        /* End tool execution */
        finish();
      }
    }
  }

  /**
   * \brief Operator for writing to stream
   *
   * @param[in] s stream to write to
   * @param[out] p the processor to write out
   **/
  inline std::ostream& operator << (std::ostream& s, const processor& p) {
    p.write(s);

    return (s);
  }

  inline processor::processor() : current_monitor(new monitor(*this)) {
  }

  /**
   * @param[in] t the tool descriptor of the tool that is to be used to produce the output from the input
   **/
  inline processor::processor(tool::sptr t) :
                tool_descriptor(t), current_monitor(new monitor(*this)) {
  }

  inline processor::~processor() {
  }

  /**
   * @param[in] t the tool descriptor of the tool that is to be used to produce the output from the input
   **/
  inline void processor::set_tool(tool::sptr& t) {
    tool_descriptor = t;
  }

  /**
   * @param[in] t the tool descriptor of the tool that is to be used to produce the output from the input
   **/
  inline void processor::set_tool(tool::sptr t) {
    tool_descriptor = t;
  }

  /**
   * @param[in] i the input combination to set
   **/
  inline void processor::set_input_combination(tool::input_combination* i) {
    selected_input_combination = i;
  }

  inline tool::input_combination const* processor::get_input_combination() const {
    return(selected_input_combination);
  }

  inline const tool::sptr processor::get_tool() {
    return (tool_descriptor);
  }

  inline const processor::monitor::sptr processor::get_monitor() {
    return (current_monitor);
  }

  inline processor::input_object_iterator processor::get_input_iterator() const {
    return (input_object_iterator(inputs));
  }

  /**
   * @param p weak pointer to an object descriptor
   **/
  inline void processor::append_input(object_descriptor::wptr& p) {
    inputs.push_back(p);
  }

  /**
   * @param p weak pointer to an object descriptor
   **/
  inline void processor::append_input(object_descriptor::wptr p) {
    inputs.push_back(p);
  }

  inline const processor::object_descriptor::sptr processor::find_output(object_descriptor* o) {
    object_descriptor::sptr s;

    output_list::const_iterator i = std::find_if(outputs.begin(), outputs.end(),
                boost::bind(std::equal_to < object_descriptor* >(), o, 
                               boost::bind(&object_descriptor::sptr::get, _1)));
                               
    if (i != outputs.end()) {
      s = *i;
    }

    return (s);
  }

  inline processor::output_object_iterator processor::get_output_iterator() const {
    return (output_object_iterator(outputs));
  }

  /**
   * @param p shared pointer to an object descriptor
   **/
  inline void processor::append_output(object_descriptor::sptr& p) {
    p->generator = this_object;

    if (std::find_if(outputs.begin(), outputs.end(),
                boost::bind(std::equal_to < std::string >(), p->location,
                        boost::bind(&object_descriptor::location,
                               boost::bind(&object_descriptor::sptr::get, _1)))) == outputs.end()) {

      outputs.push_back(p);
    }
  }

  /**
   * @param f the storage format that l uses
   * @param l a URI (local path) to where the file is stored
   **/
  inline void processor::append_output(const storage_format& f, const std::string& l) {
    object_descriptor::sptr p = object_descriptor::sptr(new object_descriptor);

    p->format    = f;
    p->location  = l;
    p->timestamp = time(0);
    p->checksum.zero_out();

    append_output(p);
  }

  /**
   * @param o a sip::object object that describes an output object
   **/
  inline void processor::append_output(sip::object const& o) {
    object_descriptor::sptr p = object_descriptor::sptr(new object_descriptor);

    p->format    = o.get_format();
    p->location  = o.get_location();
    p->timestamp = time(0);
    p->checksum.zero_out();

    append_output(p);
  }

  inline void processor::process_configuration() {
    process_configuration(current_monitor->get_configuration());
  }

  /**
   * @param c a reference to a configuration object
   **/
  inline void processor::process_configuration(sip::configuration::sptr const& c) {
    /* Extract information about output objects from the configuration */
    for (sip::configuration::object_iterator i = current_monitor->get_configuration()->get_object_iterator(); i.valid(); ++i) {
      if ((*i)->get_type() == sip::object::output) {
        /* TODO check and replace existing outputs */
        append_output(*(*i));
      }
    }
  }

  /**
   * @param[in] ic the input combination that is to be used
   * @param[in] l the file that serves as main input
   * @param[in] h a function object that is invoked when configuration has completed
   *
   * \attention This function is non-blocking
   **/
  inline void processor::configure(const tool::input_combination* ic, const boost::filesystem::path& l, boost::function < void() > h) {
    configure(ic, l);

    current_monitor->once_on_completion(h);
  }

  /**
   * @param[in] ic the input combination that is to be used
   * @param[in] l the file that serves as main input
   *
   * \attention This function is non-blocking
   **/
  inline void processor::configure(const tool::input_combination* ic, const boost::filesystem::path& l) {
    selected_input_combination = const_cast < tool::input_combination* > (ic);

    sip::configuration::sptr c(new sip::configuration);

    c->add_input(ic->identifier, ic->format, l.string());

    current_monitor->set_configuration(c);

    configure();
  }

  /**
   * \attention This function is non-blocking
   **/
  inline void processor::reconfigure(boost::function < void() > h) {
    current_monitor->once_on_completion(h);

    reconfigure();
  }

  /**
   * \pre The existing configuration must contain the input object matching the selected input combination
   *
   * \attention This function is non-blocking
   **/
  inline void processor::reconfigure() {
    sip::configuration::sptr c(new sip::configuration);

    c->add_object(current_monitor->get_configuration()->get_object(selected_input_combination->identifier));

    current_monitor->set_configuration(c);

    configure();
  }

  /**
   * \pre The existing configuration must contain the input object matching the selected input combination
   *
   * \attention This function is non-blocking
   **/
  inline void processor::configure() {
    global_tool_manager->execute(*tool_descriptor, boost::dynamic_pointer_cast < execution::task_monitor, monitor > (current_monitor), true);

    current_monitor->once_on_completion(boost::bind(&processor::process_configuration, this));
    current_monitor->start_pilot();
  }

  inline void processor::process(boost::function < void () > h) {
    process();

    current_monitor->once_on_completion(h);
  }

  /**
   * \attention This function is non-blocking
   *
   * \pre the configure member must have been called
   **/
  inline void processor::process() {
    global_tool_manager->execute(*tool_descriptor, boost::dynamic_pointer_cast < execution::task_monitor, monitor > (current_monitor), false);

    current_monitor->once_on_completion(boost::bind(&processor::process_configuration, this));
    current_monitor->start_pilot();
  }

  inline const unsigned int processor::number_of_inputs() const {
    return (inputs.size());
  }

  inline const unsigned int processor::number_of_outputs() const {
    return (outputs.size());
  }

  inline bool processor::consistent_inputs() const {
    input_list::const_iterator i = inputs.begin();

    while (i != inputs.end()) {
      if ((*i).lock().get() == 0) {
        return false;
      }

      ++i;
    }

    return (true);
  }

  inline const processor::output_status processor::get_output_status() const {
    return(current_output_status);
  }

  inline void processor::set_output_status(const processor::output_status s) {
    if (current_output_status != s) {
      current_output_status = s;
 
      current_monitor->on_status_change(current_output_status);
    }
  }
}

#endif
