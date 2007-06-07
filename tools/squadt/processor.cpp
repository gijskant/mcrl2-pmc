#include <algorithm>
#include <string>
#include <vector>
#include <iosfwd>
#include <ctime>
#include <exception>

#include <boost/function.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include <sip/controller.h>
#include <utilities/logger.h>

#include "processor.tcc"
#include "task_monitor.h"

namespace squadt {

  using namespace boost::filesystem;

  /// \cond INTERNAL_DOCS
  /**
   * \brief Helper function for writing object status to stream
   * \param[in] s stream to read from
   * \param[in] t the status to write
   **/
  std::istream& operator >> (std::istream& stream, processor::object_descriptor::t_status& s) {
    size_t t;

    stream >> t;

    s = static_cast < processor::object_descriptor::t_status > (t);

    return (stream);
  }

  /**
   * \param[in] o an object descriptor
   * \param[in] t the new status
   **/
  bool processor_impl::try_change_status(processor::object_descriptor& o, processor::object_descriptor::t_status s) {
    if (o.status != object_descriptor::generation_in_progress && s < o.status) {
      o.status = s;

      boost::shared_ptr < processor::monitor > m(o.generator.lock()->get_monitor());

      if (!m->status_change_handler.empty()) {
        m->status_change_handler();
      }

      return (true);
    }

    return (false);
  }

  /**
   * \param[in] o a sip::object object that describes an output object
   * \param[in] id the unique identifier for this object in the configuration
   * \param[in] s the status of the new object
   **/
  void processor_impl::append_output(sip::object const& o, parameter_identifier const& id, object_descriptor::t_status const& s) {
    object_descriptor::sptr p = object_descriptor::sptr(new object_descriptor(o.get_mime_type()));

    p->generator  = interface_object;
    p->location   = o.get_location();
    p->identifier = id;
    p->status     = s;
    p->timestamp  = time(0);
    p->checksum;

    append_output(p);
  }

  /**
   * \param[in] p the object descriptor that should be replaced
   * \param[in] id the unique identifier for this object in the configuration
   * \param[in] o a sip::object object that describes an output object
   * \param[in] s the new status of the object
   **/
  void processor_impl::replace_output(object_descriptor::sptr p, parameter_identifier const& id, sip::object const& o, object_descriptor::t_status const& s) {
    p->mime_type  = o.get_mime_type();
    p->location   = o.get_location();
    p->identifier = id;
    p->status     = s;
    p->timestamp  = time(0);
    p->checksum;
  }
  /// \endcond

  /**
   * \param[in] m the mime type of the object
   **/
  processor::object_descriptor::object_descriptor(sip::mime_type const& m) : mime_type(m) {
  }

  /**
   * \param[in] m the associated project manager
   **/
  bool processor::object_descriptor::present_in_store(project_manager const& m) {
    path l(m.get_path_for_name(location));

    if (exists(l)) {
      return (true);
    }
    else {
      processor_impl::try_change_status(*this, reproducible_nonexistent);

      return (false);
    }
  }

  bool processor::object_descriptor::generator_exists() {
    return (generator.lock().get() != 0);
  }

  bool processor::object_descriptor::is_up_to_date() {
    if (status != original && status != reproducible_up_to_date) {
      return (false);
    }
    else {
      processor::sptr p(generator.lock());

      if (p) {
        return (!p->check_status(true));
      }
      else {
        return (false);
      }
    }
  }

  /**
   * \param[in] m a reference to a project manager, used to obtain complete paths files in the project
   * \param[in] t objects older than this time stamp are considered obsolete
   **/
  bool processor::object_descriptor::self_check(project_manager const& m, long int const& t) {
    using namespace boost::filesystem;

    if (!generator.lock()->is_active()) {
      path l(m.get_path_for_name(location));
      
      if (exists(l)) {
        /* Input exists, get timestamp */ 
        time_t stamp = last_write_time(l);
      
        if (stamp < t) {
          return (processor_impl::try_change_status(*this, reproducible_out_of_date));
        }
        else if (timestamp < stamp) {
          /* Compare checksums and update recorded checksum */
          boost::md5::digest_type old = checksum;
      
          checksum = boost::md5(l).digest();
    
          if (timestamp != 0 && old != checksum) {
            return processor_impl::try_change_status(*this, reproducible_up_to_date);
          }
    
          timestamp = stamp;
        }
      }
      else {
        return (processor_impl::try_change_status(*this, reproducible_nonexistent));
      }
    }

    return (false);
  }

  /**
   * \param[in] o the processor that owns of this object
   **/
  processor::monitor::monitor(processor& o) : owner(o) {
  }

  /**
   * \param[in] s the new status
   **/
  void processor::monitor::signal_change(const execution::process::status s) {
    using namespace execution;

    if (owner.number_of_inputs() == 0) {
      switch (s) {
        case process::stopped:
          for (processor::output_object_iterator i = owner.get_output_iterator(); i.valid(); ++i) {
            (*i)->status = object_descriptor::reproducible_out_of_date;
          }
          break;
        case process::running:
          for (processor::output_object_iterator i = owner.get_output_iterator(); i.valid(); ++i) {
            (*i)->status = object_descriptor::generation_in_progress;
          }
          break;
        case process::completed:
          for (processor::output_object_iterator i = owner.get_output_iterator(); i.valid(); ++i) {
            (*i)->status = object_descriptor::reproducible_up_to_date;
          }
          break;
        default: /* aborted... */
          for (processor::output_object_iterator i = owner.get_output_iterator(); i.valid(); ++i) {
            (*i)->status = object_descriptor::original;
          }
          break;
      }
    }
    else {
      switch (s) {
        case process::stopped:
          for (processor::output_object_iterator i = owner.get_output_iterator(); i.valid(); ++i) {
            (*i)->status = object_descriptor::reproducible_out_of_date;
          }
          break;
        case process::running:
          for (processor::output_object_iterator i = owner.get_output_iterator(); i.valid(); ++i) {
            (*i)->status = object_descriptor::generation_in_progress;
          }
          break;
        case process::completed:
          for (processor::output_object_iterator i = owner.get_output_iterator(); i.valid(); ++i) {
            (*i)->status = object_descriptor::reproducible_up_to_date;
          }
          break;
        default: /* aborted... */
          for (processor::output_object_iterator i = owner.get_output_iterator(); i.valid(); ++i) {
            if ((*i)->status == object_descriptor::generation_in_progress) {
              (*i)->status = object_descriptor::reproducible_nonexistent;
            }
          }
          break;
      }
    }

    owner.check_status(false);

    task_monitor::signal_change(s);

    /* Update status for known processor outputs */
    if (!status_change_handler.empty()) {
      status_change_handler();
    }
  }

  execution::process::status processor::monitor::get_status() {
    execution::process::sptr p = get_process();

    if (p.get() != 0) {
      return (p->get_status());
    }

    return (execution::process::stopped);
  }

  void processor::monitor::tool_configuration(processor::sptr t, boost::shared_ptr < sip::configuration > const& c) {
    assert(t.get() == &owner);

    /* collect set of output arguments of the existing configuration */
    std::set < sip::object const* > old_outputs;

    sip::configuration::const_iterator_output_range ir(c->get_output_objects());

    for (sip::configuration::const_iterator_output_range::const_iterator i = ir.begin(); i != ir.end(); ++i) {
      old_outputs.insert(static_cast < sip::object const* > (&*i));
    }

    /* Wait until the tool has connected and identified itself */
    await_connection();

    if (is_connected()) {
      /* Make sure that the task_monitor state is not cleaned up if the tool quits unexpectedly */
      send_configuration(c);

      /* Wait until configuration is accepted, or the tool has terminated */
      if (await_message(sip::message_accept_configuration).get() != 0) {
        /* End tool execution */
        finish();

        /* Operation completed successfully */
        t->impl->process_configuration(get_configuration(), old_outputs, false);
       
        /* Now run the tool */
        t->run(true);
      }
    }
    else {
      /* End tool execution */
      finish();
    }
  }

  void processor::monitor::tool_operation(processor::sptr t, boost::shared_ptr < sip::configuration > const& c) {
    assert(t.get() == &owner);

    /* collect set of output arguments of the existing configuration */
    std::set < sip::object const* > old_outputs;

    sip::configuration::const_iterator_output_range ir(c->get_output_objects());

    for (sip::configuration::const_iterator_output_range::const_iterator i = ir.begin(); i != ir.end(); ++i) {
      old_outputs.insert(static_cast < sip::object const* > (&*i));
    }

    /* Wait until the tool has connected and identified itself */
    await_connection();

    if (is_connected()) {
      /* Make sure that the task_monitor state is not cleaned up if the tool quits unexpectedly */
      send_configuration(c);

      /* Wait until configuration is accepted, or the tool has terminated */
      if (await_message(sip::message_accept_configuration).get() != 0) {
        send_start_signal();

        /* Do not let process status influence return status */
        clear_handlers(sip::message_signal_done);

        if (await_completion()) {
          /* Successful, set new status */
          for (processor::output_object_iterator i = owner.get_output_iterator(); i.valid(); ++i) {
            (*i)->status = object_descriptor::reproducible_up_to_date;
          }

          /* Operation completed successfully */
          t->impl->process_configuration(get_configuration(), old_outputs);
        }
        else {
          /* Task completed unsuccessfully, set new status */
          for (processor::output_object_iterator i = owner.get_output_iterator(); i.valid(); ++i) {
            (*i)->status = object_descriptor::reproducible_out_of_date;
          }
        }
      }
    }

    /* End tool execution */
    finish();

    /* Force the project manager to do a status update */
    boost::shared_ptr < project_manager > g(t->impl->manager);

    if (g.get() != 0) {
      g->update_status(t.get());
    }
  }

  /**
   * \param[in] h the function or functor that is invoked at layout change
   **/
  void processor::monitor::set_display_layout_handler(display_layout_callback_function h) {
    /* Set the handler for incoming layout messages */
    activate_display_layout_handler(h);
  }

  /**
   * \param[in] d the tool display associated with this monitor
   * \param[in] h the function or functor that is invoked at layout change
   **/
  void processor::monitor::set_display_update_handler(sip::layout::tool_display::sptr d, display_update_callback_function h) {
    /* Set the handler for incoming layout messages */
    activate_display_update_handler(d, h);
  }

  /**
   * \param[in] h the function or functor that is invoked at layout change
   **/
  void processor::monitor::set_status_message_handler(status_message_callback_function h) {
    /* Set the handler for incoming layout messages */
    activate_status_message_handler(h);
  }

  void processor::monitor::reset_display_layout_handler() {
    /* Set the handler for incoming layout messages */
    deactivate_display_layout_handler();
  }

  void processor::monitor::reset_display_update_handler() {
    /* Set the handler for incoming layout messages */
    deactivate_display_update_handler();
  }

  void processor::monitor::reset_status_message_handler() {
    /* Set the handler for incoming layout messages */
    deactivate_status_message_handler();
  }

  void processor::monitor::reset_handlers() {
    /* Set the handler for incoming layout messages */
    deactivate_status_message_handler();
    deactivate_display_layout_handler();
    deactivate_display_update_handler();
    status_change_handler.clear();
  }

  void processor::monitor::set_status_handler(status_callback_function h) {
    status_change_handler = h;
  }

  void processor::monitor::start_tool_configuration(processor::sptr const& t, boost::shared_ptr < sip::configuration > const& c) {
    boost::thread thread(boost::bind(&processor::monitor::tool_configuration, this, t, c));
  }

  void processor::monitor::start_tool_operation(processor::sptr const& t, boost::shared_ptr < sip::configuration > const& c) {
    boost::thread thread(boost::bind(&processor::monitor::tool_operation, this, t, c));
  }

  processor::processor() {
  }

  /**
   * \param[in] p the associated project manager
   **/
  processor::sptr processor::create(boost::weak_ptr < project_manager > const& p) {
    processor::sptr n(new processor());

    n->impl.reset(new processor_impl(n, p));

    return (n);
  }

  /**
   * \param[in] p the associated project manager
   * \param[in] t the tool to use
   **/
  processor::sptr processor::create(boost::weak_ptr < project_manager > const& p, tool::sptr t) {
    processor::sptr n(new processor());

    n->impl.reset(new processor_impl(n, p, t));

    return (n);
  }

  /**
   * \param[in] r whether to check recursively or not
   **/
  bool processor::check_status(bool r) {
    return (impl->check_status(r));
  }

  bool processor::demote_status() {
    return (impl->demote_status());
  }

  /**
   * \param[in] t the tool descriptor of the tool that is to be used to produce the output from the input
   **/
  void processor::set_tool(tool::sptr const& t) {
    impl->tool_descriptor = t;
  }

  const tool::sptr processor::get_tool() {
    return (impl->tool_descriptor);
  }

  /**
   * \param[in] i the input combination to set
   **/
  void processor::set_input_combination(tool::input_combination* i) {
    impl->selected_input_combination = i;
  }

  bool processor::has_input_combination() {
    return (impl->selected_input_combination != 0);
  }

  tool::input_combination const* processor::get_input_combination() const {
    return(impl->selected_input_combination);
  }

  boost::shared_ptr < processor::monitor > processor::get_monitor() {
    return (impl->current_monitor);
  }

  processor::input_object_iterator processor::get_input_iterator() const {
    return (input_object_iterator(impl->inputs));
  }

  /**
   * \param p weak pointer to an object descriptor
   **/
  void processor::append_input(object_descriptor::sptr const& p) {
    impl->inputs.push_back(p);
  }

  /**
   * \param o the name (location) of the object to change
   * \param n the new name (location) of the object
   **/
  void processor::rename_input(std::string const& o, std::string const& n) {
    impl->rename_object(impl->find_input(o), n);
  }

  /**
   * \param o the name (location) of the object to change
   * \param n the new name (location) of the object
   **/
  void processor::rename_output(std::string const& o, std::string const& n) {
    impl->rename_object(impl->find_output(o), n);
  }

  processor::output_object_iterator processor::get_output_iterator() const {
    return (output_object_iterator(impl->outputs));
  }

  void processor::shutdown() {
    impl->shutdown();
  }

  void processor::flush_outputs() {
    impl->flush_outputs();
  }

  /**
   * \param[in] m the mime type of l
   * \param[in] id the unique identifier for this object in the configuration
   * \param[in] l a URI (local path) to where the file is stored
   * \param[in] s the status of the new object
   **/
  void processor::append_output(build_system::mime_type const& m, parameter_identifier const& id, const std::string& l, object_descriptor::t_status const& s) {
    object_descriptor::sptr p = object_descriptor::sptr(new object_descriptor(m));

    p->generator  = impl->interface_object;
    p->location   = l;
    p->identifier = "";
    p->status     = s;
    p->timestamp  = time(0);
    p->checksum;

    impl->append_output(p);
  }

  void processor::configure(std::string const& w) {
    assert(impl->interface_object.lock().get() == this);

    impl->configure(impl->interface_object.lock(), impl->current_monitor->get_configuration(), w);
  }

  void processor::configure(const tool::input_combination* i, const boost::filesystem::path& p, std::string const& w) {
    assert(impl->interface_object.lock().get() == this);

    impl->configure(impl->interface_object.lock(), i, p, w);
  }

  void processor::reconfigure(std::string const& w) {
    assert(impl->interface_object.lock().get() == this);

    impl->reconfigure(impl->interface_object.lock(), boost::shared_ptr < sip::configuration > (new sip::configuration(*impl->current_monitor->get_configuration())), w);
  }

  /**
   * \param[in] h the function to execute when the process terminates
   * \param[in] b whether or not to run when there are no input objects defined
   *
   * \pre t.get() == this
   **/
  void processor::update(boost::function < void () > h, bool b) {
    assert(impl->interface_object.lock().get() == this);

    impl->update(impl->interface_object.lock(), h, impl->current_monitor->get_configuration(), b);
  }

  void processor::update(bool b) {
    assert(impl->interface_object.lock().get() == this);

    impl->update(impl->interface_object.lock(), impl->current_monitor->get_configuration(), b);
  }

  /**
   * \param[in] h the function to execute when the process terminates
   * \param[in] b whether or not to run when there are no input objects defined
   *
   * \pre t.get() == this
   **/
  void processor::run(boost::function < void () > h, bool b) {
    assert(impl->interface_object.lock().get() == this);

    impl->run(impl->interface_object.lock(), h, impl->current_monitor->get_configuration(), b);
  }

  void processor::run(bool b) {
    assert(impl->interface_object.lock().get() == this);

    impl->run(impl->interface_object.lock(), impl->current_monitor->get_configuration(), b);
  }

  const size_t processor::number_of_inputs() const {
    return (impl->inputs.size());
  }

  const size_t processor::number_of_outputs() const {
    return (impl->outputs.size());
  }

  bool processor::is_active() const {
    return (impl->is_active());
  }

  /**
   * \param[in] c the edit command to execute
   **/
  void processor::edit(execution::command* c) {
    if (c != 0) {
      impl->edit(c);
    }
  }
}

