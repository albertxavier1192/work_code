#include <stdio.h>
#include <rpc/rpc.h>
#include "ssson.h"

int callback();
gettransient(proto, vers, portnum)
        int proto;
        u_long vers;
        u_short portnum;
{
        static u_long prognum = 0x40000000;

        while (!pmap_set(prognum++, vers, proto, portnum))
                continue;
        return (prognum - 1);
}

main()
{
        int tmp_prog;
        char hostname[256] = "127.0.0.1";
        SVCXPRT *xprt;
        int stat;


        gethostname(hostname, sizeof(hostname));
        if ((xprt = svcudp_create(RPC_ANYSOCK)) == NULL) {
                  fprintf(stderr, "rpc_server: svcudp_create\n");
                exit(1);
        }
        if (tmp_prog = gettransient(IPPROTO_UDP, 1,
                        xprt->xp_port) == 0) {
                fprintf(stderr, "failed to get transient number\n");
                exit(1);
        }
        fprintf(stderr, "client gets prognum %d\n", tmp_prog);

        /* protocol is 0 - gettransient does registering */

        (void)svc_register(xprt, tmp_prog, 1, callback, 0);
        stat = callrpc(hostname, SSSONPROG, SSSONVERS,
          EXAMPLEPROC_CALLBACK, xdr_int, &tmp_prog, xdr_void, 0);

        if (stat != RPC_SUCCESS) {
                clnt_perrno(stat);
                exit(1);
        }
        svc_run();
        fprintf(stderr, "Error: svc_run shouldn't return\n");
}

 

callback(rqstp, transp)
        register struct svc_req *rqstp;
        register SVCXPRT *transp;
{
        switch (rqstp->rq_proc) {
                case 0:
                        if (!svc_sendreply(transp, xdr_void, 0)) {
                                fprintf(stderr, "err: exampleprog\n");
                                return (1);
                        }
                        return (0);

 

                case 1:
                        fprintf(stderr, "client got callback\n");
                        if (!svc_sendreply(transp, xdr_void, 0)) {
                                fprintf(stderr, "err: exampleprog\n");
                                return (1);
                        }
        }
        return (0);
}
