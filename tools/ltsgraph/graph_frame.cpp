//  Copyright 2007 Didier Le Lann, Carst Tankink, Muck van Weerdenburg and Jeroen van der Wulp. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ./graph_frame.cpp

#include "graph_frame.h"
#include "workarounds.h"
#include "mcrl2/utilities/version_info.h"

#include <deque>
#include <map>

static const wxColour &border_colour_selected = *wxBLUE;

// For compatibility with older wxWidgets versions (pre 2.8)
#if (wxMINOR_VERSION < 8)
#define wxFD_SAVE wxSAVE 
#define wxFD_OVERWRITE_PROMPT wxOVERWRITE_PROMPT
#endif

BEGIN_EVENT_TABLE(GraphFrame, wxFrame)
  EVT_MENU(wxID_OPEN, GraphFrame::OnOpen)
  EVT_MENU(ID_MENU_EXPORT, GraphFrame::on_export)
  EVT_MENU(ID_BACKUP_CREATE, GraphFrame::CreateBackup)
  EVT_MENU(wxID_EXIT, GraphFrame::OnQuit)
  EVT_MENU(ID_OPTIMIZE, GraphFrame::OnOptimize)
  EVT_MENU(ID_STOP_OPTIMIZE, GraphFrame::OnStopOptimize)
  EVT_MENU(wxID_ABOUT, GraphFrame::on_about)

  EVT_CLOSE(GraphFrame::OnClose)

  EVT_CHECKBOX(ID_CHECK_NODE, GraphFrame::OnCheckNode)
  EVT_CHECKBOX(ID_CHECK_EDGE, GraphFrame::OnCheckEdge)
  EVT_CHECKBOX(ID_CHECK_CURVES, GraphFrame::on_check_curves)

  EVT_SPINCTRL(ID_SPIN_RADIUS, GraphFrame::on_spin_radius)
  
  EVT_BUTTON(ID_BUTTON_OPTI, GraphFrame::OnBtnOpti)
  EVT_BUTTON(ID_BUTTON_COLOUR, GraphFrame::on_btn_pick_colour)
  EVT_BUTTON(ID_BUTTON_LABEL_COLOUR, GraphFrame::on_btn_label_colour)
  EVT_BUTTON(ID_BUTTON_LABEL_TEXT, GraphFrame::on_btn_label_text)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(ViewPort, wxPanel)
  EVT_PAINT(ViewPort::OnPaint)
  EVT_LEFT_DOWN(ViewPort::PressLeft)
  EVT_MOTION(ViewPort::Drag)
  EVT_LEFT_UP(ViewPort::ReleaseLeft)
  EVT_RIGHT_UP(ViewPort::PressRight)
  EVT_SIZE(ViewPort::OnResize)
END_EVENT_TABLE()

static const int ctrl_radius = 3;

static vector<Node*> vectNode;
static vector<edge*> vectEdge;

GraphFrame::GraphFrame(const wxString& title, const wxPoint& pos, const wxSize& size, long style) 
  : wxFrame(NULL, -1, title, pos, size, style) {

    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    background_brush.SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    background_brush.SetStyle(wxSOLID);
 
  StopOpti = true;
  StoppedOpti = true;
  curve_edges = false;


  // values below are reset later when the right panel is setuped
  EdgeStiffness = 1.0; 
  NodeStrength = 1000.0; 
  NaturalLength = 20.0;
  node_radius = 10;

  BuildLayout();

}

void GraphFrame::BuildLayout() {

  //Menu
  CreateMenu();

  // Statusbar
  stb_coordinates = new wxStatusBar(this, wxID_ANY);
  this->SetStatusBar(stb_coordinates);

  int rightPanelWidth = INITIAL_WIN_WIDTH - (INITIAL_WIN_WIDTH / 4 + 15);
 
  wxSplitterWindow * sw = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & !wxSP_PERMIT_UNSPLIT);
  leftPanel = new ViewPort( sw, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER );
  rightPanel = new wxScrolledWindow( sw, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxVSCROLL );
  rightPanel->SetScrollbars(0, 10, 0, 100);
  sw->SetMinimumPaneSize(10);
  sw->SplitVertically(leftPanel,rightPanel);
  sw->SetSashPosition(rightPanelWidth,true);
  sw->SetSashGravity(1.0);

  wxBoxSizer * winSizer = new wxBoxSizer(wxHORIZONTAL);
  winSizer->Add(sw,1,wxEXPAND);

  wxBoxSizer * rightSizer = new  wxBoxSizer(wxVERTICAL);
  rightPanel->SetSizer(rightSizer);

  // setup the top part (information box)
  wxStaticBoxSizer* infoSizer = new wxStaticBoxSizer( wxVERTICAL, rightPanel, wxT("Information") );
  int lflags = wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL;
  int rflags = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL;
  
  wxFlexGridSizer* topRightSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
  initialStateLabel = new wxStaticText( rightPanel, wxID_ANY, wxEmptyString,
    wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  numberOfStatesLabel = new wxStaticText( rightPanel, wxID_ANY, wxEmptyString,
    wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  numberOfTransitionsLabel = new wxStaticText( rightPanel, wxID_ANY, wxEmptyString,
    wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  numberOfLabelsLabel = new wxStaticText( rightPanel, wxID_ANY, wxEmptyString,
    wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );

  topRightSizer->Add( new wxStaticText( rightPanel, wxID_ANY,	wxT("Initial state:         ") ), 0, lflags, 4 );
  topRightSizer->Add( initialStateLabel, 0, rflags, 3 );
  topRightSizer->Add( new wxStaticText( rightPanel, wxID_ANY, wxT("Number of states:      ") ), 0, lflags, 4 );
  topRightSizer->Add( numberOfStatesLabel, 0, rflags, 3 );
  topRightSizer->Add( new wxStaticText( rightPanel, wxID_ANY,	wxT("Number of transitions: ") ), 0, lflags, 4 );
  topRightSizer->Add( numberOfTransitionsLabel, 0, rflags, 3 );
  topRightSizer->Add( new wxStaticText( rightPanel, wxID_ANY,	wxT("Number of labels:      ") ), 0, lflags, 4 );
  topRightSizer->Add( numberOfLabelsLabel, 0, rflags, 3 );

  infoSizer->Add(topRightSizer, 1, wxEXPAND | wxALL, 0 );
  rightSizer->Add(infoSizer, 0, wxEXPAND | wxALL, 0 );

  rightSizer->AddSpacer(20);

  // setup the middle part (algorithm settings box)
  wxStaticBoxSizer* algoSettingsSizer = new wxStaticBoxSizer( wxVERTICAL, rightPanel, wxT("Algorithm settings") );
  wxFlexGridSizer* middleRightSizer = new wxFlexGridSizer( 0, 1, 0, 0 );
	
  sliderNodeStrength  = new wxSlider(rightPanel, wxID_ANY, 200, 0, 10000, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_LABELS);
  sliderEdgeStiffness = new wxSlider(rightPanel, wxID_ANY, 6, 0, 15, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_LABELS);
  sliderNaturalLength = new wxSlider(rightPanel, wxID_ANY, 3 * node_radius, 1, 500, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_LABELS);
  slider_speedup = new wxSlider(rightPanel, wxID_ANY, 50, 0, 250, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
  

  middleRightSizer->Add( new wxStaticText( rightPanel, wxID_ANY,	wxT("State repulsion") ), 0, lflags, 4 );
  middleRightSizer->Add(sliderNodeStrength, 0, rflags, 3 );
  middleRightSizer->Add( new wxStaticText( rightPanel, wxID_ANY,	wxT("Transition attracting force") ), 0, lflags, 4 );
  middleRightSizer->Add(sliderEdgeStiffness, 0, rflags, 3 );
  middleRightSizer->Add( new wxStaticText( rightPanel, wxID_ANY,	wxT("Natural transition length") ), 0, lflags, 4 );
  middleRightSizer->Add(sliderNaturalLength, 0, rflags, 3 );
  middleRightSizer->Add (new wxStaticText( rightPanel, wxID_ANY, wxT("Speedup") ), 0, lflags, 4);
  middleRightSizer->Add(slider_speedup, 0, rflags, 3);


  algoSettingsSizer->Add(middleRightSizer, 1, wxEXPAND | wxALL, 0 );
  rightSizer->Add(algoSettingsSizer, 0, wxEXPAND | wxALL, 0 );
  rightSizer->AddSpacer(20);
  
  // setup the bottom part (others settings box)
  wxStaticBoxSizer* othersSettingsSizer = new wxStaticBoxSizer( wxVERTICAL, rightPanel, wxT("Other settings") );
  ckNodeLabels = new wxCheckBox(rightPanel, ID_CHECK_NODE, wxT("Display state labels"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
  ckNodeLabels->SetValue(true);
  ckEdgeLabels = new wxCheckBox(rightPanel, ID_CHECK_EDGE, wxT("Display transition labels"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
  ckEdgeLabels->SetValue(true);
  ck_curve_edges = new wxCheckBox(rightPanel, ID_CHECK_CURVES, wxT("Modify transition curves"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
  ck_curve_edges->SetValue(false);

  othersSettingsSizer->Add(ckNodeLabels,   0, lflags, 4 );
  othersSettingsSizer->Add(ckEdgeLabels,   0, lflags, 4 );
  othersSettingsSizer->Add(ck_curve_edges, 0, lflags, 4 );

  wxFlexGridSizer* bottomRightSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
  spinNodeRadius = new wxSpinCtrl(rightPanel, ID_SPIN_RADIUS, wxT("10"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 2, 50, 10);
  bottomRightSizer->Add( new wxStaticText( rightPanel, wxID_ANY,	wxT("State radius") ), 0, lflags, 4 );
  bottomRightSizer->Add(spinNodeRadius, 0, rflags, 3 );
  othersSettingsSizer->Add(bottomRightSizer, 1, wxEXPAND | wxALL, 0 );

  // Button for opening colour picker for selected node.
  btn_pick_colour = new wxButton(rightPanel, ID_BUTTON_COLOUR, wxT("Edit node &colour"), wxDefaultPosition, wxDefaultSize);
  btn_pick_colour->Enable(false);

  // Button for opening colour picker for selected label.
  btn_label_colour = new wxButton(rightPanel, ID_BUTTON_LABEL_COLOUR, wxT("Edit label colour"), wxDefaultPosition, wxDefaultSize);
  btn_label_colour->Enable(false);

  // Button for opening text dialog for selected label caption
  btn_label_text = new wxButton(rightPanel, ID_BUTTON_LABEL_TEXT, wxT("Edit label text"), wxDefaultPosition, wxDefaultSize);
  btn_label_text->Enable(false);

  othersSettingsSizer->Add(btn_pick_colour, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 4);
  othersSettingsSizer->Add(btn_label_colour, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 4);
  othersSettingsSizer->Add(btn_label_text, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 4);

	
  rightSizer->Add(othersSettingsSizer, 0, wxEXPAND | wxALL, 0 );

  rightSizer->AddSpacer(20);

  btnOptiStop = new wxButton(rightPanel, ID_BUTTON_OPTI, wxT("&Neaten"), wxDefaultPosition, wxDefaultSize);
  btnOptiStop->Enable(false);

  rightSizer->Add(btnOptiStop, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 4 );

  SetSizer(winSizer);

  Layout();
  sw->UpdateSize();


}

void GraphFrame::CreateMenu() {

  menu = new wxMenuBar;
    
  //file
  file = new wxMenu;
  openItem        = file->Append( wxID_OPEN, wxT("&Open...\tCTRL-o"), wxT("") );


  export_to    = file->Append( ID_MENU_EXPORT, wxT("E&xport to... \tCTRL-x"), wxT("") );

  backupCreate    = file->Append( ID_BACKUP_CREATE, wxT("&Save layout\tCTRL-s"), wxT("") );
  
  quitItem        = file->Append( wxID_EXIT, wxT("&Quit\tCTRL-q"), wxT("") );
  menu->Append( file, wxT("&File") );

  //draw
  draw = new wxMenu;
  optimizeGraph = draw->Append(ID_OPTIMIZE, wxT("&Neaten layout \tCTRL-g"), wxT("") );
  stopOptimize = draw->Append(ID_STOP_OPTIMIZE, wxT("&Stop neatening layout\tCTRL-s"), wxT("") );
  menu->Append(draw, wxT("&Draw") );

  wxMenu *help = new wxMenu;
  about = help->Append(wxID_ABOUT, wxT("&About..."), wxEmptyString);
  menu->Append(help, wxT("Help"));


  SetMenuBar( menu );

	optimizeGraph->Enable(false);
	stopOptimize->Enable(false);
	export_to->Enable(false);
	backupCreate->Enable(false);
}

void GraphFrame::OnOpen( wxCommandEvent& /* event */ ) {
	StopOpti    = true;
	StoppedOpti = true;
	btnOptiStop->SetLabel(wxT("&Neaten"));
	wxFileDialog dialog( this, wxT("Select a LTS file (.aut .svc) or a position file (.ltsgraph)..."), wxT(""), wxT(""), 
											wxT("All supported formats (*.ltsgraph; *.aut;*.svc)|*.ltsgraph;*.aut;*.svc|Position data (*.ltsgraph)|*.ltsgraph|LTS format (*.aut; *.svc)|*.aut;*.svc|All files (*.*)|*.*"));

        dialog.SetDefaultItem(NULL);

	if ( dialog.ShowModal() == wxID_OK ) {
		vectEdge.clear();
		vectNode.clear();
		Init(dialog.GetPath());
                if ( ck_curve_edges->IsChecked() )
                {
                  for (size_t i = 0; i < vectEdge.size(); i++) {
                    vectEdge[i]->set_control_visible(curve_edges);
                  }
                }
                update_coordinates();
		Refresh();
	}
}

void GraphFrame::OnQuit( wxCommandEvent& /* event */ ) {
  Close( TRUE );
}

void GraphFrame::OnClose(wxCloseEvent& /*event*/) {
  StopOpti = true;
  SetEvtHandlerEnabled(false);

  static_cast < wxSplitterWindow* > (leftPanel->GetParent())->Unsplit(leftPanel);
  delete leftPanel;
  Destroy();
}

void GraphFrame::OnOptimize( wxCommandEvent& /* event */ ) {

	optimizeGraph->Enable(false);
	stopOptimize->Enable(true);
	btnOptiStop->SetLabel(wxT("&Stop"));
	StopOpti = false;
	StoppedOpti = false;
	while (!OptimizeDrawing(0.0) && !StopOpti) {
		wxTheApp->Yield(true); // to allow user to interrupt optimizing
	}

	StoppedOpti = true;
}

void GraphFrame::OnStopOptimize( wxCommandEvent& /* event */ ) {
	stopOptimize->Enable(false);
	optimizeGraph->Enable(true);
	btnOptiStop->SetLabel(wxT("&Neaten"));
	StopOpti = true;
}

void GraphFrame::OnCheckNode( wxCommandEvent& /* event */ ) {

	if (ckNodeLabels->IsChecked()) {
		for (size_t i = 0; i < vectNode.size(); i++)  
			vectNode[i]->ShowLabels();
	}
	else {
		for (size_t i = 0; i < vectNode.size(); i++)  
			vectNode[i]->HideLabels();
	}
        update_coordinates();
	Refresh();

}

void GraphFrame::OnCheckEdge( wxCommandEvent& /* event */ ) {

	if (ckEdgeLabels->IsChecked()) {
		for (size_t i = 0; i < vectEdge.size(); i++)  
			vectEdge[i]->ShowLabels();
	}
	else {
		for (size_t i = 0; i < vectEdge.size(); i++)  
			vectEdge[i]->HideLabels();
	}
        update_coordinates();
	Refresh();
}

void GraphFrame::on_check_curves(wxCommandEvent & /* event */ ) {
  curve_edges = ck_curve_edges->IsChecked();

  for (size_t i = 0; i < vectEdge.size(); i++) {
    vectEdge[i]->set_control_visible(curve_edges);
  }
  update_coordinates();
  Refresh();
}

void GraphFrame::OnBtnOpti( wxCommandEvent& event ) {
	if(StopOpti) {
		OnOptimize(event);
	}
	else {
		OnStopOptimize(event);
	}

}

void GraphFrame::on_btn_pick_colour( wxCommandEvent& /* event */ ) {
  wxColourData colour_picker_data;

  // Allow Windows users to pick full range of colours. (No effect on other
  // platforms
  colour_picker_data.SetChooseFull(true);

  // Get colour of the currently selected node.
  wxColour current_colour = leftPanel->get_selected_node()->get_node_colour(); 
  colour_picker_data.SetColour(current_colour);
  
  // Create colour picker dialog, and show it to the user
  wxColourDialog colour_picker(this, &colour_picker_data);

  if (colour_picker.ShowModal() == wxID_OK) {
    wxColourData returned_data = colour_picker.GetColourData();
    wxColour new_colour = returned_data.GetColour();
    leftPanel->get_selected_node()->set_node_colour(new_colour);
    leftPanel->Refresh();
  }
  
}

void GraphFrame::on_btn_label_colour( wxCommandEvent& /* event */) {
  wxColourData colour_picker_data;

  /* Allow Windows users to pick full range of colours. (No effect on other
   * platforms
   */
  colour_picker_data.SetChooseFull(true);

  /* Get colour of the currently selected label */
  wxColour current_colour = leftPanel->get_selected_edge()->get_label_colour();
  colour_picker_data.SetColour(current_colour);

  /* Create colour picker dialog, and show it to the user */
  wxColourDialog colour_picker(this, &colour_picker_data);

  if (colour_picker.ShowModal() == wxID_OK) {
    wxColourData returned_data = colour_picker.GetColourData();
    wxColour new_colour = returned_data.GetColour();
    leftPanel->get_selected_edge()->set_label_colour(new_colour);
    leftPanel->Refresh();
  }
}

void GraphFrame::on_btn_label_text( wxCommandEvent& /* event */) {
  wxString current_label= wxString(leftPanel->get_selected_edge()->get_lbl().c_str(), wxConvLocal);
  wxTextEntryDialog label_text_dialog(this, 
                                      wxT("Please enter the new label text"),
                                      wxT("Edit state label"),
                                      current_label,
                                      wxOK | wxCANCEL);

  if (label_text_dialog.ShowModal() == wxID_OK) {
    leftPanel->get_selected_edge()->set_label_text(label_text_dialog.GetValue());
    update_coordinates();
    Refresh();
  }
}

//init vectNode & vectEdge
void GraphFrame::Init(wxString LTSfile) {

  //leftPanel->sz = leftPanel->GetSize();
  // Set the title of the frame to ltsgraph - LTSfile
  wxString title = wxT("LTSGraph - ");
  title.append(LTSfile);
  SetTitle(title);
  
  string st_LTSfile = string(LTSfile.fn_str());
  
  //Find extension
  string ext = st_LTSfile.substr(st_LTSfile.find_last_of( '.' )+1);
  
  //initialize file name
  inputFileName = st_LTSfile.substr( st_LTSfile.find_last_of( '/' ) + 1 ); //without path
  inputFileName = inputFileName.substr( 0, inputFileName.find_last_of( '.' ) ); //without extension
  
  if (ext != "ltsgraph") {
    //read lts file
    lts mylts;
    
    if (mylts.read_from(st_LTSfile)) {
    	
      //Information about graph
      initialStateLabel->SetLabel(wxString::Format(wxT("%u"), mylts.initial_state()));
      numberOfStatesLabel->SetLabel(wxString::Format(wxT("%u"), mylts.num_states()));
      numberOfTransitionsLabel->SetLabel(wxString::Format(wxT("%u"), mylts.num_transitions()));
      numberOfLabelsLabel->SetLabel(wxString::Format(wxT("%u"), mylts.num_labels()));
      		
      //initialize vectNode
      int xmax = leftPanel->Get_Width()  - 2*node_radius;
      int ymax = leftPanel->Get_Height() - 2*node_radius;

      std::map< size_t, Node* > number_to_node;

      for (state_iterator si = mylts.get_states(); si.more(); ++si) {
        // The node used to contain the state number used by the input, this is changed to the index to fix a serious bug
        Node* new_node = new Node(vectNode.size(), static_cast< double > (rand() % xmax + node_radius),
                                                   static_cast< double > (rand() % ymax + node_radius), wxString::Format(wxT("%d"), *si), (mylts.initial_state() == *si));

        new_node->SetRadius(node_radius);

        //Init circle radius
        number_to_node[*si] = new_node;

        vectNode.push_back(new_node);
      }   

      //NaturalLength = (400 / vectNode.size())+8;
      
      //initialize vectEdge
      for (transition_iterator ti = mylts.get_transitions(); ti.more(); ++ti) {
        // todo error detection ...
        vectEdge.push_back(new edge(number_to_node[ti.from()], number_to_node[ti.to()], wxString(mylts.label_value_str(ti.label()).c_str(), wxConvLocal)));
      }
      
      optimizeGraph->Enable(true);
      stopOptimize->Enable(false);
      btnOptiStop->Enable(true);
      export_to->Enable(true);
      backupCreate->Enable(true);
      update_coordinates();
      Refresh();
    }
    else {
      wxMessageDialog(0, wxT("Error: cannot read LTS from file `") + LTSfile + wxT("'"), wxT("Fatal"), wxOK|wxICON_ERROR).ShowModal();
    }
  }
  else { //restore a backup
    RestoreBackup();
  }
}

// Helper for moving nodes with separated data for display and positioning
static std::deque< Node* > moved_vertices;

// Helper structure for OptimizeDrawing() method
struct vertex_and_forces {
  double x;
  double y;
  double x_force;
  double y_force;

  vertex_and_forces(double cx, double cy) : x(cx), y(cy), x_force(0), y_force(0) {
  }

  void apply_force() {
    x += x_force;
    y += y_force;

    x_force = y_force = 0;
  }
};

bool GraphFrame::OptimizeDrawing(double precision) {
  /* A short explanation is required here:
   * When the program is closed, the spinBoxes are destroyed, but when this
   * function is still running, NULL pointers are followed,leading to a 
   * segfault. 
   * However, on closing, StopOpti is set to true, op the status
   * of the spinboxes can be read from this variable (for some reason,
   * checking the spinboxes themselves is not possible, probably since
   * in between checking them and addressing them, They are destroyed).
   * After this, the function can stop. It returns false, since the 
   * function should return wether the required precision was achieved. which
   * it doesn't if it stops before calculation.
   */
  static int steps_taken = 0;

  if (!StopOpti) {
    EdgeStiffness = sliderEdgeStiffness->GetValue();
    NodeStrength  = sliderNodeStrength->GetValue();
    NaturalLength = sliderNaturalLength->GetValue();
    node_radius  = spinNodeRadius->GetValue();
  }
  else if (vectNode.size() == 0) {
    return false;
  }

  // Temporary measure to see whether vectNode is still the same (assuming that vectNode[0] changes in that case)
  static Node* recogniser  = 0;

  // vectNode[i].GetX() == vertices.x && vectNode[i].GetY() == vertices.y
  static std::vector< vertex_and_forces > vertices;

  // Copy of node positions that do not need to change for drawing (in so doing
  // avoiding the numeric instability resulting from scaling to match viewport)
  if (recogniser != vectNode[0]) {
    recogniser = vectNode[0];

    vertices.clear();
    vertices.reserve(vectNode.size());

    for (std::vector< Node* >::const_iterator i = vectNode.begin(); i != vectNode.end(); ++i) {
      vertices.push_back(vertex_and_forces((*i)->GetX(), (*i)->GetY()));
    }
  }

  // Finally calculate the attracting forces of the edges.  
  if (EdgeStiffness) {
    const double saves_division = log(NaturalLength);

    for (std::vector< edge* >::const_iterator i = vectEdge.begin(); i != vectEdge.end(); ++i) { 
      vertex_and_forces& p1 = vertices[(*i)->get_n1()->Get_num()];
      vertex_and_forces& p2 = vertices[(*i)->get_n2()->Get_num()];
 
      const double xdiff = p2.x - p1.x;
      const double ydiff = p2.y - p1.y;
      const double distance = sqrt((xdiff*xdiff) + (ydiff*ydiff));
 
      // Linear approach : 
      // double s = (EdgeStiffness * (distance - NaturalLength)) / distance;
      // Logarithmic approach : 
      const double force_shared_component = EdgeStiffness * (log(distance) - saves_division) / distance;
 
      if (force_shared_component < 1) { // 0 < s disallows repulsion
        const double sx = force_shared_component * xdiff;
        const double sy = force_shared_component * ydiff;
 
        p1.x_force += sx;
        p1.y_force += sy;
        p2.x_force -= sx;
        p2.y_force -= sy;
      }
    }
  }

  // Replace the nodes & edges according to their new position (sum of forces for all vertices and both components)
  double achieved_precision = 0.0;

  // minimum and maximum of vertex coordinates (only for scaling to viewport)
  double xmin = std::numeric_limits< double >::max();
  double xmax = std::numeric_limits< double >::min();
  double ymin = std::numeric_limits< double >::max();
  double ymax = std::numeric_limits< double >::min();

  for (std::vector< vertex_and_forces >::iterator i = vertices.begin(); i != vertices.end(); ++i) { 
    // First calculate the repulsing force for node i with respect to
    // the other nodes, and cumulate it in <arraySumForceX[i],arraySumForceY[i];
    for (std::vector< vertex_and_forces >::iterator j = i + 1; j != vertices.end(); ++j) { 
      const double xdiff = j->x - i->x;
      const double ydiff = j->y - i->y;
      
      //Euclidean distance
      const double distance = sqrt((xdiff*xdiff) + (ydiff*ydiff));

      const double force_shared_component = (node_radius < distance) ?
                      NodeStrength / (distance * distance * distance) : 0.5;

      const double sx = force_shared_component * xdiff;
      const double sy = force_shared_component * ydiff;

      i->x_force -= sx;
      i->y_force -= sy;
      j->x_force += sx;
      j->y_force += sy;
    }

    if (!vectNode[i - vertices.begin()]->IsLocked()) {
      // Calculate the just achieved precision of the drawing
      achieved_precision += fabs(i->x_force) + fabs(i->y_force);

      i->apply_force();
    }
    else {
      i->x_force = 0;
      i->y_force = 0;
    }

    if (i->x < xmin) {
      xmin = i->x;
    }
    else if (xmax < i->x) {
      xmax = i->x;
    }

    if (i->y < ymin) {
      ymin = i->y;
    }
    else if (ymax < i->y) {
      ymax = i->y;
    }
  }

  // As of this point the positioning algorithm is finished, however some
  // viewport specific transformation must be done. In a redesign positioning
  // and display should be separated.
  if (slider_speedup->GetValue() < ++steps_taken || !moved_vertices.empty()) {
    // Scale such that all nodes have a position within the viewport (relative
    // distances are respected up to numeric precision)
    double window_width  = static_cast < double > (leftPanel->Get_Width());
    double window_height = static_cast < double > (leftPanel->Get_Height());
 
    double xscale = (window_width  - 2 * node_radius) / (xmax - xmin);
    double yscale = (window_height - 2 * node_radius) / (ymax - ymin);
 
    const double xcentre = (xmin + xmax) / 2;
    const double ycentre = (ymin + ymax) / 2;
 
    if (1 < xscale) {
      xscale = 1;
    }
    if (1 < yscale) {
      yscale = 1;
    }

    const double half_window_height = window_height / 2;
    const double half_window_width  = window_width / 2;

    // Replace moved nodes
    if (!moved_vertices.empty()) {
      do {
        size_t moved = moved_vertices.front()->Get_num();
     
        vertices[moved].x = (vectNode[moved]->GetX() - half_window_width) / xscale + xcentre;
        vertices[moved].y = (vectNode[moved]->GetY() - half_window_height) / yscale + ycentre;
   
        moved_vertices.pop_front();
      }
      while (!moved_vertices.empty());
    }

    if (slider_speedup->GetValue() < steps_taken) {
      steps_taken = 0;

      for (size_t i = 0; i != vectNode.size(); ++i) {
        const double xdiff = vertices[i].x - xcentre;
        const double ydiff = vertices[i].y - ycentre;
  
        vectNode[i]->SetXY(xdiff * xscale + half_window_width, ydiff * yscale + half_window_height);
    
        vectNode[i]->SetRadius(node_radius);  
      }
    }

    update_coordinates();

    Refresh();
  }

  return achieved_precision < vectNode.size() * precision;
}

void GraphFrame::Draw(wxAutoBufferedPaintDC * myDC) {
  myDC->SetBackground(background_brush);
  myDC->Clear();
  
  //Call Edge and Node OnPaint() method (Edge 1st)
  for (size_t n = 0; n < vectEdge.size(); n++) {
    if (ckEdgeLabels->GetValue() ) {
      vectEdge[n]->ShowLabels();
    }
    else {
      vectEdge[n]->HideLabels();
    }
    vectEdge[n]->on_paint(myDC);
  }

  int radius = spinNodeRadius->GetValue();
      
  for (size_t n = 0; n < vectNode.size(); n++) {
    // Set the radius of the nodes
    if (ckNodeLabels->GetValue() ) {
      vectNode[n]->ShowLabels();
    }
    else {
      vectNode[n]->HideLabels();
    }
    vectNode[n]->SetRadius(radius);
    vectNode[n]->OnPaint(myDC);
  }
}
void GraphFrame::on_about(wxCommandEvent& /* event */) {
  wxString caption = wxT("About LTSGraph");
  wxString content = wxString();
  content += wxString(get_version_information("LTSGraph").c_str(),wxConvLocal);
  content += wxT("\n");
  content += wxT("\n");
  content += wxT("Tool for visualising labelled transition systems.\n");
  content += wxT("Developed by Didier Le Lann and Carst Tankink.\n");
  content += wxT("\n");
  content += wxT("This tool is distributed as part of the mCRL2 toolset.\n");
  content += wxT("For information see http://www.mcrl2.org\n");
  content += wxT("\n");
  content += wxT("For feature requests or bug reports,\n");
  content += wxT("please visit http://www.mcrl2.org/issuetracker");
  wxMessageBox(content, caption, wxOK| wxICON_INFORMATION, this, wxDefaultPosition.x, wxDefaultPosition.y);
}

 
void GraphFrame::on_export(wxCommandEvent& /* event */) {
  string str(inputFileName);
  wxString wx_str(str.c_str(), wxConvLocal);
  
  wxString caption = wxT("Export layout as");
  wxString wildcard = wxT("Scalable Vector Graphics (*.svg)|*.svg|LaTeX source (*.tex)|*.tex");
  wxString default_dir = wxEmptyString;
  wxString default_file_name = wx_str + wxT(".svg");

  wxFileDialog export_dialog(this, caption, default_dir, default_file_name, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (export_dialog.ShowModal() == wxID_OK) {
    wxString file_name = export_dialog.GetPath();
    wxString extension = file_name.AfterLast('.');
    if (extension == file_name) {
      /* No extension given, get it from filter index */
      switch (export_dialog.GetFilterIndex()) {
        case 0: //SVG item
          file_name.Append(wxT(".svg"));
          export_svg(file_name);
          break;
        case 1: //Latex item
          file_name.Append(wxT(".tex"));
          export_to_latex(file_name);
          break;
      }
    }
    else {
      if (extension == wxT("svg")) {
        export_svg(file_name);
      }
      else if (extension == wxT("tex")) {
        export_to_latex(file_name);
      }
      else { 
        /* Not a recognised format, assume svg */
        file_name.Append(wxT(".svg"));
        export_svg(file_name);
      }
    }
  }
}

void GraphFrame::export_to_latex( wxString export_file_name) {

	vector<nodeLatex> vectNodeLatex;
	vector<edgeLatex> vectEdgeLatex;

	nodeLatex StructNodeLatex;
	edgeLatex StructEdgeLatex;

	for (size_t n = 0; n < vectNode.size(); n++) {
			StructNodeLatex.num = vectNode[n]->Get_num();
			StructNodeLatex.x   = vectNode[n]->GetX();
			StructNodeLatex.y   = vectNode[n]->GetY();
			StructNodeLatex.lbl = vectNode[n]->GetLabel();

			vectNodeLatex.push_back(StructNodeLatex);
	}


	for (size_t n = 0; n < vectEdge.size(); n++) {
			StructEdgeLatex.numNode1 = vectEdge[n]->get_n1()->Get_num();
			StructEdgeLatex.numNode2 = vectEdge[n]->get_n2()->Get_num();
			StructEdgeLatex.lbl      = vectEdge[n]->get_lbl();

			vectEdgeLatex.push_back(StructEdgeLatex);
	}

       wxString caption = wxT("Export latex file as...");
       wxString wildcard = wxT("TeX documents (*.tex)");
       wxString default_dir = wxEmptyString;
       wxString default_filename((inputFileName + ".tex").c_str(), wxConvLocal);

       wxFileDialog export_ltx(this, caption, default_dir, default_filename, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        
       ExportToLatex * ltx = new ExportToLatex(export_file_name.c_str(), vectNodeLatex,vectEdgeLatex,leftPanel->Get_Height());
 
      if (ltx->Generate()) {
        wxMessageDialog msg_dlg(
          this,
          wxT("The layout was exported to file:\n\n") + export_file_name,
          wxT("Layout exported"),
          wxOK | wxICON_INFORMATION,
          wxDefaultPosition);
        msg_dlg.ShowModal();
      }
      //XXX else part disabled because the default wxWidgets logger already
      //    displays a message
      //else { 
      //wxMessageBox(wxT("Export unsuccessful"),wxT("Error"),wxOK | wxICON_ERROR, this, wxDefaultPosition.x, wxDefaultPosition.y);
      //}

    delete ltx;

}

void GraphFrame::export_svg(wxString export_file_name) {
  vector<node_svg> vect_node_svg;
  vector<edge_svg> vect_edge_svg;

  node_svg struct_node_svg;
  edge_svg struct_edge_svg;

  // Export node data
  for (size_t n = 0; n < vectNode.size(); n++) {
    struct_node_svg.num = vectNode[n]->Get_num();
    struct_node_svg.x = vectNode[n]->GetX();
    struct_node_svg.y = vectNode[n]->GetY();
    struct_node_svg.radius = vectNode[n]->GetRadius();
    struct_node_svg.label = vectNode[n]->GetLabel();
    struct_node_svg.red = vectNode[n]->get_node_colour().Red();
    struct_node_svg.green = vectNode[n]->get_node_colour().Green();
    struct_node_svg.blue = vectNode[n]->get_node_colour().Blue();

    double label_x;
    //Adjust label x position according to the length
    wxString lbl(vectNode[n]->GetLabel().c_str(), wxConvLocal);
    switch (lbl.Length()) {
      case 1:  label_x = vectNode[n]->GetX()-POS_NODE_LBL_X;  break;
      case 2:  label_x = vectNode[n]->GetX()-POS_NODE_LBL_X-3;break;
      case 3:  label_x = vectNode[n]->GetX()-POS_NODE_LBL_X-6;break;
      default: label_x = vectNode[n]->GetX()-POS_NODE_LBL_X;  break;
    }
	
    double label_y=vectNode[n]->GetY()+POS_NODE_LBL_Y;

    struct_node_svg.label_x = label_x;
    struct_node_svg.label_y = label_y;
    vect_node_svg.push_back(struct_node_svg);
  }

  // Export curve data
  for (size_t n = 0; n < vectEdge.size(); n++) {
    struct_edge_svg.start_x = vectEdge[n]->get_x_pos1();
    struct_edge_svg.start_y = vectEdge[n]->get_y_pos1();
    struct_edge_svg.end_x = vectEdge[n]->get_x_pos2();
    struct_edge_svg.end_y = vectEdge[n]->get_y_pos2();
    struct_edge_svg.end_radius = vectEdge[n]->get_n2()->GetRadius();
    vectEdge[n]->get_spline_control_points(struct_edge_svg.spline_control_points);
    vectEdge[n]->get_arrow_points(struct_edge_svg.arrow_points);
    struct_edge_svg.lbl = vectEdge[n]->get_lbl();
    struct_edge_svg.lbl_x = vectEdge[n]->get_label_lower_x();
    struct_edge_svg.lbl_y = vectEdge[n]->get_label_lower_y();
    struct_edge_svg.red = vectEdge[n]->get_label_colour().Red();
    struct_edge_svg.green = vectEdge[n]->get_label_colour().Green();
    struct_edge_svg.blue = vectEdge[n]->get_label_colour().Blue();

    vect_edge_svg.push_back(struct_edge_svg);
  }
  

    // Create the exporter object and generate the svg file
  export_to_svg * svg = new export_to_svg(export_file_name, vect_node_svg, vect_edge_svg, leftPanel->Get_Height(), leftPanel->Get_Width());

    int message_x = wxDefaultPosition.x;
    int message_y = wxDefaultPosition.y;
  if (svg->generate()) {
    wxMessageDialog msg_dlg(
      this,
      wxT("The layout was exported to file:\n\n") + export_file_name,
      wxT("Layout exported"),
      wxOK | wxICON_INFORMATION,
      wxPoint(message_x, message_y));
    msg_dlg.ShowModal();
  }
  //XXX else part disabled because the default wxWidgets logger already
  //    displays a message
  //else { 
  //wxMessageBox(wxT("Export unsuccessful"), wxT("Error"), wxOK | wxICON_ERROR, this, message_x, message_y);
  //}
  delete svg;
  
}

void GraphFrame::CreateBackup(wxCommandEvent& event) {

  LtsgraphBackup bckp(leftPanel->GetSize());
  bckp.SetLayout(vectNode, vectEdge);
  bckp.SetInformation(initialStateLabel->GetLabel(), 
    numberOfStatesLabel->GetLabel(), 
    numberOfTransitionsLabel->GetLabel(), 
    numberOfLabelsLabel->GetLabel()  );

  bckp.SetAlgoSettings(sliderEdgeStiffness->GetValue(), sliderNodeStrength->GetValue(), sliderNaturalLength->GetValue());
  bckp.SetOtherSettings(spinNodeRadius->GetValue(), ckNodeLabels->IsChecked(), ckEdgeLabels->IsChecked());

  // Create a file dialog to show to the user
  wxString caption = wxT("Save layout as...");
  wxString wildcard = wxT("LTSgraph layout data (*.ltsgraph)");
  wxString default_dir = wxEmptyString;
  wxString default_file_name(inputFileName.c_str(), wxConvLocal);
  default_file_name.append(wxT(".ltsgraph"));

  wxFileDialog bckup_dialog(this, caption, default_dir, default_file_name, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (bckup_dialog.ShowModal() == wxID_OK) {
    wxString backup_filename = bckup_dialog.GetPath();
    // Get last part of the filename (backup expects a local file path
    if (bckp.Backup(backup_filename)) {
      wxMessageDialog msg_dlg(
        this,
        wxT("The layout was saved to file:\n\n") + backup_filename,
        wxT("Layout exported"),
        wxOK | wxICON_INFORMATION,
        wxDefaultPosition);
      msg_dlg.ShowModal();
    }
    //XXX else part disabled because the default wxWidgets logger already
    //    displays a message
    //else {
    //  wxMessageBox(wxT("Save unsuccessful"),wxT("Error"),wxOK | wxICON_ERROR, this, wxDefaultPosition.x, wxDefaultPosition.y);
    //}
  }
}

void GraphFrame::RestoreBackup() {

	LtsgraphBackup bckp(leftPanel->GetSize());
        wxString input_string(inputFileName.c_str(), wxConvLocal);
	if (bckp.Restore(input_string)) {

		initialStateLabel->SetLabel(bckp.GetInitialState());  
		numberOfStatesLabel->SetLabel(bckp.GetNumStates()); 
		numberOfTransitionsLabel->SetLabel(bckp.GetNumTransitions()); 
		numberOfLabelsLabel->SetLabel(bckp.GetNumLabels()); 

		sliderEdgeStiffness->SetValue((int)bckp.GetEdgeStiffness());
		sliderNodeStrength->SetValue((int)bckp.GetNodeStrength());
		sliderNaturalLength->SetValue((int)bckp.GetNaturalLength());
	
		spinNodeRadius->SetValue(bckp.GetStateRadius());
		ckNodeLabels->SetValue(bckp.GetStateLabel());
		ckEdgeLabels->SetValue(bckp.GetTransitionLabel());

		vectEdge.clear();
		vectNode.clear();
		vectEdge = bckp.GetVectEdge();
		vectNode = bckp.GetVectNode();
	        
                export_to->Enable(true);
		backupCreate->Enable(true);
			
		optimizeGraph->Enable(true);
		stopOptimize->Enable(false);
		btnOptiStop->Enable(true);

	}
	else {
	  wxMessageBox(wxT("Restore unsuccessful"),wxT("Error"),wxOK | wxICON_ERROR, this, wxDefaultPosition.x, wxDefaultPosition.y);
        }
	
        update_coordinates();	
	Refresh();


}

void GraphFrame::Resize(wxSize sz2) {

  double diff_x = (double) sz2.GetWidth() / (double) leftPanel->Get_Width();
  double diff_y = (double) sz2.GetHeight() / (double) leftPanel->Get_Height();

  for (size_t m = 0; m < vectNode.size(); ++m) {
    vectNode[m]->ForceSetXY(vectNode[m]->GetX() * diff_x, 
                       vectNode[m]->GetY() * diff_y);
    // We use ForceSetXY here, otherwise the pin can fall out of the window.
  }

}

void GraphFrame::FindNode(wxPoint pt) {
  bool show_labels = ckEdgeLabels->IsChecked();

  leftPanel->selection = none_t;

  for (size_t n = 0; n < vectNode.size(); n++) {
    double radius = vectNode[n]->GetRadius();
    double node_x = vectNode[n]->GetX();
    double node_y = vectNode[n]->GetY();

    if (node_x > pt.x-radius && node_x < pt.x + radius) {
      if (node_y > pt.y- radius && node_y < pt.y+ radius) {
        leftPanel->selection = node_t;
        leftPanel->selected_node = vectNode[n];
      }
    }
  }
  
  if (leftPanel->selection == none_t) { 
    for (size_t n = 0; n < vectEdge.size(); n++) {
        if (curve_edges && vectEdge[n]->get_x_control() > pt.x-ctrl_radius && vectEdge[n]->get_x_control() < pt.x+ctrl_radius &&
            vectEdge[n]->get_y_control() > pt.y-ctrl_radius && vectEdge[n]->get_y_control() < pt.y+ctrl_radius) {
          leftPanel->selection = edge_t;
          leftPanel->selected_edge = vectEdge[n];
        }
        else if (show_labels && vectEdge[n]->get_label_lower_x() < pt.x && pt.x < vectEdge[n]->get_label_higher_x() &&
                                vectEdge[n]->get_label_lower_y() < pt.y && pt.y < vectEdge[n]->get_label_higher_y()) {
          leftPanel->selection = edge_label_t;
          leftPanel->selected_edge = vectEdge[n];
        }
    }
  }

}

void GraphFrame::ReplaceAfterDrag(wxPoint pt, bool b) {
   switch(leftPanel->selection) {
     case (node_t):
       leftPanel->selected_node->ForceSetXY(pt.x,pt.y);//redefine node coord

       if (b) {
         leftPanel->selected_node->Unlock();
       }

       moved_vertices.push_front(leftPanel->selected_node);
       break;
     case (edge_t):
       leftPanel->selected_edge->set_control(pt.x, pt.y);
       break;
     case (edge_label_t):
       leftPanel->selected_edge->set_label_x(pt.x);
       leftPanel->selected_edge->set_label_y(pt.y);
     default: 
       break;
   }
      
}

void GraphFrame::FixNode() {
  if (leftPanel->selected_node->IsLocked()) { 
    leftPanel->selected_node->Unlock();
    leftPanel->selected_node_is_locked = false;
  }
  else { 
    leftPanel->selected_node->Lock();
    leftPanel->selected_node_is_locked = true;
  }
}


wxString GraphFrame::GetInfoCurrentNode(Node* info_node) const {

  wxString text;
  text.Printf(wxT("Current node: %u  ( %u , %u )"), 
              info_node->Get_num(),
	      (unsigned int) round(info_node->GetX()),
	      (unsigned int) round(info_node->GetY()));

  return  text;

}

wxString GraphFrame::GetInfoWinSize(wxSize sz2) const {
  wxString text;
  text.Printf(wxT("Window size: %d * %d"), sz2.GetWidth(), sz2.GetHeight());
	
  return  text;

}

void GraphFrame::enable_btn_colour_picker() {
  btn_pick_colour->Enable(true);
}

void GraphFrame::disable_btn_colour_picker() {
  btn_pick_colour->Enable(false);
}

void GraphFrame::enable_btn_label_colour() {
  btn_label_colour->Enable(true);
}

void GraphFrame::disable_btn_label_colour() {
  btn_label_colour->Enable(false);
}

void GraphFrame::enable_btn_label_text() {
  btn_label_text->Enable(true);
}

void GraphFrame::disable_btn_label_text() {
  btn_label_text->Enable(false);
}

int GraphFrame::get_node_radius() {
  return node_radius;
}

void GraphFrame::on_spin_radius(wxSpinEvent& /*event */) {
  node_radius = spinNodeRadius->GetValue();
  Refresh(); 
}

void GraphFrame::update_coordinates() {
  wxString status_text = wxT("");
  if (leftPanel->selection == node_t) {
      status_text.Printf(wxT("Position of selected node: (%g, %g)"), leftPanel->get_selected_node()->GetX(), leftPanel->get_selected_node()->GetY());
      stb_coordinates -> SetStatusText(status_text);
  }
}
////////////////////////////////VIEWPORT CLASS IMPLEMENTATION////////////////////////////////

ViewPort::ViewPort(wxWindow * parent, const wxPoint& pos, const wxSize& size, long style) 
  : wxPanel(parent, wxID_ANY, pos, size, style), selected_node_is_locked(false) { 

  GF = static_cast<GraphFrame*>(GetParent()->GetParent());
  selection = none_t;
  selected_node = NULL;
  selected_edge = NULL;

  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

Node * ViewPort::get_selected_node() {
  return selected_node;
}

edge * ViewPort::get_selected_edge() {
  return selected_edge;
}

void ViewPort::OnPaint(wxPaintEvent& /* event */) {
		sz = GetClientSize();

  //Draw the graph
  wxAutoBufferedPaintDC myDC(this);
  GF->Draw(&myDC);
}

//Resizing Graph
void ViewPort::OnResize(wxSizeEvent& event) {
  wxSize sz2 = GetClientSize();

	GF->Resize(sz2); 
        GF->update_coordinates();
	
	sz = sz2;

	Refresh();

}

void ViewPort::PressLeft(wxMouseEvent& event) {

  wxPoint pt_start = event.GetPosition();

  //Reset all colours and selections
  if (selected_node) {
    selected_node->reset_border_colour();
    selected_node = NULL;
  }
  if (selected_edge) {
    selected_edge->set_control_selected(false);
    selected_edge->set_label_selected(false);
    selected_edge = NULL;
  }
  
 
  // Find the node that is clicked (if any)
  GF->FindNode(pt_start);
  
  switch (selection) {
    case none_t:
      // Disable buttons for colour picking
      GF->disable_btn_colour_picker();
      GF->disable_btn_label_colour();
      GF->disable_btn_label_text();
      break;

    case node_t:
      // Give the node a colour to identify it on-screen.
      selected_node->set_border_colour(border_colour_selected);
      selected_node->Lock();
      // Activate button for colour picking
      GF->enable_btn_colour_picker();
      GF->disable_btn_label_colour();
      GF->disable_btn_label_text();
      break;

    case edge_t:
      // Toggle the edge as selected
      selected_edge->set_control_selected(true);
      GF->disable_btn_label_colour();
      GF->disable_btn_label_text();
      GF->disable_btn_colour_picker();
      break;

    case edge_label_t:
      selected_edge->set_label_selected(true);
      GF->enable_btn_label_colour();      
      GF->enable_btn_label_text();
      GF->disable_btn_colour_picker();
      break;
  }

  GF->update_coordinates();
  Refresh();
  
}

void ViewPort::Drag(wxMouseEvent& event) {
  int node_radius = GF->get_node_radius();

  if(event.Dragging() && !event.Moving() && !event.Entering() && !event.Leaving()) {
    wxPoint pt_end = event.GetPosition();//Find the destination 

    if (selection == node_t) {
      if (pt_end.x <= node_radius || pt_end.x >= sz.GetWidth()-node_radius  ||
          pt_end.y <= node_radius || pt_end.y >= sz.GetHeight()-node_radius) {

        return;
      }
    }
    else if (selection == edge_t) {
      if (pt_end.x <= ctrl_radius || pt_end.x >= sz.GetWidth() - ctrl_radius ||
          pt_end.y <= ctrl_radius || pt_end.y >= sz.GetHeight() - ctrl_radius) {

        return;
      }
    } 
    else if (selection == edge_label_t) {
      /* Calculate height and width of the label */
      double label_width =  selected_edge->get_label_higher_x() - selected_edge->get_label_lower_x();
      double label_height = selected_edge->get_label_higher_y() - selected_edge->get_label_lower_y();
      
      if (pt_end.x <= label_width  || pt_end.x >= sz.GetWidth() - label_width ||
          pt_end.y <= label_height || pt_end.y >= sz.GetHeight() - label_height) {

        return;
      }
    }

    GF->ReplaceAfterDrag(pt_end);
    GF->update_coordinates();
    Refresh();
  }
}

void ViewPort::ReleaseLeft(wxMouseEvent& event) {
  if (selection == node_t) {
    GF->ReplaceAfterDrag(event.GetPosition(), !selected_node_is_locked);

    Refresh();
  }
}    

void ViewPort::PressRight(wxMouseEvent& event) {

  wxPoint pt_fix = event.GetPosition();

  //Reset all colours and selections
  if (selected_node) {
    selected_node->reset_border_colour();
    selected_node = NULL;
  }
  if (selected_edge) {
    selected_edge->set_control_selected(false);
    selected_edge->set_label_selected(false);
    selected_edge = NULL;
  }

  //Find the node concerned by the fixing
  GF->FindNode(pt_fix);

  if (selection == node_t) {
    GF->FixNode();
    // Give the node a colour to identify it on-screen.
    selected_node->set_border_colour(border_colour_selected);
    // Activate button for colour picking
    GF->enable_btn_colour_picker();
    GF->disable_btn_label_colour();
    GF->disable_btn_label_text();
    GF->update_coordinates();
    Refresh();
  }

}

int ViewPort::Get_Width() {
	return sz.GetWidth();
}

int ViewPort::Get_Height() {
	return sz.GetHeight();
}


