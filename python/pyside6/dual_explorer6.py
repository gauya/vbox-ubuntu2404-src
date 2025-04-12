#! /usr/bin/python

import sys
import os
import json
from pathlib import Path
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QHBoxLayout, QVBoxLayout,
    QPushButton, QFileDialog, QListWidget, QListWidgetItem, QLabel,
    QLineEdit, QMessageBox, QCheckBox, QColorDialog, QMenu
)
from PySide6.QtGui import QAction, QCursor
from PySide6.QtCore import Qt, QSize

CONFIG_PATH = os.path.expanduser("~/.myexplorer")

class FileExplorer(QWidget):
    def __init__(self, side, path, bg_color):
        super().__init__()
        self.side = side
        self.current_path = path or str(Path.home())
        self.bg_color = bg_color

        self.layout = QVBoxLayout()
        self.setLayout(self.layout)

        self.path_layout = QHBoxLayout()
        self.path_edit = QLineEdit(self.current_path)
        self.select_btn = QPushButton("Select Folder")
        self.select_btn.clicked.connect(self.select_folder)

        self.path_layout.addWidget(self.path_edit)
        self.path_layout.addWidget(self.select_btn)

        self.file_list = QListWidget()
        self.file_list.setSelectionMode(QListWidget.MultiSelection)
        self.file_list.setContextMenuPolicy(Qt.CustomContextMenu)
        self.file_list.customContextMenuRequested.connect(self.show_context_menu)

        self.layout.addLayout(self.path_layout)
        self.layout.addWidget(self.file_list)

        self.setStyleSheet(f"background-color: {self.bg_color};")

        self.load_files()

    def select_folder(self):
        path = QFileDialog.getExistingDirectory(self, f"Select {self.side} Folder", self.current_path)
        if path:
            self.current_path = path
            self.path_edit.setText(path)
            self.load_files()

    def load_files(self):
        self.file_list.clear()
        path = self.path_edit.text()
        if os.path.isdir(path):
            for entry in os.listdir(path):
                self.file_list.addItem(entry)

    def get_selected_files(self):
        return [item.text() for item in self.file_list.selectedItems()]

    def show_context_menu(self, position):
        selected = self.file_list.itemAt(position)
        if selected:
            file_path = os.path.join(self.path_edit.text(), selected.text())
            menu = QMenu()

            info = QAction("Info", self)
            info.triggered.connect(lambda: self.show_info(file_path))

            rename = QAction("Rename", self)
            rename.triggered.connect(lambda: self.rename_file(file_path, selected))

            delete = QAction("Delete", self)
            delete.triggered.connect(lambda: self.delete_file(file_path))

            menu.addAction(info)
            menu.addAction(rename)
            menu.addAction(delete)

            menu.exec(QCursor.pos())

    def show_info(self, file_path):
        try:
            size = os.path.getsize(file_path)
            info = f"File: {os.path.basename(file_path)}\nSize: {size} bytes"
            QMessageBox.information(self, "File Info", info)
        except Exception as e:
            QMessageBox.warning(self, "Error", str(e))

    def rename_file(self, file_path, item):
        new_name, ok = QFileDialog.getSaveFileName(self, "Rename File", file_path)
        if ok and new_name:
            try:
                os.rename(file_path, new_name)
                item.setText(os.path.basename(new_name))
            except Exception as e:
                QMessageBox.warning(self, "Rename Error", str(e))

    def delete_file(self, file_path):
        confirm = QMessageBox.question(self, "Confirm Delete", f"Delete {file_path}?", QMessageBox.Yes | QMessageBox.No)
        if confirm == QMessageBox.Yes:
            try:
                os.remove(file_path)
                self.load_files()
            except Exception as e:
                QMessageBox.warning(self, "Delete Error", str(e))

class ExplorerWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Dual Explorer")

        self.config = self.load_config()
        self.resize(self.config.get("window_width", 900), self.config.get("window_height", 700))

        self.left_color = self.config.get("left_color", "#f0f0f0")
        self.right_color = self.config.get("right_color", "#f0f0f0")

        self.left = FileExplorer("Left", self.config.get("left_path"), self.left_color)
        self.right = FileExplorer("Right", self.config.get("right_path"), self.right_color)

        self.left_to_right = QPushButton("→")
        self.right_to_left = QPushButton("←")
        self.move_checkbox = QCheckBox("Move files")

        self.left_to_right.clicked.connect(lambda: self.transfer(self.left, self.right))
        self.right_to_left.clicked.connect(lambda: self.transfer(self.right, self.left))

        color_btn = QPushButton("Color Settings")
        color_btn.clicked.connect(self.choose_color)

        btn_layout = QVBoxLayout()
        btn_layout.addWidget(self.left_to_right)
        btn_layout.addWidget(self.right_to_left)
        btn_layout.addWidget(self.move_checkbox)
        btn_layout.addWidget(color_btn)
        btn_layout.addStretch()

        layout = QHBoxLayout()
        layout.addWidget(self.left)
        layout.addLayout(btn_layout)
        layout.addWidget(self.right)

        container = QWidget()
        container.setLayout(layout)
        self.setCentralWidget(container)

    def transfer(self, src, dest):
        for file_name in src.get_selected_files():
            src_path = os.path.join(src.path_edit.text(), file_name)
            dest_path = os.path.join(dest.path_edit.text(), file_name)
            try:
                if self.move_checkbox.isChecked():
                    os.rename(src_path, dest_path)
                else:
                    if os.path.isdir(src_path):
                        QMessageBox.warning(self, "Warning", "Skipping folder copy")
                    else:
                        with open(src_path, 'rb') as fsrc:
                            with open(dest_path, 'wb') as fdst:
                                fdst.write(fsrc.read())
            except Exception as e:
                QMessageBox.warning(self, "Transfer Error", str(e))
        dest.load_files()
        src.load_files()

    def choose_color(self):
        color = QColorDialog.getColor()
        if color.isValid():
            if self.sender():
                self.left.setStyleSheet(f"background-color: {color.name()};")
                self.right.setStyleSheet(f"background-color: {color.name()};")
                self.left_color = self.right_color = color.name()

    def closeEvent(self, event):
        config = {
            "left_path": self.left.path_edit.text(),
            "right_path": self.right.path_edit.text(),
            "left_color": self.left_color,
            "right_color": self.right_color,
            "window_width": self.width(),
            "window_height": self.height()
        }
        with open(CONFIG_PATH, 'w') as f:
            json.dump(config, f)
        event.accept()

    def load_config(self):
        if os.path.exists(CONFIG_PATH):
            try:
                with open(CONFIG_PATH) as f:
                    return json.load(f)
            except:
                return {}
        return {}

if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = ExplorerWindow()
    win.show()
    sys.exit(app.exec())
