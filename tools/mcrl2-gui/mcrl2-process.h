/*
 * process.h
 *
 *  Created on: Jun 6, 2010
 *      Author: fstapper
 */

#ifndef MCRL2_GUI_PROCESS_H_
#define MCRL2_GUI_PROCESS_H_

#include <wx/process.h>
#include <wx/txtstrm.h>
#include "wx/defs.h"
#include <wx/timer.h>
#include <wx/thread.h>
#include <wx/listbox.h>

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_MY_PROCESS_END, 7777)
END_DECLARE_EVENT_TYPES()

DEFINE_EVENT_TYPE(wxEVT_MY_PROCESS_END)

// it may also be convenient to define an event table macro for this event type
#define EVT_MY_PROCESS_END(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_MY_PROCESS_END, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_MY_PROCESS_RUN, 7777)
END_DECLARE_EVENT_TYPES()

DEFINE_EVENT_TYPE(wxEVT_MY_PROCESS_RUN)

// it may also be convenient to define an event table macro for this event type
#define EVT_MY_PROCESS_RUN(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_MY_PROCESS_RUN, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),

class MyPipedProcess;
WX_DEFINE_ARRAY_PTR(MyPipedProcess *, MyActiveProcessArray)
;
MyActiveProcessArray running_processes;

wxTimer m_timerIdleWakeUp;

class MyProcess: public wxProcess {
public:
	MyProcess(wxProcess *parent ) :
		wxProcess(parent) {
		//m_parent = parent;
	}

	virtual void OnTerminate(int /* pid */, int /* status */) {
		// Delete object
		delete this;
	}
	;
};

// A Process for redirecting the output
class MyPipedProcess: public MyProcess {
public:

	long m_ext_pid;

	wxWindow *m_parent;

	MyPipedProcess(wxWindow *parent) :
		MyProcess(NULL) {
		Redirect();
		m_parent = parent;
	}

	void AddAsyncProcess(wxListBox *output ) {
		m_listbox_output = output;
		if (running_processes.IsEmpty()) {
			// we want to start getting the timer events to ensure that a
			// steady stream of idle events comes in -- otherwise we
			// wouldn't be able to poll the child process input
			m_timerIdleWakeUp.Start(100);
		}
		//otherwise timer is already running

		running_processes.Add(this);
	}

	virtual void OnTerminate(int pid, int status) {
		// show output remainder
		while (HasInput())
			;

		running_processes.Remove(this);

		MyProcess::OnTerminate(pid, status);

		if (running_processes.IsEmpty()) {
			// Stop timer
			m_timerIdleWakeUp.Stop();
		}

    wxCommandEvent eventCustom(wxEVT_MY_PROCESS_END);
    wxPostEvent(m_parent, eventCustom);

	}
	;

	virtual bool HasInput() {
		bool hasInput = false;

		if (IsInputAvailable()) {
			wxTextInputStream tis(*GetInputStream());
			wxString m_msg;
			// assumption output is line buffered
			// m_msg << m_cmd << wxT(" (stdout): ") << tis.ReadLine();
			m_msg << tis.ReadLine();
			if (m_listbox_output != NULL)
        {
          m_listbox_output->Append(m_msg);

          if (m_listbox_output == wxWindow::FindFocus())
          {
            // AutoScroll
            m_listbox_output->Select(m_listbox_output->GetCount() - 1);
            m_listbox_output->SetSelection(wxNOT_FOUND);
          }
        }
        m_msg.Clear();

			hasInput = true;
		}

		if (IsErrorAvailable()) {
      wxTextInputStream tis(*GetErrorStream());
      wxString m_msg;
      // assumption output is line buffered
      m_msg << tis.ReadLine();
      if (m_listbox_output != NULL)
      {
        m_listbox_output->Append(m_msg);
        if (m_listbox_output == wxWindow::FindFocus())
        {
          // AutoScroll
          m_listbox_output->Select(m_listbox_output->GetCount() - 1);
          m_listbox_output->SetSelection(wxNOT_FOUND);
        }
      }
      m_msg.Clear();
      hasInput = true;
    }

		return hasInput;
	}
	;
protected:
	wxListBox *m_listbox_output;
};

#endif /* MCRL2_GUI_PROCESS_H_ */
