#include <stdlib.h>

#include <stdio.h>
#include <string.h>

#include "tcl.h"


#ifndef TCL_TEST
#ifdef TCL_STANDALONE

#define CHUNK 1024

int main() {
  struct tcl tcl;
  int buflen = CHUNK;
  char* buf = malloc(buflen);
  int i = 0;

  tcl_init(&tcl);
  while (1) {
    int inp = fgetc(stdin);

    if (i > buflen - 1)
      buf = realloc(buf, buflen += CHUNK);

    if (inp == 0 || inp == EOF)
      break;

    buf[i++] = inp;

    tcl_each(buf, i, 1) {
      if (p.token == TERROR && (p.to - buf) != i) {
        memset(buf, 0, buflen);
        i = 0;
      } else if (p.token == TCMD && *(p.from) != '\0') {
        int r = tcl_eval(&tcl, buf, strlen(buf));
        if (r != FERROR) {
          printf("result> %.*s\n", tcl_length(tcl.result),
                 tcl_string(tcl.result));
        } else {
          printf("?!\n");
        }

        memset(buf, 0, buflen);
        i = 0;
        break;
      }
    }
  }

  if(i) {
    printf("incomplete input\n");
    return -1;
  }

  return 0;
}
#endif
#endif
