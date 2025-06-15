clean:
	@rm -f *.o lexer 2>/dev/null
#	-@rm  *.o lexer 2>/dev/null

test:
	g++ -std=c++23 -I. -DTEST lexer.cpp -o lexer

all:
	g++ -std=c++23 -I. -c lexer.cpp


