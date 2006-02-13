#ifndef SIP_OPTION_H
#define SIP_OPTION_H

#include <list>
#include <string>
#include <ostream>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <sip/detail/basic_datatype.h>
#include <sip/detail/configuration.h>

namespace sip {

  /** Type for describing an option, which is part of a tool's configuration */
  class option {
    friend class configuration;

    public:
      /** Datatype for the textual identifier of an option */
      typedef std::string                   identifier;

      typedef boost::shared_ptr < option >  option_ptr;

    private:

      typedef std::pair < datatype::basic_datatype*, std::string > type_value_pair;
      typedef std::list < type_value_pair >                        type_value_list;

      /** List of (type, default value) */
      type_value_list arguments;

      /** Must uniquely identify the option in a configuration */
      identifier id;

      /** Constructor (only accessible from class configuration) */
      inline option(identifier);

    public:

      /** Whether the option takes arguments */
      inline bool takes_arguments() const;

      /** Returns the option's identifier */
      inline const identifier& get_id() const;

      /** Append to the type (option takes an additional argument of the specified type) */
      inline void append_type(datatype::basic_datatype&);

      /** Append type and instance ... */
      inline void append_argument(datatype::basic_datatype&, std::string);

      /** Assigns a value to the n-th argument of the option */
      inline void bind_argument(const size_t n, std::string);

      /** Generate XML representation */
      inline void to_xml(std::ostream&) const;

      /** Generate XML representation */
      inline static option_ptr from_xml(xml2pp::text_reader&);
  };

  inline option::option(identifier i) : id(i) {
  }

  inline bool option::takes_arguments() const {
    return (arguments.size() != 0);
  }

  inline const option::identifier& option::get_id() const {
    return (id);
  }

  inline void option::append_type(datatype::basic_datatype& t) {
    arguments.push_back(type_value_pair(&t, ""));
  }

  inline void option::append_argument(datatype::basic_datatype& t, std::string d) {
    arguments.push_back(type_value_pair(&t, d));
  }

  inline void option::bind_argument(const size_t n, std::string s) {
    // TODO validate whether string can be interpretted as type: types[instances.size()]
    type_value_list::iterator i = arguments.begin();

    boost::next(i, n);

    assert(i != arguments.end());

    (*i).second = s;
  }

  inline void option::to_xml(std::ostream& output) const {
    output << "<option id=\"" << id << "\"";

    if (takes_arguments()) {
            type_value_list::const_iterator i = arguments.begin();
      const type_value_list::const_iterator b = arguments.end();

      output << ">";

      while (i != b) {
        (*i).first->to_xml(output, (*i).second);

        ++i;
      }

      output << "</option>";
    }
    else {
      output << "/>";
    }
  }

  inline option::option_ptr option::from_xml(xml2pp::text_reader& reader) {
    std::string id;

    assert(reader.is_element("option"));

    reader.get_attribute(&id, "id");

    option_ptr o(new option(id));

    if (!reader.is_empty_element()) {
      reader.read();

      while (!reader.is_end_element()) {
        using namespace sip::datatype;
     
        /* The current element must be a datatype specification */
        type_value_pair new_argument;
     
        /* Set the type */
        new_argument.first = basic_datatype::from_xml(reader);

        /* The current element can be a value of the previously read type (element is optional) */
        if (reader.is_element("value")) {
          reader.read();
          if (!reader.is_end_element()) {
            /* Read value */
            reader.get_value(&new_argument.second);
     
            reader.read();
          }
     
          /* Skip end tag */
          reader.read();
        }
     
        o->arguments.push_back(new_argument);
      }
    }

    reader.read();

    return (o);
  }
}

#endif
