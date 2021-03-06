/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "ssson.h"

bool_t
xdr_event (XDR *xdrs, event *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->event_id))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->desc, 128))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_file (XDR *xdrs, file *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, &objp->name, 128))
		 return FALSE;
	 if (!xdr_bytes (xdrs, (char **)&objp->data.data_val, (u_int *) &objp->data.data_len, ~0))
		 return FALSE;
	return TRUE;
}
