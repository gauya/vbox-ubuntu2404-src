#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>

// STL 파일의 삼각형 구조체
struct Triangle {
    float vertices[3][3]; // 각 정점의 x, y, z 좌표
    float normal[3];     // 법선 벡터 (선택 사항)
};

class STLViewerCanvas : public wxGLCanvas {
public:
    STLViewerCanvas(wxFrame* parent, const wxString& filename);
    ~STLViewerCanvas();

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouse(wxMouseEvent& event);
    void OnInit(wxInitDialogEvent& event);

private:
    void LoadSTL(const wxString& filename);
    void Render();
    void InitializeGL();
    void SetViewport();

    std::vector<Triangle> m_triangles;
    float m_rotateX;
    float m_rotateY;
    float m_scale;
    wxPoint m_lastMousePos;
    bool m_isDragging;
    GLuint m_vertexArrayID;
    GLuint m_vertexBufferID;
};

class STLViewerFrame : public wxFrame {
public:
    STLViewerFrame(const wxString& title, const wxString& filename);
    ~STLViewerFrame();

private:
    STLViewerCanvas* m_canvas;
};

class STLViewerApp : public wxApp {
public:
    virtual bool OnInit();
private:
    wxString m_stlFilename;
};

wxIMPLEMENT_APP_NO_MAIN(STLViewerApp);

bool STLViewerApp::OnInit()
{
    if (argc != 2) {
        wxLogError("Usage: stlviewer <stl_file>");
        return false;
    }
    m_stlFilename = argv[1];

    STLViewerFrame* frame = new STLViewerFrame("STL Viewer", m_stlFilename);
    frame->Show(true);
    return true;
}

STLViewerFrame::STLViewerFrame(const wxString& title, const wxString& filename)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600))
{
    m_canvas = new STLViewerCanvas(this, filename);
}

STLViewerFrame::~STLViewerFrame()
{
}

STLViewerCanvas::STLViewerCanvas(wxFrame* parent, const wxString& filename)
    : wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxT("STLCanvas"))
    , m_rotateX(0.0f)
    , m_rotateY(0.0f)
    , m_scale(1.0f)
    , m_isDragging(false)
{
    Connect(wxEVT_PAINT, wxPaintEventHandler(STLViewerCanvas::OnPaint), nullptr, this);
    Connect(wxEVT_SIZE, wxSizeEventHandler(STLViewerCanvas::OnSize), nullptr, this);
    Connect(wxEVT_MOTION, wxMouseEventHandler(STLViewerCanvas::OnMouse), nullptr, this);
    Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(STLViewerCanvas::OnMouse), nullptr, this);
    Connect(wxEVT_LEFT_UP, wxMouseEventHandler(STLViewerCanvas::OnMouse), nullptr, this);
    Connect(wxEVT_INIT_DIALOG, wxInitDialogEventHandler(STLViewerCanvas::OnInit), nullptr, this);

    LoadSTL(filename);
}

STLViewerCanvas::~STLViewerCanvas()
{
    wxGLContext* glContext = new wxGLContext(this);
    SetCurrent(*glContext);
    glDeleteVertexArrays(1, &m_vertexArrayID);
    glDeleteBuffers(1, &m_vertexBufferID);
    delete glContext;
}

void STLViewerCanvas::OnInit(wxInitDialogEvent& event)
{
    wxGLContext* glContext = new wxGLContext(this);
    SetCurrent(*glContext);
    InitializeGL();
    delete glContext;
}

void STLViewerCanvas::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this); // 필수
    Render();
}

void STLViewerCanvas::OnSize(wxSizeEvent& event)
{
    wxGLContext* glContext = new wxGLContext(this);
    SetCurrent(*glContext);
    SetViewport();
    delete glContext;
}

void STLViewerCanvas::OnMouse(wxMouseEvent& event)
{
    if (event.LeftDown()) {
        m_isDragging = true;
        m_lastMousePos = event.GetPosition();
    } else if (event.LeftUp()) {
        m_isDragging = false;
    } else if (event.Dragging() && event.LeftIsDown()) {
        wxPoint currentPos = event.GetPosition();
        m_rotateY += (currentPos.x - m_lastMousePos.x) * 0.5f;
        m_rotateX += (currentPos.y - m_lastMousePos.y) * 0.5f;
        m_lastMousePos = currentPos;
        Refresh();
    } else if (event.GetWheelRotation() != 0) {
        m_scale += event.GetWheelRotation() > 0 ? 0.1f : -0.1f;
        if (m_scale < 0.1f) m_scale = 0.1f;
        Refresh();
    }
}

void STLViewerCanvas::LoadSTL(const wxString& filename)
{
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        wxLogError("Failed to open STL file: %s", filename);
        return;
    }

    char header[80];
    file.read(header, 80); // 헤더 (선택 사항)

    uint32_t numTriangles;
    file.read(reinterpret_cast<char*>(&numTriangles), sizeof(uint32_t));

    m_triangles.resize(numTriangles);
    for (uint32_t i = 0; i < numTriangles; ++i) {
        float normal[3];
        file.read(reinterpret_cast<char*>(normal), sizeof(float) * 3);
        m_triangles[i].normal[0] = normal[0];
        m_triangles[i].normal[1] = normal[1];
        m_triangles[i].normal[2] = normal[2];
        for (int j = 0; j < 3; ++j) {
            file.read(reinterpret_cast<char*>(m_triangles[i].vertices[j]), sizeof(float) * 3);
        }
        uint16_t attributeByteCount;
        file.read(reinterpret_cast<char*>(&attributeByteCount), sizeof(uint16_t)); // 속성 바이트 수 (일반적으로 0)
    }

    file.close();

    // OpenGL 버퍼 생성 및 데이터 로드 (OnInit에서 수행하는 것이 더 안전)
}

void STLViewerCanvas::InitializeGL()
{
    ::wxSetGLContext(GetHDC());
    glewInit(); // GLEW 초기화 (OpenGL 확장 기능 사용)
    if (GLEW_OK != glewInit()) {
        wxLogError("GLEW initialization failed!");
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    // 버텍스 데이터 준비
    std::vector<float> vertices;
    for (const auto& triangle : m_triangles) {
        for (int i = 0; i < 3; ++i) {
            vertices.push_back(triangle.vertices[i][0]);
            vertices.push_back(triangle.vertices[i][1]);
            vertices.push_back(triangle.vertices[i][2]);
        }
    }

    // VAO (Vertex Array Object) 생성 및 바인딩
    glGenVertexArrays(1, &m_vertexArrayID);
    glBindVertexArray(m_vertexArrayID);

    // VBO (Vertex Buffer Object) 생성 및 데이터 로드
    glGenBuffers(1, &m_vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 버텍스 속성 설정 (위치)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // VAO 바인딩 해제
}

void STLViewerCanvas::SetViewport()
{
    int width, height;
    GetClientSize(&width, &height);
    ::wxSetGLContext(GetHDC());
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)width / height, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f); // 카메라 위치
}

void STLViewerCanvas::Render()
{
    ::wxSetGLContext(GetHDC());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f * m_scale); // 확대/축소 적용
    glRotatef(m_rotateX, 1.0f, 0.0f, 0.0f);   // X축 회전
    glRotatef(m_rotateY, 0.0f, 1.0f, 0.0f);   // Y축 회전

    glBindVertexArray(m_vertexArrayID);
    glDrawArrays(GL_TRIANGLES, 0, m_triangles.size() * 3);
    glBindVertexArray(0);

    SwapBuffers();
}

int main(int argc, char* argv[])
{
    wxApp::SetInstance(new STLViewerApp());
    return wxEntry(argc, argv);
}

// g++ -o stlviewer stlviewer.cpp `wx-config --cxxflags --libs` -lGL -lGLEW
