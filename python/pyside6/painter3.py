import sys
from PySide6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                              QHBoxLayout, QPushButton, QColorDialog, QFileDialog,
                              QComboBox, QSpinBox, QLabel)
from PySide6.QtGui import QPainter, QPen, QPainterPath, QImage, QPixmap, QColor
from PySide6.QtCore import Qt, QPoint


class DrawingArea(QWidget):
    def __init__(self):
        super().__init__()
        self.setAttribute(Qt.WA_StaticContents)
        self.pen_color = Qt.black
        self.pen_width = 3
        self.path = QPainterPath()
        self.background_image = None
        self.is_eraser = False
        self.last_point = QPoint()
        
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.last_point = event.position().toPoint()
            self.path.moveTo(self.last_point)
            
    def mouseMoveEvent(self, event):
        if event.buttons() & Qt.LeftButton:
            new_point = event.position().toPoint()
            
            if self.is_eraser:
                # 지우개 모드
                painter = QPainter(self)
                painter.setPen(QPen(Qt.white, self.pen_width, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin))
                painter.drawLine(self.last_point, new_point)
                self.last_point = new_point
                painter.end()
            else:
                # 브러시 모드
                self.path.lineTo(new_point)
            
            self.update()
            
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        
        # 배경 이미지 그리기
        if self.background_image:
            painter.drawImage(self.rect(), self.background_image)
        
        # 사용자 그림 그리기 (지우개 모드가 아닐 때만)
        if not self.is_eraser:
            pen = QPen(self.pen_color, self.pen_width)
            pen.setCapStyle(Qt.RoundCap)
            pen.setJoinStyle(Qt.RoundJoin)
            painter.setPen(pen)
            painter.drawPath(self.path)
        
    def clear(self):
        self.path = QPainterPath()
        self.background_image = None
        self.update()
        
    def change_color(self, color):
        self.pen_color = color
        
    def change_width(self, width):
        self.pen_width = width
        
    def set_eraser(self, is_eraser):
        self.is_eraser = is_eraser
        
    def load_image(self, file_path):
        image = QImage(file_path)
        if not image.isNull():
            # 위젯 크기에 맞게 이미지 조정
            self.background_image = image.scaled(
                self.size(), 
                Qt.KeepAspectRatioByExpanding, 
                Qt.SmoothTransformation
            )
            self.update()
            
    def save_image(self, file_path, file_format):
        # 그림판 내용을 이미지로 저장
        image = QImage(self.size(), QImage.Format_ARGB32)
        image.fill(Qt.white)
        
        painter = QPainter(image)
        painter.setRenderHint(QPainter.Antialiasing)
        
        # 배경 이미지 그리기
        if self.background_image:
            painter.drawImage(self.rect(), self.background_image)
        
        # 사용자 그림 그리기
        pen = QPen(self.pen_color, self.pen_width)
        pen.setCapStyle(Qt.RoundCap)
        pen.setJoinStyle(Qt.RoundJoin)
        painter.setPen(pen)
        painter.drawPath(self.path)
        painter.end()
        
        image.save(file_path, file_format)


class AdvancedPaintApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("고급 그림판")
        self.setGeometry(100, 100, 1000, 800)
        
        # 중앙 위젯과 레이아웃 설정
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)
        
        # 그림 영역 생성
        self.drawing_area = DrawingArea()
        self.drawing_area.setStyleSheet("background-color: white;")
        layout.addWidget(self.drawing_area)
        
        # 상단 버튼 패널
        top_panel = QHBoxLayout()
        
        # 브러시/지우개 토글 버튼
        self.tool_button = QPushButton("브러시")
        self.tool_button.setCheckable(True)
        self.tool_button.clicked.connect(self.toggle_tool)
        top_panel.addWidget(self.tool_button)
        
        # 색상 선택
        color_btn = QPushButton("색상 선택")
        color_btn.clicked.connect(self.choose_color)
        top_panel.addWidget(color_btn)
        
        # 굵기 선택
        top_panel.addWidget(QLabel("굵기:"))
        self.width_spin = QSpinBox()
        self.width_spin.setRange(1, 50)
        self.width_spin.setValue(3)
        self.width_spin.valueChanged.connect(self.change_width)
        top_panel.addWidget(self.width_spin)
        
        layout.addLayout(top_panel)
        
        # 하단 버튼 패널
        bottom_panel = QHBoxLayout()
        
        # 이미지 불러오기
        load_btn = QPushButton("이미지 불러오기")
        load_btn.clicked.connect(self.load_image)
        bottom_panel.addWidget(load_btn)
        
        # 지우기
        clear_btn = QPushButton("지우기")
        clear_btn.clicked.connect(self.drawing_area.clear)
        bottom_panel.addWidget(clear_btn)
        
        # 저장 형식 선택
        bottom_panel.addWidget(QLabel("저장 형식:"))
        self.format_combo = QComboBox()
        self.format_combo.addItems(["PNG", "JPEG", "BMP"])
        bottom_panel.addWidget(self.format_combo)
        
        # 저장 버튼
        save_btn = QPushButton("이미지 저장")
        save_btn.clicked.connect(self.save_image)
        bottom_panel.addWidget(save_btn)
        
        layout.addLayout(bottom_panel)
        
    def toggle_tool(self):
        if self.tool_button.isChecked():
            self.tool_button.setText("지우개")
            self.drawing_area.set_eraser(True)
        else:
            self.tool_button.setText("브러시")
            self.drawing_area.set_eraser(False)
            
    def choose_color(self):
        color = QColorDialog.getColor()
        if color.isValid():
            self.drawing_area.change_color(color)
            
    def change_width(self, width):
        self.drawing_area.change_width(width)
            
    def load_image(self):
        file_dialog = QFileDialog(self)
        file_dialog.setNameFilter("이미지 파일 (*.png *.jpg *.jpeg *.bmp)")
        file_dialog.setViewMode(QFileDialog.Detail)
        
        if file_dialog.exec():
            file_path = file_dialog.selectedFiles()[0]
            self.drawing_area.load_image(file_path)
            
    def save_image(self):
        file_dialog = QFileDialog(self)
        file_dialog.setAcceptMode(QFileDialog.AcceptSave)
        
        # 선택한 형식에 따라 기본 확장자 설정
        format_map = {
            "PNG": "png",
            "JPEG": "jpg",
            "BMP": "bmp"
        }
        selected_format = self.format_combo.currentText()
        file_dialog.setDefaultSuffix(format_map[selected_format])
        file_dialog.setNameFilter(f"{selected_format} 파일 (*.{format_map[selected_format]})")
        
        if file_dialog.exec():
            file_path = file_dialog.selectedFiles()[0]
            self.drawing_area.save_image(file_path, selected_format)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = AdvancedPaintApp()
    window.show()
    sys.exit(app.exec())
