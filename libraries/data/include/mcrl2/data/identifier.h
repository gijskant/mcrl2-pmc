#ifndef MCRL2_DATA_IDENTIFIER_H
#define MCRL2_DATA_IDENTIFIER_H

#include "mcrl2/data/data_expression.h"

namespace mcrl2
{

namespace data
{

//--- start generated class identifier ---//
/// \brief An identifier
class identifier: public data_expression
{
  public:
    /// \brief Default constructor.
    identifier()
      : data_expression(core::detail::constructId())
    {}

    /// \brief Constructor.
    /// \param term A term
    identifier(const atermpp::aterm& term)
      : data_expression(term)
    {
      assert(core::detail::check_term_Id(*this));
    }

    /// \brief Constructor.
    identifier(const core::identifier_string& name)
      : data_expression(core::detail::gsMakeId(name))
    {}

    /// \brief Constructor.
    identifier(const std::string& name)
      : data_expression(core::detail::gsMakeId(core::identifier_string(name)))
    {}

    const core::identifier_string& name() const
    {
      return atermpp::aterm_cast<const core::identifier_string>(atermpp::arg1(*this));
    }
};
//--- end generated class identifier ---//

/// \brief list of identifiers
typedef atermpp::term_list< identifier > identifier_list;
/// \brief vector of identifiers
typedef std::vector< identifier > identifier_vector;


} // namespace data
} // namespace mcrl2

namespace std {
//--- start generated swap functions ---//
template <>
inline void swap(mcrl2::data::identifier& t1, mcrl2::data::identifier& t2)
{
  t1.swap(t2);
}
//--- end generated swap functions ---//
} // namespace std

#endif
