#! /usr/bin/python

import sys
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QTextEdit, QFileDialog,
    QFontComboBox, QComboBox, QPushButton, QToolBar, QStatusBar
)
from PySide6.QtGui import QFont, QTextCursor
from PySide6.QtCore import QFile, QTimer

class Notepad(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Simple Notepad")
        self.resize(900, 700)

        self.editor = QTextEdit()
        self.setCentralWidget(self.editor)
        self.file_path = None

        self.init_toolbar()
        self.init_statusbar()
        self.init_menu()  # fallback using QPushButton

        self.editor.cursorPositionChanged.connect(self.update_statusbar)

    def init_toolbar(self):
        toolbar = QToolBar()
        self.addToolBar(toolbar)

        self.font_box = QFontComboBox()
        self.font_box.currentFontChanged.connect(self.change_font)
        toolbar.addWidget(self.font_box)

        self.size_box = QComboBox()
        self.size_box.addItems([str(i) for i in range(8, 30, 2)])
        self.size_box.currentTextChanged.connect(self.change_font_size)
        toolbar.addWidget(self.size_box)

        bold_btn = QPushButton("Bold")
        bold_btn.setCheckable(True)
        bold_btn.clicked.connect(self.toggle_bold)
        toolbar.addWidget(bold_btn)

        underline_btn = QPushButton("Underline")
        underline_btn.setCheckable(True)
        underline_btn.clicked.connect(self.toggle_underline)
        toolbar.addWidget(underline_btn)

    def init_statusbar(self):
        self.status = QStatusBar()
        self.setStatusBar(self.status)
        self.update_statusbar()

    def update_statusbar(self):
        cursor = self.editor.textCursor()
        line = cursor.blockNumber() + 1
        col = cursor.positionInBlock() + 1
        name = self.file_path if self.file_path else "Untitled"
        self.status.showMessage(f"{name} | Line: {line} Column: {col}")

    def init_menu(self):
        toolbar = QToolBar("File")
        self.addToolBar(toolbar)

        for label, callback in [
            ("New", self.new_file),
            ("Open", self.open_file),
            ("Save", self.save_file),
            ("Save As", self.save_file_as),
            ("Settings", self.show_settings)
        ]:
            btn = QPushButton(label)
            btn.clicked.connect(callback)
            toolbar.addWidget(btn)

        for label, callback in [
            ("Find", self.find_text),
            ("Replace", self.replace_text)
        ]:
            btn = QPushButton(label)
            btn.clicked.connect(callback)
            toolbar.addWidget(btn)

    def change_font(self, font):
        self.editor.setCurrentFont(font)

    def change_font_size(self, size):
        self.editor.setFontPointSize(float(size))

    def toggle_bold(self):
        fmt = self.editor.currentCharFormat()
        fmt.setFontWeight(QFont.Bold if fmt.fontWeight() != QFont.Bold else QFont.Normal)
        self.editor.setCurrentCharFormat(fmt)

    def toggle_underline(self):
        fmt = self.editor.currentCharFormat()
        fmt.setFontUnderline(not fmt.fontUnderline())
        self.editor.setCurrentCharFormat(fmt)

    def new_file(self):
        self.editor.clear()
        self.file_path = None
        self.update_statusbar()

    def open_file(self):
        path, _ = QFileDialog.getOpenFileName(self, "Open File")
        if path:
            with open(path, 'r') as f:
                self.editor.setText(f.read())
            self.file_path = path
            self.update_statusbar()

    def save_file(self):
        if self.file_path:
            with open(self.file_path, 'w') as f:
                f.write(self.editor.toPlainText())
        else:
            self.save_file_as()

    def save_file_as(self):
        path, _ = QFileDialog.getSaveFileName(self, "Save File")
        if path:
            self.file_path = path
            self.save_file()

    def show_settings(self):
        pass  # 미구현: 설정 대화상자

    def find_text(self):
        text, ok = QFileDialog.getText(self, "Find", "Text:")
        if ok:
            cursor = self.editor.textCursor()
            doc = self.editor.document()
            found = doc.find(text, cursor)
            if found.isNull():
                self.status.showMessage("Text not found")
            else:
                self.editor.setTextCursor(found)

    def replace_text(self):
        pass  # 미구현: 대체 기능

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = Notepad()
    window.show()
    sys.exit(app.exec())

