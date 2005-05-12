#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "xsim.h"
#endif

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "xsim.h"
#include "xsimmain.h"
#include "aterm2.h"
#include "gsfunc.h"

//------------------------------------------------------------------------------
// XSim
//------------------------------------------------------------------------------


XSim::XSim()
{
}

bool XSim::OnInit()
{
    XSimMain *frame = new XSimMain( NULL, -1, wxT("XSim"), wxPoint(-1,-1), wxSize(500,400) );
    frame->Show( TRUE );
    
    gsEnableConstructorFunctions();

    if ( argc > 1 )
    {
	frame->LoadFile(wxT(argv[1]));
    }

    return TRUE;
}

int XSim::OnExit()
{
    return 0;
}



IMPLEMENT_APP_NO_MAIN(XSim)
IMPLEMENT_WX_THEME_SUPPORT

int main(int argc, char **argv)
{
	ATerm bot;

	ATinit(argc,argv,&bot);

	return wxEntry(argc, argv);
}
