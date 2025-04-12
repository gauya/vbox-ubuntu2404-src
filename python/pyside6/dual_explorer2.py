#! /usr/bin/python

import os
from PySide6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QListWidget,
    QPushButton, QFileDialog, QLabel, QLineEdit, QListWidgetItem
)
from PySide6.QtCore import Qt
import shutil
import fnmatch

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

    def create_pane(self):
        layout = QVBoxLayout()
        path_input = QLineEdit()
        pattern_input = QLineEdit("*")
        file_list = QListWidget()

        def browse():
            folder = QFileDialog.getExistingDirectory(self, "Select Folder")
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

        browse_button = QPushButton("Select Folder")
        browse_button.clicked.connect(browse)

        layout.addWidget(QLabel("Folder Path:"))
        layout.addWidget(path_input)
        layout.addWidget(browse_button)
        layout.addWidget(QLabel("Filter Pattern (e.g., *.txt):"))
        layout.addWidget(pattern_input)
        layout.addWidget(file_list)

        return {
            "layout": layout,
            "path_input": path_input,
            "pattern_input": pattern_input,
            "file_list": file_list
        }

    def create_controls(self):
        layout = QVBoxLayout()
        layout.addStretch()

        to_right_btn = QPushButton("→")
        to_left_btn = QPushButton("←")

        to_right_btn.clicked.connect(lambda: self.transfer_files(self.left_pane, self.right_pane))
        to_left_btn.clicked.connect(lambda: self.transfer_files(self.right_pane, self.left_pane))

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
                shutil.copy2(src_file, dst_file)
            except Exception as e:
                print(f"Failed to copy {src_file} to {dst_file}: {e}")

        destination["file_list"].clear()
        for fname in os.listdir(dst_folder):
            if os.path.isfile(os.path.join(dst_folder, fname)):
                new_item = QListWidgetItem(fname)
                new_item.setData(Qt.UserRole, os.path.join(dst_folder, fname))
                destination["file_list"].addItem(new_item)

if __name__ == "__main__":
    app = QApplication([])
    window = FileExplorer()
    window.show()
    app.exec()
