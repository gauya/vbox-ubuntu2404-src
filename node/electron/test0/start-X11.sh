#!/bin/bash

# Electron 앱 경로 설정 (필요시 수정)
APP_DIR="$(dirname "$0")"

# X11로 실행 강제
ELECTRON_CMD="npx electron . --ozone-platform=x11"

# X11 백엔드에서 실행 시도
echo "[INFO] Forcing Electron to use X11 (via XWayland)"
XDG_SESSION_TYPE=x11 \
    ELECTRON_ENABLE_WAYLAND=0 \
    QT_QPA_PLATFORM=xcb \
    GTK_USE_PORTAL=0 \
    $ELECTRON_CMD

