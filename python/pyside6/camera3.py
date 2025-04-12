#! /usr/bin/python

import sys
import cv2
from PySide6.QtCore import Qt, QTimer, QDateTime
from PySide6.QtGui import QImage, QPixmap
from PySide6.QtWidgets import QApplication, QLabel, QVBoxLayout, QWidget, QPushButton, QHBoxLayout
import datetime
import os

class CameraWidget(QWidget):
    def __init__(self, camera_index=0):
        super().__init__()
        self.camera_index = camera_index
        self.capture = cv2.VideoCapture(self.camera_index)

        if not self.capture.isOpened():
            raise ValueError(f"Could not open camera {self.camera_index}")

        self.is_recording = False
        self.video_writer = None

        self.image_label = QLabel()
        self.image_label.setAlignment(Qt.AlignCenter)

        self.record_button = QPushButton("녹화 시작")
        self.record_button.clicked.connect(self.toggle_recording)

        button_layout = QHBoxLayout()
        button_layout.addWidget(self.record_button)

        self.layout = QVBoxLayout(self)
        self.layout.addWidget(self.image_label)
        self.layout.addLayout(button_layout)

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_frame)
        self.timer.start(30)

    def update_frame(self):
        ret, frame = self.capture.read()
        if ret:
            rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            h, w, ch = rgb_frame.shape
            bytes_per_line = ch * w
            qt_image = QImage(rgb_frame.data, w, h, bytes_per_line, QImage.Format_RGB888)
            pixmap = QPixmap.fromImage(qt_image)
            self.image_label.setPixmap(pixmap)

            if self.is_recording and self.video_writer is not None:
                self.video_writer.write(frame)

    def toggle_recording(self):
        if not self.is_recording:
            now = datetime.datetime.now()
            timestamp = now.strftime("%Y%m%d_%H%M%S")
            filename = f"recording_{timestamp}.avi"
            filepath = os.path.join(os.getcwd(), filename)

            fourcc = cv2.VideoWriter_fourcc(*'XVID')  # 코덱 설정 (다른 코덱도 가능)
            fps = 30.0
            frame_width = int(self.capture.get(cv2.CAP_PROP_FRAME_WIDTH))
            frame_height = int(self.capture.get(cv2.CAP_PROP_FRAME_HEIGHT))
            self.video_writer = cv2.VideoWriter(filepath, fourcc, fps, (frame_width, frame_height))

            if self.video_writer.isOpened():
                self.is_recording = True
                self.record_button.setText("녹화 중지")
                print(f"녹화 시작: {filepath}")
            else:
                print("비디오 라이터 초기화 실패!")
        else:
            self.is_recording = False
            self.record_button.setText("녹화 시작")
            if self.video_writer is not None:
                self.video_writer.release()
                self.video_writer = None
                print("녹화 중지")

    def closeEvent(self, event):
        self.capture.release()
        if self.video_writer is not None:
            self.video_writer.release()
        event.accept()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    camera_widget = CameraWidget(camera_index=0)
    camera_widget.setWindowTitle("USB Camera with Recording")
    camera_widget.setGeometry(100, 100, 640, 520)
    camera_widget.show()
    sys.exit(app.exec())
