import sys
from PySide6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                              QHBoxLayout, QPushButton, QColorDialog, QFileDialog)
from PySide6.QtGui import QPainter, QPen, QPainterPath, QImage, QPixmap
from PySide6.QtCore import Qt, QPoint


class DrawingArea(QWidget):
    def __init__(self):
        super().__init__()
        self.setAttribute(Qt.WA_StaticContents)
        self.pen_color = Qt.black
        self.pen_width = 3
        self.path = QPainterPath()
        self.background_image = None
        
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.path.moveTo(event.position())
            
    def mouseMoveEvent(self, event):
        if event.buttons() & Qt.LeftButton:
            self.path.lineTo(event.position())
            self.update()
            
    def paintEvent(self, event):
        painter = QPainter(self)
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
        
    def clear(self):
        self.path = QPainterPath()
        self.background_image = None
        self.update()
        
    def change_color(self, color):
        self.pen_color = color
        
    def change_width(self, width):
        self.pen_width = width
        
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


class SimplePaintApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("간단한 그림판 (이미지 불러오기 기능 추가)")
        self.setGeometry(100, 100, 800, 600)
        
        # 중앙 위젯과 레이아웃 설정
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)
        
        # 그림 영역 생성
        self.drawing_area = DrawingArea()
        self.drawing_area.setStyleSheet("background-color: white;")
        layout.addWidget(self.drawing_area)
        
        # 버튼 패널 생성
        button_panel = QHBoxLayout()
        
        clear_btn = QPushButton("지우기")
        clear_btn.clicked.connect(self.drawing_area.clear)
        button_panel.addWidget(clear_btn)
        
        color_btn = QPushButton("색상 선택")
        color_btn.clicked.connect(self.choose_color)
        button_panel.addWidget(color_btn)
        
        load_btn = QPushButton("이미지 불러오기")
        load_btn.clicked.connect(self.load_image)
        button_panel.addWidget(load_btn)
        
        layout.addLayout(button_panel)
        
    def choose_color(self):
        color = QColorDialog.getColor()
        if color.isValid():
            self.drawing_area.change_color(color)
            
    def load_image(self):
        file_dialog = QFileDialog(self)
        file_dialog.setNameFilter("이미지 파일 (*.png *.jpg *.jpeg *.bmp)")
        file_dialog.setViewMode(QFileDialog.Detail)
        
        if file_dialog.exec():
            file_path = file_dialog.selectedFiles()[0]
            self.drawing_area.load_image(file_path)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = SimplePaintApp()
    window.show()
    sys.exit(app.exec())
