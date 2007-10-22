//  Copyright 2007 Jeroen van der Wulp. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/// \file source/restore_visitor.cpp

#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#include <tipi/utility/generic_visitor.tcc>
#include "tipi/visitors.hpp"
#include "tipi/report.hpp"
#include "tipi/tool/capabilities.hpp"
#include "tipi/controller/capabilities.hpp"
#include "tipi/basic_datatype.hpp"
#include "tipi/layout_elements.hpp"
#include "tipi/layout_manager.hpp"
#include "tipi/tool_display.hpp"
#include "tipi/detail/event_handlers.hpp"
#include "tipi/common.hpp"

#include "ticpp.h"

namespace tipi {
  /// \cond INTERNAL_DOCS
  class restore_visitor_impl : private boost::noncopyable {

    friend class visitors;
    friend class restore_visitor;

    protected:

      /** \brief Points to the current element */
      ticpp::Element*  tree;

    public:

      /** \brief Reads from parse tree */
      restore_visitor_impl();

      /** \brief Reads from parse tree */
      restore_visitor_impl(ticpp::Element* s);
  };

  class restore_visitor_impl_frontend : public ::utility::visitor< restore_visitor_impl > {

    private:

      /** \brief The source of input */
      ticpp::Document in;

    public:

      /** \brief Reads from stream */
      restore_visitor_impl_frontend(std::istream& s);

      /** \brief Reads from string */
      restore_visitor_impl_frontend(std::string const&);

      /** \brief Reads from file */
      restore_visitor_impl_frontend(boost::filesystem::path const&);
  };

  inline restore_visitor_impl::restore_visitor_impl() {
  }

  inline restore_visitor_impl::restore_visitor_impl(ticpp::Element* s) : tree(s) {
  }

  inline restore_visitor_impl_frontend::restore_visitor_impl_frontend(std::istream& s) {
    std::ostringstream l;

    l << s.rdbuf();

    in.Parse(l.str());

    tree = in.FirstChildElement(false);
  }

  inline restore_visitor_impl_frontend::restore_visitor_impl_frontend(std::string const& s) {
    in.Parse(s, false);

    tree = in.FirstChildElement(false);
  }

  inline restore_visitor_impl_frontend::restore_visitor_impl_frontend(boost::filesystem::path const& p) {
    in.LoadFile(p.native_file_string().c_str());

    tree = in.FirstChildElement(false);
  }

  std::istream& operator >> (std::istream& s, tipi::message::type_identifier_t& id) {
    size_t t;

    s >> t;

    id = static_cast < tipi::message::type_identifier_t > (t);

    return (s);
  }

  /**
   * \brief Constructor for reading from a ticpp parse tree
   * \param[in] s the parse tree to read from
   **/
  template < >
  restore_visitor::restore_visitor(ticpp::Element& s) :
        ::utility::visitor_interface< restore_visitor_impl >(boost::shared_ptr < ::utility::visitor< restore_visitor_impl > > (new ::utility::visitor< restore_visitor_impl >(&s))) {
  }

  /**
   * \param[in] s the string to read from
   **/
  restore_visitor::restore_visitor(std::string const& s) :
        ::utility::visitor_interface< restore_visitor_impl >(boost::shared_ptr < ::utility::visitor< restore_visitor_impl > > (new restore_visitor_impl_frontend(s))) {
  }

  /**
   * \param[in] p the path of the file to read from
   **/
  restore_visitor::restore_visitor(boost::filesystem::path const& p) :
        ::utility::visitor_interface< restore_visitor_impl >(boost::shared_ptr < ::utility::visitor< restore_visitor_impl > > (new restore_visitor_impl_frontend(p))) {
  }
  /// \endcond
}

namespace utility {
  /// \cond INTERNAL_DOCS

  /**
   * \param[in] o the tipi::message object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::message& o) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "message");

    tree->GetAttributeOrDefault("type", &o.m_type, tipi::message::message_unknown);

    o.m_content.clear();

    for (ticpp::Node* e = tree->FirstChild(false); e != 0; e = e->NextSibling(false)) {
      if (e->Type() == TiXmlNode::TEXT) {
        o.m_content += e->Value();
      }
    }
  }

  /**
   * \param[in] e the tipi::datatype::boolean object to restore
   * \param[in] s the textual representation of an instance
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::datatype::boolean& e, std::string& s) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "boolean");

    s = tree->GetAttribute("value", false);

    if (s != tipi::datatype::boolean::true_string) {
      s = tipi::datatype::boolean::false_string;
    }

    assert(e.validate(s));

    /* Set to default if value is invalid */
    if (!(e.validate(s))) {
      s = tipi::datatype::boolean::false_string;
    }
  }

  /**
   * \param[in] e the tipi::datatype::integer object to restore
   * \param[in] s the textual representation of an instance
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::datatype::integer& e, std::string& s) {
    /* Current element must be <integer> */
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "integer");

    tree->GetAttributeOrDefault("minimum", &e.m_minimum, tipi::datatype::integer::implementation_minimum);
    tree->GetAttributeOrDefault("maximum", &e.m_maximum, tipi::datatype::integer::implementation_maximum);
    tree->GetAttributeOrDefault("default", &e.m_default_value, e.m_minimum);
    tree->GetAttributeOrDefault("value", &s, boost::lexical_cast < std::string > (e.m_default_value));

    assert(e.validate(s));

    /* Set to default if value is invalid */
    if (!(e.validate(s))) {
      s = "0";
    }
  }

  /**
   * \param[in] e the tipi::datatype::real object to restore
   * \param[in] s the textual representation of an instance
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::datatype::real& e, std::string& s) {
    /* Current element must be <integer> */
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "real");

    tree->GetAttributeOrDefault("minimum", &e.m_minimum, tipi::datatype::integer::implementation_minimum);
    tree->GetAttributeOrDefault("maximum", &e.m_maximum, tipi::datatype::integer::implementation_maximum);
    tree->GetAttributeOrDefault("default", &e.m_default_value, e.m_minimum);
    tree->GetAttributeOrDefault("value", &s, boost::lexical_cast < std::string > (e.m_default_value));

    assert(e.validate(s));

    /* Set to default if value is invalid */
    if (!(e.validate(s))) {
      s = "0";
    }
  }

  /**
   * \param[in] e the tipi::datatype::enumeration object to restore
   * \param[in] s the textual representation of an instance
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::datatype::enumeration& e, std::string& s) {
    /* Current element must be <enumeration> */
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "enumeration");

    tree->GetAttributeOrDefault("default", &e.m_default_value, 0);

    for (ticpp::Element* ae = tree->FirstChildElement(false); ae != 0; ae = ae->NextSiblingElement(false)) {
      if (ae->Value() == "element") {
        e.add_value(ae->GetAttribute("value"));
      }
    }

    tree->GetAttributeOrDefault("value", &s, e.m_values[e.m_default_value]);

    assert(e.validate(s));
  }

  /**
   * \param[in] e the tipi::datatype::string object to restore
   * \param[in] s the textual representation of an instance
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::datatype::string& e, std::string& s) {
    /* Current element must be <string> */
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "string");

    tree->GetAttributeOrDefault("default", &e.m_default_value, "");
    tree->GetAttributeOrDefault("minimum", &e.m_minimum_length, 0);
    tree->GetAttributeOrDefault("maximum", &e.m_maximum_length, tipi::datatype::string::implementation_maximum_length);

    s = tree->GetText(false);

    assert(e.validate(s));
  }

  /**
   * \param[in,out] c a shared pointer to a tipi::basic_datatype object to restore
   * \param[in] v the textual representation of an instance
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(boost::shared_ptr < tipi::datatype::basic_datatype >& c, std::string& v) {
    std::string name(tree->Value());

    if (name == "enumeration") {
      c.reset(new tipi::datatype::enumeration);
    }
    else if (name == "boolean") {
      c.reset(new tipi::datatype::boolean);
    }
    else if (name == "integer") {
      c.reset(new tipi::datatype::integer);
    }
    else if (name == "real") {
      c.reset(new tipi::datatype::real);
    }
    else if (name == "uri") {
    }
    else if (name == "string") {
      c.reset(new tipi::datatype::string);
    }
    else {
      /* Unknown type in configuration */
      throw std::runtime_error("Message received with unknown type: '" + name + "'");
    }

    do_visit(*c, v);
  }

  /**
   * \param[in] o a tipi::object object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::object& o) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "object");

    o.m_mime_type = tipi::mime_type(tree->GetAttribute("format"));
    
    tree->GetAttribute("location", &o.m_location, false);
  }

  /**
   * \param[in] o a tipi::option object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::option& o) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "option");

    for (ticpp::Element* e = tree->FirstChildElement(false); e != 0; e = e->NextSiblingElement(false)) {
      /* The current element must be a valid datatype specification */
      std::pair < boost::shared_ptr < tipi::datatype::basic_datatype >, std::string >      p;

      visitor< tipi::restore_visitor_impl >(e).visit(p.first, p.second);

      o.m_arguments.push_back(p);
    }
  }

  /**
   * \param[in] c a tipi::configuration object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::configuration& c) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "configuration");

    /* reset object state */
    c.m_parameter_by_id.clear();
    c.m_options.clear();
    c.m_input_objects.clear();
    c.m_output_objects.clear();
    c.m_positions.clear();

    tree->GetAttributeOrDefault("fresh", &c.m_fresh, false);
    tree->GetAttribute("output-prefix", &c.m_output_prefix, false);
    tree->GetAttribute("category", &c.m_category);

    for (ticpp::Element* e = tree->FirstChildElement(false); e != 0; e = e->NextSiblingElement(false)) {
      std::string identifier = e->GetAttribute("id");
     
      if (e->Value() == "option") {
        boost::shared_ptr < tipi::option > o(new tipi::option);

        visitor< tipi::restore_visitor_impl >(e).visit(*o);

        c.add_option(identifier, o);
      }
      else if (e->Value() == "object") {
        boost::shared_ptr < tipi::object > o(new tipi::object);

        visitor< tipi::restore_visitor_impl >(e).visit(*o);

        if (e->GetAttribute("type") == "input") {
          c.add_input(identifier, o);
        }
        else {
          c.add_output(identifier, o);
        }
      }
    }
  }

  /**
   * \param[in] c a tipi::tool::capabilities object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::tool::capabilities& c) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "capabilities");

    if (tree->Value() == "capabilities") {
      for (ticpp::Element* e = tree->FirstChildElement(false); e != 0; e = e->NextSiblingElement(false)) {
     
        if (e->Value() == "protocol-version") {
          c.m_protocol_version.major = static_cast < unsigned char > (boost::lexical_cast < unsigned short > (e->GetAttribute("major")));
          c.m_protocol_version.minor = static_cast < unsigned char > (boost::lexical_cast < unsigned short > (e->GetAttribute("minor")));
        }
        else if (e->Value() == "input-configuration") {
          c.m_input_combinations.insert(
              tipi::tool::capabilities::input_combination(
                  tipi::tool::category::fit(e->GetAttribute("category")),
                  tipi::mime_type(e->GetAttribute("format")), e->GetAttribute("id")));
        }
        else if (e->Value() == "output-configuration") {
          c.m_output_combinations.insert(
              tipi::tool::capabilities::output_combination(
                      tipi::mime_type(e->GetAttribute("format")), e->GetAttribute("id")));
        }
      }
    }
  }

  /**
   * \param[in] c a tipi::controller::capabilities object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::controller::capabilities& c) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "capabilities");

    if (tree->Value() == "capabilities") {
      for (ticpp::Element* e = tree->FirstChildElement(false); e != 0; e = e->NextSiblingElement(false)) {
        if (e->Value() == "protocol-version") {
          tree->GetAttribute("major", &c.m_protocol_version.major);
          tree->GetAttribute("minor", &c.m_protocol_version.minor);
        }
      }
    }
  }  

  /**
   * \param[in] c a tipi::report object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::report& c) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "report");

    c.m_report_type = static_cast < tipi::report::type > (boost::lexical_cast < unsigned int > (tree->GetAttribute("type")));

    c.description.clear();

    for (ticpp::Element* e = tree->FirstChildElement(false); e != 0; e = e->NextSiblingElement(false)) {
      if (e->Value() == "description") {
        c.description += e->GetText(false);
      }
    }
  }

  /**
   * \param[in] c a tipi::layout::elements::label object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::elements::label& c) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "label");
    
    c.m_text = tree->GetText(false);

    c.m_event_handler->process(&c, false);
  }

  /**
   * \param[in] c a tipi::layout::elements::button object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::elements::button& c) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "button");

    c.m_label = tree->GetText(false);

    c.m_event_handler->process(&c, false);
  }

  /// \cond INTERNAL_DOCS
  template < typename T >
  std::istream& operator>>(std::istream& i, T*& t) {
    return (i >> reinterpret_cast < void*& > (t));
  }
  /// \endcond

  /**
   * \param[in] c a tipi::layout::elements::radio_button object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::elements::radio_button& c) {
    using tipi::layout::elements::radio_button;

    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "radio-button");

    c.m_label = tree->GetText(false);

    tree->GetAttributeOrDefault("selected", &c.m_selected, false);

    if (c.m_selected) {
      c.set_selected();
    }
  }

  /**
   * \param[in] c a tipi::layout::elements::radio_button object to restore
   * \param[in,out] d display with which the element is associated
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::elements::radio_button& c, tipi::display& d) {
    using ::tipi::layout::elements::radio_button;
    using ::tipi::display;

    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "radio-button");

    c.m_label = tree->GetText(false);

    display::element_identifier id = reinterpret_cast < display::element_identifier > (&c);
    
    tree->GetAttribute("connected", &id, false);
    tree->GetAttributeOrDefault("selected", &c.m_selected, false);

    c.m_connection = reinterpret_cast < radio_button* > (id);

    try {
      // Check whether the group is complete
      for (radio_button* i = &c; i != &c; d.find< radio_button >(reinterpret_cast < display::element_identifier > (i->m_connection))) {
      }

      radio_button* i = &c;

      do {
        i->m_connection = d.find< radio_button >(reinterpret_cast < display::element_identifier > (i->m_connection));
        i               = i->m_connection;
      } while (i != &c);

      if (c.m_selected) {
        /* Make sure all associated radio buttons are unselected */
        c.set_selected();
      }
    }
    catch (...) {
    }

    c.m_event_handler->process(&c, false);
  }

  /**
   * \param[in] c a tipi::layout::elements::checkbox object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::elements::checkbox& c) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "checkbox");

    c.m_label = tree->GetText(false);

    tree->GetAttributeOrDefault("checked", &c.m_status, false);

    c.m_event_handler->process(&c, false);
  }

  /**
   * \param[in] c a tipi::layout::elements::progress_bar object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::elements::progress_bar& c) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "progress-bar");

    tree->GetAttribute("minimum", &c.m_minimum);
    tree->GetAttribute("maximum", &c.m_maximum);
    tree->GetAttribute("current", &c.m_current);

    c.m_event_handler->process(&c, false);
  }

  /**
   * \param[in] c a tipi::layout::elements::text_field object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::elements::text_field& c) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "text-field");

    for (ticpp::Element* e = tree->FirstChildElement(false); e != 0; e = e->NextSiblingElement(false)) {
      if (e->Value() == "text") {
        c.m_text = e->GetText(false);
      }
    }

    c.m_event_handler->process(&c, false);
  }

  /// \cond INTERNAL_DOCS
  /** \brief Finds a member of the visibility domain for a string */
  static tipi::layout::visibility text_to_visibility(std::string const& s) {
    if (s == "visible") {
      return (tipi::layout::visible);
    }
    else if (s == "hidden") {
      return (tipi::layout::hidden);
    }
    else { /* Assume none */
      return (tipi::layout::none);
    }
  }

  /** \brief Finds a member of the visibility domain for a string */
  static tipi::layout::vertical_alignment text_to_vertical_alignment(std::string const& s) {
    if (s == "top") {
      return (tipi::layout::top);
    }
    else if (s == "middle") {
      return (tipi::layout::middle);
    }
    else { /* Assume bottom */
      return (tipi::layout::bottom);
    }
  }

  /** \brief Finds a member of the visibility domain for a string */
  static tipi::layout::horizontal_alignment text_to_horizontal_alignment(std::string const& s) {
    if (s == "left") {
      return (tipi::layout::left);
    }
    else if (s == "center") {
      return (tipi::layout::center);
    }
    else { /* Assume right */
      return (tipi::layout::right);
    }
  }
  /// \endcond

  /**
   * \param[in] c a tipi::layout::properties object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::properties& c) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "properties");

    std::string s;

    tree->GetAttribute("horizontal-alignment", &s, false);

    if (!s.empty()) {
      c.m_alignment_horizontal = text_to_horizontal_alignment(s); 
    } else {
      s.clear();
    }

    tree->GetAttribute("vertical-alignment", &s, false);

    if (!s.empty()) {
      c.m_alignment_vertical = text_to_vertical_alignment(s); 
    } else {
      s.clear();
    }

    tree->GetAttribute("visibility",&s , false);

    if (!s.empty()) {
      c.m_visible = text_to_visibility(s); 
    }

    tree->GetAttribute("margin-top", &c.m_margin.top, false);
    tree->GetAttribute("margin-left", &c.m_margin.left, false);
    tree->GetAttribute("margin-bottom", &c.m_margin.bottom, false);
    tree->GetAttribute("margin-right", &c.m_margin.right, false);
    tree->GetAttribute("grow", &c.m_grow, false);
    tree->GetAttribute("enabled", &c.m_enabled, false);
  }

  /**
   * \param[in] c a pointer to the tipi::layout::manager object to restore
   * \param[in,out] d display with which the element is associated
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(boost::shared_ptr < tipi::layout::element >&, tipi::display&);

  /**
   * \param[in] c a tipi::layout::box object to restore
   * \param[in,out] d display with which the element is associated
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::vertical_box& c, tipi::display& d) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "box-layout-manager");

    tipi::layout::properties current_properties;

    c.clear();

    for (ticpp::Element* e = tree->FirstChildElement(false); e != 0; e = e->NextSiblingElement(false)) {
      if (e->Value() == "properties") {
        visitor< tipi::restore_visitor_impl >(e).visit(current_properties);
      }
      else {
        boost::shared_ptr < tipi::layout::element > p;

        visitor< tipi::restore_visitor_impl >(e).visit(p, d);

        if (p.get() != 0) {
          tipi::layout::properties cn = current_properties;

          cn.set_growth(p->get_grow());

          c.m_children.push_back(tipi::layout::manager::layout_descriptor(p.get(), cn));
        }
      }
    }
  }

  /**
   * \param[in] c a tipi::layout::box object to restore
   * \param[in,out] d display with which the element is associated
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::horizontal_box& c, tipi::display& d) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "box-layout-manager");

    tipi::layout::properties current_properties;

    c.clear();

    for (ticpp::Element* e = tree->FirstChildElement(false); e != 0; e = e->NextSiblingElement(false)) {
      if (e->Value() == "properties") {
        visitor< tipi::restore_visitor_impl >(e).visit(current_properties);
      }
      else {
        boost::shared_ptr < tipi::layout::element > p;

        visitor< tipi::restore_visitor_impl >(e).visit(p, d);

        if (p.get() != 0) {
          tipi::layout::properties cn(current_properties);

          cn.set_growth(p->get_grow());

          c.m_children.push_back(tipi::layout::manager::layout_descriptor(p.get(), cn));
        }
      }
    }
  }

  /**
   * \param[in] c a shared pointer to a tipi::layout::manager object to restore
   * \param[in,out] d display with which the element is associated
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(boost::shared_ptr < tipi::layout::manager >& c, tipi::display& d) {
    std::string name(tree->Value());

    if (name == "box-layout-manager") {
      ::tipi::display::element_identifier id;

      tree->GetAttributeOrDefault("id", &id, 0);

      boost::shared_ptr < tipi::layout::element > m;

      if (tree->GetAttribute("variant", false) == "vertical") {
        d.create< tipi::layout::vertical_box >(m, id);

        visit(static_cast < tipi::layout::vertical_box& > (*m), d);
      }
      else {
        d.create< tipi::layout::horizontal_box >(m, id);

        visit(static_cast < tipi::layout::horizontal_box& > (*m), d);
      }

      c = boost::static_pointer_cast < tipi::layout::manager > (m);
    }
    else {
      throw std::runtime_error("Layout manager: '" + name + "' unknown");
    }
  }

  /**
   * \param[in] c a shared pointer to a tipi::layout::element object to restore
   * \param[in,out] d display with which the element is associated
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(boost::shared_ptr < tipi::layout::element >& c, ::tipi::display& d) {
    using namespace tipi::layout::elements;

    ::tipi::display::element_identifier id;

    tree->GetAttributeOrDefault("id", &id, 0);

    std::string name(tree->Value());

    if (name == "radio-button") {
      d.create< radio_button >(c, id);

      // Read concrete element data
      do_visit(*c, d);
    }
    else {
      if (name == "label") {
        d.create< label >(c, id);
      }
      else if (name == "button") {
        d.create< button >(c, id);
      }
      else if (name == "checkbox") {
        d.create< checkbox >(c, id);
      }
      else if (name == "progress-bar") {
        d.create< progress_bar >(c, id);
      }
      else if (name == "text-field") {
        d.create< text_field >(c, id);
      }

      if (c.get()) {
        do_visit(*c);
      }
      else  {
        if (name == "box-layout-manager") {
          if (tree->GetAttribute("variant", false) == "vertical") {
            d.create< tipi::layout::vertical_box >(c, id);
       
            visit(static_cast < tipi::layout::vertical_box& > (*c), d);
          }
          else {
            d.create< tipi::layout::horizontal_box >(c, id);
       
            visit(static_cast < tipi::layout::horizontal_box& > (*c), d);
          }
        }
        else {
          throw std::runtime_error("Layout manager: '" + name + "' unknown");
        }
      }
    }
  }

  /**
   * \param[in] c a tipi::layout::tool_display object to restore
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::tool_display& c) {
    assert((tree->Type() == TiXmlNode::ELEMENT) && tree->Value() == "display-layout");

    if (tree->Value() == "display-layout") {
      c.m_element_by_id.clear();

      tree->GetAttribute("visible", &c.m_visible, false);
     
      for (ticpp::Element* e = tree->FirstChildElement(false); e != 0; e = e->NextSiblingElement(false)) {
        if (e->Value() == "layout-manager" && !e->NoChildren()) {
          visitor< tipi::restore_visitor_impl >(e->FirstChildElement(false)).visit(c.m_manager, static_cast < tipi::display& > (c));
        }
      }
    }
  }

  /**
   * \param[in] c a tipi::layout::tool_display object to restore
   * \param[in,out] elements the list of tipi::layout::element objects that has been modified
   * \todo create and move this functionality to mediator / update visitors
   **/
  template <>
  template <>
  void visitor< tipi::restore_visitor_impl >::visit(tipi::layout::tool_display& c, std::vector < tipi::layout::element const* >& elements) {
  
    if (c.m_manager.get() != 0) {
      try {
        for (ticpp::Element* e = tree; e != 0; e = e->NextSiblingElement(false)) {
          ::tipi::display::element_identifier id;

          e->GetAttribute("id", &id, false);

          if (tipi::layout::element const* t = c.find< tipi::layout::element >(id)) {
            visitor< tipi::restore_visitor_impl >(e).do_visit(*t);

            elements.push_back(t);
          }
        }
      }
      catch (...) {
      }
    }
  }

  /** Registers all available visit methods */
  template <>
  bool visitor< tipi::restore_visitor_impl >::initialise() {
    register_visit_method< tipi::message >();
    register_visit_method< tipi::datatype::boolean, std::string >();
    register_visit_method< tipi::datatype::integer, std::string >();
    register_visit_method< tipi::datatype::real, std::string >();
    register_visit_method< tipi::datatype::enumeration, std::string >();
    register_visit_method< tipi::datatype::string, std::string >();
    register_visit_method< tipi::object >();
    register_visit_method< tipi::option >();
    register_visit_method< tipi::configuration >();
    register_visit_method< tipi::controller::capabilities >();
    register_visit_method< tipi::tool::capabilities >();
    register_visit_method< tipi::report >();
    register_visit_method< tipi::layout::tool_display >();
    register_visit_method< tipi::layout::tool_display, std::vector< tipi::layout::element const* > >();
    register_visit_method< tipi::layout::elements::button >();
    register_visit_method< tipi::layout::elements::checkbox >();
    register_visit_method< tipi::layout::elements::label >();
    register_visit_method< tipi::layout::elements::progress_bar >();
    register_visit_method< tipi::layout::elements::radio_button >();
    register_visit_method< tipi::layout::elements::radio_button, ::tipi::display >();
    register_visit_method< tipi::layout::elements::text_field >();
    register_visit_method< tipi::layout::horizontal_box, ::tipi::display >();
    register_visit_method< tipi::layout::vertical_box, ::tipi::display >();
    register_visit_method< tipi::layout::properties >();

    return true;
  }
  /// \endcond
}
