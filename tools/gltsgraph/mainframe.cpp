#include <memory>
#include "mainframe.h"
#include "ids.h"
#include <wx/menu.h>
#include "export_svg.h"
#include "export_xml.h"
#include "export_latex.h"

// For compatibility with older wxWidgets versions (pre 2.8)
#if (wxMINOR_VERSION < 8)
# define wxFD_SAVE wxSAVE
# define wxFD_OPEN wxOPEN
# define wxFD_CHANGE_DIR wxCHANGE_DIR
# define wxFD_OVERWRITE_PROMPT wxOVERWRITE_PROMPT
#endif

using namespace IDS;

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_MENU(wxID_OPEN, MainFrame::onOpen)
  EVT_MENU(myID_MENU_EXPORT, MainFrame::onExport)
  EVT_MENU(wxID_EXIT, MainFrame::onQuit)
  EVT_MENU(myID_DLG_INFO, MainFrame::onInfo)
  EVT_MENU(myID_DLG_ALGO, MainFrame::onAlgo)
  EVT_MENU(wxID_PREFERENCES, MainFrame::onSettings)
  EVT_MENU(wxID_PREFERENCES, MainFrame::onSettings)
END_EVENT_TABLE()


MainFrame::MainFrame(GLTSGraph* owner)
  : wxFrame(NULL, wxID_ANY, wxT("GLTSGraph"))
{
  app = owner;

  algoDlg = new AlgoDialog(app, this);
  settingsDlg = new SettingsDialog(app, this);
  infoDlg = new InfoDialog(this);

  setupMenuBar();
  setupMainArea();

  SetSize(800, 600);
  CentreOnScreen();
}

void MainFrame::setupMenuBar()
{
  wxMenuBar* menuBar = new wxMenuBar;

  // File menu
  wxMenu* fileMenu = new wxMenu;
  fileMenu->Append(wxID_OPEN, wxT("&Open...\tCTRL-o"), 
               wxT("Read an LTS from a file."));
  fileMenu->Append(myID_MENU_EXPORT, wxT("E&xport to...\tCTRL-x"), 
               wxT("Export this LTS to file."));

  fileMenu->Append(wxID_EXIT, wxT("&Quit \tCTRL-q"), wxT("Quit GLTSGraph."));

  // Tools menu
  wxMenu* toolsMenu = new wxMenu;
  toolsMenu->Append(myID_DLG_ALGO, wxT("O&ptimization... \tCTRL-p"),
                    wxT("Display dialog for layout optimization algorithm."));
  toolsMenu->Append(wxID_PREFERENCES, wxT("Settings..."), 
                   wxT("Display the visualization settings dialog."));
  toolsMenu->Append(myID_DLG_INFO, wxT("&Information... \tCTRL-i"), 
                    wxT("Display dialog with information about this LTS."));

  // Help menu
  wxMenu* helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, wxT("&About..."), wxEmptyString);


  menuBar->Append(fileMenu, wxT("&File"));
  menuBar->Append(toolsMenu, wxT("&Tools"));
  menuBar->Append(helpMenu, wxT("&Help"));

  SetMenuBar(menuBar);
}

void MainFrame::setupMainArea()
{
  wxFlexGridSizer* mainSizer = new wxFlexGridSizer(1,1,0,0);
  mainSizer->AddGrowableCol(0);
  mainSizer->AddGrowableRow(0);

  int attribList[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };
  glCanvas = new GLCanvas(app, this, wxDefaultSize, attribList);

  mainSizer->Add(glCanvas, 1, wxALIGN_CENTER|wxEXPAND|wxALL, 0);
  
  mainSizer->Fit(this);
  SetSizer(mainSizer);
  Layout();
}

void MainFrame::onOpen(wxCommandEvent& /*event*/)
{
  wxFileDialog dialog(this, wxT("Select a file"), wxEmptyString, wxEmptyString,
    wxT("All supported formats(*.xml;*.aut;*.svc)|*.xml;*.aut;*.svc|XML layout file (*.xml)|*.xml|LTS format (*.aut; *.svc)|*.aut;*.svc|All files (*.*)|*.*"),
    wxFD_OPEN|wxFD_CHANGE_DIR);
  
  if (dialog.ShowModal() == wxID_OK)
  {
    wxString path = dialog.GetPath();
    std::string stPath(path.fn_str()); 

    app->openFile(stPath);
  }
}

void MainFrame::onExport(wxCommandEvent& /*event*/)
{
  
  wxString caption = wxT("Export layout as");
  wxString wildcard = wxT("Scalable Vector Graphics (*.svg)|*.svg|XML Layout file (*.xml)|*.xml|LaTeX TikZ drawing (*.tex)|*.tex");
  wxString defaultDir = wxEmptyString; // Empty string -> cwd
  
  wxString defaultFileName(app->getFileName().c_str(), wxConvLocal);
  
  // Strip extension from filename
  if(defaultFileName.Find('.') != -1 )
  {
    defaultFileName = defaultFileName.BeforeLast('.');
  }

  wxFileDialog dialog(this, caption, defaultDir, defaultFileName, wildcard,
                      wxFD_SAVE | wxFD_OVERWRITE_PROMPT | wxFD_CHANGE_DIR);

  if(dialog.ShowModal() == wxID_OK)
  {
    std::auto_ptr< Exporter > exporter;
    wxString fileName = dialog.GetPath();
    wxString extension = fileName.AfterLast('.');
    if(extension == fileName)
    {
      // No extension given, get it from the filter index
      switch(dialog.GetFilterIndex())
      {
        case 0: // SVG item
          fileName.Append(wxT(".svg"));
          exporter.reset(new ExporterSVG(app->getGraph()));
          break;
        case 1: // XML item
          fileName.Append(wxT(".xml"));
          exporter.reset(new ExporterXML(app->getGraph()));
          break;
        case 2: // TeX item
          fileName.Append(wxT(".tex"));
          exporter.reset(new ExporterLatex(app->getGraph()));
          break;
      }
    }
    else
    {
      // An extension was given
      if (extension == wxT("svg"))
      {
        exporter.reset(new ExporterSVG(app->getGraph()));
      }
      else if (extension == wxT("xml"))
      {
        exporter.reset(new ExporterXML(app->getGraph()));
      }
      else if (extension == wxT("tex"))
      {
        exporter.reset(new ExporterLatex(app->getGraph()));
      }
      else 
      {
        // Unknown file format, export to xml 
        exporter.reset(new ExporterXML(app->getGraph()));
      }
    }
    
    if(exporter->export_to(fileName))
    {
      wxMessageDialog msgDlg(
        this,
        wxT("The layout was exported to file:\n\n") + fileName,
        wxT("Layout exported"),
        wxOK | wxICON_INFORMATION);
      msgDlg.ShowModal();
    }
  }
}


void MainFrame::onQuit(wxCommandEvent& /*event */)
{
  Close(TRUE);
}
GLCanvas* MainFrame::getGLCanvas()
{
  return glCanvas;
}

void MainFrame::onInfo(wxCommandEvent& /* event */)
{
  infoDlg->Show();
}

void MainFrame::onAlgo(wxCommandEvent& /* event */)
{
  algoDlg->Show();
}

void MainFrame::onSettings(wxCommandEvent& /* event */)
{
  settingsDlg->Show();
}


void MainFrame::setLTSInfo(int is, int ns, int nt, int nl)
{
  infoDlg->setLTSInfo(is, ns, nt, nl);
  
  wxString title(app->getFileName().c_str(), wxConvLocal);
  title = wxT("GLTSGraph - ") + title;
  SetTitle(title);
}

