#include <stdio.h>

#define TEST
#include "tcl.c"

int status = 0;
#define FAIL(...)                                                              \
  do {                                                                         \
    printf("FAILED: " __VA_ARGS__);                                            \
    status = 1;                                                                \
  } while (0)


static void check_eval(struct tcl *tcl, const char *s, char *expected);

#include "tcl_test_lexer.h"

#include "tcl_test_subst.h"

#include "tcl_test_flow.h"

#include "tcl_test_math.h"

static void check_eval(struct tcl *tcl, const char *s, char *expected) {
  int destroy = 0;
  if (tcl == NULL) {
    struct tcl tmp;
    tcl_init(&tmp);
    tcl = &tmp;
    destroy = 1;
  }
  if (tcl_eval(tcl, s, strlen(s) + 1) == FERROR) {
    FAIL("eval returned error: %s, (%s)\n", tcl_string(tcl->result), s);
  } else if (strcmp(tcl_string(tcl->result), expected) != 0) {
    FAIL("Expected %s, but got %s. (%s)\n", expected, tcl_string(tcl->result),
         s);
  } else {
    printf("OK: %s -> %s\n", s, expected);
  }
  if (destroy) {
    tcl_destroy(tcl);
  }
}

int main() {
  test_lexer();
  test_subst();
  test_flow();
  test_math();
  return status;
}
