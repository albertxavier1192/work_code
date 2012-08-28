#include <rpc/rpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "ssson.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
	CLIENT *cl;
	char *server;
	char *filename;
	struct event event = {88, "this is the event 8"};
	struct file file;
	int *result;

	if (argc != 3) {
		fprintf(stderr, "usage: %s host filename\n", argv[0]);
		exit(1);
	}

	server = argv[1];
	filename = argv[2];

	cl = clnt_create(server, SSSONPROG, SSSONVERS, "tcp");
	if (cl == NULL) {
		clnt_pcreateerror(server);
		exit(1);
	}

	result = send_event_1(&event, cl);
	if (result == NULL) {
		clnt_perror(cl, server);
		exit(1);
	}
	printf("return value of send_event_1(): %d\n", *result);

	int fd;
	struct stat buf;
	int len = 0;

	fd = open(filename, O_RDONLY);
	stat(filename, &buf);

	file.name = filename;
	file.data.data_len = buf.st_size;
	file.data.data_val = mmap(0, file.data.data_len, PROT_READ,
			MAP_SHARED, fd, 0);

	result = send_file_1(&file, cl);
	if (result == NULL) {
		clnt_perror(cl, server);
		exit(1);
	}
	printf("return value of send_file_1(): %d\n", *result);

	munmap(file.data.data_val, file.data.data_len);
	close(fd);

	struct file *rcv_file;

	int i = 1;
	rcv_file = rcv_event_1(&i, cl);

	if(rcv_file == NULL){
		clnt_perror(cl, server);
		exit(1);
	}

	//FILE *fp = fopen(rcv_file->name, "w");
	FILE *fp = fopen("rcv_temp", "w");

	fwrite(rcv_file->data.data_val, rcv_file->data.data_len, 1, fp);

	fclose(fp);
	printf("file %s\n",rcv_file->name);

	return 0;
}
