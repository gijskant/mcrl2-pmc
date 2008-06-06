// Author(s): VitaminB100
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file dataexpression.h
//
// Declares the dataexpression datatype.

#ifndef LIBGRAPE_DATAEXPRESSION_H
#define LIBGRAPE_DATAEXPRESSION_H

#include "sortexpression.h"

namespace grape
{
  namespace libgrape
  {

    /**
     * \short Represents a data expression.
     */
    class dataexpression
    {
      protected:
        wxString  m_expression;       /**< The data expression's text */
        sortexpression  m_sortexpression; /**< The data expression's sort */
      private:
      public:
        /**
         * Default constructor.
         * Initializes dataexpression.
         */
        dataexpression( void );

        /**
         * Copy constructor
         * Creates a new data expression based on an existing one.
         * @param p_dataexpression The data expression to copy.
         */
        dataexpression( const dataexpression &p_dataexpression );

        /**
         * Default destructor.
         * Frees allocated memory.
         */
        ~dataexpression( void );

        /**
         * Data expression assignment function
         * @param p_expression The new data expression
         */
        void set_expression( const wxString &p_expression );

        /**
         * Data expression retrieval function
         * @return The dataexpression
         */
        wxString get_expression( void ) const;
    };

    /**
     * List of dataexpression. Represents a list of dataexpression datatypes.
     */
    WX_DECLARE_OBJARRAY( dataexpression, list_of_dataexpression );

  } // namespace libgrape
} // namespace grape

#endif // LIBGRAPE_DATAEXPRESSION_H
