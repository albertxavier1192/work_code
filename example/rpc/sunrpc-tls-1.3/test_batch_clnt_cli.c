/* Command line interface test client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include "test_batch.h"

/* You can't call the auto-generated functions like test_batch1_1
 * because those functions on the server side are batched (they
 * don't return any result).  Instead, write custom wrappers
 * for those functions here.
 *
 * For details, see clnt_tcp.c and search for 'batch' and 'shipnow'.
 */
static struct timeval zero_timeout = {0, 0};

static void
test_batch1_1_asynch (char **argp, CLIENT *clnt)
{
  if (clnt_call (clnt, test_batch1,
		 (xdrproc_t) xdr_wrapstring, (caddr_t) argp,
		 NULL, NULL,
		 zero_timeout) != RPC_SUCCESS) {
    fprintf (stderr, "warning: batched call failed\n");
  }
}

static void
test_batch2_1_asynch (char **argp, CLIENT *clnt)
{
  if (clnt_call (clnt, test_batch2,
		 (xdrproc_t) xdr_wrapstring, (caddr_t) argp,
		 NULL, NULL,
		 zero_timeout) != RPC_SUCCESS) {
    fprintf (stderr, "warning: batched call failed\n");
  }
}

static void
test_batch3_1_asynch (char **argp, CLIENT *clnt)
{
  if (clnt_call (clnt, test_batch3,
		 (xdrproc_t) xdr_wrapstring, (caddr_t) argp,
		 NULL, NULL,
		 zero_timeout) != RPC_SUCCESS) {
    fprintf (stderr, "warning: batched call failed\n");
  }
}

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

  /* Make some batched calls. */
  char *s_batch1 = "batch1";
  char *s_batch2 = "batch2";
  char *s_batch3 = "batch3";
  test_batch1_1_asynch (&s_batch1, cl);
  test_batch2_1_asynch (&s_batch2, cl);
  test_batch3_1_asynch (&s_batch3, cl);
  test_batch1_1_asynch (&s_batch1, cl);
  test_batch2_1_asynch (&s_batch2, cl);
  test_batch3_1_asynch (&s_batch3, cl);
  test_batch1_1_asynch (&s_batch1, cl);
  test_batch2_1_asynch (&s_batch2, cl);
  test_batch3_1_asynch (&s_batch3, cl);

  /* Count number of calls sent. */
  int *count = test_num_calls_1 (NULL, cl);

  printf ("Server recorded %d calls (expected 9).\n", *count);

  /* Send another batch. */
  test_batch1_1_asynch (&s_batch1, cl);
  test_batch2_1_asynch (&s_batch2, cl);
  test_batch3_1_asynch (&s_batch3, cl);
  test_batch1_1_asynch (&s_batch1, cl);
  test_batch2_1_asynch (&s_batch2, cl);
  test_batch3_1_asynch (&s_batch3, cl);
  test_batch1_1_asynch (&s_batch1, cl);
  test_batch2_1_asynch (&s_batch2, cl);
  test_batch3_1_asynch (&s_batch3, cl);

  /* Count number of calls sent. */
  count = test_num_calls_1 (NULL, cl);

  printf ("Server recorded %d calls (expected 9).\n", *count);

  clnt_destroy (cl);

  exit (0);
}
