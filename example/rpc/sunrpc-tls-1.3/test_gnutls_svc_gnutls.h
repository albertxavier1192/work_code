#ifndef __TEST_GNUTLS_SVC_GNUTLS_H__
#define __TEST_GNUTLS_SVC_GNUTLS_H__

extern SVCXPRT *svcgnutls_create (int sock, u_int sendsize, u_int recvsize,
				  gnutls_certificate_credentials_t x509_cred);

#endif /* __TEST_GNUTLS_SVC_GNUTLS_H__ */
