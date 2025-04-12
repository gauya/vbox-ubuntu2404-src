#! /usr/bin/python

import os
import json
from PySide6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QListWidget,
    QPushButton, QFileDialog, QLabel, QLineEdit, QListWidgetItem,
    QCheckBox
)
from PySide6.QtCore import Qt
import shutil
import fnmatch

CONFIG_FILE = os.path.expanduser("~/.myexplorer")

class FileExplorer(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Dual Pane File Explorer")
        self.setMinimumSize(900, 700)

        layout = QHBoxLayout(self)

        self.left_pane = self.create_pane()
        self.right_pane = self.create_pane()

        layout.addLayout(self.left_pane["layout"])
        layout.addLayout(self.create_controls())
        layout.addLayout(self.right_pane["layout"])

        self.setLayout(layout)

        self.load_last_folders()

    def create_pane(self):
        layout = QVBoxLayout()

        # Top folder path input + select folder button
        path_layout = QHBoxLayout()
        path_input = QLineEdit()
        browse_button = QPushButton("Folder")
        path_layout.addWidget(path_input)
        path_layout.addWidget(browse_button)

        pattern_input = QLineEdit("*")
        file_list = QListWidget()

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
                    full_path = os.path.join(folder, fname)
                    if os.path.isfile(full_path) and fnmatch.fnmatch(fname, pattern):
                        item = QListWidgetItem(fname)
                        item.setData(Qt.UserRole, full_path)
                        file_list.addItem(item)

        path_input.textChanged.connect(update_file_list)
        pattern_input.textChanged.connect(update_file_list)

        browse_button.clicked.connect(browse)

        layout.addLayout(path_layout)
        layout.addWidget(QLabel("Filter Pattern (e.g., *.txt):"))
        layout.addWidget(pattern_input)
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

        self.move_checkbox = QCheckBox("Move")
        to_right_btn = QPushButton("→")
        to_left_btn = QPushButton("←")

        to_right_btn.clicked.connect(lambda: self.transfer_files(self.left_pane, self.right_pane))
        to_left_btn.clicked.connect(lambda: self.transfer_files(self.right_pane, self.left_pane))

        layout.addWidget(self.move_checkbox)
        layout.addWidget(to_right_btn)
        layout.addWidget(to_left_btn)
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
                print(f"Failed to transfer {src_file} to {dst_file}: {e}")

        source["update_file_list"]()
        destination["update_file_list"]()

    def load_last_folders(self):
        default = os.path.expanduser("~")
        left_path = right_path = default
        if os.path.exists(CONFIG_FILE):
            try:
                with open(CONFIG_FILE, "r") as f:
                    data = json.load(f)
                    left_path = data.get("left", default)
                    right_path = data.get("right", default)
            except Exception as e:
                print("Failed to load config:", e)
        self.left_pane["path_input"].setText(left_path)
        self.left_pane["update_file_list"]()
        self.right_pane["path_input"].setText(right_path)
        self.right_pane["update_file_list"]()

    def closeEvent(self, event):
        try:
            data = {
                "left": self.left_pane["path_input"].text(),
                "right": self.right_pane["path_input"].text()
            }
            with open(CONFIG_FILE, "w") as f:
                json.dump(data, f)
        except Exception as e:
            print("Failed to save config:", e)
        super().closeEvent(event)

if __name__ == "__main__":
    app = QApplication([])
    window = FileExplorer()
    window.show()
    app.exec()
