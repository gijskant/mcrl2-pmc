//  Copyright 2007 Jeroen van der Wulp. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/// \file include/tipi/detail/event_handlers.h

#ifndef EVENT_HANDLERS_H
#define EVENT_HANDLERS_H

#include <map>
#include <utility>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace tipi {
  namespace layout {

    /**
     * \brief A basic event handler that executes other event handlers that can be registered
     *
     * Multiple handlers for the same id are invoked in arbitrary order.
     *
     * \attention The event handler should always outlive execution of all event handlers.
     **/
    class basic_event_handler {
      public:
   
        /** \brief Basic type of event handler functions */
        typedef boost::function < void (const void*) > handler_function;
   
      private:
   
        /** \brief Type for the map that associates a number of handlers to a pointer */
        typedef std::multimap < const void*, handler_function >                   handler_map;
   
        /** \brief Type for the map that associates a number of handlers to a pointer */
        typedef std::map < const void*, boost::shared_ptr < boost::condition > >  waiter_map;
   
      private:
   
        /** \brief For waiter events, and for processing events in a mutual exclusive manner */
        boost::mutex      lock;

        /** \brief The list of functions that are to be executed  */
        handler_function  global_handler;
   
        /** \brief The list of functions that are to be executed  */
        handler_map       handlers;
   
        /** \brief The synchronisation constructs for waking up waiters */
        waiter_map        waiters;
   
      private:
   
        /** \brief Wakes up all waiters that match an identifier or all waiters if the identifier is 0 */
        void wake(const void* = 0);
   
        /** \brief Execute handlers for a specific object */
        void execute_handlers(const void*, bool);
   
      public:

        /** \brief Set a global handler */
        void add(handler_function);
   
        /** \brief Register an arbitrary handler for a specific object */
        void add(const void*, handler_function);

        /** \brief Moves registered event handlers that match the id to another object */
        void transfer(basic_event_handler&, const void* = 0);

        /** \brief Remove the global handler */
        void remove();

        /** \brief Remove the handlers for a specific object */
        void remove(const void*);

        /** \brief Process an event for a specific object */
        void process(const void*, bool = true);
   
        /** \brief Block until the next event has been processed */
        void await_change(const void*);
   
        /** \brief Destructor */
        ~basic_event_handler();
    };
   
    /**
     * @param[in] id a pointer that serves as an identifier for the originator of the event
     *
     * \pre lock is in the locked state
     **/
    inline void basic_event_handler::wake(const void* id) {
      waiter_map::iterator w = waiters.find(id);
   
      if (w != waiters.end()) {
        (*w).second->notify_all();
   
        waiters.erase(w);
      }
    }
   
    /**
     * @param[in] h a function object that is to be invoked at an event
     **/
    inline void basic_event_handler::add(handler_function h) {
      boost::mutex::scoped_lock l(lock);
   
      global_handler = h;
    }
   
    /**
     * @param[in] id a pointer that serves as an identifier for the originator of the event
     * @param[in] h a function object that is to be invoked at an event
     **/
    inline void basic_event_handler::add(const void* id, handler_function h) {
      boost::mutex::scoped_lock l(lock);
   
      handlers.insert(std::make_pair(id, h));
    }
   
    /**
     * @param[in] e the target event handler object to which the handlers should be relocated
     * @param[in] id a pointer that serves as an identifier for the originator of the event
     **/
    inline void basic_event_handler::transfer(basic_event_handler& e, const void* id) {
      if (&e != this) {
        boost::mutex::scoped_lock l(lock);
        boost::mutex::scoped_lock k(e.lock);
      
        std::pair < handler_map::iterator, handler_map::iterator > p = handlers.equal_range(id);
      
        e.handlers.insert(p.first, p.second);
      
        handlers.erase(p.first, p.second);
      
        waiter_map::iterator w = waiters.find(id);
      
        if (w != waiters.end()) {
          /* Transfer waiters */
          e.waiters.insert(*w);
      
          waiters.erase(w);
        }
      }
    }
   
    /**
     * @param[in] id a pointer that serves as an identifier for the originator of the event
     * \param[in] b whether or not to execute the global handler
     **/
    inline void basic_event_handler::process(const void* id, bool b) {
      if (0 < handlers.count(id) || !global_handler.empty()) {
        boost::thread t(boost::bind(&basic_event_handler::execute_handlers, this, id, b));
      }
      else if (0 < waiters.size()) {
        boost::mutex::scoped_lock l(lock);

        wake(id);
      }
    }

    /**
     * \param[in] id a pointer that serves as an identifier for the originator of the event
     * \param[in] b whether or not to execute the global handler
     **/
    inline void basic_event_handler::execute_handlers(const void* id, bool b) {
      boost::mutex::scoped_lock l(lock);

      if (!global_handler.empty() && b) {
        global_handler(id);
      }
   
      std::pair < handler_map::const_iterator, handler_map::const_iterator > range(handlers.equal_range(id));
   
      BOOST_FOREACH(handler_map::value_type p, range) {
        p.second(id);
      }
   
      if (0 < waiters.size()) {
        wake(id);
      }
    }
   
    inline void basic_event_handler::remove() {
      boost::mutex::scoped_lock l(lock);
   
      global_handler.clear();
    }

    /**
     * @param[in] id a pointer that serves as an identifier for the originator of the event
     **/
    inline void basic_event_handler::remove(const void* id) {
      boost::mutex::scoped_lock l(lock);
   
      std::pair < handler_map::iterator, handler_map::iterator > p = handlers.equal_range(id);
   
      handlers.erase(p.first, p.second);

      wake(id);
    }
   
    /**
     * @param[in] id a pointer that serves as an identifier for the originator of the event
     *
     * \attention also unblocks if the object is destroyed
     **/
    inline void basic_event_handler::await_change(const void* id) {
      boost::mutex::scoped_lock l(lock);
   
      boost::shared_ptr < boost::condition > anchor;

      waiter_map::iterator w = waiters.find(id);
   
      if (w == waiters.end()) {
        /* Create new waiter */
        anchor = boost::shared_ptr < boost::condition > (new boost::condition);

        waiters[id] = anchor;
      }
      else {
        anchor = (*w).second;
      }
   
      /* The condition with which synchronisation is performed */
      anchor->wait(l);
    }
   
    inline basic_event_handler::~basic_event_handler() {
      boost::mutex::scoped_lock l(lock);

      wake();
    }
  }
}
#endif
