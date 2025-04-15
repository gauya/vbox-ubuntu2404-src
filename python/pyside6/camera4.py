#! /usr/bin/python

import sys
import cv2
from PySide6.QtCore import Qt, QTimer, QDateTime
from PySide6.QtGui import QImage, QPixmap
from PySide6.QtWidgets import QApplication, QLabel, QVBoxLayout, QWidget, QPushButton, QHBoxLayout, QLineEdit
import datetime
import os

class CameraWidget(QWidget):
    def __init__(self, camera_index=0):
        super().__init__()
        self.camera_index = camera_index
        self.capture = cv2.VideoCapture(self.camera_index)

        if not self.capture.isOpened():
            raise ValueError(f"Could not open camera {self.camera_index}")

        #w = capture.get(cv2.CAP_PROP_FRAME_WIDTH)
        #h = capture.get(cv2.CAP_PROP_FRAME_HEIGHT)

        #capture.set(cv2.CAP_PROP_FRAME_WIDTH, desired_width)
        #capture.set(cv2.CAP_PROP_FRAME_HEIGHT, desired_height)
        
        self.current_frame = None
        self.is_recording = False
        self.video_writer = None
        self.serial_number = 0

        self.image_label = QLabel()
        self.image_label.setAlignment(Qt.AlignCenter)

        self.capture_button = QPushButton("스틸샷 찍기")
        self.capture_button.clicked.connect(self.capture_still_image)

        self.record_button = QPushButton("녹화 시작")
        self.record_button.clicked.connect(self.toggle_recording)

        self.serial_input = QLineEdit()
        self.serial_input.setPlaceholderText("시작 일련번호 (선택)")
        self.serial_input.textChanged.connect(self.update_serial_number)

        button_layout = QHBoxLayout()
        button_layout.addWidget(self.capture_button)
        button_layout.addWidget(self.record_button)
        button_layout.addWidget(self.serial_input)

        self.layout = QVBoxLayout(self)
        self.layout.addWidget(self.image_label)
        self.layout.addLayout(button_layout)

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_frame)
        self.timer.start(30)

    def update_frame(self):
        ret, frame = self.capture.read()
        if ret:
            self.current_frame = frame
            rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            h, w, ch = rgb_frame.shape
            bytes_per_line = ch * w
            qt_image = QImage(rgb_frame.data, w, h, bytes_per_line, QImage.Format_RGB888)
            pixmap = QPixmap.fromImage(qt_image)
            self.image_label.setPixmap(pixmap)

            if self.is_recording and self.video_writer is not None:
                self.video_writer.write(frame)

    def update_serial_number(self, text):
        try:
            self.serial_number = int(text)
        except ValueError:
            self.serial_number = 0

    def capture_still_image(self):
        if self.current_frame is not None:
            now = datetime.datetime.now()
            cur_date = now.strftime("%Y%m%d_%H%M%S")
            serialno_str = f"{self.serial_number:04d}"
            filename = f"still_image-{cur_date}_{serialno_str}.png"
            filepath = os.path.join(os.getcwd(), filename)

            cv2.imwrite(filepath, self.current_frame)
            print(f"스틸샷 저장됨: {filepath}")
            self.serial_number += 1
            self.serial_input.setText(str(self.serial_number))

    def toggle_recording(self):
        if not self.is_recording:
            now = datetime.datetime.now()
            timestamp = now.strftime("%Y%m%d_%H%M%S")
            filename = f"recording_{timestamp}.avi"
            filepath = os.path.join(os.getcwd(), filename)

            fourcc = cv2.VideoWriter_fourcc(*'XVID')
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
    camera_widget.setWindowTitle("USB Camera with Capture & Recording")
    camera_widget.setGeometry(100, 100, 700, 520)  # 버튼과 입력창을 위해 약간 더 넓게
    #camera_widget.setGeometry(100, 100, 1024, 768)  # 버튼과 입력창을 위해 약간 더 넓게
    camera_widget.show()
    sys.exit(app.exec())
