CC=gcc
CFLAGS=-Wall -Wextra -Werror -lcheck
LIBS=-lcheck
LFLAGS=-fprofile-arcs -ftest-coverage

all: clean s21_decimal.a test

s21_decimal.a: s21_decimal.o
	ar rcs s21_decimal.a s21_decimal.o
	cp s21_decimal.a libs21_decimal.a
	cp s21_decimal.a ls21_decimal.a
	ranlib libs21_decimal.a
	ranlib ls21_decimal.a

s21_decimal.o: s21_decimal.c
	$(CC) $(CFLAGS) -c s21_decimal.c -o s21_decimal.o

test: s21_decimal.a
	$(CC) $(CFLAGS) test.c s21_decimal.a -o test.out $(LIBS)
	./test.out

gcov_report: s21_decimal.c
	$(CC) $(CFLAGS) $(LFLAGS) test.c s21_decimal.c -o test.out $(LIBS)
	./test.out
	lcov -t "test" -o test.info -c -d .
	genhtml -o report test.info

open:
	open report/index.html

rebuild: clean all

clean:
	rm -rf *.gc* *.o *.a *.out *.info test.out
	rm -rf report
