#g++ main.cpp -I/usr/include/imgui -I/usr/include/stb -lglfw -lGL -lGLEW -limgui -L/usr/lib/x86_64-linux-gnu -lstb -o imgui_example

# 컴파일러
CC = g++

INCDIRS = /usr/include/imgui /usr/include/stb .
LIBDIRS = /usr/lib/x86_64-linux-gnu

# 컴파일러 플래그
CFLAGS = -std=c++23 -Wall -Wextra $(addprefix -I,$(INCDIRS)) $(addprefix -L, $(LIBDIRS))

# 링커 플래그
LDFLAGS = -lglfw -lGL -lGLEW -limgui -lstb  # 필요한 라이브러리

# 소스 파일
SRC = main.cpp 

# 오브젝트 파일
OBJ = $(SRC:.cpp=.o)

# 실행 파일 이름
EXECUTABLE = imgui_example

# 빌드 규칙
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(OBJ) -o $(EXECUTABLE) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# 가짜 규칙 (필요한 경우)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 정리 규칙
clean:
	rm -f $(OBJ) $(EXECUTABLE)

# 실행 규칙 (선택 사항)
run: all
	./$(EXECUTABLE)

