#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif


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

uint64_t current_utc_time() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_usec;
}

static void check_eval(struct tcl *tcl, const char *s, char *expected) {
  uint64_t ts1;
  uint64_t ts2;
  uint64_t ts3;
  uint64_t ts4;

  ts1 = current_utc_time();

  int destroy = 0;
  if (tcl == NULL) {
    struct tcl tmp;
    tcl_init(&tmp);
    tcl = &tmp;
    destroy = 1;
  }

  ts2 = current_utc_time();

  if (tcl_eval(tcl, s, strlen(s) + 1) == FERROR) {
    FAIL("eval returned error: %s, (%s)\n", tcl_string(tcl->result), s);
  } else if (strcmp(tcl_string(tcl->result), expected) != 0) {
    FAIL("Expected %s, but got %s. (%s)\n", expected, tcl_string(tcl->result),
         s);
  } else {
    printf("OK: %s -> %s\n", s, expected);
  }

  /* current_utc_time(&ts3); */
  ts3 = current_utc_time();

  if (destroy) {
    tcl_destroy(tcl);
  }

  /* current_utc_time(&ts4); */
  ts4 = current_utc_time();

  printf("\t\t total: %5.2f (init: %5.2f eval: %5.2f destroy: %5.2f )\n",
            (ts4 - ts1 + 1) / 1.0,
            (ts2 - ts1 + 1) / 1.0,
            (ts3 - ts2 + 1) / 1.0,
            (ts4 - ts3 + 1) / 1.0);

}

int main() {
  test_lexer();
  test_subst();
  test_flow();
  test_math();
  return status;
}
