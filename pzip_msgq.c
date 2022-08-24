#include <stdio.h>
#include <mqueue.h>

#include "pzip_msgq.h"

int create_message_queue(struct mq_fds *p_ret, const char *vritual_path, int maxmsg, int msgsize)
{
	struct mq_attr attr = {};

    attr.mq_flags = 0;
    attr.mq_maxmsg = maxmsg;
    attr.mq_msgsize = msgsize;
    attr.mq_curmsgs = 0;

	mq_unlink(vritual_path);

	if ((p_ret->mq_fd_recv = mq_open (vritual_path, O_RDONLY | O_CREAT, 0660, &attr)) == -1) {
		perror ("qd_bgr_thread_recv: mq_open (server)");
		return -1;
	}

	if ((p_ret->mq_fd_send = mq_open (vritual_path, O_WRONLY)) == -1) {
		perror ("qd_zip_thread_send: mq_open (client)");
		return -1;
	}

	return 0;
}

