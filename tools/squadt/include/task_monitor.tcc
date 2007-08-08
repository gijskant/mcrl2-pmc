// Author(s): Jeroen van der Wulp
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file task_monitor.tcc
/// \brief Add your file description here.

#include "task_monitor.hpp"

#include "tipi/detail/controller.tcc"

namespace squadt {
  namespace execution {
    /// \cond INTERNAL_DOCS

    class task_monitor_impl : public tipi::controller::communicator_impl {
      friend class task_monitor;

      template < typename S, typename T >
      friend class utility::visitor;

      private:
 
        /** \brief Type for event distinction */
        enum event_type {
          change,     ///< Status of the associated process has changed
          connection, ///< A new connection has been established
          completion  ///< A tool completed its operation
        };
       
        /** \brief The type for a map that contains the event handlers */
        typedef std::multimap < const event_type, boost::function < bool () > >  handler_map;

      private:
 
        /** \brief Flag that a connection has been established */
        bool                                            connected;

        /** \brief Flag that a connection has been established */
        bool                                            done;

        /** \brief Semaphore to guarantee mutual exclusion (for use with register_condition) */
        mutable boost::mutex                            register_lock;

        /** \brief Monitor for waiting until process has registered */
        mutable boost::condition                        register_condition;
        
        /** \brief A pointer to the process associated to this listener or 0 */
        process::sptr                                   associated_process;

        /** \brief The event handlers that have been registered */
        handler_map                                     handlers;

      private:

        /** \brief Constructor */
        inline task_monitor_impl();
 
        /** \brief Executes a handler a specified number of times and then */
        inline static bool countdown_handler_wrapper(boost::shared_ptr < unsigned int >, boost::function < void () >);

        /** \brief Executes a handler until it is manually removed */
        inline static bool perpetual_handler_wrapper(boost::function < void () >);

        /** \brief Terminate an associated process */
        inline void terminate_process();

        /** \brief Blocks until the process has registered */
        inline void await_process() const;

        /** \brief Waits until a connection has been established with the running process */
        inline void await_connection(unsigned int const&);

        /** \brief Waits until a connection has been established with the running process */
        inline void await_connection();

        /** \brief Waits until the current task has been completed */
        inline bool await_completion();

        /** \brief Helper method used for unblocking the await_completion member */
        void handle_task_completion(boost::shared_ptr < tipi::message > const&, bool&, bool&);

        /** \brief Signals that a new connection has been established */
        inline void signal_connection(boost::shared_ptr < task_monitor_impl >&, tipi::message::end_point);

        /** \brief Checks the process status and removes */
        inline void signal_change(boost::shared_ptr < task_monitor_impl >&, const execution::process::status);

        /** \brief Execute event handlers */
        static void service_handlers(boost::shared_ptr < task_monitor_impl >&, event_type);

        /** \brief Executes a handler function once the tool has connected */
        inline void on_connection(boost::function < void () >);

        /** \brief Executes a handler function once the tool has connected */
        inline void once_on_connection(boost::function < void () >);

        /** \brief Executes a handler function once the tool has connected */
        inline void on_completion(boost::function < void () >);

        /** \brief Executes a handler function once on tool completion */
        inline void once_on_completion(boost::function < void () >);

        /** \brief Executes a handler function once status change */
        inline void on_status_change(boost::function < void () >);

        /** \brief Associates a process with this listener */
        void attach_process(const process::sptr& p);

        /** \brief Terminate communication and reset internal state */
        inline void disconnect();

        /** \brief Unblocks waiters and requests a tool to prepare termination */
        inline void finish(bool = false);

        /** \brief Clears handlers and terminates processes */
        inline void shutdown();
    };

    task_monitor_impl::task_monitor_impl() : tipi::controller::communicator_impl(), connected(false), done(false) {
    }

    /**
     * @param[in] n the amount of times that the handler is executed
     * @param[in] h the function object that is executed once a connection is established
     **/
    inline bool task_monitor_impl::countdown_handler_wrapper(boost::shared_ptr < unsigned int > n, boost::function < void () > h) {
      h();

      return (--*n == 0); 
    }

    /**
     * @param[in] h the function object that is executed once a connection is established
     **/
    inline bool task_monitor_impl::perpetual_handler_wrapper(boost::function < void () > h) {
      h();

      return (false); 
    }

    inline void task_monitor_impl::await_process() const {
      boost::mutex::scoped_lock l(register_lock);

      if (associated_process.get() == 0) {
        register_condition.wait(l);
      }
    }

    /**
     * Waits until a connection has been established a timeout has occurred, or the process has terminated
     **/
    inline void task_monitor_impl::await_connection(unsigned int const& ts) {
      boost::mutex::scoped_lock l(register_lock);
 
      if (!connected) {
        boost::xtime time;

        xtime_get(&time, boost::TIME_UTC);

        time.sec += ts;

        while (associated_process.get() == 0 || !connected) {
          /* Other side has not connected and the process has not been registered as terminated */
          if (!register_condition.timed_wait(l, time)) {
            /* Timeout occurred */
            break;
          }
        }
      }
    }

    /**
     * Waits until a connection has been established, or the process has terminated
     **/
    inline void task_monitor_impl::await_connection() {
      boost::mutex::scoped_lock l(register_lock);
 
      while (!connected || associated_process.get() == 0) {
        /* Other side has not connected and the process has not been registered as terminated */
        register_condition.wait(l);
      }
    }

    // Declaring this function inline produces unexpected results (at least on Linux with gcc 4)
    void task_monitor_impl::handle_task_completion(boost::shared_ptr < tipi::message > const& m, bool& result, bool& changed) {
      changed = true;

      result = m.get() && !m->is_empty();

      boost::mutex::scoped_lock l(register_lock);

      register_condition.notify_all();
    }

    /**
     * Waits until a connection has been established, or the process has terminated
     * \return whether the task has been completed successfully
     **/
    inline bool task_monitor_impl::await_completion() {
      bool result  = false;
      bool changed = false;
 
      boost::mutex::scoped_lock l(register_lock);

      add_handler(tipi::message_task_done, boost::bind(&task_monitor_impl::handle_task_completion, this, _1, boost::ref(result), boost::ref(changed)));

      while (!changed) {
        /* Other side has not connected and the process has not been registered as terminated */
        register_condition.wait(l);
      }

      return result;
    }

    /**
     * \param[in] m a shared pointer to the current object
     **/
    inline void task_monitor_impl::signal_connection(boost::shared_ptr < task_monitor_impl >& m, tipi::message::end_point) {
      boost::mutex::scoped_lock l(register_lock);
 
      connected = true;
      done      = false;
 
      register_condition.notify_all();

      /* Service connection handlers */
      if (0 < handlers.count(completion)) {
        task_monitor_impl::service_handlers(m, connection);
      }
    }

    inline void task_monitor_impl::shutdown() {
      boost::mutex::scoped_lock l(register_lock);

      handlers.clear();

      /* Signal completion to waiters */
      register_condition.notify_all();
    }

    /**
     * @param[in] m a shared pointer to the current object
     * @param[in] s the current status of the process
     **/
    inline void task_monitor_impl::signal_change(boost::shared_ptr < task_monitor_impl >& m, const execution::process::status s) {
      boost::mutex::scoped_lock l(register_lock);

      /* Service change handlers */
      if (0 < handlers.count(change)) {
        task_monitor_impl::service_handlers(m, change);
      }

      if (s == execution::process::completed || s == execution::process::aborted) {
        /* Unblock any remaining waiters */
        done = true;
 
        /* Service connection handlers */
        if (0 < handlers.count(completion)) {
          task_monitor_impl::service_handlers(m, completion);
        }

        /* Signal completion to waiters */
        register_condition.notify_all();
      }
    }

    /**
     * @param[in] m a shared pointer to this object
     * @param[in] e the event type of which to execute the handler
     **/
    inline void task_monitor_impl::service_handlers(boost::shared_ptr < task_monitor_impl >& m, const event_type e) {
      std::pair < handler_map::iterator, handler_map::iterator > p = m->handlers.equal_range(e);
      
      for (handler_map::iterator i = p.first; i != p.second; ++i) {
        if (i->second()) {
          m->handlers.erase(i);
        }
      }
    }

    /**
     * @param[in] h the function object that is executed once a connection is established
     **/
    inline void task_monitor_impl::on_connection(boost::function < void () > h) {
      handlers.insert(std::make_pair(connection, boost::bind(&task_monitor_impl::perpetual_handler_wrapper, h)));
    }

    /**
     * @param[in] h the function object that is executed once a connection is established
     **/
    inline void task_monitor_impl::once_on_connection(boost::function < void () > h) {
      boost::shared_ptr < unsigned int > n(new unsigned int);

      *n = 1;

      handlers.insert(std::make_pair(connection, boost::bind(&task_monitor_impl::countdown_handler_wrapper, n, h)));
    }

    /**
     * @param[in] h the function object that is executed after the process status has changed
     **/
    inline void task_monitor_impl::on_status_change(boost::function < void () > h) {
      handlers.insert(std::make_pair(change, boost::bind(&task_monitor_impl::perpetual_handler_wrapper, h)));
    }

    /**
     * @param[in] h the function object that is executed once a connection is established
     **/
    inline void task_monitor_impl::on_completion(boost::function < void () > h) {
      handlers.insert(std::make_pair(completion, boost::bind(&task_monitor_impl::perpetual_handler_wrapper, h)));
    }

    /**
     * @param[in] h the function object that is executed once a connection is established
     **/
    inline void task_monitor_impl::once_on_completion(boost::function < void () > h) {
      boost::shared_ptr < unsigned int > n(new unsigned int);

      *n = 1;

      handlers.insert(std::make_pair(completion, boost::bind(&task_monitor_impl::countdown_handler_wrapper, n, h)));
    }

    /**
     * @param[in] p shared pointer to the process
     **/
    inline void task_monitor_impl::attach_process(const process::sptr& p) {
      boost::mutex::scoped_lock l(register_lock);

      /* Wake up waiting threads */
      register_condition.notify_all();

      associated_process = p;
    }

    /**
     * \param[in] b whether to wait for the process to terminate (whether or not to block)
     **/
    inline void task_monitor_impl::finish(bool b) {
      /* Let the tool know that it should prepare for termination */
      if (b) {
        terminate_process();
      }
      else {
        boost::thread t(boost::bind(&task_monitor_impl::terminate_process, this));
      }
    }

    inline void task_monitor_impl::disconnect() {
      if (associated_process.get() && associated_process->get_status() == process::running && connected) {
        // request termination
        send_message(tipi::message_termination);

        logger->log(1, boost::str(boost::format("termination request sent to %s pid(%u)\n")
                  % associated_process->get_executable_name() % associated_process->get_identifier()));

        await_message(tipi::message_termination, 1);

        tipi::controller::communicator_impl::disconnect();
      }

      connected = false;
    }

    /** \brief Terminates a running process */
    inline void task_monitor_impl::terminate_process() {
      if (connected) {
        if (associated_process && associated_process->get_status() == process::running) {
          disconnect();

          boost::xtime timeout;
          boost::xtime_get(&timeout, boost::TIME_UTC);
          timeout.sec += 1;

          boost::thread::sleep(timeout);
        }
      }
      if (associated_process.get() != 0) {
        std::string log_message(boost::str(
                boost::format("process terminated (tool %s pid(%u))\n") %
                        associated_process->get_executable_name() %
                        associated_process->get_identifier()));

        if (associated_process->terminate()) {
          logger->log(1,log_message);
        }
      }

      boost::mutex::scoped_lock l(register_lock);

      connected = false;
      done      = true;

      /* Signal completion to waiters */
      register_condition.notify_all();
    }

    /// \endcond
  }
}
