#include <rpc/rpc.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ssson.h"

int *send_event_1_svc(struct event *event, struct svc_req *rqstp)
{
	static int rc = 0;

	printf("got event[%d] %s\n", event->event_id, event->desc);

	return &rc;
}

int *send_file_1_svc(struct file *file, struct svc_req *rqstp)
{
	FILE *fp = fopen(file->name, "w");
	static int rc = 0;

	fwrite(file->data.data_val, file->data.data_len, 1, fp);

	fclose(fp);
	printf("file %s\n",file->name);

	return &rc;
}

struct file *rcv_event_1_svc(int *id, struct svc_req *rqstp)
{
	int fd;
	struct stat buf;
	static struct file file;
	fd = open("temp", O_RDONLY);
	stat("temp", &buf);

	file.name = "temp";
	file.data.data_len = buf.st_size;
	file.data.data_val = mmap(0, file.data.data_len, PROT_READ, 
			MAP_SHARED, fd, 0);
	close(fd);

	return &file;
}
