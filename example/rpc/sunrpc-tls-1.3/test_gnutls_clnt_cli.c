/* Command line interface test client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>
#include <rpc/rpc.h>
#include <gnutls/gnutls.h>
#include "test_gnutls.h"

static void override_tcp_functions (CLIENT *);
static void start_gnutls (int sock);
static void print_info (gnutls_session_t session);

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

  override_tcp_functions (cl);
  start_gnutls (sock);

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

  /* Print session info. */
  print_info (session);
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

/* This is an informational function which prints details of the GnuTLS
 * session.
 */
static void
print_info (gnutls_session_t session)
{
  const char *tmp;
  gnutls_credentials_type_t cred;
  gnutls_kx_algorithm_t kx;

  /* print the key exchange's algorithm name
   */
  kx = gnutls_kx_get (session);
  tmp = gnutls_kx_get_name (kx);
  printf ("- Key Exchange: %s\n", tmp);

  /* Check the authentication type used and switch
   * to the appropriate.
   */
  cred = gnutls_auth_get_type (session);
  switch (cred)
    {
    case GNUTLS_CRD_SRP:
      printf ("- SRP session with username <not supported>\n");
      // The following function has gone walkies in my version of GnuTLS:
      //gnutls_srp_server_get_username (session);
      break;

    case GNUTLS_CRD_ANON:	/* anonymous authentication */

      printf ("- Anonymous DH using prime of %d bits\n",
	      gnutls_dh_get_prime_bits (session));
      break;

    case GNUTLS_CRD_CERTIFICATE:	/* certificate authentication */

      /* Check if we have been using ephemeral Diffie Hellman.
       */
      if (kx == GNUTLS_KX_DHE_RSA || kx == GNUTLS_KX_DHE_DSS)
	{
	  printf ("\n- Ephemeral DH using prime of %d bits\n",
		  gnutls_dh_get_prime_bits (session));
	}

      /* if the certificate list is available, then
       * print some information about it.
       */
      //print_x509_certificate_info (session);

    case GNUTLS_CRD_PSK:
      printf ("- PSK\n");
      break;

    case GNUTLS_CRD_IA:
      printf ("- IA\n");
      break;
    }				/* switch */

  /* print the protocol's name (ie TLS 1.0) 
   */
  tmp = gnutls_protocol_get_name (gnutls_protocol_get_version (session));
  printf ("- Protocol: %s\n", tmp);

  /* print the certificate type of the peer.
   * ie X.509
   */
  tmp =
    gnutls_certificate_type_get_name (gnutls_certificate_type_get (session));

  printf ("- Certificate Type: %s\n", tmp);

  /* print the compression algorithm (if any)
   */
  tmp = gnutls_compression_get_name (gnutls_compression_get (session));
  printf ("- Compression: %s\n", tmp);

  /* print the name of the cipher used.
   * ie 3DES.
   */
  tmp = gnutls_cipher_get_name (gnutls_cipher_get (session));
  printf ("- Cipher: %s\n", tmp);

  /* Print the MAC algorithms name.
   * ie SHA1
   */
  tmp = gnutls_mac_get_name (gnutls_mac_get (session));
  printf ("- MAC: %s\n", tmp);
}
