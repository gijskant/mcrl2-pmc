#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>

#include <xml2pp/text_reader.h>
#include <sip/controller.h>

#include "task_monitor.h"
#include "processor.tcc"
#include "exception.h"
#include "core.h"

namespace squadt {

  void processor::monitor::status_change_dummy(output_status) {
    std::cerr << "No custom status change event handler connected!" << std::endl;
  }

  void processor::monitor::display_layout_change_dummy(sip::layout::tool_display::sptr) {
    std::cerr << "No custom display change event handler connected!" << std::endl;
  }

  void processor::monitor::display_data_change_dummy(sip::layout::tool_display::constant_elements const&) {
    std::cerr << "No custom display state change event handler connected!" << std::endl;
  }

  inline processor::processor(project_manager& p) : current_monitor(new monitor(*this)), manager(&p) {
  }

  /**
   * @param[in] t the tool descriptor of the tool that is to be used to produce the output from the input
   **/
  inline processor::processor(project_manager& p, tool::sptr t) :
    tool_descriptor(t), current_monitor(new monitor(*this)), manager(&p) {
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
    assert(ic != 0);

    selected_input_combination = const_cast < tool::input_combination* > (ic);

    sip::configuration::sptr c(sip::controller::communicator::new_configuration(*selected_input_combination));

    /* Establish what prefix, if any, was used */
//    ic.identifier TODO

    c->set_output_prefix(boost::str(boost::format("%s%04X") % (boost::filesystem::basename(l)) % manager->get_unique_count()));

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
    using namespace boost::filesystem;

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
    using namespace boost::filesystem;

    set_output_status(non_existent);

    /* Make sure any output objects are removed from storage */
    for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
      path p((*i)->location);

      if (exists(p)) {
        remove(p);
      }
    }
  }
}

