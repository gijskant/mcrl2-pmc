// Author(s): Jeroen van der Wulp
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file project_manager.h
/// \brief Add your file description here.

#ifndef PROJECT_MANAGER_H_
#define PROJECT_MANAGER_H_

#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>

#include "tipi/utility/indirect_iterator.hpp"
#include "processor.hpp"

namespace squadt {

  using iterator_wrapper::constant_indirect_iterator;

  class project_manager_impl;

  /**
   * \brief Basic component that stores and retrieves information about projects
   *
   * A project is a collection of processors, that describe how to make output
   * from input objects. The output of one processor can be served as input to
   * another processor which creates potentially complex dependencies among
   * processors. The project manager stores this information and facilitates
   * the running the tools behind the processors to obtain a consistent set of
   * outputs.
   *
   * \attention Processors may not depend on themselves.
   **/
  class project_manager : public utility::visitable, public boost::noncopyable {
    friend class project_manager_impl;
    friend class processor_impl;

    template < typename R, typename S >
    friend class utility::visitor;

    public:

      /** \brief Convenience type alias */
      typedef std::vector < boost::shared_ptr< processor > >                    processor_list;

      /** \brief Iterator type for the processor list */
      typedef constant_indirect_iterator < processor_list, processor >          processor_iterator;

      /** \brief Convenience type alias for a list of conflicting objects */
      typedef std::vector < boost::shared_ptr< processor::object_descriptor > > conflict_list;

      /** \brief Finite type for counting the number of added processors */
      typedef boost::uint32_t                                                   processor_count;

    private:

      /** \brief Pointer to the implementation object (handle-body idiom) */
      boost::shared_ptr < project_manager_impl > impl;

    private:

      /** \brief Constructor for use by read() */
      project_manager();

      /** \brief Load project */
      void load(const boost::filesystem::path&, bool);

    public:
 
      /** \brief Factory function */
      static boost::shared_ptr < project_manager > create(const boost::filesystem::path&, bool);

      /** \brief Checks whether or not a path points to a directory that can be recognised as a project store */
      static bool is_project_store(std::string const&);

      /** \brief Get the name of the project */
      std::string get_name() const;

      /** \brief Get the path to the project store */
      std::string get_project_store() const;

      /** \brief Get the full path to a file (by its name) in the project store */
      boost::filesystem::path get_path_for_name(const std::string&) const;

      /** \brief Recursively add all files in a directory to the project */
      void import_directory(const boost::filesystem::path&);
 
      /** \brief Add a file to the project under a new name */
      boost::shared_ptr< processor > import_file(const boost::filesystem::path&, const std::string& = "");

      /** \brief Get a reference to the list of processors in this project */
      processor_iterator get_processor_iterator() const;

      /** \brief Get the count value */
      processor_count get_unique_count();

      /** \brief Get the description */
      void set_description(const std::string&);

      /** \brief Get the description */
      const std::string& get_description() const;

      /** \brief Read project information from project_store */
      void read();
 
      /** \brief Writes project configuration to the project file */
      void store() const;

      /** \brief Constructs a new processor */
      boost::shared_ptr< processor > construct();

      /** \brief Constructs a new processor */
      boost::shared_ptr< processor > construct(boost::shared_ptr < const tool >, boost::shared_ptr < const tool::input_configuration >);

      /** \brief Check for conflicts and add to project */
      void commit(boost::shared_ptr< processor > const&);

      /** \brief Remove a processor and all processors that depend one one of its outputs */
      void remove(boost::shared_ptr< processor > const&, bool = true);

      /** \brief Updates the status of all outputs that depend on the argument */
      void update_status(boost::shared_ptr< processor > const&, bool = false);

      /** \brief Updates the status of all outputs that depend on the argument, sets it to out-of-date (unless it does not exist) */
      void demote_status(boost::shared_ptr< processor > const&);

      /** \brief Given a processor, it produces a list of object_descriptors that conflict with its outputs */
      std::auto_ptr < conflict_list > get_conflict_list(boost::shared_ptr< processor > const& p) const;
 
      /** \brief Removes all files that cannot be recreated by any of the processors */
      void clean_store(boost::shared_ptr< processor > const& p, bool b);

      /** \brief Make objects in the project up to date */
      void update(boost::shared_ptr< processor > const&, boost::function< void (processor*) >);

      /** \brief Make objects in the project up to date */
      void update(boost::function< void (processor*) >);
  };
}
#endif
