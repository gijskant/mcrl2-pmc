// Author(s): Frank Stappers
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file outputpanel.h
//
// Implements the output class used to display log messages.

#include "wx.hpp" // precompiled headers

#include <memory>
#include <streambuf>
#include <iostream>

#include "outputpanel.h"
#include "mcrl2/core/messaging.h"

class message_relay;

static void relay_message(const ::mcrl2::core::messageType t, const char* data);

class text_control_buf : public std::streambuf {

  private:
    wxTextCtrl&    m_control;
  public:

   text_control_buf(wxTextCtrl& control) : std::streambuf(), m_control(control) {
   }

   int overflow(int c) {
     wxDateTime now = wxDateTime::Now();
     m_control.AppendText( now.FormatTime() +  wxString(static_cast< wxChar >(c)));

     return 1;
   }

   std::streamsize xsputn(const char * s, std::streamsize n) {
     wxDateTime now = wxDateTime::Now();
     m_control.AppendText( now.FormatTime() +  wxString(s, wxConvLocal, n));

     pbump(n);

     return n;
   }
};

class message_relay;

std::auto_ptr < message_relay > communicator;

class message_relay {
  friend void relay_message(const ::mcrl2::core::messageType, const char* data);

  private:

    wxTextCtrl&      m_control;
    std::streambuf*  m_error_stream;

  private:

    static bool initialise_once(wxTextCtrl& control) {
      communicator.reset(new message_relay(control));

      return true;
    }

    message_relay(wxTextCtrl& control) : m_control(control) {
      m_error_stream = std::cerr.rdbuf(new text_control_buf(m_control));

      mcrl2::core::gsSetCustomMessageHandler(relay_message);
    }

    void message(const char* data) {
      wxDateTime now = wxDateTime::Now();
      m_control.AppendText( now.FormatTime() + wxT(" ** ") + wxString(data, wxConvLocal));
    }

  public:

    static bool initialise(wxTextCtrl& control) {
      static bool initialised = initialise_once(control);

      return initialised;
    }

    ~message_relay() {
      mcrl2::core::gsSetCustomMessageHandler(0);

      delete std::cerr.rdbuf(m_error_stream);
    }
};

static void relay_message(const ::mcrl2::core::messageType t, const char* data) {
  switch (t) {
    case mcrl2::core::gs_notice:
      break;
    case mcrl2::core::gs_warning:
      break;
    case mcrl2::core::gs_error:
    default:
      communicator->message(data);
      break;
  }
}

outputpanel::outputpanel(wxWindow *p_parent)
: wxTextCtrl(p_parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP)
{
  message_relay::initialise(*this);
}

std::string outputpanel::PrintTime(){
  wxDateTime now = wxDateTime::Now();
  return std::string(now.FormatTime().mb_str()) + " ** ";
};


