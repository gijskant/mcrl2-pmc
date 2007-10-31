// Author(s): Jeroen van der Wulp
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file processor.tcc
/// \brief Add your file description here.

#ifndef PROCESSOR_TCC
#define PROCESSOR_TCC

#include <algorithm>
#include <functional>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/filesystem/convenience.hpp>

#include "tipi/visitors.hpp"

#include "task_monitor.hpp"
#include "processor.hpp"
#include "project_manager.hpp"
#include "executor.hpp"
#include "tool_manager.tcc"

namespace squadt {
  /// \cond INTERNAL_DOCS

  class processor_impl : public utility::visitable {
    friend class processor;
    friend class processor::monitor;

    friend struct processor::object_descriptor;

    template < typename R, typename S >
    friend class utility::visitor;

    private:

      /** \brief Pointer type for implementation object (handle-body idiom) */
      typedef boost::shared_ptr < processor_impl >      impl_ptr;

      /** \brief Pointer type for interface object (handle-body idiom) */
      typedef boost::shared_ptr < processor >           interface_ptr;

      /** \brief Type for object specification */
      typedef processor::object_descriptor              object_descriptor;

      /** \brief Type for object status specification */
      typedef processor::object_descriptor::t_status    object_status;

      /** \brief Type alias for monitor class */
      typedef processor::monitor                        monitor;

      /** \brief Type alias for list of inputs */
      typedef processor::input_list                     input_list;

      /** \brief Type alias for list of inputs */
      typedef processor::output_list                    output_list;

      /** \brief Convenient type alias */
      typedef processor::parameter_identifier           parameter_identifier;

    private:

      /** \brief Helper type for read() members */
      typedef std::map < unsigned long, object_descriptor::sptr >           id_conversion_map;

    private:

      /** \brief Weak pointer to this object for passing */
      boost::weak_ptr < processor >        interface_object;
 
      /** \brief Identifies the tool that is required to run the command */
      tool::sptr                           tool_descriptor;

      /** \brief The information about inputs of this processor */
      input_list                           inputs;

      /** \brief The information about outputs of this processor */
      output_list                          outputs;
 
      /** \brief The current task that is running or about to run */
      monitor::sptr                        current_monitor;

      /** \brief The associated project manager */
      boost::weak_ptr < project_manager >  manager;
 
      /** \brief The selected input combination of the tool */
      tool::input_combination const*       selected_input_combination;

      /** \brief The directory from which tools should be run on behalf of this object */
      std::string                          output_directory;

    private:

      /** \brief Helper function for adjusting status */
      static bool try_change_status(processor::object_descriptor&, object_status);

      /** \brief Update if object is up-to-date */
      void update_on_success(boost::shared_ptr < object_descriptor >, interface_ptr const&, boost::shared_ptr < tipi::configuration >, bool);

      /** \brief Handler that is executed when an edit operation is completed */
      void edit_completed();

    private:

      /** \brief Basic constructor */
      inline processor_impl(boost::shared_ptr < processor > const&, boost::weak_ptr < project_manager >);

      /** \brief Constructor with tool selection */
      inline processor_impl(boost::shared_ptr < processor > const&, boost::weak_ptr < project_manager >, tool::sptr);

      /** \brief Execute an edit command on one of the outputs */
      void edit(execution::command*);

      /** \brief Extracts useful information from a configuration object */
      void process_configuration(boost::shared_ptr < tipi::configuration > const&, std::set < tipi::object const* >&, bool = true);

      /** \brief Find an object descriptor for a given pointer to an object */
      const object_descriptor::sptr find_output(object_descriptor*) const;
 
      /** \brief Find an object descriptor for a given pointer to an object */
      const object_descriptor::sptr find_input(object_descriptor*) const;
 
      /** \brief Find an object descriptor for a given pointer to an object */
      const object_descriptor::sptr find_output_by_id(parameter_identifier const&) const;
 
      /** \brief Find an object descriptor for a given pointer to an object */
      const object_descriptor::sptr find_input_by_id(parameter_identifier const&) const;
 
      /** \brief Find an object descriptor for a given pointer to an object */
      const object_descriptor::sptr find_output(std::string const&) const;
 
      /** \brief Find an object descriptor for a given pointer to an object */
      const object_descriptor::sptr find_input(std::string const&) const;
 
      /** \brief Get the most original (main) input */
      const object_descriptor::sptr find_initial_object() const;
 
      /** \brief Find an object descriptor for a given name and rename if it exists */
      void rename_object(object_descriptor::sptr const&, std::string const&);

      /** \brief Prepends the absolute path to the project store */
      std::string make_output_path(std::string const&) const;

      /** \brief Check the inputs with respect to the outputs and adjust status accordingly */
      bool check_status(bool);

      /** \brief Sets the status of the inputs to out-of-date if the processor is inactive */
      bool demote_status();

      /** \brief Start tool configuration */
      void configure(interface_ptr const&, const tool::input_combination*, const boost::filesystem::path&, std::string const& = "");
 
      /** \brief Start tool configuration */
      void configure(interface_ptr const&, boost::shared_ptr < tipi::configuration > const&, std::string const& = "");

      /** \brief Start tool reconfiguration */
      void reconfigure(interface_ptr const&, boost::shared_ptr < tipi::configuration > const&, std::string const& = "");
 
      /** \brief Start processing: generate outputs from inputs */
      void run(interface_ptr const&, boost::shared_ptr < tipi::configuration > c, bool b = false);

      /** \brief Start running and afterward execute a function */
      void run(interface_ptr const&, boost::function < void () > h, boost::shared_ptr < tipi::configuration > c, bool b = false);

      /** \brief Start processing if not all outputs are up to date */
      void update(interface_ptr const&, boost::shared_ptr < tipi::configuration > c, bool b = false);
 
      /** \brief Start updating and afterward execute a function */
      void update(interface_ptr const&, boost::function < void () > h, boost::shared_ptr < tipi::configuration > c, bool b = false);

      /** \brief Add an output object */
      void append_output(object_descriptor::sptr&);

      /** \brief Add an output object */
      void append_output(tipi::object const&, parameter_identifier const&,
                object_descriptor::t_status const& = object_descriptor::reproducible_nonexistent);

      /** \brief Replace an existing output object */
      void replace_output(object_descriptor::sptr, parameter_identifier const&, tipi::object const&,
                object_descriptor::t_status const& = object_descriptor::reproducible_up_to_date);

      /** \brief Removes the outputs of this processor from storage */
      void flush_outputs();

      /** \brief Whether or not a process is running on behalf of this processor */
      bool is_active() const;

      /** \brief Terminates running processes and deactivates monitor */
      void shutdown();

    public:

      /** \brief Destructor */
      ~processor_impl();
  };

  /**
   * \param[in] t shared pointer to the interface object
   * \param[in] h the function to execute when the process terminates
   * \param[in] c the configuration object to use
   * \param[in] b whether or not to run when there are no input objects defined
   *
   * \pre t.get() == this
   **/
  inline void processor_impl::update(interface_ptr const& t, boost::function < void () > h, boost::shared_ptr < tipi::configuration > c, bool b) {
    current_monitor->once_on_completion(h);

    update(t, c, b);
  }

  /**
   * \param[in] t shared pointer to the interface object
   * \param[in] h the function to execute when the process terminates
   * \param[in] c the configuration object to use
   * \param[in] b whether or not to run when there are no input objects defined
   *
   * \pre t.get() == this
   **/
  inline void processor_impl::run(interface_ptr const& t, boost::function < void () > h, boost::shared_ptr < tipi::configuration > c, bool b) {
    current_monitor->once_on_completion(h);

    run(t, c, b);
  }

  /**
   * \param p shared pointer to an object descriptor
   **/
  inline void processor_impl::append_output(object_descriptor::sptr& p) {
    p->generator = interface_object;

    if (std::find_if(outputs.begin(), outputs.end(),
                boost::bind(std::equal_to < std::string >(), p->location,
                        boost::bind(&object_descriptor::location,
                               boost::bind(&object_descriptor::sptr::get, _1)))) == outputs.end()) {

      outputs.push_back(p);
    }
  }

  inline processor_impl::processor_impl(boost::shared_ptr < processor > const& tp, boost::weak_ptr < project_manager > p) :
                interface_object(tp), current_monitor(new monitor(*tp)), manager(p), selected_input_combination(0) {
  }

  /**
   * \param[in] tp shared pointer to the interface object
   * \param[in] p the associated project manager
   * \param[in] t the tool descriptor of the tool that is to be used to produce the output from the input
   **/
  inline processor_impl::processor_impl(boost::shared_ptr < processor > const& tp, boost::weak_ptr < project_manager > p, tool::sptr t) :
    interface_object(tp), tool_descriptor(t), current_monitor(new monitor(*tp)), manager(p), selected_input_combination(0) {
  }

  /**
   * \param[in] r whether to check recursively or not
   *
   * \return whether or not the cached status has been changed or is being changed
   **/
  inline bool processor_impl::check_status(const bool r) {
    bool   result = false;

    if (!is_active()) {
      time_t maximum_input_timestamp  = 0;
     
      if (r) {
        /* Check recursively */
        BOOST_FOREACH(object_descriptor::wptr i, inputs) {
          object_descriptor::sptr d = i.lock();
      
          if (d.get() == 0) {
            throw std::runtime_error("dependency on a missing object");
          }
      
          processor::sptr p(d->generator);
      
          if (p.get() != 0) {
            result |= p->check_status(true);
          }
        }
      }
       
      boost::shared_ptr < project_manager > g(manager.lock());
     
      if (g.get()) {
        /* Find the maximum timestamp of the inputs */
        BOOST_FOREACH(boost::weak_ptr < object_descriptor > i, inputs) {
          object_descriptor::sptr d = i.lock();
        
          if (d.get() == 0) {
            throw std::runtime_error("dependency on a missing object");
          }
        
          d->self_check(*g);
     
          maximum_input_timestamp = (std::max)(maximum_input_timestamp, d->timestamp);
     
          result |= (d->status != object_descriptor::original) && (d->status != object_descriptor::reproducible_up_to_date);
        }
       
        /* Check whether outputs all exist and find the minimum timestamp of the inputs */
        BOOST_FOREACH(object_descriptor::sptr o, outputs) {
          o->self_check(*g, static_cast < const long int > (maximum_input_timestamp));
     
          result |= (o->status != object_descriptor::original) && (o->status != object_descriptor::reproducible_up_to_date);
        }
       
        if (result) {
          if (0 < inputs.size()) {
            BOOST_FOREACH(object_descriptor::sptr i, outputs) {
              if (i->status == object_descriptor::reproducible_up_to_date) {
                try_change_status(*i, object_descriptor::reproducible_out_of_date);
              }
            }
          }
          else {
            /* User added files are always up to date */
            g->demote_status(interface_object.lock().get());
          }
        }
      }
     
      return (result);
    }

    return (true);
  }

  inline bool processor_impl::demote_status() {
    bool result = false;

    if (!is_active()) {
      BOOST_FOREACH(object_descriptor::sptr i, outputs) {
        result |= try_change_status(*i, object_descriptor::reproducible_out_of_date);
      }
    }

    return (result);
  }

  inline bool processor_impl::is_active() const {
    return (current_monitor->get_status() == execution::process::running);
  }

  inline processor_impl::~processor_impl() {
    shutdown();
  }

  inline void processor_impl::shutdown() {
    current_monitor->shutdown();
    current_monitor->reset_handlers();
    current_monitor->finish(true);
  }

  inline void processor_impl::flush_outputs() {
    using namespace boost::filesystem;

    boost::shared_ptr < project_manager > g(manager.lock());

    if (g.get() && !is_active()) {
      /* Make sure any output objects are removed from storage */
      for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
        path p(g->get_path_for_name((*i)->location));

        if (exists(p)) {
          remove(p);

          try_change_status(*(*i), object_descriptor::reproducible_nonexistent);
        }
      }
      
      g->update_status(interface_object.lock().get());
    }
  }

  inline const processor::object_descriptor::sptr processor_impl::find_initial_object() const {
    if (inputs.size() != 0) {
      object_descriptor::sptr o(inputs[0]);

      assert(o.get() != 0);

      processor::sptr a(o->generator.lock());

      assert(a.get() != 0);

      return (a->impl->find_initial_object());
    }
    else {
      assert(0 < outputs.size());

      return (outputs[0]);
    }
  }

  /**
   * \param o a pointer to the object to find
   **/
  inline const processor::object_descriptor::sptr processor_impl::find_output(object_descriptor* o) const {
    output_list::const_iterator i = std::find_if(outputs.begin(), outputs.end(),
                boost::bind(std::equal_to < object_descriptor* >(), o, 
                               boost::bind(&object_descriptor::sptr::get, _1)));
                               
    if (i != outputs.end()) {
      return (*i);
    }

    object_descriptor::sptr s;

    return (s);
  }

  /**
   * \param o a pointer to the object to find
   **/
  inline const processor::object_descriptor::sptr processor_impl::find_input(object_descriptor* o) const {
    for (input_list::const_iterator i = inputs.begin(); i != inputs.end(); ++i) {
      object_descriptor::sptr s = (*i);

      if (s.get() == o) {
        return (s);
      }
    }
                               
    object_descriptor::sptr s;

    return (s);
  }

  /**
   * \param[in] id the identifier of the object to find
   **/
  inline const processor::object_descriptor::sptr processor_impl::find_output_by_id(parameter_identifier const& id) const {
    for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
      if ((*i)->identifier == id) {

        return (*i);
      }
    }

    object_descriptor::sptr s;

    return (s);
  }

  /**
   * \param[in] id the identifier of the object to find
   **/
  inline const processor::object_descriptor::sptr processor_impl::find_input_by_id(parameter_identifier const& id) const {
    for (input_list::const_iterator i = inputs.begin(); i != inputs.end(); ++i) {
      object_descriptor::sptr s = (*i);

      if (s.get() != 0 && s->identifier == id) {
        return (s);
      }
    }
                               
    object_descriptor::sptr s;

    return (s);
  }

  /**
   * \param[in] id the name of the object to find
   **/
  inline const processor::object_descriptor::sptr processor_impl::find_output(std::string const& name) const {
    for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
      if ((*i)->location == name) {

        return (*i);
      }
    }

    object_descriptor::sptr s;

    return (s);
  }

  /**
   * \param[in] id the name of the object to find
   **/
  inline const processor::object_descriptor::sptr processor_impl::find_input(std::string const& name) const {
    for (input_list::const_iterator i = inputs.begin(); i != inputs.end(); ++i) {
      object_descriptor::sptr s = (*i);

      if (s.get() != 0 && s->location == name) {
        return (s);
      }
    }
                               
    object_descriptor::sptr s;

    return (s);
  }

  /**
   * \param[in] o the name (location) of the object to change
   * \param[in] n the new name (location) of the object
   **/
  inline void processor_impl::rename_object(object_descriptor::sptr const& o, std::string const& n) {
    using namespace boost::filesystem;

    boost::shared_ptr < project_manager > g(manager.lock());

    if (g.get() != 0 && o.get() != 0) {
      path source(g->get_path_for_name(o->location));
      path target(g->get_path_for_name(n));

      if (exists(source) && source != target) {
        if (exists(target)) {
          remove(target);
        }

        rename(source, target);
      }

      o->location = n;

      /* Update configuration */
      boost::shared_ptr < tipi::configuration > c = current_monitor->get_configuration();

      if (c.get() != 0) {
        tipi::object& object(c->get_output(o->identifier));

        object.set_location(n);
      }
    }
  }

  /**
   * \param[in] c a reference to the new configuration object
   * \param[in] p the previous set of output objects part of the old configuration
   * \param[in] check whether or not to check for existence of concrete outputs
   **/
  inline void processor_impl::process_configuration(boost::shared_ptr < tipi::configuration > const& c,
                                                    std::set < tipi::object const* >& p, bool check) {
    boost::shared_ptr < project_manager > g(manager.lock());

    if (g.get() != 0) {
      tipi::configuration::const_iterator_output_range ir(c->get_output_objects());

      /* Extract information about output objects from the configuration */
      for (tipi::configuration::const_iterator_output_range::const_iterator i = ir.begin(); i != ir.end(); ++i) {
        tipi::configuration::object const& object(static_cast < tipi::configuration::object& > (*i));

        parameter_identifier    id = c->get_identifier(*i);
        object_descriptor::sptr o  = find_output_by_id(id);
       
        if (o.get() == 0) {
          /* Output not registered yet */
          append_output(object, id, object_descriptor::reproducible_nonexistent);
        }
        else {
          if (object.get_location() != o->location) {
            /* Output already known, but filenames do not match */
            remove(g->get_path_for_name(o->location));
          }
       
          replace_output(o, id, object);

          /* Check status */
          o->self_check(*g);
        }

        if (!boost::filesystem::exists(g->get_path_for_name(object.get_location())) && check) {
          std::cerr << "Critical error, output file with name: " << object.get_location() << " does not exist!\n";
          continue;
        }

        /* Remove object from p if it is part of the new configuration too */
        for (std::set< tipi::object const* >::iterator j = p.begin(); j != p.end(); ++j) {
          if ((*j)->get_location() == object.get_location()) {
            p.erase(j);
            break;
          }
        }
      }

      /* Remove files from the old configuration that do not appear in the new one */
      for (std::set< tipi::object const* >::const_iterator i = p.begin(); i != p.end(); ++i) {
        remove(g->get_path_for_name((*i)->get_location()));
      }

      if (0 < outputs.size()) {
        g->add(interface_object.lock());
      }
    }
  }

  /*
   * Prepends the project store to the argument and returns a native filesystem path
   *
   * \param[in] w a directory relative to the project store
   */
  inline std::string processor_impl::make_output_path(std::string const& w) const {
    using namespace boost::filesystem;

    boost::shared_ptr < project_manager > g(manager.lock());

    if (g.get()) {
      path output_path(g->get_project_store());
    
      if (!output_directory.empty()) {
        output_path /= path(output_directory);
      }

      return (output_path.native_file_string());
    }

    return (w);
  }

  /**
   * \param[in] t shared pointer to the interface object
   * \param[in] ic the input combination that is to be used
   * \param[in] l absolute path to the file that serves as main input
   * \param[in] w the path to the directory in which to run the tool
   *
   * \attention This function is non-blocking
   * \pre t.get() == this
   **/
  inline void processor_impl::configure(interface_ptr const& t, const tool::input_combination* ic, const boost::filesystem::path& l, std::string const& w) {
    using namespace boost;
    using namespace boost::filesystem;

    boost::shared_ptr < project_manager > g(manager.lock());

    if (g.get()) {
      assert(ic != 0);

      selected_input_combination = const_cast < tool::input_combination* > (ic);

      boost::shared_ptr < tipi::configuration > c(tipi::controller::communicator::new_configuration(*selected_input_combination));

      c->set_output_prefix(str(format("%s%04X") % (basename(find_initial_object()->location)) % g->get_unique_count()));

      c->add_input(ic->m_identifier, ic->m_mime_type.as_string(), l.string());

      configure(t, c, w);
    }
  }

  /**
   * \param[in] t shared pointer to the interface object
   * \param[in] w the path to the directory relative to the project directory in which to run the tool
   * \param[in] c the configuration object to use
   *
   * \pre The existing configuration must contain the input object matching the selected input combination
   * \pre t->impl.get() == this
   *
   * \attention This function is non-blocking
   **/
  inline void processor_impl::configure(interface_ptr const& t, boost::shared_ptr < tipi::configuration > const& c, std::string const& w) {
    output_directory = w;

    global_build_system.get_tool_manager()->impl->execute(*tool_descriptor, make_output_path(w),
         boost::dynamic_pointer_cast < execution::task_monitor > (current_monitor), true);

    current_monitor->start_tool_configuration(t, c);
  }

  /**
   * \param[in] t shared pointer to the interface object
   * \param[in] w the path to the directory in which to run the tool
   * \param[in] c the configuration object to use
   *
   * \pre The existing configuration must contain the input object matching the selected input combination
   * \pre t.get() == this
   *
   * \attention This function is non-blocking
   **/
  inline void processor_impl::reconfigure(interface_ptr const& t, boost::shared_ptr < tipi::configuration > const& c, std::string const& w) {
    assert(selected_input_combination != 0);

    c->set_fresh();

    configure(t, c, w);
  }

  /**
   * \param[in] t shared pointer to the interface object
   * \param[in] b whether or not to run when there are no input objects defined
   * \param[in] c the configuration object to use
   *
   * \attention This function is non-blocking
   *
   * \pre !is_active() and t.get() == this
   **/
  inline void processor_impl::run(interface_ptr const& t, boost::shared_ptr < tipi::configuration > c, bool b) {
    if (!is_active()) {
      if (b || 0 < inputs.size()) {
        boost::shared_ptr < project_manager > g(manager);

        assert(t->impl.get() == this && g.get());

        /* Check that dependent files exist and rebuild if this is not the case */
        BOOST_FOREACH(input_list::value_type i, inputs) {
          if (!i->present_in_store(*g)) {
            processor::sptr p(i->generator.lock());

            if (p.get() != 0) {
              /* Reschedule process operation after process p has completed */
              p->run(boost::bind(&processor_impl::run, this, t, c, false));
     
              return;
            }
            else {
              /* Should signal an error via the monitor ... */
              throw std::runtime_error("Do not know how to (re)create " + i->location);
            }
          }
        }

        current_monitor->start_tool_operation(t, c);

        global_build_system.get_tool_manager()->impl->execute(*tool_descriptor, make_output_path(output_directory),
           boost::dynamic_pointer_cast < execution::task_monitor > (current_monitor), false);
      }
      else {
        /* Signal completion to environment via monitor */
        current_monitor->signal_change(execution::process::aborted);
      }
    }
  }

  /**
   * \param[in] o object that is checked to be up-to-date
   * \param[in] t shared pointer to the interface object
   * \param[in] c the configuration object to use
   * \param[in] b whether or not to run when there are no input objects are specified
   **/
  inline void processor_impl::update_on_success(boost::shared_ptr < object_descriptor > o, interface_ptr const& t, boost::shared_ptr < tipi::configuration > c, bool b) {
    if (o->is_up_to_date()) {
      update(t, c, b);
    }
  }

  /**
   * \param[in] t shared pointer to the interface object
   * \param[in] c the configuration object to use
   * \param[in] b whether or not to run when there are no input objects are specified
   *
   * \attention This function is non-blocking
   *
   * \pre !is_active() and t.get() == this
   **/
  inline void processor_impl::update(interface_ptr const& t, boost::shared_ptr < tipi::configuration > c, bool b) {
    assert(t->impl.get() == this);

    if (!is_active()) {
      if (b || 0 < inputs.size()) {

        /* Check that dependent files exist and rebuild if this is not the case */
        BOOST_FOREACH(input_list::value_type i, inputs) {
          processor::sptr p(i->generator.lock());

          if (p.get() != 0) {
            if (p->check_status(true)) {

              /* Reschedule process operation after process p has completed */
              p->update(boost::bind(&processor_impl::update_on_success, this, i, t, c, false));

              return;
            }
          }
          else {
            /* Should signal an error via the monitor ... */
            throw std::runtime_error("Do not know how to (re)create " + i->location);
          }
        }

        current_monitor->start_tool_operation(t, c);

        global_build_system.get_tool_manager()->impl->execute(*tool_descriptor, make_output_path(output_directory),
           boost::dynamic_pointer_cast < execution::task_monitor > (current_monitor), false);
      }
      else {
        /* Signal completion to environment via monitor */
        current_monitor->signal_change(execution::process::aborted);
      }
    }
  }

  inline void processor_impl::edit_completed() {
    boost::shared_ptr < processor > p(interface_object.lock());

    if (p.get()) {
      boost::shared_ptr < project_manager > m(manager.lock());

      object_descriptor::t_status new_status = (inputs.size() == 0) ?
                object_descriptor::original : object_descriptor::reproducible_up_to_date;

      for (output_list::iterator i = outputs.begin(); i != outputs.end(); ++i) {
        (*i)->status = new_status;
      }

      if (check_status(true) && m) {
        m->update_status(p.get(), inputs.size() == 0);
      }
    }
  }

  /**
   * \param[in] c the edit command to execute
   **/
  inline void processor_impl::edit(execution::command* c) {
    assert(c != 0);

    c->set_working_directory(make_output_path(output_directory));

    current_monitor->once_on_completion(boost::bind(&processor_impl::edit_completed, this));

    global_build_system.get_tool_manager()->impl->execute(c, boost::dynamic_pointer_cast < execution::task_monitor > (current_monitor), true);
  }

  /// \endcond
}

#endif
