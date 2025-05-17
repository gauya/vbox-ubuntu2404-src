#include <wx/wx.h>
#include "gl_canvas.h"

class MyApp : public wxApp {
public:
    bool OnInit() override;
};

class MyFrame : public wxFrame {
public:
    MyFrame();
    void OnOpenFile(wxCommandEvent&);

private:
    MyGLCanvas* glCanvas;
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "STL Viewer", wxDefaultPosition, wxSize(800, 600)) {
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_OPEN, "&Open STL...\tCtrl-O");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    SetMenuBar(menuBar);

    glCanvas = new MyGLCanvas(this);

    Bind(wxEVT_MENU, &MyFrame::OnOpenFile, this, wxID_OPEN);
}

void MyFrame::OnOpenFile(wxCommandEvent&) {
    wxFileDialog openFileDialog(this, "Open STL file", "", "",
                                "STL files (*.stl)|*.stl", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL) return;

    glCanvas->LoadModel(openFileDialog.GetPath().ToStdString());
}

