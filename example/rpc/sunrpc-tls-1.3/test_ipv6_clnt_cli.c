/* Command line interface test client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include "test_ipv6.h"

int
main (int argc, char *argv[])
{
  if (argc != 1) {
    fprintf (stderr, "usage: %s\n", argv[0]);
    exit (1);
  }

  /* Create a TCP client connection to service listening on localhost:5000. */
  struct sockaddr_in6 addr;
  memset (&addr, 0, sizeof addr);
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons (5000);
  addr.sin6_addr = in6addr_loopback;

  /* Unlike in the IPv4 case we have to create our own socket here,
   * because the code in the standard TCP transport (clnt_tcp.c) assumes
   * IPv4.  For real we'll be writing a custom transport anyway so
   * we can have that understand IPv6.
   */
  int sock = socket (PF_INET6, SOCK_STREAM, 0);
  if (connect (sock, (struct sockaddr *) &addr, sizeof addr) == -1) {
    perror ("connect");
    exit (1);
  }

  /* Nasty cast here works because IPv4 and IPv6 address structures
   * are compatible up to the port field.  All that clnttcp_create
   * uses this for anyway is to determine if addr->sin_port is zero.
   */
  CLIENT *cl = clnttcp_create ((struct sockaddr_in *) &addr,
			       TESTPROG, TESTPROG_VERS1, &sock, 0, 0);
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

  /* We have to close it, because we opened it above. */
  close (sock);

  exit (0);
}
