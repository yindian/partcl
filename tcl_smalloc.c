
#include <stdint.h>
#include <string.h>
#include "tcl.h"

/* #define MAX_VAR_LENGTH 128 */

#define MAX_STRING_LENGTH ((uint8_t)250)
#define MAX_AMOUNT_OF_STRINGTH ((uint8_t)255)

#define IN_USE ((uint8_t)1)
#define NOT_IN_USE ((uint8_t)0)

static uint8_t buffer[MAX_AMOUNT_OF_STRINGTH][MAX_STRING_LENGTH];
static uint8_t buf_info[MAX_AMOUNT_OF_STRINGTH];

void smalloc_init(void)
{
  printf("MALLOC_INIT\n");
  memset(&buffer, 0, sizeof buffer);
  memset(&buf_info, NOT_IN_USE, sizeof buf_info);
}

void* smalloc(size_t size)
{
  if (size > MAX_STRING_LENGTH) {
    puts("MALLOC ERROR: too big length");
    return NULL;
  }
  uint8_t buf_verifier = 0;
  while (buf_verifier < MAX_AMOUNT_OF_STRINGTH) {
    if (buf_info[buf_verifier] == NOT_IN_USE) {
      buf_info[buf_verifier] = IN_USE;
      return &buffer[buf_verifier][0];
    }
    buf_verifier++;
  }
  puts("MALLOC ERROR: no memory left");
  return NULL;
}

void* srealloc(void* ptr, size_t size)
{
  if (size > MAX_STRING_LENGTH) {
    puts("REALLOC ERROR: too big length");
    return NULL;
  }
  if (ptr == NULL)
    return smalloc(size);
  uint8_t str_num = 0;
  while (str_num < MAX_AMOUNT_OF_STRINGTH) {
    if ((ptr == &buffer[str_num][0]) && (buf_info[str_num] == IN_USE))
      return (void*)ptr;
    str_num++;
  }
  puts("REALLOC ERROR");
  return NULL;
}

void sfree(void* ptr)
{
  puts("free called");
  uint8_t str_num = 0;
  while (str_num < MAX_AMOUNT_OF_STRINGTH) {
    if (ptr == &buffer[str_num][0]) {
      memset(&buffer[str_num][0], 0, MAX_STRING_LENGTH);
      buf_info[str_num] = NOT_IN_USE;
      return;
    }
    str_num++;
  }
  puts("FREE ERROR");
}

