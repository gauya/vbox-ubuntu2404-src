SRCS = lexer.cpp
TARGET = lexer
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

CXX = g++
CXXFLAGS = -std=c++23 -Wall -O3
LDFLAGS = 

INCDIR = -I.
LIBDIR = 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCDIR) $(LDFLAGS) $(SRCS) -o $(TARGET) 

clean:
	@rm -f *.o lexer 2>/dev/null
#	-@rm  *.o lexer 2>/dev/null

test: 
	g++ $(CXXFLAGS) $(INCDIR) $(LDFLAGS) -DTEST $(SRCS) -o $(TARGET) 

testg: 
	g++ $(CXXFLAGS) $(INCDIR) $(LDFLAGS) -g -DTEST $(SRCS) -o $(TARGET) 

all: $(TARGET)


