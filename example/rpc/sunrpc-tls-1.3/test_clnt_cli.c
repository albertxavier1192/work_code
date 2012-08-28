/* Command line interface test client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include "test.h"

int
main (int argc, char *argv[])
{
  if (argc != 2) {
    fprintf (stderr, "usage: %s hostname\n", argv[0]);
    exit (1);
  }

  char *hostname = argv[1];

  CLIENT *cl = clnt_create (hostname, TESTPROG, TESTPROG_VERS1, "tcp");
  if (cl == NULL) {
    clnt_pcreateerror (hostname);
    exit (1);
  }

  /* Make some client calls and print the results. */
  char **str;

  char *str1 = "Richard";
  str = test_hello_1 (&str1, cl);
  if (str == 0) {
    clnt_perror (cl, hostname);
    exit (1);
  }
  printf ("test_hello (\"%s\") = \"%s\"\n", str1, *str);

  str1 = "Bob";
  str = test_hello_1 (&str1, cl);
  if (str == 0) {
    clnt_perror (cl, hostname);
    exit (1);
  }
  printf ("test_hello (\"%s\") = \"%s\"\n", str1, *str);

  str = test_ctime_1 (NULL, cl);
  if (str == 0) {
    clnt_perror (cl, hostname);
    exit (1);
  }
  printf ("test_ctime () = \"%s\"\n", *str);

  clnt_destroy (cl);

  exit (0);
}
