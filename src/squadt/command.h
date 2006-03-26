#ifndef COMMAND_H
#define COMMAND_H

#include <boost/shared_array.hpp>

namespace squadt {
  namespace execution {

    /**
     * \brief Captures the concept of a command
     *
     * The class serves as a helper for platform independent process execution.
     * Sadly the most portable standard available popen() and pclose() give not
     * enough controll over a process. We need the ability to kill it. And on
     * the other hand system() is not thread safe, so it is unusable.
     **/
    class command {
      friend class process;
  
      private:
  
        /** \brief path to the program that is to be executed */
        const std::string            executable;
  
        /** \brief the arguments to the command */
        std::vector < std::string >  arguments;
  
      public:
  
        /** \brief Constructor */
        inline command(const std::string&);
  
        /** \brief Adds an argument */
        inline void append_argument(const std::string&);

        /** \brief Get arguments as a traditional C string */
        inline std::string argument_string(bool b = true) const;

        /** \brief Get arguments as an array of constant C strings */
        inline boost::shared_array < char const* > argument_array(bool b = true) const;
    };
  
    /**
     * @param e a complete path to an executable of the program that is to be executed
     **/
    inline command::command(const std::string& e) : executable(e) {
    }
  
    /**
     * @param a a single argument
     **/
    inline void command::append_argument(const std::string& a) {
      arguments.push_back(a);
    }
  
    inline std::string command::argument_string(bool b) const {
      std::string s;

      if (b) {
        s.append(executable).append(" ");
      }
  
      for (std::vector < std::string >::const_iterator i = arguments.begin(); i != arguments.end(); ++i) {
        s.append(*i).append(" ");
      }
  
      return (s);
    }
  
    /**
     * @param[in] b whether the first argument must be the executable or not
     *
     * \attention the array of arguments that is returned consists of 
     *   - the name of the executable, then
     *   - the arguments in the order in which they were added
     *   - a NULL pointer
     **/
    inline boost::shared_array < char const* > command::argument_array(bool b) const {
      boost::shared_array < char const* > p(new char const*[arguments.size() + 2]);

      char const** d = p.get();

      if (b) {
        *(d++) = executable.c_str();
      }

      for (std::vector < std::string >::const_iterator i = arguments.begin(); i != arguments.end(); ++i) {
        *(d++) = (*i).c_str();
      }

      *d = 0;

      return (p);
    }
  }
}

#endif
