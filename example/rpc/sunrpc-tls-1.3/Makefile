# $Id: Makefile,v 1.5 2007/01/25 15:51:33 rjones Exp $

PACKAGE := sunrpc-tls
VERSION := 1.3

# To enable debugging, make sure you have installed:
#  glibc-debuginfo
# then enable the following:
#GLIBC_DEBUG := -static -L/usr/lib/debug/usr/lib64

CC	:= gcc
CFLAGS	:= -g -Wall -Werror

all:	test_svc test_clnt \
	test_nopmap_svc test_nopmap_clnt \
	test_gnutls_svc test_gnutls_clnt \
	test_gnutls_clnt_perf \
	test_batch_svc test_batch_clnt \
	test_ipv6_svc test_ipv6_clnt

clean:
	rm -f *~ core *.o \
	  test_svc test_clnt \
	  test_clnt.c test_svc.c test.h \
	  test_nopmap_svc test_nopmap_clnt \
	  test_nopmap_clnt.c test_nopmap_svc.c test_nopmap.h \
	  test_gnutls_svc test_gnutls_clnt \
	  test_gnutls_clnt.c test_gnutls_svc.c test_gnutls.h \
	  test_gnutls_clnt_perf \
	  test_batch_svc test_batch_clnt \
	  test_batch_clnt.c test_batch_svc.c test_batch.h \
	  test_ipv6_svc test_ipv6_clnt \
	  test_ipv6_clnt.c test_ipv6_svc.c test_ipv6.h

# Simple client/server, using portmap and no fixed port.

test_svc: test_svc.o test_svc_impl.o
	$(CC) $(CFLAGS) $^ -o $@

test_clnt: test_clnt.o test_clnt_cli.o
	$(CC) $(CFLAGS) $^ -o $@

test_svc.c test_clnt.c test.h: test.x
	rpcgen test.x

# Client/server, without portmap on fixed port 5000.

test_nopmap_svc: test_nopmap_svc.o test_nopmap_svc_impl.o
	$(CC) $(CFLAGS) $^ -o $@

test_nopmap_svc.o: test_nopmap_svc.c
test_nopmap_svc.o: test_nopmap.h

test_nopmap_clnt: test_nopmap_clnt.o test_nopmap_clnt_cli.o
	$(CC) $(CFLAGS) $^ -o $@

test_nopmap_clnt.c test_nopmap.h: test_nopmap.x
	rpcgen test_nopmap.x
	rm -f test_nopmap_svc.c
	$(MAKE) test_nopmap_svc.c

test_nopmap_svc.c: test_nopmap.x
	rpcgen -m $< > $@

# Client/server, GNUTLS transport (no portmap, fixed port 5000).

test_gnutls_svc: test_gnutls_svc.o test_gnutls_svc_gnutls.o test_gnutls_svc_impl.o
	$(CC) $(CFLAGS) $^ -lgnutls -o $@

test_gnutls_svc.o: test_gnutls_svc.c
test_gnutls_svc.o: test_gnutls.h

test_gnutls_clnt: test_gnutls_clnt.o test_gnutls_clnt_cli.o
	$(CC) $(CFLAGS) $^ -lgnutls -o $@

test_gnutls_clnt.c test_gnutls.h: test_gnutls.x
	rpcgen test_gnutls.x
	rm -f test_gnutls_svc.c
	$(MAKE) test_gnutls_svc.c

test_gnutls_svc.c: test_gnutls.x
	rpcgen -m $< > $@

# For performance testing of SunRPC over GnuTLS.

test_gnutls_clnt_perf: test_gnutls_clnt.o test_gnutls_clnt_perf.o
	$(CC) $(CFLAGS) $^ -lgnutls -o $@

# Test batching.

test_batch_svc: test_batch_svc.o test_batch_svc_impl.o
	$(CC) $(CFLAGS) $(GLIBC_DEBUG) $^ -o $@

test_batch_svc.o: test_batch_svc.c
test_batch_svc.o: test_batch.h

test_batch_clnt: test_batch_clnt.o test_batch_clnt_cli.o
	$(CC) $(CFLAGS) $(GLIBC_DEBUG) $^ -o $@

test_batch_clnt.c test_batch.h: test_batch.x
	rpcgen test_batch.x
	rm -f test_batch_svc.c
	$(MAKE) test_batch_svc.c

test_batch_svc.c: test_batch.x
	rpcgen -m $< > $@

# Test IPv6 support.

test_ipv6_svc: test_ipv6_svc.o test_ipv6_svc_impl.o
	$(CC) $(CFLAGS) $^ -o $@

test_ipv6_svc.o: test_ipv6_svc.c
test_ipv6_svc.o: test_ipv6.h

test_ipv6_clnt: test_ipv6_clnt.o test_ipv6_clnt_cli.o
	$(CC) $(CFLAGS) $^ -o $@

test_ipv6_clnt.c test_ipv6.h: test_ipv6.x
	rpcgen test_ipv6.x
	rm -f test_ipv6_svc.c
	$(MAKE) test_ipv6_svc.c

test_ipv6_svc.c: test_ipv6.x
	rpcgen -m $< > $@

# Set up symlinks (rjones's private dev only)
symlinks:
	rm -f demoCA newcert.pem newkey.pem
	ln -s ../xmlrpc/demoCA
	ln -s ../xmlrpc/newcert.pem
	ln -s ../xmlrpc/newkey.pem

# Distribution.

dist:
	$(MAKE) check-manifest
	rm -rf $(PACKAGE)-$(VERSION)
	mkdir $(PACKAGE)-$(VERSION)
	tar -cf - -T MANIFEST | tar -C $(PACKAGE)-$(VERSION) -xf -
	tar zcf $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE)-$(VERSION)
	rm -rf $(PACKAGE)-$(VERSION)
	ls -l $(PACKAGE)-$(VERSION).tar.gz

check-manifest:
	@for d in `find -type d -name CVS | grep -v '^\./debian/'`; \
	do \
	b=`dirname $$d`/; \
	awk -F/ '$$1 != "D" {print $$2}' $$d/Entries | \
	sed -e "s|^|$$b|" -e "s|^\./||"; \
	done | sort > .check-manifest; \
	sort MANIFEST > .orig-manifest; \
	diff -u .orig-manifest .check-manifest; rv=$$?; \
	rm -f .orig-manifest .check-manifest; \
	exit $$rv
