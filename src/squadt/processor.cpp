#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>

#include <xml2pp/text_reader.h>
#include <sip/controller.h>

#include "task_monitor.h"
#include "processor.tcc"
#include "exception.h"
#include "core.h"

namespace squadt {

  using namespace boost::filesystem;

  void processor::monitor::status_change_dummy(output_status) {
    std::cerr << "No custom status change event handler connected!" << std::endl;
  }

  void processor::monitor::display_layout_change_dummy(sip::layout::tool_display::sptr) {
    std::cerr << "No custom display change event handler connected!" << std::endl;
  }

  void processor::monitor::display_data_change_dummy(sip::layout::tool_display::constant_elements const&) {
    std::cerr << "No custom display state change event handler connected!" << std::endl;
  }

  void processor::monitor::status_message_change_dummy(sip::report::sptr) {
    std::cerr << "Incoming report lost!" << std::endl;
  }

  inline processor::processor(project_manager& p) : current_monitor(new monitor(*this)), manager(&p), selected_input_combination(0) {
  }

  /**
   * @param[in] p the associated project manager
   * @param[in] t the tool descriptor of the tool that is to be used to produce the output from the input
   **/
  inline processor::processor(project_manager& p, tool::sptr t) :
    tool_descriptor(t), current_monitor(new monitor(*this)), manager(&p), selected_input_combination(0) {
  }

  /**
   * @param[in] p the associated project manager
   **/
  processor::sptr processor::create(project_manager& p) {
    processor::sptr n(new processor(p));

    n->this_object = processor::wptr(n);

    return (n);
  }

  /**
   * @param[in] p the associated project manager
   * @param[in] t the tool to use
   **/
  processor::sptr processor::create(project_manager& p, tool::sptr t) {
    processor::sptr n(new processor(p, t));

    n->this_object = processor::wptr(n);

    return (n);
  }
  
  /**
   * @param[in] ic the input combination that is to be used
   * @param[in] w the path to the directory in which to run the tool
   * @param[in] l absolute path to the file that serves as main input
   *
   * \attention This function is non-blocking
   **/
  void processor::configure(const tool::input_combination* ic, std::string const& w, const boost::filesystem::path& l) {
    using namespace boost;

    assert(ic != 0);

    selected_input_combination = const_cast < tool::input_combination* > (ic);

    sip::configuration::sptr c(sip::controller::communicator::new_configuration(*selected_input_combination));

    c->set_output_prefix(str(format("%s%04X") % (basename(find_initial_object()->location)) % manager->get_unique_count()));

    c->add_input(ic->identifier, ic->format, l.string());

    current_monitor->set_configuration(c);

    configure(w);
  }

  /**
   * Currently it is only checked whether a status of up_to_date is still
   * correct with respect to the state of the inputs and ouputs on physical
   * storage. If the processor is in any other state the function will return
   * false without doing checks.
   *
   * Other checks simply do not look useful at this point given the way a
   * processor is used.
   *
   * \return whether the status was adjusted or not
   *
   * @param[in] r whether to check recursively or not
   **/
  inline bool processor::check_status(const bool r) {
    if (current_output_status == up_to_date) {
      output_status new_status = current_output_status;
     
      time_t maximum_input_timestamp  = 0;
      time_t minimum_output_timestamp = 0;
     
      /* Check whether outputs all exist and find the minimum timestamp of the inputs */
      for (output_list::const_iterator i; i != outputs.end(); ++i) {
        path l((*i)->location);
     
        if (exists(l)) {
          /* Output exists, get timestamp */ 
          minimum_output_timestamp = std::min(minimum_output_timestamp, last_write_time(l));
        }
        else {
          /* Output does not exist; consequently the output is not up-to-date */
          new_status = not_up_to_date;

          break;
        }
      }
     
      /* Find the maximum timestamp of the inputs */
      for (input_list::const_iterator i; i != inputs.end(); ++i) {
        object_descriptor::sptr d = (*i).lock();
     
        if (d.get() == 0) {
          throw (exception::exception(exception::missing_object_descriptor));
        }
     
        path l(d->location);
     
        if (exists(l)) {
          /* Input exists, get timestamp */ 
          time_t stamp = last_write_time(l);
     
          maximum_input_timestamp = std::max(maximum_input_timestamp, stamp);
     
          if (d->timestamp < stamp) {
            /* Compare checksums and update recorded checksum */
            md5pp::compact_digest old = d->checksum;
     
            d->timestamp = stamp;
            d->checksum  = md5pp::MD5::MD5_sum(l);
     
            if (old != d->checksum) {
              new_status = not_up_to_date;
     
              break;
            }
          }
        }
      }
     
      if (minimum_output_timestamp <= maximum_input_timestamp) {
        new_status = not_up_to_date;
      }
      else if (r && current_output_status <= new_status) {
        /* Status can still be okay, check recursively */
        for (input_list::const_iterator i; i != inputs.end(); ++i) {
          object_descriptor::sptr d = (*i).lock();
     
          if (d.get() == 0) {
            throw (exception::exception(exception::missing_object_descriptor));
          }

          processor::sptr p(d->generator);
     
          if (p.get() != 0 && p->check_status(true)) {
            new_status = not_up_to_date;
     
            break;
          }
        }
      }
     
      if (new_status < current_output_status) {
        current_output_status = new_status;
     
        return (true);
      }
    }

    return (false);
  }

  /**
   * @param s the stream to write to
   **/
  void processor::write(std::ostream& s) const {
    s << "<processor";

    if (tool_descriptor.get() != 0) {
      s << " tool-name=\"" << tool_descriptor->get_name() << "\"";

      if (selected_input_combination != 0) {
        s << " format=\"" << selected_input_combination->format << "\"";
        s << " category=\"" << selected_input_combination->category << "\"";
      }
    }

    s << ">\n";

    /* The last received configuration from the tool */
    sip::configuration::sptr c = current_monitor->get_configuration();

    if (c.get() != 0) {
      c->write(s);
    }

    /* The inputs */
    for (input_list::const_iterator i = inputs.begin(); i != inputs.end(); ++i) {
      s << "<input id=\"" << std::dec << reinterpret_cast < unsigned long > ((*i).lock().get()) << "\"/>\n";
    }

    /* The outputs */
    for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
      s << "<output id=\"" << std::dec << reinterpret_cast < unsigned long > ((*i).get())
        << "\" format=\"" << (*i)->format
        << "\" location=\"" << (*i)->location
        << "\" identifier=\"" << std::dec << (*i)->identifier
        << "\" digest=\"" << (*i)->checksum
        << "\" timestamp=\"" << std::dec << (*i)->timestamp << "\"/>\n";
    }

    s << "</processor>\n";
  }

  /**
   * @param[in] p reference to the associated project_manager object
   * @param[in] r an XML text reader object to read from
   * @param[in] m a map that is used to associate shared pointers to processors with identifiers
   *
   * \pre must point to a processor element
   * \attention the same map m must be used to read back all processor instances that were written with write()
   **/
  processor::sptr processor::read(project_manager& p, id_conversion_map& m, xml2pp::text_reader& r) throw () {
    processor::sptr c = create(p);
    std::string     temporary;

    if (r.get_attribute(&temporary, "tool-name")) {
      c->tool_descriptor = global_tool_manager->find(temporary);

      /* Check tool existence */
      if (!global_tool_manager->exists(temporary)) {
        throw (exception::exception(exception::requested_tool_unavailable, temporary));
      }

      storage_format format;
      tool_category  category;

      if (r.get_attribute(&category, "category") && r.get_attribute(&format, "format")) {
        c->selected_input_combination = c->tool_descriptor->find_input_combination(category, format);
      }
    }

    r.next_element();

    if (r.is_element("configuration")) {
      c->current_monitor->set_configuration(sip::configuration::read(r));
    }

    /* Read inputs */
    while (r.is_element("input")) {
      unsigned long id;

      if (!r.get_attribute(&id, "id")) {
        throw (exception::exception(exception::required_attributes_missing, "processor->input"));
      }
      else {
        assert(m.find(id) != m.end());

        c->inputs.push_back(object_descriptor::wptr(m[id]));
      }

      r.next_element();

      r.skip_end_element("input");
    }

    /* Read outputs */
    while (r.is_element("output")) {
      unsigned long id;
      bool          b = r.get_attribute(&id, "id");

      if (b) {
        assert(m.find(id) == m.end());

        m[id] = object_descriptor::sptr(new object_descriptor);

        c->outputs.push_back(m[id]);
      }

      object_descriptor* n = m[id].get();

      if (!(b && r.get_attribute(&n->format, "format")
              && r.get_attribute(&n->location, "location")
              && r.get_attribute(&n->identifier, "identifier")
              && r.get_attribute(&temporary, "digest")
              && r.get_attribute(&n->timestamp, "timestamp"))) {

        throw (exception::exception(exception::required_attributes_missing, "processor->output"));
      }

      n->generator = c;
      n->checksum.read(temporary.c_str());

      r.next_element();

      r.skip_end_element("output");
    }

    r.skip_end_element("processor");

    return (c);
  }

  void processor::flush_outputs() {
    set_output_status(non_existent);

    /* Make sure any output objects are removed from storage */
    for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
      path p((*i)->location);

      if (exists(p)) {
        remove(p);
      }
    }
  }

  /**
   * @param o a pointer to the object to find
   **/
  const processor::object_descriptor::sptr processor::find_output(object_descriptor* o) const {
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
   * @param o a pointer to the object to find
   **/
  const processor::object_descriptor::sptr processor::find_input(object_descriptor* o) const {
    for (input_list::const_iterator i = inputs.begin(); i != inputs.end(); ++i) {
      object_descriptor::sptr s = (*i).lock();

      if (s.get() == o) {
        return (s);
      }
    }
                               
    object_descriptor::sptr s;

    return (s);
  }

  /**
   * @param[in] id a pointer to the object to find
   **/
  const processor::object_descriptor::sptr processor::find_output(const unsigned int id) const {
    for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
      if ((*i)->identifier == id) {

        return (*i);
      }
    }

    object_descriptor::sptr s;

    return (s);
  }

  /**
   * @param[in] id the id of the object to find
   **/
  const processor::object_descriptor::sptr processor::find_input(const unsigned int id) const {
    for (input_list::const_iterator i = inputs.begin(); i != inputs.end(); ++i) {
      object_descriptor::sptr s = (*i).lock();

      if (s.get() != 0 && s->identifier == id) {
        return (s);
      }
    }
                               
    object_descriptor::sptr s;

    return (s);
  }

  /**
   * @param o the name of the input object to find
   **/
  const processor::object_descriptor::sptr processor::find_output(std::string const& o) const {
    for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
      if ((*i)->location == o) {

        return (*i);
      }
    }

    object_descriptor::sptr s;

    return (s);
  }

  /**
   * @param o the name of the input object to find
   **/
  const processor::object_descriptor::sptr processor::find_input(std::string const& o) const {
    for (input_list::const_iterator i = inputs.begin(); i != inputs.end(); ++i) {
      object_descriptor::sptr s = (*i).lock();

      if (s.get() != 0 && s->location == o) {
        return (s);
      }
    }
                               
    object_descriptor::sptr s;

    return (s);
  }

  /**
   * @param[in] o the name (location) of the object to change
   * @param[in] n the new name (location) of the object
   **/
  void processor::rename_object(object_descriptor::sptr const& o, std::string const& n) {

    if (o.get() != 0) {
      path source(o->location);
      path target(n);

      if (exists(source) && source != target) {
        if (exists(target)) {
          remove(target);
        }

        rename(source, target);
      }

      o->location = n;

      /* Update configuration */
      sip::configuration::sptr c = current_monitor->get_configuration();

      if (c.get() != 0) {
        sip::object::sptr object(c->get_output(o->identifier));

        if (object.get() != 0) {
          object->set_location(n);
        }
      }
    }
  }

  /**
   * @param[in] c a reference to a configuration object
   **/
  void processor::process_configuration(sip::configuration::sptr const& c) {
    /* Extract information about output objects from the configuration */
    for (sip::configuration::object_iterator i = c->get_object_iterator(); i.valid(); ++i) {
      if ((*i)->get_type() == sip::object::output) {
        object_descriptor::sptr o = find_output((*i)->get_id());

        if (o.get() == 0) {
          /* Output not registered yet */
          append_output(*(*i));
        }
        else {
          if ((*i)->get_location() != o->location) {
            /* Output already known, but filenames do not match */
            remove(path(o->location));
          }

          replace_output(o, *(*i));
        }
      }
    }
  }

  bool processor::consistent_inputs() const {
    input_list::const_iterator i = inputs.begin();

    while (i != inputs.end()) {
      if ((*i).lock().get() == 0) {
        return false;
      }

      ++i;
    }

    return (true);
  }
}

