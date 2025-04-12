import glfw
from OpenGL.GL import *
import sys

def main():
    # GLFW 초기화
    if not glfw.init():
        return

    # 창 생성
    window = glfw.create_window(800, 600, "PyOpenGL + GLFW 예제", None, None)
    if not window:
        glfw.terminate()
        return

    # OpenGL 컨텍스트를 현재 스레드의 컨텍스트로 만듦
    glfw.make_context_current(window)

    # OpenGL 버전 출력 (확인용)
    print("OpenGL Vendor:", glGetString(GL_VENDOR))
    print("OpenGL Renderer:", glGetString(GL_RENDERER))
    print("OpenGL Version:", glGetString(GL_VERSION))

    # 배경색 설정
    glClearColor(0.1, 0.1, 0.1, 1.0)

    # 메인 루프
    while not glfw.window_should_close(window):
        # 이벤트 처리
        glfw.poll_events()

        # 버퍼 지우기
        glClear(GL_COLOR_BUFFER_BIT)

        # 간단한 삼각형 그리기 (오래된 방식)
        glBegin(GL_TRIANGLES)
        glColor3f(1.0, 0.0, 0.0)
        glVertex2f(0.0, 0.5)
        glColor3f(0.0, 1.0, 0.0)
        glVertex2f(-0.5, -0.5)
        glColor3f(0.0, 0.0, 1.0)
        glVertex2f(0.5, -0.5)
        glEnd()

        # 버퍼 스왑 (그린 것을 화면에 표시)
        glfw.swap_buffers(window)

    # 종료
    glfw.terminate()

if __name__ == "__main__":
    main()
