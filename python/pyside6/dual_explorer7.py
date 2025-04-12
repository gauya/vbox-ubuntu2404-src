#! /usr/bin/python

import os
import sys
import json

from PySide6.QtWidgets import (
    QApplication, QWidget, QHBoxLayout, QVBoxLayout, QListWidget, QPushButton,
    QFileDialog, QLabel, QSplitter, QListWidgetItem, QAbstractItemView,
    QCheckBox, QLineEdit, QMessageBox, QMenu, QInputDialog, QFileSystemModel, QTreeView
)
"""
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QPushButton, QCheckBox,
    QColorDialog, QFileSystemModel, QTreeView, QLabel
)
"""
from PySide6.QtCore import Qt, QSize
from PySide6.QtGui import QCursor

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
                # 누락된 항목을 기본값으로 채움
                for key in DEFAULT_CONFIG:
                    if key not in loaded:
                        loaded[key] = DEFAULT_CONFIG[key]
                config = loaded
        except Exception as e:
            print(f"⚠️ 설정 파일 로드 실패: {e} — 기본값 사용")
    return config

def save_config(config):
    with open(CONFIG_PATH, 'w') as f:
        json.dump(config, f)

class FileExplorer(QWidget):
    def __init__(self):
        super().__init__()
        self.config = load_config()

        self.setWindowTitle("Dual File Explorer")
        self.resize(*self.config.get("window_size", [900, 700]))
        self.setStyleSheet(f"background-color: {self.config['bg_color']};")

        self.splitter = QSplitter(Qt.Horizontal)

        self.left_pane = self.create_pane('left')
        self.right_pane = self.create_pane('right')

        self.splitter.addWidget(self.left_pane['widget'])
        self.splitter.addWidget(self.right_pane['widget'])

        self.arrow_layout = QVBoxLayout()
        self.move_checkbox = QCheckBox("Move")
        self.arrow_layout.addWidget(self.move_checkbox)

        self.left_to_right = QPushButton("→")
        self.left_to_right.clicked.connect(lambda: self.transfer_files(self.left_pane, self.right_pane))
        self.arrow_layout.addWidget(self.left_to_right)

        self.right_to_left = QPushButton("←")
        self.right_to_left.clicked.connect(lambda: self.transfer_files(self.right_pane, self.left_pane))
        self.arrow_layout.addWidget(self.right_to_left)

        main_layout = QHBoxLayout(self)
        main_layout.addWidget(self.splitter)
        main_layout.addLayout(self.arrow_layout)

        color_btn = QPushButton("Change Color")
        color_btn.clicked.connect(self.toggle_color)
        main_layout.addWidget(color_btn)

    def create_pane(self, side):
        pane = {}
        layout = QVBoxLayout()
        widget = QWidget()

        path = self.config.get(f"{side}_dir", os.path.expanduser("~"))
        pane['current_path'] = path

        # Header Layout
        header_layout = QHBoxLayout()
        path_label = QLabel(path)
        pane['path_label'] = path_label
        header_layout.addWidget(path_label)
        select_btn = QPushButton("Select Folder")
        select_btn.clicked.connect(lambda: self.select_folder(pane, side))
        header_layout.addWidget(select_btn)
        layout.addLayout(header_layout)

        # File type and hidden filter layout
        filter_layout = QHBoxLayout()
        file_filter = QLineEdit(self.config.get("file_filter", "*"))
        file_filter.setPlaceholderText("Filter e.g. *.txt")
        file_filter.textChanged.connect(lambda: self.update_file_list(pane))
        pane['file_filter'] = file_filter
        filter_layout.addWidget(file_filter)

        show_hidden = QCheckBox("Show Hidden")
        show_hidden.setChecked(self.config.get("show_hidden", False))
        show_hidden.stateChanged.connect(lambda: self.update_file_list(pane))
        pane['show_hidden'] = show_hidden
        filter_layout.addWidget(show_hidden)

        layout.addLayout(filter_layout)

        file_list = QListWidget()
        file_list.setSelectionMode(QAbstractItemView.ExtendedSelection)
        file_list.setStyleSheet(f"background-color: {self.config['explorer_color']};")
        file_list.setContextMenuPolicy(Qt.CustomContextMenu)
        file_list.customContextMenuRequested.connect(lambda pos, p=pane: self.show_context_menu(pos, p))
        pane['list_widget'] = file_list
        layout.addWidget(file_list)

        widget.setLayout(layout)
        self.update_file_list(pane)

        pane['widget'] = widget
        return pane

    def select_folder(self, pane, side):
        folder = QFileDialog.getExistingDirectory(self, f"Select {side.capitalize()} Folder")
        if folder:
            pane['current_path'] = folder
            pane['path_label'].setText(folder)
            self.config[f"{side}_dir"] = folder
            self.update_file_list(pane)

    def update_file_list(self, pane):
        directory = pane['current_path']
        filter_pattern = pane['file_filter'].text()
        show_hidden = pane['show_hidden'].isChecked()
        list_widget = pane['list_widget']

        list_widget.clear()
        try:
            for item in sorted(os.listdir(directory)):
                if not show_hidden and item.startswith('.'):
                    continue
                full_path = os.path.join(directory, item)
                if os.path.isfile(full_path) or os.path.isdir(full_path):
                    if QFileDialog().nameFilters()[0] or filter_pattern == "*" or QFileDialog().filterSelected():
                        list_widget.addItem(item)
        except Exception as e:
            QMessageBox.warning(self, "Error", str(e))

    def transfer_files(self, src_pane, dst_pane):
        src_dir = src_pane['current_path']
        dst_dir = dst_pane['current_path']
        for item in src_pane['list_widget'].selectedItems():
            src_path = os.path.join(src_dir, item.text())
            dst_path = os.path.join(dst_dir, item.text())
            try:
                if self.move_checkbox.isChecked():
                    os.rename(src_path, dst_path)
                else:
                    if os.path.isdir(src_path):
                        import shutil
                        shutil.copytree(src_path, dst_path, dirs_exist_ok=True)
                    else:
                        import shutil
                        shutil.copy2(src_path, dst_path)
            except Exception as e:
                QMessageBox.warning(self, "Transfer Error", str(e))
        self.update_file_list(dst_pane)

    def toggle_color(self):
        if self.config['bg_color'] == "#ffffff":
            self.config['bg_color'] = "#222222"
            self.config['explorer_color'] = "#aaaaaa"
        else:
            self.config['bg_color'] = "#ffffff"
            self.config['explorer_color'] = "#f0f0f0"
        self.setStyleSheet(f"background-color: {self.config['bg_color']};")
        for pane in [self.left_pane, self.right_pane]:
            pane['list_widget'].setStyleSheet(f"background-color: {self.config['explorer_color']};")

    def show_context_menu(self, pos, pane):
        item = pane['list_widget'].itemAt(pos)
        if item:
            menu = QMenu()
            file_path = os.path.join(pane['current_path'], item.text())

            size = os.path.getsize(file_path)
            menu.addAction(f"Size: {size} bytes")

            rename_action = menu.addAction("Rename")
            delete_action = menu.addAction("Delete")

            action = menu.exec(QCursor.pos())

            if action == rename_action:
                new_name, ok = QInputDialog.getText(self, "Rename", "New name:")
                if ok and new_name:
                    os.rename(file_path, os.path.join(pane['current_path'], new_name))
                    self.update_file_list(pane)
            elif action == delete_action:
                os.remove(file_path)
                self.update_file_list(pane)

    def closeEvent(self, event):
        self.config['window_size'] = [self.width(), self.height()]
        self.config['file_filter'] = self.left_pane['file_filter'].text()
        self.config['show_hidden'] = self.left_pane['show_hidden'].isChecked()
        save_config(self.config)
        event.accept()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    explorer = FileExplorer()
    explorer.show()
    sys.exit(app.exec())

