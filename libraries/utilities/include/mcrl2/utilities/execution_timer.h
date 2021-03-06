// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/utilities/execution_timer.h
/// \brief Class to obtain running times of code.

#ifndef MCRL2_UTILITIES_EXECUTION_TIMER_H
#define MCRL2_UTILITIES_EXECUTION_TIMER_H

#include <ctime>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include "mcrl2/utilities/exception.h"

namespace mcrl2
{

namespace utilities
{

/// \brief Simple timer to time the CPU time used by a piece of code.
///
/// Example usage:
/// execution_timer timer("test_tool", "/path/to/file")
/// timer.start("hint")
/// ... (execute some code here) ...
/// timer.finish("hint")
/// timer.report()
///
/// This will output the following to the file "/path/to/file", or
/// standard error if filename is empty, assuming that the time between
/// start and finish is n seconds (with precision of 2 decimal places).
/// - tool: test_tool
///   timing:
///     hint: n
///
/// Note that this is an output format that can immediately be parsed using
/// YAML (http://www.yaml.org/)
class execution_timer
{
  protected:

    /// \brief Pair of start and finish times
    struct timing
    {
      clock_t start;
      clock_t finish;

      timing() :
        start(0),
        finish(0)
      {}
    };

    std::string m_tool_name; //!< name of the tool we are timing
    std::string m_filename; //!< name of the file to write timings to
    std::map<std::string, timing> m_timings; //!< collection of timings

    /// \brief Write the report to an output stream.
    /// \param[in] s The output stream to which the report is written.
    void write_report(std::ostream& s)
    {
      s.setf(std::ios::fixed, std::ios::floatfield); // Print floats in 2 decimals
      s.precision(2);

      s << "- tool: " << m_tool_name << std::endl
        << "  timing:" << std::endl;

      for (std::map<std::string, timing>::const_iterator i = m_timings.begin(); i != m_timings.end(); ++i)
      {
        if (i->second.start > i->second.finish)
        {
          throw mcrl2::runtime_error("Start of " + i->first + " occurred after finish");
        }

        s << "    " << i->first << ": "
          << (static_cast<float>(i->second.finish - i->second.start))/CLOCKS_PER_SEC
          << std::endl;
      }
    }

  public:

    /// \brief Constructor of a simple execution timer
    /// \param[in] tool_name Name of the tool that does the measurements
    /// \param[in] filename Name of the file to which the measurements are written
    execution_timer(std::string const& tool_name = "", std::string const& filename = "") :
      m_tool_name(tool_name),
      m_filename(filename)
    {}

    /// \brief Destructor
    ~execution_timer()
    {}

    /// \brief Start measurement with a hint
    /// \param[in] timing_name Name of the measurement being started
    /// \pre No start(timing_name) has occurred before
    /// \post The current time has been recorded as starting time of timing_name
    void start(std::string const& timing_name)
    {
      if (m_timings.find(timing_name) != m_timings.end())
      {
        throw mcrl2::runtime_error("Starting already known timing; causes unreliable results");
      }
      timing t;
      t.start = clock();
      m_timings[timing_name] = t;
    }

    /// \brief Finish a measurement with a hint
    /// \param[in] timing_name Name of the measurment being finished
    /// \pre A start(timing_name) was executed before
    /// \post The current time has been recorded as end time of timing_name
    void finish(std::string const& timing_name)
    {
      if (m_timings.find(timing_name) == m_timings.end())
      {
        throw mcrl2::runtime_error("Finishing timing that was not started");
      }
      m_timings[timing_name].finish = clock();
    }

    /// \brief Write all timing information that has been recorded.
    ///
    /// Timing information is written to the filename that was provided in
    /// the constructor. If no filename was provided (i.e. the filename is
    /// empty) the information is written to standard error.
    /// The output is in YAML compatible format.
    void report()
    {
      if (m_filename.empty())
      {
        write_report(std::cerr);
      }
      else
      {
        std::ofstream out;
        out.open(m_filename.c_str(), std::ios::app);
        write_report(out);
        out.close();
      }
    }

};

} // namespace utilities

} // namespace mcrl2

#endif //MCRL2_UTILITIES_EXECUTION_TIMER_H
