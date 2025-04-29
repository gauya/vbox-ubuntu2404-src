import sys
from PySide6.QtWidgets import QApplication, QMainWindow
from PySide6.QtCore import Qt
from PySide6.QtGui import QIcon

def selmon( n ):
    screen = QApplication.primaryScreen()  # 또는 screens()[n]으로 특정 모니터 선택
    geometry = screen.availableGeometry()
    self.setGeometry(geometry)

class FullScreenApp(QMainWindow):
    def __init__(self):
        super().__init__()
        
        # 윈도우 프레임 제거
        self.setWindowFlags(
            Qt.FramelessWindowHint |  # 프레임 제거
            Qt.WindowStaysOnTopHint   # 항상 위에 유지 (선택 사항)
        )
        
        # 전체 화면 설정
        self.showFullScreen()
        
        # ESC 키로 종료 기능 추가 (선택 사항)
        self.setFocusPolicy(Qt.StrongFocus)
    
    def setwt(self):
        icon = QIcon.fromTheme("applications-development")
        self.setWindowIcon(icon)

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Escape:
            self.close()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    mainWin = FullScreenApp()
    mainWin.setwt()
    sys.exit(app.exec())

    
