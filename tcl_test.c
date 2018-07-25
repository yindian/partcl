#include <stdio.h>

#ifdef TCL_TEST

#define TCL_DISABLE_MALLOC false

#include "tcl.h"

int status = 0;
#define FAIL(...)                  \
  do {                             \
    printf("ERROR: " __VA_ARGS__); \
    status = 1;                    \
  } while (0)

void check_eval(struct tcl* tcl, const char* s, char* expected);


#define TCL_MALLOC(m) smalloc(m)
#define TCL_REALLOC(m) realloc(m)
#define TCL_FREE(m) sfree(m)

#include "tcl_test_lexer.h"

#include "tcl_test_subst.h"

#include "tcl_test_flow.h"

#include "tcl_test_math.h"

#include <time.h>

#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

// thanks @jbenet -- https://gist.github.com/jbenet/1087739
void current_utc_time(struct timespec* ts)
{
  #ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
  #else
    clock_gettime(CLOCK_REALTIME, ts);
  #endif
}

void check_eval(struct tcl* tcl, const char* s, char* expected)
{
  struct timeval ts1;
  struct timeval ts2;
  struct timeval ts3;
  struct timeval ts4;

  gettimeofday(&ts1, NULL);

  int destroy = 0;
  if (tcl == NULL) {
    struct tcl tmp;
    tcl_init(&tmp);
    tcl = &tmp;
    destroy = 1;
  }

  gettimeofday(&ts2, NULL);

  if (tcl_eval(tcl, s, strlen(s) + 1) == FERROR) {
    FAIL("eval returned error: %s, (%s)\n", tcl_string(tcl->result), s);
    status = 2;
  } else if (strcmp(tcl_string(tcl->result), expected) != 0) {
    FAIL("Expected %s, but got %s. (%s)\n", expected, tcl_string(tcl->result),
        s);
    status = 2;
  } else {
    printf("OK: %s -> %s\n", s, expected);
  }

  gettimeofday(&ts3, NULL);

  if (destroy) {
    tcl_destroy(tcl);
  }

  gettimeofday(&ts4, NULL);

  printf("\t\t total: %5.2f (init: %5.2f eval: %5.2f destroy: %5.2f )\n",
      (ts4.tv_usec - ts1.tv_usec) / 1.0,
      (ts2.tv_usec - ts1.tv_usec) / 1.0,
      (ts3.tv_usec - ts2.tv_usec) / 1.0,
      (ts4.tv_usec - ts3.tv_usec) / 1.0);
}

int main()
{
  test_lexer();
  test_subst();
  test_flow();
  test_math();
  return status;
}
#endif
