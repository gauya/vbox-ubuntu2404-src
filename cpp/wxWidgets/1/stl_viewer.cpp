#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/sizer.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <memory>


bool LoadStl(const std::string& filename, std::vector<float>& vertices) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        wxLogError("Error loading STL: %s", importer.GetErrorString());
        return false;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
            const aiFace& face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; ++k) {
                unsigned int index = face.mIndices[k];
                vertices.push_back(mesh->mVertices[index].x);
                vertices.push_back(mesh->mVertices[index].y);
                vertices.push_back(mesh->mVertices[index].z);
            }
        }
    }

    return true;
}




class MyGLCanvas : public wxGLCanvas {
public:
    MyGLCanvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, nullptr, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE) {
        wxGLContextAttrs ctxAttrs;
        ctxAttrs.PlatformDefaults().CoreProfile().EndList();
        m_context = std::make_unique<wxGLContext>(this, nullptr, &ctxAttrs);


        Bind(wxEVT_PAINT, &MyGLCanvas::OnPaint, this);
        Bind(wxEVT_SIZE, &MyGLCanvas::OnSize, this);
        Bind(wxEVT_LEFT_DOWN, &MyGLCanvas::OnMouseDown, this);
        Bind(wxEVT_MOTION, &MyGLCanvas::OnMouseMotion, this);
        Bind(wxEVT_MOUSEWHEEL, &MyGLCanvas::OnMouseWheel, this);

        if (!LoadStl("your_stl_file.stl", m_vertices)) {  // ***REPLACE with your STL file!***
            wxLogError("Could not load STL file.");
        }
    }


    void OnPaint(wxPaintEvent& event) {
        wxPaintDC dc(this);
        SetCurrent(*m_context);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -5.0f);
        glRotatef(m_rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(m_rotationY, 0.0f, 1.0f, 0.0f);
        glScalef(m_zoom, m_zoom, m_zoom);

        glColor3f(1.0f, 1.0f, 1.0f);


        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < m_vertices.size(); i += 3) {
            glVertex3f(m_vertices[i], m_vertices[i+1], m_vertices[i+2]);
        }
        glEnd();

        glFlush();
        SwapBuffers();
    }

    void OnSize(wxSizeEvent& event) {
        wxSize size = event.GetSize();
        SetCurrent(*m_context);

        glViewport(0, 0, size.GetWidth(), size.GetHeight());

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0f, (double)size.GetWidth() / size.GetHeight(), 1.0f, 100.0f);

        glMatrixMode(GL_MODELVIEW);

        Refresh();
    }

    void OnMouseDown(wxMouseEvent& event) {
        m_previousMouseX = event.GetX();
        m_previousMouseY = event.GetY();
    }

    void OnMouseMotion(wxMouseEvent& event) {
        if (event.Dragging()) {
            m_rotationX += event.GetY() - m_previousMouseY;
            m_rotationY += event.GetX() - m_previousMouseX;
            m_previousMouseX = event.GetX();
            m_previousMouseY = event.GetY();
            Refresh();
        }
    }

    void OnMouseWheel(wxMouseEvent& event) {
        m_zoom += event.GetWheelRotation() / 1000.0f;
        Refresh();
    }




private:
    std::unique_ptr<wxGLContext> m_context;
    std::vector<float> m_vertices;
    float m_rotationX = 0.0f;
    float m_rotationY = 0.0f;
    float m_zoom = 1.0f;
    int m_previousMouseX = 0;
    int m_previousMouseY = 0;
};



class MyFrame : public wxFrame {
public:
    MyFrame() : wxFrame(NULL, wxID_ANY, "STL Viewer") {
        MyGLCanvas* canvas = new MyGLCanvas(this);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(canvas, 1, wxEXPAND);
        SetSizer(sizer);

        SetClientSize(800, 600);
        Show();
    }
};

class MyApp : public wxApp {
public:
    bool OnInit() override {
        MyFrame* frame = new MyFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);

//g++ -g -o stlviewer stl_viewer.cpp `wx-config --cxxflags --libs` -lGL -lGLEW -lassimp


