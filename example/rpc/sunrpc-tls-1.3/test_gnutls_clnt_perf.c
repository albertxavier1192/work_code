/* Command line interface test client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/poll.h>
#include <rpc/rpc.h>
#include <gnutls/gnutls.h>
#include "test_gnutls.h"

static void override_tcp_functions (CLIENT *);
static void start_gnutls (int sock);

int
main (int argc, char *argv[])
{
  if (argc != 2) {
    fprintf (stderr, "usage: %s count\n", argv[0]);
    exit (1);
  }

  int count;
  if (sscanf (argv[1], "%d", &count) != 1) {
    fprintf (stderr, "not a number\n");
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

  override_tcp_functions (cl);
  start_gnutls (sock);

  /* Time a large number of echo calls. */
  (void) test_num_calls_1 (NULL, cl); /* Reset the counter. */

  int i;
  for (i = 0; i < count; ++i)
    test_echo_1 (NULL, cl);

  int *check = test_num_calls_1 (NULL, cl);
  assert (*check == count);

  clnt_destroy (cl);

  exit (0);
}


/* Something of a hack: Here we avoid having to replicate the whole
 * of clnt_tcp.c to create a whole new backend transport.
 *
 * clnttcp_create has set cl->cl_private->ct_xdrs to an xdrrec which
 * serialises through private functions called readtcp and writetcp.
 * Now we are going to override those functions ...
 */

/* XXX This copies far too much private info from clnt_tcp.c.  We
 * can either write a proper backend, or else petition to get some
 * hooks added into glibc to make this easier.
 */
#define MCALL_MSG_SIZE 24

struct ct_data {
  int ct_sock;
  bool_t ct_closeit;
  struct timeval ct_wait;
  bool_t ct_waitset;          /* wait set by clnt_control? */
  struct sockaddr_in ct_addr;
  struct rpc_err ct_error;
  char ct_mcall[MCALL_MSG_SIZE];      /* marshalled callmsg */
  u_int ct_mpos;              /* pos after marshal */
  XDR ct_xdrs;
};

static int readtcp (char *, char *, int);
static int writetcp (char *, char *, int);

static void
override_tcp_functions (CLIENT *cl)
{
  struct ct_data *ct = (struct ct_data *) cl->cl_private;
  xdrrec_create (&(ct->ct_xdrs), 0, 0,
		 (caddr_t) ct, readtcp, writetcp);
}

#define CAFILE "demoCA/cacert.pem"

/* Mostly snarfed from:
 * http://www.gnu.org/software/gnutls/manual/gnutls.html#Simple-client-example-with-X_002e509-certificate-supportstatic void
 */

/* In a real implementation, this would be stored in the ct_data
 * structure.
 */
static gnutls_session_t session;

static void
start_gnutls (int sock)
{
  gnutls_certificate_credentials_t xcred;
    const int cert_type_priority[3] = { GNUTLS_CRT_X509,
    GNUTLS_CRT_OPENPGP, 0
  };

  gnutls_global_init ();

  /* X509 stuff */
  gnutls_certificate_allocate_credentials (&xcred);

  /* sets the trusted cas file
   */
  gnutls_certificate_set_x509_trust_file (xcred, CAFILE, GNUTLS_X509_FMT_PEM);

  /* Initialize TLS session 
   */
  gnutls_init (&session, GNUTLS_CLIENT);

  /* Use default priorities */
  gnutls_set_default_priority (session);
  gnutls_certificate_type_set_priority (session, cert_type_priority);

  /* put the x509 credentials to the current session
   */
  gnutls_credentials_set (session, GNUTLS_CRD_CERTIFICATE, xcred);

  gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) (long) sock);

  /* Perform the TLS handshake
   */
  int ret = gnutls_handshake (session);

  if (ret < 0)
    {
      fprintf (stderr, "*** Handshake failed\n");
      gnutls_perror (ret);
      exit (1);
    }

  /* XXX You need to verify the peer's certificate matches its name. */
  printf ("XXX need to verify peer's certificate matches its name.\n");
}

static int
readtcp (char *ctptr, char *buf, int len)
{
  struct ct_data *ct = (struct ct_data *)ctptr;
  struct pollfd fd;
  int milliseconds = (ct->ct_wait.tv_sec * 1000) +
    (ct->ct_wait.tv_usec / 1000);

  //printf ("client: overriding readtcp, len = %d\n", len);

  if (len == 0)
    return 0;

  /* The poll here is copied from the original readtcp.  It's
   * to allow the RPC layer to implement a timeout.
   */
  fd.fd = ct->ct_sock;
  fd.events = POLLIN;
  while (TRUE)
    {
      switch (poll(&fd, 1, milliseconds))
	{
	case 0:
	  ct->ct_error.re_status = RPC_TIMEDOUT;
	  return -1;

	case -1:
	  if (errno == EINTR)
	    continue;
	  ct->ct_error.re_status = RPC_CANTRECV;
	  ct->ct_error.re_errno = errno;
	  return -1;
	}
      break;
    }

  /* Perform the actual read using GnuTLS, which will read
   * one TLS "record", which is hopefully a complete message.
   */
  switch (len = gnutls_record_recv (session, buf, len))
    {
    case 0:
      /* premature eof */
      ct->ct_error.re_errno = ECONNRESET;
      ct->ct_error.re_status = RPC_CANTRECV;
      len = -1;			/* it's really an error */
      break;

    case -1:
      ct->ct_error.re_errno = errno;
      ct->ct_error.re_status = RPC_CANTRECV;
      break;
    }

  return len;
}

static int
writetcp (char *ctptr, char *buf, int len)
{
  struct ct_data *ct = (struct ct_data*)ctptr;

  //printf ("client: overriding writetcp, len = %d\n", len);

  if (gnutls_record_send (session, buf, len) < 0) {
    ct->ct_error.re_errno = errno;
    ct->ct_error.re_status = RPC_CANTSEND;
    return -1;
  }

  return len;
}
