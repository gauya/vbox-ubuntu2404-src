#! /usr/bin/python

import sys
import os
import json
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QFileSystemModel, QTreeView,
    QVBoxLayout, QHBoxLayout, QPushButton, QCheckBox, QColorDialog,
    QFileDialog, QLineEdit, QLabel, QMenu, QMessageBox
)
from PySide6.QtCore import Qt, QModelIndex, QSize
from PySide6.QtGui import QCursor, QAction

CONFIG_PATH = os.path.expanduser("~/.myexplorer")

DEFAULT_CONFIG = {
    "left_folder": os.path.expanduser("~"),
    "right_folder": os.path.expanduser("~"),
    "window_size": [900, 700],
    "bg_color": "#FFFFFF",
    "explorer_color": "#F0F0F0",
    "show_hidden": False,
    "file_filter": "*"
}

def load_config():
    config = DEFAULT_CONFIG.copy()
    if os.path.exists(CONFIG_PATH):
        try:
            with open(CONFIG_PATH, "r") as f:
                loaded = json.load(f)
                for key in DEFAULT_CONFIG:
                    if key not in loaded:
                        loaded[key] = DEFAULT_CONFIG[key]
                config = loaded
        except Exception as e:
            print(f"⚠️ 설정 파일 로드 실패: {e} — 기본값 사용")
    return config

def save_config(config):
    try:
        with open(CONFIG_PATH, "w") as f:
            json.dump(config, f, indent=2)
    except Exception as e:
        print(f"⚠️ 설정 저장 실패: {e}")

class ExplorerApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.config = load_config()
        self.setWindowTitle("Dual Explorer")
        self.resize(*self.config["window_size"])
        self.setStyleSheet(f"background-color: {self.config['bg_color']};")

        self.model_left = QFileSystemModel()
        self.model_left.setRootPath("")

        self.model_right = QFileSystemModel()
        self.model_right.setRootPath("")

        self.init_ui()

    def init_ui(self):
        central_widget = QWidget()
        main_layout = QVBoxLayout(central_widget)

        # 필터와 히든 설정
        filter_layout = QHBoxLayout()
        self.filter_input = QLineEdit(self.config["file_filter"])
        self.hidden_check = QCheckBox("Show hidden")
        self.hidden_check.setChecked(self.config["show_hidden"])
        filter_layout.addWidget(QLabel("File Filter:"))
        filter_layout.addWidget(self.filter_input)
        filter_layout.addWidget(self.hidden_check)
        main_layout.addLayout(filter_layout)

        body_layout = QHBoxLayout()

        # 왼쪽 탐색기
        self.left_view = QTreeView()
        self.left_view.setModel(self.model_left)
        self.left_view.setRootIndex(self.model_left.index(self.config["left_folder"]))
        self.left_view.setStyleSheet(f"background-color: {self.config['explorer_color']};")
        self.left_view.setSelectionMode(QTreeView.ExtendedSelection)
        self.left_view.setContextMenuPolicy(Qt.CustomContextMenu)
        self.left_view.customContextMenuRequested.connect(lambda pos: self.show_context_menu(pos, self.left_view))

        # 오른쪽 탐색기
        self.right_view = QTreeView()
        self.right_view.setModel(self.model_right)
        self.right_view.setRootIndex(self.model_right.index(self.config["right_folder"]))
        self.right_view.setStyleSheet(f"background-color: {self.config['explorer_color']};")
        self.right_view.setSelectionMode(QTreeView.ExtendedSelection)
        self.right_view.setContextMenuPolicy(Qt.CustomContextMenu)
        self.right_view.customContextMenuRequested.connect(lambda pos: self.show_context_menu(pos, self.right_view))

        # 가운데 버튼들
        middle_buttons = QWidget()
        middle_layout = QVBoxLayout(middle_buttons)
        middle_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.move_checkbox = QCheckBox("Move")
        left_button = QPushButton("←")
        right_button = QPushButton("→")
        bg_button = QPushButton("BG Color")
        explorer_button = QPushButton("Explorer Color")

        left_button.clicked.connect(lambda: self.transfer_files(self.right_view, self.left_view))
        right_button.clicked.connect(lambda: self.transfer_files(self.left_view, self.right_view))
        bg_button.clicked.connect(self.change_bg_color)
        explorer_button.clicked.connect(self.change_explorer_color)

        middle_layout.addWidget(self.move_checkbox)
        middle_layout.addSpacing(10)
        middle_layout.addWidget(left_button)
        middle_layout.addWidget(right_button)
        middle_layout.addSpacing(10)
        middle_layout.addWidget(bg_button)
        middle_layout.addWidget(explorer_button)

        # 탐색기 선택기 추가 가능
        body_layout.addWidget(self.left_view)
        body_layout.addWidget(middle_buttons)
        body_layout.addWidget(self.right_view)

        main_layout.addLayout(body_layout)
        self.setCentralWidget(central_widget)

    def transfer_files(self, source_view, target_view):
        model = source_view.model()
        indexes = source_view.selectedIndexes()
        target_path = target_view.model().filePath(target_view.rootIndex())
        moved = self.move_checkbox.isChecked()

        for index in indexes:
            if index.column() != 0:
                continue
            src_path = model.filePath(index)
            name = os.path.basename(src_path)
            dst_path = os.path.join(target_path, name)
            try:
                if moved:
                    os.rename(src_path, dst_path)
                else:
                    if os.path.isdir(src_path):
                        os.system(f"cp -r '{src_path}' '{dst_path}'")
                    else:
                        with open(src_path, 'rb') as fsrc, open(dst_path, 'wb') as fdst:
                            fdst.write(fsrc.read())
            except Exception as e:
                QMessageBox.warning(self, "Error", str(e))

    def change_bg_color(self):
        color = QColorDialog.getColor()
        if color.isValid():
            self.config['bg_color'] = color.name()
            self.setStyleSheet(f"background-color: {self.config['bg_color']};")

    def change_explorer_color(self):
        color = QColorDialog.getColor()
        if color.isValid():
            self.config['explorer_color'] = color.name()
            self.left_view.setStyleSheet(f"background-color: {self.config['explorer_color']};")
            self.right_view.setStyleSheet(f"background-color: {self.config['explorer_color']};")

    def closeEvent(self, event):
        self.config["left_folder"] = self.model_left.filePath(self.left_view.rootIndex())
        self.config["right_folder"] = self.model_right.filePath(self.right_view.rootIndex())
        self.config["window_size"] = [self.width(), self.height()]
        self.config["file_filter"] = self.filter_input.text()
        self.config["show_hidden"] = self.hidden_check.isChecked()
        save_config(self.config)
        event.accept()

    def show_context_menu(self, pos, view):
        index = view.indexAt(pos)
        if not index.isValid():
            return
        model = view.model()
        file_path = model.filePath(index)

        menu = QMenu()
        info = QAction("Info")
        delete = QAction("Delete")
        rename = QAction("Rename")

        def show_info():
            try:
                size = os.path.getsize(file_path)
                QMessageBox.information(self, "Info", f"{file_path}\nSize: {size} bytes")
            except Exception as e:
                QMessageBox.warning(self, "Error", str(e))

        def delete_file():
            try:
                os.remove(file_path)
                view.model().refresh()
            except Exception as e:
                QMessageBox.warning(self, "Error", str(e))

        def rename_file():
            new_name, ok = QFileDialog.getSaveFileName(self, "Rename to", file_path)
            if ok and new_name:
                try:
                    os.rename(file_path, new_name)
                    view.model().refresh()
                except Exception as e:
                    QMessageBox.warning(self, "Error", str(e))

        info.triggered.connect(show_info)
        delete.triggered.connect(delete_file)
        rename.triggered.connect(rename_file)

        menu.addAction(info)
        menu.addAction(rename)
        menu.addAction(delete)
        menu.exec(QCursor.pos())

if __name__ == "__main__":
    app = QApplication(sys.argv)
    explorer = ExplorerApp()
    explorer.show()
    sys.exit(app.exec())

