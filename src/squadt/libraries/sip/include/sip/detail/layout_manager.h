#ifndef LAYOUT_MANAGER_H
#define LAYOUT_MANAGER_H

#include <utility>
#include <vector>
#include <map>
#include <memory>

#include <boost/integer.hpp>

#include <sip/layout_base.h>
#include <sip/detail/layout_mediator.h>

namespace sip {
  namespace layout {

    /** \brief Type for element visibility */
    enum visibility {
      visible = 10,  ///< the element is visible
      hidden = 11,   ///< the element is not visible but still has effect on the layout
      none = 12      ///< the element is invisible and has no effect on the layout
    };

    /** \brief The directional alignment of layout elements perpendicular to the orientation of the box */
    enum vertical_alignment {
      top = 0,    ///< element is put as much to the top as possible
      middle = 1, ///< element is centered
      bottom = 2  ///< element is put as much to the bottom as possible
    };

    /** \brief The directional alignment of layout elements perpendicular to the orientation of the box */
    enum horizontal_alignment {
      left = 3,   ///< element is put as much to the left as possible
      center = 4, ///< element is centered
      right = 5   ///< element is put as much to the right as possible
    };

    /** \brief Type for the margins that should be observed around the element */
    struct margins {
      unsigned short top;    ///< top margin in pixels
      unsigned short right;  ///< right margin in pixels
      unsigned short bottom; ///< bottom margin in pixels
      unsigned short left;   ///< left margin in pixels

      /** \brief Constructor */
      inline margins(const unsigned short = 0, const unsigned short = 0, const unsigned short = 0, const unsigned short = 0);

      /** \brief Compares for equality */
      inline bool operator==(margins const&) const;

      /** \brief Compares for inequality */
      inline bool operator!=(margins const&) const;
    };

    /** \brief Type for element identifiers () */
    typedef boost::uint_t< (sizeof(sip::layout::element*) << 3) >::least  element_identifier;

    /** \brief Base class for layout constraint containers */
    class properties : public utility::visitable {
      template < typename R, typename S >
      friend class utility::visitor;

      public:

        horizontal_alignment m_alignment_horizontal; ///< how the element is aligned horizontally
        vertical_alignment   m_alignment_vertical;   ///< how the element is aligned vertically
        margins              m_margin;               ///< the margins that should be observed around the element
        visibility           m_visible;              ///< whether or not the element affects layout and is visible
        bool                 m_grow;                 ///< whether or not to expand the control to fill available space
        bool                 m_enabled;              ///< whether or not the control is enabled

        /** \brief Constructor */
        inline properties();
       
        /** \brief Constructor */
        inline properties(vertical_alignment const&, horizontal_alignment const&, margins const&, visibility const&);
       
        /** \brief Constructor, for when horizontal alignment does not matter */
        inline properties(vertical_alignment const&, margins const&, visibility const&);
       
        /** \brief Constructor, for when vertical alignment does not matter */
        inline properties(horizontal_alignment const&, margins const&, visibility const&);

        /** \brief Whether the control is allowed to grow */
        inline void set_growth(bool b);

        /** \brief Compares for equality */
        inline bool operator==(properties const&) const;

        /** \brief Compares for inequality */
        inline bool operator!=(properties const&) const;
    };

    /** \brief Abstract base class for layout managers */
    class manager : public sip::layout::element {
      friend class sip::layout::element;

      template < typename R, typename S >
      friend class utility::visitor;

      public:

        struct layout_descriptor {
          element*           layout_element;
          properties         layout_properties;
          element_identifier identifier;

          layout_descriptor(element* e, properties const& p, element_identifier const& id) : layout_element(e), layout_properties(p), identifier(id) { }
        };

        /** \brief Type alias to simplify using auto pointers */
        typedef std::auto_ptr < manager > aptr;

      public:

        /** \brief The default margins between elements */
        static const margins     default_margins;

        /** \brief The default visibility of elements */
        static const visibility  default_visibility;

        /** \brief Default properties */
        static const properties  default_properties;

      protected:

        /** \brief Attaches a layout element to a manager, using layout properties */
        inline void attach(layout::mediator*, mediator::wrapper_aptr, properties const*) const;

      public:

        /** \brief Adds a new element to the box */
        virtual element* add(element*) = 0;

        /** \brief Adds a new element to the box */
        virtual element* add(element*, margins const&,
                                  visibility const& = manager::default_visibility) = 0;

        /** \brief Adds a new element to the box */
        virtual element* add(element*, visibility const&) = 0;

        /** \brief Enables an element visible */ 
        virtual void enable(element*, bool = true) = 0;

        /** \brief Disables an element invisible */ 
        virtual void disable(element*) = 0;

        /** \brief Makes an element visible */ 
        virtual void show(element*, bool = true) = 0;

        /** \brief Makes an element invisible */ 
        virtual void hide(element*) = 0;

        /** \brief Instantiate a layout element, through a mediator */
        virtual mediator::wrapper_aptr instantiate(layout::mediator*) = 0;

        /** \brief Destructor */
        virtual ~manager() = 0;
    };

    /**
     * \brief Box layout manager
     *
     * Elements are laid out horizontally or vertically, according to the chosen box variant.
     **/
    class box : public manager {
      template < typename R, typename S >
      friend class utility::visitor;

      protected:

        /** \brief The type of the list with the element managed by this manager */
        typedef std::vector< layout_descriptor > children_list;

      protected:

        /** \brief The layout elements directly contained in this box */
        children_list    m_children;

      protected:

        /** \brief Resets private members to defaults */
        inline void clear();

        /** \brief Instantiate a layout element, through a mediator */
        inline mediator::wrapper_aptr instantiate(layout::mediator::aptr) const;

      public:

        /** \brief Constructor */
        inline box();

        /** \brief Adds a new element to the box */
        inline element* add(element*);

        /** \brief Adds a new element to the box */
        inline element* add(element*, properties const&);

        /** \brief Adds a new element to the box */
        inline element* add(element*, margins const&,
                                  visibility const& = manager::default_visibility);

        /** \brief Adds a new element to the box */
        inline element* add(element*, visibility const&);

        /** \brief Enables an element visible */ 
        void enable(element*, bool = true);

        /** \brief Disables an element invisible */ 
        void disable(element*);

        /** \brief Makes an element */ 
        void show(element*, bool = true);

        /** \brief Makes an element invisible */ 
        void hide(element*);

        /** \brief Instantiate a layout element, through a mediator */
        virtual mediator::wrapper_aptr instantiate(layout::mediator*) = 0;

        /** \brief Destructor */
        virtual ~box() = 0;
    };

    /**
     * \brief Vertical box layout manager
     *
     * Elements are laid out vertically
     **/
    class vertical_box : public box {
      friend class sip::layout::manager;
      friend class sip::layout::element;

      template < typename R, typename S >
      friend class utility::visitor;

      public:

        /** \brief Element positioning is subject only to horizontal alignment */
        typedef horizontal_alignment alignment;

      public:

        /** \brief Default alignment */
        static const alignment   default_alignment;

      public:

        /** \brief Constructor */
        inline vertical_box();

        /** \brief Instantiates a layout manager and returns a shared pointer */
        inline static std::auto_ptr < vertical_box > create();

        /** Adds a new element to the box */
        inline element* add(element*, alignment const&,
                                  margins const& = manager::default_margins,
                                  visibility const& = manager::default_visibility);

        /** \brief Instantiate a layout element, through a mediator */
        inline mediator::wrapper_aptr instantiate(layout::mediator*);
    };

    /**
     * \brief Vertical box layout manager
     *
     * Elements are laid out vertically
     **/
    class horizontal_box : public box {
      friend class sip::layout::manager;
      friend class sip::layout::element;

      template < typename R, typename S >
      friend class utility::visitor;

      public:

        /** \brief Element positioning is subject only to vertical alignment */
        typedef vertical_alignment alignment;

      public:

        /** \brief Default alignment */
        static const alignment   default_alignment;

      public:

        /** \brief Constructor */
        inline horizontal_box();

        /** \brief Instantiates a layout manager and returns a shared pointer */
        inline static std::auto_ptr < horizontal_box > create();

        /** Adds a new element to the box */
        inline element* add(element*, alignment const&,
                                  margins const& = manager::default_margins,
                                  visibility const& = manager::default_visibility);

        /** \brief Instantiate a layout element, through a mediator */
        inline mediator::wrapper_aptr instantiate(layout::mediator*);
    };

    /**
     * \param m the mediator object to use
     * \param d the data needed
     * \param c the layout properties
     **/
    inline void manager::attach(layout::mediator* m, mediator::wrapper_aptr d, properties const* c) const {
      m->attach(d, c);
    }

    inline manager::~manager() {
    }

    inline properties::properties() : m_alignment_horizontal(center), m_alignment_vertical(middle),
                m_margin(manager::default_margins), m_visible(manager::default_visibility), m_grow(false) {
    }

    inline properties::properties(vertical_alignment const& av, horizontal_alignment const& ah, margins const& m, visibility const& v) :
                                                m_alignment_horizontal(ah), m_alignment_vertical(av), m_margin(m), m_visible(v), m_grow(false) {
    }

    inline properties::properties(vertical_alignment const& av, margins const& m, visibility const& v) :
                                                m_alignment_horizontal(center), m_alignment_vertical(av), m_margin(m), m_visible(v), m_grow(false) {
    }

    inline properties::properties(horizontal_alignment const& ah, margins const& m, visibility const& v) :
                                                m_alignment_horizontal(ah), m_alignment_vertical(middle), m_margin(m), m_visible(v), m_grow(false) {
    }

    inline void properties::set_growth(bool b) {
      m_grow = b;
    }

    /**
     * \param[in] t the top margin
     * \param[in] r the right margin
     * \param[in] b the bottom margin
     * \param[in] l the left margin
     **/
    inline margins::margins(const unsigned short t, const unsigned short r, const unsigned short b, const unsigned short l) :
                                                                top(t), right(r), bottom(b), left(l) {
    }

    /**
     * \param[in] m the margins to compare agains
     **/
    inline bool margins::operator==(margins const& m) const {
      return (top == m.top && left == m.left && bottom == m.bottom && right == m.right);
    }

    /**
     * \param[in] m the margins to compare agains
     **/
    inline bool margins::operator!=(margins const& m) const {
      return (top != m.top || left != m.left || bottom != m.bottom || right != m.right);
    }

    /**
     * \param[in] c the properties object to compare against
     **/
    inline bool properties::operator==(properties const& c) const {
      return (m_alignment_horizontal == c.m_alignment_horizontal &&
              m_alignment_vertical == c.m_alignment_vertical &&
              m_margin == c.m_margin && m_visible == c.m_visible &&
              m_grow == c.m_grow && m_enabled == c.m_enabled);
    }

    /**
     * \param[in] c the properties object to compare against
     **/
    inline bool properties::operator!=(properties const& c) const {
      return (m_alignment_horizontal != c.m_alignment_horizontal ||
              m_alignment_vertical != c.m_alignment_vertical ||
              m_margin != c.m_margin || m_visible != c.m_visible ||
              m_grow != c.m_grow || m_enabled != c.m_enabled);
    }

    inline box::box() {
    }

    inline vertical_box::vertical_box() {
    }

    inline horizontal_box::horizontal_box() {
    }

    inline std::auto_ptr < vertical_box > vertical_box::create() {
      return (std::auto_ptr < vertical_box >(new vertical_box()));
    }

    inline std::auto_ptr < horizontal_box > horizontal_box::create() {
      return (std::auto_ptr < horizontal_box >(new horizontal_box()));
    }

    inline void box::clear() {
      m_children.clear();
    }

    /**
     * \param[in] e a pointer to a layout element
     **/
    inline element* box::add(element* e) {
      return (add(e, manager::default_properties));
    }

    /**
     * \param[in] e a pointer to a layout element
     * \param[in] c the layout properties to observe
     **/
    inline element* box::add(element* e, properties const& c) {
      properties cn = c;

      cn.set_growth(e->get_grow());

      m_children.push_back(layout_descriptor(e,cn,reinterpret_cast < element_identifier > (e)));

      return (e);
    }

    inline void box::enable(element* e, bool b) {
      for (children_list::iterator i = m_children.begin(); i != m_children.end(); ++i) {
        if (i->layout_element == e) {
          i->layout_properties.m_enabled = b;
        }
      }

      activate_handlers();
    }

    inline void box::disable(element* e) {
      enable(e, false);
    }

    inline void box::show(element* e, bool b) {
      for (children_list::iterator i = m_children.begin(); i != m_children.end(); ++i) {
        if (i->layout_element == e) {
          i->layout_properties.m_visible = b ? visible : hidden;
        }
      }

      activate_handlers();
    }

    inline void box::hide(element* e) {
      show(e, false);
    }

    /**
     * \param[in] e a pointer to a layout element
     * \param[in] a how the element should be aligned relative to the box
     * \param[in] m the margins of the element relative to other elements that occupy the box
     * \param[in] v whether the element is visible and has an effect on other elements that occupy the box
     **/
    inline element* vertical_box::add(element* e, alignment const& a, margins const& m, visibility const& v) {
      return (box::add(e, properties(middle, a, m, v)));
    }

    /**
     * \param[in] e a pointer to a layout element
     * \param[in] a how the element should be aligned relative to the box
     * \param[in] m the margins of the element relative to other elements that occupy the box
     * \param[in] v whether the element is visible and has an effect on other elements that occupy the box
     **/
    inline element* horizontal_box::add(element* e, alignment const& a, margins const& m, visibility const& v) {
      return (box::add(e, properties(a, left, m, v)));
    }

    /**
     * \param[in] e a pointer to a layout element
     * \param[in] m the margins of the element relative to other elements that occupy the box
     * \param[in] v whether the element is visible and has an effect on other elements that occupy the box
     **/
    inline element* box::add(element* e, margins const& m, visibility const& v) {
      return (box::add(e, properties(middle, left, m, v)));
    }

    /**
     * \param[in] e a pointer to a layout element
     * \param[in] v whether the element is visible and has an effect on other elements that occupy the box
     **/
    inline element* box::add(element* e, visibility const& v) {
      return (box::add(e, properties(middle, left, manager::default_margins, v)));
    }

    /**
     * \param m the mediator object to use
     **/
    inline layout::mediator::wrapper_aptr box::instantiate(layout::mediator::aptr m) const {
      layout::mediator* n = m.get();

      for (children_list::const_iterator i = m_children.begin(); i != m_children.end(); ++i) {
        manager::attach(n, (i->layout_element)->instantiate(n), static_cast < const layout::properties* > (&(i->layout_properties)));
      }

      return (n->extract_data());
    }

    /**
     * \param m the mediator object to use
     **/
    inline layout::mediator::wrapper_aptr vertical_box::instantiate(layout::mediator* m) {
      return (box::instantiate(m->build_vertical_box()));
    }

    /**
     * \param m the mediator object to use
     **/
    inline layout::mediator::wrapper_aptr horizontal_box::instantiate(layout::mediator* m) {
      return (box::instantiate(m->build_horizontal_box()));
    }

    inline box::~box() {
      for (children_list::const_iterator i = m_children.begin(); i != m_children.end(); ++i) {
        delete (i->layout_element);
      }
    }
  }
}

#endif
