import sys
from PySide6.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget, QComboBox
from PySide6.QtGui import QPixmap, QPainter, QBrush, QPalette
from PySide6.QtCore import Qt, QSize
from PySide6.QtWidgets import QFileDialog, QPushButton

class ImageDisplayApp(QMainWindow):
    def __init__(self, image=None):
        self.image = None
        super().__init__()
        self.setWindowTitle("이미지 배경 처리 도구")
        self.setGeometry(100, 100, 800, 600)
        
        # 중앙 위젯 설정
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # 레이아웃 설정
        layout = QVBoxLayout()
        central_widget.setLayout(layout)
        
        # 콤보박스로 표시 모드 선택
        self.mode_combo = QComboBox()
        self.mode_combo.addItems([
            "1. 배경 해상도에 맞게 꽉 채움 (비율 무시)",
            "2. 사진 사이즈대로 (크면 잘림)",
            "3. 비율 유지하며 가로/세로 꽉 채움",
            "4. 바둑판 모양으로 반복"
        ])
        self.mode_combo.currentIndexChanged.connect(self.update_display)
        layout.addWidget(self.mode_combo)
        
        # 이미지 표시 라벨
        self.image_label = QLabel()
        self.image_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.image_label)
        
        self.fullscreen_btn = QPushButton("전체 화면")
        self.fullscreen_btn.clicked.connect(self.toggle_fullscreen)
        layout.addWidget(self.fullscreen_btn)

        self.select_btn = QPushButton("이미지 선택")
        self.select_btn.clicked.connect(self.select_image)
        layout.addWidget(self.select_btn)

        # 테스트 이미지 로드 (실제 사용시 파일 선택기 구현 필요)
        if image:
            self.image = image
        if self.image:
            self.load_image(self.image)

    def select_image(self):
        file_path, _ = QFileDialog.getOpenFileName(
            self, "이미지 선택", "", 
            "이미지 파일 (*.png *.jpg *.jpeg *.bmp *.gif)"
        )
        if file_path:
            self.load_image(file_path)


    def load_image(self, path):
        if not self.image or self.image != path:
            self.image = path
        self.original_pixmap = QPixmap(self.image)
        self.update_display()
    

    def toggle_fullscreen(self):
        if self.isFullScreen():
            self.showNormal()
        else:
            self.showFullScreen()
        
    def update_display(self):
        if not hasattr(self, 'original_pixmap'):
            return
            
        mode = self.mode_combo.currentIndex()
        pixmap = self.original_pixmap.copy()
        label_size = self.image_label.size()
        
        if mode == 0:  # 1. 배경 해상도에 맞게 꽉 채움 (비율 무시)
            scaled_pixmap = pixmap.scaled(
                label_size, 
                Qt.IgnoreAspectRatio, 
                Qt.SmoothTransformation
            )
            self.image_label.setPixmap(scaled_pixmap)
            
        elif mode == 1:  # 2. 사진 사이즈대로 (크면 잘림)
            self.image_label.setPixmap(pixmap)
            self.image_label.setScaledContents(False)
            
        elif mode == 2:  # 3. 비율 유지하며 가로/세로 꽉 채움
            if (self.original_pixmap.width() > label_size.width() or
    self.original_pixmap.height() > label_size.height()):
                scaled_pixmap = pixmap.scaled(
                    label_size, 
                    Qt.KeepAspectRatioByExpanding, 
                    Qt.SmoothTransformation
                )
                self.image_label.setPixmap(scaled_pixmap)
            else:
                self.image_label.setPixmap(self.original_pixmap)

        elif mode == 3:  # 4. 바둑판 모양으로 반복
            # 바둑판 배경을 위한 위젯 스타일 설정
            self.image_label.setStyleSheet(f"""
                QLabel {{
                    background-image: url({self.original_pixmap.cacheKey()});
                    background-repeat: repeat-xy;
                    background-position: center;
                }}
            """)
            self.image_label.setPixmap(QPixmap())  # 기본 pixmap은 비움

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ImageDisplayApp()
    window.load_image(sys.argv[1])
    window.show()
    sys.exit(app.exec())
