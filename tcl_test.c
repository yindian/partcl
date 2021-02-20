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
#ifndef _WIN32
#include <sys/time.h>
#endif
#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#ifdef _WIN32
#include <windows.h>
const __int64 DELTA_EPOCH_IN_MICROSECS= 11644473600000000;
struct timezone2 
{
  __int32  tz_minuteswest; /* minutes W of Greenwich */
  BOOL     tz_dsttime;     /* type of dst correction */
};

struct timeval2 {
__int32    tv_sec;         /* seconds */
__int32    tv_usec;        /* microseconds */
};

int gettimeofday2(struct timeval2 *tv/*in*/, struct timezone2 *tz/*in*/)
{
  FILETIME ft;
  __int64 tmpres = 0;
  TIME_ZONE_INFORMATION tz_winapi;
  int rez=0;

   ZeroMemory(&ft,sizeof(ft));
   ZeroMemory(&tz_winapi,sizeof(tz_winapi));

    GetSystemTimeAsFileTime(&ft);

    tmpres = ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tv->tv_sec = (__int32)(tmpres*0.000001);
    tv->tv_usec =(tmpres%1000000);


   if (tz)
   {
    //_tzset(),don't work properly, so we use GetTimeZoneInformation
    rez=GetTimeZoneInformation(&tz_winapi);
    tz->tz_dsttime=(rez==2)?TRUE:FALSE;
    tz->tz_minuteswest = tz_winapi.Bias + ((rez==2)?tz_winapi.DaylightBias:0);
   }

  return 0;
}
#define gettimeofday gettimeofday2
#endif

// thanks @jbenet -- https://gist.github.com/jbenet/1087739
void current_utc_time(struct timespec* ts)
{

  /* #ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time */
  /*   clock_serv_t cclock; */
  /*   mach_timespec_t mts; */
  /*   host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock); */
  /*   clock_get_time(cclock, &mts); */
  /*   mach_port_deallocate(mach_task_self(), cclock); */
  /*   ts->tv_sec = mts.tv_sec; */
  /*   ts->tv_nsec = mts.tv_nsec; */
  /* #else */
  /*   clock_gettime(CLOCK_REALTIME, ts); */
  /* #endif */
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
