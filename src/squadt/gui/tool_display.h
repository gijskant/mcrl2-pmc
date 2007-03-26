#ifndef GUI_TOOL_DISPLAY_H
#define GUI_TOOL_DISPLAY_H

#include <map>

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/event.h>
#include <wx/textctrl.h>

#include <sip/report.h>
#include <sip/detail/layout_tool_display.h>

#include "../processor.h"

namespace squadt {
  namespace GUI {

    class project;

    /// \cond PRIVATE_PART
    namespace detail {

      class tool_display;
      class tool_display_mediator;

      /** \brief Handles state events of widgets */
      class state_change_handler : public wxEvtHandler {
        friend class squadt::GUI::tool_display;
        friend class detail::tool_display_mediator;

        private:
     
          /** \brief Associates a sip layout element with a wxWidgets control */
          typedef std::map < wxObject*, sip::layout::element const* > element_for_window_map;
     
        private:
     
          /** \brief Associated processor */
          boost::shared_ptr < processor::monitor >               monitor;

          /** \brief Associates a sip layout element with a wxWidgets control */
          element_for_window_map                                 element_for_window;
     
          /** \brief Delegate to actually send a display update */
          boost::function < void (sip::layout::element const&) > send_display_update;

        private:
     
          /** \brief Handler for when a button was clicked */
          void button_clicked(wxCommandEvent&);
     
          /** \brief Handler for when radio buttons changed state */
          void radio_button_selected(wxCommandEvent&);
     
          /** \brief Handler for when radio buttons changed state */
          void checkbox_clicked(wxCommandEvent&);
     
          /** \brief Handler for when radio buttons changed state */
          void text_field_changed(wxCommandEvent&);

        public:

          /** \brief Constructor */
          state_change_handler(boost::shared_ptr < processor::monitor >&);

          /** \brief Clears the element_for_window map */
          inline void clear();

          /** \brief Associate a sip layout element pointer with a wxWindow pointer */
          inline void associate(wxObject*, sip::layout::element const*);

          /** \brief Update the (G)UI state for a specific element */
          void update(sip::layout::mediator* m, sip::layout::element const*);

          /** \brief Gets the monitor for the associated process */
          boost::shared_ptr < processor::monitor >& get_monitor();
      };

      /**
       * @param[in] s the processor associated with this display
       **/
      inline state_change_handler::state_change_handler(boost::shared_ptr < processor::monitor >& s) : monitor(s) {
      }

      inline void state_change_handler::clear() {
        element_for_window.clear();
      }

      inline void state_change_handler::associate(wxObject* o, sip::layout::element const* e) {
        element_for_window[o] = e;
      }

      inline boost::shared_ptr < processor::monitor >& state_change_handler::get_monitor() {
        return (monitor);
      }
    }
    /// \endcond

    /**
     * @brief Display window associated with a tool
     **/
    class tool_display : public wxPanel {
      friend class GUI::project;
      friend class GUI::project::builder;

      private:

        /** \brief The GUI project view to which this display `belongs' */
        GUI::project*                   context;

        /** \brief The event handler */
        detail::state_change_handler    event_handler;

        /** \brief Abstract description of the layout of this panel */
        sip::layout::tool_display::sptr current_layout;

        /** \brief Sizer that contains the content part */
        wxSizer*                        content;

        /** \brief Sizer of the tool display control buttons */
        wxSizer*                        control_bar;

        /** \brief Sizer of the tool display control buttons */
        wxTextCtrl*                     log;

      private:

        /** \brief Initialises widgets */
        void build();

        /** \brief Builds the specified layout within this window */
        void instantiate(boost::weak_ptr < sip::layout::tool_display >, sip::layout::tool_display::sptr l);

        /** \brief Update the (G)UI state for a list of elements */
        void update(boost::weak_ptr < sip::layout::tool_display >, std::vector < sip::layout::element const* >);

        /** \brief Update the log with incoming status messages */
        void update_log(boost::weak_ptr < sip::layout::tool_display >, sip::report::sptr l);

        /** \brief Set a new layout description */
        void schedule_layout_change(sip::layout::tool_display::sptr);

        /** \brief Set a new layout description */
        void schedule_layout_update(std::vector < sip::layout::element const* > const&);

        /** \brief Set a new layout description */
        void schedule_log_update(sip::report::sptr);

        /** \brief Handler for panel control button events */
        void on_panel_button_clicked(wxCommandEvent&);

        /** \brief Helper function to ensure a scrollbar is (in)visible when necessary */
        void toggle_scrollbar_helper();

      public:

        /** \brief Constructor */
        tool_display(wxWindow*, project*, boost::shared_ptr < processor::monitor >& s);

        /** \brief Sets the title of the display window */
        void set_title(wxString);

        /** \brief Removes itself from the parent window */
        void remove();
    };
  }
}

#endif
