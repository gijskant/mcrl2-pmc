#include <algorithm>
#include <functional>
#include <sstream>

#include <sip/detail/basic_messenger.h>
#include <sip/detail/exception.h>

namespace sip {

  namespace messenger {

    template < class M >
    const std::string basic_messenger< M >::tag_open("<message>");

    template < class M >
    const std::string basic_messenger< M >::tag_close("</message>");

    template < class M >
    void basic_messenger< M >::deliver(std::istream& data) {
      std::ostringstream s;
 
      s << data.rdbuf() << std::flush;
 
      std::string content = s.str();
 
      deliver(content);
    }

    /* Send a message */
    template < class M >
    inline void basic_messenger< M >::send_message(const message& m) {
      send(tag_open + m.to_xml() + tag_close);
    }
 
    /** \pre the message queue is not empty */
    template < class M >
    inline M basic_messenger< M >::pop_message() {
      message_ptr m = message_queue.front();
 
      message_queue.pop_front();
 
      return (*m);
    }
 
    /** \pre the message queue is not empty  */
    template < class M >
    inline M& basic_messenger< M >::peek_message() {
      message_ptr m = message_queue.front();
 
      message_queue.front();
 
      return (*m);
    }
 
    template < class M >
    inline size_t basic_messenger< M >::number_of_messages() {
      return (message_queue.size());
    }

    /** \pre there is no waiter for this type */
    template < class M >
    inline void basic_messenger< M >::set_handler(handler_type h, const typename M::type_identifier_t t) {
      assert(waiters.count(t) == 0);

      handlers[t] = h;
    }

    template < class M >
    inline void basic_messenger< M >::unset_handler(const typename M::type_identifier_t t) {
      handlers.erase(t);
    }
 
    /**
     * \pre A message is of the form tag_open...tag_close
     * \pre Both tag_close == tag_open start with == '<' and do not further contain this character 
     *
     * \attention Works under the assumption that tag_close.size() < data.size()
     **/
    template < class M >
    void basic_messenger< M >::deliver(std::string& data) {
      std::string::const_iterator i = data.begin();
 
      while (i != data.end()) {
        std::string::const_iterator j = i;
 
        if (message_open) {
          /* The start message tag was matched before */

          if (0 < partially_matched) {
            /* A prefix of the close message tag was matched before */
            j              = std::mismatch(tag_close.begin() + partially_matched, tag_close.end(), j).first;

            const size_t c = (j - tag_close.begin()) - partially_matched;

            if (j == tag_close.end()) {
              /* Signal that message is closed */
              message_open = false;

              /* Remove previously matched part from buffer */
              buffer.resize(buffer.size() - c);

              i += tag_close.size() - c;
            }

            partially_matched = 0;
          }

          if (message_open) {
            /* Continuing search for the end of the current message; next: try to match close tag */
            size_t n = data.find(tag_close, i - data.begin());
           
            if (n != std::string::npos) {
              /* End message sequence matched; signal message close */
              message_open = false;

              j = data.begin() + n;

              /* Append data to buffer */
              buffer.append(i, j);

              /* Skip close tag */
              i = j + tag_close.size();
            }
            else {
              const std::string::const_iterator b = data.end();
              const size_t                      s = data.size() - std::min(tag_close.size(), data.size());

              /* End message sequence not matched look for partial match in data[(i - tag_close.size())..i] */
              n = data.substr(s).rfind('<');

              if (n != std::string::npos) {
                const std::string::const_iterator k = data.begin() + s + n;
               
                j = std::mismatch(k, b, tag_close.begin()).first;
               
                if (j == b) {
                  partially_matched = (j - k);
                }
              }

              /* Append */
              buffer.append(i, b);

              i = b;
            }
          }

          if (!message_open) {
            typename M::type_identifier_t t;

            /* End message sequence matched; move message from buffer to queue  */
            std::string new_string;

            new_string.swap(buffer);

            t = M::extract_type(new_string);

            message_ptr m(new message(t));

            m->set_content(new_string);

            typename handler_map::iterator h = handlers.find(t);

            if (h != handlers.end()) {
              /* Service handler */
              (*h).second(m);
            }
            else {
              /* Put message into queue */
              message_queue.push_back(m);

              /* Unblock waiters, if necessary */
              if (waiters.count(t) != 0) {
                barrier_ptr b = waiters[t];
                
                waiters.erase(t);

                b->wait();
              }
            }
          }
        }
        else {
          if (0 < partially_matched) {
            const std::string::const_iterator k = tag_open.begin() + partially_matched;

            /* Part of a start message tag was matched */
            j = std::mismatch(k, tag_open.end(), i).first;

            assert (j == tag_open.end());

            if (j == tag_open.end()) {
              i = data.begin() + tag_open.size() - partially_matched;

              message_open = true;
            }

            partially_matched = 0;
          }

          if (!message_open) {
            size_t n = data.find(tag_open, i - data.begin());
           
            if (n != std::string::npos) {
              /* Skip message tag */
              i += n + tag_open.size();
           
              message_open = true;
            }
            else {
              const std::string::const_iterator b = data.end();
              const size_t                      s = data.size() - std::min(tag_open.size(), data.size());

              n = data.substr(s).rfind('<');

              if (n != std::string::npos) {
                const std::string::const_iterator k = data.begin() + s + n;
               
                /* End message sequence not matched look for partial match in data[(i - tag_close.size())..i] */
                j = std::mismatch(k, b, tag_open.begin()).first;
               
                if (j == b) {
                  partially_matched = (j - k);
                }
              }

              i = b;
            }
          }
        }
      }
    }
 
    template < class M >
    inline bool basic_messenger< M >::find_message(typename M::type_identifier_t t) {
      using namespace boost;

      return (message_queue.end() !=
              std::find_if(message_queue.begin(), message_queue.end(),
                      bind(std::equal_to<typename M::type_identifier_t>(), t,
                              bind(&M::get_type,
                                      bind(&message_ptr::get, _1)))));
    }

    /**
     * \pre no handler or other waiter for this type is registered
     * \attention must not be called from multiple threads for the same type
     **/
    template < class M >
    void basic_messenger< M >::await_message(typename M::type_identifier_t t) {
      assert(waiters.count(t) == 0 && handlers.count(t) == 0);

      if (!find_message(t)) {
        barrier_ptr b(new boost::barrier(2));

        waiters[t] = b;

        b->wait();
      }
    }
  }
}
