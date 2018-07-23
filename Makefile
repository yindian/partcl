CC ?= clang
CFLAGS ?= -O2 -Wall -Wextra -std=c99 -pedantic
LDFLAGS ?= -O2

TCLBIN := tcl

TEST_CC := clang
TEST_CFLAGS := -O2 -g -std=c99 -pedantic -fprofile-arcs -ftest-coverage
TEST_LDFLAGS := $(TEST_CFLAGS)
TCLTESTBIN := tcl_test

all: $(TCLBIN) test
tcl: tcl.o

test: $(TCLTESTBIN)
	./tcl_test

$(TCLTESTBIN): tcl_test.o
	$(TEST_CC) $(TEST_LDFLAGS) -o $@ $^

tcl_test.o: tcl_test.c tcl.c \
	tcl_test_lexer.h tcl_test_subst.h tcl_test_flow.h tcl_test_math.h
	$(TEST_CC) $(TEST_CFLAGS) -c tcl_test.c -o $@

format:
	clang-format -i *.c *.h

clean:
	rm -f $(TCLBIN) $(TCLTESTBIN) *.o *.gcda *.gcno

.PHONY: test clean fmt
