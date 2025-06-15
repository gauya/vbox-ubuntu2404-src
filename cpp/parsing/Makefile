parse:
	gcc parse.c -I./ -lm -std=gnu2x -DTEST -o parse

gstr:
	g++ -std=c++23 -I./ -c gstr.cpp parse.c

clean:
	@rm -f *.o parse 2>/dev/null
#	-@rm  *.o parse 2>/dev/null

