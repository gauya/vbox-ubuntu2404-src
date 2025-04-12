import sys
from PySide6.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout, QToolBar, QPushButton, QColorDialog, QSpinBox, QLabel
from PySide6.QtGui import QPainter, QColor, QPen, QImage
from PySide6.QtCore import Qt, QPoint, QSize

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

    def resizeEvent(self, event):
        if event.size() != self.image.size():
            new_image = QImage(event.size(), QImage.Format_RGB32)
            new_image.fill(Qt.white)
            painter = QPainter(new_image)
            painter.drawImage(QPoint(0, 0), self.image)
            self.image = new_image
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

        color_button = QPushButton("색상 선택")
        color_button.clicked.connect(self.choose_color)
        self.toolbar.addWidget(color_button)

        width_label = QLabel("두께:")
        self.toolbar.addWidget(width_label)

        width_spinbox = QSpinBox()
        width_spinbox.setRange(1, 20)
        width_spinbox.setValue(2)
        width_spinbox.valueChanged.connect(self.change_pen_width)
        self.toolbar.addWidget(width_spinbox)

    def choose_color(self):
        color = QColorDialog.getColor(self.paint_area.current_pen.color(), self)
        if color.isValid():
            self.paint_area.set_pen_color(color)

    def change_pen_width(self, width):
        self.paint_area.set_pen_width(width)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
