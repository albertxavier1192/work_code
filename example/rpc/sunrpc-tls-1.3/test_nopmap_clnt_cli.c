/* Command line interface test client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include "test_nopmap.h"

int
main (int argc, char *argv[])
{
  if (argc != 1) {
    fprintf (stderr, "usage: %s\n", argv[0]);
    exit (1);
  }

  /* Create a TCP client connection to service listening on localhost:5000. */
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons (5000);
  addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  int sock = RPC_ANYSOCK;
  CLIENT *cl = clnttcp_create (&addr, TESTPROG, TESTPROG_VERS1, &sock, 0, 0);
  if (cl == NULL) {
    clnt_pcreateerror ("localhost");
    exit (1);
  }

  /* Make some client calls and print the results. */
  char **str;

  char *str1 = "Richard";
  str = test_hello_1 (&str1, cl);
  if (str == 0) {
    clnt_perror (cl, "localhost");
    exit (1);
  }
  printf ("test_hello (\"%s\") = \"%s\"\n", str1, *str);

  str1 = "Bob";
  str = test_hello_1 (&str1, cl);
  if (str == 0) {
    clnt_perror (cl, "localhost");
    exit (1);
  }
  printf ("test_hello (\"%s\") = \"%s\"\n", str1, *str);

  str = test_ctime_1 (NULL, cl);
  if (str == 0) {
    clnt_perror (cl, "localhost");
    exit (1);
  }
  printf ("test_ctime () = \"%s\"\n", *str);

  clnt_destroy (cl);

  exit (0);
}
