#ifndef SIP_BASIC_DATATYPE
#define SIP_BASIC_DATATYPE

#include <string>
#include <ostream>

#include <sip/detail/exception.h>

#include <xml2pp/detail/text_reader.tcc>

/* Interface classes for both the tool and the controller side of the Squadt Interaction Protocol */
namespace sip {

  namespace datatype {
    /** \brief Basic datatypes used in the protocol (for validation purposes) */
    class basic_datatype {
      public:
        /** \brief Reconstruct a type specification from XML stream */
        static basic_datatype* read(xml2pp::text_reader&);

        /** \brief Write from XML stream, using value */
        virtual void write(std::ostream&, std::string value = "") const = 0;

        /** \brief Establishes whether value is valid for an element of this type */
        virtual bool validate(std::string& value) const = 0;

        /** \brief Pure virtual destructor */
        virtual ~basic_datatype() = 0;
    };

//    class enumeration : public basic_datatype {
//    };

//    class integer : public basic_datatype {
//    };

//    class real : public basic_datatype {
//    };

//    class uri : public basic_datatype {
//    };

    /** \brief Derived datatype for strings */
    class string : public basic_datatype {
      private:
        /** \brief The minimum length a string of this type has */
        size_t minimum_length;

        /** \brief The maximum length a string of this type has */
        size_t maximum_length;

      public:
        /** \brief Constructor */
        inline string(size_t minimum = 0, size_t maximum = 0); 

        /** \brief Reconstruct a type from XML stream */
        inline static string* read(xml2pp::text_reader&);

        /** \brief Set the minimum length of a string of this type */
        inline void set_minimum_length(size_t);

        /** \brief Set the maximum length of a string of this type */
        inline void set_maximum_length(size_t);

        /** \brief Write from XML stream, using value */
        inline void write(std::ostream&, std::string value = "") const;

        /** \brief Establishes whether value is valid for an element of this type */
        inline bool validate(std::string& value) const;

        /** \brief Destructor */
        inline ~string();
    };

    /*************************************************************************
     * Implementation of basic_datatype
     ************************************************************************/
    inline basic_datatype::~basic_datatype() {
    }

    inline basic_datatype* basic_datatype::read(xml2pp::text_reader& r) {
      if (r.is_element("enumeration")) {
//        return (enumeration::read(r));
      }
      else if (r.is_element("integer")) {
//        return (integer::read(r));
      }
      else if (r.is_element("real")) {
//        return (real::read(r));
      }
      else if (r.is_element("uri")) {
//        return (uri::read(r));
      }
      else if (r.is_element("string")) {
        return (string::read(r));
      }

      /* Unknown type in configuration */
      throw (sip::exception::exception(exception::message_unknown_type, r.element_name()));
    }

//    static integer standard_integer;
//    static integer standard_natural;
//    static real    standard_real;

    /************************************************************************
     * Implementation of string
     ************************************************************************/

    /** Instance of a string (without limitations) */
    static string standard_string;

    inline string::string(size_t minimum, size_t maximum) : minimum_length(minimum), maximum_length(maximum_length) {
    }

    /** \pre The current element must be \<string\>  */
    inline string* string::read(xml2pp::text_reader& r) {
      size_t minimum = 0;
      size_t maximum = 0;

      /* Current element must be <string> */
      assert(r.is_element("string"));

      r.get_attribute(&minimum, "minimum-length");
      r.get_attribute(&maximum, "maximum-length");

      r.read();
      r.skip_end_element("string");

      if (minimum == 0 && maximum == 0) {
        return (&standard_string);
      }
      else {
        return (new string(minimum, maximum));
      }
    }

    inline void string::set_maximum_length(size_t m) {
      maximum_length = m;
    }

    inline void string::set_minimum_length(size_t m) {
      minimum_length = m;
    }

    inline void string::write(std::ostream& output, std::string value) const {
      assert(validate(value));

      output << "<string";

      if (minimum_length != 0) {
        output << " minimum-length=\"" << minimum_length << "\"";
      }
      if (maximum_length != 0) {
        output << " maximum-length=\"" << maximum_length << "\"";
      }

      output << "/>";

      if (!value.empty()) {
        output << "<value>" << value << "</value>";
      }
    }

    inline bool string::validate(std::string& s) const {
      return (minimum_length <= s.size() && (maximum_length <= minimum_length || s.size() <= maximum_length));
    }

    inline string::~string() {
    }
  }
}

#endif

