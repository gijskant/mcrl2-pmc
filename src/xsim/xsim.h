#ifndef __xsim_H__
#define __xsim_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "xsim.h"
#endif

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

class XSim: public wxApp
{
public:
    virtual bool OnInit();
    virtual int OnExit();
};

#endif
