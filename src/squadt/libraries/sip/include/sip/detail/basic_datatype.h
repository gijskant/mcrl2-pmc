#ifndef SIP_BASIC_DATATYPE
#define SIP_BASIC_DATATYPE

#include <cstdio>
#include <climits>
#include <cfloat>
#include <string>
#include <ostream>
#include <utility>
#include <iostream>

#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_floating_point.hpp>

#include <sip/visitors.h>

namespace sip {

  namespace datatype {

    class basic_datatype : public utility::visitable {

      public:

        /** \brief Boost shared pointer type alias */
        typedef boost::shared_ptr < basic_datatype > sptr;

      public:

        /** \brief Converts a boolean to a string representation */
        template < typename U >
        std::string convert(U const&) const;

        /** \brief Converts to underlying type */
        virtual boost::any evaluate(std::string const&) const = 0;

        /** \brief Establishes whether value is valid for an element of this type */
        virtual bool validate(std::string const& value) const = 0;

        /** \brief Pure virtual destructor */
        virtual ~basic_datatype() = 0;
    };

    /**
     * \brief Derived data type specifier for enumerations
     *
     * An enumeration is a finite set of alternatives.
     **/
    class enumeration : public basic_datatype {
      template < typename R, typename S >
      friend class utility::visitor;

      private:
        
        /** \brief The possible values in the domain */
        std::vector < std::string > m_values;

        /** \brief Index into values of the default value for elements of the specified type */
        size_t                      m_default_value;

      public:

        /** \brief POD type used for implementation */
        typedef std::string implementation_type;

      public:
        
        /** \brief Constructor */
        enumeration();

        /** \brief Constructor */
        enumeration(std::string const& s);

        /** \brief Add value */
        void add_value(std::string const&, bool = false);

        /** \brief Converts to a string representation */
        template < typename T >
        std::string convert(T const&);

        /** \brief Converts a string to an index representation */
        boost::any evaluate(std::string const&) const;

        /** \brief Establishes whether value is valid for an element of this type */
        inline bool validate(std::string const& value) const;

        /** \brief Convenience method for adding values */
        enumeration& operator% (std::string const&);
    };

    /**
     * \brief Derived data type specifier for integer number ranges (finite using long int)
     * 
     * The range is specified by a minimum and a maximum. The minimum, of
     * course, must be smaller than the maximum. The default value is taken to
     * be the minimum, unless it is specified at construction time.
     **/
    class integer : public basic_datatype {
      template < typename R, typename S >
      friend class utility::visitor;

      protected:

        /** \brief The minimum integer value in the range */
        long int m_minimum;

        /** \brief The maximum integer value in the range */
        long int m_maximum;

        /** \brief The default value for elements of the specified type */
        long int m_default_value;

      public:

        /** \brief Implementation dependent limitation (minimum value) */
        static const long int implementation_minimum;

        /** \brief Implementation dependent limitation (maximum value) */
        static const long int implementation_maximum;

      public:

        /** \brief POD type used for implementation */
        typedef long int implementation_type;

      public:

        /** \brief Constructor */
        integer(long int = implementation_minimum, long int = implementation_minimum, long int = implementation_maximum);

        /** \brief Converts a long int to a string representation */
        static std::string convert(long int const&);

        /** \brief Converts a string to a long int representation */
        boost::any evaluate(std::string const&) const;

        /** \brief Establishes whether value is valid for an element of this type */
        inline bool validate(std::string const& value) const;
    };

    class natural : public datatype::integer {
      template < typename R, typename S >
      friend class utility::visitor;

      public:

        /** \brief Constructor */
        natural();
    };

    class positive_integer : public datatype::integer {
      template < typename R, typename S >
      friend class utility::visitor;

      public:

        /** \brief Constructor */
        positive_integer();
    };

    class negative_integer : public datatype::integer {
      template < typename R, typename S >
      friend class utility::visitor;

      public:

        /** \brief Constructor */
        negative_integer();
    };

    /**
     * \brief Derived data type specifier for real number ranges (finite using double)
     *
     * The range is specified by a minimum and a maximum. The minimum, of
     * course, must be smaller than the maximum. The default value is taken to
     * be the minimum, unless it is specified at construction time.
     **/
    class real : public basic_datatype {
      template < typename R, typename S >
      friend class utility::visitor;

      protected:

        /** \brief The minimum integer value in the range */
        double m_minimum;

        /** \brief The maximum integer value in the range */
        double m_maximum;

        /** \brief The default value for elements of the specified type */
        double m_default_value;

        /** \brief Whether or not the minimum is included in the range */
        bool   m_minimum_included;

        /** \brief Whether or not the maximum is included in the range */
        bool   m_maximum_included;

      public:

        /** \brief Implementation dependent limitation (minimum value) */
        static const double implementation_minimum;

        /** \brief Implementation dependent limitation (maximum value) */
        static const double implementation_maximum;

      public:

        /** \brief POD type used for implementation */
        typedef double implementation_type;

      public:

        /** \brief Constructor */
        real(double d = implementation_minimum, double = implementation_minimum, double = implementation_maximum);

        /** \brief whether the minimum bound should be interpreted as open-ended or close-ended */
        void set_include_minimum(bool b);

        /** \brief whether the maximum bound should be interpreted as open-ended or close-ended */
        void set_include_maximum(bool b);

        /** \brief Converts a double to a string representation */
        static std::string convert(double const&);

        /** \brief Converts a string to a long int representation */
        boost::any evaluate(std::string const&) const;

        /** \brief Establishes whether value is valid for an element of this type */
        inline bool validate(std::string const& value) const;
    };

    class positive_real : public datatype::real {
      template < typename R, typename S >
      friend class utility::visitor;

      public:

        /** \brief Constructor */
        positive_real();
    };

    class negative_real : public datatype::real {
      template < typename R, typename S >
      friend class utility::visitor;

      public:

        /** \brief Constructor */
        negative_real();
    };

//    class uri : public basic_datatype {
//    };

    /** \brief Derived data type specifier for booleans */
    class boolean : public basic_datatype {
      template < typename R, typename S >
      friend class utility::visitor;

      public:

        /** \brief The string that represents true */
        static const std::string true_string;

        /** \brief The string that represents false */
        static const std::string false_string;

      public:

        /** \brief POD type used for implementation */
        typedef bool implementation_type;

      public:

        /** \brief Constructor */
        boolean();

        /** \brief Converts a boolean to a string representation */
        static std::string convert(bool const&);

        /** \brief Converts a string to a boolean representation */
        boost::any evaluate(std::string const&) const;

        /** \brief Establishes whether value is valid for an element of this type */
        inline bool validate(std::string const& value) const;
    };

    /** \brief Derived data type for strings */
    class string : public basic_datatype {
      template < typename R, typename S >
      friend class utility::visitor;

      protected:

        /** \brief The minimum length a string of this type has */
        unsigned int       m_minimum_length;

        /** \brief The maximum length a string of this type has */
        unsigned int       m_maximum_length;

        /** \brief The default value for elements of the specified type */
        std::string        m_default_value;

      public:

        /** \brief The maximum length a string may have */
        static const unsigned int implementation_maximum_length;

      public:

        /** \brief POD type used for implementation */
        typedef std::string implementation_type;

      public:

        /** \brief Constructor */
        string();

        /** \brief Constructor */
        string(std::string const&, unsigned int minimum = 0, unsigned int maximum = implementation_maximum_length); 

        /** \brief Set the minimum length of a string of this type */
        void set_minimum_length(unsigned int);

        /** \brief Set the maximum length of a string of this type */
        void set_maximum_length(unsigned int);

        /** \brief Converts a string to a string representation (copy) */
        static std::string convert(std::string const& s);

        /** \brief Converts a string to a string representation (copy) */
        boost::any evaluate(std::string const&) const;

        /** \brief Establishes whether value is valid for an element of this type */
        bool validate(std::string const& value) const;
    };

    /*************************************************************************
     * Implementation of basic_datatype
     ************************************************************************/
    inline basic_datatype::~basic_datatype() {
    }

    template < typename T, bool e >
    inline std::string convert(basic_datatype const* t, T const& s, boost::integral_constant< bool, e > const&) {
      return static_cast < enumeration const* > (t)->convert(s);
    }

    template < typename T >
    inline std::string convert(basic_datatype const* t, T const& s, boost::false_type const&) {
      return convertr(t, s, boost::is_floating_point< T >());
    }

    template < typename T, bool e >
    inline std::string convertr(basic_datatype const* t, T const& s, boost::integral_constant< bool, e > const&) {
      return static_cast < real const* > (t)->convert(s);
    }

    template < typename T >
    inline std::string convertr(basic_datatype const* t, T const& s, boost::false_type const&) {
      return converti(t, s, boost::is_integral< T >());
    }

    template < typename T, bool e >
    inline std::string converti(basic_datatype const* t, T const& s, boost::integral_constant< bool, e > const&) {
      return static_cast < integer const* > (t)->convert(s);
    }

    template < typename T >
    inline std::string converti(basic_datatype const* t, T const& s, boost::false_type const&) {
      return static_cast < string const* > (t)->convert(s);
    }

    template < typename T >
    inline std::string basic_datatype::convert(T const& s) const {
      return datatype::convert(this, s, boost::is_enum< T >());
    }

    /** \brief Converts a boolean */
    template <>
    inline std::string basic_datatype::convert(bool const& s) const {
      return (boolean::convert(s));
    }

    /** \brief Converts a long int */
    template <>
    inline std::string basic_datatype::convert(long int const& s) const {
      return (integer::convert(s));
    }

    /** \brief Converts a double */
    template <>
    inline std::string basic_datatype::convert(double const& s) const {
      return (real::convert(s));
    }

    /** \brief Converts a string */
    template <>
    inline std::string basic_datatype::convert(std::string const& s) const {
      return (s);
    }

    /** \brief Converts a string */
    template <>
    inline std::string basic_datatype::convert(const char* const& s) const {
      return (std::string(s));
    }

    /************************************************************************
     * Implementation of Boolean
     ************************************************************************/

    inline boolean::boolean() {
    }

    /**
     * \param[in] s the boolean to convert
     **/
    inline std::string boolean::convert(bool const& s) {
      return ((s) ? boolean::true_string : boolean::false_string);
    }

    /**
     * \param[in] s the string to convert
     **/
    inline boost::any boolean::evaluate(std::string const& s) const {
      return (boost::any(s == boolean::true_string));
    }

    /**
     * \param[in] s any string
     **/
    inline bool boolean::validate(std::string const& s) const {
      return (s == true_string || s == false_string);
    }

    /************************************************************************
     * Implementation of Integer
     ************************************************************************/

    /**
     * \param[in] min the minimum value in the domain
     * \param[in] max the maximum value in the domain
     * \param[in] d the default value in the domain
     **/
    inline integer::integer(long int d, long int min, long int max) : m_minimum(min), m_maximum(max), m_default_value(d) {
    }

    inline natural::natural() : integer(0, 0, implementation_maximum) {
    }

    inline positive_integer::positive_integer() : integer(1, 1, implementation_maximum) {
    }

    inline negative_integer::negative_integer() : integer(-1, implementation_minimum, -1) {
    }

    /**
     * \param[in] s the integer to convert
     **/
    inline std::string integer::convert(long int const& s) {
      boost::format f("%ld");

      return ((f % s).str());
    }

    /**
     * \param[in] s the string to convert
     *
     * \pre the string should be parsable as long int
     **/
    inline boost::any integer::evaluate(std::string const& s) const {
      long int b;

      sscanf(s.c_str(), "%ld", &b);

      return (b);
    }

    /**
     * \param[in] s any string
     **/
    inline bool integer::validate(std::string const& s) const {
      long int b;

      return (sscanf(s.c_str(), "%ld", &b) == 1);
    }

    /************************************************************************
     * Implementation of Real 
     ************************************************************************/

    /**
     * \param[in] min the minimum value in the domain
     * \param[in] max the maximum value in the domain
     * \param[in] d the default value in the domain
     **/
    inline real::real(double d, double min, double max) : m_minimum(min), m_maximum(max), m_default_value(d),
       m_minimum_included(true), m_maximum_included(true) {
    }

    inline positive_real::positive_real() : real(implementation_maximum, 0, implementation_maximum) {
      m_minimum_included = false;
    }

    inline negative_real::negative_real() : real(implementation_minimum, implementation_minimum, 0) {
      m_maximum_included = false;
    }

    /**
     * \param[in] b the new value for whether the minimum is open-ended or close-ended
     **/
    inline void real::set_include_minimum(bool b) {
      m_minimum_included = b;
    }

    /**
     * \param[in] b the new value for whether the minimum is open-ended or close-ended
     **/
    inline void real::set_include_maximum(bool b) {
      m_maximum_included = b;
    }

    /**
     * \param[in] s the double to convert
     **/
    inline std::string real::convert(double const& s) {
      boost::format f("%lf");

      return ((f % s).str());
    }

    /**
     * \param[in] s the string to convert
     *
     * \pre the string should be parsable as long int
     **/
    inline boost::any real::evaluate(std::string const& s) const {
      double b;

      sscanf(s.c_str(), "%lf", &b);

      return (b);
    }

    /**
     * \param[in] s any string
     **/
    inline bool real::validate(std::string const& s) const {
      double b;

      return (sscanf(s.c_str(), "%lf", &b) == 1);
    }

    /************************************************************************
     * Implementation of Enumeration
     ************************************************************************/

    inline enumeration::enumeration() : m_default_value(0) {
    }

    /**
     * \param[in] s any string
     **/
    inline enumeration& enumeration::operator% (std::string const& s) {
      add_value(s);

      return (*this);
    }

    /**
     * \param[in] s the string to convert (value must be in the domain)
     **/
    template < typename T >
    inline std::string enumeration::convert(T const& s) {
      return (0 < s && static_cast < size_t > (s) < m_values.size()) ? m_values[s] : m_values[m_default_value];
    }

    /**
     * \param[in] s the string to convert
     *
     * \pre the string should be parsable as one of the values
     **/
    inline boost::any enumeration::evaluate(std::string const& s) const {
      std::vector< std::string >::const_iterator i = std::lower_bound(m_values.begin(), m_values.end(), s);

      return static_cast < size_t > (i == m_values.end() ? m_default_value : i - m_values.begin());
    }

    /**
     * \param[in] s any string
     **/
    inline bool enumeration::validate(std::string const& s) const {
      return boost::any_cast < size_t > (evaluate(s)) < m_values.size();
    }

    /************************************************************************
     * Implementation of String
     ************************************************************************/

    inline string::string() : m_minimum_length(0), m_maximum_length(UINT_MAX), m_default_value("") {
    }

    /**
     * \param[in] minimum the minimum length
     * \param[in] maximum the maximum length
     * \param[in] d the default value
     **/
    inline string::string(std::string const& d, unsigned int minimum, unsigned int maximum) :
                m_minimum_length(minimum), m_maximum_length(maximum), m_default_value(d) {
    }

    inline void string::set_maximum_length(unsigned int m) {
      m_maximum_length = m;
    }

    inline void string::set_minimum_length(unsigned int m) {
      m_minimum_length = m;
    }

    /**
     * \param[in] s the string to convert
     **/
    inline std::string string::convert(std::string const& s) {
      return (s);
    }

    /**
     * \param[in] s the string to convert
     **/
    inline boost::any string::evaluate(std::string const& s) const {
      return (s);
    }

    inline bool string::validate(std::string const& s) const {
      return (m_minimum_length <= s.size() && (m_maximum_length <= m_minimum_length || s.size() <= m_maximum_length));
    }
  }
}

#endif

