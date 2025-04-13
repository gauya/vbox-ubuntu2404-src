
import sys
import os
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QTextEdit, QFileDialog, QToolBar, QLabel,
    QFontComboBox, QComboBox, QStatusBar, QMessageBox
)
from PySide6.QtGui import QIcon, QKeySequence, QTextCursor, QTextCharFormat, QFont, QAction
from PySide6.QtCore import Qt


class Notepad(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("PySide6 Notepad")
        self.resize(800, 600)

        self.text_edit = QTextEdit()
        self.setCentralWidget(self.text_edit)

        self.current_file = None

        self.create_menu()
        self.create_toolbar()
        self.create_status_bar()

        self.text_edit.textChanged.connect(self.update_status_bar)
        self.text_edit.cursorPositionChanged.connect(self.update_status_bar)

    def create_menu(self):
        menu = self.menuBar()

        file_menu = menu.addMenu("파일")
        new_action = QAction("새로", self)
        new_action.triggered.connect(self.new_file)
        file_menu.addAction(new_action)

        open_action = QAction("불러오기", self)
        open_action.triggered.connect(self.open_file)
        file_menu.addAction(open_action)

        save_action = QAction("저장", self)
        save_action.triggered.connect(self.save_file)
        file_menu.addAction(save_action)

        save_as_action = QAction("다른이름으로 저장", self)
        save_as_action.triggered.connect(self.save_file_as)
        file_menu.addAction(save_as_action)

        settings_action = QAction("설정", self)
        file_menu.addAction(settings_action)  # 구현 예정

        edit_menu = menu.addMenu("편집")
        find_action = QAction("찾기", self)
        find_action.triggered.connect(self.find_text)
        edit_menu.addAction(find_action)

        replace_action = QAction("찾아바꾸기", self)
        replace_action.triggered.connect(self.replace_text)
        edit_menu.addAction(replace_action)

    def create_toolbar(self):
        toolbar = QToolBar("Font Toolbar")
        self.addToolBar(toolbar)

        self.font_box = QFontComboBox()
        self.font_box.currentFontChanged.connect(self.set_font)
        toolbar.addWidget(self.font_box)

        self.size_box = QComboBox()
        self.size_box.addItems([str(i) for i in range(8, 30, 2)])
        self.size_box.currentTextChanged.connect(self.set_font_size)
        toolbar.addWidget(self.size_box)

        bold_action = QAction("굵게", self)
        bold_action.setCheckable(True)
        bold_action.triggered.connect(self.set_bold)
        toolbar.addAction(bold_action)

        underline_action = QAction("밑줄", self)
        underline_action.setCheckable(True)
        underline_action.triggered.connect(self.set_underline)
        toolbar.addAction(underline_action)

    def create_status_bar(self):
        self.status = QStatusBar()
        self.setStatusBar(self.status)
        self.status_label = QLabel("Ready")
        self.status.addPermanentWidget(self.status_label)
        self.update_status_bar()

    def update_status_bar(self):
        cursor = self.text_edit.textCursor()
        line = cursor.blockNumber() + 1
        col = cursor.columnNumber() + 1
        file_display = self.current_file if self.current_file else "(새 파일)"
        self.status_label.setText(f"{file_display} | 줄: {line}, 칼럼: {col}")

    def new_file(self):
        self.text_edit.clear()
        self.current_file = None
        self.update_status_bar()

    def open_file(self):
        path, _ = QFileDialog.getOpenFileName(self, "파일 열기")
        if path:
            with open(path, 'r', encoding='utf-8') as f:
                self.text_edit.setPlainText(f.read())
                self.current_file = path
                self.update_status_bar()

    def save_file(self):
        if self.current_file:
            with open(self.current_file, 'w', encoding='utf-8') as f:
                f.write(self.text_edit.toPlainText())
        else:
            self.save_file_as()

    def save_file_as(self):
        path, _ = QFileDialog.getSaveFileName(self, "다른 이름으로 저장")
        if path:
            self.current_file = path
            self.save_file()
            self.update_status_bar()

    def find_text(self):
        text, ok = QInputDialog.getText(self, "찾기", "찾을 내용:")
        if ok and text:
            cursor = self.text_edit.textCursor()
            document = self.text_edit.document()
            found = document.find(text, cursor)
            if found.isNull():
                QMessageBox.information(self, "찾기", "찾을 수 없음")
            else:
                self.text_edit.setTextCursor(found)

    def replace_text(self):
        from PySide6.QtWidgets import QInputDialog
        find, ok1 = QInputDialog.getText(self, "찾기", "찾을 내용:")
        if not ok1 or not find:
            return
        replace, ok2 = QInputDialog.getText(self, "바꾸기", "바꿀 내용:")
        if not ok2:
            return

        text = self.text_edit.toPlainText().replace(find, replace)
        self.text_edit.setPlainText(text)

    def set_font(self, font):
        fmt = QTextCharFormat()
        fmt.setFont(font)
        self.merge_format_on_selection(fmt)

    def set_font_size(self, size):
        fmt = QTextCharFormat()
        fmt.setFontPointSize(float(size))
        self.merge_format_on_selection(fmt)

    def set_bold(self, checked):
        fmt = QTextCharFormat()
        fmt.setFontWeight(QFont.Bold if checked else QFont.Normal)
        self.merge_format_on_selection(fmt)

    def set_underline(self, checked):
        fmt = QTextCharFormat()
        fmt.setFontUnderline(checked)
        self.merge_format_on_selection(fmt)

    def merge_format_on_selection(self, format):
        cursor = self.text_edit.textCursor()
        if not cursor.hasSelection():
            cursor.select(QTextCursor.WordUnderCursor)
        cursor.mergeCharFormat(format)
        self.text_edit.mergeCurrentCharFormat(format)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = Notepad()
    window.show()
    sys.exit(app.exec())

