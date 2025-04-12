from PySide6.QtWidgets import QApplication, QMainWindow, QAction

app = QApplication([])
window = QMainWindow()
action = QAction("Test", window)
window.menuBar().addAction(action)
window.show()
app.exec()
