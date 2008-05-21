// Author(s): Jeroen van der Wulp
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file tipi/tool/category.hpp

#ifndef __TIPI_CATEGORY_H__
#define __TIPI_CATEGORY_H__

#include <string>
#include <algorithm>
#include <functional>

#include <boost/bind.hpp>
#include <boost/array.hpp>

namespace tipi {

  class configuration;

  namespace tool {

    class capabilities;

    /**
     * \brief Category used to classify functionality of tools
     **/
    class category {
      friend class tipi::tool::capabilities;
      friend class tipi::configuration;
      friend std::ostream& operator <<(std::ostream&, category const&);

      private:

        /** \brief Name of the category */
        std::string name;

      private:

        /** \brief unknown, for everything that does not map to one of the public categories */
        static const category unknown;

      public:

        static const category editing;        ///< show edit
        static const category reporting;      ///< show properties of objects
        static const category conversion;     ///< transformations of objects between different storage formats
        static const category transformation; ///< changing objects but retaining the storage format: e.g. optimisation, editing
        static const category visualisation;  ///< visualisation of objects
        static const category simulation;     ///< simulation

        /** \brief The standard available tool categories */
        static const boost::array < category const*, 7 > categories;

      public:

        /** \brief Constructor */
        inline category(std::string const&);

        /** \brief Gets the name of the category */
        inline std::string get_name() const;

        /** \brief Whether or not the category is unknown */
        inline bool is_unknown();

        /** \brief Chooses the best matching category for a string that is interpreted as category name */
        inline static category const& fit(std::string const&);

        /** \brief Compare for smaller */
        inline bool operator <(category const&) const;

        /** \brief Compare for equality */
        inline bool operator ==(category const&) const;

        /** \brief Conversion to STL string */
        inline operator std::string() const;
    };

#ifdef TIPI_IMPORT_STATIC_DEFINITIONS
    const category category::unknown("unknown");
    const category category::editing("editing");
    const category category::reporting("reporting");
    const category category::conversion("conversion");
    const category category::transformation("transformation");
    const category category::visualisation("visualisation");
    const category category::simulation("simulation");

    const boost::array < category const*, 7 > category::categories = { {
      &category::unknown,
      &category::editing,
      &category::reporting,
      &category::conversion,
      &category::transformation,
      &category::visualisation,
      &category::simulation,
    } };
#endif

    inline category::category(std::string const& n) : name(n) {
    }

    inline std::string category::get_name() const {
      return (name);
    }

    inline bool category::is_unknown() {
      return (&unknown == this);
    }

    inline category const& category::fit(std::string const& n) {
      boost::array < category const*, 5 >::const_iterator i = std::find_if(categories.begin(), categories.end(),
                  boost::bind(std::equal_to< std::string const >(), n, boost::bind(&category::name, _1)));

      if (i != categories.end()) {
        return (**i);
      }

      return (unknown);
    }

    /** \brief Conversion to print category to a standard stream */
    inline std::ostream& operator <<(std::ostream& s, category const& c) {
      return (s << c.name);
    }

    inline bool category::operator <(category const& c) const {
      return (name < c.name);
    }

    inline bool category::operator ==(category const& c) const {
      return (name == c.name);
    }

    inline category::operator std::string() const {
      return (name);
    }
  }
}

#endif
