/* rpcgen ssson.x */
struct event {
	int event_id;
	string desc<128>;
};

struct file {
	string name<128>;
	opaque data<>;
};

program SSSONPROG {
	version SSSONVERS {
		int send_event(struct event) = 1;
		int send_file(struct file) = 2;
		struct file rcv_event(int id) = 3;
	} = 1;
} = 33330000;
