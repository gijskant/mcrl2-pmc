#ifndef GUI_DIALOG_PROCESSOR_H
#define GUI_DIALOG_PROCESSOR_H

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/treectrl.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>

#include "gui_dialog_base.h"

namespace squadt {
  namespace GUI {
    namespace dialog {
      
      /** \brief Base class for processor level dialogs */
      class processor : public dialog::basic {
        public:

          /** \brief Constructor */
          processor(wxWindow*, wxString);

          /** \brief Virtual destructor */
          virtual ~processor() = 0;
      };

      class processor_details : public dialog::processor {
        friend class squadt::GUI::project;

        private:

          /** \brief Path to the project store where input/output objects are stored */
          wxString            project_store;

          /** \brief Tree control for tool selection */
          wxTreeCtrl*         tool_selector;

          /** \brief Text field that contains a name */
          wxTextCtrl*         name;

          /** \brief The list of input objects */
          wxListCtrl*         input_objects;

          /** \brief The list of output objects */
          wxListCtrl*         output_objects;

          /** \brief The processor for which to display information */
          squadt::processor*  target_processor;

        private:
        
          /** \brief Helper function that places the widgets */
          void build();

          /** \brief Helper function for filling the tool list with tools */
          void populate_tool_list(storage_format);

          /** \brief Helper function that actually adds a tool to the list of tools */
          void add_to_tool_list(const miscellaneous::tool_selection_helper::tools_by_category::value_type&);

          /** \brief Event handler for when something is selected in the tool_selector control */
          void on_tool_selector_item_selected(wxTreeEvent& e);

          /** \brief Event handler for when something is about to be selected in the tool_selector control */
          void on_tool_selector_item_select(wxTreeEvent& e);

        public:

          /** \brief Constructor */
          processor_details(wxWindow*, wxString, squadt::processor*);

          /** \brief Show or hide the tool selector */
          inline void show_tool_selector(bool b);

          /** \brief Show or hide the list of input objects */
          inline void show_input_objects(bool b);

          /** \brief Show or hide the list of output objects */
          inline void show_output_objects(bool b);

          /** \brief Constructor sets the name field */
          inline void set_name(wxString);
      };

      /**
       * @param p the parent window
       * @param t the title for the window
       **/
      inline processor::processor(wxWindow* p, wxString t) : dialog::basic(p, t, wxSize(650, 375)) {
      }

      inline processor::~processor() {
      }

      /**
       * @param b whether to show tool_selector
       **/
      inline void processor_details::show_tool_selector(bool b) {
        tool_selector->Show(b);
      }

      /**
       * @param b whether to show input_objects
       **/
      inline void processor_details::show_input_objects(bool b) {
        input_objects->Show(b);
      }

      /**
       * @param b whether to show output_objects
       **/
      inline void processor_details::show_output_objects(bool b) {
        output_objects->Show(b);
      }

      /**
       * @param n the new name
       **/
      inline void processor_details::set_name(wxString n) {
        name->SetValue(n);
      }
    }
  }
}

#endif

