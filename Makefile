CC ?= clang
CFLAGS ?= -Os -Wall -std=c99 -pedantic -DTCL_STANDALONE
LDFLAGS ?= -Os

TCLBIN := tcl

TEST_CC := clang
TEST_CFLAGS := -O0 -g -std=c11 -pedantic -DTCL_TEST
TEST_LDFLAGS := $(TEST_CFLAGS)
TCLTESTBIN := tcl_test

.c.o:
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $^

all: $(TCLBIN) test
# tcl: tcl.o tcllib.o

test: $(TCLTESTBIN)
	./tcl_test

tcllib_test.o: tcllib.c
	$(TEST_CC) $(TEST_LDFLAGS) -c -o $@ $^

$(TCLBIN): tcl.c tcllib.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(TCLTESTBIN): tcl_test.c tcllib_test.o
	$(TEST_CC) $(TEST_LDFLAGS) -o $@ $^

# tcl_test.o: tcl_test.c tcllib.c tcl_smalloc.c \
#	tcl_test_lexer.h tcl_test_subst.h tcl_test_flow.h tcl_test_math.h
#	$(TEST_CC) $(TEST_CFLAGS) -c tcl_test.c -o $@

format:
	clang-format -i *.c *.h

clean:
	rm -f $(TCLBIN) $(TCLTESTBIN) *.o *.gcda *.gcno

.PHONY: test clean fmt
