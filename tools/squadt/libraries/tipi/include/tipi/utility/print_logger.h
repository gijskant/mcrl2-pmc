//  Copyright 2007 Jeroen van der Wulp. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/// \file include/tipi/utility/print_logger.h

#ifndef PRINT_LOGGER_H_
#define PRINT_LOGGER_H_

#include <iostream>
#include <fstream>

#include <boost/filesystem/path.hpp>

#include "logger.h"

namespace utility {

  /** \brief Simple logger that prints to a standard stream */
  class print_logger : public logger {

    private:

      /** \brief The stream to which the output should be printed */
      std::ostream& stream;

    private:

      /** \brief Adds a log message with a string and a log level */
      inline void actually_log(std::string const&);

      /** \brief Adds a log message with a format object and a log level */
      inline void actually_log(boost::format const&);

    public:
     
      /** \brief Constructor */
      inline print_logger(std::ostream&, log_level = logger::default_filter_level);

      /** \brief Destructor */
      inline ~print_logger();
  };

  class file_print_logger : public print_logger {

    private:

      /** \brief The stream to print to */
      std::ofstream out;

    public:

      /** \brief Constructor */
      file_print_logger(boost::filesystem::path const& p, log_level = logger::default_filter_level);

      /** \brief Destructor */
      ~file_print_logger();
  };

  /**
   * \param[in,out] s the stream on which to print during the lifetime of the object
   * \param[in] l the log level below which all messages will be printed
   **/
  inline print_logger::print_logger(std::ostream& s, log_level l) : logger(l), stream(s) {
  }

  /**
   * \param[in] m the message content
   **/
  inline void print_logger::actually_log(std::string const& m) {
    stream << m;
  }

  /**
   * \param[in] m the message content
   **/
  inline void print_logger::actually_log(boost::format const& m) {
    stream << m;
  }

  inline print_logger::~print_logger() {
  }

  /**
   * \param[in] p the path to the file to which to write the log messages
   * \param[in] l the log filter level
   **/
  inline file_print_logger::file_print_logger(boost::filesystem::path const& p, log_level l) : print_logger(out, l), out(p.native_file_string().c_str()) {
  }

  inline file_print_logger::~file_print_logger() {
    out.close();
  }
}

#endif
