#include "gl_canvas.h"
#include <GL/gl.h>
#include <GL/glu.h>

MyGLCanvas::MyGLCanvas(wxWindow* parent)
    : wxGLCanvas(parent, wxID_ANY, nullptr, wxDefaultPosition, wxSize(800, 600)) {

    if (!parent->IsShown()) {
        parent->Show(true);
    }

    context = new wxGLContext(this);
    SetCurrent(*context);
}

void MyGLCanvas::LoadModel(const std::string& filename) {
    modelData = LoadSTL(filename);
    Refresh();
}

void MyGLCanvas::Render() {
    if (!IsShown()) return;  // 창이 표시되지 않으면 렌더링하지 않음

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // OpenGL 렌더링 코드...

    SwapBuffers();  // 화면을 업데이트
}

