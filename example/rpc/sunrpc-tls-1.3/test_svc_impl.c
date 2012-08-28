/* Implementation of remote procedure calls.
 */

#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <rpc/rpc.h>
#include "test.h"

char **
test_hello_1_svc (char **str, struct svc_req *req)
{
  static char *ptr;
  int len = strlen (*str);
  /* XXX What cleans up this string? */
  ptr = malloc (6 + len + 1);
  strcpy (ptr, "Hello ");
  strcpy (ptr+6, *str);
  return &ptr;
}

char **
test_ctime_1_svc (void *v, struct svc_req *req)
{
  static char *ptr;
  time_t t;
  time (&t);
  ptr = ctime (&t);
  return &ptr;
}
