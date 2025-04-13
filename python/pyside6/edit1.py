#! /usr/bin/python

import sys
from PySide6.QtWidgets import (QApplication, QMainWindow, QTextEdit, QFileDialog,
                               QMessageBox, QStatusBar, QLabel, QDialog, QVBoxLayout,
                               QHBoxLayout, QPushButton, QLineEdit)
from PySide6.QtGui import QTextCursor, QAction, QKeySequence
from PySide6.QtCore import Qt

class FindDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("찾기")
        self.resize(300, 100)
        
        layout = QVBoxLayout()
        
        # 찾을 텍스트 입력 필드
        self.find_line_edit = QLineEdit()
        layout.addWidget(self.find_line_edit)
        
        # 버튼 레이아웃
        button_layout = QHBoxLayout()
        
        self.find_button = QPushButton("찾기")
        self.find_button.clicked.connect(self.find_text)
        button_layout.addWidget(self.find_button)
        
        self.cancel_button = QPushButton("취소")
        self.cancel_button.clicked.connect(self.close)
        button_layout.addWidget(self.cancel_button)
        
        layout.addLayout(button_layout)
        self.setLayout(layout)
    
    def find_text(self):
        text_to_find = self.find_line_edit.text()
        if text_to_find:
            self.parent().find_text(text_to_find)

class TextEditor(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("PySide6 메모장")
        self.resize(800, 600)
        
        # 중앙 텍스트 편집기
        self.text_edit = QTextEdit()
        self.setCentralWidget(self.text_edit)
        
        # 상태 표시줄 설정
        self.status_bar = QStatusBar()
        self.setStatusBar(self.status_bar)
        
        # 줄/열 위치 표시 라벨
        self.position_label = QLabel("줄: 1, 열: 1")
        self.status_bar.addPermanentWidget(self.position_label)
        
        # 커서 위치 변경 시 업데이트
        self.text_edit.cursorPositionChanged.connect(self.update_cursor_position)
        
        # 파일 관련 변수
        self.current_file = None
        
        # 메뉴바 생성
        self.create_menus()
        
        # 초기 상태 업데이트
        self.update_cursor_position()
    
    def create_menus(self):
        # 파일 메뉴
        file_menu = self.menuBar().addMenu("파일(&F)")
        
        # 새 파일
        new_action = QAction("새로 만들기(&N)", self)
        new_action.setShortcut(QKeySequence.New)
        new_action.triggered.connect(self.new_file)
        file_menu.addAction(new_action)
        
        # 열기
        open_action = QAction("열기(&O)...", self)
        open_action.setShortcut(QKeySequence.Open)
        open_action.triggered.connect(self.open_file)
        file_menu.addAction(open_action)
        
        # 저장
        save_action = QAction("저장(&S)", self)
        save_action.setShortcut(QKeySequence.Save)
        save_action.triggered.connect(self.save_file)
        file_menu.addAction(save_action)
        
        # 다른 이름으로 저장
        save_as_action = QAction("다른 이름으로 저장(&A)...", self)
        save_as_action.triggered.connect(self.save_file_as)
        file_menu.addAction(save_as_action)
        
        file_menu.addSeparator()
        
        # 종료
        exit_action = QAction("종료(&X)", self)
        exit_action.setShortcut(QKeySequence.Quit)
        exit_action.triggered.connect(self.close)
        file_menu.addAction(exit_action)
        
        # 편집 메뉴
        edit_menu = self.menuBar().addMenu("편집(&E)")
        
        # 찾기
        find_action = QAction("찾기(&F)...", self)
        find_action.setShortcut(QKeySequence.Find)
        find_action.triggered.connect(self.show_find_dialog)
        edit_menu.addAction(find_action)
        
        # 잘라내기
        cut_action = QAction("잘라내기(&T)", self)
        cut_action.setShortcut(QKeySequence.Cut)
        cut_action.triggered.connect(self.text_edit.cut)
        edit_menu.addAction(cut_action)
        
        # 복사
        copy_action = QAction("복사(&C)", self)
        copy_action.setShortcut(QKeySequence.Copy)
        copy_action.triggered.connect(self.text_edit.copy)
        edit_menu.addAction(copy_action)
        
        # 붙여넣기
        paste_action = QAction("붙여넣기(&P)", self)
        paste_action.setShortcut(QKeySequence.Paste)
        paste_action.triggered.connect(self.text_edit.paste)
        edit_menu.addAction(paste_action)
        
        # 실행 취소
        undo_action = QAction("실행 취소(&U)", self)
        undo_action.setShortcut(QKeySequence.Undo)
        undo_action.triggered.connect(self.text_edit.undo)
        edit_menu.addAction(undo_action)
        
        # 다시 실행
        redo_action = QAction("다시 실행(&R)", self)
        redo_action.setShortcut(QKeySequence.Redo)
        redo_action.triggered.connect(self.text_edit.redo)
        edit_menu.addAction(redo_action)
    
    def update_cursor_position(self):
        cursor = self.text_edit.textCursor()
        line = cursor.blockNumber() + 1
        col = cursor.columnNumber() + 1
        self.position_label.setText(f"줄: {line}, 열: {col}")
    
    def new_file(self):
        if self.maybe_save():
            self.text_edit.clear()
            self.current_file = None
    
    def open_file(self):
        if self.maybe_save():
            file_name, _ = QFileDialog.getOpenFileName(self, "파일 열기", "", "텍스트 파일 (*.txt);;모든 파일 (*)")
            if file_name:
                try:
                    with open(file_name, 'r', encoding='utf-8') as f:
                        self.text_edit.setText(f.read())
                    self.current_file = file_name
                except Exception as e:
                    QMessageBox.warning(self, "오류", f"파일을 열 수 없습니다:\n{str(e)}")
    
    def save_file(self):
        if self.current_file:
            try:
                with open(self.current_file, 'w', encoding='utf-8') as f:
                    f.write(self.text_edit.toPlainText())
                return True
            except Exception as e:
                QMessageBox.warning(self, "오류", f"파일을 저장할 수 없습니다:\n{str(e)}")
                return False
        else:
            return self.save_file_as()
    
    def save_file_as(self):
        file_name, _ = QFileDialog.getSaveFileName(self, "다른 이름으로 저장", "", "텍스트 파일 (*.txt);;모든 파일 (*)")
        if file_name:
            if not file_name.endswith('.txt'):
                file_name += '.txt'
            self.current_file = file_name
            return self.save_file()
        return False
    
    def maybe_save(self):
        if not self.text_edit.document().isModified():
            return True
        
        ret = QMessageBox.question(self, "저장 확인",
                                 "문서가 변경되었습니다. 저장하시겠습니까?",
                                 QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
        
        if ret == QMessageBox.Save:
            return self.save_file()
        elif ret == QMessageBox.Cancel:
            return False
        
        return True
    
    def show_find_dialog(self):
        dialog = FindDialog(self)
        dialog.exec()
    
    def find_text(self, text):
        if not text:
            return
        
        # 현재 커서 위치에서 검색 시작
        cursor = self.text_edit.textCursor()
        document = self.text_edit.document()
        
        # 검색 옵션 설정
        flags = QTextCursor.FindFlag(0)
        
        # 문서에서 텍스트 찾기
        found = document.find(text, cursor.position(), flags)
        
        if found.isNull():
            # 처음부터 다시 검색
            found = document.find(text, 0, flags)
            if found.isNull():
                QMessageBox.information(self, "찾기", "텍스트를 찾을 수 없습니다.")
                return
        
        # 찾은 텍스트 선택
        self.text_edit.setTextCursor(found)
    
    def closeEvent(self, event):
        if self.maybe_save():
            event.accept()
        else:
            event.ignore()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    editor = TextEditor()
    editor.show()
    sys.exit(app.exec())
