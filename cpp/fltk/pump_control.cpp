#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>

// 버튼 콜백 함수
void button_cb(Fl_Widget* widget, void* data) {
    Fl_Box* box = (Fl_Box*)data;
    box->label("Pump ON");
}

int main(int argc, char **argv) {
    // 창 생성 (400x300 크기)
    Fl_Window *window = new Fl_Window(400, 300, "Pump Control");

    // 상태 표시 텍스트
    Fl_Box *box = new Fl_Box(100, 50, 200, 50, "Pump OFF");
    box->box(FL_UP_BOX);
    box->labelsize(20);

    // 시작 버튼
    Fl_Button *button = new Fl_Button(150, 150, 100, 40, "Start Pump");
    button->callback(button_cb, box);

    // 창 표시 및 이벤트 루프 시작
    window->end();
    window->show(argc, argv);
    return Fl::run();
}
