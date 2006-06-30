#ifndef SIP_OBJECT_H
#define SIP_OBJECT_H

#include <set>
#include <ostream>
#include <cstring>

#include <boost/shared_ptr.hpp>

#include <xml2pp/text_reader.h>
#include <sip/detail/exception.h>
#include <sip/detail/option.h>

namespace sip {

  namespace controller {
    class communicator;
  }

  namespace tool {
    class communicator;
  }

  /**
   * \brief Describes some tool capabilities (e.g. supported protocol version)
   *
   * Objects of this type contain information about the capabilities of a tool:
   *
   *  - what version of the protocol the controller uses
   *  - a list of input configurations
   *  - ...
   *
   * As well as any information about the controller that might be interesting
   * for a tool developer.
   **/
  class object {
    friend class tool::communicator;
    friend class controller::communicator;
    friend class configuration;

    public:
      /**
       * \brief Type to indicate what the object is used for.
       *
       * This is useful for instance to a controller that can make sure that
       * input objects exists before giving the tool the start signal.
       **/
      enum type {
        input,          ///< \brief object is used as input
        output          ///< \brief object is used as output
      };

      /** \brief Until there is something better this is the type for a URI */
      typedef std::string                   uri;

      /** \brief Until there is something better this is the type for a storage format */
      typedef std::string                   storage_format;

      /** \brief convenience type to hide the shared pointer implementation */
      typedef boost::shared_ptr < object >  sptr;

      /** \brief Datatype for the textual identifier of an option/object */
      typedef option::identifier            identifier;

    private:

      /** \brief The format used for storing this object */
      storage_format       format;

      /** \brief The format used for storing this object */
      uri                  location;

      /** \brief The type of this object */
      type                 _type;

      /** \brief Must uniquely identify the object in a configuration */
      identifier           id;

      /** \brief String representations for types, used for XML encoding */
      const static char*   type_strings[];

    private:

      /** \brief Read from XML stream */
      inline static object::sptr read(xml2pp::text_reader& reader);

      /** \brief Constructor */
      inline object(const identifier, const storage_format, const uri = "", const type = input);

    public:
      /** \brief Returns the objects identifier */
      inline const identifier get_id() const;

      /** \brief Returns the objects type */
      inline const type get_type() const;

      /** \brief Returns the object storage format */
      inline const storage_format get_format() const;

      /** \brief Sets the object storage format */
      inline void set_format(object::storage_format const&);

      /** \brief Returns the object location */
      inline const uri get_location() const;

      /** \brief Sets the object location */
      inline void set_location(object::uri const&);

      /** \brief Write to XML stream */
      inline void write(std::ostream&) const;
  };

#ifdef IMPORT_STATIC_MEMBERS
  const char* object::type_strings[] = {
    "input"      /// \brief object is used for input
   ,"output"     /// \brief object is used for output
   ,0
  };
#endif

  inline object::object(const identifier i, const storage_format f, const uri l, const type t) : format(f), location(l), _type(t), id(i) {
  }

  inline const object::identifier object::get_id() const {
    return (id);
  }

  inline const object::type object::get_type() const {
    return (_type);
  }

  inline const object::storage_format object::get_format() const {
    return (format);
  }

  inline void object::set_format(object::storage_format const& f) {
    format = f;
  }

  inline const object::uri object::get_location() const {
    return (location);
  }

  inline void object::set_location(object::uri const& l) {
    location = l;
  }

  inline void object::write(std::ostream& output) const {
    output << "<object id=\"" << id
           << "\" type=\"" << type_strings[_type]
           << "\" storage-format=\"" << format << "\"";
    
    if (!location.empty()) {
      output << " location=\"" << location << "\"";
    }

    output << "/>";
  }

  /** \pre the reader must point at an object element} */
  inline object::sptr object::read(xml2pp::text_reader& r) {
    using sip::exception::exception;

    object::identifier id = 0;

    assert(r.is_element("object"));
    
    if (!r.get_attribute(&id, "id")) {
      throw (exception(sip::exception::message_missing_required_attribute, "id", "object"));
    }
    else {
      std::string new_type;

      if (!r.get_attribute(&new_type, "type")) {
        throw (exception(sip::exception::message_missing_required_attribute, "type", "object"));
      }
     
      size_t i = 0;
     
      while (type_strings[i] != 0 && strcmp(new_type.c_str(), type_strings[i]) != 0) {
        ++i;
      }
     
      if (type_strings[i] == 0) {
        throw (exception(sip::exception::message_unknown_type, new_type, "object"));
      }
     
      std::string new_format;
     
      if (!r.get_attribute(&new_format, "storage-format")) {
        throw (exception(sip::exception::message_missing_required_attribute, "storage-format", "object"));
      }
     
      std::string new_location;
     
      r.get_attribute(&new_location, "location");
     
      r.next_element();
      r.skip_end_element("object");
     
      return (object::sptr(new object(id, new_format, new_location, static_cast < object::type > (i))));
    }
  }
}

#endif

