import sys
from PySide6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                               QToolBar, QPushButton, QColorDialog, QSpinBox,
                               QLabel, QFileDialog, QComboBox)
from PySide6.QtGui import QPainter, QColor, QPen, QImage, QAction
from PySide6.QtCore import Qt, QPoint, QSize
import os

class PaintArea(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.image = QImage(self.size(), QImage.Format_RGB32)
        self.image.fill(Qt.white)
        self.drawing = False
        self.last_point = QPoint()
        self.current_pen = QPen(Qt.black, 2, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin)

    def set_pen_color(self, color):
        self.current_pen.setColor(color)

    def set_pen_width(self, width):
        self.current_pen.setWidth(width)

    def set_pen_style(self, style_str):
        if style_str == "Solid":
            self.current_pen.setStyle(Qt.SolidLine)
        elif style_str == "Dashed":
            self.current_pen.setStyle(Qt.DashLine)
        elif style_str == "Dotted":
            self.current_pen.setStyle(Qt.DotLine)
        elif style_str == "DashDot":
            self.current_pen.setStyle(Qt.DashDotLine)
        elif style_str == "DashDotDot":
            self.current_pen.setStyle(Qt.DashDotDotLine)

    def loadImage(self, path):
        loaded_image = QImage(path)
        if not loaded_image.isNull():
            self.image = loaded_image.scaled(self.size(), Qt.KeepAspectRatio, Qt.SmoothTransformation)
            self.update()

    def saveImage(self, path):
        self.image.save(path)

    def resizeEvent(self, event):
        if event.size() != self.image.size() and not self.image.isNull():
            new_image = QImage(event.size(), QImage.Format_RGB32)
            new_image.fill(Qt.white)
            painter = QPainter(new_image)
            painter.drawImage(QPoint(0, 0), self.image)
            self.image = new_image
        elif self.image.isNull():
            self.image = QImage(event.size(), QImage.Format_RGB32)
            self.image.fill(Qt.white)
        super().resizeEvent(event)

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.drawing = True
            self.last_point = event.pos()

    def mouseMoveEvent(self, event):
        if (event.buttons() & Qt.LeftButton) and self.drawing:
            painter = QPainter(self.image)
            painter.setPen(self.current_pen)
            painter.drawLine(self.last_point, event.pos())
            self.last_point = event.pos()
            self.update()

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.LeftButton and self.drawing:
            self.drawing = False

    def paintEvent(self, event):
        canvas_painter = QPainter(self)
        canvas_painter.drawImage(self.rect(), self.image, self.rect())

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Simple Paint")
        self.setGeometry(100, 100, 800, 600)

        self.paint_area = PaintArea()
        self.setCentralWidget(self.paint_area)

        self.toolbar = QToolBar("Tools")
        self.addToolBar(self.toolbar)

        # 파일 메뉴
        file_menu = self.menuBar().addMenu("파일")
        load_action = QAction("불러오기...", self)
        load_action.triggered.connect(self.load_image)
        file_menu.addAction(load_action)
        save_action = QAction("저장...", self)
        save_action.triggered.connect(self.save_image)
        file_menu.addAction(save_action)

        # 색상 선택 버튼
        color_button = QPushButton("색상 선택")
        color_button.clicked.connect(self.choose_color)
        self.toolbar.addWidget(color_button)

        # 펜 두께 조절
        width_label = QLabel("두께:")
        self.toolbar.addWidget(width_label)
        width_spinbox = QSpinBox()
        width_spinbox.setRange(1, 20)
        width_spinbox.setValue(self.paint_area.current_pen.width())
        width_spinbox.valueChanged.connect(self.change_pen_width)
        self.toolbar.addWidget(width_spinbox)

        # 펜 모양 선택 콤보 박스
        style_label = QLabel("모양:")
        self.toolbar.addWidget(style_label)
        style_combo = QComboBox()
        style_combo.addItems(["Solid", "Dashed", "Dotted", "DashDot", "DashDotDot"])
        style_combo.setCurrentText("Solid")
        style_combo.currentIndexChanged.connect(self.change_pen_style)
        self.toolbar.addWidget(style_combo)

    def load_image(self):
        file_dialog = QFileDialog(self, "이미지 불러오기")
        file_dialog.setNameFilter("Images (*.png *.jpg *.jpeg *.bmp *.gif)")
        if file_dialog.exec():
            file_path = file_dialog.selectedFiles()[0]
            self.paint_area.loadImage(file_path)

    def save_image(self):
        file_dialog = QFileDialog(self, "이미지 저장", ".", "Images (*.png)")
        file_dialog.setDefaultSuffix("png")
        if file_dialog.exec():
            file_path = file_dialog.selectedFiles()[0]
            self.paint_area.saveImage(file_path)

    def choose_color(self):
        color = QColorDialog.getColor(self.paint_area.current_pen.color(), self)
        if color.isValid():
            self.paint_area.set_pen_color(color)

    def change_pen_width(self, width):
        self.paint_area.set_pen_width(width)

    def change_pen_style(self, index):
        style_str = self.sender().currentText()
        self.paint_area.set_pen_style(style_str)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
