#ifndef GL_CANVAS_H
#define GL_CANVAS_H

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <vector>
#include <string>
#include "stl_loader.h"

class MyGLCanvas : public wxGLCanvas {
public:
    MyGLCanvas(wxWindow* parent);
    void LoadModel(const std::string& filename);

protected:
    void Render();
    void OnPaint(wxPaintEvent&);
    void OnMouse(wxMouseEvent&);

private:
    std::vector<Triangle> modelData;
    wxGLContext* context;
    float rotationAngle;
};

#endif
