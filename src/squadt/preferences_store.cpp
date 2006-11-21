#include "preferences_visitors.h"

#include <fstream>

#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>

#include "build_system.h"
#include "tool_manager.h"
#include "settings_manager.h"
#include "type_registry.h"
#include "executor.h"

#include <utility/visitor.h>

namespace squadt {

  class write_preferences_visitor_impl : utility::visitor< write_preferences_visitor, void > {

    public:

      write_preferences_visitor_impl(boost::filesystem::path const&);

      /** \brief Writes state for objects of type T */
      template < typename T >
      void visit(T const&) const;
  };

  /**
   * \param[in] b reference to a build_system instance
   * \param[in] p a path to the file to write to
   **/
  write_preferences_visitor::write_preferences_visitor(boost::filesystem::path const& p) :
                        impl(new write_preferences_visitor_impl(p)) {
  }

  /**
   * \param[in] b reference to a build_system instance
   * \param[in] p a path to the file from which to read
   **/
  write_preferences_visitor_impl::write_preferences_visitor_impl(boost::filesystem::path const& p) {
  }

  template <>
  void write_preferences_visitor_impl::visit(tool_manager const& tm) const {
    const boost::filesystem::path file_name(global_build_system.get_settings_manager()->path_to_user_settings(settings_manager::tool_catalog_base_name));

    std::ofstream out(file_name.native_file_string().c_str(), std::ofstream::out|std::ofstream::trunc);

    /* Write header */
    out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        << "<tool-catalog xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
        << " xsi:noNamespaceSchemaLocation=\"tool_catalog.xsd\" version=\"1.0\">\n";
 
    BOOST_FOREACH(tool_manager::tool_list::value_type t, tm.tools) {
      t->write(out);
    }
 
    /* Write footer */
    out << "</tool-catalog>\n";
  }

  template <>
  void write_preferences_visitor_impl::visit(executor const&) const {
  }

  template <>
  void write_preferences_visitor_impl::visit(type_registry const&) const {
  }

  template <>
  void write_preferences_visitor_impl::visit(build_system const& b) const {
    visit(*b.get_tool_manager());
    visit(*b.get_type_registry());
  }

  /**
   * \param[in] p a path to the file from which to read
   **/
  void write_preferences_visitor::store(build_system& b, boost::filesystem::path const& p) {
    write_preferences_visitor v(p);
    
    v.impl->visit(b);
  }
}

