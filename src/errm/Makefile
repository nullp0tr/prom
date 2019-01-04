main:
	gcc -c errm.c -Wall -Wextra -pedantic -std=c17
tests:
	gcc tests.c errm.c -Wall -Wextra -pedantic -std=c17 -fsanitize=address -lcmocka -pthread -O3 -fsanitize=address && ./a.out
	gcc tests.c errm.c -Wall -Wextra -pedantic -std=c17 -fsanitize=address -lcmocka -pthread -fsanitize=address -fprofile-arcs -ftest-coverage && ./a.out
cov:
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
