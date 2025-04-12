#! /usr/bin/python

import sys
import cv2
from PySide6.QtCore import Qt, QTimer
from PySide6.QtGui import QImage, QPixmap
from PySide6.QtWidgets import QApplication, QLabel, QVBoxLayout, QWidget

class CameraWidget(QWidget):
    def __init__(self, camera_index=0):
        super().__init__()
        self.camera_index = camera_index
        self.capture = cv2.VideoCapture(self.camera_index)

        if not self.capture.isOpened():
            raise ValueError(f"Could not open camera {self.camera_index}")

        self.label = QLabel()
        self.label.setAlignment(Qt.AlignCenter)

        self.layout = QVBoxLayout(self)
        self.layout.addWidget(self.label)

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_frame)
        self.timer.start(30)  # Update every 30 milliseconds (adjust as needed)

    def update_frame(self):
        ret, frame = self.capture.read()
        if ret:
            # Convert the OpenCV frame (BGR) to RGB
            rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            h, w, ch = rgb_frame.shape
            bytes_per_line = ch * w
            qt_image = QImage(rgb_frame.data, w, h, bytes_per_line, QImage.Format_RGB888)
            pixmap = QPixmap.fromImage(qt_image)
            self.label.setPixmap(pixmap)

    def closeEvent(self, event):
        self.capture.release()
        event.accept()

if __name__ == '__main__':
    app = QApplication(sys.argv)

    # You can try different camera indices (0, 1, 2, ...)
    # to find your USB camera.
    camera_widget = CameraWidget(camera_index=0)
    camera_widget.setWindowTitle("USB Camera Feed")
    camera_widget.setGeometry(100, 100, 640, 480)  # Adjust window size as needed
    camera_widget.show()

    sys.exit(app.exec())
