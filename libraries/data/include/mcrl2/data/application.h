// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/application.h
/// \brief The class application.

// This is to avoid incomplete type errors when including this header standalone
#ifndef MCRL2_DATA_DATA_EXPRESSION_H
#include "mcrl2/data/data_expression.h"
#endif

#ifndef MCRL2_DATA_APPLICATION_H
#define MCRL2_DATA_APPLICATION_H

#include "mcrl2/atermpp/aterm_list.h"
#include "mcrl2/atermpp/make_list.h"
#include "mcrl2/atermpp/convert.h"

namespace mcrl2
{

namespace data
{

//--- start generated class application ---//
/// \brief An application of a data expression to a number of arguments
class application: public data_expression
{
  public:
    /// \brief Default constructor.
    application()
      : data_expression(core::detail::constructDataAppl())
    {}

    /// \brief Constructor.
    /// \param term A term
    application(const atermpp::aterm& term)
      : data_expression(term)
    {
      assert(core::detail::check_term_DataAppl(*this));
    }

    /// \brief Constructor.
    application(const data_expression& head, const data_expression_list& arguments)
      : data_expression(core::detail::gsMakeDataAppl(head, arguments))
    {}

    /// \brief Constructor.
    template <typename Container>
    application(const data_expression& head, const Container& arguments, typename atermpp::detail::enable_if_container<Container, data_expression>::type* = 0)
      : data_expression(core::detail::gsMakeDataAppl(head, data_expression_list(arguments.begin(), arguments.end())))
    {}

    const data_expression& head() const
    {
      return atermpp::aterm_cast<const data_expression>(atermpp::arg1(*this));
    }

    const data_expression_list& arguments() const
    {
      return atermpp::aterm_cast<const data_expression_list>(atermpp::list_arg2(*this));
    }
//--- start user section application ---//
  private:
    // forbid the use of iterator, which is silently inherited from
    // aterm_appl. Modifying the arguments of an application through the iterator
    // is not allowed!
    typedef data_expression_list::iterator iterator;

  public:
    typedef data_expression_list::const_iterator const_iterator;

    /// \brief Constructor.
    template <typename FwdIter>
    application(const data_expression& head,
                FwdIter first,
                FwdIter last)
      : data_expression(core::detail::gsMakeDataAppl(head, data_expression_list(first, last)))
    {}

    /// \brief Returns an iterator pointing to the first argument of the
    ///        application.
    const_iterator begin() const
    {
      return arguments().begin();
    }

    /// \brief Returns an iterator pointing past the last argument of the
    ///        application.
    const_iterator end() const
    {
      return arguments().end();
    }

    /// \brief Returns an iterator pointing past the last argument of the
    ///        application.
    size_t size() const
    {
      return arguments().size();
    }

/*
    /// \brief Returns the first argument of the application
    /// \pre head() is a binary operator
    /// \return arguments()[0]
    inline
    const data_expression& left() const
    {
      assert(size() == 2);
      return *(begin());
    }

    /// \brief Returns the second argument of the application
    /// \pre head() is a binary operator
    /// \return arguments()[1]
    inline
    const data_expression& right() const
    {
      assert(size() == 2);
      return *(++(begin()));
    }
*/
//--- end user section application ---//
};
//--- end generated class application ---//

/// \brief Apply data expression to a data expression
inline application make_application(data_expression const& head,
                                    data_expression const& e0)
{
  // Due to sort aliasing, the asserts below are not necessarily
  // valid anymore.
  // assert(is_function_sort(head.sort()));
  // assert(function_sort(head.sort()).domain().size() == 1);
  return application(head, atermpp::make_list(e0));
}

/// \brief Apply data expression to two data expression
inline application make_application(data_expression const& head,
                                    data_expression const& e0,
                                    data_expression const& e1)
{
  // See above for the reason to outcomment the asserts below
  // assert(is_function_sort(head.sort()));
  // assert(function_sort(head.sort()).domain().size() == 2);
  return application(head, atermpp::make_list(e0, e1));
}

/// \brief Apply data expression to three data expression
inline application make_application(data_expression const& head,
                                    data_expression const& e0,
                                    data_expression const& e1,
                                    data_expression const& e2)
{
  // See above for the reason to outcomment the asserts below
  // assert(is_function_sort(head.sort()));
  // assert(function_sort(head.sort()).domain().size() == 3);
  return application(head, atermpp::make_list(e0, e1, e2));
}

/// \brief Apply data expression to four data expression
inline application make_application(data_expression const& head,
                                    data_expression const& e0,
                                    data_expression const& e1,
                                    data_expression const& e2,
                                    data_expression const& e3)
{
  // See above for the reason to outcomment the asserts below
  // assert(is_function_sort(head.sort()));
  // assert(function_sort(head.sort()).domain().size() == 4);
  return application(head, atermpp::make_list(e0, e1, e2, e3));
}

// The precedence function must be declared here. Unfortunately this cannot be done using the include below.
// #include "mcrl2/data/precedence.h"
// Instead we do a forward declare of the precedence function. The user must make sure the file precedence.h is actually included.
// TOOO: fix this by moving the is_??? functions to the file application.h
int precedence(const data_expression& x);
int precedence(const application& x);

inline
const data_expression& unary_operand(const application& x)
{
  return *x.begin();
}

inline
const data_expression& binary_left(const application& x)
{
  return *x.begin();
}

inline
const data_expression& binary_right(const application& x)
{
  return *(++x.begin());
}

} // namespace data

} // namespace mcrl2

namespace std {
//--- start generated swap functions ---//
template <>
inline void swap(mcrl2::data::application& t1, mcrl2::data::application& t2)
{
  t1.swap(t2);
}
//--- end generated swap functions ---//
} // namespace std

#endif // MCRL2_DATA_APPLICATION_H
