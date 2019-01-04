tests:
	gcc tests.c filesystem.c ../errm/errm.o ../dga/dga.o -Wall -Wextra -pedantic -fsanitize=address -lasan -lubsan -lcmocka && ./a.out
cov:
	gcc tests.c filesystem.c ../errm/errm.o ../dga/dga.o -Wall -Wextra -pedantic -std=c17 -lasan -lubsan -lcmocka -pthread -fsanitize=address -fprofile-arcs -ftest-coverage -g && ./a.out
	lcov --capture --directory . --output-file main_coverage.info
	genhtml main_coverage.info --output-directory out
	firefox out/index.html
clean:
	rm -rf out
	rm *.gcno
	rm *.gcda
	rm a.out
	rm *.info
	rm *.o
