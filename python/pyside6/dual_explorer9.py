#! /usr/bin/python

import sys
import os
import json
from pathlib import Path
from PySide6.QtWidgets import (
    QApplication, QWidget, QFileDialog, QListWidget, QVBoxLayout,
    QHBoxLayout, QPushButton, QLabel, QListWidgetItem, QSplitter,
    QCheckBox, QLineEdit, QMessageBox, QColorDialog, QMenu, QInputDialog
)
from PySide6.QtCore import Qt, QSize
from PySide6.QtGui import QColor

CONFIG_FILE = str(Path.home() / ".myexplorer")

class FileExplorer(QWidget):
    def __init__(self):
        super().__init__()

        self.config = self.load_config()
        self.setWindowTitle("Dual File Explorer")
        self.resize(self.config.get("width", 900), self.config.get("height", 700))

        self.left_path = self.config.get("left_path", str(Path.home()))
        self.right_path = self.config.get("right_path", str(Path.home()))

        self.bg_color = QColor(self.config.get("bg_color", "#ffffff"))
        self.explorer_bg_color = QColor(self.config.get("explorer_bg_color", "#f0f0f0"))

        self.init_ui()

    def init_ui(self):
        main_layout = QVBoxLayout(self)

        splitter = QSplitter(Qt.Horizontal)
        self.left_list = QListWidget()
        self.right_list = QListWidget()
        self.left_list.setSelectionMode(QListWidget.ExtendedSelection)
        self.right_list.setSelectionMode(QListWidget.ExtendedSelection)

        self.left_list.setStyleSheet(f"background-color: {self.explorer_bg_color.name()}")
        self.right_list.setStyleSheet(f"background-color: {self.explorer_bg_color.name()}")

        self.left_list.setContextMenuPolicy(Qt.CustomContextMenu)
        self.right_list.setContextMenuPolicy(Qt.CustomContextMenu)
        self.left_list.customContextMenuRequested.connect(lambda pos: self.show_context_menu(self.left_list, pos))
        self.right_list.customContextMenuRequested.connect(lambda pos: self.show_context_menu(self.right_list, pos))

        left_layout = QVBoxLayout()
        left_path_bar = QHBoxLayout()
        self.left_path_label = QLabel(self.left_path)
        self.left_select_button = QPushButton("Select Folder")
        self.left_select_button.clicked.connect(lambda: self.select_folder(True))
        left_path_bar.addWidget(self.left_path_label)
        left_path_bar.addWidget(self.left_select_button)
        left_layout.addLayout(left_path_bar)
        left_layout.addWidget(self.left_list)

        right_layout = QVBoxLayout()
        right_path_bar = QHBoxLayout()
        self.right_path_label = QLabel(self.right_path)
        self.right_select_button = QPushButton("Select Folder")
        self.right_select_button.clicked.connect(lambda: self.select_folder(False))
        right_path_bar.addWidget(self.right_path_label)
        right_path_bar.addWidget(self.right_select_button)
        right_layout.addLayout(right_path_bar)
        right_layout.addWidget(self.right_list)

        left_widget = QWidget()
        left_widget.setLayout(left_layout)
        right_widget = QWidget()
        right_widget.setLayout(right_layout)

        splitter.addWidget(left_widget)
        splitter.addWidget(right_widget)

        control_layout = QHBoxLayout()
        self.check_move = QCheckBox("Move")
        self.copy_left = QPushButton("←")
        self.copy_right = QPushButton("→")
        self.copy_left.clicked.connect(lambda: self.transfer_files(True))
        self.copy_right.clicked.connect(lambda: self.transfer_files(False))

        self.color_button = QPushButton("Colors")
        self.color_button.clicked.connect(self.select_colors)

        control_layout.addStretch()
        control_layout.addWidget(self.copy_left)
        control_layout.addWidget(self.check_move)
        control_layout.addWidget(self.copy_right)
        control_layout.addWidget(self.color_button)
        control_layout.addStretch()

        filter_layout = QHBoxLayout()
        self.file_filter = QLineEdit("*")
        self.show_hidden = QCheckBox("Show hidden")
        self.file_filter.textChanged.connect(self.refresh_lists)
        self.show_hidden.stateChanged.connect(self.refresh_lists)
        filter_layout.addWidget(QLabel("Filter:"))
        filter_layout.addWidget(self.file_filter)
        filter_layout.addWidget(self.show_hidden)

        main_layout.addLayout(filter_layout)
        main_layout.addWidget(splitter)
        main_layout.addLayout(control_layout)

        self.setStyleSheet(f"background-color: {self.bg_color.name()}")

        self.refresh_lists()

    def load_config(self):
        try:
            with open(CONFIG_FILE) as f:
                config = json.load(f)
        except:
            config = {}

        defaults = {
            "left_path": str(Path.home()),
            "right_path": str(Path.home()),
            "width": 900,
            "height": 700,
            "bg_color": "#ffffff",
            "explorer_bg_color": "#f0f0f0"
        }
        for k, v in defaults.items():
            config.setdefault(k, v)
        return config

    def save_config(self):
        self.config.update({
            "left_path": self.left_path,
            "right_path": self.right_path,
            "width": self.width(),
            "height": self.height(),
            "bg_color": self.bg_color.name(),
            "explorer_bg_color": self.explorer_bg_color.name()
        })
        with open(CONFIG_FILE, 'w') as f:
            json.dump(self.config, f)

    def select_folder(self, left):
        path = QFileDialog.getExistingDirectory(self, "Select Folder", str(Path.home()))
        if path:
            if left:
                self.left_path = path
                self.left_path_label.setText(path)
            else:
                self.right_path = path
                self.right_path_label.setText(path)
            self.refresh_lists()

    def refresh_lists(self):
        def populate(list_widget, path):
            list_widget.clear()
            filter_text = self.file_filter.text()
            show_hidden = self.show_hidden.isChecked()
            try:
                for entry in sorted(Path(path).glob(filter_text)):
                    if not show_hidden and entry.name.startswith('.'):
                        continue
                    item = QListWidgetItem(entry.name)
                    item.setData(Qt.UserRole, str(entry))
                    list_widget.addItem(item)
            except Exception as e:
                print(e)

        populate(self.left_list, self.left_path)
        populate(self.right_list, self.right_path)

    def transfer_files(self, to_left):
        src_list = self.right_list if to_left else self.left_list
        dst_path = self.left_path if to_left else self.right_path

        import shutil
        for item in src_list.selectedItems():
            src = Path(item.data(Qt.UserRole))
            dst = Path(dst_path) / src.name
            try:
                if self.check_move.isChecked():
                    shutil.move(str(src), str(dst))
                else:
                    if src.is_dir():
                        shutil.copytree(str(src), str(dst), dirs_exist_ok=True)
                    else:
                        shutil.copy2(str(src), str(dst))
            except Exception as e:
                QMessageBox.critical(self, "Error", str(e))

        self.refresh_lists()

    def show_context_menu(self, list_widget, pos):
        item = list_widget.itemAt(pos)
        if not item:
            return
        file_path = Path(item.data(Qt.UserRole))
        menu = QMenu()

        size_action = menu.addAction(f"Size: {file_path.stat().st_size} bytes")
        rename_action = menu.addAction("Rename")
        delete_action = menu.addAction("Delete")
        action = menu.exec(list_widget.mapToGlobal(pos))

        if action == rename_action:
            new_name, ok = QInputDialog.getText(self, "Rename", "New name:", text=file_path.name)
            if ok and new_name:
                file_path.rename(file_path.parent / new_name)
                self.refresh_lists()
        elif action == delete_action:
            confirm = QMessageBox.question(self, "Delete", f"Delete {file_path.name}?", QMessageBox.Yes | QMessageBox.No)
            if confirm == QMessageBox.Yes:
                if file_path.is_dir():
                    import shutil
                    shutil.rmtree(file_path)
                else:
                    file_path.unlink()
                self.refresh_lists()

    def select_colors(self):
        bg = QColorDialog.getColor(self.bg_color, self, "Select Background Color")
        if bg.isValid():
            self.bg_color = bg
            self.setStyleSheet(f"background-color: {self.bg_color.name()}")

        explorer_bg = QColorDialog.getColor(self.explorer_bg_color, self, "Select Explorer Color")
        if explorer_bg.isValid():
            self.explorer_bg_color = explorer_bg
            self.left_list.setStyleSheet(f"background-color: {self.explorer_bg_color.name()}")
            self.right_list.setStyleSheet(f"background-color: {self.explorer_bg_color.name()}")

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = FileExplorer()
    window.show()
    app.exec()
    window.save_config()

