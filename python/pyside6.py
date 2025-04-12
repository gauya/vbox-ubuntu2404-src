from PySide6.QtWidgets import QApplication, QPushButton

app = QApplication([])
button = QPushButton("Click (PySide6)")
button.clicked.connect(lambda: print("PySide6 clicked!"))
button.show()
app.exec()
