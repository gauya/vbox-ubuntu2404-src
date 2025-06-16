SRCS = lexer.cpp
TARGET = lexer
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

CXX = g++
CXXFLAGS = -std=c++23 -Wall
LDFLAGS = 

INCDIR = .
LIBDIR = 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	@rm -f *.o lexer 2>/dev/null
#	-@rm  *.o lexer 2>/dev/null

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS)-I$(INCDIR) -L$(LDFLAGS) -o $(TARGET) 

test:
	g++ -std=c++23 -I. -DTEST lexer.cpp -o lexer

all: $(TARGET)


