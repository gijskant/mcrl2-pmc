#include <algorithm>
#include <ostream>
#include <deque>
#include <functional>
#include <string>
#include <list>

#include <boost/bind.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/foreach.hpp>

#include <utilities/generic_visitor.h>

#include "executor.tcc"

namespace squadt {
  namespace execution {

    inline executor_impl::executor_impl(unsigned int m) : maximum_instance_count(m) {
    }
    
    /**
     * \param[in] p the process to remove
     **/
    inline void executor_impl::remove(process* p) {
      static boost::mutex lock;

      boost::mutex::scoped_lock w(lock);

      std::list < process::sptr >::iterator i = std::find_if(processes.begin(), processes.end(),
                              boost::bind(std::equal_to < process* >(), p,
                                      boost::bind(&process::sptr::get, _1)));

      if (i != processes.end()) {
        processes.erase(i);
      }
    }
 
    /**
     * \param[in] c the command to execute
     * \param[in] w a pointer to the associated implementation object
     **/
    inline void executor_impl::start_process(const command& c, boost::shared_ptr < executor_impl >& w) {
      process::sptr p(new process(boost::bind(&executor_impl::handle_process_termination, this, _1, w)));

      processes.push_back(p);

      p->execute(c);
    }

    /**
     * \param[in] c the command to execute
     * \param[in] l reference to a process listener
     * \param[in] w a pointer to the associated implementation object
     **/
    inline void executor_impl::start_process(const command& c, task_monitor::sptr& l, boost::shared_ptr < executor_impl >& w) {
      process::sptr p(new process(boost::bind(&executor_impl::handle_process_termination, this, _1, w), l));

      if (l.get() != 0) {
        l->attach_process(p);
      }

      processes.push_back(p);

      l->get_logger()->log(1, "executing command `" + c.as_string() + "'\n");

      p->execute(c);
    }

    /**
     * \param c the command that is to be executed
     * \param l a shared pointer a listener (or reference to) for process state changes
     * \param b whether or not to circumvent the number of running processes limit
     **/
    inline void executor_impl::execute(const command& c, boost::shared_ptr < task_monitor >& l, bool b, boost::shared_ptr < executor_impl >& w) {
      if (b || processes.size() < maximum_instance_count) {
        boost::thread t(boost::bind(&executor_impl::start_process, this, c, l, w));
      }
      else {
        /* queue command for later execution */
        delayed_commands.push_back(command_pair(c, l));
      }
    }

    /**
     * \param c the command that is to be executed
     * \param b whether or not to circumvent the number of running processes limit
     **/
    inline void executor_impl::execute(command const& c, bool b, boost::shared_ptr < executor_impl >& w) {
      boost::shared_ptr < task_monitor > p;

      if (b || processes.size() < maximum_instance_count) {
        boost::thread t(boost::bind(&executor_impl::start_process, this, c, p, w));
      }
      else {
        /* queue command for later execution */
        delayed_commands.push_back(std::make_pair(c, p));
      }
    }

    /**
     * The queue with commands for to be started processes is also cleared by this function.
     **/
    inline void executor_impl::terminate_all() {
    
      delayed_commands.clear();

      BOOST_FOREACH(process::sptr p, processes) {
        p->terminate();
      }
    }

    /**
     * Start processing commands if the queue contains any waiters
     *
     * \param[in] w a pointer to the associated implementation object
     **/
    inline void executor_impl::start_delayed(boost::shared_ptr < executor_impl >& w) {
      if (0 < delayed_commands.size()) {
        command_pair c = delayed_commands.front();
 
        delayed_commands.pop_front();
 
        start_process(c.first, c.second, w);
      }
    }
    
    size_t executor_impl::get_maximum_instance_count() const {
      return (maximum_instance_count);
    }
 
    void executor_impl::set_maximum_instance_count(size_t m) {
      maximum_instance_count = m;
    }
 
    /**
     * \param p a pointer to a process object
     **/
    inline void executor_impl::handle_process_termination(process* p, boost::weak_ptr < executor_impl > w) {
      boost::shared_ptr < executor_impl > g(w.lock());

      if (g.get() != 0) {
        remove(p);
 
        start_delayed(g);
      }
    }

    executor::executor(unsigned int const& m) : impl(new executor_impl(m)) {
    }
    
    executor::~executor() {
      impl->terminate_all();
    }

    size_t executor::get_maximum_instance_count() const {
      return (impl->maximum_instance_count);
    }
 
    void executor::set_maximum_instance_count(size_t m) {
      impl->maximum_instance_count = m;
    }
 
    /**
     * \param[in] p the process to remove
     **/
    void executor::remove(process* p) {
      impl->remove(p);
    }

    /**
     * \param[in] c the command to execute
     **/
    void executor::start_process(const command& c) {
      impl->start_process(c, impl);
    }
 
    /**
     * \param[in] c the command to execute
     * \param[in] l reference to a process listener
     **/
    void executor::start_process(const command& c, task_monitor::sptr& l) {
      impl->start_process(c, l, impl);
    }

    /**
     * The queue with commands for to be started processes is also cleared by this function.
     **/
    void executor::terminate_all() {
      impl->terminate_all();
    }
 
    /* Start processing commands if the queue contains any waiters */
    void executor::start_delayed() {
      impl->start_delayed(impl);
    }
 
    /**
     * \param[in] c the command that is to be executed
     * \param[in] l a shared pointer a listener (or reference to) for process state changes
     * \param[in] b whether or not to circumvent the number of running processes limit
     **/
    void executor::execute(command const& c, boost::shared_ptr < task_monitor >& l, bool b) {
      impl->execute(c, l, b, impl);
    }

    /**
     * \param[in] c the command that is to be executed
     * \param[in] b whether or not to circumvent the number of running processes limit
     **/
    void executor::execute(command const& c, bool b) {
      impl->execute(c, b, impl);
    }
  }
}

