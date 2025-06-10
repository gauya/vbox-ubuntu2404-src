#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Light_Button.H>

// 펌프 상태 콜백
void start_button_cb(Fl_Widget* widget, void* data) {
    Fl_Box* status = (Fl_Box*)data;
    status->label("Pump ON");
    status->labelcolor(FL_BLACK);
    status->color(FL_GREEN); // 펌프 ON: 초록색
    status->redraw();
}

void stop_button_cb(Fl_Widget* widget, void* data) {
    Fl_Box* status = (Fl_Box*)data;
    status->label("Pump OFF");
    status->labelcolor(FL_WHITE);
    status->color(FL_RED); // 펌프 OFF: 빨강
    status->redraw();
}

// 슬라이더 콜백
void slider_cb(Fl_Widget* widget, void* data) {
    Fl_Slider* slider = (Fl_Slider*)widget;
    Fl_Box* flow = (Fl_Box*)data;
    char buf[32];
    snprintf(buf, sizeof(buf), "Flow: %.1f L/min", slider->value());
    flow->copy_label(buf);
}

int main(int argc, char **argv) {
    // 창 생성 (500x400, 밝은 회색 배경)
    Fl_Window *window = new Fl_Window(500, 400, "Pump Control Panel");
    window->color(FL_LIGHT2); // 밝은 회색 배경

    // 상태 표시 (상단 중앙)
    Fl_Box *status = new Fl_Box(100, 30, 300, 80, "Pump OFF");
    status->box(FL_ROUND_UP_BOX); // 둥근 테두리
    status->color(FL_RED); // 초기: 빨강
    status->labelcolor(FL_WHITE);
    status->labelsize(24);
    status->labelfont(FL_BOLD);

    // 시작 버튼 (둥근 버튼, 청록색)
    Fl_Round_Button *start_button = new Fl_Round_Button(100, 150, 120, 50, "Start");
    start_button->color(FL_CYAN); // 청록색 배경
    start_button->labelcolor(FL_BLACK);
    start_button->labelsize(18);
    start_button->callback(start_button_cb, status);

    // 정지 버튼 (둥근 버튼, 회색)
    Fl_Round_Button *stop_button = new Fl_Round_Button(280, 150, 120, 50, "Stop");
    stop_button->color(FL_DARK3); // 어두운 회색
    stop_button->labelcolor(FL_WHITE);
    stop_button->labelsize(18);
    stop_button->callback(stop_button_cb, status);

    // 유량 조절 슬라이더
    Fl_Slider *slider = new Fl_Slider(100, 230, 300, 30, "Flow Rate");
    slider->type(FL_HOR_NICE_SLIDER);
    slider->range(0.0, 10.0); // 0~10 L/min
    slider->value(5.0);
    slider->color(FL_LIGHT1);
    slider->selection_color(FL_BLUE); // 슬라이더 바 색상
    slider->labelsize(14);

    // 유량 표시
    Fl_Box *flow_display = new Fl_Box(100, 280, 300, 40, "Flow: 5.0 L/min");
    flow_display->box(FL_FLAT_BOX);
    flow_display->color(FL_LIGHT2);
    flow_display->labelsize(16);
    slider->callback(slider_cb, flow_display);

    // 경고등 (간단한 상태 표시)
    Fl_Light_Button *warning = new Fl_Light_Button(200, 340, 100, 30, "Warning");
    warning->color(FL_YELLOW); // 노란색 경고등
    warning->selection_color(FL_RED); // 경고 ON: 빨강
    warning->labelsize(14);

    // 창 표시 및 이벤트 루프
    window->end();
    window->show(argc, argv);
    return Fl::run();
}
