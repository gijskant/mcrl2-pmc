#include <ostream>
#include <sstream>
#include <utility>

#include <boost/bind.hpp>

#include <xml2pp/text_reader.h>

#include <sip/tool/capabilities.h>

namespace sip {
  namespace tool {
    /**
     * \brief Operator for writing to stream
     *
     * @param s stream to write to
     * @param t the capabilities object to write out
     **/
    std::ostream& operator << (std::ostream& s, const capabilities& t) {
      t.write(s);
 
      return (s);
    }

    /** \brief Smaller, performs simple lexicographic comparison (included for use with standard data structures) */
    inline bool operator < (const capabilities::input_combination& a, const capabilities::input_combination& b) {
      return (a.m_mime_type < b.m_mime_type || ((a.m_mime_type == b.m_mime_type) && a.m_category < b.m_category));
    }
 
    /** \brief Smaller, performs simple lexicographic comparison (included for use with standard data structures) */
    inline bool operator < (const capabilities::output_combination& a, const capabilities::output_combination& b) {
      return (a.m_mime_type < b.m_mime_type || a.m_mime_type == b.m_mime_type);
    }
 
    capabilities::capabilities(const version v) : protocol_version(v), interactive(false) {
    }
 
    void capabilities::add_input_combination(object::identifier const& id, mime_type const& f, tool::category const& c) {
      input_combination ic(c, f, id);
 
      input_combinations.insert(ic);
    }
 
    void capabilities::add_output_combination(object::identifier const& id, mime_type const& f) {
      output_combination oc(f, id);
 
      output_combinations.insert(oc);
    }
 
    version capabilities::get_version() const {
      return (protocol_version);
    }
 
    std::string capabilities::write() const {
      std::ostringstream output;
 
      write(output);
 
      return (output.str());
    }
 
    capabilities::input_combination_range capabilities::get_input_combinations() const {
      return (boost::make_iterator_range(input_combinations));
    }
 
    capabilities::output_combination_range capabilities::get_output_combinations() const {
      return (boost::make_iterator_range(output_combinations));
    }
 
    void capabilities::write(std::ostream& output) const {
      output << "<capabilities>"
             << "<protocol-version major=\"" << (unsigned short) protocol_version.major
             << "\" minor=\"" << (unsigned short) protocol_version.minor << "\"/>";
 
      if (interactive) {
        // Tool is interactive
        output << "<interactivity level=\"1\"/>";
      }
      
      for (input_combination_list::const_iterator i = input_combinations.begin(); i != input_combinations.end(); ++i) {
        output << "<input-configuration category=\"" << (*i).m_category
               << "\" format=\"" << (*i).m_mime_type
               << "\" identifier=\"" << (*i).m_identifier << "\"/>";
      }
      for (output_combination_list::const_iterator i = output_combinations.begin(); i != output_combinations.end(); ++i) {
        output << "<output-configuration format=\"" << (*i).m_mime_type
               << "\" identifier=\"" << (*i).m_identifier << "\"/>";
      }
 
      output << "</capabilities>";
    }
 
    void capabilities::set_interactive(bool b) {
      interactive = b;
    }
 
    /**
     * @param s the string to read from
     **/
    capabilities::sptr capabilities::read(const std::string& s) {
      xml2pp::text_reader r(s);

      return (read(r));
    }

    /**
     * @param r the XML text reader to read from
     *
     * \attention if the reader does not point at a capabilities element nothing is read
     **/
    capabilities::sptr capabilities::read(xml2pp::text_reader& r) {
      capabilities::sptr c;

      if (r.is_element("capabilities")) {
        version v = {0,0};
 
        r.next_element();
 
        assert (r.is_element("protocol-version"));
 
        r.get_attribute(&v.major, "major");
        r.get_attribute(&v.minor, "minor");
 
        c = capabilities::sptr(new capabilities(v));
 
        r.next_element();
        r.skip_end_element("protocol-version");
 
        if (r.is_element("interactivity")) {
          c->interactive = r.get_attribute("level");
 
          r.next_element();
          r.skip_end_element("interactivity");
        }
 
        assert (r.is_element("input-configuration"));
 
        while (r.is_element("input-configuration")) {
          std::string        category_name;
          std::string        format;
          object::identifier identifier;
 
          r.get_attribute(&category_name, "category");
          r.get_attribute(&format, "format");
          r.get_attribute(&identifier, "identifier");
 
          c->input_combinations.insert(input_combination(
                  tool::category::fit(category_name),mime_type(format),identifier));
 
          r.next_element();
          r.skip_end_element("input-configuration");
        }

        while (r.is_element("output-configuration")) {
          std::string        format;
          object::identifier identifier;
 
          r.get_attribute(&format, "format");
          r.get_attribute(&identifier, "identifier");
 
          c->output_combinations.insert(output_combination(mime_type(format),identifier));
 
          r.next_element();
          r.skip_end_element("output-configuration");
        }
      }
 
      return (c);
    }

    /**
     * @param f the storage format
     * @param t the category in which the tool operates
     **/
    capabilities::input_combination const*
              capabilities::find_input_combination(const mime_type& f, const tool::category& t) const {
 
      input_combination p(t, f, 0);

      input_combination_list::const_iterator i = std::find_if(input_combinations.begin(),
                      input_combinations.end(), boost::bind(&input_combination::equal, _1, p));

      if (i == input_combinations.end()) {
        return (0);
      }
      else {
        return (&(*i));
      }
    }
  }
}
