#! /usr/bin/python

import os
import json
from PySide6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QListWidget,
    QPushButton, QFileDialog, QLabel, QLineEdit, QListWidgetItem,
    QColorDialog, QCheckBox
)
from PySide6.QtCore import Qt
from PySide6.QtGui import QColor
import shutil
import fnmatch

CONFIG_FILE = os.path.expanduser("~/.myexplorer")

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
    if os.path.exists(CONFIG_FILE):
        try:
            with open(CONFIG_FILE, "r") as f:
                loaded = json.load(f)
                # 누락된 항목을 기본값으로 채움
                for key in DEFAULT_CONFIG:
                    if key not in loaded:
                        loaded[key] = DEFAULT_CONFIG[key]
                config = loaded
        except Exception as e:
            print(f"⚠️ 설정 파일 로드 실패: {e} — 기본값 사용")
    return config

def save_config(config):
    with open(CONFIG_FILE, 'w') as f:
        json.dump(config, f)

class FileExplorer(QWidget):
    def __init__(self):
        super().__init__()
        self.config = load_config()
        self.pane = {}

        self.setWindowTitle("Dual Pane File Explorer")
        self.resize(*self.config.get("window_size", [900, 700]))
        #self.setMinimumSize(900, 700)

        self.main_bg_color = self.config['bg_color']
        self.pane_bg_color = self.config['explorer_color']

        self.load_last_folders()

        layout = QHBoxLayout(self)

        self.left_pane = self.create_pane()
        self.right_pane = self.create_pane()

        layout.addLayout(self.left_pane["layout"])
        layout.addLayout(self.create_controls())
        layout.addLayout(self.right_pane["layout"])

        self.setLayout(layout)
        self.apply_colors()

    def create_pane(self):
        layout = QVBoxLayout()
        path_layout = QHBoxLayout()

        path_input = QLineEdit()
        browse_button = QPushButton("Folder")
        path_layout.addWidget(path_input)
        path_layout.addWidget(browse_button)

        pattern_input = QLineEdit("*")
        file_list = QListWidget()

        show_hidden = QCheckBox("Show Hidden")
        show_hidden.setChecked(self.config.get("show_hidden", False))
        show_hidden.stateChanged.connect(lambda: update_file_list())
        self.pane['show_hidden'] = show_hidden
        self.pane['file_filter'] = pattern_input.text

        def browse():
            folder = QFileDialog.getExistingDirectory(self, "Folder")
            if folder:
                path_input.setText(folder)
                update_file_list()

        def update_file_list():
            folder = path_input.text()
            pattern = pattern_input.text()
            file_list.clear()
            if os.path.isdir(folder):
                for fname in os.listdir(folder):
                    if not show_hidden.isChecked() and fname[0] == '.':
                        continue
                    
                    full_path = os.path.join(folder, fname)
                    if os.path.isfile(full_path) and fnmatch.fnmatch(fname, pattern):
                        item = QListWidgetItem(fname)
                        item.setData(Qt.UserRole, full_path)
                        file_list.addItem(item)

        path_input.textChanged.connect(update_file_list)
        show_hidden.stateChanged.connect(update_file_list)
        pattern_input.textChanged.connect(update_file_list)
        browse_button.clicked.connect(browse)

        layout.addLayout(path_layout)
        layout.addWidget(pattern_input)
        layout.addWidget(show_hidden)
        layout.addWidget(file_list)

        return {
            "layout": layout,
            "path_input": path_input,
            "pattern_input": pattern_input,
            "file_list": file_list,
            "update_file_list": update_file_list
        }

    def create_controls(self):
        layout = QVBoxLayout()
        layout.addStretch()

        self.move_checkbox = QCheckBox("Move files")
        to_right_btn = QPushButton("→")
        to_left_btn = QPushButton("←")

        to_right_btn.clicked.connect(lambda: self.transfer_files(self.left_pane, self.right_pane))
        to_left_btn.clicked.connect(lambda: self.transfer_files(self.right_pane, self.left_pane))

        bgcolor_btn = QPushButton("Set bgColors")
        bgcolor_btn.clicked.connect(self.set_bgcolors)
        pncolor_btn = QPushButton("Set pnColors")
        pncolor_btn.clicked.connect(self.set_pncolors)

        layout.addWidget(self.move_checkbox)
        layout.addWidget(to_right_btn)
        layout.addWidget(to_left_btn)
        layout.addWidget(bgcolor_btn)
        layout.addWidget(pncolor_btn)
        layout.addStretch()
        return layout

    def transfer_files(self, source, destination):
        src_folder = source["path_input"].text()
        dst_folder = destination["path_input"].text()
        selected_items = source["file_list"].selectedItems()

        if not os.path.isdir(src_folder) or not os.path.isdir(dst_folder):
            return

        for item in selected_items:
            src_file = item.data(Qt.UserRole)
            dst_file = os.path.join(dst_folder, os.path.basename(src_file))
            try:
                if self.move_checkbox.isChecked():
                    shutil.move(src_file, dst_file)
                else:
                    shutil.copy2(src_file, dst_file)
            except Exception as e:
                print(f"Failed to copy/move {src_file} to {dst_file}: {e}")

        destination["update_file_list"]()
        source["update_file_list"]()

    def apply_colors(self):
        self.setStyleSheet(f"background-color: {self.main_bg_color};")
        for pane in [self.left_pane, self.right_pane]:
            pane["file_list"].setStyleSheet(f"background-color: {self.explorer_color};")

    def set_bgcolors(self):
        main_color = QColorDialog.getColor(QColor(self.main_bg_color), self, "Select Main Background Color")
        if main_color.isValid():
            self.main_bg_color = main_color.name()
        self.setStyleSheet(f"background-color: {self.main_bg_color};")

    def set_pncolors(self):
        explorer_color = QColorDialog.getColor(QColor(self.explorer_color), self, "Select Pane Background Color")
        if explorer_color.isValid():
            self.explorer_color = explorer_color.name()
        for pane in [self.left_pane, self.right_pane]:
            pane["file_list"].setStyleSheet(f"background-color: {self.explorer_color};")

    def load_last_folders(self):
        default = os.path.expanduser("~")
        self.last_left_path = self.last_right_path = default
        self.main_bg_color = "white"
        self.explorer_color = "#f0f0f0"
        if os.path.exists(CONFIG_FILE):
            try:
                with open(CONFIG_FILE, "r") as f:
                    data = json.load(f)
                    self.last_left_path = data.get("left", default)
                    self.last_right_path = data.get("right", default)
                    self.main_bg_color = data.get("main_bg_color", "white")
                    self.explorer_color = data.get("explorer_color", "#f0f0f0")
            except Exception as e:
                print("Failed to load config:", e)

    def showEvent(self, event):
        self.left_pane["path_input"].setText(self.last_left_path)
        self.left_pane["update_file_list"]()
        self.right_pane["path_input"].setText(self.last_right_path)
        self.right_pane["update_file_list"]()
        super().showEvent(event)

    def closeEvent(self, event):
        self.config['window_size'] = [self.width(), self.height()]
        #self.config['file_filter'] = self.pane['file_filter']
        #self.config['show_hidden'] = self.pane['show_hidden'].isChecked()
        save_config(self.config)
        event.accept()
        super().closeEvent(event)

if __name__ == "__main__":
    app = QApplication([])
    window = FileExplorer()
    window.show()
    app.exec()
