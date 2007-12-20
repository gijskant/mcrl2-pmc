//  Copyright 2007 Jeroen van der Wulp. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/// \file tipi/detail/utility/logger.hpp

#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>

#include <boost/format.hpp>

#if defined(_MSC_VER) || defined(__MINGW32__)
# include <workarounds.h>
#endif // _MSC_VER

namespace tipi {
  namespace utility {

    /** \brief Minimal interface for a logger component */
    class logger {
 
      public:
 
        /** \brief Type for log level specification */
        typedef unsigned char log_level;
 
      protected:
 
        /** \brief The log level below which all messages are printed */
        log_level        filter_level;
 
      protected:
 
        /** \brief The default log level */
        static logger::log_level default_filter_level;
 
      private:
       
        /** \brief Adds a log message with a string and a log level */
        virtual void actually_log(std::string const& m) = 0;
 
        /** \brief Adds a log message with a format object and a log level */
        virtual void actually_log(boost::format const&) = 0;
 
      public:
 
        /** \brief Constructor */
        inline logger(log_level = default_filter_level);
 
        /** \brief Adds a log message with a string and a log level */
        inline void log(log_level, std::string const&);
 
        /** \brief Adds a log message with a format object and a log level */
        inline void log(log_level, boost::format const&);
 
        /** \brief Sets default filter level */
        inline static void set_default_filter_level(log_level l);
 
        /** \brief Sets default filter level */
        inline static log_level get_default_filter_level();
 
        /** \brief Sets filter level below which messages are logged */
        inline void set_filter_level(log_level l);
 
        /** \brief Returns filter level below which messages are logged */
        inline log_level get_filter_level();
 
        /** \brief Destructor */
        virtual ~logger() = 0;
    };
 
    /**
     * \param[in] l log level
     **/
    inline logger::logger(log_level l) : filter_level(l) {
    }
 
    /**
     * \param[in] l log level
     **/
    inline void logger::set_default_filter_level(log_level l) {
      default_filter_level = l;
    }
 
    inline logger::log_level logger::get_default_filter_level() {
      return (default_filter_level);
    }
 
    /**
     * \param[in] l log level
     **/
    inline void logger::set_filter_level(log_level l) {
      filter_level = l;
    }
 
    inline logger::log_level logger::get_filter_level() {
      return (filter_level);
    }
 
    /**
     * @param[in] l the log level of the message
     * @param[in] m the message content
     **/
    inline void logger::log(log_level l, std::string const& m) {
      if (l < filter_level) {
        actually_log(m);
      }
    }
 
    /**
     * @param[in] l the log level of the message
     * @param[in] m the message content
     **/
    inline void logger::log(log_level l, boost::format const& m) {
      if (l < filter_level) {
        actually_log(m);
      }
    }
 
    inline logger::~logger() {
    }
  }
}

#endif
